
#include "anthill/states/DiscoverServicesState.h"
#include "anthill/states/CheckExternalAuthenticationState.h"
#include "anthill/states/StateMachine.h"
#include "anthill/Log.h"
#include "anthill/Utils.h"
#include "anthill/AnthillRuntime.h"

using namespace std::placeholders;

namespace online
{
	DiscoverServicesState::DiscoverServicesState(std::shared_ptr<StateMachine> stateMachine, const std::set<std::string>& services) :
		State(stateMachine),
		m_services(services)
	{
	}

	void DiscoverServicesState::discoverServices(const DiscoveryService&, Request::Result result, const Request& request, const DiscoveredServices& services)
	{
        unlock();
    
		if (Request::isSuccessful(result))
		{
			Log::get() << "DiscoverServices successful!" << std::endl;

			DiscoveryServicePtr ptr = AnthillRuntime::Instance().get<DiscoveryService>();
			OnlineAssert((bool)ptr, "Discovery service is not initialized!");

			proceed();
		}
		else
		{
			Log::get() << "DiscoverServices failed: " << result << ": " << request.getResponseAsString() << std::endl;

			switch (result)
			{
				case Request::NOT_FOUND:
				{
					Log::get() << " ERROR: Not found service to discover!" << std::endl;
				}
                default:
                {
                    break;
                }
			}

			retry();
		}
	}

	void DiscoverServicesState::proceed()
	{
        AnthillRuntime& online = AnthillRuntime::Instance();
        const ListenerPtr& listener = online.getListener();

		if (listener)
		{
			listener->servicesDiscovered([this]
			{
				switchTo<CheckExternalAuthenticationState>();
			});
		}
		else
		{
			switchTo<CheckExternalAuthenticationState>();
		}
	}

	void DiscoverServicesState::init()
	{
		DiscoveryServicePtr ptr = AnthillRuntime::Instance().get<DiscoveryService>();
		OnlineAssert((bool)ptr, "Discovery service is not initialized!");
        
        lock();
		
		ptr->discoverServices(m_services,
			std::bind(&DiscoverServicesState::discoverServices, this, _1, _2, _3, _4));
	}

	void DiscoverServicesState::release()
	{
		//
	}

	DiscoverServicesState::~DiscoverServicesState()
	{
		//
	}
}
