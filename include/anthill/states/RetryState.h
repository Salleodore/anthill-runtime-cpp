
#ifndef ONLINE_Retry_State_H
#define ONLINE_Retry_State_H

#include "State.h"

namespace online
{
	class StateMachine;


	class RetryState: public State
	{
		friend class StateMachine;

	public:
		static const float MaxRetryTime;

	public:
		RetryState(std::shared_ptr<StateMachine> stateMachine, std::shared_ptr<State> tryState);
		virtual ~RetryState();

		virtual std::string ID() override { return "RetryState"; }

	protected:
		virtual void init() override;
		virtual void release() override;
		
	private:
		std::shared_ptr<State> m_tryState;
	}; 
};

#endif