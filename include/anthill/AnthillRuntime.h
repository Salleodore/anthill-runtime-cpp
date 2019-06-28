
#ifndef ONLINE_LIB_H
#define ONLINE_LIB_H

#include "ApplicationInfo.h"
#include "Future.h"
#include "Singleton.h"

#include "Storage.h"
#include "OnlineListener.h"

#include "services/Service.h"
#include "requests/Request.h"
#include "requests/JsonRequest.h"
#include "services/EnvironmentService.h"

#include "curl_multi.h"

#include <set>
#include <unordered_map>

namespace online
{
	typedef std::shared_ptr< class AnthillRuntime > AnthillRuntimePtr;

	class AnthillRuntime : public Singleton<AnthillRuntime>
	{
	private:
		typedef std::function<ServicePtr (const std::string&)> ServiceCreator;

		template <class T>
		void Register()
		{
			m_serviceCreators[T::ID] = [](const std::string& location) 
			{ 
				return T::Create(location); 
			};
		}

	public:
		static AnthillRuntimePtr Create(
			const std::string& environment,
			const std::set<std::string>& enabledServices, 
			StoragePtr storage,
            ListenerPtr listener,
			const ApplicationInfo& applicationInfo);

		virtual ~AnthillRuntime();

		// updates the online lib, should be called every frame
		void update(float dt);

		const std::set<std::string>& getEnabledServices();

		const ApplicationInfo& getApplicationInfo() const;
		Futures& getFutures();

		const StoragePtr& getStorage() const;
        const ListenerPtr& getListener() const;

		ServicePtr SetService(const std::string& id, const std::string& location);

		template <class T>
		ServicePtr SetService(const std::string& location)
		{
			return SetService(T::ID, location);
		}

		template <class T>
		std::shared_ptr<T> get()
		{
			std::unordered_map<std::string, ServicePtr>::iterator service = m_services.find(T::ID);

			if (service == m_services.end())
			{
				return std::shared_ptr<T>(nullptr);
			}

			return std::static_pointer_cast<T>(service->second);
		}

		// adds a request to a process loop
		void addRequest(RequestPtr request);

		const std::function< void(std::string&,std::string&) >& getGenerateGuestUserCredentialsFunction() const { return m_generateGuestUserCredentialsFunction; }
		void setGenerateGuestUserCredentialsFunction( const std::function< void(std::string&,std::string&) >& function ){ m_generateGuestUserCredentialsFunction = function; }
		
		void setSocketDisconnectedInfoCallback( std::function< void( int code, const std::string& str ) > callback ) { m_onSocketDisconnectedInfo = callback; }

		void processSocketDisconnectionInfo( int code, const std::string& str )
		{
			if( m_onSocketDisconnectedInfo )
				m_onSocketDisconnectedInfo( code, str );
		}

	protected:
		AnthillRuntime(
			const std::string& environment, 
			const std::set<std::string>& enabledServices, 
			StoragePtr storage,
            ListenerPtr listener,
			const ApplicationInfo& applicationInfo);

	private:
		curl::curl_multi m_transport;
		std::unordered_map<curl::curl_easy*, RequestPtr> m_requests;
		ApplicationInfo m_applicationInfo;
		Futures m_futures;
		StoragePtr m_storage;
        ListenerPtr m_listener;

		std::unordered_map<std::string, ServiceCreator> m_serviceCreators;
		std::set<std::string> m_enabledServices;
		std::unordered_map<std::string, ServicePtr> m_services;

		std::function< void(std::string&,std::string&) > m_generateGuestUserCredentialsFunction;
		std::function< void( int code, const std::string& str ) > m_onSocketDisconnectedInfo;
	};
};

#endif
