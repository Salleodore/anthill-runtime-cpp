
#include "anthill/services/EnvironmentService.h"
#include "anthill/requests/JsonRequest.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"

namespace online
{
	const std::string EnvironmentService::ID = "environment";
    const std::string EnvironmentService::API_VERSION = "0.2";

	EnvironmentServicePtr EnvironmentService::Create(const std::string& environmentLocation)
	{
		EnvironmentServicePtr _object(new EnvironmentService(environmentLocation));
		if (!_object->init())
			return EnvironmentServicePtr(nullptr);

		return _object;
	}

	EnvironmentService::EnvironmentService(const std::string& location) :
		Service(location)
	{
		
	}

	void EnvironmentService::setDiscoveryLocation(const std::string& location)
	{
		AnthillRuntime::Instance().SetService<DiscoveryService>(location);
	}

	DiscoveryServicePtr EnvironmentService::getDiscoveryService() const
	{
		return m_discovery;
	}

	void EnvironmentService::getEnvironmentInfo(EnvironmentService::EnvironmentInfoCallback callback)
	{
		const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();

        JsonRequestPtr request = JsonRequest::Create(
			getLocation() + "/" + applicationInfo.applicationName + "/" + applicationInfo.applicationVersion, Request::METHOD_GET);
		if (request)
		{
			request->setName("environment");
            request->setAPIVersion(API_VERSION);
        
			request->setOnResponse([=](const online::JsonRequest& request)
			{
				if (request.isSuccessful())
				{
					const Json::Value& value = request.getResponseValue();

					if (value.isMember("discovery"))
					{
						const std::string& discovery = value["discovery"].asString();
						callback(*this, request.getResult(), request, discovery, value);
					}
					else
					{
						callback(*this, Request::MISSING_RESPONSE_FIELDS, request, "", value);
					}
				}
				else
				{
					callback(*this, request.getResult(), request, "", EnvironmentInformation());
				}
			});
		}
		else
		{
			OnlineAssert(false, "Failed to construct a request.");
		}

		request->start();
	}

	EnvironmentService::~EnvironmentService()
	{
		//
	}

	bool EnvironmentService::init()
	{
		return true;
	}
}
