
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
        ConfigServicePtr _object(new ConfigService(location));
        if (!_object->init())
            return ConfigServicePtr(nullptr);
        
        return _object;
    }
    
    ConfigService::ConfigService(const std::string& location) :
        Service(location)
    {
        
	}
    
    void ConfigService::setConfigTempLocation(const std::string& tempConfigFileLocation)
    {
        m_configFileTempLocation = tempConfigFileLocation;
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
			request->setName("config");
            request->setAPIVersion(API_VERSION);
        
            request->setRequestArguments({
                {"gamespace", applicationInfo.gamespace}
            });
            
            m_configTempStream.open(m_configFileTempLocation, 
                static_cast<std::ios_base::openmode>(std::ios_base::beg | std::ios_base::trunc | std::ios_base::binary | std::ios_base::in | std::ios_base::out));

            request->setOnResponse([this, callback](const online::JsonRequest& request)
            {
				bool isRequestSuccessful = request.isSuccessful();
				bool isResponseValueValid = request.isResponseValueValid();

               if( isRequestSuccessful && isResponseValueValid )
               {
				   auto& value = request.getResponseValue();
                   if( value.isMember("url") )
                   {
                       std::string url = value["url"].asString();
                       
                       FileRequestPtr actualConfig = FileRequest::Create(url, Request::METHOD_GET, m_configTempStream);

                       if( actualConfig )
                       {
                           actualConfig->setOnResponse([this, callback](const online::FileRequest& actualConfig)
                           {                               
                               auto& dataStream = actualConfig.getResponse();

							   bool actualConfigIsSuccessful = actualConfig.isSuccessful();
							   bool dataStreamIsGood = dataStream.good();
                               if( actualConfigIsSuccessful && dataStreamIsGood )
                               {
                                   dataStream.seekg(std::ios_base::beg);
                                   
                                   std::string content = { std::istreambuf_iterator<char>(dataStream), std::istreambuf_iterator<char>() };
                                   dataStream.close();
                                   callback(*this, actualConfig.getResult(), actualConfig, content);
                               }
                               else
                               {
								   if( !actualConfigIsSuccessful )
								   {
									   Log::get() << "ERROR: Actual Config Is Unsuccessful!" << std::endl;
								   }

								   if( !dataStreamIsGood )
								   {
										Log::get() << "ERROR: Data Stream Is Bad!" << std::endl;
								   }

                                   dataStream.clear();
                                   dataStream.close();
                                   callback(*this, Request::INTERNAL_ERROR, actualConfig, std::string());
                               }
                           });
                       
                           actualConfig->start();
                       }
                       else
                       {
						   Log::get() << "ERROR: Data Stream Is Bad!" << std::endl;

                           callback(*this, Request::INTERNAL_ERROR, request, std::string());
                       }
                   }
                   else
                   {
					   Log::get() << "ERROR: Response without url!" << std::endl;

                        callback(*this, Request::INTERNAL_ERROR, request, std::string());
                   }
               }
               else
               {
				   if( !isRequestSuccessful )
					   Log::get() << "ERROR: Request is not succesful!" << std::endl;

				   if( !isResponseValueValid )
					   Log::get() << "ERROR: Response value is invalid!" << std::endl;

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
        if( m_configTempStream.is_open() )
            m_configTempStream.close();
    }
    
    bool ConfigService::init()
    {
        return true;
    }
}
