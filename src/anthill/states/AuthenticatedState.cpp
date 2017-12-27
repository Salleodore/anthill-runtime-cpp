
#include "anthill/states/AuthenticatedState.h"
#include "anthill/states/AuthenticateState.h"
#include "anthill/states/CheckUserState.h"
#include "anthill/states/StateMachine.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Log.h"

namespace online
{
	AuthenticatedState::AuthenticatedState(std::shared_ptr<StateMachine> stateMachine) :
		State(stateMachine)
	{
	}

	void AuthenticatedState::authenticate()
	{
        Log::get() << "Authenticating" << std::endl;

		AnthillRuntime& online = AnthillRuntime::Instance();
		StoragePtr storage = online.getStorage();

		std::string username = storage->get(Storage::StorageUsernameField);
		std::string password = storage->get(Storage::StoragePasswordField);

		const ApplicationInfo& info = online.getApplicationInfo();

		switchTo<AuthenticateState>(username, password, info.gamespace, info.requiredScopes, shared_from_this(), info.shouldHaveScopes);
	}

	AuthenticatedState::~AuthenticatedState()
	{
		//
	}
}
