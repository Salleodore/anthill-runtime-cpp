
#include "anthill/services/ConfigService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/requests/FileRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include <json/writer.h>

namespace online
{
    const std::string ConfigService::ID = "config";
    const std::string ConfigService::API_VERSION = "0.2";
    
    ConfigServicePtr ConfigService::Create(const std::string& location)
    {
        ConfigServicePtr _object(new ConfigService(location, "TEMP-CONFIG.zip"));
        if (!_object->init())
            return ConfigServicePtr(nullptr);
        
        return _object;
    }
    
    ConfigService::ConfigService(const std::string& location, const std::string& configFileTempLocation) :
        Service(location), m_configFile(configFileTempLocation.c_str(), std::ios_base::beg | std::ios_base::trunc | std::ios_base::binary | std::ios_base::in | std::ios_base::out)
    {
        
	}
    
    void ConfigService::getConfig(GetConfigCallback callback)
    {
        const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

		std::string url = getLocation() + "/config/" +
            applicationInfo.applicationName + "/" + applicationInfo.applicationVersion;
        JsonRequestPtr request = JsonRequest::Create(
            url,
            Request::METHOD_GET);
        

        if (request)
        {
            request->setAPIVersion(API_VERSION);
        
            request->setRequestArguments({
                {"gamespace", applicationInfo.gamespace}
            });
            
            request->setOnResponse([this, callback](const online::JsonRequest& request)
            {
               if (request.isSuccessful() && request.isResponseValueValid())
               {
				   auto& value = request.getResponseValue();
                   if (value.isMember("url"))
                   {
                       std::string url = value["url"].asString();
                       
                       FileRequestPtr actualConfig = FileRequest::Create(url, Request::METHOD_GET, m_configFile);

                       //JsonRequestPtr actualConfig = JsonRequest::Create(url, Request::METHOD_GET);
                       
                       if (actualConfig)
                       {
                           //actualConfig->setParseAsJsonAnyway();
                           
                           actualConfig->setOnResponse([this, callback](const online::FileRequest& actualConfig)
                           {
                               if (actualConfig.isSuccessful())
                               {
                                   auto& data = actualConfig.getResponse();
                                   data.seekg(std::ios_base::beg);
                                   
                                   std::string content = { std::istreambuf_iterator< char >(data), std::istreambuf_iterator< char >() };
                                   callback(*this, actualConfig.getResult(), actualConfig, content);
                               }
                               else
                               {
                                   callback(*this, Request::INTERNAL_ERROR, actualConfig, std::string());
                               }
                           });
                       
                           actualConfig->start();
                       }
                       else
                       {
                           callback(*this, Request::INTERNAL_ERROR, request, std::string());
                       }
                   }
                   else
                   {
                        callback(*this, Request::INTERNAL_ERROR, request, std::string());
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
    
    ConfigService::~ConfigService()
    {
        //
    }
    
    bool ConfigService::init()
    {
        return true;
    }
}
