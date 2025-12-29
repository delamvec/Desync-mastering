#ifndef _fsm_fsm_h
#define _fsm_fsm_h

// Local Includes
#include "state.h"

// FSM Class
class CFSM
{
	protected:
		CState *		m_pCurrentState;	// Current State
		CState *		m_pNewState;		// New State
		CStateTemplate<CFSM>	m_stateInitial;		// Initial State

		// PHASE 3: Concurrent State Support
		CState *		m_pConcurrentState;		// Concurrent state running in parallel
		CState *		m_pNewConcurrentState;	// New concurrent state
		bool			bStopConcurrent;		// Flag to stop concurrent state

	public:
		// Constructor
		CFSM();

		// Destructor
		virtual ~CFSM() {}

		// Global Functions
		virtual void Update();

		// State Functions
		bool IsState(CState &State) const;
		bool GotoState(CState &NewState);

		// PHASE 3: Concurrent State Functions
		void ConcurrentState(CState &State);
		void StopConcurrentState();

		virtual void BeginStateInitial() {}
		virtual void StateInitial() {}
		virtual void EndStateInitial() {}
};

#endif
