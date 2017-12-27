
#ifndef ONLINE_ExternalAuthentication_State_H
#define ONLINE_ExternalAuthentication_State_H

#include "State.h"

namespace online
{
	class StateMachine;

	class ExternalAuthenticationState: public State
	{
		friend class StateMachine;

	public:
		ExternalAuthenticationState(std::shared_ptr<StateMachine> stateMachine);
		virtual ~ExternalAuthenticationState();

		virtual std::string ID() override { return "ExternalAuthenticationState"; }
		
	protected:
		virtual void init() override;
		virtual void release() override;

	private:
	};
};

#endif