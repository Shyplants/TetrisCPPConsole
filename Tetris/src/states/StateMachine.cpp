#include "StateMachine.h"

StateMachine::~StateMachine()
{
}

void StateMachine::PushState(std::unique_ptr<IState> newState)
{
	m_States.push(std::move(newState));
	m_States.top()->OnEnter();
}

std::unique_ptr<IState> StateMachine::PopState()
{
	if (m_States.empty())
		return nullptr;

	auto oldState = std::move(m_States.top());
	m_States.pop();

	oldState->OnExit();
	return oldState;
}

std::unique_ptr<IState>& StateMachine::GetCurrentState()
{
	return m_States.top();
}

void StateMachine::Update()
{
	if (m_PopRequested > 0)
	{
		for (int i = 0; i < m_PopRequested; ++i)
		{
			PopState();
		}

		m_PopRequested = 0;
	}

	if (m_PopRequestedDepth > 0)
	{
		while (m_States.size() > m_PopRequestedDepth)
		{
			PopState();
		}

		m_PopRequestedDepth = 0;
	}
}