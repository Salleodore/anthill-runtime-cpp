
#include "anthill/AnthillRuntime.h"
#include "anthill/Utils.h"
#include <algorithm>

#include "anthill/services/EnvironmentService.h"
#include "anthill/services/DiscoveryService.h"
#include "anthill/services/LoginService.h"
#include "anthill/services/ProfileService.h"
#include "anthill/services/ConfigService.h"
#include "anthill/services/GameService.h"
#include "anthill/services/ExecService.h"
#include "anthill/services/MessageService.h"
#include "anthill/services/DLCService.h"
#include "anthill/services/StoreService.h"
#include "anthill/services/LeaderboardService.h"
#include "anthill/services/SocialService.h"
#include "anthill/services/EventService.h"
#include "anthill/services/QuestService.h"
#include "anthill/services/ReportService.h"

namespace online
{
	AnthillRuntimePtr AnthillRuntime::Create(
		const std::string& environment, 
		const std::set<std::string>& enabledServices, 
		StoragePtr storage,
        ListenerPtr listener,
		const ApplicationInfo& applicationInfo)
	{
		return AnthillRuntimePtr(new AnthillRuntime(environment, enabledServices, storage, listener, applicationInfo));
	}

	void AnthillRuntime::addRequest(RequestPtr request)
	{
		m_transport.add(request->getTransport());
		m_requests[&request->getTransport()] = request;
	}

	AnthillRuntime::AnthillRuntime(
		const std::string& environment, 
		const std::set<std::string>& enabledServices,
		StoragePtr storage,
        ListenerPtr listener,
		const ApplicationInfo& applicationInfo) :

		m_transport(),
		m_applicationInfo(applicationInfo),
        m_storage(storage),
        m_listener(listener),
        m_enabledServices(enabledServices)
	{
        Register<EnvironmentService>();
        Register<DiscoveryService>();
        Register<LoginService>();
        Register<ProfileService>();
        Register<ConfigService>();
        Register<GameService>();
        Register<ExecService>();
        Register<MessageService>();
        Register<DLCService>();
        Register<StoreService>();
        Register<LeaderboardService>();
        Register<SocialService>();
        Register<EventService>();
        Register<QuestService>();
        Register<ReportService>();
		
		SetService<EnvironmentService>(environment);
	}

	AnthillRuntime::~AnthillRuntime()
	{
		//
	}

	const StoragePtr& AnthillRuntime::getStorage() const
	{
		return m_storage;
	}
    
    const ListenerPtr& AnthillRuntime::getListener() const
    {
        return m_listener;
    }

	Futures& AnthillRuntime::getFutures()
	{
		return m_futures;
	}

	const std::set<std::string>& AnthillRuntime::getEnabledServices()
	{
		return m_enabledServices;
	}

	void AnthillRuntime::update(float dt)
	{
        while (!m_transport.perform());
        
        curl::curl_easy* next;
        
        while ((next = m_transport.get_next_finished()))
        {
            std::unordered_map<curl::curl_easy*, RequestPtr>::const_iterator it = m_requests.find(next);
            
            if (it != m_requests.end())
            {
                const RequestPtr& request = it->second;
                request->done();
                m_transport.remove(*next);
                m_requests.erase(it);
            }
        }
        
		m_futures.update(dt);
        
        for (const std::unordered_map<std::string, ServicePtr>::value_type& entry: m_services)
        {
            entry.second->update(dt);
        }
	}

	ServicePtr AnthillRuntime::SetService(const std::string& id, const std::string& location)
	{
		std::unordered_map<std::string, ServicePtr>::iterator service = m_services.find(id);

		if (service != m_services.end())
		{
			service->second->setLocation(location);
			return service->second;
		}

		std::unordered_map<std::string, ServiceCreator>::iterator creator = m_serviceCreators.find(id);

		if (creator != m_serviceCreators.end())
		{
			ServicePtr newService = creator->second(location);
			m_services[id] = newService;

			return newService;
		}
		else
		{
			OnlineAssert(false, "Unknown service: " + id);
		}

		return ServicePtr(nullptr);
	}

	const ApplicationInfo& AnthillRuntime::getApplicationInfo() const
	{
		return m_applicationInfo;
	}
}
