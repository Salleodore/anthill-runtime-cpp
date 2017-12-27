
#include "anthill/states/CheckExternalAuthenticationState.h"
#include "anthill/states/ValidateExternalAuthenticationState.h"
#include "anthill/states/CheckUserState.h"
#include "anthill/states/StateMachine.h"
#include "anthill/Utils.h"
#include "anthill/AnthillRuntime.h"
#include "anthill/Log.h"

namespace online
{
	CheckExternalAuthenticationState::CheckExternalAuthenticationState(std::shared_ptr<StateMachine> stateMachine) :
		State(stateMachine)
	{
        
	}

	void CheckExternalAuthenticationState::init()
	{
        Log::get() << "Checking for external authentication... " << std::endl;
		proceed();
	}

	void CheckExternalAuthenticationState::proceed()
	{
        AnthillRuntime& online = AnthillRuntime::Instance();
        const ListenerPtr& listener = online.getListener();

		if (listener->shouldHaveExternalAuthenticator())
		{
			switchTo<ValidateExternalAuthenticationState>();
		}
		else
		{
			switchTo<CheckUserState>();
		}
	}

	void CheckExternalAuthenticationState::release()
	{
		//
	}

	CheckExternalAuthenticationState::~CheckExternalAuthenticationState()
	{
		//
	}
}