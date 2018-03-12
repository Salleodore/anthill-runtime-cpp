
#include "anthill/states/ValidateExternalAuthenticationState.h"
#include "anthill/states/AttachExternalAuthenticationState.h"
#include "anthill/states/ExternalAuthenticationState.h"
#include "anthill/states/GenerateUserState.h"
#include "anthill/states/ValidateTokenState.h"
#include "anthill/states/AuthenticateState.h"
#include "anthill/states/StateMachine.h"
#include "anthill/Log.h"
#include "anthill/Utils.h"
#include "anthill/AnthillRuntime.h"

#include "anthill/Log.h"

namespace online
{
	ValidateExternalAuthenticationState::ValidateExternalAuthenticationState(std::shared_ptr<StateMachine> stateMachine) :
		State(stateMachine)
	{
        
	}

	void ValidateExternalAuthenticationState::init()
	{
        Log::get() << "Validating external authentication... " << std::endl;
        
		LoginServicePtr ptr = AnthillRuntime::Instance().get<LoginService>();
		OnlineAssert((bool)ptr, "Login service is not initialized!");

		AnthillRuntime& online = AnthillRuntime::Instance();
		StoragePtr storage = online.getStorage();
		
		const ListenerPtr& listener = AnthillRuntime::Instance().getListener();

		if (listener && !listener->shouldSaveExternalStorageAccessToken())
		{
			Log::get() << "Local access token is not used by external authenticator." << std::endl;
			switchTo<ExternalAuthenticationState>();
		}
		else
		{

			if (storage->has(Storage::StorageAccessTokeneField))
			{
				Log::get() << "Validating (probably external) access token." << std::endl;

				std::string accessToken = storage->get(Storage::StorageAccessTokeneField);
			
				lock();

				ptr->validateAccessToken(accessToken, 
					[this, accessToken](const LoginService& service, Request::Result result, const Request& request,
						const std::string& credential, const std::string& account, const LoginService::Scopes& scopes)
				{
					unlock();
        
					LoginServicePtr ptr = AnthillRuntime::Instance().get<LoginService>();
					ptr->setCurrentAccessToken(accessToken);

					const online::ExternalAuthenticatorPtr& authenticator = ptr->getExternalAuthenticator();

					if (request.isSuccessful())
					{
						online::Credential parsed(credential);

						if (parsed.getCredentialType() != authenticator->getCredentialType())
						{
							Log::get() << "Credential types don't match! ";
							Log::get() << "Parsed credential type=" << parsed.getCredentialType();
							Log::get() << "External credential type=" << authenticator->getCredentialType() << std::endl;
							switchTo<AttachExternalAuthenticationState>();
							return;
						}

						const ApplicationInfo& applicationInfo = AnthillRuntime::Instance().getApplicationInfo();
                
						// check if the requiredScopes are satisfied
						if (std::includes(scopes.begin(), scopes.end(), applicationInfo.shouldHaveScopes.begin(), applicationInfo.shouldHaveScopes.end()))
						{
							Log::get() << "Access token is valid!" << std::endl;

							const ListenerPtr& listener = AnthillRuntime::Instance().getListener();

							if (listener)
							{
								listener->authenticated(account, credential, scopes);
							}

							complete();
							return;
						}
						else
						{
							Log::get() << "Required scopes are not satisfied!" << std::endl;
						}
					}
   
					Log::get() << "Access token is invalid!" << std::endl;
            
					AnthillRuntime& online = AnthillRuntime::Instance();
					StoragePtr storage = online.getStorage();

					storage->remove(Storage::StorageAccessTokeneField);
					storage->save();

					switchTo<ExternalAuthenticationState>();
				});


			}
			else
			{
				Log::get() << "Access token is not found." << std::endl;

				switchTo<ExternalAuthenticationState>();
			}
		}
	}

	void ValidateExternalAuthenticationState::release()
	{
		//
	}

	ValidateExternalAuthenticationState::~ValidateExternalAuthenticationState()
	{
		//
	}
}