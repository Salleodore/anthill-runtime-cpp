
#ifndef ONLINE_ValidateExternalAuthentication_State_H
#define ONLINE_ValidateExternalAuthentication_State_H

#include "State.h"

namespace online
{
	class StateMachine;

	class ValidateExternalAuthenticationState: public State
	{
		friend class StateMachine;

	public:
		ValidateExternalAuthenticationState(std::shared_ptr<StateMachine> stateMachine);
		virtual ~ValidateExternalAuthenticationState();

		virtual std::string ID() override { return "ValidateExternalAuthenticationState"; }
		
	protected:
		virtual void init() override;
		virtual void release() override;

	private:
	};
};

#endif