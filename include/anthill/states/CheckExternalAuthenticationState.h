
#ifndef ONLINE_CheckExternalAuthentication_State_H
#define ONLINE_CheckExternalAuthentication_State_H

#include "State.h"

namespace online
{
	class StateMachine;

	class CheckExternalAuthenticationState: public State
	{
		friend class StateMachine;

	public:
		CheckExternalAuthenticationState(std::shared_ptr<StateMachine> stateMachine);
		virtual ~CheckExternalAuthenticationState();

		virtual std::string ID() override { return "CheckExternalAuthenticationState"; }

	private:
		void proceed();

	protected:
		virtual void init() override;
		virtual void release() override;

	private:
	};
};

#endif