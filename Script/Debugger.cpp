#include "stdafx.h"
#include <assert.h>
#include <vector>

#include "Basic.h"
#ifdef SCRIPT_DEBUGGER
#include "Debugger.h"
#include "Compiler.h"

using namespace std;
using namespace script;

Debugger::Debugger(/*DebuggerEvents *pDebuggerNotify*/) :
    m_states{
    new DebuggerStop(this),
    new DebuggerRun(this),
    new DebuggerPause(this),
    new DebuggerStepIn(this),
    new DebuggerStep(this),
    new DebuggerStepOut(this) },
    m_pCurrentState(0),
    m_pInterpreterIf(0) {
    m_pCurrentState = m_states[DEBUGGER_STOP];
    InitializeCriticalSection(&m_cs);
}

Debugger::~Debugger() {
    DeleteCriticalSection(&m_cs);
    for (ui32 i = 0; i < DEBUGGER_MAX; ++i) {
        if (m_states[i]) {
            delete m_states[i];
        }
    }
}

void Debugger::SetAction(DebuggerAction *pAction) {
    assert(m_pInterpreterIf);
    Iterate<DebuggerState::SET_ACTION_FOO, DebuggerAction*>(&DebuggerState::SetAction, pAction);
}

SymbolStore* Debugger::GetSymbolStore() { 
    return m_pInterpreterIf ? &m_pInterpreterIf->GetSymbolStore() : 0; 
};

Memory* Debugger::GetMemory() {
    return m_pInterpreterIf ? &m_pInterpreterIf->GetMemory() : 0;
}

DebuggerState* Debugger::GetState(DEBUGGER_STATES state) {
    return m_states[state];
}

void Debugger::Stop() {
    //assert(m_pInterpreterIf);
    if (TryEnterCriticalSection(&m_cs)) {
        m_pCurrentState = m_pCurrentState->Stop();
        LeaveCriticalSection(&m_cs);
    }
}

void Debugger::Run() {
    assert(m_pInterpreterIf);
    if (TryEnterCriticalSection(&m_cs)) {
        m_pCurrentState = m_pCurrentState->Run();
        LeaveCriticalSection(&m_cs);
    }
}

void Debugger::Pause(PAUSE_TYPE type) {
    assert(m_pInterpreterIf);
    if (TryEnterCriticalSection(&m_cs)) {
        m_pCurrentState = m_pCurrentState->Pause(type);
        LeaveCriticalSection(&m_cs);
    }
}

void Debugger::StepIn() {
    assert(m_pInterpreterIf);
    if (TryEnterCriticalSection(&m_cs)) {
        m_pCurrentState = m_pCurrentState->StepIn();
        LeaveCriticalSection(&m_cs);
    }
}

void Debugger::StepOver() {
    assert(m_pInterpreterIf);
    if (TryEnterCriticalSection(&m_cs)) {
        m_pCurrentState = m_pCurrentState->StepOver();
        LeaveCriticalSection(&m_cs);
    }
}

void Debugger::StepOut() {
    assert(m_pInterpreterIf);
    if (TryEnterCriticalSection(&m_cs)) {
        m_pCurrentState = m_pCurrentState->StepOut();
        LeaveCriticalSection(&m_cs);
    }
}

void Debugger::PropagateBreakpoints(BREAKPOINTS &breakpoints) {
    BREAKPOINTS::iterator itFileNames;
    //Debugger &debugger = Debugger::GetInstance();
    if (SymbolStore *pSymStore = GetSymbolStore()) {
        for (itFileNames = breakpoints.begin(); itFileNames != breakpoints.end(); ++itFileNames) {
            unordered_set<BREAKPOINT_INFO, BREAKPOINT_INFO>::iterator itLines;
            ui32 fileId = pSymStore->GetFileId(itFileNames->first);
            for (itLines = itFileNames->second.begin(); itLines != itFileNames->second.end(); ++itLines) {
                SetBreakpoint(fileId, itLines->line, true);
            }
        }
    }
}

bool Debugger::SetBreakpoint(ui32 fileId, ui32 line, bool set) {
    assert(m_pInterpreterIf);
    return m_pCurrentState->SetBreakpoint(fileId, line, set);
}

/***************************************/

DebuggerState::DebuggerState(Debugger *pDebugger) : 
    m_pDebugger(pDebugger),
    m_pAction(0) {

}

DebuggerState::~DebuggerState() {
    
}

DebuggerState* DebuggerState::Run() {
    if (m_pAction) {
        m_pAction->Run();
    }
    return m_pDebugger->GetState(DEBUGGER_RUN);
}

DebuggerState* DebuggerState::Stop() {
    if (m_pAction) {
        m_pAction->Stop();
    }
    return m_pDebugger->GetState(DEBUGGER_STOP);
}

DebuggerState* DebuggerState::Pause(PAUSE_TYPE type) {
    if (m_pAction) {
        m_pAction->Pause(type);
    }
    return m_pDebugger->GetState(DEBUGGER_PAUSE);
}

DebuggerState* DebuggerState::StepIn() {
    if (m_pAction) {
        m_pAction->StepIn();
    }
    return m_pDebugger->GetState(DEBUGGER_STEP_IN);
}

DebuggerState* DebuggerState::StepOver() {
    if (m_pAction) {
        m_pAction->StepOver();
    }
    return m_pDebugger->GetState(DEBUGGER_STEP);
}

DebuggerState* DebuggerState::StepOut() {
    if (m_pAction) {
        m_pAction->StepOut();
    }
    return m_pDebugger->GetState(DEBUGGER_STEP_OUT);
}

bool DebuggerState::SetBreakpoint(ui32 fileId, ui32 line, bool set) {
    if (m_pAction) {
        return m_pAction->SetBreakpoint(fileId, line, set);
    }
    return false;
}

/***************************************/

DebuggerStop::DebuggerStop(Debugger *pDebugger) : 
    DebuggerState(pDebugger) {
}

DebuggerState* DebuggerStop::Stop() {
    return this;
}

DebuggerState* DebuggerStop::Pause(PAUSE_TYPE type) {
    return this;
}

/***************************************/
DebuggerRun::DebuggerRun(Debugger *pDebugger) : 
    DebuggerState(pDebugger) {
}

DebuggerState* DebuggerRun::Run() {
    return this;
}

DebuggerState* DebuggerRun::StepIn() {
    return this;
}

DebuggerState* DebuggerRun::StepOver() {
    return this;
}

DebuggerState* DebuggerRun::StepOut() {
    return this;
}


/***************************************/

DebuggerPause::DebuggerPause(Debugger *pDebugger) :
    DebuggerStop(pDebugger) {
}

DebuggerState* DebuggerPause::Stop() {
    return DebuggerState::Stop();
}

DebuggerState* DebuggerPause::Pause(PAUSE_TYPE type) {
    return this;
}

/***************************************/

DebuggerStepIn::DebuggerStepIn(Debugger *pDebugger) : 
    DebuggerState(pDebugger) {
}
//
DebuggerState* DebuggerStepIn::Run() {
    return this;
}

DebuggerState* DebuggerStepIn::StepIn() {
    return this;
}

DebuggerState* DebuggerStepIn::StepOver() {
    return this;
}

DebuggerState* DebuggerStepIn::SteOut() {
    return this;
}

/***************************************/

DebuggerStep::DebuggerStep(Debugger *pDebugger) : 
    DebuggerStepIn(pDebugger) {
}

/***************************************/

DebuggerStepOut::DebuggerStepOut(Debugger *pDebugger) : 
    DebuggerStepIn(pDebugger) {
}

DebuggerState* DebuggerStepOut::Run() {
    return DebuggerState::Run();
}
/***************************************/

#endif // SCRIPT_DEBUGGER