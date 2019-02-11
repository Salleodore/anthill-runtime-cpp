
#include "anthill/services/ExecService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"
#include "anthill/Websockets.h"

#include <fstream>
#include <streambuf>

#include <json/writer.h>

namespace online
{
	const std::string ExecService::ID = "exec";
    const std::string ExecService::API_VERSION = "0.2";
    
    
    ExecSession::ExecSession(const std::string& location, const std::string& mode) :
        m_sockets(WebsocketRPC::Create()),
        m_location(location),
        m_mode(mode)
    {
    }

    void ExecSession::callMain(const Json::Value& arguments, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout)
    {
        call("main", arguments, success, failture, timeout);
    }
    
    void ExecSession::call(const std::string& method, const Json::Value& arguments, FunctionSuccessCallback success, FunctionFailCallback failture, float timeout)
    {
        Json::Value args(Json::ValueType::objectValue);
        
        args["method_name"] = method;
        args["arguments"] = arguments;

		if (!m_sockets->isConnected())
		{
			failture(599, "Socket is closed", "");
			return;
		}
    
        m_sockets->request("call", success, failture, args, timeout);
    }
    
    bool ExecSession::isActive() const
    {
        return m_sockets->isConnected();
    }
    
    void ExecSession::connect(const std::string& accessToken, const std::string& className, const Json::Value& args,
        ExecSession::SessionCreatedCallback onCreated, ExecSession::SessionClosedCallback onClosed)
    {
        const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        WebsocketRPC::Options options = {
            {"access_token", accessToken},
            {"args", Json::FastWriter().write(args)},
        };
        
        std::string path = m_location + "/" + m_mode + "/" + applicationInfo.applicationName + "/" +
            applicationInfo.applicationVersion + "/" + className;
        
        std::map<std::string, std::string> extraHeaders = {
            {"X-Api-Version", ExecService::API_VERSION}
        };
        
        m_sockets->connect(path, options,
            [=](bool success, int code)
        {
            if (success)
            {
                onCreated(*this, Request::SUCCESS);
            }
            else
            {
                onCreated(*this, (Request::Result)code);
            }
        }, onClosed, extraHeaders);
    }
    
    void ExecSession::update()
    {
        m_sockets->update();
    }
    
	void ExecSession::waitForShutdown()
	{
		m_sockets->waitForShutdown();
	}

    void ExecSession::disconnect(int code, const std::string& reason)
    {
        m_sockets->disconnect(code, reason);
    }
    
    ExecSessionPtr ExecSession::Create(const std::string& location)
    {
        return std::make_shared<ExecSession>(location);
    }
    
    //////////////////////
    
    ExecDebugSession::ExecDebugSession(const std::string& location, LogCallback logCallback, const std::string& scriptsDirectory) :
        ExecSession(location, "debug"),
        m_scriptsDirectory(scriptsDirectory)
    {
        m_sockets->handle("log", [logCallback](const Json::Value& params, JsonRPC::Success success, JsonRPC::Failture failture)
        {
            if (params.isMember("message"))
            {
                logCallback(params["message"].asString());
            }
        });
    }
    
    ExecDebugSessionPtr ExecDebugSession::Create(const std::string& location, LogCallback logCallback, const std::string& scriptsDirectory)
    {
        return std::make_shared<ExecDebugSession>(location, logCallback, scriptsDirectory);
    }
    
    inline bool ends_with(const std::string& value, const std::string& ending)
    {
        if (ending.size() > value.size()) return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }
    
    void ExecDebugSession::eval(const std::string& text, std::function<void(bool success, const std::string& result)> callback)
    {
        Json::Value args(Json::ValueType::objectValue);
        
        args["text"] = text;
        
        m_sockets->request("eval", [this, callback](const Json::Value& callResult)
        {
            if (callResult.isMember("result"))
            {
				if (callResult.isString())
				{
					callback(true, callResult["result"].asString());
				}
				else
				{
					callback(true, Json::FastWriter().write(callResult["result"]));
				}
            }
        }, [callback](int code, const std::string& reason, const std::string& data)
        {
            Log::get() << "Failed to evel: " << code << " " << reason << std::endl;
            callback(false, reason);
        }, args, 5.0f);
    }
    
    void ExecDebugSession::uploadFiles(std::function<void(bool success)> uploadCallback)
    {
        if (m_filesToUpload.empty())
        {
            uploadCallback(true);
            return;
        }
        
        std::string fileName = m_filesToUpload.back();
        m_filesToUpload.pop_back();
        
        std::ifstream fileStream(m_scriptsDirectory + "/" + fileName);
        std::string contents((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        
        Json::Value args(Json::ValueType::objectValue);
        
        args["filename"] = fileName;
        args["contents"] = contents;
        
        Log::get() << "Uploading script " << fileName << " ..." << std::endl;
        
        m_sockets->request("upload", [this, uploadCallback](const Json::Value& callResult)
        {
            if (m_filesToUpload.empty())
            {
                uploadCallback(true);
            }
            else
            {
                uploadFiles(uploadCallback);
            }
        }, [uploadCallback, fileName](int code, const std::string& reason, const std::string& data)
        {
            Log::get() << "Failed to upload script " << fileName << ": " << code << " " << reason << std::endl;
            uploadCallback(false);
        }, args, 5.0f);
        
    }
    
    void ExecDebugSession::startDebugging(DebugStartCallback callback)
    {
        if (!list_files_in_directory(m_scriptsDirectory, m_filesToUpload, [](const std::string& filename) -> bool { return ends_with(filename, ".js"); }))
        {
            callback(false);
        }
        
        uploadFiles([this, callback](bool success)
        {
            if (!success)
            {
                callback(false);
                return;
            }
            
            Json::Value args(Json::ValueType::objectValue);
            
            m_sockets->request("start", [callback](const Json::Value& callResult)
            {
                callback(true);
            }, [callback](int code, const std::string& reason, const std::string& data)
            {
                Log::get() << "Failed to start debugging: " << code << " " << reason << std::endl;
                callback(false);
            }, args, 5.0f);
        });
        
        int a = 0;
    }

    //////////////////////

	ExecServicePtr ExecService::Create(const std::string& location)
	{
		ExecServicePtr _object(new ExecService(location));
		if (!_object->init())
			return ExecServicePtr(nullptr);

		return _object;
	}

	ExecService::ExecService(const std::string& location) :
		Service(location)
	{
		
    }
    
    void ExecService::callMain(const std::string& accessToken, const Json::Value& arguments, FunctionCallCallback callback)
    {
        call(accessToken, "main", arguments, callback);
    }
    
    void ExecService::call(const std::string& accessToken, const std::string& method, const Json::Value& arguments, FunctionCallCallback callback)
    {
        OnlineAssert(arguments.isArray() || arguments.isObject(), "Arguments should be an array or an object.");
    
        const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/call/" + applicationInfo.applicationName + "/" + applicationInfo.applicationVersion + "/" + method, Request::METHOD_POST);
        
		if (request)
        {
			request->setName("exec_call");
            request->setAPIVersion(API_VERSION);
        
            Json::FastWriter fastWriter;
            
            request->setPostFields({
                {"access_token", accessToken },
                {"args", fastWriter.write(arguments) }
            });
            
			request->setOnResponse([=](const online::JsonRequest& request)
			{
                const Json::Value& value = request.getResponseValue();
                callback(*this, request.getResult(), request, value);
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
    }
    
    ExecSessionPtr ExecService::session()
    {
        return ExecSession::Create(getLocation());
    }
    
    ExecDebugSessionPtr ExecService::debugSession(ExecDebugSession::LogCallback logCallback, const std::string& scriptsDirectory)
    {
        return ExecDebugSession::Create(getLocation(), logCallback, scriptsDirectory);
    }
    
	ExecService::~ExecService()
	{
		//
	}

	bool ExecService::init()
	{
		return true;
	}
}
