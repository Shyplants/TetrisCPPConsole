#pragma once

#include <memory>
#include <stack>
#include "IState.h"
#include <asio.hpp>

class StateMachine
{
public:
	StateMachine() = default;
	~StateMachine();

	void PushState(std::unique_ptr<IState> newState);
	std::unique_ptr<IState> PopState();

	const bool Empty() const { return m_States.empty(); }
	std::unique_ptr<IState>& GetCurrentState();

	void RequestPop(int count) { m_PopRequested = count; }
	void RequestPopDepth(int depth) { m_PopRequestedDepth = depth; }

	void Update();

private:
	std::stack<std::unique_ptr<IState>> m_States{};
	int m_PopRequested{ 0 };
	int m_PopRequestedDepth{ 0 };
};