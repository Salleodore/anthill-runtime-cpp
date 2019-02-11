
#include "anthill/services/DiscoveryService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

#include <numeric>

namespace online
{
	const std::string DiscoveryService::ID = "discovery";
    const std::string DiscoveryService::API_VERSION = "0.2";

	DiscoveryServicePtr DiscoveryService::Create(const std::string& location)
	{
		DiscoveryServicePtr _object(new DiscoveryService(location));
		if (!_object->init())
			return DiscoveryServicePtr(nullptr);

		return _object;
	}

	DiscoveryService::DiscoveryService(const std::string& location) :
		Service(location)
	{
		
    }
    
    void DiscoveryService::setServices(const Json::Value& services)
    {
        m_services.clear();
        
        AnthillRuntime& online = AnthillRuntime::Instance();
        
        for (Json::ValueConstIterator it = services.begin(); it != services.end(); it++)
        {
            const std::string& id = it.key().asString();
            const std::string& location = it->asString();
            
            m_services[id] = online.SetService(id, location);
        }
    }


	void DiscoveryService::discoverServices(const std::set<std::string>& services, DiscoveryService::DiscoveryInfoCallback callback)
	{
		std::string ids = join(services, ",");
		
		JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/services/" + ids, Request::METHOD_GET);

		if (request)
		{
			request->setName("services");
            request->setAPIVersion(API_VERSION);
        
			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful() && request.isResponseValueValid())
				{
					const Json::Value& value = request.getResponseValue();
                    setServices(value);

					callback(*this, request.getResult(), request, m_services);
				}
				else
				{
					callback(*this, request.getResult(), request, DiscoveredServices());
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}

	DiscoveryService::~DiscoveryService()
	{
		//
	}

	bool DiscoveryService::init()
	{
		return true;
	}
}
