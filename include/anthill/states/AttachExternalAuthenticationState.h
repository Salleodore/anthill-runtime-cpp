
#ifndef ONLINE_AttachExternalAuthentication_State_H
#define ONLINE_AttachExternalAuthentication_State_H

#include "State.h"

namespace online
{
	class StateMachine;

	class AttachExternalAuthenticationState: public State
	{
		friend class StateMachine;

	public:
		AttachExternalAuthenticationState(std::shared_ptr<StateMachine> stateMachine);
		virtual ~AttachExternalAuthenticationState();

		virtual std::string ID() override { return "AttachExternalAuthenticationState"; }

	protected:
		virtual void init() override;
		virtual void release() override;

	private:
	};
};

#endif