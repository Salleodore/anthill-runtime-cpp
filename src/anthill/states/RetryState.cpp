
#include "anthill/states/RetryState.h"
#include "anthill/states/GetEnvironmentState.h"
#include "anthill/states/StateMachine.h"

#include "anthill/AnthillRuntime.h"
#include "anthill/Log.h"
#include "anthill/Utils.h"

namespace online
{
	const float RetryState::MaxRetryTime = 32.0f;

	RetryState::RetryState(std::shared_ptr<StateMachine> stateMachine, std::shared_ptr<State> tryState) :
		State(stateMachine),
		m_tryState(tryState)
	{
		OnlineAssert(tryState->ID() != RetryState::ID(), "Cannot put RetryState inside of RetryState.");


		std::weak_ptr< RetryState > weakPtr = std::static_pointer_cast< RetryState >( shared_from_this() );

		AnthillRuntime::Instance().getFutures().add(
			tryState->getRetryTime(), 
			[weakPtr]()
			{
				std::shared_ptr< RetryState > ptr = weakPtr.lock();
				if (!ptr)
					return;

				Log::get() << "Retrying state '" << ptr->m_tryState->ID() << std::endl;

				ptr->switchTo(ptr->m_tryState);
			} );
	}

	void RetryState::init()
	{
		//
	}

	void RetryState::release()
	{
		//
	}

	RetryState::~RetryState()
	{
		//
	}
}
