
#include "anthill/states/AttachExternalAuthenticationState.h"
#include "anthill/states/ValidateExternalAuthenticationState.h"
#include "anthill/states/GetEnvironmentState.h"
#include "anthill/states/StateMachine.h"
#include "anthill/Log.h"
#include "anthill/Utils.h"
#include "anthill/AnthillRuntime.h"

namespace online
{
	AttachExternalAuthenticationState::AttachExternalAuthenticationState(std::shared_ptr<StateMachine> stateMachine) :
		State(stateMachine)
	{
        
	}

	void AttachExternalAuthenticationState::init()
	{
        Log::get() << "Attaching external authentication... " << std::endl;
        
		LoginServicePtr ptr = AnthillRuntime::Instance().get<LoginService>();
		OnlineAssert((bool)ptr, "Login service is not initialized!");

		AnthillRuntime& online = AnthillRuntime::Instance();
		const ApplicationInfo& info = online.getApplicationInfo();
						
		lock();

		ptr->attachExternally(info.gamespace, info.requiredScopes, Request::Fields(),
			[this](const LoginService& service, Request::Result result, const Request& request, const std::string& accessToken,
				const std::string& credential,  const std::string& account, const LoginService::Scopes& scopes)
		{
			unlock();

			if (request.isSuccessful())
			{
				Log::get() << "External credential attached!" << std::endl;

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
				Log::get() << "External credential failed!" << std::endl;

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

	void AttachExternalAuthenticationState::release()
	{
		//
	}

	AttachExternalAuthenticationState::~AttachExternalAuthenticationState()
	{
		//
	}
}