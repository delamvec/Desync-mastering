// Local Includes

#include <cassert>
#include <cstdlib>

#include "FSM.h"

// Constructor
CFSM::CFSM()
{
	// Initialize States
	m_stateInitial.Set(this, &CFSM::BeginStateInitial, &CFSM::StateInitial, &CFSM::EndStateInitial);

	// Initialize State Machine
	m_pCurrentState = static_cast<CState *>(&m_stateInitial);
	m_pNewState = NULL;

	// PHASE 3: Initialize Concurrent State members
	m_pConcurrentState = NULL;
	m_pNewConcurrentState = NULL;
	bStopConcurrent = false;
}

//======================================================================================================
// Global Functions

// Update
void CFSM::Update()
{
	// Check New State
	if (m_pNewState)
	{
		if (NULL != m_pCurrentState)
		{
			m_pCurrentState->ExecuteEndState();
		}

		// Set New State
		m_pCurrentState = m_pNewState;
		m_pNewState = 0;

		// Execute Begin State
		m_pCurrentState->ExecuteBeginState();
	}

	// Execute State
	m_pCurrentState->ExecuteState();

	// ============================================================
	// PHASE 3: CONCURRENT STATE EXECUTION
	// Runs in parallel with main state for position tracking
	// ============================================================

	// Check if concurrent state should be stopped
	if (bStopConcurrent)
	{
		if (m_pConcurrentState)
		{
			m_pConcurrentState->ExecuteEndState();
			m_pConcurrentState = NULL;
		}
		bStopConcurrent = false;
	}

	// Check for new concurrent state
	if (m_pNewConcurrentState)
	{
		// End previous concurrent state if exists
		if (m_pConcurrentState)
		{
			m_pConcurrentState->ExecuteEndState();
		}

		// Set new concurrent state
		m_pConcurrentState = m_pNewConcurrentState;
		m_pNewConcurrentState = NULL;

		// Execute Begin State
		m_pConcurrentState->ExecuteBeginState();
	}

	// Execute concurrent state if exists
	if (m_pConcurrentState)
	{
		m_pConcurrentState->ExecuteState();
	}
}

//======================================================================================================
// State Functions

// Is State
bool CFSM::IsState(CState & State) const
{
	return (m_pCurrentState == &State);
}

// Goto State
bool CFSM::GotoState(CState & NewState)
{
	if (IsState(NewState) && m_pNewState == &NewState)
		return true;

	// Set New State
	m_pNewState = &NewState;
	return true;
}

//======================================================================================================
// PHASE 3: Concurrent State Functions

// Concurrent State - Start a state that runs in parallel with main state
void CFSM::ConcurrentState(CState &State)
{
	// Set new concurrent state
	m_pNewConcurrentState = &State;
	bStopConcurrent = false;
}

// Stop Concurrent State - Stop currently running concurrent state
void CFSM::StopConcurrentState()
{
	// Set flag to stop concurrent state
	bStopConcurrent = true;
}
