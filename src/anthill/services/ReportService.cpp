
#include "anthill/services/ReportService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include <json/writer.h>
#include <string>

namespace online
{
    const std::string ReportService::ID = "report";
    const std::string ReportService::API_VERSION = "0.2";
    
    const std::array<std::string, (size_t)ReportService::ReportFormat::count> ReportService::ReportFormatValues = {
        {
            "json",
            "binary",
            "text"
        }
    };
    
    //////////////////
    
    ReportServicePtr ReportService::Create(const std::string& location)
    {
        ReportServicePtr _object(new ReportService(location));
        if (!_object->init())
            return ReportServicePtr(nullptr);
        
        return _object;
    }
    
    ReportService::ReportService(const std::string& location) :
        Service(location)
    {
        
	}
 
    void ReportService::uploadReportJSON(const std::string& category, const std::string& message,
        const Json::Value& info, const Json::Value& contents,
        const std::string& accessToken, UploadReportCallback callback)
    {
        uploadReport(category, message, ReportFormat::json, info, Json::FastWriter().write(contents), accessToken, callback);
    }

    void ReportService::uploadReport(const std::string& category, const std::string& message,
        ReportFormat format, const Json::Value& info, const std::string& contents,
        const std::string& accessToken, UploadReportCallback callback)
    {
        const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        JsonRequestPtr request = JsonRequest::Create(getLocation() + "/upload/" + applicationInfo.applicationName + "/" + applicationInfo.applicationVersion,
            Request::METHOD_PUT);
        
        if (request)
        {
			request->setName("report_upload");
            request->setAPIVersion(API_VERSION);
        
            request->setRequestArguments({
                {"category", category},
                {"message", message},
                {"format", ReportFormatValues[(int)format]},
                {"info", Json::FastWriter().write(info)},
                {"access_token", accessToken}
            });
            
            request->setRequestBody(contents);
            
            request->setOnResponse([this, callback](const online::JsonRequest& request)
            {
                if (request.isSuccessful() && request.isResponseValueValid())
                {
                    const Json::Value& value = request.getResponseValue();
                   
                    if (value.isMember("id"))
                    {
                        std::string requestId = value["id"].asString();
                        callback(*this, request.getResult(), request, requestId);
                    }
					else
					{
						callback(*this, request.getResult(), request, std::string());
					}
                }
                else
                {
                    callback(*this, request.getResult(), request, std::string());
                }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
    }

    void ReportService::uploadReport(const std::string& category, const std::string& message,
        ReportFormat format, const Json::Value& info, std::istream& contents,
        const std::string& accessToken, ReportService::UploadReportCallback callback)
    {
        const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        JsonRequestPtr request = JsonRequest::Create(getLocation() + "/upload/" + applicationInfo.applicationName + "/" + applicationInfo.applicationVersion,
            Request::METHOD_PUT);
        
        if (request)
        {
			request->setName("report_upload2");
            request->setAPIVersion(API_VERSION);
        
            request->setRequestArguments({
                {"category", category},
                {"message", message},
                {"format", ReportFormatValues[(int)format]},
                {"info", Json::FastWriter().write(info)},
                {"access_token", accessToken}
            });
            
            request->setRequestBody(contents);
            
            request->setOnResponse([this, callback](const online::JsonRequest& request)
            {
                if (request.isSuccessful() && request.isResponseValueValid())
                {
				    const Json::Value& value = request.getResponseValue();
                   
                    if (value.isMember("id"))
                    {
                        std::string requestId = value["id"].asString();
                        callback(*this, request.getResult(), request, requestId);
                    }
                   
                    callback(*this, request.getResult(), request, std::string());
                }
                else
                {
                    callback(*this, request.getResult(), request, std::string());
                }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
    }
    
    ReportService::~ReportService()
    {
        //
    }
    
    bool ReportService::init()
    {
        return true;
    }
}
