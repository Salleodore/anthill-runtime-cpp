
#include "anthill/states/CheckUserState.h"
#include "anthill/states/GenerateUserState.h"
#include "anthill/states/ValidateTokenState.h"
#include "anthill/states/AuthenticateState.h"
#include "anthill/states/StateMachine.h"
#include "anthill/Log.h"
#include "anthill/Utils.h"
#include "anthill/Storage.h"
#include "anthill/AnthillRuntime.h"

using namespace std::placeholders;

namespace online
{

    bool CheckUserState::s_doAuthenticate = true;

	CheckUserState::CheckUserState(std::shared_ptr<StateMachine> stateMachine) :
		AuthenticatedState(stateMachine)
	{
	}

	void CheckUserState::init()
	{
        if (!s_doAuthenticate)
        {
            complete();
            return;
        }
        
		LoginServicePtr ptr = AnthillRuntime::Instance().get<LoginService>();
		OnlineAssert((bool)ptr, "Login service is not initialized!");

		AnthillRuntime& online = AnthillRuntime::Instance();
		StoragePtr storage = online.getStorage();

		if (storage->has(Storage::StorageUsernameField) && storage->has(Storage::StoragePasswordField))
		{
			if (storage->has(Storage::StorageAccessTokeneField))
			{
				Log::get() << "Validating access token." << std::endl;

				std::string accessToken = storage->get(Storage::StorageAccessTokeneField);
                ptr->setCurrentAccessToken(accessToken);
				switchTo<ValidateTokenState>(accessToken);
			}
			else
			{
				authenticate();
			}
		}
		else
		{
			Log::get() << "No user found, generating new one." << std::endl;

			switchTo<GenerateUserState>();
		}
	}

	void CheckUserState::release()
	{
		//
	}

	CheckUserState::~CheckUserState()
	{
		//
	}
}
