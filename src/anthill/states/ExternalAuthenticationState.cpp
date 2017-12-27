
#include "anthill/states/ExternalAuthenticationState.h"
#include "anthill/states/ValidateExternalAuthenticationState.h"
#include "anthill/states/GetEnvironmentState.h"
#include "anthill/states/StateMachine.h"

#include "anthill/Log.h"
#include "anthill/Utils.h"
#include "anthill/AnthillRuntime.h"

namespace online
{
	ExternalAuthenticationState::ExternalAuthenticationState(std::shared_ptr<StateMachine> stateMachine) :
		State(stateMachine)
	{
        
	}

	void ExternalAuthenticationState::init()
	{
        Log::get() << "Eexternal authentication... " << std::endl;
        
		LoginServicePtr ptr = AnthillRuntime::Instance().get<LoginService>();
		OnlineAssert((bool)ptr, "Login service is not initialized!");

		AnthillRuntime& online = AnthillRuntime::Instance();
		const ApplicationInfo& info = online.getApplicationInfo();
						
		lock();

		ptr->authenticateExternally(info.gamespace, info.requiredScopes, {},
			[this](const LoginService& service, Request::Result result, const Request& request, const std::string& accessToken,
				const std::string& credential,  const std::string& account, const LoginService::Scopes& scopes)
		{
			unlock();

			if (request.isSuccessful())
			{
				LoginServicePtr ptr = AnthillRuntime::Instance().get<LoginService>();
				ptr->setCurrentAccessToken(accessToken);

				const ListenerPtr& listener = AnthillRuntime::Instance().getListener();

				if (listener)
				{
					listener->authenticated(account, credential, scopes);
				}
								
				AnthillRuntime& online = AnthillRuntime::Instance();
				StoragePtr storage = online.getStorage();

				storage->set(Storage::StorageAccessTokeneField, accessToken);
				storage->save();

				complete();
			}
			else
			{
				switchTo<ValidateExternalAuthenticationState>();
			}
		},
			[this](const LoginService& service, const LoginService::MergeOptions options, LoginService::MergeResolveCallback resolve)
		{      
			unlock();

			AnthillRuntime& online = AnthillRuntime::Instance();
			const ListenerPtr& listener = online.getListener();
			if (listener)
			{
				listener->multipleAccountsAttached(service, options, resolve);
			}
		},
		info.shouldHaveScopes);
	}

	void ExternalAuthenticationState::release()
	{
		//
	}

	ExternalAuthenticationState::~ExternalAuthenticationState()
	{
		//
	}
}