
#include "anthill/states/AuthenticateState.h"
#include "anthill/states/GenerateUserState.h"
#include "anthill/states/CheckUserState.h"
#include "anthill/states/StateMachine.h"
#include "anthill/Log.h"
#include "anthill/Utils.h"
#include "anthill/AnthillRuntime.h"

using namespace std::placeholders;

namespace online
{
	AuthenticateState::AuthenticateState(std::shared_ptr<StateMachine> stateMachine, 
		const std::string& username, 
		const std::string& password,
		const std::string& gamespace,
		const LoginService::Scopes& scopes, 
		std::shared_ptr<State> returnState,
        const LoginService::Scopes& shouldHaveScopes
	) :
		State(stateMachine),
		m_username(username),
		m_password(password),
		m_gamespace(gamespace),
		m_scopes(scopes),
        m_shouldHaveScopes(shouldHaveScopes),
		m_returnState(returnState)
	{
	}
    
    void AuthenticateState::merge(const LoginService& service, const LoginService::MergeOptions options, LoginService::MergeResolveCallback resolve)
    {
        unlock();
        
        AnthillRuntime& online = AnthillRuntime::Instance();
        const ListenerPtr& listener = online.getListener();
		if (listener)
		{
			listener->multipleAccountsAttached(service, options, resolve);
		}
    }

	void AuthenticateState::authenticateResult(const LoginService& service, Request::Result result, const Request& request, const std::string& accessToken,
		const std::string& credential,  const std::string& account, const LoginService::Scopes& scopes)
	{
        unlock();
        
		if (Request::isSuccessful(result))
		{
			Log::get() << "Authenticate successful!" << std::endl;

			AnthillRuntime& online = AnthillRuntime::Instance();
			const StoragePtr& storage = online.getStorage();

			LoginServicePtr ptr = online.get<LoginService>();
			OnlineAssert((bool)ptr, "Login service is not initialized!");

			ptr->setCurrentAccessToken(accessToken);
			

			const ListenerPtr& listener = online.getListener();

			if (listener)
			{
				listener->authenticated(account, credential, scopes);
			}

			storage->set(Storage::StorageAccessTokeneField, accessToken);
			storage->save();

			proceed();
		}
		else
		{
			Log::get() << "Authenticate failed: " << result << ": " << request.getResponseAsString() << std::endl;

			switch (result)
			{
				case Request::FORBIDDEN:
				{
					Log::get() << " ERROR: Forbidden to authenticate!" << std::endl;

					// basically we have no options here except to generate a new fresh user

					switchTo<GenerateUserState>();
					return;
				}
                default:
                {
                    break;
                }
			}

			retry();
		}
	}

	void AuthenticateState::proceed()
    {
		if (m_returnState)
		{
			switchTo(m_returnState);
		}
		else
		{
			complete();
		}
	}
	
	void AuthenticateState::init()
	{
		LoginServicePtr ptr = AnthillRuntime::Instance().get<LoginService>();
		OnlineAssert((bool)ptr, "Login service is not initialized!");

		Log::get() << "Authentication: " << m_username << std::endl;
        
        lock();

		ptr->authenticateAnonymous(m_username, m_password, m_gamespace, m_scopes, Request::Fields(),
			std::bind(&AuthenticateState::authenticateResult, this, _1, _2, _3, _4, _5, _6, _7),
            std::bind(&AuthenticateState::merge, this, _1, _2, _3), m_shouldHaveScopes);
	}

	void AuthenticateState::release()
	{
		//
	}

	AuthenticateState::~AuthenticateState()
	{
		//
	}
}
