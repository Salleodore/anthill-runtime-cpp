
#ifndef ONLINE_Listener_H
#define ONLINE_Listener_H

#include "services/LoginService.h"
#include "services/EnvironmentService.h"

namespace online
{
	typedef std::shared_ptr< class Listener > ListenerPtr;

	class Listener
	{
	public:
        virtual void multipleAccountsAttached(
            const LoginService& service,
            const LoginService::MergeOptions options,
            LoginService::MergeResolveCallback resolve) = 0;

		virtual void servicesDiscovered(std::function<void()> proceed)
		{
			proceed();
		}

		virtual void environmentVariablesReceived(const EnvironmentInformation& variables)
		{
			//
		}

		virtual void authenticated(const std::string& account, const std::string& credential, const online::LoginService::Scopes& scopes)
		{
			//
		}

		virtual bool shouldHaveExternalAuthenticator()
		{
			return false;
		}

		virtual ExternalAuthenticatorPtr createExternalAuthenticator()
		{
			return nullptr;
		}
	};
};

#endif
