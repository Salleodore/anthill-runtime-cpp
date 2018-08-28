
#include "anthill/services/DLCService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include <json/writer.h>

namespace online
{
    const std::string DLCService::ID = "dlc";
    const std::string DLCService::API_VERSION = "0.2";
    
    DLCServicePtr DLCService::Create(const std::string& location)
    {
        DLCServicePtr _object(new DLCService(location));
        if (!_object->init())
            return DLCServicePtr(nullptr);
        
        return _object;
    }
    
    DLCService::DLCService(const std::string& location) :
        Service(location)
    {
        
	}
    
    void DLCService::getUpdates(DLCService::Bundles& bundlesOutput, GetUpdatesCallback callback)
    {
        const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

		Json::Value env;
        getUpdates(bundlesOutput, callback, applicationInfo.applicationName, applicationInfo.applicationVersion, env);
    }
    
    void DLCService::getUpdates(DLCService::Bundles& bundlesOutput, GetUpdatesCallback callback,
        const std::string& applicationName, const std::string& applicationVersion,
		const Json::Value& env)
    {
        JsonRequestPtr request = JsonRequest::Create(
            getLocation() + "/data/" + applicationName + "/" + applicationVersion,
            Request::METHOD_GET);
        
        if (request)
        {
            request->setAPIVersion(API_VERSION);
        
            Json::FastWriter fastWriter;

			request->setRequestArguments({
				{"env", fastWriter.write(env)}
			});

            request->setOnResponse([this, callback, &bundlesOutput](const online::JsonRequest& request)
            {
                if (request.isSuccessful() && request.isResponseValueValid())
                {
                    const Json::Value& value = request.getResponseValue();
                   
                    if (value.isMember("bundles"))
                    {
                        const Json::Value& bundles = value["bundles"];
                        
                        bundlesOutput.clear();
                        
                        for (Json::ValueConstIterator it = bundles.begin(); it != bundles.end(); it++)
                        {
                            const std::string& name = it.key().asString();
                            const Json::Value& value = *it;
                            
                            std::string url = value["url"].asString();
                            std::string sha256 = value["hash"].asString();
							std::string crc32;
							if( value.isMember( "crc32" ) )
								crc32 = value["crc32"].asString();

                            long size = value["size"].asLargestInt();
                            const Json::Value& payload = value["payload"];
                            
                            bundlesOutput.emplace_back(name, size, url, sha256, crc32, payload);
                        }
                    
                        callback(*this, request.getResult(), request);
                    }
                    else
                    {
                        callback(*this, Request::INTERNAL_ERROR, request);
                    }
                }
                else
                {
                    callback(*this, request.getResult(), request);
                }
            });
        }
        else
        {
            OnlineAssert(false, "Failed to construct a request.");
        }
        
        request->start();
    }
    
    DLCService::~DLCService()
    {
        //
    }
    
    bool DLCService::init()
    {
        return true;
    }
}
