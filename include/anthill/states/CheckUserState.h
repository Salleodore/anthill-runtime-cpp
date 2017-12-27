
#ifndef ONLINE_CheckUser_State_H
#define ONLINE_CheckUser_State_H

#include "State.h"
#include "AuthenticatedState.h"
#include "../services/LoginService.h"

#include <set>

namespace online
{
	class StateMachine;

	class CheckUserState : public AuthenticatedState
	{
		friend class StateMachine;

	public:
		CheckUserState(std::shared_ptr<StateMachine> stateMachine);
		virtual ~CheckUserState();

		virtual std::string ID() override { return "CheckUserState"; }
        
        // Allows to skip authentication step and complete the state machine instead
        static void SetDoAuthenticate(bool value) { s_doAuthenticate = value; }

	protected:
		virtual void init() override;
		virtual void release() override;

	private:
        static bool s_doAuthenticate;
	};
};

#endif
