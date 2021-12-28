#include "stdafx.h"

//#include <string>
#include <assert.h>
#include <Windows.h>
#include <comutil.h>

#include "Expressions.h"
#include "Compiler.h"
#include <sstream>

#define _ERROR(e) error.SetErrorEx(e, SCRIPT_SYM_TO_STR(e), __FILE__, __FUNCTION__, __LINE__, m_symDesc)
#define _ERROR_EX(e, sym) error.SetErrorEx(e, SCRIPT_SYM_TO_STR(e), __FILE__, __FUNCTION__, __LINE__, sym)

#ifdef MEMORY_ACCESS_EXCEPTION
#define _PROTECT_BEGIN    try {
#define _PROTECT_END(err) } catch (SE_Exception &e) {\
                              param.m_pStackMemory->GetRunTimeError().SetError("MEMORY_ACCESS_EXCEPTION!", m_symDesc, e.getSeNumber(), param.m_pStackMemory);\
                              ret = err;\
                          }
#else // MEMORY_ACCESS_EXCEPTION
#define _PROTECT_BEGIN
#define _PROTECT_END
#endif // MEMORY_ACCESS_EXCEPTION

RUNTIME_RETURN AssertError() {
    assert(0);
    return RT_ERROR;
}

using namespace std;
using namespace script;

/*************************************************************/
#ifdef SCRIPT_DEBUGGER

FlowSemaphor::FlowSemaphor(DebuggerHandler *pDebuggerHandler, FLOW_TYPE flowType) :
    DebuggerAction(),
    m_Script2DllSwitchCounter(0),
    m_stepStackIndex(INVALID_STACK_FRAME_IDX),
    m_flowType(flowType),
    m_threadId(GetCurrentThreadId()),
    m_flowTypeEvent(CreateEvent(0, TRUE, FALSE, 0)),
    m_pDebugger(pDebuggerHandler->GetDebugger()),
    m_pDebuggerHandler(pDebuggerHandler) {
    SetDebuggerNotify(pDebuggerHandler->GetDebuggerNotify());
    InitializeCriticalSection(&m_cs);
}

FlowSemaphor::~FlowSemaphor() {
    if (m_flowTypeEvent) { // revisit it!
        LOCK _(m_cs);
        SetEvent(m_flowTypeEvent);
        CloseHandle(m_flowTypeEvent);
        m_flowTypeEvent = 0;
    }
    DeleteCriticalSection(&m_cs);
}

void FlowSemaphor::Stop() {
    if (m_flowTypeEvent) {
        LOCK _(m_cs);
        if (m_flowType != FLOW_STOP) {
            m_flowType = FLOW_STOP;
            if (m_pNotify) {
                m_pNotify->OnStop(m_threadId);
            }
            SetEvent(m_flowTypeEvent);
            CloseHandle(m_flowTypeEvent);
            m_flowTypeEvent = 0;
        }
    }
}

void FlowSemaphor::Run() {
    if (m_flowTypeEvent) {
        LOCK _(m_cs);
        switch (m_flowType) {
            case FLOW_STOP:
                m_flowType = FLOW_RUN;
                if (m_pNotify) {
                    m_pNotify->OnRun(m_threadId);
                }
                break;
            case FLOW_PAUSE:
            case FLOW_STEP_OUT:
                m_flowType = FLOW_RUN;
                if (m_pNotify) {
                    m_pNotify->OnRun(m_threadId);
                }
                SetEvent(m_flowTypeEvent);
                break;
            case FLOW_RUN:
            case FLOW_STEP_IN:
            case FLOW_STEP_OVER:
            default: // FLOW_ERROR
                //assert(0);
                break;
        }
    }
}

void FlowSemaphor::Pause(PAUSE_TYPE type) {
    if (m_flowTypeEvent) {
        LOCK _(m_cs);
        switch (m_flowType) {
            case FLOW_RUN:
            case FLOW_STEP_IN:
            case FLOW_STEP_OVER:
            case FLOW_STEP_OUT:
                m_flowType = FLOW_PAUSE;
                if (m_pNotify) {
                    m_pNotify->OnPause(m_threadId);
                }
                ResetEvent(m_flowTypeEvent);
                break;
            case FLOW_PAUSE:
                if (m_pNotify) {
                    m_pNotify->OnPause(m_threadId);
                }
                break;
            case FLOW_STOP:
            default: // FLOW_ERROR
                break;
        }
    }
}

void FlowSemaphor::StepIn() {
    if (m_flowTypeEvent) {
        LOCK _(m_cs);
        switch (m_flowType) {
            case FLOW_STOP:
                m_flowType = FLOW_STEP_IN;
                if (m_pNotify) {
                    m_pNotify->OnStepIn(m_threadId);
                }
                break;
            case FLOW_PAUSE:
                m_flowType = FLOW_STEP_IN;
                if (m_pNotify) {
                    m_pNotify->OnStepIn(m_threadId);
                }
                SetEvent(m_flowTypeEvent);
                break;
            case FLOW_STEP_IN:
            case FLOW_STEP_OVER:
            case FLOW_STEP_OUT:
            case FLOW_RUN:
            default: // FLOW_ERROR
                break;
        }
    }
}

void FlowSemaphor::StepOver() {
    if (m_flowTypeEvent) {
        LOCK _(m_cs);
        switch (m_flowType) {
            case FLOW_STOP:
                m_flowType = FLOW_STEP_OVER;
                if (m_pNotify) {
                    m_pNotify->OnStepOver(m_threadId);
                }
                break;
            case FLOW_PAUSE:
                m_flowType = FLOW_STEP_OVER;
                if (m_pNotify) {
                    m_pNotify->OnStepOver(m_threadId);
                }
                SetEvent(m_flowTypeEvent);
                break;
            case FLOW_STEP_IN:
            case FLOW_STEP_OVER:
            case FLOW_STEP_OUT:
            case FLOW_RUN:
            default: // FLOW_ERROR
                break;
        }
    }
}

void FlowSemaphor::StepOut() {
    if (m_flowTypeEvent) {
        LOCK _(m_cs);
        switch (m_flowType) {
            case FLOW_PAUSE:
                m_flowType = FLOW_STEP_OUT;
                if (m_pNotify) {
                    m_pNotify->OnStepOut(m_threadId);
                }
                SetEvent(m_flowTypeEvent);
                break;
            case FLOW_STOP:
            case FLOW_STEP_IN:
            case FLOW_STEP_OVER:
            case FLOW_STEP_OUT:
            case FLOW_RUN:
            default: // FLOW_ERROR
                break;
        }
    }
}

bool FlowSemaphor::SetBreakpoint(ui32 fileId, ui32 line, bool set) {
    if (m_breakPointLines.size() <= fileId) {
        return false;
    }
    BREAK_POINT_LINES &breakPointLines = m_breakPointLines[fileId];
    if (breakPointLines.size() < line) { // line 
        return false;
    }
    Node *pNode = breakPointLines[line];
    if (!pNode) {
        return false;
    }
    if (Scope *pScope = pNode->GetParent<Scope>(TYPE_SCOPE)) {
        pScope->SetBreakpoint(fileId, pNode->GetLine(), set);
        if (m_pNotify) {
            m_pNotify->OnSetBreakPoint(fileId, line, set);
        }
        return true;
    }
    return false;
}

void FlowSemaphor::RegisterBreakPointLine(Node* pNode) {
    assert(pNode);

    ui32 fileId = pNode->GetFileId(),
         line = pNode->GetLine();

    assert(line != 0);

    if (m_breakPointLines.size() <= fileId) {
        m_breakPointLines.resize(fileId + 1);
    }
    BREAK_POINT_LINES &breakPointLines = m_breakPointLines[fileId];
    if (breakPointLines.size() < line) {
        breakPointLines.resize(line/* line is not zero based: START_POSITION_OFFSET*/);
    }
    if (!breakPointLines[line - 1]) {
        breakPointLines[line - 1] = pNode;
    } // else multiple expressions on the same line, just skip it.
}

void FlowSemaphor::ResetStepOver(Node *pNode, ui32 stackIndex) {
    assert(FLOW_STEP_OVER == m_flowType);
    if ((m_stepStackIndex >= stackIndex) && (INVALID_STACK_FRAME_IDX != m_stepStackIndex)){
        if (m_pNotify) {
            m_pNotify->OnBreakHit(pNode->GetFileId(), pNode->GetLine() - 1, m_threadId, 0);
        }
        m_stepStackIndex = INVALID_STACK_FRAME_IDX;
        m_pDebugger->Pause(PauseType(m_threadId));
    }
}

void FlowSemaphor::ResetStepOut(Node *pNode, ui32 stackIndex) {
    assert(FLOW_STEP_OUT == m_flowType);
    if (m_stepStackIndex > stackIndex) {
        if (m_pNotify) {
            m_pNotify->OnBreakHit(pNode->GetFileId(), pNode->GetLine() - 1, m_threadId, 0);
        }
        m_stepStackIndex = INVALID_STACK_FRAME_IDX;
        m_pDebugger->Pause(PauseType(m_threadId));
    }
}

void FlowSemaphor::ResetOnEndOfThread() {
    Memory *pMemory = m_pDebugger->GetMemory();
    StackMemory *pStackMemory = pMemory->GetThreadMemory(m_threadId);
    Node *pFunction = 0, *pCurNode = 0;
    Scope *pScope = 0;
    ui32 frameIdx = pStackMemory->GetFrameIdx();
    if (pStackMemory->GetFrameFunctionAndScope(frameIdx, pFunction, pScope, pCurNode)) {
        if (!pScope && frameIdx) {
            pStackMemory->GetFrameFunctionAndScope(frameIdx - 1, pFunction, pScope, pCurNode);
        }
        m_pNotify->OnBreakHit(pCurNode->GetFileId(), pCurNode->GetLine(), m_threadId, pScope);
    }
}

void FlowSemaphor::SetCurrentStackIndex(ui32 stackIndex) {
    if (INVALID_STACK_FRAME_IDX == m_stepStackIndex) {
        m_stepStackIndex = stackIndex;
    }
}

void FlowSemaphor::ResetOnEndThread() {
    // delegate step_xx actions
    switch (m_flowType) {
        case FLOW_STEP_OVER:
            //m_pDebugger->Pause(PauseType(PAUSE_TYPE::PAUSE_ON_END_OF_THREAD));
            //m_pDebugger->StepOver();
            //break;
        case FLOW_STEP_OUT:
            //m_pDebugger->Pause(PauseType(PAUSE_TYPE::PAUSE_ON_END_OF_THREAD));
            //m_pDebugger->StepOut();
            //break;
        case FLOW_STEP_IN:
            m_pDebugger->Pause(PauseType(PAUSE_TYPE::PAUSE_ON_END_OF_THREAD));
            //m_pDebugger->StepIn();
            break;
        case FLOW_RUN:
        case FLOW_STOP:
        case FLOW_PAUSE:
        default:
            break;
    }
}

RUNTIME_RETURN FlowSemaphor::CheckFlow(Node *pNode, ui32 stackIndex, const unordered_set<ui64> &breakLines) {
    Scope::BREAKPOINT_LOCATION bl = { pNode->GetFileId(), pNode->GetLine() };

    bool breakPoint = breakLines.find(bl.breakpoint) != breakLines.end();

    if (!breakPoint) {
        switch (m_flowType) {
            case FLOW_RUN:
                return RT_NO_ERROR;
            case FLOW_PAUSE:
                if (m_pNotify) {
                    m_pNotify->OnBreakHit(bl.fileId, bl.line - 1, m_threadId, 0);
                }
                break;
            case FLOW_STEP_OVER:
                ResetStepOver(pNode, stackIndex);
                break;
            case FLOW_STEP_OUT:
                ResetStepOut(pNode, stackIndex);
                break;
            case FLOW_STEP_IN:
                break;
            case FLOW_STOP:
            //case FLOW_ERROR:
            default:
                return RT_STOP;
        }
    }
    else {
        switch (m_flowType) {
            case FLOW_STEP_OUT:
            case FLOW_STEP_OVER:
                m_stepStackIndex = INVALID_STACK_FRAME_IDX;
            case FLOW_STEP_IN:
            case FLOW_RUN:
                if (m_pNotify) {
                    m_pNotify->OnBreakHit(bl.fileId, bl.line - 1, m_threadId, 0);
                }
                m_pDebugger->Pause(PauseType(m_threadId));
            case FLOW_PAUSE:
                if (m_pNotify) {
                    m_pNotify->OnBreakHit(bl.fileId, bl.line - 1, m_threadId, 0);
                }
                break;
            case FLOW_STOP:
            //case FLOW_ERROR:
            default:
                return RT_STOP;
        }
    }

    if (WaitForSingleObject(m_flowTypeEvent, INFINITE) == WAIT_OBJECT_0) { // event
        switch (m_flowType) {
            case FLOW_RUN:
                break;
            case FLOW_STEP_IN:
                m_pDebugger->Pause(PauseType(m_threadId));
                break;
            case FLOW_STEP_OVER:
                SetCurrentStackIndex(stackIndex);
                break;
            case FLOW_STEP_OUT:
                if (!stackIndex) {
                    m_stepStackIndex = INVALID_STACK_FRAME_IDX;
                    m_pDebugger->Run();
                }
                else {
                    SetCurrentStackIndex(stackIndex);
                }
                break;
            case FLOW_PAUSE:
                assert(0);
            case FLOW_STOP:
            //case FLOW_ERROR:
            default:
                return RT_STOP;
        }
    }
    else { // error
        return RT_STOP;
    }

    return RT_NO_ERROR;
}

/*************************************************************/

DebuggerHandler::DebuggerHandler() : 
    DebuggerAction(),
    m_pDebugger(0),
    m_semaphorIdx(0)
{
    InitializeCriticalSection(&m_cs);
    m_hReadyEvent = CreateEvent(0, TRUE, FALSE, 0);
}

DebuggerHandler::~DebuggerHandler() {
    assert(m_flowSemaphors.size() == 0);
    DeleteCriticalSection(&m_cs);
}

void DebuggerHandler::Stop() {
    Handler(&DebuggerAction::Stop);
}

void DebuggerHandler::Run() {
    Handler(&DebuggerAction::Run);
}

void DebuggerHandler::Pause(PAUSE_TYPE type) {
    // Revist it!
    if (PAUSE_TYPE::PAUSE_ON_END_OF_THREAD == type.type) {
        LOCK _(m_cs);
        // all thread are paused, so just notify next thread with break hit
        ui32 idx = m_semaphorIdx  % m_flowSemaphors.size();
        assert(idx < m_flowSemaphors.size());
        ui32 i = idx;
        do {
            FlowSemaphor *pFlowSemaphor = m_flowSemaphors[i];
            if ((pFlowSemaphor->m_Script2DllSwitchCounter & 1) == 0) {
                pFlowSemaphor->ResetOnEndOfThread();
                return;
            }
            else {
                i = ++m_semaphorIdx % m_flowSemaphors.size();
            }
        } while (idx != i);
    }
    else {
        LOCK _(m_cs);
        FLOW_SEMAPHOR_MAP::iterator it = m_flowSemaphors.begin();
        for (; it != m_flowSemaphors.end(); ++it) {
            (*it)->Pause(type);
        }
    }
}

void DebuggerHandler::Handler(ACTION_HANDLER fn) {
    LOCK _(m_cs);
    FLOW_SEMAPHOR_MAP::iterator it = m_flowSemaphors.begin();
    for (; it != m_flowSemaphors.end(); ++it) {
        ((*it)->*fn)();
    }
}

void DebuggerHandler::StepHandler(ACTION_HANDLER fn) {
    LOCK _(m_cs);
    ui32 idx = m_semaphorIdx  % m_flowSemaphors.size();
    assert(idx < m_flowSemaphors.size());

    ui32 i = idx;
    do {
        FlowSemaphor *pFlowSemaphor = m_flowSemaphors[i];
        ++m_semaphorIdx;
        if ((pFlowSemaphor->m_Script2DllSwitchCounter & 1) == 0) {
            (pFlowSemaphor->*fn)();
            return;
        }
        else {
            i = m_semaphorIdx % m_flowSemaphors.size();
        }
    } while (idx != i);

    Handler(fn);
}

void DebuggerHandler::StepIn() {
    StepHandler(&DebuggerAction::StepIn);
}

void DebuggerHandler::StepOver() {
    StepHandler(&DebuggerAction::StepOver);
}

void DebuggerHandler::StepOut() {
    StepHandler(&DebuggerAction::StepOut);
}

bool DebuggerHandler::SetBreakpoint(ui32 fileId, ui32 line, bool set) {
    //LOCK _(m_cs);
    if (m_breakPointLines.size() <= fileId) {
        return false;
    }
    BREAK_POINT_LINES &breakPointLines = m_breakPointLines[fileId];
    if (breakPointLines.size() <= line) { // line 
        return false;
    }
    Node *pNode = breakPointLines[line];
    if (!pNode) {
        return false;
    }
    if (Scope *pScope = pNode->GetParent<Scope>(TYPE_SCOPE)) {
        if ((pNode->GetType() != TYPE_SCOPE) && (pScope->GetParent())) {
            if (Type *pType = pNode->QueryType<Type>(TYPE_AGGREGATE_TYPE_COLLECTION)) {
                if (Function *pFunction = pScope->GetParent()->QueryType<Function>(TYPE_FUNCTION)) {
                    vector<SYMBOL_DESC>& argSymIds = pFunction->GetArgumentVector();
                    for (ui32 i = 0; i < argSymIds.size(); ++i) {
                        if (pType->FindVariableDecNode(argSymIds[i].m_symId)) {
                            return false;
                        }
                    }
                }
            }
        }
        pScope->SetBreakpoint(fileId, pNode->GetLine(), set);
        if (m_pNotify) {
            m_pNotify->OnSetBreakPoint(fileId, line, set);
        }
        return true;
    }
    return false;

}

void DebuggerHandler::Wait() {
    WaitForSingleObject(m_hReadyEvent, INFINITE);
}

void DebuggerHandler::Ready() {
    SetEvent(m_hReadyEvent);
}
void DebuggerHandler::RegisterBreakPointLine(Node* pNode) {
    assert(pNode);
    //LOCK _(m_cs);
    ui32 fileId = pNode->GetFileId(),
         line   = pNode->GetLine();

    assert(line != 0);

    if (m_breakPointLines.size() <= fileId) {
        m_breakPointLines.resize(fileId + 1);
    }
    BREAK_POINT_LINES &breakPointLines = m_breakPointLines[fileId];
    if (breakPointLines.size() < line) {
        breakPointLines.resize(line/* line is not zero based: START_POSITION_OFFSET*/);
    }
    if (!breakPointLines[line - 1]) {
        breakPointLines[line - 1] = pNode;
    } // else multiple expressions on the same line, just skip it.
}

FlowSemaphor* DebuggerHandler::CreateFlowSemaphor(ThreadId threadId) {
    LOCK _(m_cs);
    FlowSemaphor *pFlowSemaphor = new FlowSemaphor(this, m_flowSemaphors.size() ? FlowSemaphor::FLOW_RUN : FlowSemaphor::FLOW_PAUSE);
    m_flowSemaphors.push_back(pFlowSemaphor);
    return pFlowSemaphor;
}

void DebuggerHandler::DeleteFlowSemaphor(ThreadId threadId) {
    FlowSemaphor *pFlowSemaphor = 0;
    {
        LOCK _(m_cs);
        FLOW_SEMAPHOR_MAP::iterator it = m_flowSemaphors.begin();
        for (; it != m_flowSemaphors.end(); ++it) {
            if ((*it)->GetThreadId() == threadId) {
                pFlowSemaphor = *it;
                m_flowSemaphors.erase(it);
                break;
            }
        }
    }
    if (pFlowSemaphor) {
        delete pFlowSemaphor;
    }
}

#endif // SCRIPT_DEBUGGER

/*************************************************************/

Value::Value() :
    m_pType(0),
    m_type(NULL_TYPE),
    m_pMemoryBlockRef(0) {
}

Value::Value(MemoryBlockRef *pMemBlockRef, Type *pType) :
    m_pType(pType),
    m_type(pType->GetValueType()), 
    m_pMemoryBlockRef(pMemBlockRef) {
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->AddRef();
    }
}

Value::Value(const Value &val) :
    m_pType(val.m_pType),
    m_type(val.m_type),
    m_pMemoryBlockRef(val.m_pMemoryBlockRef) {
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->AddRef();
    }
};


template<class T> T* Value::GetPtr() {
    GetMemoryBlockRef(m_pType);
    assert(m_pType->GetTypeSize() == sizeof(T));
    T *p = m_pMemoryBlockRef->GetMemoryBlock()->GetPtr<T>(m_pMemoryBlockRef->GetOffset());

    if ((m_type == STRING_TYPE) && !*(ui32*)p) {
        new(p) string();
    }
    return p;
}

MemoryBlockRef* Value::GetMemoryBlockRef(Type *pType) {
    if (!m_pMemoryBlockRef) {
        m_type = pType->GetValueType();
        m_pType = pType;
        m_pMemoryBlockRef = Memory::Allocate(pType, 0, pType->GetTypeSize());
    }
    return m_pMemoryBlockRef;
}

Value::~Value() {
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->Release();
    }
};

Value::VALUE_TYPE Value::GetType() {
    return m_type;
}

ui32 Value::SizeOf() { 
    return m_pType ? m_pType->GetTypeSize() : 0;
}

void Value::Set(MemoryBlockRef *pMemBlockRef, Type *pType, ui32 offset) {
    m_pType = pType;
    m_type = pType->GetValueType();
    if (!m_pMemoryBlockRef) {
        m_pMemoryBlockRef = Memory::Allocate(pType, offset, pMemBlockRef->GetMemoryBlock());
    }
    else {
        m_pMemoryBlockRef->SetMemoryBlock(pType, offset, pMemBlockRef->GetMemoryBlock());
    }
}

void Value::SetRef(MemoryBlockRef *pMemBlockRef) {
    assert(pMemBlockRef);
    m_pType = pMemBlockRef->GetReferenceTypeNode();
    m_type = m_pType->GetValueType();
    pMemBlockRef->AddRef();
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->Release();
    }
    m_pMemoryBlockRef = pMemBlockRef;
}

void Value::Set(const Value &val) {
    m_type = val.m_type;
    m_pType = val.m_pType;
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->Release();
    }
    m_pMemoryBlockRef = val.m_pMemoryBlockRef;
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->AddRef();
    }
}

void Value::Set(bool b) {
    Clear();
    m_type = BOOL_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_BOOL);
    *GetPtr<bool>() = b;
}

void Value::Set(i8 v) {
    Clear();
    m_type = I8_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_I8);
    *GetPtr<i8>() = v;
};

void Value::Set(ui8 v) {
    Clear();
    m_type = UI8_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_UI8);
    *GetPtr<ui8>() = v;
}

void Value::Set(i16 v) {
    Clear();
    m_type = I16_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_I16);
    *GetPtr<i16>() = v;
}

void Value::Set(ui16 v) {
    Clear();
    m_type = UI16_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_UI16);
    *GetPtr<ui16>() = v;
}

void Value::Set(i32 v) {
    Clear();
    m_type = I32_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_I32);
    *GetPtr<i32>() = v;
}

void Value::Set(ui32 v) {
    Clear();
    m_type = UI32_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_UI32);
    *GetPtr<ui32>() = v;
}

void Value::Set(i64 v) {
    Clear();
    m_type = I64_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_I64);
    *GetPtr<i64>() = v;
}

void Value::Set(ui64 v) {
    Clear();
    m_type = UI64_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_UI64);
    *GetPtr<ui64>() = v;
}

void Value::Set(float v) {
    Clear();
    m_type = FLOAT_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_FLOAT);
    *GetPtr<float>() = v;
}

void Value::Set(double v) {
    Clear();
    m_type = DOUBLE_TYPE;
    m_pType = Scope::GetDummyTypeNode(TYPE_DOUBLE);
    *GetPtr<double>() = v;
}

void Value::DeepCopy(const Value &val) {
    if (Type *pType = m_pType->QueryType<Type>(TYPE_RUNABLE)) {
        MemoryBlockRef *pDestMemoryBlockRef = GetMemoryBlockRef(pType),
                       *pSrcMemoryBlockRef = const_cast<Value&>(val).GetMemoryBlockRef(val.m_pType);
        MemoryBlock *pDestMemoryBlock = pDestMemoryBlockRef->GetMemoryBlock(),
                    *pSrcMemoryBlock = pSrcMemoryBlockRef->GetMemoryBlock();
        ui32 destOffset = pDestMemoryBlockRef->GetOffset(),
             srcOffset = pSrcMemoryBlockRef->GetOffset();
        // must handle if src and/or dest are/is reference(s)?
        //...
        pType->CopyVariableMemory(Memory::GetInstance(), pDestMemoryBlock, destOffset, pSrcMemoryBlock, srcOffset, 0, false);
    }
    else {
        assert(0);
    }
}

Value & Value::operator= (Value &val) {
ASSIGN:
    switch (m_type) {
        case AGGREGATE_TYPE: {
            switch (val.GetType()) {
                case AGGREGATE_TYPE:
                    DeepCopy(val);
                    break;
                default:
                    assert(0);
                    break;
            }
            break;
        }
        case NULL_TYPE:
            assert(m_pType == 0);
            m_pType = val.m_pType;
            m_type = val.GetType();
            goto ASSIGN;
            break;
        case BOOL_TYPE:
            *GetPtr<bool>() = val.GetBool();
            break;
        case I8_TYPE:
            *GetPtr<i8>() = val.GetI8();
            break;
        case UI8_TYPE:
            *GetPtr<ui8>() = val.GetUI8();
            break;
        case I16_TYPE:
            *GetPtr<i16>() = val.GetI16();
            break;
        case UI16_TYPE:
            *GetPtr<ui16>() = val.GetUI16();
            break;
        case I32_TYPE:
            *GetPtr<i32>() = val.GetI32();
            break;
        case FUNCTION_REF_TYPE:
        case UI32_TYPE:
            *GetPtr<ui32>() = val.GetUI32();
            break;
        case I64_TYPE:
            *GetPtr<i64>() = val.GetI64();
            break;
        case UI64_TYPE:
            *GetPtr<ui64>() = val.GetUI64();
            break;
        case FLOAT_TYPE:
            *GetPtr<float>() = val.GetFloat();
            break;
        case DOUBLE_TYPE:
            *GetPtr<double>() = val.GetDouble();
            break;
        case STRING_TYPE:
            if (val.GetType() == ARRAY_TYPE) {
                MemoryBlockRef *pMemoryBlockRef = val.GetMemoryBlockRef(val.GetTypeNode());
                Type *pType = pMemoryBlockRef->GetReferenceTypeNode();
                ArrayType *pArrayType = static_cast<ArrayType*>(pType);
                ui32 arrLen = pArrayType->GetValueSize();
                string *pString = GetPtr<string>();
                //ui32 strLen = pString->length();
                //ui32 min = (strLen > arrLen) ? arrLen : strLen;
                MemoryBlock *pMemoryblock = pMemoryBlockRef->GetMemoryBlock();
                char *pMemory = pMemoryblock->GetPtr<char>(pMemoryBlockRef->GetOffset());
                if (!arrLen) {
                    arrLen = pMemoryblock->GetSize() - pMemoryBlockRef->GetOffset();
                }
                if (arrLen != 0) {
                    ui32 i = 0;
                    for (; i < arrLen; ++i) {
                        if (pMemory[i] != '\0') {
                            pString->push_back(pMemory[i]);
                        }
                        else {
                            break;
                        }
                    }
                }
                else {
                    pString->clear();
                }
                
                //pString->push_back('\0');
            }
            else {
                *GetPtr<string>() = val.GetString();
            }
            break;
        case ARRAY_TYPE:
            if (val.GetType() == STRING_TYPE) {
                //MemoryBlockRef *pMemoryBlockRef = val.GetMemoryBlockRef(val.GetTypeNode());
                string *pString = val.GetPtr<string>();// pMemoryBlockRef->GetMemoryBlock()->GetPtr<string>(pMemoryBlockRef->GetOffset());
                ui32 strLen = pString->length() + 1;
                Type *pType = m_pMemoryBlockRef->GetReferenceTypeNode();
                ArrayType *pArrayType = static_cast<ArrayType*>(pType);
                ui32 arrLen = pArrayType->GetValueSize();
                MemoryBlock *pMemoryblock = m_pMemoryBlockRef->GetMemoryBlock();
                ui32 memSize;
                if (arrLen) {
                    memSize = (strLen >= arrLen) ? (arrLen) : strLen;
                }
                else {
                    memSize = pMemoryblock->GetSize() - m_pMemoryBlockRef->GetOffset();
                }
                if (memSize) {
                    char *pMemory = pMemoryblock->GetPtr<char>(m_pMemoryBlockRef->GetOffset());
                    memcpy(pMemory, pString->c_str(), memSize);
                    if (strLen < arrLen) {
                        pMemory[memSize] = '\0';
                    }
                }
            }
            else {
                DeepCopy(val);
            }
            break;
        default:
            assert(0);
            break;
    }

    return *this;
}

bool Value::Set(char *pArray, ui32 size) {
    if (pArray) {
        Clear();
        m_type = I64_TYPE;
        m_pType = Scope::GetDummyTypeNode(TYPE_STRING);
        GetPtr<string>()->append(pArray, size);
        return true;
    }
    return false;
}

bool Value::Set(string& str, VALUE_TYPE type, Type *pType) {
    m_pType = pType;
    if ((type == STRING_TYPE) && (m_type == STRING_TYPE)) {
        *GetPtr<string>() = str;
    }
    else {
        Clear();
        m_type = type;
        switch (type) {
            case I8_TYPE:
                *GetPtr<i8>() = (i8)stol(str);
                break;
            case UI8_TYPE:
                *GetPtr<ui8>() = (ui8)stoul(str);
                break;
            case I16_TYPE:
                *GetPtr<i16>() = (i16)stol(str);
                break;
            case UI16_TYPE:
                *GetPtr<ui16>() = (ui16)stoul(str);
                break;
            case I32_TYPE:
                *GetPtr<i32>() = (i32)stol(str);
                break;
            case UI32_TYPE:
                *GetPtr<ui32>() = (ui32)stoul(str);
                break;
            case I64_TYPE:
                *GetPtr<i64>() = (i64)stoll(str);
                break;
            case UI64_TYPE:
                *GetPtr<ui64>() = (ui64)stoull(str);
                break;
            case BOOL_TYPE:
                *GetPtr<bool>() = !(str.empty() || (str == "0") || (str == "false"));
                break;
            case FLOAT_TYPE:
                *GetPtr<float>() = stof(str);
                break;
            case DOUBLE_TYPE:
                *GetPtr<double>() = stold(str);
                break;
            case STRING_TYPE:
                *GetPtr<string>() = str;
                break;
            default:
                assert(0);
                m_type = NULL_TYPE;
                return false;
        }
    }
    return true;
}

void Value::SetType(VALUE_TYPE type, Type *pType) {
    if (m_type != type) {
        Clear();
        m_pType = pType;
        m_type = type;
    }
}

void Value::Clear() {
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->Release();
        m_pMemoryBlockRef = 0;
    }
    m_type = NULL_TYPE;
}

inline i8 Value::GetI8() {
    return GetValue<i8, i8(*)(const string&)>(Value::S2I8);
}

inline ui8 Value::GetUI8() {
    return GetValue<ui8, ui8(*)(const string&)>(Value::S2UI8);
}

inline i16 Value::GetI16() {
    return GetValue<i16, i16(*)(const string&)>(Value::S2I16);
}

inline ui16 Value::GetUI16() {
    return GetValue<ui16, ui16(*)(const string&)>(Value::S2UI16);
}

inline i32 Value::GetI32() {
    return GetValue<i32, i32(*)(const string&)>(Value::S2I32);
}

inline ui32 Value::GetUI32() {
    return GetValue<ui32, ui32(*)(const string&)>(Value::S2UI32);
}

inline i64 Value::GetI64() {
    return GetValue<i64, i64(*)(const string&)>(Value::S2I64);
}

inline ui64 Value::GetUI64() {
    return GetValue<ui64, ui64(*)(const string&)>(Value::S2UI64);
}

inline float Value::GetFloat() {
    switch (m_type) {
        case STRING_TYPE:
            return Value::S2FLOAT(*GetPtr<string>());
        case BOOL_TYPE:
            return GetBool() ? (float)1 : 0;
        case I8_TYPE:
            return (float)GetI8();
        case UI8_TYPE:
            return (float)GetUI8();
        case I16_TYPE:
            return (float)GetI16();
        case UI16_TYPE:
            return (float)GetUI16();
        case I32_TYPE:
            return (float)GetI32();
        case FUNCTION_REF_TYPE:
        case UI32_TYPE:
            return (float)GetUI32();
        case I64_TYPE:
            return (float)GetI64();
        case UI64_TYPE:
            return (float)GetUI64();
        case FLOAT_TYPE:
            return *GetPtr<float>();
        case DOUBLE_TYPE:
            return (float)GetDouble();
        default:
            assert(0);
            return 0;
    }
}

inline double Value::GetDouble() {
    switch (m_type) {
        case STRING_TYPE:
            return Value::S2DOUBLE(*GetPtr<string>());
        case BOOL_TYPE:
            return GetBool() ? (double)1 : 0;
        case I8_TYPE:
            return (double)GetI8();
        case UI8_TYPE:
            return (double)GetUI8();
        case I16_TYPE:
            return (double)GetI16();
        case UI16_TYPE:
            return (double)GetUI16();
        case I32_TYPE:
            return (double)GetI32();
        case FUNCTION_REF_TYPE:
        case UI32_TYPE:
            return (double)GetUI32();
        case I64_TYPE:
            return (double)GetI64();
        case UI64_TYPE:
            return (double)GetUI64();
        case FLOAT_TYPE:
            return (double)GetFloat();
        case DOUBLE_TYPE:
            return *GetPtr<double>();
        default:
            assert(0);
            return 0;
    }
}

inline bool Value::GetBool() {
    return GetValue<ui64, bool(*)(const string&)>(Value::S2BOOL) != 0;
}

string Value::GetString() {
    switch (m_type) {
        case I8_TYPE:
            return to_string(GetI8());
        case UI8_TYPE:
            return to_string(GetUI8());
        case I16_TYPE:
            return to_string(GetI16());
        case UI16_TYPE:
            return to_string(GetUI16());
        case I32_TYPE:
            return to_string(GetI32());
        case UI32_TYPE:
            return to_string(GetUI32());
        case I64_TYPE:
            return to_string(GetI64());
        case UI64_TYPE:
            return to_string(GetUI64());
        case FLOAT_TYPE: {
            std::stringstream ss;
            ss << std::scientific << GetFloat();
            return ss.str();
        }
        case DOUBLE_TYPE: {
            std::stringstream ss;
            ss << std::scientific << GetDouble();
            return ss.str();
        }
        case BOOL_TYPE:
            return GetBool() ? "true" : "false";
        case STRING_TYPE:
            return *GetPtr<string>();
        default:
            return "0";
    }
}

bool Value::GetHexString(string &value) {
    if (m_pType) {
        ui32 size = m_pType->GetTypeSize() * 2;
        ui64 val64;
        switch (m_type) {
            case BOOL_TYPE:
            case I8_TYPE:
            case UI8_TYPE:
                val64 = GetUI8();
                break;
            case I16_TYPE:
            case UI16_TYPE:
                val64 = GetUI16();
                break;
            case I32_TYPE:
            case UI32_TYPE:
                val64 = GetUI32();
                break;
            case FUNCTION_REF_TYPE:
                val64 = GetUI32();
                if (val64 == 0) {
                    value = "<null>";
                    return true;
                }
                break;
            case I64_TYPE:
            case UI64_TYPE:
                val64 = GetUI64();
                break;
            default:
                return false;
        }
        //str = "0x";
        while (size) {
            ui8 v8 = val64 % 16;
            if (v8 <= 9) {
                value.push_back(v8 + '0');
            }
            else {
                value.push_back(v8 + 'a' - 10);
            }
            val64 = val64 / 16;
            --size;
        }
        value += "x0";
        std::reverse(value.begin(), value.end());
    }
    else {
        assert(m_type == NULL_TYPE);
        value = "<null>";
    }
    return true;
}

bool Value::GetBinaryString(string &value) {
    if (m_pType) {
        ui32 size = m_pType->GetTypeSize() * 8;
        ui64 val64;
        switch (m_type) {
            case BOOL_TYPE:
            case I8_TYPE:
            case UI8_TYPE:
                val64 = GetUI8();
                break;
            case I16_TYPE:
            case UI16_TYPE:
                val64 = GetUI16();
                break;
            case I32_TYPE:
            case UI32_TYPE:
                val64 = GetUI32();
                break;
            case FUNCTION_REF_TYPE:
                val64 = GetUI32();
                if (val64 == 0) {
                    value = "<null>";
                    return true;
                }
                break;
            case I64_TYPE:
            case UI64_TYPE:
                val64 = GetUI64();
                break;
            default:
                return false;
        }
        while (size) {
            value.push_back((val64 & 1) + '0');
            val64 >>= 1;
            --size;
        }

        std::reverse(value.begin(), value.end());
    }
    else {
        assert(m_type == NULL_TYPE);
        value = "<null>";
    }
    return true;
}

inline bool Value::S2BOOL(const string& str) {
    return (str.length() && (str != "false")) ;
}

inline i8 Value::S2I8(const string& str) {
    return (i8)S2I32(str);
}

inline ui8 Value::S2UI8(const string& str) {
    return (ui8)S2UI32(str);
}

inline i16 Value::S2I16(const string& str) {
    return (i16)S2I32(str);
}

inline ui16 Value::S2UI16(const string& str) {
    return (ui16)S2UI32(str);
}

inline i32 Value::S2I32(const string& str) {
    return stol(str);
}

inline ui32 Value::S2UI32(const string& str) {
    return stoul(str);
}

inline i64 Value::S2I64(const string& str) {
    return stoll(str);
}

inline ui64 Value::S2UI64(const string& str) {
    return stoull(str);
}

inline float Value::S2FLOAT(const string& str) {
    return stof(str);
}

inline double Value::S2DOUBLE(const string& str) {
    return stold(str);
}

template<class T, class STR_CONV_FOO> T Value::GetValue(STR_CONV_FOO fnStrConv) {
    switch (m_type) {
        case STRING_TYPE:
            return fnStrConv(*GetPtr<string>());
        case BOOL_TYPE:
            return ((T)*(bool*)GetPtr<bool>()) != 0;
        case I8_TYPE:
            return (T)*(i8*)GetPtr<i8>();
        case UI8_TYPE:
            return (T)*(ui8*)GetPtr<ui8>();
        case I16_TYPE:
            return (T)*(i16*)GetPtr<i16>();
        case UI16_TYPE:
            return (T)*(ui16*)GetPtr<ui16>();
        case I32_TYPE:
            return (T)*(i32*)GetPtr<i32>();
        case FUNCTION_REF_TYPE:
        case UI32_TYPE:
            return (T)*(ui32*)GetPtr<ui32>();
        case I64_TYPE:
            return (T)*(i64*)GetPtr<i64>();
        case NULL_TYPE:
        case UI64_TYPE:
            return (T)*(ui64*)GetPtr<ui64>();
        case FLOAT_TYPE:
            return (T)GetFloat();
        case DOUBLE_TYPE:
            return (T)GetDouble();
        default:
            assert(0);
            return 0;
    }
}

ERROR_TYPE Value::Add(Value& v) {
    switch (m_type) {
        case NULL_TYPE:
            *this = v;
            break;
        case I8_TYPE:
            *GetPtr<i8>() += v.GetI8();
            break;
        case UI8_TYPE:
            *GetPtr<ui8>() += v.GetUI8();
            break;
        case I16_TYPE:
            *GetPtr<i16>() += v.GetI16();
            break;
        case UI16_TYPE:
            *GetPtr<ui16>() += v.GetUI16();
            break;
        case I32_TYPE:
            *GetPtr<i32>() += v.GetI32();
            break;
        case UI32_TYPE:
            *GetPtr<ui32>() += v.GetUI32();
            break;
        case I64_TYPE:
            *GetPtr<i64>() += v.GetI64();
            break;
        case UI64_TYPE:
            *GetPtr<ui64>() += v.GetUI64();
            break;
        case FLOAT_TYPE:
            *GetPtr<float>() += v.GetFloat();
            break;
        case DOUBLE_TYPE:
            *GetPtr<double>() += v.GetDouble();
            break;
        case STRING_TYPE:
            *GetPtr<string>() += v.GetString();
            break;
        default:
            return INCOMPATIBLE_TYPE;
    }
    return NO_ERROR;
}

ERROR_TYPE Value::BitNot(Value& v) {
REPEAT:
    switch (m_type) {
        case BOOL_TYPE:
            *GetPtr<bool>() = !v.GetBool();
            break;
        case I8_TYPE:
            *GetPtr<i8>() = ~v.GetI8();
            break;
        case UI8_TYPE:
            *GetPtr<ui8>() = ~v.GetUI8();
            break;
        case I16_TYPE:
            *GetPtr<i16>() = ~v.GetI16();
            break;
        case UI16_TYPE:
            *GetPtr<ui16>() = ~v.GetUI16();
            break;
        case I32_TYPE:
            *GetPtr<i32>() = ~v.GetI32();
            break;
        case UI32_TYPE:
            *GetPtr<ui32>() = ~v.GetUI32();
            break;
        case I64_TYPE:
            *GetPtr<i64>() = ~v.GetI64();
            break;
        case UI64_TYPE:
            *GetPtr<ui64>() = ~v.GetUI64();
            break;
        case NULL_TYPE:
            if (v.GetType() != NULL_TYPE) {
                SetType(v.GetType(), v.m_pType);
            }
            else {
                SetType(UI64_TYPE, v.m_pType);
            }
            goto REPEAT;
            break;
        default:
            return INCOMPATIBLE_TYPE;
    }
    return NO_ERROR;

}

void* const Value::GetRaw() {
    switch (m_type) {
        case STRING_TYPE: {
            MemoryBlockRef *pMemoryBlockRef = GetMemoryBlockRef(m_pType);
            return (void*)(pMemoryBlockRef->GetMemoryBlock()->GetPtr<string>(pMemoryBlockRef->GetOffset()))->c_str();
        }
        case NULL_TYPE:
            assert(0);
            return 0;
        default: {
            MemoryBlockRef *pMemoryBlockRef = GetMemoryBlockRef(m_pType);
            return (void*)((ui8*)*pMemoryBlockRef->GetMemoryBlock()->GetRefToMemoryPtr() + pMemoryBlockRef->GetOffset());
        }
    }
}

/**************************************************************/
StackMemory::StackMemory(ThreadId threadId
#ifdef SCRIPT_DEBUGGER
    , DebuggerHandler *pDebuggerHandler
#endif // SCRIPT_DEBUGGER
    , SymbolStore     *pSymStore
    , StdOutInterface *pStdOutInterface) :
    m_error(pSymStore, pStdOutInterface),
#ifdef SCRIPT_DEBUGGER
    m_pDebuggerHandler(pDebuggerHandler),
    m_pFlowSemaphor(m_pDebuggerHandler->CreateFlowSemaphor(threadId)),
#endif // SCRIPT_DEBUGGER
    m_threadId(threadId), 
    m_memoryFrameIdx(INVALID_STACK_FRAME_IDX)//, 
    /*m_bMainThread(false)*/ {
}

StackMemory::~StackMemory() {
#ifdef SCRIPT_DEBUGGER
    m_pDebuggerHandler->DeleteFlowSemaphor(m_threadId);
#endif // SCRIPT_DEBUGGER
}

void StackMemory::PushFrame(Node *pFunctionNode, Scope *pFunctionScope, vector<LOCAL_VAR_DESCRIPTOR> &types) {
    m_memoryFrames.push_back(new MEM_FRAME(pFunctionNode, pFunctionScope));
    m_memoryFrameIdx = m_memoryFrames.size() - 1;
    for (ui32 i = 0; i < types.size(); ++i) {
        Type *pType = types[i];
        PushMemoryBlockRef(pType);
    }
}

ui32 StackMemory::PopFrame() {
    assert(m_memoryFrames.size());
    delete m_memoryFrames.back();
    m_memoryFrames.pop_back();
    m_memoryFrameIdx = m_memoryFrames.size() - 1;
    return m_memoryFrameIdx;
}

void StackMemory::PushResultMemoryBlockRef(Node *pFunctionNode, Scope *pFunctionScope, MemoryBlockRef *pMemoryBlockRef) {
    MEM_FRAME *pFrame = new MEM_FRAME(pFunctionNode, pFunctionScope);
    m_memoryFrames.push_back(pFrame);
    m_memoryFrameIdx = m_memoryFrames.size() - 1;
    pFrame->push_back(MEM_REF_FRAME_SPOT(pMemoryBlockRef));
}

ui32 StackMemory::PopResultMemoryBlockRef() {
    return PopFrame();
}

void StackMemory::PushReferencedMemoryBlock(ui32 idx, Type *pType, ui32 offset, MemoryBlock *pMemoryBlock) {
    ui32 frameIdx = m_memoryFrameIdx;
    ui32 memBlockBit = (MEMORY_BLOCK_BIT_MASK & idx);
    if (memBlockBit & MEMORY_BLOCK_BIT_GLOBAL) {
        idx &= MEMORY_BLOCK_MASK;
        frameIdx = 0;
    }

    MEM_FRAME *pFrame = m_memoryFrames[frameIdx];

    if (memBlockBit & MEMORY_OFFSET_BIT_SUB_STACK) {
        MEM_REF_FRAME_SPOT &spot = (*pFrame)[idx & MEMORY_BLOCK_MASK];
        spot.PushReferencedMemoryBlockRef(pType, offset, pMemoryBlock);
    }
    else {
        assert(0);
    }
}

void StackMemory::PopReferencedMemoryBlock(ui32 idx) {
    ui32 frameIdx = m_memoryFrameIdx;
    ui32 memBlockBit = (MEMORY_BLOCK_BIT_MASK & idx);
    if (memBlockBit & MEMORY_BLOCK_BIT_GLOBAL) {
        idx &= MEMORY_BLOCK_MASK;
        frameIdx = 0;
    }

    MEM_FRAME *pFrame = m_memoryFrames[frameIdx];

    if (memBlockBit & MEMORY_OFFSET_BIT_SUB_STACK) {
        MEM_REF_FRAME_SPOT &spot = (*pFrame)[idx & MEMORY_BLOCK_MASK];
        spot.PopReferencedMemoryBlockRef();
    }
    else {
        assert(0);
    }
}

void StackMemory::PushMemoryBlockRef(Type *pType) {
    MEM_FRAME *pFrame = m_memoryFrames[m_memoryFrameIdx];
    MemoryBlockRef *pMemoryBlockRef = Memory::Allocate(pType, 0, pType->GetValueSize());
    pFrame->push_back(MEM_REF_FRAME_SPOT(pMemoryBlockRef));
    pMemoryBlockRef->Release();
}

MemoryBlockRef* StackMemory::GetMemoryBlockRef(ui32 idx) {
    ui32 frameIdx = m_memoryFrameIdx;
    ui32 memBlockBit = (MEMORY_BLOCK_BIT_MASK & idx);
    if (memBlockBit & MEMORY_BLOCK_BIT_GLOBAL) {
        idx &= MEMORY_BLOCK_MASK;
        frameIdx = 0;
    }

    MemoryBlockRef *pMemoryBlockRef = 0;
    MEM_FRAME *pFrame = m_memoryFrames[frameIdx];

    if (memBlockBit & MEMORY_OFFSET_BIT_SUB_STACK) {
        MEM_REF_FRAME_SPOT &spot = (*pFrame)[idx & MEMORY_BLOCK_MASK];
        pMemoryBlockRef = spot.GetReferencedMemoryBlockRef();
    }
    else {
        pMemoryBlockRef = (*pFrame)[idx].m_pMemoryBlockRef;
    }

    return pMemoryBlockRef;
}

MemoryBlockRef* StackMemory::GetMemoryBlockRef(ui32 idx, ui32 frameIdx) {
    assert(m_memoryFrameIdx >= frameIdx);

    MEM_FRAME *pFrame = m_memoryFrames[frameIdx];
    
    assert(pFrame->size() > idx);

    return (*pFrame)[idx].m_pMemoryBlockRef;
}

ui32 StackMemory::GetCurFrameMemoryBlockRefCount() {
    assert(m_memoryFrameIdx < m_memoryFrames.size());
    return m_memoryFrames[m_memoryFrameIdx]->size();
}

void StackMemory::UpdateMemoryBlockRef(ui32 idx, MemoryBlockRef *pMemoryBlockRef) {
    ui32 frameIdx = m_memoryFrameIdx;
    ui32 memBlockBit = (MEMORY_BLOCK_BIT_MASK & idx);
    if (memBlockBit & MEMORY_BLOCK_BIT_GLOBAL) {
        idx &= MEMORY_BLOCK_MASK;
        frameIdx = 0;
    }

    MEM_FRAME *pFrame = m_memoryFrames[frameIdx];

    if (memBlockBit & MEMORY_OFFSET_BIT_SUB_STACK) {
        idx &= MEMORY_BLOCK_MASK;
        (*pFrame)[idx].UpdateReferencedMemoryBlockRef(pMemoryBlockRef);
    }
    pMemoryBlockRef->AddRef();
    (*pFrame)[idx].m_pMemoryBlockRef->Release();
    (*pFrame)[idx].m_pMemoryBlockRef = pMemoryBlockRef;
}

void StackMemory::UpdateMemoryBlock(ui32 idx, Type *pType, ui32 offset, MemoryBlock *pMemoryBlock) {
    ui32 frameIdx = m_memoryFrameIdx;
    ui32 memBlockBit = (MEMORY_BLOCK_BIT_MASK & idx);
    if (memBlockBit & MEMORY_BLOCK_BIT_GLOBAL) {
        idx = idx & MEMORY_BLOCK_MASK;
        frameIdx = 0;
    }


    if (memBlockBit & MEMORY_OFFSET_BIT_SUB_STACK) {
        idx &= MEMORY_BLOCK_MASK;
    }

    MEM_FRAME *pFrame = m_memoryFrames[frameIdx];
    (*pFrame)[idx].m_pMemoryBlockRef->SetMemoryBlock(pType, offset, pMemoryBlock);
}

void StackMemory::SetFrameIndex(ui32 frameIdx) {
    assert(frameIdx < m_memoryFrames.size());
    m_memoryFrameIdx = frameIdx;
}

bool StackMemory::GetFrameFunctionAndScope(ui32 frameIdx, Node *&pFunctionNode, Scope *&pFunctionScope, Node *&pCurrentNode) {
    if (frameIdx < m_memoryFrames.size()) {
        MEM_FRAME *pFrame = m_memoryFrames[frameIdx];
        pFunctionNode  = pFrame->m_pFunctionNode;
        pFunctionScope = pFrame->m_pFunctionScope;
        pCurrentNode   = pFrame->m_pCurrentNode;
        return true;
    }
    return false;
}

bool StackMemory::GetFrameFunctionAsStrings(ui32 frameIdx, string &fooType, string &location, string &fileName, ui32 &line, ui32 &pos) {
    SymbolStore *pSymStore = m_error.GetSymbolStore();
    Node  *pFunctionNode = 0;
    Scope *pFunctionScope = 0;
    Node  *pCurrentNode = 0;
    
    GetFrameFunctionAndScope(frameIdx, pFunctionNode, pFunctionScope, pCurrentNode);
    
    if (!pCurrentNode) { // this is a thread or a callback, just skip this frame for now!
        return false;
    }

    if (0 == frameIdx) {
//FRAME_0:
        pSymStore->GetFileName(pCurrentNode->GetFileId(), fileName);
        fooType = fileName.substr(fileName.rfind('\\') + 1);
        location = "Script";
        line = pCurrentNode->GetLine();
        pos = pCurrentNode->GetPosition();
    }
    else {
        //if (!pFunctionScope && frameIdx) {
        //    // this is a function call from Dll
        //    // step down one frame to get actual function call
        //    --frameIdx;
        //    GetFrameFunctionAndScope(frameIdx - 1, pFunctionNode, pFunctionScope, pCurrentNode);
        //    if (0 == frameIdx) {
        //        goto FRAME_0;
        //    }
        //}

        pSymStore->GetFileName(pCurrentNode->GetFileId(), fileName);

        line = pCurrentNode->GetLine();
        pos = pCurrentNode->GetPosition();

        pSymStore->GetSymbolName(pFunctionNode, fooType);

        Function *pFunction = pFunctionNode->QueryType<Function>(TYPE_FUNCTION);
        if (!pFunction) {
            FunctionCallbackPtr *pFunctionCallbackPtr = pFunctionNode->QueryType<FunctionCallbackPtr>(TYPE_FUNCTION_CALLBACK_PTR);
            
            assert(pFunctionCallbackPtr);

            pFunction = pFunctionCallbackPtr->GetParent()->QueryType<Function>(TYPE_FUNCTION);
        }

        /*************************************/
        FunctionRefType *pFunctionRefType = 0;
        if (pFunction->GetType() == TYPE_FUNCTION_LIB_PTR) {
            Variable *pVariable = pFunction->GetParent()->QueryType<Variable>(TYPE_VAR);

            assert(pVariable);

            pFunctionRefType = pVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
        }
        else {
            pFunctionRefType = pFunction->GetFunctionRefTypeNode();
        }
        
        assert(pFunctionRefType);

        Type *pType = pFunctionRefType->GetReturnType();
        fooType = pType->GetTypeString(pSymStore) + " " + fooType + "(";
        ui32 argCount = pFunctionRefType->GetArgCount();
        for (ui32 k = 1; k <= argCount; ++k) {
            Type *pType = pFunctionRefType->GetArgType(k);
            fooType += pType->GetTypeString(pSymStore);
            if (k != argCount) {
                fooType += ", ";
            }
        }
        fooType += ")";

        /*************************************/
        if (pFunctionNode->GetType() == TYPE_FUNCTION_LIB_PTR) {
            location = ((LibType*)pFunctionNode->GetParent()->GetParent()->GetParent())->GetPath();
        }
        else {
            location = "Script";
        }
    }
    return true;
}

Node* StackMemory::GetFrameFunctionNode(ui32 frameIdx) {
    if (frameIdx < m_memoryFrames.size()) {
        MEM_FRAME *pFrame = m_memoryFrames[frameIdx];
        return pFrame->m_pFunctionNode;
    }
    return 0;
}

//#ifdef SCRIPT_DEBUGGER
void StackMemory::SetCurRunningNode(Node *pNode) {
    assert(m_memoryFrameIdx != INVALID_STACK_FRAME_IDX);
    MEM_FRAME *pFrame = m_memoryFrames[m_memoryFrameIdx];
    pFrame->m_pCurrentNode = pNode;
}
//#endif // SCRIPT_DEBUGGER

void StackMemory::PopulateGlobals(StackMemory *pStackMemory) {
    if (pStackMemory->m_memoryFrameIdx != INVALID_STACK_FRAME_IDX) {
        MEM_FRAME *pOutFrame = pStackMemory->m_memoryFrames[0];
        m_memoryFrames.push_back(new MEM_FRAME(pOutFrame->m_pFunctionNode, pOutFrame->m_pFunctionScope));
        m_memoryFrameIdx = 0;
        MEM_FRAME *pFrame = m_memoryFrames[m_memoryFrameIdx];
        ui32 count = pOutFrame->size();
        for (ui32 i = 0; i < count; ++i) {
            pFrame->push_back(MEM_REF_FRAME_SPOT((*pOutFrame)[i].m_pMemoryBlockRef));
        }
    }
    else {
        assert(0);
    }
}

void StackMemory::GetString(string &value) {
    SymbolStore *pSymbolStore = m_error.GetSymbolStore();
    ui32 count = m_memoryFrameIdx;
    value.clear();

    for (i32 i = count; i >= 0; --i) {
        ui32   frameIdx = i;
        string fooType, location, fileName;
        ui32 line, pos;

        if (GetFrameFunctionAsStrings(frameIdx, fooType, location, fileName, line, pos)) {
            value += fooType + " [" + fileName + ", line: " + to_string(line) + ", pos: " + to_string(pos) +"]";
            if (i > 0) {
                value += "\r\n";
            }
        }
    }
}

void StackMemory::GetTrace(vector<string> &trace) {
    SymbolStore *pSymbolStore = m_error.GetSymbolStore();
    ui32 count = m_memoryFrameIdx;
    //value.clear();

    for (i32 i = count; i >= 0; --i) {
        ui32   frameIdx = i;
        string fooType, location, fileName;
        ui32 line, pos;

        if (GetFrameFunctionAsStrings(frameIdx, fooType, location, fileName, line, pos)) {
            string str;
            str = "        " + fooType + " : \"" + fileName + "\", line: " + to_string(line) + ", pos: " + to_string(pos);
            trace.push_back(str);
        }
    }
}

/*************************************************************/
#include "Script.h"

Memory::Memory(SymbolStore *pSymbolStore,
#ifdef SCRIPT_DEBUGGER
    DebuggerHandler *pDebuggerHandler,
#endif // SCRIPT_DEBUGGER
    StdOutInterface *pStdOutInterface) : 
    m_pRootScope(0),
    m_pMainStackMemory(0),
    m_pVirtualMemory(0),
    m_virtualMemoryNextOffset(0),
#ifdef SCRIPT_DEBUGGER
    m_pDebuggerHandler(pDebuggerHandler),
#endif // SCRIPT_DEBUGGER
    m_pSymbolStore(pSymbolStore),
    m_pStdOutInterface(pStdOutInterface),
    m_pDestroyMemoryHandler(new DestroyMemoryHandler(this)),
    m_pClearMemoryHandler(new ClearMemoryHandler(this)) {
    InitializeCriticalSection(&m_cs);
#if defined(TRACE_OBJECTS) || defined(TRACE_MEMORY)
    InitializeCriticalSection(&m_traceCS);
#endif // defined(TRACE_OBJECTS) || defined(TRACE_MEMORY)
}


Memory::~Memory() {
    if (m_pVirtualMemory) {
        VirtualFree(m_pVirtualMemory, 0, MEM_RELEASE);
    }
    STACK_MEM_BY_THREAD_ID::iterator it;
    for (it = m_stackMemoryByThreadId.begin(); it != m_stackMemoryByThreadId.end(); ++it) {
        delete it->second;
    }
    m_stackMemoryByThreadId.clear();
#if defined(TRACE_OBJECTS) || defined(TRACE_MEMORY)
    DeleteCriticalSection(&m_traceCS);
#endif // defined(TRACE_OBJECTS) || defined(TRACE_MEMORY)
    DeleteCriticalSection(&m_cs);
    delete m_pDestroyMemoryHandler;
}

void Memory::Clean(MEMORY &vec) {
    for (MEMORY::iterator it = vec.begin(); it != vec.end(); ++it) {
        delete[](*it);
    }
}

// this is valid while the script is running
static Memory *s_pMemory;

Memory& Memory::GetInstance() {
    return *s_pMemory;
}

void Memory::SetInstance(Memory *pMemory) {
    s_pMemory = pMemory;
}

#ifdef TRACE_OBJECTS

void Memory::AddObject(Runable* p) {
    LOCK _(m_traceCS);
    try {
        m_objects.insert(p);
    }
    catch (...) {}
}

#define CASE(n) case n: return #n;

string TypeToString(NODE_TYPE type) {
    switch (type) {
        CASE(TYPE_NONE)
        CASE(TYPE_RUNABLE)
        CASE(TYPE_TOKEN)
        CASE(TYPE_CONST)
        CASE(TYPE_CONST_NUM)
        CASE(TYPE_OPERATOR)
        CASE(TYPE_ASSIGN)
        CASE(TYPE_EQ)
        CASE(TYPE_ADD)
        CASE(TYPE_ADD_ASSIGN)
        CASE(TYPE_INC_OP)
        CASE(TYPE_PRE_INC_OP)
        CASE(TYPE_POST_INC_OP)
        CASE(TYPE_SUB)                  // -
        CASE(TYPE_SUB_ASSIGN)           // -=
        CASE(TYPE_DEC_OP)               // -- this is invalid type, just for intermidiate parsing
        CASE(TYPE_PRE_DEC_OP)           // --N
        CASE(TYPE_POST_DEC_OP)          // N--
        CASE(TYPE_BIT_AND)              // &
        CASE(TYPE_BIT_AND_ASSIGN)       // &=
        CASE(TYPE_AND)                  // &&
        CASE(TYPE_BIT_OR)               // |
        CASE(TYPE_BIT_OR_ASSIGN)        // |=
        CASE(TYPE_OR)                   // ||
        CASE(TYPE_LESS)                 // <
        CASE(TYPE_LESS_EQ)              // <=
        CASE(TYPE_SHIFT_LEFT)           // <<
        CASE(TYPE_SHIFT_LEFT_ASSIGN)    // <<=
        CASE(TYPE_GREATER)              // >
        CASE(TYPE_GREATER_EQ)           // >=
        CASE(TYPE_SHIFT_RIGHT)          // >>
        CASE(TYPE_SHIFT_RIGHT_ASSIGN)   // >>=
        CASE(TYPE_MUL)                  // *
        CASE(TYPE_MUL_ASSIGN)           // *=
        CASE(TYPE_DIV)                  // /
        CASE(TYPE_DIV_ASSIGN)           // /=
        CASE(TYPE_MOD)                  // %
        CASE(TYPE_MOD_ASSIGN)           // %=
        CASE(TYPE_BIT_NOT)              // ~
        CASE(TYPE_BIT_NOT_ASSIGN)       // ~= this is invalid operation!
        CASE(TYPE_NOT)                  // !
        CASE(TYPE_NOT_EQ)               // !=
        CASE(TYPE_XOR)                  // ^
        CASE(TYPE_XOR_ASSIGN)           // ^=
        CASE(TYPE_REF_ASSIGN)           // @=
        CASE(TYPE_SUBSCRIPT)            // []
        CASE(TYPE_PARENTHESIS)          // ()
        CASE(TYPE_BASIC_SCOPE)          // 
        CASE(TYPE_SCOPE)                // {}
        CASE(TYPE_CASE_SCOPE)           // scope of "case/default VAL: CASESCOPE break;"
        CASE(TYPE_DOT)                  // .
        //CASE(TYPE_MEMBER_ACCESS)      // .
        CASE(TYPE_COMMA)                // ,
        CASE(TYPE_SEMICOLON)            // ;
        CASE(TYPE_COLON)                // :
        CASE(TYPE_REF)                  // @
        CASE(TYPE_CONDITIONAL)          // 
        CASE(TYPE_FOR)                  // for
        CASE(TYPE_WHILE)                // while
        CASE(TYPE_IF)                   // if
        CASE(TYPE_ELSE)                 // else
        CASE(TYPE_SWITCH)               // switch
        CASE(TYPE_CASE)                 // case
        CASE(TYPE_DEFAULT)              // default
        CASE(TYPE_BREAK)                // break
        CASE(TYPE_CONTINUE)             // continue
        CASE(TYPE_FUNCTION_CALL)        // foo(arg) function call
        CASE(TYPE_FUNCTION)             // actual function implementation
        CASE(TYPE_FUNCTION_PTR)
        CASE(TYPE_FUNCTION_LIB_PTR)
        CASE(TYPE_FUNCTION_CALLBACK_PTR)
        CASE(TYPE_RETURN)               // return
        //CASE(TYPE_NEW)                // new
        CASE(TYPE_FUNCTION_REF_TYPE)    // function<Foo_Name> - function type declaration
        CASE(TYPE_QUOTATION_MARK)       // "
        // built in types
        CASE(TYPE_BOOL)
        CASE(TYPE_I8)
        CASE(TYPE_UI8)
        CASE(TYPE_I16)
        CASE(TYPE_UI16)
        CASE(TYPE_I32)
        CASE(TYPE_UI32)
        CASE(TYPE_I64)
        CASE(TYPE_UI64)
        CASE(TYPE_FLOAT)
        CASE(TYPE_DOUBLE)
        CASE(TYPE_STRING)
        CASE(TYPE_CONST_BOOL)
        CASE(TYPE_LIB_VAR)
        CASE(TYPE_ERROR_VAR)
        CASE(TYPE_ERROR)
        CASE(TYPE_ERROR_TYPE)
        CASE(TYPE_SIZEOF)
        CASE(TYPE_ARR2STR)
        CASE(TYPE_WARR2STR)
        CASE(TYPE_STR2ARR)
        CASE(TYPE_STR2WARR)
        CASE(TYPE_LOCK)
        CASE(TYPE_UNLOCK)
        CASE(TYPE_CAST)

        // artificial types
        CASE(TYPE_SCOPE_END_EXPRESSION)              // this is a special type for TextSerializer prints ";\n"
        CASE(TYPE_BLANK_SCOPE)                       // a scope without {}
        CASE(TYPE_EMPTY)                             // this is a hack for TextSerializer
        CASE(TYPE_AGGREGATE_SYMBOL)                  // for int and floating point numbers
        CASE(TYPE_INCLUDE)
        CASE(TYPE_LIB)
        CASE(TYPE_STRUCT)
        CASE(TYPE_ARRAY)
        CASE(TYPE_AGGREGATE_TYPE)
        CASE(TYPE_AGGREGATE_TYPE_COLLECTION)
        CASE(TYPE_PRE_POST_EXPRESSIONS)
        default:
            return "UNKNOWN";
    }
}

void Memory::ReleaseObject(Runable* p) {
    assert(m_pSymbolStore);
#ifdef TRACE_DELETE_OBJECTS
    SYMBOL_DESC symDesc;
    string token;
    p->GetSymbolDesc(symDesc);
    m_pSymbolStore->GetSymbolName(symDesc.m_symId, token);
    string type = TypeToString(p->GetType());
    //m_pStdOutInterface->Print(DEBUG_OUT, string("\n\n**** Leaked Memory Block References ****\n"));
    //printf("\n DELETE: %p %s (l:%d, p:%d) : %s\n", p, token.c_str(), symDesc.m_line, symDesc.m_pos, type.c_str());

    std::stringstream ss;
    ss << " DELETE: " << std::hex << p << " " << token.c_str() << " (l:" << std::dec << symDesc.m_line << ", p:" << symDesc.m_pos << ") : " << type.c_str();
    m_pStdOutInterface->Print(DEBUG_OUT, ss.str());
#endif // TRACE_DELETE_OBJECTS
    LOCK _(m_traceCS);
    try {
        m_objects.erase(p);
    }
    catch (...) {}
}

void Memory::DumpObjects() {
    assert(m_pSymbolStore);
    m_pStdOutInterface->Print(DEBUG_OUT, string("\n\n**** Leaked Objects ****\n"));
    if (m_objects.size()) {
        for (OBJECTS::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
            SYMBOL_DESC symDesc;
            string token;
            (*it)->GetSymbolDesc(symDesc);
            m_pSymbolStore->GetSymbolName(symDesc.m_symId, token);
            string type = TypeToString((*it)->GetType());
            std::stringstream ss;
            ss << std::hex << (*it) << " " << token.c_str() << " (l:" << std::dec << symDesc.m_line << ", p:" << symDesc.m_pos << ") : " << type.c_str();
            //printf("%p %s (l:%d, p:%d) : %s\n", (*it), token.c_str(), symDesc.m_line, symDesc.m_pos, type.c_str());
            m_pStdOutInterface->Print(DEBUG_OUT, ss.str());
        }
    }
    else {
        m_pStdOutInterface->Print(DEBUG_OUT, string("NONE\n"));
    }
    m_pStdOutInterface->Print(DEBUG_OUT, string("************************\n"));
}

#endif // TRACE_OBJECTS

#if defined(TRACE_MEMORY)

void Memory::AddMemoryBlock(MemoryBlock *p) {
    LOCK _(m_traceCS);
    try {
        m_memoryBlocks.insert(p);
    }
    catch (...) {}
}

void Memory::ReleaseMemoryBlock(MemoryBlock *p) {
    LOCK _(m_traceCS);
    try {
        m_memoryBlocks.erase(p);
    }
    catch (...) {}
}

void Memory::AddMemoryBlockRef(MemoryBlockRef *p) {
    LOCK _(m_traceCS);
    try {
        m_memoryBlockRefs.insert(p);
    }
    catch (...) {}
}

void Memory::ReleaseMemoryBlockRef(MemoryBlockRef *p) {
    LOCK _(m_traceCS);
    try {
        m_memoryBlockRefs.erase(p);
    }
    catch (...) {}
}

void Memory::DumpMemoryBlocks() {
    m_pStdOutInterface->Print(DEBUG_OUT, string("\n\n**** Leaked Memory Block References ****\n"));
    LOCK _(m_traceCS);
    if (m_memoryBlockRefs.size()) {
        for (MEMORY_BLOCK_REFS::iterator it = m_memoryBlockRefs.begin(); it != m_memoryBlockRefs.end(); ++it) {
            MemoryBlock *pMemoryBlock = (*it)->GetMemoryBlock();
            Type *pType = pMemoryBlock ? pMemoryBlock->GetTypeNode() : (*it)->GetReferenceTypeNode();
            if (Scope::GetDummyTypeNode(pType->GetType()) != pType) {
                SymbolDesc symDesc;
                pType->GetSymbolDesc(symDesc);
                string name;
                m_pSymbolStore->GetSymbolName(symDesc.m_symId, name);
                std::stringstream ss;
                ss << "0x" << std::hex << (*it) << ", name: \"" << name.c_str() << "\", line: " << std::dec << symDesc.m_line << ", pos: " << symDesc.m_pos
                    << ", memBlock: 0x" << std::hex << pMemoryBlock << ", refCount: " << std::dec << (*it)->GetRefCount();
                m_pStdOutInterface->Print(DEBUG_OUT, ss.str());
            }
            continue;

            std::stringstream ss;
            ss << "0x" << std::hex << (*it) << ", memBlock: 0x" << (*it)->GetMemoryBlock() << 
                ", offset: " << std::dec << (*it)->GetOffset() <<
                ", refCount: " << std::dec << (*it)->GetRefCount();
            m_pStdOutInterface->Print(DEBUG_OUT, ss.str());
        }
    }
    else {
        //printf("NONE\n");
        m_pStdOutInterface->Print(DEBUG_OUT, string("NONE"));
    }
    m_pStdOutInterface->Print(DEBUG_OUT, string("****************************************"));

    m_pStdOutInterface->Print(DEBUG_OUT, string("**** Leaked Memory Blocks ****"));
    if (m_memoryBlocks.size()) {
        for (MEMORY_BLOCKS::iterator it = m_memoryBlocks.begin(); it != m_memoryBlocks.end(); ++it) {
            Node *pNode = (*it)->GetTypeNode();
            if (Scope::GetDummyTypeNode(pNode->GetType()) != pNode) {
                SymbolDesc symDesc;
                pNode->GetSymbolDesc(symDesc);
                string name;
                m_pSymbolStore->GetSymbolName(symDesc.m_symId, name);
                ui32 refCount = (*it)->GetRefCount();
                bool locked = (refCount & 0x80000000) != 0;
                refCount = 0x7fffffff & refCount;
                std::stringstream ss;
                ss << "0x" << std::hex << (*it) << ", name: \"" << name.c_str() << "\", line: " << std::dec << symDesc.m_line << ", pos: " << symDesc.m_pos
                   << ", size: " << (*it)->GetSize() << ", refCount: " << refCount << (locked ? " , LOCKED" : "");
                m_pStdOutInterface->Print(DEBUG_OUT, ss.str());
            }
        }
    }
    else {
        m_pStdOutInterface->Print(DEBUG_OUT, string("NONE"));
    }
    m_pStdOutInterface->Print(DEBUG_OUT, string("****************************************"));
}

#endif // defined(TRACE_MEMORY)

StackMemory* Memory::GetThreadMemory(ThreadId threadId) {
    StackMemory *pStackMemory = 0;
    LOCK _(m_cs);
    try {
        pair<STACK_MEM_BY_THREAD_ID::iterator, bool> res = m_stackMemoryByThreadId.insert(pair<ThreadId, StackMemory*>(threadId, 0));
        if (res.second) {
            res.first->second = new StackMemory(threadId,
#ifdef SCRIPT_DEBUGGER
                m_pDebuggerHandler,
#endif // SCRIPT_DEBUGGER
                m_pSymbolStore,
                m_pStdOutInterface);
            if (m_stackMemoryByThreadId.size() == 1) {
                m_pMainStackMemory = res.first->second;
                //m_pMainStackMemory->SetMainThread();
            }
        }
        pStackMemory = res.first->second;
    }
    catch (...) {}
    return pStackMemory;
}

void Memory::GetThreadIds(vector<ThreadId> &threadIds) {
    LOCK _(m_cs);
    try {
        STACK_MEM_BY_THREAD_ID::iterator it = m_stackMemoryByThreadId.begin();
        while (it != m_stackMemoryByThreadId.end()) {
            threadIds.push_back(it++->first);
        }
    }
    catch (...) {}
}

void Memory::ReleaseThreadMemory(ThreadId threadId) {
    StackMemory *pStackMemory = 0;
    {
        LOCK _(m_cs);
        try {
            STACK_MEM_BY_THREAD_ID::iterator it = m_stackMemoryByThreadId.find(threadId);
            if (it != m_stackMemoryByThreadId.end()) {
                if (it->second == m_pMainStackMemory) {
                    m_pMainStackMemory = 0;
                }
                pStackMemory = it->second;
                m_stackMemoryByThreadId.erase(it);
            }
        }
        catch (...) {}
    }
    if (pStackMemory) {
        delete pStackMemory;
    }
}

MemoryBlock* Memory::GetReferencedMemoryBlock(Type *pType, MemoryBlock *pMemoryBlock, ui32 &offset) {//, ui32 size) {
    MemoryBlock *pReferencedMemoryBlock = 0;
    void **pMemory = pMemoryBlock->GetPtr<void*>(offset);
    if (!*pMemory) {
        ui32 size = pType->GetTypeSize();
        if (ui8* pMem = new ui8[size + sizeof(MemoryBlock)]) {
            pReferencedMemoryBlock = new(pMem) MemoryBlock(pType, size);
            *pMemory = *pReferencedMemoryBlock->GetRefToMemoryPtr();
            pReferencedMemoryBlock->Register(*this);
            pReferencedMemoryBlock->AddRef();
            offset = 0;
        }
        else {
            assert(0);
            return 0;
        }
    }
    else {
        if (pReferencedMemoryBlock = QueryMemoryBlock(*pMemory)) { // this call adds reference
            offset = (ui32)*pMemory - (ui32)*pReferencedMemoryBlock->GetRefToMemoryPtr();
        }
        else {
            TODO("if dynamic array?");
            ui32 size = pType->GetTypeSize();
            offset = 0;
            pReferencedMemoryBlock = Memory::AllocateMemoryBlockForExternal(pType, *pMemory, size);
            assert(pReferencedMemoryBlock);
            pReferencedMemoryBlock->AddRef();
        }
    }
    return pReferencedMemoryBlock;
}

MemoryBlockRef* Memory::Allocate(Type *pType, ui32 offset, ui32 size) {
    MemoryBlockRef *pMemoryBlockRef = 0;
    pMemoryBlockRef = new MemoryBlockRef(pType, offset, size);
    return pMemoryBlockRef;
}

MemoryBlockRef* Memory::Allocate(Type *pType, ui32 offset, MemoryBlock *pMemoryBlock) {
    MemoryBlockRef* pMemoryBlockRef = new MemoryBlockRef(pType, offset, pMemoryBlock);
    return pMemoryBlockRef;
}

MemoryBlockRef* Memory::QueryMemoryBlockRef(void *pMemory, Type *pType) {
    MemoryBlockRef *pMemoryBlockRef = 0;
    if (MemoryBlock *pMemoryBlock = QueryMemoryBlock(pMemory)) { // this call adds reference
        pMemoryBlockRef = Allocate(pType, (ui32)pMemory - (ui32)*pMemoryBlock->GetRefToMemoryPtr(), pMemoryBlock);
        pMemoryBlock->Release();
    }
    else {
        pMemoryBlockRef = AllocateWrapper(pType, pMemory, pType->GetTypeSize()); //  should this block to be registered??? !!!
    }

    return pMemoryBlockRef;
}

void Memory::DereferenceMemory(Type *pType, /*[in,out] parameter, returns dereferenced memory block*/MemoryBlock *&pMemoryBlock, /*[in,out] parameter, returns offset of returned memory block*/ui32 &offset) {

    void **pMemory = pMemoryBlock->GetPtr<void*>(offset);
    if (!*pMemory) {
        offset = 0;
        pMemoryBlock = 0;
        return;
    }

    MemoryBlock *pDereferencedMemoryBlock = QueryMemoryBlock(*pMemory); // this call adds reference

    if (pDereferencedMemoryBlock) {

        offset = (ui32)*pMemory - (ui32)*pDereferencedMemoryBlock->GetRefToMemoryPtr();
        pMemoryBlock = pDereferencedMemoryBlock;
    }
    else {
        TODO("what if it's an array and it's dynamic??");
        ui32 size = pType->GetTypeSize();
        offset = 0;
        pMemoryBlock = Memory::AllocateMemoryBlockForExternal(pType, *pMemory, size);
        assert(pMemoryBlock);
        pMemoryBlock->AddRef();
    }
}

void Memory::RegisterMemoryBlock(MemoryBlock *pMemoryBlock) {
    LOCK _(m_cs);
    try {
        m_memoryBlockRanges.insert(RANGE(pMemoryBlock));
    }
    catch (...) {}
}

void Memory::UnRegisterMemoryBlock(void* pMemory) {
    LOCK _(m_cs);
    try {
        m_memoryBlockRanges.erase(RANGE((ui32)pMemory));
    }
    catch (...) {}
}

MemoryBlock* Memory::QueryMemoryBlock(void *pMemory) {
    MemoryBlock *pMemoryBlock = 0;
    LOCK _(m_cs);
    try {
        set<RANGE>::iterator it = m_memoryBlockRanges.find(RANGE((ui32)pMemory));
        if (it != m_memoryBlockRanges.end()) {
            pMemoryBlock = it->m_pMemoryBlock;
            pMemoryBlock->AddRef();
        }
    }
    catch (...) {}
    
    return pMemoryBlock;
}

MemoryBlockRef* Memory::AllocateWrapper(Type *pType, void *pMemory, ui32 size) {
    MemoryBlockRef *pMemoryBlockRef = new MemoryBlockRef(pType, pMemory, size);
    return pMemoryBlockRef;
}

MemoryBlock* Memory::AllocateMemoryBlockForExternal(Type *pType, void *pMemory, ui32 size) {
    if (ui8* pMem = new ui8[size + sizeof(MemoryBlock)]) {
        MemoryBlock *pMemoryBlock = new (pMem) MemoryBlock(pType, size, pMemory);
        pMemoryBlock->Register(Memory::GetInstance());
        return pMemoryBlock;
    }
    return 0;
}

template<class T> ui8* GetFooJmpPtr(T fn) {
    union TYPE_CAST {
        ui8 *p;
        T     v;
    } u;
    u.v = fn;
    return u.p;
}

void* Memory::AllocFunctionStub(FunctionCallbackPtr *pFunctionCallbackPtr) {
    HANDLE handle = GetCurrentProcess();
    static ui8 s_code[] = {
        0x55,//push ebp 
        0x8B, 0xEC,//mov ebp,esp
        0x68, 0x00, 0x00, 0x00, 0x00,//push 0x00000000, will set to pThis
        0x68, 0x00, 0x00, 0x00, 0x00,//push 0x00000000, will set to espAdjustment
                                     //store register we are going to use
        0x51,//push ecx
             //0x52,//push edx
        0x53, // push ebx
        0x56, // push esi
        0x57, // push edi
              // propagate parameters in reverse order
              // argument loop
        0xB8, 0x00, 0x00, 0x00, 0x00,//mov eax,0; i is stored in EAX, set to iArgCount
                                     //for (int i = iArgCount; i > 0; --i) {
        0xEB, 0x03,//jmp EIP + 2 + 03h                ->   
        0x83, 0xE8, 0x01,//sub eax,1                    |
        0x83, 0xF8, 0x00,//cmp eax,0                  <-   <- 
        0x7E, 0x0E,//jle EIP + XX function call section->    |
        0x8B, 0xC8,//mov ecx,eax                         |   |
        0xC1, 0xE1, 0x02,//shl ecx,4                     |   |
        0x03, 0xCD,//add ecx,ebp                         |   |
        0x83, 0xC1, 0x04,//add ecx,4                     |   |
        0xFF, 0x31,//push dword ptr[ecx]                 |   |
        0xEB, 0xEA,//jmp - 22                            | ->
                   //}                                   |
                   //function call a,b,c,d,e,f           |
        0x8B, 0x4D, 0xFC,//mov ecx,dword ptr[ebp - 4]  <-   get pThis
        0x8B, 0x11,//mov edx,dword ptr[ecx]                 get virtual table pointer
        0x8B, 0x42, 0x08,//mov eax,dword ptr[edx + 8]       function address is at offset (8 / sizeof(DWORD))
        0xFF, 0xD0,//call eax
                   // end of function call
        0x03, 0x65, 0xF8, // add esp, dword ptr[ebp - 8];
                          // restore registers
                          //0x5A,//pop edx
        0x5F, // pop edi
        0x5E, // pop esi
        0x5B, // pop ebx
        0x59,//pop ecx
        0x8B, 0xE5,//mov esp,ebp
        0x5D,//pop ebp
        0xC3,//ret
    };
    
    void *pExec = 0;
    {
        LOCK _(m_cs);
        try {
                                                                                        //align memory
            const ui32 cSizeOfSector = (sizeof(s_code) + sizeof(FUNCTION_PTR_HEADER) + (sizeof(void*) - 1)) & -(i32)sizeof(void*);

            assert(m_virtualMemoryNextOffset < (m_pRootScope->GetFunctionDefsCount() * cSizeOfSector));

            if (!m_pVirtualMemory) {
                const ui32 cSectionSize = m_pRootScope->GetFunctionDefsCount() * cSizeOfSector;
                m_pVirtualMemory = VirtualAllocEx(handle, 0, cSectionSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            }

            FUNCTION_PTR_HEADER *pFunctionPtrHeader = reinterpret_cast<FUNCTION_PTR_HEADER*>((ui32)m_pVirtualMemory + m_virtualMemoryNextOffset);
            pFunctionPtrHeader->Init(pFunctionCallbackPtr);
            pExec = pFunctionPtrHeader->m_pFunctionPtrStub;

            m_virtualMemoryNextOffset += cSizeOfSector;
        }
        catch (...) {}
    }

    if (WriteProcessMemory(handle, pExec, s_code, sizeof(s_code), 0)) {
        ui8 *code = static_cast<ui8*>(pExec);
        // set pThis
        ui32 pThis = (ui32)pFunctionCallbackPtr;
        code[4] = pThis & 0xff;
        code[5] = (pThis >> 8) & 0xff;
        code[6] = (pThis >> 16) & 0xff;
        code[7] = (pThis >> 24) & 0xff;

        ui32 argDWORDCount = pFunctionCallbackPtr->GetArgDWORDCount();
        ui32 espAdjustment = argDWORDCount * 4;
        code[9] = espAdjustment & 0xff;
        code[10] = (espAdjustment >> 8) & 0xff;
        code[11] = (espAdjustment >> 16) & 0xff;
        code[12] = (espAdjustment >> 24) & 0xff;
        code[11 + 2 + 5] = argDWORDCount & 0xff;
        code[12 + 2 + 5] = (argDWORDCount >> 8) & 0xff;
        code[13 + 2 + 5] = (argDWORDCount >> 16) & 0xff;
        code[14 + 2 + 5] = (argDWORDCount >> 24) & 0xff;

        ui8* ptr = GetFooJmpPtr(&FunctionCallbackPtr::Callback);
        ui8 offset = 0;
#ifdef _DEBUG
        if (ptr[0] == 0xE9) { // jmp offset -> 0xE9 XX XX XX XX
            ptr = ptr + *(ui32*)&ptr[1] + 5/* jmp command length in bytes*/;
        }
#endif // _DEBUG 
        if (*(ui32*)ptr == 0x60FF018B) { // mov eax,dword ptr [ecx] -> 8B 01; jmp dword ptr[eax+XXh] -> FF 60 XX 
            offset = ptr[4];
        }
        else {
            assert(0);
            // Callback offset is 4 * 12 !!!!
            offset = 12 * 4; // I am not sure if compiler does something else, so have hardcoded one in case
                             // but once function order is changed this must be changed as well !!
        }

        code[46 + 2 + 5] = offset;// fooOffset;

        if (!FlushInstructionCache(handle, pExec, sizeof(code))) {
            assert(0);
        }
    }
    else {
        assert(0);
    }

    return pExec;
}

/*************************************************************/

StackMemory::MemRefFrameSpot::MemRefFrameSpot(MemoryBlockRef *pMemoryBlockRef) :
    m_pMemoryBlockRef(pMemoryBlockRef), 
    m_pReferencedMemoryBlocks(0) {
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->AddRef();
    }
}

StackMemory::MemRefFrameSpot::MemRefFrameSpot(const MemRefFrameSpot& that) :
    m_pMemoryBlockRef(0), 
    m_pReferencedMemoryBlocks(0) {
    *this = that;
}

StackMemory::MemRefFrameSpot::~MemRefFrameSpot() {
    if (m_pReferencedMemoryBlocks) {
        assert(m_pReferencedMemoryBlocks->empty());
        for (vector<MemoryBlockRef*>::iterator it = m_pReferencedMemoryBlocks->begin(); it != m_pReferencedMemoryBlocks->end(); ++it) {
            (*it)->Release();
        }
        delete m_pReferencedMemoryBlocks;
    }
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->Release();
    }
}

void StackMemory::MemRefFrameSpot::PushReferencedMemoryBlockRef(Type *pType, ui32 offset, MemoryBlock *pMemoryBlock/*MemoryBlockRef *pMemoryBlockRef*/) {
    if (!m_pReferencedMemoryBlocks) {
        m_pReferencedMemoryBlocks = new vector<MemoryBlockRef*>();
    }
    m_pReferencedMemoryBlocks->push_back(Memory::Allocate(pType, offset, pMemoryBlock));
}

void StackMemory::MemRefFrameSpot::UpdateReferencedMemoryBlockRef(MemoryBlockRef *pMemoryBlockRef) {
    assert(m_pReferencedMemoryBlocks && (m_pReferencedMemoryBlocks->size() == 1));
    MemoryBlockRef *p = m_pReferencedMemoryBlocks->back();
    m_pReferencedMemoryBlocks->pop_back();
    pMemoryBlockRef->AddRef();
    m_pReferencedMemoryBlocks->push_back(pMemoryBlockRef);
    p->Release();
}

void StackMemory::MemRefFrameSpot::PopReferencedMemoryBlockRef() {
    if (m_pReferencedMemoryBlocks) {
        if (!m_pReferencedMemoryBlocks->empty()) {
            m_pReferencedMemoryBlocks->back()->Release();
            m_pReferencedMemoryBlocks->pop_back();
        }
    }
}

MemoryBlockRef* StackMemory::MemRefFrameSpot::GetReferencedMemoryBlockRef() {
    assert(!m_pReferencedMemoryBlocks->empty());
    return m_pReferencedMemoryBlocks->back();
}

StackMemory::MemRefFrameSpot& StackMemory::MemRefFrameSpot::operator=(const MemRefFrameSpot& that) {
    assert(that.m_pMemoryBlockRef);
    assert(m_pMemoryBlockRef == 0);

    if (m_pMemoryBlockRef = that.m_pMemoryBlockRef) {
        m_pMemoryBlockRef->AddRef();
    }
    m_pReferencedMemoryBlocks = that.m_pReferencedMemoryBlocks;
    const_cast<MemRefFrameSpot&>(that).m_pReferencedMemoryBlocks = 0;
    return *this;
}

StackMemory::MemoryFrame& StackMemory::MemoryFrame::operator=(const StackMemory::MemoryFrame& that) {
    m_pFunctionNode = that.m_pFunctionNode;
    m_pFunctionScope = that.m_pFunctionScope;
    m_pCurrentNode = that.m_pCurrentNode;
    assign(that.begin(), that.end());
    return *this;
}

/*************************************************************/

MemoryBlock::MemoryBlock(Type *pType, ui32 size) :
    m_pMemoryTypeNode(pType),
    m_refCount(1),
    m_size(size),
    m_flag(MEM_LOCAL),
    m_pMemory(&m_pMemory + 1) {
    memset(m_pMemory, 0, m_size);
#ifdef TRACE_MEMORY
    Memory::GetInstance().AddMemoryBlock(this);
#endif // TRACE_MEMORY
}
MemoryBlock::MemoryBlock(Type *pType, ui32 size, void *pExternal) :
    m_pMemoryTypeNode(pType),
    m_refCount(1),
    m_size(size),
    m_flag(MEM_EXTERNAL),
    m_pMemory(pExternal) {
#ifdef TRACE_MEMORY
    Memory::GetInstance().AddMemoryBlock(this);
#endif // TRACE_MEMORY
}

MemoryBlock::~MemoryBlock() {
    if (m_flag & MEM_REGISTERED) {
        Memory::GetInstance().UnRegisterMemoryBlock(m_pMemory);
    }

#ifdef TRACE_MEMORY
    Memory::GetInstance().ReleaseMemoryBlock(this);
#endif // TRACE_MEMORY
}

ui32 MemoryBlock::AddRef() {
    return ++m_refCount;
}

#ifdef DEL
bool MemoryBlock::Release() {
    if (!--m_refCount) {
        Clear(0, 0);
        delete this;
        return true;
    }
    return false;
}
#else // DEL
bool MemoryBlock::Release(ui32 offset) {
    if (!--m_refCount) {
        Clear(offset, true);
        (this->*m_fnDelete)();
        return true;
    }
    return false;
}
#endif // DEL

void MemoryBlock::Lock() {
    m_refCount |= 0x80000000;
}

void MemoryBlock::UnLock() {
    m_refCount &= 0x7fffffff;
}

bool MemoryBlock::IsLocked() {
    return (m_refCount & 0x80000000) != 0;
}

void MemoryBlock::Clear(ui32 offset, Type *pType) {
    Memory &memory = Memory::GetInstance();
    //if pType is not provided then it's destroy memory call
    InstanceHandler *pHandler;
    if (pType == 0) {
        pHandler = memory.GetDestroyMemoryHandler();
        pType = m_pMemoryTypeNode;
    }
    else {
        pHandler = memory.GetClearMemoryHandler();
    }

    pHandler->Run(false, pType, this, offset);
}

void MemoryBlock::Register(Memory &memory) {
    m_flag = (MEMORY_FLAG)(m_flag | MEM_REGISTERED);
    memory.RegisterMemoryBlock(this); 
}

/*************************************************************/
/*************************************************************/

MemoryBlockRef::MemoryBlockRef(Type *pType, ui32 offset, ui32 size) :
    m_refCount(1),
    m_offset(offset),
    m_pMemoryBlock(0),
    m_pReferenceTypeNode(pType) {
#ifdef TRACE_MEMORY
    Memory::GetInstance().AddMemoryBlockRef(this);
#endif // TRACE_MEMORY
    if (ui8* pMem = new ui8[size + sizeof(MemoryBlock)]) {
        m_pMemoryBlock = new (pMem) MemoryBlock(pType, size);
    }
}

MemoryBlockRef::MemoryBlockRef(Type *pType, void *pExternal, ui32 size) :
    m_refCount(1),
    m_offset(0),
    m_pMemoryBlock(0),
    m_pReferenceTypeNode(pType) {
#ifdef TRACE_MEMORY
    Memory::GetInstance().AddMemoryBlockRef(this);
#endif // TRACE_MEMORY
    m_pMemoryBlock = Memory::AllocateMemoryBlockForExternal(pType, pExternal, size);
}

MemoryBlockRef::MemoryBlockRef(Type *pType, ui32 offset, MemoryBlock *pMemoryBlock) :
    m_refCount(1),
    m_offset(offset),
    m_pMemoryBlock(pMemoryBlock),
    m_pReferenceTypeNode(pType) {
    assert(m_pMemoryBlock);
    m_pMemoryBlock->AddRef();
}

MemoryBlockRef::~MemoryBlockRef() {
    if (m_pMemoryBlock) {
#ifdef DEL
        m_pMemoryBlock->Release();
#else // DEL
        m_pMemoryBlock->Release(m_offset);
#endif // DEL
    }
#ifdef TRACE_MEMORY
    Memory::GetInstance().ReleaseMemoryBlockRef(this);
#endif // TRACE_MEMORY
}

ui32 MemoryBlockRef::AddRef() {
    return ++m_refCount;
}

void MemoryBlockRef::Release() {
    if (!--m_refCount) {
        delete this;
    }
}

void MemoryBlockRef::SetMemoryBlock(Type *pType, ui32 offset, MemoryBlock *pMemoryBlock) {
    m_pReferenceTypeNode = pType;
    pMemoryBlock->AddRef();
#ifdef DEL
    m_pMemoryBlock->Release();
#else // DEL
    m_pMemoryBlock->Release(m_offset);
#endif // DEL
    m_pMemoryBlock = pMemoryBlock;
    m_offset = offset;
}

/*************************************************************/

Runable::Param::Param(const Runable::Param& that) :
    m_pValue(that.m_pValue),
    m_pMemory(that.m_pMemory),
    m_pStackMemory(that.m_pStackMemory),
    m_pCurrentScope(that.m_pCurrentScope),
    m_pReferenceOfValueMemoryBlock(0),
    m_referenceOfValueOffset(0) {
}

Runable::Param::Param(Value *pValue, const Param& that) : 
    m_pValue(pValue),
    m_pMemory(that.m_pMemory),
    m_pStackMemory(that.m_pStackMemory),
    m_pCurrentScope(that.m_pCurrentScope),
    m_pReferenceOfValueMemoryBlock(0),
    m_referenceOfValueOffset(0) {

}

Runable::Param::Param(Value *pValue, Memory *pMemory, StackMemory *pStackMemory) :
    m_pValue(pValue),
    m_pMemory(pMemory),
    m_pStackMemory(pStackMemory),
    m_pCurrentScope(0),
    m_pReferenceOfValueMemoryBlock(0),
    m_referenceOfValueOffset(0) {
}

Runable::Param::~Param() {
    assert(!m_pReferenceOfValueMemoryBlock || (m_pReferenceOfValueMemoryBlock == GET_VALUE_REFERENCE));
}

Runable::Param& Runable::Param::operator=(const Runable::Param& that) {
    m_pValue        = that.m_pValue;
    m_pMemory       = that.m_pMemory;
    m_pStackMemory  = that.m_pStackMemory;
    m_pCurrentScope = that.m_pCurrentScope;
    return *this;
}

void Runable::Param::SetReferenceOfValue(MemoryBlock *pMemoryBlock, ui32 offset) {
    switch ((ui32)m_pReferenceOfValueMemoryBlock) {
        case 0:
            break;
        default:
            m_pReferenceOfValueMemoryBlock->Release();
        case (ui32)GET_VALUE_REFERENCE:
            m_pReferenceOfValueMemoryBlock = pMemoryBlock;
            m_pReferenceOfValueMemoryBlock->AddRef();
            m_referenceOfValueOffset = offset;
            break;
    }
}

void Runable::Param::ReleaseReferenceOfValue(bool final) {
    if (m_pReferenceOfValueMemoryBlock && (m_pReferenceOfValueMemoryBlock != GET_VALUE_REFERENCE)) {
        m_pReferenceOfValueMemoryBlock->Release();
        m_referenceOfValueOffset = 0;
        m_pReferenceOfValueMemoryBlock = final ? 0 : GET_VALUE_REFERENCE;
    }
}

bool Runable::Param::OverwriteReferenceOfValue(MemoryBlockRef *pRightMemoryBlockRef) {
    if (m_pReferenceOfValueMemoryBlock && (m_pReferenceOfValueMemoryBlock != GET_VALUE_REFERENCE)) {
        void **pMemory = m_pReferenceOfValueMemoryBlock->GetPtr<void*>(m_referenceOfValueOffset);
        if (!*pMemory) { // reference is NULL
            Type *pType = pRightMemoryBlockRef->GetReferenceTypeNode();
            ui32 size = pType->GetTypeSize();
            if (ui8* pMem = new ui8[size + sizeof(MemoryBlock)]) {
                MemoryBlock *pNewMemoryBlock = new(pMem) MemoryBlock(pType, size);
                *pMemory = *pNewMemoryBlock->GetRefToMemoryPtr();
                pNewMemoryBlock->Register(*m_pMemory);
                pNewMemoryBlock->AddRef();
            }
            else {
                assert(0); // no memory
            }
        }
        else { // overwrite reference
            // query for memory block
            if (MemoryBlock *pLeftReferencedMemoryBlock = m_pMemory->QueryMemoryBlock(*pMemory)) {
                pLeftReferencedMemoryBlock->Release(); // QueryMemoryBlock() adds reference
                pLeftReferencedMemoryBlock->Release(); // release as owner of this block
            } // else left memory is not registered, so just overwrite it
            MemoryBlock *pRightMemoryBlock = pRightMemoryBlockRef->GetMemoryBlock();
            *(ui32*)pMemory = (ui32)*pRightMemoryBlock->GetRefToMemoryPtr() + pRightMemoryBlockRef->GetOffset();

            if (!(MemoryBlock::MEM_REGISTERED & pRightMemoryBlock->GetMemoryFlag())) {
                pRightMemoryBlock->Register(*m_pMemory);
            }
            pRightMemoryBlock->AddRef();
        }
        return true;
    }
    else {
        return false;
    }
}

bool Runable::Param::PopulateGlobals() {
    if (m_pStackMemory->GetFrameIdx() == INVALID_STACK_FRAME_IDX) {
        //populate global vars
        m_pStackMemory->PopulateGlobals(m_pMemory->GetMainStackMemory());
        return true;
    }
    return false;
}

/*************************************************************/
/*************************************************************/
Runable::Runable(SYMBOL_DESC &symDesc, const NODE_TYPE type) : m_symDesc(symDesc), m_type(type) {
#ifdef TRACE_OBJECTS
    Memory::GetInstance().AddObject(this);
#endif // TRACE_OBJECTS
}

Runable::~Runable() {
#ifdef TRACE_OBJECTS
    Memory::GetInstance().ReleaseObject(this);
#endif // TRACE_OBJECTS
}

bool Runable::CanTypecast(NODE_TYPE typeFrom, NODE_TYPE typeTo) {
    bool ret = false;
    if (typeTo == typeFrom) {
        ret = true;
    }
    else {
        switch (typeFrom) {
            case TYPE_NONE:
                break;
            case TYPE_RUNABLE:
                ret = (typeTo == TYPE_NONE);
                break;
            case TYPE_SCOPE:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_AGGREGATE_TYPE_COLLECTION:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_PRE_POST_EXPRESSIONS:
            case TYPE_VAR:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_STRUCT:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_VAR:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_LIB_VAR:
            case TYPE_ERROR_VAR:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_VAR:
                    case TYPE_STRUCT:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_ERROR_TYPE:
            case TYPE_LIB:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_AGGREGATE_TYPE_COLLECTION:
                    case TYPE_AGGREGATE_TYPE:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_REF:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_AGGREGATE_TYPE:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_AGGREGATE_TYPE_COLLECTION:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_FUNCTION_REF_TYPE:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_AGGREGATE_TYPE_COLLECTION:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_ARRAY:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_AGGREGATE_TYPE_COLLECTION:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_FUNCTION:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_SIZEOF:
            case TYPE_ARR2STR:
            case TYPE_WARR2STR:
            case TYPE_STR2ARR:
            case TYPE_STR2WARR:
            case TYPE_LOCK:
            case TYPE_UNLOCK:
            case TYPE_CAST:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_FUNCTION:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_FUNCTION_CALL:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_FUNCTION_PTR:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_FUNCTION:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_FUNCTION_LIB_PTR:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_FUNCTION:
                    case TYPE_FUNCTION_PTR:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_FUNCTION_CALLBACK_PTR:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_CONST_BOOL:
            case TYPE_CONST:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_VAR:
                    case TYPE_CONST_BOOL:
                    case TYPE_CONST:
                    case TYPE_AGGREGATE_SYMBOL:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_REF_ASSIGN:
            case TYPE_ASSIGN:
            case TYPE_EQ:
            case TYPE_SUBSCRIPT:
            case TYPE_PARENTHESIS:
            case TYPE_MEMBER_ACCESS:
            case TYPE_OPERATOR:
            case TYPE_ADD:
            case TYPE_ADD_ASSIGN:
            case TYPE_INC_OP:
            case TYPE_PRE_INC_OP:
            case TYPE_POST_INC_OP:
            case TYPE_SUB:
            case TYPE_SUB_ASSIGN:
            case TYPE_DEC_OP:
            case TYPE_PRE_DEC_OP:
            case TYPE_POST_DEC_OP:
            case TYPE_AND:
            case TYPE_BIT_AND_ASSIGN:
            case TYPE_BIT_AND:
            case TYPE_OR:
            case TYPE_BIT_OR_ASSIGN:
            case TYPE_BIT_OR:
            case TYPE_LESS:
            case TYPE_LESS_EQ:
            case TYPE_SHIFT_LEFT:
            case TYPE_GREATER:
            case TYPE_GREATER_EQ:
            case TYPE_SHIFT_RIGHT:
            case TYPE_MUL:
            case TYPE_MUL_ASSIGN:
            case TYPE_DIV:
            case TYPE_DIV_ASSIGN:
            case TYPE_MOD:
            case TYPE_MOD_ASSIGN:
            case TYPE_BIT_NOT:
            case TYPE_NOT:
            case TYPE_NOT_EQ:
            case TYPE_XOR:
            case TYPE_XOR_ASSIGN:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_OPERATOR:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_IF:
            case TYPE_FOR:
            case TYPE_WHILE:
            case TYPE_SWITCH:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_CONDITIONAL:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_CASE:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_CONDITIONAL:
                    case TYPE_AGGREGATE_SYMBOL:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            case TYPE_BOOL:
            case TYPE_I8:
            case TYPE_UI8:
            case TYPE_I16:
            case TYPE_UI16:
            case TYPE_I32:
            case TYPE_UI32:
            case TYPE_I64:
            case TYPE_UI64:
            case TYPE_FLOAT:
            case TYPE_DOUBLE:
            case TYPE_STRING:
                switch (typeTo) {
                    case TYPE_NONE:
                    case TYPE_RUNABLE:
                    case TYPE_AGGREGATE_TYPE_COLLECTION:
                        ret = true;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    return ret;
}
/*
---------------------Operators Precedence------------------------
highest precedence appear at the top
-----------------------------------------------------------------
#   Category	    Operator	                        Associativity
-----------------------------------------------------------------
0   Postfix	        () [] -> . ++ --	                Left to right
1   Unary	        + - ! ~ ++ -- (type)* & sizeof	    Right to left
2   Multiplicative	* / %	                            Left to right
3   Additive	    + -	                                Left to right
4   Shift	        << >>	                            Left to right
5   Relational	    < <= > >=	                        Left to right
6   Equality	    == !=	                            Left to right
7   Bitwise         AND	&	                            Left to right
8   Bitwise         XOR	^	                            Left to right
9   Bitwise         OR	|	                            Left to right
10  Logical         AND	&&	                            Left to right
11  Logical         OR	||	                            Left to right
12  Conditional	    ?:	                                Right to left
13  Assignment	    = += -= *= /= %=>>= <<= &= ^= |=	Right to left
14  Comma	        ,	                                Left to right
-----------------------------------------------------------------
*/
PRECEDENCE Runable::GetPrecedence() {
    PRECEDENCE ret = PRECEDENCE_INVALID;

    switch (m_type) {
        case TYPE_POST_INC_OP://?
        case TYPE_POST_DEC_OP://?
            //break;
        case TYPE_PARENTHESIS:
        case TYPE_SUBSCRIPT:
        case TYPE_MEMBER_ACCESS:
        case TYPE_FUNCTION_CALL:
            ret = 0;
            break;
        case TYPE_PRE_INC_OP:
        case TYPE_PRE_DEC_OP:
        case TYPE_NOT:
        case TYPE_BIT_NOT:
            ret = 1;
            break;
        case TYPE_MUL:
        case TYPE_DIV:
        case TYPE_MOD:
            ret = 2;
            break;
        case TYPE_ADD:
        case TYPE_SUB:
            ret = 3;
            break;
        case TYPE_SHIFT_LEFT:
        case TYPE_SHIFT_RIGHT:
            ret = 4;
            break;
        case TYPE_LESS:
        case TYPE_LESS_EQ:
        case TYPE_GREATER:
        case TYPE_GREATER_EQ:
            ret = 5;
            break;
        case TYPE_EQ:
        case TYPE_NOT_EQ:
            ret = 6;
            break;
        case TYPE_BIT_AND:
            ret = 7;
            break;
        case TYPE_XOR:
            ret = 8;
            break;
        case TYPE_BIT_OR:
            ret = 9;
            break;
        case TYPE_AND:
            ret = 10;
            break;
        case TYPE_OR:
            ret = 11;
            break;
        case TYPE_REF_ASSIGN:
        case TYPE_ASSIGN:
        case TYPE_ADD_ASSIGN:
        case TYPE_SUB_ASSIGN:
        case TYPE_MUL_ASSIGN:
        case TYPE_DIV_ASSIGN:
        case TYPE_MOD_ASSIGN:
        case TYPE_SHIFT_RIGHT_ASSIGN:
        case TYPE_SHIFT_LEFT_ASSIGN:
        case TYPE_BIT_AND_ASSIGN:
        case TYPE_XOR_ASSIGN:
        case TYPE_BIT_OR_ASSIGN:
            ret = 13;
            break;

        case TYPE_NONE:
        case TYPE_RUNABLE:
        case TYPE_SCOPE:
        case TYPE_VAR:
        case TYPE_CONST_BOOL:
        case TYPE_CONST:
        case TYPE_OPERATOR:
        case TYPE_INC_OP:
        case TYPE_DEC_OP:
        //case TYPE_BIT_NOT_ASSIGN:// invalid
        default:
            break;
    }
    return ret;
}

Runable::ASSOCIATIVITY Runable::GetAssociativity() {
    ASSOCIATIVITY a = ASSOCIATIVITY_INVALID;
    switch (m_type) {
        case TYPE_POST_INC_OP:
        case TYPE_POST_DEC_OP:
        case TYPE_PARENTHESIS:
        case TYPE_SUBSCRIPT:
        case TYPE_MEMBER_ACCESS:
        case TYPE_FUNCTION_CALL:
        case TYPE_MUL:
        case TYPE_DIV:
        case TYPE_MOD:
        case TYPE_ADD:
        case TYPE_SUB:
        case TYPE_SHIFT_LEFT:
        case TYPE_SHIFT_RIGHT:
        case TYPE_LESS:
        case TYPE_LESS_EQ:
        case TYPE_GREATER:
        case TYPE_GREATER_EQ:
        case TYPE_EQ:
        case TYPE_NOT_EQ:
        case TYPE_BIT_AND:
        case TYPE_XOR:
        case TYPE_BIT_OR:
        case TYPE_AND:
        case TYPE_OR:
            a = LEFT_TO_RIGHT;
            break;
        case TYPE_PRE_INC_OP:
        case TYPE_PRE_DEC_OP:
        case TYPE_NOT:
        case TYPE_BIT_NOT:
        case TYPE_REF_ASSIGN:
        case TYPE_ASSIGN:
        case TYPE_ADD_ASSIGN:
        case TYPE_SUB_ASSIGN:
        case TYPE_MUL_ASSIGN:
        case TYPE_DIV_ASSIGN:
        case TYPE_MOD_ASSIGN:
        case TYPE_SHIFT_RIGHT_ASSIGN:
        case TYPE_SHIFT_LEFT_ASSIGN:
        case TYPE_BIT_AND_ASSIGN:
        case TYPE_XOR_ASSIGN:
        case TYPE_BIT_OR_ASSIGN:
            a = RIGHT_TO_LEFT;
            break;

        //case TYPE_NONE:
        //case TYPE_RUNABLE:
        //case TYPE_SCOPE:
        //case TYPE_VAR:
        //case TYPE_CONST_BOOL:
        //case TYPE_CONST:
        //case TYPE_OPERATOR:
        //case TYPE_INC_OP:
        //case TYPE_DEC_OP:
        //case TYPE_BIT_NOT_ASSIGN:// invalid
        default:
            break;
    }
    return a;
}

bool Runable::IsValue(NODE_TYPE type) {
    switch (type) {
        case TYPE_VAR:
            //case TYPE_STRUCT:
            //case TYPE_LIB_VAR:
            //case TYPE_PARENTHESIS:
        case TYPE_ERROR_VAR:
        case TYPE_CONST_BOOL:
        case TYPE_CONST:
        case TYPE_CONST_NUM:
            //case TYPE_MEMBER:
        case TYPE_FUNCTION_CALL:
        case TYPE_SUBSCRIPT:
            return true;
        default:
            return false;
    }
}

bool Runable::IsOperator(NODE_TYPE type) {
    return (type > TYPE_OPERATOR) && (type < TYPE_SUBSCRIPT);
};

bool Runable::IsExpression(Type *&pReturnNode, PrePostExpressions *&pPrePostExp, Error &error) {
    _ERROR(INTERNAL_ERROR);
    return false;
}

void Runable::Serialize(Serializer *pSerializer) {
    if (pSerializer) {
        SerializeBegin(pSerializer);
        SerializeBody(pSerializer);
        SerializeEnd(pSerializer);
    }
}

void Runable::SerializeBegin(Serializer *pSerializer) {
    assert(pSerializer);
    pSerializer->Begin((SYMBOL_ID_BLANK == m_symDesc.m_symId) ? TYPE_BLANK_SCOPE : m_type);
}

void Runable::SerializeBody(Serializer *pSerializer) {
    assert(pSerializer);
    pSerializer->WriteSymbol(m_symDesc);
}

void Runable::SerializeEnd(Serializer *pSerializer) {
    assert(pSerializer);
    pSerializer->End((SYMBOL_ID_BLANK == m_symDesc.m_symId) ? TYPE_BLANK_SCOPE : m_type);
}

/*************************************************************/

Node::Node(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type = TYPE_NONE) :
    Runable(symDesc, type),
    m_pParent(pParent),
    m_pLeft(0),
    m_pRight(0) {
}

Node::Node(Node* pParent, const NODE_TYPE type) :
    Runable(SYMBOL_DESC(), type),
    m_pParent(pParent),
    m_pLeft(0),
    m_pRight(0) {
}

Node::~Node() {
    if (m_pLeft)
        delete m_pLeft;
    if (m_pRight)
        delete m_pRight;
};

void Node::SerializeBody(Serializer *pSerializer) {
    if (m_pLeft) {
        m_pLeft->Serialize(pSerializer);
    }
    pSerializer->WriteSymbol(m_symDesc);
    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }
}

Node* Node::GetExpressionRoot() {
    Node *pNode = this,
         *pExpNode = 0;
    while (pNode) {
        NODE_TYPE type = pNode->GetType();
        if ((type == TYPE_ERROR_VAR) && (pNode->GetParent()->GetType() == TYPE_FUNCTION_CALL)) {
            // this is error expression:
            // error(e) {}
            return pNode;
        }
        else if (pNode->GetParent() && 
            (type != TYPE_ERROR_VAR) &&
            ((pNode->GetParent()->GetType() == TYPE_FUNCTION_CALL) || (pNode->GetParent()->GetType() == TYPE_SUBSCRIPT)) &&
            (pNode->GetParent()->GetRight() != pNode)) {
                return pNode;
        }
        else if (Runable::IsOperator(type)) {
            pExpNode = pNode;
        }
        else if (Runable::IsValue(type)) {
            pExpNode = pNode;
        }
        else {
            switch (type) {
                case TYPE_SUBSCRIPT:
                    if (pNode->GetSymbolId() == SYMBOL_ID_OPENSUBSCRIPT) {
                        // done
                        pNode = 0;
                        break;
                    }
                case TYPE_PARENTHESIS:
                case TYPE_RETURN:
                case TYPE_LIB_VAR:
                case TYPE_MEMBER_ACCESS:
                    pExpNode = pNode;
                    break;
                case TYPE_FUNCTION_CALL:
                    pExpNode = pNode;
                    break;
                case TYPE_SCOPE:
                case TYPE_IF:
                case TYPE_WHILE:
                case TYPE_FOR:
                case TYPE_SWITCH:
                case TYPE_BREAK:
                case TYPE_DEFAULT:
                case TYPE_BOOL:
                case TYPE_I8:
                case TYPE_UI8:
                case TYPE_I16:
                case TYPE_UI16:
                case TYPE_I32:
                case TYPE_UI32:
                case TYPE_I64:
                case TYPE_UI64:
                case TYPE_FLOAT:
                case TYPE_DOUBLE:
                case TYPE_STRING:
                case TYPE_LIB:
                case TYPE_AGGREGATE_TYPE:
                case TYPE_PRE_POST_EXPRESSIONS:
                    return pExpNode;
                default:
                    return pExpNode;
            }
        }
        pNode = pNode->GetParent();
    }

    return pExpNode;
}

Node* Node::GetParent(NODE_TYPE type) {
    Node* pParent = m_pParent;
    while (pParent) {
        if (type == pParent->GetType()) {
            break;
        }
        pParent = pParent->GetParent();
    }
    return pParent;
}

Function* Node::GetFunctionNode() {
    return static_cast<Function*>(GetParent(TYPE_FUNCTION));
}

Node* Node::GetBreakableNode() {
    Node* pNode = this;
    while (pNode) {
        switch (pNode->GetType()) {
            //case TYPE_SWITCH:
            case TYPE_FOR:
            case TYPE_WHILE:
            case TYPE_CASE:
            case TYPE_DEFAULT:
                return static_cast<BreakableNode*>(pNode);
            case TYPE_FUNCTION:
                return 0;
            default:
                pNode = pNode->GetParent();
                break;
        }
    }
    return 0;
}

Node* Node::GetLoopNode() {
    Node* pNode = this;
    while (pNode) {
        switch (pNode->GetType()) {
            case TYPE_FOR:
            case TYPE_WHILE:
                return pNode;
            case TYPE_FUNCTION:
                return 0;
            default:
                pNode = pNode->GetParent();
                break;
        }
    }
    return 0;
}
// compile time API!
Node* Node::GetRootValueNode() {
    Node *pNode = this, 
         *pRootValueNode = this;
    while (pNode) {
        if (pNode->GetType() == TYPE_ERROR_VAR) {
            pRootValueNode = pNode;
            break;
        }
        if (pNode->GetParent() && (pNode->GetParent()->GetType() == TYPE_FUNCTION_CALL) /*|| (pNode->GetParent()->GetType() == TYPE_SUBSCRIPT)*/) {
            if (pNode->GetParent()->GetRight() != pNode) {
                if (Variable *pVariable = pNode->QueryType<Variable>(TYPE_VAR)) {
                    pRootValueNode = pNode;
                }
                break;
            }
        }
        switch (pNode->GetType()) {
            case TYPE_PARENTHESIS:
                if (pNode->GetSymbolId() == SYMBOL_ID_OPENPARENTHESIS) {
                    pNode = 0;
                }
                else {
                    pRootValueNode = pNode;
                    pNode = pNode->GetParent();
                    assert(pNode);
                }
                break;
            case TYPE_SUBSCRIPT:
                if (pNode->GetSymbolId() == SYMBOL_ID_OPENSUBSCRIPT) {
                    pNode = 0;
                }
                else {
                    pRootValueNode = pNode;
                    pNode = pNode->GetParent();
                    assert(pNode);
                }
                break;
            case TYPE_CONST:
            case TYPE_CONST_BOOL:
            case TYPE_LIB_VAR:
                pRootValueNode = pNode;
                pNode = 0;
                break;
            case TYPE_VAR:
            case TYPE_FUNCTION_CALL:
            case TYPE_MEMBER_ACCESS:
                pRootValueNode = pNode;
                pNode = pNode->GetParent();
                assert(pNode);
                break;
            default:
                pNode = 0;
                break;
        }
    }

    return pRootValueNode;
}

bool Node::IsFunctionCall() {
    return (GetType() == TYPE_VAR) ?
           ((static_cast<Variable*>(this)->GetTypeNode()->GetType() == TYPE_FUNCTION_REF_TYPE) && GetRight()) :
           (GetType() == TYPE_FUNCTION_CALL);
}

bool Node::IsAssignOp() {
    switch (GetType()) {
        case TYPE_REF_ASSIGN:
        case TYPE_ASSIGN:
        case TYPE_ADD_ASSIGN:
        case TYPE_SUB_ASSIGN:
        case TYPE_BIT_OR_ASSIGN:
        case TYPE_BIT_AND_ASSIGN:
        case TYPE_MUL_ASSIGN:
        case TYPE_DIV_ASSIGN:
        case TYPE_XOR_ASSIGN:
        case TYPE_MOD_ASSIGN:
        case TYPE_SHIFT_LEFT_ASSIGN:
        case TYPE_SHIFT_RIGHT_ASSIGN:
            return true;
        default:
            return false;
    }
}

Variable* Node::FindVariableDec(SYMBOL_ID symId) {
    /*
    1) search local
    2) go to the upper scope and search again
    3) repeat up to the function scope
    4) go to the root scope and search again
    */

    Variable *pVariable = 0;
    Node *pNode = this;
    while (pNode) {
        switch (pNode->GetType()) {
            case TYPE_SCOPE:
                pVariable = static_cast<Scope*>(pNode)->FindTypeVariableDeclNode(symId);
                break;
            case TYPE_AGGREGATE_TYPE:
                pVariable = static_cast<AggregateType*>(pNode)->FindVariableDecNode(symId);
                break;
            case TYPE_FUNCTION: // stop local search and search global scope
                // since function definition can only be at the root scope, go to the root
                while (pNode->GetParent()) {
                    pNode = pNode->GetParent();
                }
                continue;
            default:
                break;
        }
        if (pVariable) {
            break;
        }
        pNode = pNode->GetParent();
    }

    return pVariable;

}

bool Node::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    assert(0);
    delete pPrePostExpressions;
    return false;
}

#ifdef SCRIPT_DEBUGGER
RUNTIME_RETURN Node::CheckFlow(Node *pNode, Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    if (pNode->GetType() != TYPE_SCOPE) {
        if (FlowSemaphor *pFlowSemaphor = param.m_pStackMemory->GetFlowSemaphor()) {
            param.m_pStackMemory->SetCurRunningNode(pNode);
            assert(param.m_pCurrentScope);
            ret = pFlowSemaphor->CheckFlow(pNode, param.m_pStackMemory->GetFrameIdx(), param.m_pCurrentScope->GetActiveBreakPointLines());
        }
    }
    return ret;
}
#endif // SCRIPT_DEBUGGER

/*************************************************************/
void InstanceHandler::Run(bool reference, Type *pType, MemoryBlock *pMemoryBlock, ui32 offset) { 
    if (reference) {
        MemoryBlock *pDerefMemoryBlock = pMemoryBlock;
        ui32 derefOffset = offset;
        m_pMemory->DereferenceMemory(pType, pDerefMemoryBlock, derefOffset);
        if (pDerefMemoryBlock) {
            pType->HandleInstance(this, pDerefMemoryBlock, derefOffset);
            pDerefMemoryBlock->Release();
        }
    }
    else {
        pType->HandleInstance(this, pMemoryBlock, offset);
    }
}

/*************************************************************/

void DestroyMemoryHandler::Run(bool reference, Type *pType, MemoryBlock *pMemoryBlock, ui32 offset) {
    if (reference) {
        MemoryBlock *pDerefMemoryBlock = pMemoryBlock;
        ui32 derefOffset = offset;
        m_pMemory->DereferenceMemory(pType, pDerefMemoryBlock, derefOffset); // adds reference
        if (pDerefMemoryBlock) {
            pDerefMemoryBlock->Release(); // release reference added by DereferenceMemory()
            pDerefMemoryBlock->Release(); // now release reference 
#ifdef SCRIPT_DEBUGGER
            *pMemoryBlock->GetPtr<void*>(offset) = (void*)0xDEADDEAD;
#endif // 
        }
    }
    else {
        void **pMemory = pMemoryBlock->GetPtr<void*>(offset);

        switch (pType->GetType()) {
            case TYPE_FUNCTION_REF_TYPE:
                if (pType->GetParent()->GetType() == TYPE_LIB) {
                    //case TYPE_LIB: // must unload library! revisit
                    Scope::UnRegisterFunctionPtr(*pMemory);
                }
                break;
            case TYPE_STRING:
                ((string*)pMemory)->string::~string();
                break;
            case TYPE_ARRAY:
            case TYPE_AGGREGATE_TYPE:
                pType->HandleInstance(this, pMemoryBlock, offset);
            default:
#ifdef SCRIPT_DEBUGGER
#ifdef MEMORY_ACCESS_EXCEPTION
                try {
                    if (!(MemoryBlock::MEM_EXTERNAL & pMemoryBlock->GetMemoryFlag())) {
                        memset(pMemory, 0xDD, pType->GetTypeSize());
                    }
                } catch (SE_Exception &e) {
                              //param.m_pStackMemory->GetRunTimeError().SetError("MEMORY_ACCESS_EXCEPTION!", m_symDesc, e.getSeNumber(), param.m_pStackMemory);
                              //ret = RT_ERROR;
                }
#else MEMORY_ACCESS_EXCEPTION
                if (!(MemoryBlock::MEM_EXTERNAL & pMemoryBlock->GetMemoryFlag())) {
                    memset(pMemory, 0xDD, pType->GetTypeSize());
                }
#endif // MEMORY_ACCESS_EXCEPTION
#endif // SCRIPT_DEBUGGER
                break;
        }
    }
}

/*************************************************************/

void ClearMemoryHandler::Run(bool reference, Type *pType, MemoryBlock *pMemoryBlock, ui32 offset) {
    if (reference) {
        MemoryBlock *pDerefMemoryBlock = pMemoryBlock;
        ui32 derefOffset = offset;
        m_pMemory->DereferenceMemory(pType, pDerefMemoryBlock, derefOffset); // adds reference
        if (pDerefMemoryBlock) {
            pType->HandleInstance(this, pDerefMemoryBlock, derefOffset);
            pDerefMemoryBlock->Release(); // release reference added by DereferenceMemory()
        }
    }
    else {
        void **pMemory = pMemoryBlock->GetPtr<void*>(offset);

        switch (pType->GetType()) {
            case TYPE_STRING:
                ((string*)pMemory)->clear();
                break;
            case TYPE_ARRAY:
            case TYPE_AGGREGATE_TYPE:
                pType->HandleInstance(this, pMemoryBlock, offset);
                break;
            default:
#ifdef MEMORY_ACCESS_EXCEPTION
                try {
                    if (!(MemoryBlock::MEM_EXTERNAL & pMemoryBlock->GetMemoryFlag())) {
                        memset(pMemory, 0x00, pType->GetTypeSize());
                    }
                }
                catch (SE_Exception &e) {
                    //param.m_pStackMemory->GetRunTimeError().SetError("MEMORY_ACCESS_EXCEPTION!", m_symDesc, e.getSeNumber(), param.m_pStackMemory);
                    //ret = RT_ERROR;
                }
#else MEMORY_ACCESS_EXCEPTION
                if (!(MemoryBlock::MEM_EXTERNAL & pMemoryBlock->GetMemoryFlag())) {
                    memset(pMemory, 0x00, pType->GetTypeSize());
                }
#endif // MEMORY_ACCESS_EXCEPTION
                break;
        }
    }
}

/*************************************************************/


Type::~Type() {
    Delete(m_variables);
}

RUNTIME_RETURN Type::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;

    for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
        Value tempValue;
        Param tempParam(&tempValue, param);
        ret = (*it)->Run(tempParam);
        if (ret != RT_NO_ERROR) {
            break;
        }
    }
    return ret;
}

bool Type::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    _ERROR(INTERNAL_ERROR);
    return false;
}

bool Type::IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath) {
    if (GetType() == TYPE_STRING) {
        SYMBOL_DESC symDesc;
        GetSymbolDesc(symDesc);
        symbolPath.push_back(symDesc);
        _ERROR(STRING_IS_NOT_SAFE_EXTERNAL_TYPE);
        return false;
    }
    return true;
}

void Type::PushVariable(Variable* pVariable) {
    m_variables.push_back(pVariable);
}

bool Type::UpdateExpression(Node* pNode) {
    if (m_variables.size()) {
        m_variables.pop_back();
        m_variables.push_back(pNode);
        return true;
    }
    return false;
}

Variable* Type::FindVariableDecNode(SYMBOL_ID symId) {
    for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
        Node *pNode = (*it);
        if (pNode->GetType() == TYPE_PRE_POST_EXPRESSIONS) {
            pNode = static_cast<PrePostExpressions*>(pNode)->GetExpressionNode();
        }
        if (pNode->IsAssignOp()) {
            if (pNode->GetLeft()->GetSymbolId() == symId) {
                return static_cast<Variable*>(pNode->GetLeft());
            }
        }
        else if (pNode->GetSymbolId() == symId) {
            return static_cast<Variable*>(pNode);
        }
    }
    return 0;
}

Variable* Type::GetVariable(ui32 idx) {
    return static_cast<Variable*>((m_variables.size() > idx) ? m_variables[idx] : 0);
}

Node* Type::GetLastDecVariable() {
    return m_variables.size() ? *m_variables.rbegin() : 0;
}

bool Type::Align(AGGREGATE_TYPE_OFFSET baseOffset) {
    for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
        Variable *pVariable = (*it)->QueryType<Variable>(TYPE_RUNABLE);
        if (pVariable && pVariable->SetAlignedMemberOffset(baseOffset)) {
            baseOffset = pVariable->GetEndOfAlignedMemberOffset();
        }
        else {
            return false;
        }
    }
    return true;
}

bool Type::IsReference() {
    bool reference = (GetPassBy() != BY_VALUE) && ((GetParent()->GetType() == TYPE_AGGREGATE_TYPE) || (GetParent()->GetType() == TYPE_ARRAY));
    return reference;
}

ui32 Type::GetTypeSize() {
    ui32 size = 0;
    switch (GetType()) {
        case TYPE_BOOL:
        case TYPE_I8:
        case TYPE_UI8:
            size = 1;
            break;
        case TYPE_I16:
        case TYPE_UI16:
            size = 2;
            break;
        case TYPE_I32:
        case TYPE_UI32:
        case TYPE_FLOAT:
        case TYPE_LIB:
        case TYPE_FUNCTION_REF_TYPE:
            size = 4;
            break;
        case TYPE_I64:
        case TYPE_UI64:
        case TYPE_DOUBLE: // 8 bytes
            size = 8;
            break;
        case TYPE_STRING:
            size = sizeof(std::string);
            break;
        default:
            assert(0);
            break;
    }
    return size;
};

ui32 Type::GetValueSize() {
    return IsReference() ? sizeof(void*) : GetTypeSize();
}

ui32 Type::GetAlignment() {
    ui32 alignment = 1;

    if (GetPassBy() != BY_VALUE) {
        return 4;
    }

    switch (GetType()) {
        case TYPE_BOOL:
        case TYPE_I8:
        case TYPE_UI8:
            alignment = 1;
            break;
        case TYPE_I16:
        case TYPE_UI16:
            alignment = 2;
            break;
        case TYPE_I32:
        case TYPE_UI32:
        case TYPE_STRING:
        case TYPE_FLOAT:
        case TYPE_LIB:
        case TYPE_FUNCTION_REF_TYPE:
            alignment = 4;
            break;
        case TYPE_I64:
        case TYPE_UI64:
        case TYPE_DOUBLE: // 8 bytes
            alignment = 8;
            break;
        default:
            assert(0);
            break;
    }
    return alignment;
};

Value::VALUE_TYPE Type::GetValueType() {
    Value::VALUE_TYPE valueType;
    switch (GetType()) {
        case TYPE_BOOL:
        //case TYPE_CONST_BOOL:
            valueType = Value::BOOL_TYPE;
            break;
        case TYPE_I8:
            valueType = Value::I8_TYPE;
            break;
        case TYPE_UI8:
            valueType = Value::UI8_TYPE;
            break;
        case TYPE_I16:
            valueType = Value::I16_TYPE;
            break;
        case TYPE_UI16:
            valueType = Value::UI16_TYPE;
            break;
        case TYPE_I32:
            valueType = Value::I32_TYPE;
            break;
        case TYPE_FUNCTION_REF_TYPE:
            valueType = Value::FUNCTION_REF_TYPE;
            break;
        case TYPE_UI32:
            valueType = Value::UI32_TYPE;
            break;
        case TYPE_I64:
            valueType = Value::I64_TYPE;
            break;
        case TYPE_UI64:
            valueType = Value::UI64_TYPE;
            break;
        case TYPE_FLOAT:
            valueType = Value::FLOAT_TYPE;
            break;
        case TYPE_DOUBLE:
            valueType = Value::DOUBLE_TYPE;
            break;
        case TYPE_STRING:
            valueType = Value::STRING_TYPE;
            break;
        case TYPE_LIB:
            valueType = Value::LIB_TYPE;
            break;
        case TYPE_AGGREGATE_TYPE:
        case TYPE_ERROR_TYPE:
            valueType = Value::AGGREGATE_TYPE;
            break;
        case TYPE_ARRAY:
            valueType = Value::ARRAY_TYPE;
            break;
        default:
            assert(0);
            break;
    }

    return valueType;
}

NODE_TYPE Type::ValueTypeToNodeType(Value::VALUE_TYPE type) {
    NODE_TYPE nodeType;
    switch (type) {
        case Value::BOOL_TYPE:
            nodeType = TYPE_BOOL;
            break;
        case Value::I8_TYPE:
            nodeType = TYPE_I8;
            break;
        case Value::UI8_TYPE:
            nodeType = TYPE_UI8;
            break;
        case Value::I16_TYPE:
            nodeType = TYPE_I16;
            break;
        case Value::UI16_TYPE:
            nodeType = TYPE_UI16;
            break;
        case Value::I32_TYPE:
            nodeType = TYPE_I32;
            break;
        //case Value::FUNCTION_REF_TYPE:
        //    nodeType = TYPE_FUNCTION_REF_TYPE;
        //    break;
        case Value::UI32_TYPE:
            nodeType = TYPE_UI32;
            break;
        case Value::NULL_TYPE:
        case Value::I64_TYPE:
            nodeType = TYPE_I64;
            break;
        case Value::UI64_TYPE:
            nodeType = TYPE_UI64;
            break;
        case Value::FLOAT_TYPE:
            nodeType = TYPE_FLOAT;
            break;
        case Value::DOUBLE_TYPE:
            nodeType = TYPE_DOUBLE;
            break;
        case Value::STRING_TYPE:
            nodeType = TYPE_STRING;
            break;
        case Value::AGGREGATE_TYPE:
            nodeType = TYPE_AGGREGATE_TYPE;
            break;
        case Value::LIB_TYPE:
            nodeType = TYPE_LIB;
            break;
        default:
            assert(0);
            nodeType = TYPE_NONE;
            break;
    }
    return nodeType;
}

void Type::CopyVariableMemory(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset, ui32 variablemAlignedMemberOffset, bool reference) {
    assert(pDestMemoryBlock && pSrcMemoryBlock);

    destOffset += variablemAlignedMemberOffset;
    srcOffset += variablemAlignedMemberOffset;

    CopyMemoryType(memory, pDestMemoryBlock, destOffset, pSrcMemoryBlock, srcOffset, reference);
}

void Type::CopyMemoryType(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset, bool reference) {
    void **pDestMemory = 0, 
         **pSrcMemory = 0;
    MemoryBlockRef *pNewDestMemoryBlockRef = 0;
    MemoryBlock *pDereferencedDestMemoryBlock = 0,
                *pDereferencedSrcMemoryBlock = 0;
    ui32 dereferencedDestOffset = destOffset,
         dereferencedSrcOffset = srcOffset;

    if (reference) {
        
        pDestMemory = pDestMemoryBlock->GetPtr<void*>(destOffset);
        pSrcMemory = pSrcMemoryBlock->GetPtr<void*>(srcOffset);

        if (!*pDestMemory && !*pSrcMemory) {
            return;
        }
        
        pDereferencedDestMemoryBlock = memory.GetReferencedMemoryBlock(this, pDestMemoryBlock, dereferencedDestOffset);// GetTypeSize());
        
        assert(pDereferencedDestMemoryBlock);
        
        if (!*pSrcMemory) {
            // clear destination memory
            pDereferencedDestMemoryBlock->Clear(dereferencedDestOffset, this);
            pDereferencedDestMemoryBlock->Release();
            return;
        }

        // copy all
        pDereferencedSrcMemoryBlock = memory.GetReferencedMemoryBlock(this, pSrcMemoryBlock, dereferencedSrcOffset);// , GetTypeSize());

        assert(pDereferencedSrcMemoryBlock);

        CopySubValues(memory, pDereferencedDestMemoryBlock, dereferencedDestOffset, pDereferencedSrcMemoryBlock, dereferencedSrcOffset);
        pDereferencedDestMemoryBlock->Release();
        pDereferencedSrcMemoryBlock->Release();
    }
    else {
        CopySubValues(memory, pDestMemoryBlock, destOffset, pSrcMemoryBlock, srcOffset);
    }
}

void Type::CopySubValues(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset) {
    ui32 size = GetTypeSize();
    memcpy(pDestMemoryBlock->GetPtr(destOffset), pSrcMemoryBlock->GetPtr(srcOffset), size);
}

void Type::CopyVariables(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset) {
    Variable *pVariable = 0;
    TODO("Use IsReference()!");
    bool reference = (GetPassBy() != Type::BY_VALUE) && (GetParent()->GetType() == TYPE_AGGREGATE_TYPE);

    for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
        Node *pNode = (*it);
        pVariable = static_cast<Variable*>(pNode);
        CopyVariableMemory(memory, pDestMemoryBlock, destOffset, pSrcMemoryBlock, srcOffset, pVariable->GetAlignedMemberOffset(), reference);
    }
}

const char * const Type::GetSubPath(const char * path, ui32 &idx) {
    idx = 0;
    while (*path) {
        if (*path == '_') {
            ++path;
            break;
        }
        idx = idx * 10 + (*path - '0');
        ++path;
    }
    return path;
}

MemoryBlock* Type::GetDerefMemoryBlock(MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem) {
    void **pMemory = pMemoryBlock->GetPtr<void*>(offset);
    MemoryBlockRef * pDerefMemoryBlockRef = 0;
    if (*pMemory) {
        MemoryBlock *pDerefMemoryBlock = mem.QueryMemoryBlock(*pMemory);
        assert(pDerefMemoryBlock);
        offsetOut = (ui32)*pMemory - (ui32)*pDerefMemoryBlock->GetRefToMemoryPtr();
        return pDerefMemoryBlock;
    }
    return 0;
}

MemoryBlock* Type::GetMemoryBlock(MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem) {
    if (IsReference()) {
        offsetOut = offset;
        MemoryBlock *pDerefMemoryBlock = pMemoryBlock;
        Memory::GetInstance().DereferenceMemory(this, pDerefMemoryBlock, offsetOut);
        //MemoryBlock *pDerefMemoryBlock = GetDerefMemoryBlock(pMemoryBlock, offset, offsetOut, mem);
        return pDerefMemoryBlock;
    }
    else {
        offsetOut = offset;
        pMemoryBlock->AddRef();
        return pMemoryBlock;
    }
}

MemoryBlock* Type::FindMemoryBlock(const char * const path, MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem) {
    assert(*path == 0);

    MemoryBlock *pRealMemoryBlock = GetMemoryBlock(pMemoryBlock, offset, offsetOut, mem);

    return pRealMemoryBlock;
}

void Type::HandleEachInstance(bool reference, InstanceHandler *pInstanceHandler, MemoryBlock *pMemoryBlock, ui32 offset) {
    Variable *pVariable = 0;

    for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
        Node *pNode = (*it);
        if (pNode->GetType() == TYPE_PRE_POST_EXPRESSIONS) {
            pNode = static_cast<PrePostExpressions*>(pNode)->GetExpressionNode();
        }
        if (pNode->IsAssignOp()) {
            pVariable = static_cast<Variable*>(pNode->GetLeft());
        }
        else {
            pVariable = static_cast<Variable*>(pNode);
        }

        assert(pVariable);

        ui32 varOffset = offset + pVariable->GetAlignedMemberOffset();

        pInstanceHandler->Run(reference, this, pMemoryBlock, varOffset);
    }
}

void Type::HandleInstance(InstanceHandler *pInstanceHandler, MemoryBlock *pMemoryBlock, ui32 offset) {
}

ui32 Type::GetVarOffset(ui32 idx) {

    assert(idx < m_variables.size());

    Variable *pVariable = 0;
    Node *pNode = m_variables[idx];

    if (pNode->GetType() == TYPE_PRE_POST_EXPRESSIONS) {
        pNode = static_cast<PrePostExpressions*>(pNode)->GetExpressionNode();
    }
    if (pNode->IsAssignOp()) {
        pVariable = static_cast<Variable*>(pNode->GetLeft());
    }
    else {
        pVariable = static_cast<Variable*>(pNode);
    }

    return pVariable->GetAlignedMemberOffset();
}

bool Type::IsRelatedType(Type *pType) {
    NODE_TYPE type = pType->GetType();
    if ((GetType() >= TYPE_BOOL) && (GetType() <= TYPE_STRING) && (type >= TYPE_BOOL) && (type <= TYPE_STRING)) {
        return true;
    }
    return false;
}

bool Type::IsEqualType(Type *pType) {
    switch (GetType()) {
        case TYPE_I8:
        case TYPE_UI8:
            switch (pType->GetType()) {
                case TYPE_I8:
                case TYPE_UI8:
                    return true;
                default:
                    return false;
            }
        case TYPE_I16:
        case TYPE_UI16:
            switch (pType->GetType()) {
                case TYPE_I16:
                case TYPE_UI16:
                    return true;
                default:
                    return false;
            }
        case TYPE_I32:
        case TYPE_UI32:
            switch (pType->GetType()) {
                case TYPE_I32:
                case TYPE_UI32:
                    return true;
                default:
                    return false;
            }
        case TYPE_I64:
        case TYPE_UI64:
            switch (pType->GetType()) {
                case TYPE_I64:
                case TYPE_UI64:
                    return true;
                default:
                    return false;
            }
        default:
            return GetType() == pType->GetType();
    }
}

Type::PASS_BY Type::GetPassBy() {
    if (GetRight() && (GetRight()->GetType() == TYPE_REF)) { // pass reference by reference
        return BY_REF_TO_REF;
    }
    else if (GetLeft() && (GetLeft()->GetType() == TYPE_REF)) { // pass by reference
        return BY_REF;
    }
    return BY_VALUE;
}

bool Type::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();
    for (ui32 i = 0; i < m_variables.size(); ++i) {
        if (m_variables[i] == pExpNode) {
            pPrePostExpressions->SetParent(this);
            pExpNode->SetParent(pPrePostExpressions);
            m_variables[i] = pPrePostExpressions;
            return true;
        }
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

string Type::GetTypeString(SymbolStore *pSymStore) {
    string typeName, ref;
    pSymStore->GetSymbolName(this, typeName);
    switch (GetPassBy()) {
        case Type::PASS_BY::BY_REF_TO_REF:
            ref = "@";
        case Type::PASS_BY::BY_REF:
            ref += "@";
            break;
        default:
            break;
    }
    return typeName + ref;
}

void Type::SerializeBody(Serializer *pSerializer) {
    if (m_pLeft) {
        m_pLeft->Serialize(pSerializer);
    }
    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }
    if (m_variables.size()) {
        pSerializer->Begin(TYPE_EMPTY);
    }
    for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
        (*it)->Serialize(pSerializer);
        if ((it + 1) != m_variables.end()) {
            pSerializer->Begin(TYPE_COMMA);
        }
    }
}
void Type::SerializeEnd(Serializer *pSerializer) {
    NODE_TYPE parentType = GetParent()->GetType();
    switch (parentType) {
        case TYPE_SCOPE:
        case TYPE_FUNCTION:
        case TYPE_ARRAY:
        case TYPE_FUNCTION_REF_TYPE:
            break;
        case TYPE_VAR: {
            Type *pParentType = static_cast<Variable*>(GetParent())->GetTypeNode();
            if (FunctionRefType *pFunctionRefType = pParentType->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
                if (pFunctionRefType->GetParent()->GetType() == TYPE_CAST) {
                    break;
                }
            }
        }
        default:
            pSerializer->End(TYPE_SCOPE_END_EXPRESSION);
            break;
    }
}

/*************************************************************/
AggregateType::AggregateType(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) :
    Type(pParent, symDesc, type),
    m_pTypeDefinition(0),
    m_alignedSize(0),
    m_alignment(1)
{
};

AggregateType::~AggregateType() {
    Delete(m_types);
}

RUNTIME_RETURN AggregateType::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = Type::Run(param);
    return ret;
}

void AggregateType::AddType(Type* pType) {
    m_types.push_back(pType);
}

Variable* AggregateType::FindMemberDecNode(SYMBOL_ID symId) {
    if (m_pTypeDefinition) {
        return m_pTypeDefinition->FindMemberDecNode(symId);
    }
    else {
        Variable* pVariable = 0;
        for (vector<Type*>::iterator it = m_types.begin(); !pVariable && (it != m_types.end()); ++it) {
            pVariable = (*it)->FindVariableDecNode(symId);
        }
        return pVariable;
    }
}

/*
padding = (align - (offset mod align)) mod align
aligned = offset + padding
        = offset + ((align - (offset mod align)) mod align)

        ---- OR ----

padding = (align - (offset & (align - 1))) & (align - 1)
        = (-offset & (align - 1))
aligned = (offset + (align - 1)) & ~(align - 1)
        = (offset + (align - 1)) & -align
*/
MEMORY_OFFSET AggregateType::GetNextAlignedOffset(NODE_TYPE type) {
    MEMORY_OFFSET offset = INVALID_MEMORY_OFFSET;
    for (vector<Type*>::reverse_iterator it = m_types.rbegin(); it != m_types.rend(); ++it) {
        if (AggregateType *pAggregateType = (*it)->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
            offset = pAggregateType->GetNextAlignedOffset(type);
            if (offset != INVALID_MEMORY_OFFSET) {
                break;
            }
        }
        else {
            if (Variable *pVariable = static_cast<Variable*>((*it)->GetLastDecVariable())) {
                offset = pVariable->GetEndOfAlignedOffset();
                if (offset != INVALID_MEMORY_OFFSET) {
                    MEMORY_OFFSET unalignedOffset;
                    switch (type) {
                        case TYPE_BOOL:
                        case TYPE_I8:
                        case TYPE_UI8: // 1 byte
                            //++offset;
                            break;
                        case TYPE_I16:
                        case TYPE_UI16: // 2 bytes
                            offset = offset + (offset & 0x1);
                            break;
                        case TYPE_STRING: // string should be char array internally!
                        case TYPE_I32:
                        case TYPE_UI32: // 4 bytes
                        case TYPE_FLOAT: // 0->+0; 1->+3; 2->+2; 3->+1; 4->+0;
                            unalignedOffset = (offset % 4);
                            offset = offset + (unalignedOffset ? (4 - unalignedOffset) : 0);
                            break;
                        case TYPE_VAR: // reserve 8 bytes to hold any possible value
                        case TYPE_I64:
                        case TYPE_UI64:
                        case TYPE_DOUBLE:// 8 bytes
                            unalignedOffset = (offset % 8);
                            offset = offset + (unalignedOffset ? (8 - unalignedOffset) : 0);
                            break;
                        case TYPE_LIB:
                        case TYPE_AGGREGATE_TYPE:
                        default:
                            assert(0);
                            break;
                    }
                }
                break;
            }
        }
    }
    return offset;
}

bool AggregateType::AlignStruct(AGGREGATE_TYPE_OFFSET baseOffset) {
    AGGREGATE_TYPE_OFFSET memberOffset = 0;
    m_alignment = 1;
    for (vector<Type*>::iterator it = m_types.begin(); it != m_types.end(); ++it) {
        if ((*it)->Align(baseOffset)) {
            ui32 alignment = (*it)->GetAlignment();
            if (m_alignment < alignment) {
                m_alignment = alignment;
            }
            if (Node *pNode = (*it)->GetLastDecVariable()) {
                if (Variable *pVariable = pNode->QueryType<Variable >(TYPE_VAR)) {
                    baseOffset = pVariable->GetEndOfAlignedMemberOffset();
                    if (baseOffset != INVALID_AGGREGATE_TYPE_OFFSET) {
                        continue;
                    }
                }
            }
            else if ((*it)->GetType() == TYPE_AGGREGATE_TYPE) { // this is a structure
                continue;
            }
        }
        return false;
    }
    m_alignedSize = (baseOffset + (-(i32)baseOffset & ((i32)m_alignment - 1)));
    return true;
}

ui32 AggregateType::GetTypeSize() {
    return (m_pTypeDefinition) ? m_pTypeDefinition->GetTypeSize() : m_alignedSize;
}

ui32 AggregateType::GetValueSize() {
    return IsReference() ? sizeof(void*) : GetTypeSize();
}

ui32 AggregateType::GetRawAlignment() {
    return m_alignment;
}

ui32 AggregateType::GetAlignment() {
    return (GetPassBy() == BY_VALUE) ? ((m_pTypeDefinition) ? m_pTypeDefinition->GetRawAlignment() : m_alignment) : 4;
}

bool AggregateType::IsRelatedType(Type *pType) {
    return IsEqualType(pType);
}

bool AggregateType::IsEqualType(Type *pType) {
    if (AggregateType *pAggregateType = pType->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
        vector<Type*> &thisTypes = m_pTypeDefinition ? m_pTypeDefinition->m_types : m_types,
                      &thatTypes = pAggregateType->m_pTypeDefinition ? pAggregateType->m_pTypeDefinition->m_types : pAggregateType->m_types;
        ui32 thisTypeCount = thisTypes.size(),
             thatTypeCount = thatTypes.size();
        if (thisTypeCount == thatTypeCount) {
            for (ui32 i = 0; i < thisTypeCount; ++i) {
                if (!thisTypes[i]->IsEqualType(thatTypes[i])) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

void AggregateType::SetTypeDefinition(AggregateType *pAggregateType) {
    assert(m_pTypeDefinition == 0);
    m_pTypeDefinition = pAggregateType;
}

ui32 AggregateType::GetSubTypeCount() {
    return m_pTypeDefinition ? m_pTypeDefinition->GetSubTypeCount() : m_types.size();
}

Type* AggregateType::GetSubType(ui32 idx) {
    return m_pTypeDefinition ? m_pTypeDefinition->GetSubType(idx) : ((m_types.size() > idx) ? m_types[idx] : 0);
}

void AggregateType::SerializeBegin(Serializer *pSerializer) {
    if (!m_pTypeDefinition) {
        Runable::SerializeBegin(pSerializer);
    }
    else {
        pSerializer->WriteSymbol(m_symDesc);
    }
}

void AggregateType::SerializeBody(Serializer *pSerializer) {
    if (!m_pTypeDefinition) {
        pSerializer->Begin(TYPE_EMPTY);
        pSerializer->WriteSymbol(m_symDesc);
        pSerializer->Begin(TYPE_EMPTY);
        pSerializer->Begin(TYPE_SCOPE);
        for (vector<Type*>::iterator it = m_types.begin(); it != m_types.end(); ++it) {
            (*it)->Serialize(pSerializer);
        }
        pSerializer->End(TYPE_SCOPE_NO_NEW_LINE);//TYPE_SCOPE);
    }
    Type::SerializeBody(pSerializer);
}

void AggregateType::CopySubValues(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset) {
    ui32 count = GetSubTypeCount();

    for (ui32 i = 0; i < count; ++i) {
        Type *pSubType = GetSubType(i);
        pSubType->CopyVariables(memory, pDestMemoryBlock, destOffset, pSrcMemoryBlock, srcOffset);
    }
}

MemoryBlock* AggregateType::FindMemoryBlock(const char * const path, MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem) {
    MemoryBlock *pRealMemoryBlock = GetMemoryBlock(pMemoryBlock, offset, offsetOut, mem);

    if (!*path || !pRealMemoryBlock) {
        return pRealMemoryBlock;
    }

    ui32 count = GetSubTypeCount();
    ui32 subTypeIdx = 0;
    const char * const subVarPath = GetSubPath(path, subTypeIdx);

    assert(*subVarPath);
    assert(subTypeIdx < count);
    
    Type *pSubType = GetSubType(subTypeIdx);

    ui32 varIdx = 0;
    const char * const subPath = GetSubPath(subVarPath, varIdx);
    ui32 varOffset = offsetOut + pSubType->GetVarOffset(varIdx);

    MemoryBlock *pOutMemoryBlock = 0;
    
    ui32 offsetSubPath = varOffset;
    pOutMemoryBlock = pSubType->FindMemoryBlock(subPath, pRealMemoryBlock, offsetSubPath, offsetOut, mem);
    pRealMemoryBlock->Release();

    return pOutMemoryBlock;
}

void AggregateType::HandleInstance(InstanceHandler *pInstanceHandler, MemoryBlock *pMemoryBlock, ui32 offset) {
    ui32 count = GetSubTypeCount();

    for (ui32 i = 0; i < count; ++i) {
        Type *pSubType = GetSubType(i);
        pSubType->HandleEachInstance(pSubType->IsReference(), pInstanceHandler, pMemoryBlock, offset);
    }
}

bool AggregateType::IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath) {
    ui32 count = GetSubTypeCount();

    for (ui32 i = 0; i < count; ++i) {
        Type *pSubType = GetSubType(i);
        if (!pSubType->IsSafeType(error, symbolPath)) {
            SYMBOL_DESC symDesc;
            GetSymbolDesc(symDesc);
            symbolPath.push_back(symDesc);
            return false;
        }
    }
    return true;
}

/*************************************************************/

LibType::LibType(Node *pParent, SYMBOL_DESC &symDesc) :
    AggregateType(pParent, symDesc, TYPE_LIB) {

}

LibType::~LibType() {
}

RUNTIME_RETURN LibType::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    //initialize all vars;
    for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
        Value value;
        Param _param(&value, param);
        ret = (*it)->Run(_param);
        if (ret != RT_NO_ERROR) {
            return ret;
        }
    }
    return ret;
}

bool LibType::IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath) {
    for (vector<Type*>::iterator it = m_types.begin(); it != m_types.end(); ++it) {
        if (!(*it)->IsSafeType(error, symbolPath)) {
            SYMBOL_DESC symDesc;
            GetSymbolDesc(symDesc);
            symbolPath.push_back(symDesc);
            return false;
        }
    }
    return true;
}

FunctionRefType* LibType::GetFunctionRefType(SYMBOL_ID symId) {
    for (vector<Type*>::iterator it = m_types.begin(); it != m_types.end(); ++it) {
        if (symId == (*it)->GetSymbolId()) {
            return (*it)->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
        }
    }
    return 0;
}

LibNode* LibType::GetLibNode() { 
    assert(m_variables.size() != 0);
    return static_cast<LibNode*>(*m_variables.begin()); 
};

void *LibType::GetModuleHandle() {
    if (m_variables.size()) {
        return static_cast<LibNode*>((*m_variables.begin()))->GetModuleHandle();
    }
    return 0;
}

void LibType::SerializeBegin(Serializer *pSerializer) {
    pSerializer->Begin(TYPE_LIB);
    pSerializer->Begin(TYPE_PARENTHESIS);
    pSerializer->WriteSymbol(m_symDesc);
    pSerializer->End(TYPE_PARENTHESIS);
}

void LibType::SerializeBody(Serializer *pSerializer) {
    pSerializer->Begin(TYPE_EMPTY);
    pSerializer->Begin(TYPE_SCOPE);
    for (vector<Type*>::iterator it = m_types.begin(); it != m_types.end(); ++it) {
        FunctionRefType *pFunctionRefType = static_cast<FunctionRefType*>(*it);
        ui32 i = 0;
        // serialize return type:
        Type *pType = pFunctionRefType->GetArgType(i);
        SerializeType(pSerializer, pType);

        // serialize function name:
        pSerializer->Begin(TYPE_EMPTY);
        SYMBOL_DESC symDesc;
        pFunctionRefType->GetSymbolDesc(symDesc);
        pSerializer->WriteSymbol(symDesc);
        pSerializer->Begin(TYPE_PARENTHESIS);

        // serialize argument types:
        ui32 argCount = pFunctionRefType->GetArgCount();
        while (i < argCount) {
            pType = pFunctionRefType->GetArgType(++i);
            SerializeType(pSerializer, pType);
            if (i != argCount) {
                pSerializer->End(TYPE_COMMA);
                pSerializer->Begin(TYPE_EMPTY);
            }
        }
        pSerializer->End(TYPE_PARENTHESIS);
        pSerializer->End(TYPE_SCOPE_END_EXPRESSION);
    }
    pSerializer->End(TYPE_SCOPE_NO_NEW_LINE);
    Type::SerializeBody(pSerializer);
}

void LibType::SerializeType(Serializer *pSerializer, Type *pType) {
    if (FunctionRefType *pRefType = pType->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
        pRefType->SerializeBegin(pSerializer);
    }
    else if (ArrayType *pArrayType = pType->QueryType<ArrayType>(TYPE_ARRAY)) {
        pArrayType->Serialize(pSerializer);
    }
    else {
        SYMBOL_DESC symDesc;
        pType->GetSymbolDesc(symDesc);
        pSerializer->WriteSymbol(symDesc);
    }

    if (pType->GetType() != TYPE_ARRAY) {
        if (pType->GetLeft()) {
            pType->GetLeft()->Serialize(pSerializer);
            if (pType->GetRight()) {
                pType->GetRight()->Serialize(pSerializer);
            }
        }
    }
}

/*************************************************************/
FunctionRefType::~FunctionRefType() {
    if (Node *pParent = GetParent()) {
        switch (pParent->GetType()) {
            case TYPE_LIB:
                for (vector<LOCAL_VAR_DESCRIPTOR>::iterator it = m_argDescs.begin(); it != m_argDescs.end(); ++it) {
                    delete (Type*)*it; // must typecast!
                }
                break;
            case TYPE_FUNCTION: // delete return type object!
            case TYPE_SIZEOF:
            case TYPE_ARR2STR:
            case TYPE_WARR2STR:
            case TYPE_STR2ARR:
            case TYPE_STR2WARR:
            case TYPE_CAST:
                if (m_argDescs.size()) {
                    delete (Type*)m_argDescs[0]; // must typecast!
                }
                break;
            case TYPE_LOCK:
            case TYPE_UNLOCK:
            default:
                break;
        }
    }
}

void FunctionRefType::SetReturnType(Type *pType) {
    assert(m_argDescs.size() == 0);
#ifdef SCRIPT_DEBUGGER
    m_argDescs.push_back(LOCAL_VAR_DESCRIPTOR(SYMBOL_DESC(),pType));
#else // SCRIPT_DEBUGGER
    m_argDescs.push_back(pType);
#endif // SCRIPT_DEBUGGER
}

void FunctionRefType::PushArgDesc(LOCAL_VAR_DESCRIPTOR &varDesc) {
    assert(m_argDescs.size());
    m_argDescs.push_back(varDesc);
}

Type* FunctionRefType::GetArgType(ui32 i) {
    if (i < m_argDescs.size()) {
        return m_argDescs[i];
    }
    return 0;
}

void FunctionRefType::PopulateTypes(Function *pFromFunction) {
    SetReturnType(pFromFunction->GetReturnType());
    vector<SYMBOL_DESC> &args = pFromFunction->GetArgumentVector();
    for (vector<SYMBOL_DESC>::iterator it = args.begin(); it != args.end(); ++it) {
#ifdef SCRIPT_DEBUGGER
        PushArgDesc(LOCAL_VAR_DESCRIPTOR(*it, pFromFunction->GetArgumentType((*it).m_symId)));
#else // SCRIPT_DEBUGGER
        Type *pType = pFromFunction->GetArgumentType((*it).m_symId);
        PushArgDesc(pType);
#endif // SCRIPT_DEBUGGER
    }
}

bool FunctionRefType::IsRelatedType(Type *pType) {
    return IsEqualType(pType);
}

bool FunctionRefType::IsEqualType(Type *pType) {
    if (FunctionRefType *pFunctionRefType = pType->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
        if (m_argDescs.size() == pFunctionRefType->m_argDescs.size()) {
            for (ui32 i = 0; i < m_argDescs.size(); ++i) {
                Type *pArgType = m_argDescs[i],
                     *pOutType = pFunctionRefType->m_argDescs[i];
                if (!pArgType) {
                    if (!pOutType) {
                        continue;
                    }
                    return false;
                }
                else if (!pOutType) {
                    return false;
                }
                else if (!pArgType->IsEqualType(pOutType)) {
                    return false;
                }
                else if (pArgType->GetPassBy() != pOutType->GetPassBy()) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

void FunctionRefType::SerializeBegin(Serializer *pSerializer) {
    Runable::SerializeBegin(pSerializer);
    pSerializer->Begin(TYPE_LESS);
    pSerializer->WriteSymbol(m_symDesc);
    pSerializer->End(TYPE_GREATER);
}

void FunctionRefType::GetArgDescs(vector<LOCAL_VAR_DESCRIPTOR> &argDescs) {
    for (vector<LOCAL_VAR_DESCRIPTOR>::iterator it = m_argDescs.begin() + 1; it != m_argDescs.end(); ++it) {
        argDescs.push_back(*it);
    }
}

string FunctionRefType::GetTypeString(SymbolStore *pSymStore) {
    string typeName, ref;
    pSymStore->GetSymbolName(this, typeName);
    switch (GetPassBy()) {
        case Type::PASS_BY::BY_REF_TO_REF:
            ref = "@";
        case Type::PASS_BY::BY_REF:
            ref += "@";
            break;
        default:
            break;
    }
    if (LibType *pLibType = GetParent()->QueryType<LibType>(TYPE_LIB)) {
        FunctionRefType *pFunctionRefType = static_cast<FunctionRefType*>(this);
        Type *pRetunrType = pFunctionRefType->GetReturnType();
        string retType = pRetunrType->GetTypeString(pSymStore);
        retType += " " + typeName + "(";
        ui32 argCount = pFunctionRefType->GetArgCount();
        for (ui32 i = 0; i < argCount; ++i) {
            Type *pArgType = pFunctionRefType->GetArgType(i + 1);
            retType += pArgType->GetTypeString(pSymStore);
            if (i + 1 != argCount) {
                retType += " ,";
            }
        }
        retType += ")";
        return retType;
    }
    else {
        return "function<" + typeName + ">" + ref;
    }
}

bool FunctionRefType::IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath) {
    if (!GetReturnType()->IsSafeType(error, symbolPath)) {
        SYMBOL_DESC symDesc;
        GetSymbolDesc(symDesc);
        symbolPath.push_back(symDesc);
        return false;
    }

    ui32 argCount = GetArgCount();
    for (ui32 i = 0; i < argCount; ++i) {
        Type *pArgType = GetArgType(i + 1);
        if (!pArgType->IsSafeType(error, symbolPath)) {
            SYMBOL_DESC symDesc;
            GetSymbolDesc(symDesc);
            symbolPath.push_back(symDesc);
            return false;
        }
    }
    return true;
}

/*************************************************************/

ArrayType::~ArrayType() {
    if (m_pDynExp) {
        delete m_pDynExp;
    }
    if (m_pTypeNode) {
        delete m_pTypeNode;
    }
}

RUNTIME_RETURN ArrayType::Run(Runable::Param &param) {
    if (m_pDynExp) {
        Memory &memory = *param.m_pMemory;
        Value sizeValue;
        Param sizeParam(&sizeValue, param);
        RUNTIME_RETURN ret = m_pDynExp->Run(sizeParam);
        if (ret == RT_NO_ERROR) {
            ui32 size = sizeValue.GetUI32() * m_pTypeNode->GetValueSize();
            // initialize all variables
            for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
                Node *pNode = *it;
                Variable *pVariable;
                if (pNode->IsAssignOp()) {
                    pVariable = static_cast<Variable*>(pNode->GetLeft());
                }
                else {
                    pVariable = static_cast<Variable*>(pNode);
                }
        
                ui32 idx = pVariable->GetVarIdx();
                MemoryBlockRef *pStackMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(idx);
                TODO("Revise how MemoryBlock is allocated!");
                MemoryBlockRef *pNewMemoryBlockRef = Memory::Allocate(this, 0, size);
                pStackMemoryBlockRef->SetMemoryBlock(this, 0, pNewMemoryBlockRef->GetMemoryBlock());
                pNewMemoryBlockRef->Release();
            }
        }
        return ret;
    }
    return Type::Run(param);
}

ui32 ArrayType::GetTypeSize() {
    assert(m_pTypeNode);
    ui32 typeSize = m_pTypeNode->GetValueSize();
    TODO("What to do in case of dynamic array or no dimensions?");
    if (m_dimensions.size() == 0) {
        typeSize = 0;
    }
    else {
        for (vector<ui32>::iterator it = m_dimensions.begin(); it != m_dimensions.end(); ++it) {
            typeSize *= *it;
        }
    }
    return typeSize;
}

ui32 ArrayType::GetValueSize() {
    assert(m_pTypeNode);
    if (IsReference()) {
        return sizeof(void*);
    }
    ui32 valueSize = m_dimensions.size() ? m_pTypeNode->GetValueSize() : 0;
    for (vector<ui32>::iterator it = m_dimensions.begin(); it != m_dimensions.end(); ++it) {
        valueSize *= *it;
    }
    return valueSize;
}

ui32 ArrayType::GetAlignment() {
    assert(m_pTypeNode);

    if (GetPassBy() != BY_VALUE) {
        return 4;
    }
    return m_pTypeNode->GetAlignment();
}

Value::VALUE_TYPE ArrayType::GetValueType() {
    assert(m_pTypeNode);
    return Value::ARRAY_TYPE;
}

bool ArrayType::IsRelatedType(Type *pType) {
    if (ArrayType *pArrayType = pType->QueryType<ArrayType>(TYPE_ARRAY)) {
        if (GetTypeNode()->IsEqualType(pArrayType->GetTypeNode()) && (GetTypeNode()->GetPassBy() == pArrayType->GetTypeNode()->GetPassBy())) {
            return true;
        }
    }
    return false;
}

bool ArrayType::IsEqualType(Type *pType) {
    if (ArrayType *pArrayType = pType->QueryType<ArrayType>(TYPE_ARRAY)) {
        if (GetTypeNode()->IsEqualType(pArrayType->GetTypeNode()) && (GetTypeNode()->GetPassBy() == pArrayType->GetTypeNode()->GetPassBy())) {
TODO("Revisit! What if dynamic arrays?")
            ui32 leftDim = GetDimensionCount(), rightDim = pArrayType->GetDimensionCount();
            if (!leftDim || !rightDim) {
                return true; // check it at runtime
            }
            // check if array size is the same regardless of dimensions!
            return GetTypeSize() == pArrayType->GetTypeSize();
        }
    }
    return false;
}

void ArrayType::SerializeBody(Serializer *pSerializer) {
    pSerializer->Begin(TYPE_LESS);
    m_pTypeNode->Serialize(pSerializer);
    if (m_pDynExp) {
        pSerializer->Begin(TYPE_COMMA);
        pSerializer->Begin(TYPE_EMPTY);
        m_pDynExp->Serialize(pSerializer);
    }
    else {
        for (ui32 i = 0; i < m_dimensions.size(); ++i) {
            SYMBOL_DESC symDesc(AggregateSymbol::GetSymbolId(i), 0, 0, 0);
            pSerializer->Begin(TYPE_COMMA);
            pSerializer->Begin(TYPE_EMPTY);
            pSerializer->WriteSymbol(symDesc);
        }
    }
    pSerializer->End(TYPE_GREATER);
    if (m_pLeft) {
        m_pLeft->Serialize(pSerializer);
    }
    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }
    if (m_variables.size()) {
        pSerializer->Begin(TYPE_EMPTY);
    }
    for (vector<Node*>::iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
        (*it)->Serialize(pSerializer);
        if ((it + 1) != m_variables.end()) {
            pSerializer->Begin(TYPE_COMMA);
            pSerializer->Begin(TYPE_EMPTY);
        }
    }

}

void ArrayType::CopySubValues(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset) {
    bool referenceItem = (m_pTypeNode->GetPassBy() != Type::BY_VALUE);
    ui32 valueSize = m_pTypeNode->GetValueSize();
    ui32 leftLength = (pDestMemoryBlock->GetSize() - destOffset) / valueSize, rightLength = (pSrcMemoryBlock->GetSize() - srcOffset) / valueSize;
    ui32 itemCount = (leftLength <= rightLength) ? leftLength : rightLength;

    for (ui32 i = 0; i < itemCount; ++i) {
        m_pTypeNode->CopyMemoryType(memory, pDestMemoryBlock, destOffset, pSrcMemoryBlock, srcOffset, referenceItem);
        destOffset += valueSize;
        srcOffset += valueSize;
    }
}

MemoryBlock* ArrayType::FindMemoryBlock(const char * const path, MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem) {
    MemoryBlock *pRealMemoryBlock = GetMemoryBlock(pMemoryBlock, offset, offsetOut, mem);
    
    if (!*path || !pRealMemoryBlock) {
        return pRealMemoryBlock;
    }

    ui32 valueSize = m_pTypeNode->GetValueSize();
    ui32 length;
    if (ui32 arraySize = GetTypeSize()) {
        length = arraySize / valueSize;
    }
    else {
        length = (pRealMemoryBlock->GetSize() - offsetOut) / valueSize;
    }
    ui32 idx = 0;
    const char * const subPath = GetSubPath(path, idx);

    MemoryBlock *pOutMemoryBlock = 0;
    if (length > idx) {
        ui32 offsetArray = offsetOut + (valueSize * idx);

        pOutMemoryBlock = m_pTypeNode->FindMemoryBlock(subPath, pRealMemoryBlock, offsetArray, offsetOut, mem);
    }
    pRealMemoryBlock->Release();
    return pOutMemoryBlock;
}

bool ArrayType::SetType(Type *pType) { 
    if (!m_pTypeNode) {
        m_pTypeNode = pType;
        return true;
    }
    return false;
}

ui32 ArrayType::GetDimensionSubscript(ui32 dimension) {
    return m_dimensions.size() > dimension ? m_dimensions[dimension] : 0;
}

void ArrayType::SetDynExpression(Node *pNode) {
    m_pDynExp = pNode;
}

Node* ArrayType::GetDynExpression() {
    return m_pDynExp;
}

string ArrayType::GetTypeString(SymbolStore *pSymStore) {
    string typeName, ref;
    pSymStore->GetSymbolName(this, typeName);
    switch (GetPassBy()) {
        case Type::PASS_BY::BY_REF_TO_REF:
            ref = "@";
        case Type::PASS_BY::BY_REF:
            ref += "@";
            break;
        default:
            break;
    }

    typeName.clear();
    ArrayType *pArrayType = (ArrayType*)this;
    typeName = pArrayType->GetTypeNode()->GetTypeString(pSymStore);
    ui32 dimCount = pArrayType->GetDimensionCount();
    string subscript;
    for (ui32 i = 0; i < dimCount; ++i) {
        ui32 k = pArrayType->GetDimensionSubscript(i);
        subscript += to_string(k);
        if ((i + 1) != dimCount) {
            subscript += ",";
        }
    }
    if (dimCount) {
        return "array<" + typeName + "," + subscript + ">" + ref;
    }
    else {
        return "array<" + typeName + ">" + ref;
    }
}

void ArrayType::HandleInstance(InstanceHandler *pInstanceHandler, MemoryBlock *pMemoryBlock, ui32 offset) {
    Type *pType = GetTypeNode();
    bool referenceItem = (pType->GetPassBy() != Type::BY_VALUE);
    ui32 valueSize = pType->GetValueSize();
    ui32 itemCount = GetTypeSize() / valueSize;
    // use memory size to get array size
    TODO("Check if this memory belogns to a struct then check end of array memory if array is not reference else?");
    if (!itemCount) { // use pMemoryBlock
        itemCount = (pMemoryBlock->GetSize() - offset) / valueSize;
    }
    for (ui32 i = 0; i < itemCount; ++i) {
        pInstanceHandler->Run(referenceItem, pType, pMemoryBlock, offset);
        offset += valueSize;
    }
}

bool ArrayType::IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath) {
    if (!GetTypeNode()->IsSafeType(error, symbolPath)) {
        SYMBOL_DESC symDesc;
        GetSymbolDesc(symDesc);
        symbolPath.push_back(symDesc);
        return false;
    }
    return true;
}
/*************************************************************/
ErrorType::ErrorType(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc) : AggregateType(pParent, symDesc, TYPE_ERROR_TYPE), m_pSymbolStore(pSymbolStore) {
    InitMember(string("name"), TYPE_STRING);
    InitMember(string("line"), TYPE_UI32);
    InitMember(string("position"), TYPE_UI32);
    InitMember(string("file"), TYPE_STRING);
    InitMember(string("trace"), TYPE_STRING);
    InitMember(string("symbol"), TYPE_STRING);
    AlignStruct(0);
}

void ErrorType::InitMember(string &name, NODE_TYPE type) {
    SYMBOL_DESC symDesc;// (m_pSymbolStore->UpdateSymbolMap(name), 0, 0, 0);
    Scope::GetDummyTypeNode(type)->GetSymbolDesc(symDesc);
    
    Type *pType = new Type(this, symDesc, type);
    symDesc.m_symId = m_pSymbolStore->UpdateSymbolMap(name);
    Variable *pVariable = new Variable(pType, symDesc, INVALID_VAR_IDX, INVALID_MEMORY_OFFSET, 0, pType);
    pType->PushVariable(pVariable);
    AddType(pType);
}

RUNTIME_RETURN ErrorType::Run(Runable::Param &param) {
    return AssertError();
}

void ErrorType::SerializeBegin(Serializer *pSerializer) {

}

void ErrorType::SerializeBody(Serializer *pSerializer) {
}

/*************************************************************/

Variable::Variable(Node *pParent,
    SYMBOL_DESC &symDesc,
    ui32 idx,
    MEMORY_OFFSET alignedOffset,
    AGGREGATE_TYPE_OFFSET alignedMemberOffset,
    Type *pType,
    const NODE_TYPE type) :
    Node(pParent, symDesc, type),
    m_idx(idx),
    m_alignedOffset(alignedOffset),
    m_alignedMemberOffset(alignedMemberOffset),
    m_pType(pType) {
}

MEMORY_OFFSET Variable::GetEndOfAlignedOffset() {
    MEMORY_OFFSET offset = INVALID_MEMORY_OFFSET;
    if (m_pType) {
        switch (m_pType->GetType()) {
            case TYPE_BOOL:
            case TYPE_I8:
            case TYPE_UI8: // 1 byte
                offset = m_alignedOffset + 1;
                break;
            case TYPE_I16:
            case TYPE_UI16: // 2 bytes
                offset = m_alignedOffset + 2;
                break;
            case TYPE_STRING:
            case TYPE_I32:
            case TYPE_UI32:
            case TYPE_FLOAT: // 4 bytes 
                offset = m_alignedOffset + 4;
                break;
            case TYPE_I64:
            case TYPE_UI64:
            case TYPE_DOUBLE:// 8 bytes
                offset = m_alignedOffset + 8;
                break;
            case TYPE_LIB:
            case TYPE_AGGREGATE_TYPE:
            default:
                assert(0);
                break;
        }
    }
    else {
        assert(0);
    }
    return offset;
}
/*
padding = (align - (offset & (align - 1))) & (align - 1)
        = (-offset & (align - 1))
aligned = (offset + (align - 1)) & ~(align - 1)
        = (offset + (align - 1)) & -align
*/
bool Variable::SetAlignedMemberOffset(AGGREGATE_TYPE_OFFSET lastOffset) {
    AGGREGATE_TYPE_OFFSET align = 0;
    if (m_pType) {
        align = m_pType->GetAlignment();
        m_alignedMemberOffset = (lastOffset + (align - 1)) & -(i32)align;
        return true;
    }
    return false;
}

AGGREGATE_TYPE_OFFSET Variable::GetEndOfAlignedMemberOffset() {
    if (m_pType) {
        return m_alignedMemberOffset + m_pType->GetValueSize();
    }
    return INVALID_AGGREGATE_TYPE_OFFSET;
}

RUNTIME_RETURN Variable::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    if (!param.m_pValue) { // no return 
        return ret;
    }
    if (m_pRight) {
        return m_pRight->Run(param); // either a function call, a member access or an array subscript
    }
    else {
        Value::VALUE_TYPE valueType = m_pType->GetValueType();
        Memory &memory = *param.m_pMemory;
        MemoryBlockRef *pMemoryBlockRef = 0;
        if (m_alignedOffset != INVALID_MEMORY_OFFSET) {
            pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(m_idx);
            ui32 offset = pMemoryBlockRef->GetOffset();
            param.m_pValue->SetRef(pMemoryBlockRef);
            if (valueType == Value::FUNCTION_REF_TYPE) {
_PROTECT_BEGIN
                void **pFunctionMemoryRef = pMemoryBlockRef->GetMemoryBlock()->GetPtr<void*>(offset);
                if (!*pFunctionMemoryRef) { // the functon is not resolved yet, so do it
                    Node *pParent = GetParent();
                    LibNode *pLibNode = 0;
                    if (pParent && (pParent = pParent->GetParent()) && (pLibNode = pParent->QueryType<LibNode>(TYPE_LIB_VAR))) {
                        LibType *pLibType = pLibNode->GetTypeNode()->QueryType<LibType>(TYPE_LIB);
                        assert(pLibType);
                        FunctionRefType *pFunctionRefType = pLibType->GetFunctionRefType(GetSymbolId());
                        assert(pFunctionRefType);
                        Variable *pFooVar = pFunctionRefType->FindVariableDecNode(GetSymbolId());
                        assert(pFooVar);
                        FunctionLibPtr *pFunctionLibPtr = pFooVar->GetRight()->QueryType<FunctionLibPtr>(TYPE_FUNCTION_LIB_PTR);
                        assert(pFunctionLibPtr);
                        *pFunctionMemoryRef = pFunctionLibPtr->GetFooPtr(param);
                    }
                    else if (Function *pFunction = memory.GetRootScope()->ResolveFunction(m_pType->GetSymbolId())) {
                        *pFunctionMemoryRef = pFunction->GetRawPtr(param);
                    }
                    else {
                        return AssertError();
                    }
                }
_PROTECT_END(RT_ERROR)
            }
        }
        else {
            if (valueType == Value::FUNCTION_REF_TYPE) {
                if (Function *pFunction = memory.GetRootScope()->ResolveFunction(m_symDesc.m_symId)) {
                    assert(GetAlignedMemberOffset() == 0);
                    pMemoryBlockRef = Memory::Allocate(m_pType, 0, m_pType->GetTypeSize());//????
                    *pMemoryBlockRef->GetMemoryBlock()->GetPtr<void*>(0) = pFunction->GetRawPtr(param);
                    param.m_pValue->SetRef(pMemoryBlockRef);
                    pMemoryBlockRef->Release();
                }
                else {
                    return AssertError();
                }
            }
            else {
                return AssertError();
            }
        }
    }
    return ret;
}

bool Variable::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(pReturnType == 0);

    switch (m_pType->GetType()) {
        //case TYPE_CONST:
        //case TYPE_CONST_BOOL:
        //case TYPE_CONST_NUM:
        //    if (m_pLeft) { // prepost operator
        //        SYMBOL_DESC symDesc;
        //        m_pLeft->GetSymbolDesc(symDesc);
        //        error.SetErrorEx(OPERATOR_NOT_ALLOWED_ON_CONST, SCRIPT_SYM_TO_STR(OPERATOR_NOT_ALLOWED_ON_CONST), __FILE__, __FUNCTION__, __LINE__, symDesc);
        //        return false;
        //    }
        //    pReturnType = m_pType;
        //    break;
        case TYPE_FUNCTION_REF_TYPE:
            if (m_pLeft) { // prepost operator
                SYMBOL_DESC symDesc;
                m_pLeft->GetSymbolDesc(symDesc);
                if (GetRight()) {
                    error.SetErrorEx(OPERATOR_NOT_ALLOWED_ON_FUNCTION_CALL, SCRIPT_SYM_TO_STR(OPERATOR_NOT_ALLOWED_ON_FUNCTION_CALL), __FILE__, __FUNCTION__, __LINE__, symDesc);
                }
                else {
                    error.SetErrorEx(OPERATOR_NOT_ALLOWED_ON_TYPE, SCRIPT_SYM_TO_STR(OPERATOR_NOT_ALLOWED_ON_TYPE), __FILE__, __FUNCTION__, __LINE__, symDesc);
                }
                return false;
            }
            if (m_pRight) { // function call
                return m_pRight->IsExpression(pReturnType, pPrePostExp, error);
                //FunctionRefType *pFunctionRefType = m_pType->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
                //assert(pFunctionRefType);
                //NODE_TYPE type = pFunctionRefType->GetParent()->GetType();
                //switch (type) {
                //    case TYPE_CAST:
                //        assert(0);
                //        break;
                //    case TYPE_LOCK:
                //    case TYPE_UNLOCK:
                //        if (Variable *pArgVar = static_cast<FunctionCall*>(GetRight())->GetArgNode(0)->QueryType<Variable>(TYPE_VAR)) {
                //            if (pArgVar->IsExpression(pReturnType, pPrePostExp, error)) {
                //                if (type == TYPE_CAST) {
                //                    pReturnType = pFunctionRefType->GetReturnType();
                //                //    FunctionRefType *pFunctionRefType = pArgVar->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
                //                //    pReturnType = pFunctionRefType->GetReturnType();
                //                }
                //                return true;
                //            }
                //            return false;
                //            //pReturnType = pArgVar->GetTypeNode();
                //        }
                //        else {
                //            _ERROR(INTERNAL_ERROR);
                //            return false;
                //        }
                //        break;
                //    default:
                //        pReturnType = pFunctionRefType->GetReturnType();
                //        break;
                //}
            }
            else {
                if ((Scope::s_reservedFooUpper < m_symDesc.m_symId) || (Scope::s_reservedFooLower > m_symDesc.m_symId)) {
                    pReturnType = m_pType;
                }
                else {
                    _ERROR(SYNTAX_ERROR);
                }
            }
            break;
        case TYPE_AGGREGATE_TYPE:
            if (m_pLeft) { // prepost exp
                SYMBOL_DESC symDesc;
                m_pLeft->GetSymbolDesc(symDesc);
                error.SetErrorEx(OPERATOR_NOT_ALLOWED_ON_STRUCT, SCRIPT_SYM_TO_STR(OPERATOR_NOT_ALLOWED_ON_STRUCT), __FILE__, __FUNCTION__, __LINE__, symDesc);
                return false;
            }
            if (m_pRight) { // member access
                return m_pRight->IsExpression(pReturnType, pPrePostExp, error);
            }
            pReturnType = m_pType;
            break;
        case TYPE_ARRAY:
            if (m_pLeft) { // prepost expression
                SYMBOL_DESC symDesc;
                m_pLeft->GetSymbolDesc(symDesc);
                error.SetErrorEx(OPERATOR_NOT_ALLOWED_ON_ARRAY, SCRIPT_SYM_TO_STR(OPERATOR_NOT_ALLOWED_ON_ARRAY), __FILE__, __FUNCTION__, __LINE__, symDesc);
                return false;
            }
            if (m_pRight) { // subscript
                return m_pRight->IsExpression(pReturnType, pPrePostExp, error);
                //// ???? skip subscript expression validation, it's validated separately ????
                //if (Node *pRightNode = m_pRight->GetRight()) {
                //    if (pRightNode->GetType() == TYPE_MEMBER_ACCESS) {
                //        return pRightNode->IsExpression(pReturnType, pPrePostExp, error);
                //    }
                //    else {
                //        _ERROR(INTERNAL_ERROR);
                //        return false;
                //    }
                //}
                //else {
                //    if (m_pRight->GetLeft()) { // prepost expression
                //        return m_pRight->GetLeft()->IsExpression(pReturnType, pPrePostExp, error);
                //    }
                //    pReturnType = m_pType->QueryType<ArrayType>(TYPE_ARRAY)->GetTypeNode();
                //}
            }
            else {
                pReturnType = m_pType;
            }
            break;
        default:
            switch (GetType()) {
                case TYPE_CONST:
                case TYPE_CONST_BOOL:
                case TYPE_CONST_NUM:
                    if (m_pLeft) { // prepost operator
                        SYMBOL_DESC symDesc;
                        m_pLeft->GetSymbolDesc(symDesc);
                        error.SetErrorEx(OPERATOR_NOT_ALLOWED_ON_CONST, SCRIPT_SYM_TO_STR(OPERATOR_NOT_ALLOWED_ON_CONST), __FILE__, __FUNCTION__, __LINE__, symDesc);
                        return false;
                    }
                default:
                    if (m_pLeft) {
                        return m_pLeft->IsExpression(pReturnType, pPrePostExp, error);
                    }
                    break;
            }
            pReturnType = m_pType;
            break;
    }
    return true;
}
// runtime API
Variable* Variable::GetRootVariableNode() {
    Node *pNode = this, 
         *pRootVariableNode = this;
    while (pNode) {
        if (pNode->GetType() == TYPE_ERROR_VAR) {
            pRootVariableNode = pNode;
            break;
        }
        if (pNode->GetParent() && 
            ((pNode->GetParent()->GetType() == TYPE_FUNCTION_CALL) || (pNode->GetParent()->GetType() == TYPE_SUBSCRIPT))) {
            if (pNode->GetParent()->GetRight() != pNode) {
                break;
            }
        }
        switch (pNode->GetType()) {
            case TYPE_SUBSCRIPT:
            case TYPE_VAR:
            case TYPE_FUNCTION_CALL:
            case TYPE_MEMBER_ACCESS:
                pRootVariableNode = pNode;
                pNode = pNode->GetParent();
                break;
            case TYPE_CONST:
            case TYPE_CONST_BOOL:
            case TYPE_LIB_VAR:
                pRootVariableNode = pNode;
                pNode = 0;
                break;
            default:
                pNode = 0;
                break;
        }
    }
    Variable *pVariable = pRootVariableNode->QueryType<Variable>(TYPE_VAR);
    return pVariable;
}

Variable* Variable::GetSubVariable() {
    Variable *pVariable = this;
    Node *pRight = this;
    while (pRight) {
        pRight = pRight->GetRight();
        if (pRight) {
            if (Variable *pSubVariable = pRight->QueryType<Variable>(TYPE_VAR)) {
                pVariable = pSubVariable;
            }
        }
    }
    return pVariable;
}

Node* Variable::FindFirstMemberSubscriptOrFooCall() {
    Node *pRight = this;
    while (pRight) {
        switch (pRight->GetType()) {
            case TYPE_FUNCTION_CALL:
            case TYPE_SUBSCRIPT:
                return pRight;
            default:
                pRight = pRight->GetRight();
                break;
        }
    }
    return 0;
}

void Variable::SetPreOperator(Node *pPreOperator, Node* pNewParent) {
    Node *pLast = pPreOperator;
    while (pLast) {
        pLast->SetParent(this);
        if (pLast->GetLeft()) {
            pLast = pLast->GetLeft();
        }
        else {
            break;
        }
    }
    SetLeft(pPreOperator);
    SetParent(pNewParent);
}

void Variable::SetPostOperator(Node *pPostOperator) {
    Node *pLast = this;
    while (pLast) {
        if (pLast->GetLeft()) {
            pLast = pLast->GetLeft();
        }
        else {
            break;
        }
    }
    pLast->SetLeft(pPostOperator);
    pPostOperator->SetParent(this); // duplicate?
}

Node* Variable::GetPrePostExpressionParent() {
    Node *pNode = this;
    while (pNode->GetRight()) {
        pNode = pNode->GetRight();
        if (Variable *pVariable = pNode->QueryType<Variable>(TYPE_VAR)) {
            if (pVariable->GetTypeNode()->GetType() == TYPE_FUNCTION_REF_TYPE) {
                break;
            }
            else {
            }
        }
    }
    return pNode;
}

void Variable::SerializePreExp(Node *pNode, Serializer *pSerializer) {
    while (pNode) {
        switch (pNode->GetType()) {
            case TYPE_PRE_INC_OP:
            case TYPE_PRE_DEC_OP: {
                SYMBOL_DESC symDesc;
                pNode->GetSymbolDesc(symDesc);
                pSerializer->WriteSymbol(symDesc);
                break;
            }
            default:
                break;
        }
        pNode = pNode->GetLeft();
    }
}

void Variable::SerializePostExp(Node *pNode, Serializer *pSerializer) {
    while (pNode) {
        switch (pNode->GetType()) {
            case TYPE_POST_INC_OP:
            case TYPE_POST_DEC_OP: {
                SYMBOL_DESC symDesc;
                pNode->GetSymbolDesc(symDesc);
                pSerializer->WriteSymbol(symDesc);
                break;
            }
            default:
                break;
        }
        pNode = pNode->GetLeft();
    }
}

void Variable::SerializeBody(Serializer *pSerializer) {
    assert(pSerializer);
    Node *pPrePostExpParent = GetPrePostExpressionParent();
    Node *pPrePostExp = pPrePostExpParent->GetLeft();
    if (pPrePostExp) {
        pPrePostExpParent->SetLeft(0);
    }

    SerializePreExp(pPrePostExp, pSerializer);

    pSerializer->WriteSymbol(m_symDesc);
    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }

    SerializePostExp(pPrePostExp, pSerializer);

    pPrePostExpParent->SetLeft(pPrePostExp);
}

/*************************************************************/

void ConstVariable::SerializeBody(Serializer *pSerializer) {
    assert(pSerializer);
    if (SYMBOL_ID_BLANK == m_symDesc.m_symId) {
        AggregateSymbol::SerializeBody(pSerializer);
    }
    else {
        pSerializer->WriteSymbol(m_symDesc);
    }
}

RUNTIME_RETURN ConstVariable::Run(Runable::Param &param) {
    *param.m_pValue = m_value;
    return RT_NO_ERROR;
}

/*************************************************************/

//void ConstAlias::SerializeBody(Serializer *pSerializer) {
//    assert(pSerializer);
//    if (SYMBOL_ID_BLANK == m_symDesc.m_symId) {
//        AggregateSymbol::SerializeBody(pSerializer);
//    }
//    else {
//        pSerializer->WriteSymbol(m_symDesc);
//    }
//}
//
//RUNTIME_RETURN ConstAlias::Run(Runable::Param &param) {
//    RUNTIME_RETURN ret = ConstVariable::Run(param);
//    return ret;
//    //*param.m_pValue = m_value;
//    //return RT_NO_ERROR;
//}

/*************************************************************/

CastVariable::CastVariable(Node *pParent, SYMBOL_DESC &symDesc, ui32 idx, Function *pFunction) :
    Variable(pParent, symDesc, idx, INVALID_MEMORY_OFFSET, 0, new FunctionRefType(pFunction, symDesc), TYPE_VAR) {
}

CastVariable::~CastVariable() {
    delete GetTypeNode();
}

RUNTIME_RETURN CastVariable::Run(Runable::Param &param) {
    FunctionRefType *pFunctionRefType = GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
    Type *pType = pFunctionRefType->GetReturnType();
    param.m_pValue->SetType(pType->GetValueType(), pType);
    return Variable::Run(param);
}

bool CastVariable::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    if (m_pRight) { // function call
        if (!m_pRight->IsExpression(pReturnType, pPrePostExp, error)) {
            return false;
        }
        FunctionRefType *pFunctionRefType = GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
        pReturnType = pFunctionRefType->GetReturnType();
        return true;
    }
    else {
        _ERROR(SYNTAX_ERROR);
        return false;
    }
}

//void CastVariable::SerializeBegin(Serializer *pSerializer) {
//
//}
//
//void CastVariable::SerializeBody(Serializer *pSerializer) {
//    pSerializer->WriteSymbol(m_symDesc);
//    pSerializer->Begin(TYPE_LESS);
//    FunctionRefType *pFunctionRefType = GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
//    pFunctionRefType->GetReturnType()->Serialize(pSerializer);
//    pSerializer->End(TYPE_GREATER);
//    m_pRight->Serialize(pSerializer);
//}

/*************************************************************/

RUNTIME_RETURN MemberAccess::Run(Runable::Param &param) {
    if (Variable *pDataMemberVariable = m_pRight->QueryType<Variable>(TYPE_VAR)) {

        Memory         &memory = *param.m_pMemory;
        StackMemory    &stackMemory = *param.m_pStackMemory;
        ui32            varIdx;
        switch (GetParent()->GetType()) {
            case TYPE_SUBSCRIPT:
                varIdx = static_cast<SubscriptNode*>(GetParent())->GetVarIdx();
                break;
            case TYPE_VAR:
            case TYPE_LIB_VAR:
            case TYPE_ERROR_VAR:
                varIdx = static_cast<Variable*>(GetParent())->GetVarIdx();
                break;
            case TYPE_FUNCTION_CALL:
                varIdx = 0;
                break;
            default:
                assert(0);
                break;
        }
        MemoryBlockRef *pParentVariableMemoryBlockRef = stackMemory.GetMemoryBlockRef(varIdx);
        MemoryBlock    *pParentVariableMemoryBlock = pParentVariableMemoryBlockRef->GetMemoryBlock();
        MemoryBlock    *pDataMemberMemoryBlock = 0;
        Type           *pDataMemberType = pDataMemberVariable->GetTypeNode();
        ui32            offset = pParentVariableMemoryBlockRef->GetOffset() + pDataMemberVariable->GetAlignedMemberOffset(); // offset of data member variable
        RUNTIME_RETURN ret = RT_NO_ERROR;
        if ((pDataMemberType->GetPassBy() != Type::BY_VALUE)) {
            // dereference memory 
            // this call adds reference
            param.SetReferenceOfValue(pParentVariableMemoryBlock, offset); // set it first!
_PROTECT_BEGIN
            pDataMemberMemoryBlock = memory.GetReferencedMemoryBlock(pDataMemberType, pParentVariableMemoryBlock, offset/*new offset returned*/);
_PROTECT_END(RT_ERROR)
#ifdef MEMORY_ACCESS_EXCEPTION
            if (ret == RT_ERROR) {
                return ret;
            }
#endif // MEMORY_ACCESS_EXCEPTION
        }
        else {
            param.ReleaseReferenceOfValue(false);
            pDataMemberMemoryBlock = pParentVariableMemoryBlock;
            pDataMemberMemoryBlock->AddRef();
        }
        varIdx = pDataMemberVariable->GetVarIdx();
        stackMemory.PushReferencedMemoryBlock(varIdx, pDataMemberType, offset, pDataMemberMemoryBlock);

        ret = m_pRight->Run(param);
        stackMemory.PopReferencedMemoryBlock(varIdx);
        pDataMemberMemoryBlock->Release();
        return ret;
    }

    return AssertError();
}

bool MemberAccess::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    if (m_pRight != 0) {
        return m_pRight->IsExpression(pReturnType, pPrePostExp, error);
    }
    _ERROR(SYNTAX_ERROR);
    return false;
}

/*Variable* MemberAccess::GetVariableNode() {
    Node *pNode = GetParent();
    while (pNode) {
        if (Node *pNextParentToVisit = pNode->GetParent()->GetParent()) {
            if (pNextParentToVisit->GetType() == TYPE_MEMBER_ACCESS) {
                pNode = pNextParentToVisit->GetParent();
                continue;
            }
        }
        break;
    }
    return static_cast<Variable*>(pNode);
}*/

void MemberAccess::SerializeBody(Serializer *pSerializer) {
    assert(m_pRight);
    pSerializer->Begin(TYPE_MEMBER_ACCESS);
    m_pRight->Serialize(pSerializer);
}

/*************************************************************/

RUNTIME_RETURN OperatorRefAssign::Run(Runable::Param &param) {
    if (Variable *pLeftVariable = m_pLeft->QueryType<Variable>(TYPE_VAR)) {
        Value rightValue;
        Param rightParam(&rightValue, param);
        RUNTIME_RETURN ret = m_pRight->Run(rightParam);
        if (ret == RT_NO_ERROR) {
            if (rightParam.m_pValue->GetType() == Value::NULL_TYPE) {
                param.m_pStackMemory->GetRunTimeError().SetError("Right value is NULL!", m_symDesc, 0, param.m_pStackMemory);
                return RT_ERROR;
            }
            param.m_pReferenceOfValueMemoryBlock = GET_VALUE_REFERENCE;
            ret = pLeftVariable->Run(param);

            MemoryBlockRef *pRightMemoryBlockRef = rightValue.GetMemoryBlockRef(pLeftVariable->GetTypeNode());
            //if (!
            param.OverwriteReferenceOfValue(pRightMemoryBlockRef);//) {
                // left is not a reference, it must be a not struct and not array
                //MemoryBlockRef *pLeftMemoryBlockRef = param.m_pValue->GetMemoryBlockRef();
                //NODE_TYPE type = pLeftMemoryBlockRef->GetReferenceTypeNode()->GetType();
                //switch (type) {
                //    case TYPE_ARRAY:
                //        param.m_pStackMemory->GetRunTimeError().SetError("Right value is NULL!", m_symDesc, 0, param.m_pStackMemory);
                //        _ERROR(CANNOT_ASSIGN_REFERENCE_TO_ARRAY_OF_NOT_REFERENCES);
                //        break;
                //    case TYPE_STRUCT:
                //        _ERROR(CANNOT_ASSIGN_REFERENCE_TO_MEMBER);
                //        break;
                //}
                param.m_pValue->Set(pRightMemoryBlockRef, pLeftVariable->GetTypeNode(), pRightMemoryBlockRef->GetOffset());
            //}
            param.ReleaseReferenceOfValue(true);// after we set new reference
        }
        return ret;
    }
    return AssertError();
}

bool OperatorRefAssign::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(!pReturnType);

    if (!m_pLeft) {
        _ERROR(REFERENCE_ASSIGNMENT_NEEDS_LEFT_VALUE);
        return false;
    }
    if (!m_pRight) {
        _ERROR(REFERENCE_ASSIGNMENT_NEEDS_RIGHT_VALUE);
        return false;
    }
    
    if (Variable *pVariable = m_pLeft->QueryType<Variable>(TYPE_VAR)) {
        Variable *pSubVariable = pVariable->GetSubVariable();
        if (pSubVariable->IsFunctionCall()) {
            _ERROR(REFERENCE_ASSIGNMENT_NEEDS_LEFT_VALUE);
            return false;
        }
        else if (pSubVariable->IsConst()) {
            _ERROR(LEFT_VALUE_CANNOT_BE_CONST);
            return false;
        }

        Type *pLeftType = 0, *pRightType = 0;
        if (m_pLeft->IsExpression(pLeftType, pPrePostExp, error) &&
            m_pRight->IsExpression(pRightType, pPrePostExp, error)) {
            if (pLeftType->IsEqualType(pRightType)) {
                switch (pLeftType->GetParent()->GetType()) {
                    case TYPE_ARRAY:
                    case TYPE_AGGREGATE_TYPE:
                        if (!pLeftType->IsReference()) {
                            _ERROR(REFERENCE_ASSIGNMENT_NEEDS_LEFT_VALUE_AS_REFERENCE);
                            return false;
                        }
                        break;
                    default:
                        break;
                }
                pReturnType = pLeftType;
                return true;
            }
            else {
                _ERROR(TYPES_MISMATCHED);
            }
        }
        //if (m_pRight->IsExpression(pRightType, pPrePostExp, error)) {
        //    //Type *pLeftType = pSubVariable->GetTypeNode();
        //    if (pSubVariable == pVariable) { // variable
        //        if (pLeftType->IsEqualType(pReturnType)) {
        //            return true;
        //        }
        //        else {
        //            _ERROR(TYPES_MISMATCHED);
        //        }
        //    }
        //    else { // handle array and struct here
        //        // check if left is struct member or an array item
        //        switch (pSubVariable->GetParent()->GetType()) {
        //            case TYPE_MEMBER_ACCESS:
        //            case TYPE_SUBSCRIPT:
        //                if (pLeftType->GetPassBy() == Type::BY_VALUE) {
        //                    _ERROR(REFERENCE_ASSIGNMENT_NEEDS_LEFT_VALUE_AS_REFERENCE);
        //                    break;
        //                }
        //                else {
        //                    if (pLeftType->IsEqualType(pReturnType)) {
        //                        return true;
        //                    }
        //                    else {
        //                        _ERROR(TYPES_MISMATCHED);
        //                    }
        //                }
        //                break;
        //            default:
        //                _ERROR(INTERNAL_ERROR);
        //                break;
        //        }
        //    }
        //}
    }
    else {
        _ERROR(REFERENCE_ASSIGNMENT_NEEDS_LEFT_VALUE_AS_VARAIBLE);
    }
    return false;
}

//bool OperatorRefAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
//
//}

/*************************************************************/

FunctionPtrWrapper::~FunctionPtrWrapper() {
    if (m_pFunctionPtr && (m_pFunctionPtr->GetType() == TYPE_FUNCTION_PTR)) {
        delete m_pFunctionPtr;
    }
}

FunctionPtrWrapper::FunctionPtrWrapper(FunctionPtr *pFunctionPtr) :
    m_pRefFunctionPtr(&m_pFunctionPtr),
    m_pFunctionPtr(pFunctionPtr) {
}

FunctionPtrWrapper::FunctionPtrWrapper(const FunctionPtrWrapper &that) :
    m_pRefFunctionPtr(&m_pFunctionPtr),
    m_pFunctionPtr(0) {
    *this = that;
}

FunctionPtrWrapper& FunctionPtrWrapper::operator=(const FunctionPtrWrapper &that) {
    if (that.m_pRefFunctionPtr != &that.m_pFunctionPtr) {
        m_pRefFunctionPtr = const_cast<FunctionPtr**>(&that.m_pFunctionPtr);
    }
    m_pFunctionPtr  = that.m_pFunctionPtr;
    return *this;
}

/*************************************************************/

AggregateDefCollection::~AggregateDefCollection() {

}

bool AggregateDefCollection::PushAggregateDef(AggregateType *pAggregateType) {
    assert(pAggregateType);
    SYMBOL_DESC symDesc;
    pAggregateType->GetSymbolDesc(symDesc);
    pair<AGGREGATE_REF_MAP::iterator, bool> ret = m_aggregateDefs.insert(pair<SYMBOL_ID, AggregateType*>(symDesc.m_symId, pAggregateType));
    return ret.second;
}

AggregateType* AggregateDefCollection::ResolveAggregateType(SYMBOL_ID symId) {
    AGGREGATE_REF_MAP::iterator it = m_aggregateDefs.find(symId);
    if (it != m_aggregateDefs.end()) {
        return it->second;
    }
    return 0;
}

/*************************************************************/
/*************************************************************/

FUNCTION_PTR_WRAPPER_MAP        Scope::s_functionPtrWrapperMap;
vector<Type*>                   Scope::s_dummyTypes;
SYMBOL_ID                       Scope::s_reservedFooLower = SYMBOL_ID_MAX;
SYMBOL_ID                       Scope::s_reservedFooUpper = SYMBOL_ID_MAX;

Scope::Scope(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) :
    AggregateDefCollection(pParent, symDesc, type),
#ifdef SCRIPT_DEBUGGER
    m_pDebuggerHandler(0),
#endif // SCRIPT_DEBUGGER

    m_unalignedEndOfStack(0) {
    if (!pParent) {
        s_reservedFooLower = SYMBOL_ID_MAX;
        s_reservedFooUpper = SYMBOL_ID_MAX;
        InitDummyTypes();
        //InitBuiltInFunctions();
    }
};

Scope::~Scope() {
    if (!GetParent()) {
        for (FUNCTION_PTR_WRAPPER_MAP::iterator it = s_functionPtrWrapperMap.begin(); it != s_functionPtrWrapperMap.end(); ++it) {
            delete it->second;
        }

        Delete(m_expressions);

        s_functionPtrWrapperMap.clear();
        Delete(s_dummyTypes);
        s_dummyTypes.clear();
        s_reservedFooLower = SYMBOL_ID_MAX;
        s_reservedFooUpper = SYMBOL_ID_MAX;
    }
    else {
        Delete(m_expressions);
    }
}

void Scope::InitDummyTypes() {
    SymbolStore *pSymbolStore = SymbolStore::GetSymbolStore();
    SYMBOL_DESC symDesc(SYMBOL_ID_BLANK, 0, 0, 0);
    typedef struct SymMap {
        NODE_TYPE type;
        string    name;
    } _SYM_MAP;
    _SYM_MAP map[] = {
        { TYPE_BOOL, "bool"},
        { TYPE_I8, "i8" },
        { TYPE_UI8, "ui8" },
        { TYPE_I16, "i16" },
        { TYPE_UI16, "ui16" },
        { TYPE_I32, "i32" },
        { TYPE_UI32, "ui32" },
        { TYPE_I64, "i64" },
        { TYPE_UI64, "ui64" },
        { TYPE_FLOAT, "float" },
        { TYPE_DOUBLE, "double" },
        { TYPE_STRING, "string" },
    };

    for (ui32 i = 0; i < sizeof(map) / sizeof(map[0]); ++i) {// type = TYPE_BOOL; type <= TYPE_STRING; ++type) {
        symDesc.m_symId = pSymbolStore->UpdateSymbolMap(map[i].name);
        Type *pType = new Type(this, symDesc, static_cast<NODE_TYPE>(map[i].type));
        s_dummyTypes.push_back(pType);
    }
    symDesc.m_symId = pSymbolStore->UpdateSymbolMap(string("Error"));
    ErrorType *pErrorType = new ErrorType(pSymbolStore, this, symDesc);
    s_dummyTypes.push_back(pErrorType);
}

//void Scope::InitBuiltInFunctions() {
//}

Type* Scope::GetDummyTypeNode(NODE_TYPE type) {
    if ((type >= TYPE_BOOL) && (type <= TYPE_STRING)) {
        return s_dummyTypes[type - TYPE_BOOL];
    }
    else if (TYPE_ERROR_TYPE == type) {
        Type *pType = s_dummyTypes.back();
        if (pType->GetType() == type) {
            return pType;
        }
    }
    return 0;
}

RUNTIME_RETURN Scope::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
#ifdef SCRIPT_DEBUGGER
    Scope *pOutterScope = param.m_pCurrentScope;
#endif // SCRIPT_DEBUGGER
    for (vector<Node*>::iterator it = m_expressions.begin(); it != m_expressions.end(); ++it) {
        Node* pNode = (*it);
        switch (pNode->GetType()) {
            case TYPE_FUNCTION:
            case TYPE_ERROR:
            case TYPE_SIZEOF:
            case TYPE_ARR2STR:
            case TYPE_WARR2STR:
            case TYPE_STR2ARR:
            case TYPE_STR2WARR:
            case TYPE_LOCK:
            case TYPE_UNLOCK:
            case TYPE_CAST:
                break;
            default: {
NEXT:
                Value value;
                Param _param(&value, param);

#ifdef SCRIPT_DEBUGGER
                param.SetCurrentScope(this);
                if (pNode->GetType() != TYPE_WHILE) {
                    ret = CheckFlow(pNode, param);
                    if (RT_STOP == ret) {
                        return ret;
                    }
                }
#endif // SCRIPT_DEBUGGER

                ret = pNode->Run(_param);

                switch (ret) {
                    case RT_NO_ERROR:
                        break;
                    case RT_RETURN:
                        param.m_pValue->Set(value);
#ifdef SCRIPT_DEBUGGER
                        param.m_pCurrentScope = pOutterScope;
#endif // SCRIPT_DEBUGGER
                        return ret;
                    case RT_ERROR: // skip all expression until error() node is found
                        do {
                            pNode = (*it);
                            if (pNode->GetType() == TYPE_ERROR) {
                                ret = RT_NO_ERROR;
                                goto NEXT;
                            }
                        } while (++it != m_expressions.end());
#ifdef SCRIPT_DEBUGGER
                        param.m_pCurrentScope = pOutterScope;
#endif // SCRIPT_DEBUGGER
                        return ret;
                    case RT_CONTINUE:
#ifdef SCRIPT_DEBUGGER
                        param.m_pCurrentScope = pOutterScope;
#endif // SCRIPT_DEBUGGER
                        return ret;
                    default:
#ifdef SCRIPT_DEBUGGER
                        param.m_pCurrentScope = pOutterScope;
#endif // SCRIPT_DEBUGGER
                        return ret;
                }
            }
            break;
        }
    }
#ifdef SCRIPT_DEBUGGER
    param.m_pCurrentScope = pOutterScope;
#endif // SCRIPT_DEBUGGER
    return ret;
}

void Scope::SerializeBody(Serializer *pSerializer) {
    if (pSerializer) {
        Function *pFunction = GetParent() ? GetParent()->QueryType<Function>(TYPE_FUNCTION) : 0;
        bool skip = false;
        for (vector<Node*>::iterator it = m_expressions.begin(); it != m_expressions.end(); ++it) {
            if (pFunction) {
                vector<SYMBOL_DESC>& argSymIds = pFunction->GetArgumentVector();
                for (ui32 i = 0; i < argSymIds.size(); ++i) {
                    if ((*it)->QueryType<Type>(TYPE_RUNABLE)) {
                        if (static_cast<Type*>(*it)->FindVariableDecNode(argSymIds[i].m_symId)) {
                            skip = true;
                            break;
                        }
                    }
                }
            }

            if (!skip) {
                switch ((*it)->GetType()) {
                    case TYPE_IF:
                    case TYPE_FOR:
                    case TYPE_WHILE:
                    case TYPE_ERROR:
                    case TYPE_FUNCTION:
                    case TYPE_SCOPE:
                    case TYPE_SWITCH:
                        (*it)->Serialize(pSerializer);
                        break;
                    case TYPE_SIZEOF:
                    case TYPE_ARR2STR:
                    case TYPE_WARR2STR:
                    case TYPE_STR2ARR:
                    case TYPE_STR2WARR:
                    case TYPE_LOCK:
                    case TYPE_UNLOCK:
                    case TYPE_CAST:
                    //case TYPE_ERROR_VAR:
                        break;
                    default:
                        (*it)->Serialize(pSerializer);
                        pSerializer->End(TYPE_SCOPE_END_EXPRESSION);
                        break;
                }
            }
            skip = false;
        }
    }
}

void Scope::PushExpression(Node* pNode) {
    assert(pNode);
    m_expressions.push_back(pNode);
#ifdef SCRIPT_DEBUGGER
    if (m_pDebuggerHandler) {
        NODE_TYPE type = pNode->GetType();
        switch (type) {
            case TYPE_FUNCTION:
            case TYPE_SIZEOF:
            case TYPE_ARR2STR:
            case TYPE_WARR2STR:
            case TYPE_STR2ARR:
            case TYPE_STR2WARR:
            case TYPE_LOCK:
            case TYPE_UNLOCK:
            case TYPE_CAST:
                break;
            default:
                m_pDebuggerHandler->RegisterBreakPointLine(pNode);
                RegisterBreakPointLine(pNode);
                break;
        }
    }        
#endif // SCRIPT_DEBUGGER
}

bool Scope::UpdateExpression(Node* pNode) {
    if (m_expressions.size()) {
        m_expressions.pop_back();
        m_expressions.push_back(pNode);
        return true;
    }
    return false;
}

bool Scope::PushAggregateDef(AggregateType *pAggregateType) {
    assert(pAggregateType);
    PushExpression(pAggregateType);
    return AggregateDefCollection::PushAggregateDef(pAggregateType);
}

bool Scope::PushFunctionDef(Function *pFunction) {
    assert(pFunction);
    SYMBOL_DESC symDesc;
    pFunction->GetSymbolDesc(symDesc);
    pair<FUNCTION_REF_MAP::iterator, bool> ret = m_functionDefs.insert(pair<SYMBOL_ID, Function*>(symDesc.m_symId, pFunction));
    if (ret.second) {
        PushExpression(pFunction);
    }
    return ret.second;
}

bool Scope::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();
    for (ui32 i = 0; i < m_expressions.size(); ++i) {
        if (m_expressions[i] == pExpNode) {
            pPrePostExpressions->SetParent(this);
            pExpNode->SetParent(pPrePostExpressions);
            m_expressions[i] = pPrePostExpressions;
            return true;
        }
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

Function* Scope::ResolveFunction(SYMBOL_ID symId) {
    FUNCTION_REF_MAP::iterator it = m_functionDefs.find(symId);
    if (it != m_functionDefs.end()) {
        return it->second;
    }
    return 0;
}

FunctionPtr* Scope::GetFunctionPtr(void *pFooPtr) {
    TODO("Revisit and move it to Memory class!");
    FUNCTION_PTR_WRAPPER_MAP::iterator it = s_functionPtrWrapperMap.find(pFooPtr);
    if (it != s_functionPtrWrapperMap.end()) {
        return it->second->m_pFunctionPtr;
    }
    return 0;
}

bool Scope::RegisterFunctionPtr(void *pFooPtr, FunctionPtr* pFunctionPtr) {
    pair<FUNCTION_PTR_WRAPPER_MAP::iterator, bool> ret = s_functionPtrWrapperMap.insert(pair<void*, FUNCTION_PTR_WRAPPER*>(pFooPtr, new FUNCTION_PTR_WRAPPER(pFunctionPtr)));
    return ret.second;
}

void Scope::UnRegisterFunctionPtr(void *pFooPtr) {
    FUNCTION_PTR_WRAPPER_MAP::iterator it = s_functionPtrWrapperMap.find(pFooPtr);
    if (it != s_functionPtrWrapperMap.end()) {
        delete it->second;
        s_functionPtrWrapperMap.erase(it);
    }
}

bool Scope::ReserveAlignedStackSopt(
#ifdef SCRIPT_DEBUGGER
    SYMBOL_DESC &symDesc, 
#endif // SCRIPT_DEBUGGER
    Type *pType, MEMORY_OFFSET &alignedStackOffset, ui32 &idx) {
    bool good = (pType != 0);
    if (good) {
        ui32 alignedSize = pType->GetValueSize();
        ui32 alignment = pType->GetAlignment();
        alignedStackOffset = (m_unalignedEndOfStack + (alignment - 1)) & -(i32)alignment;
        m_unalignedEndOfStack = alignedStackOffset + alignedSize;
        if (!GetParent()) {
            alignedStackOffset |= MEMORY_BLOCK_BIT_GLOBAL;
        }
#ifdef SCRIPT_DEBUGGER
        m_localVarTypes.push_back(LOCAL_VAR_DESCRIPTOR(symDesc, pType));
#else // SCRIPT_DEBUGGER
        m_localVarTypes.push_back(pType);
#endif // SCRIPT_DEBUGGER
        idx = m_localVarTypes.size() - 1;
        if (!GetParent()) {
            idx |= MEMORY_BLOCK_BIT_GLOBAL;
        }
    }
    return good;
}

Variable* Scope::FindTypeVariableDeclNode(SYMBOL_ID symId) {
    Variable *pVariable = 0;
    for (vector<Node*>::iterator it = m_expressions.begin(); it != m_expressions.end(); ++it) {
        NODE_TYPE type = (*it)->GetType();
        switch (type) {
            case TYPE_BOOL:
            case TYPE_I8:
            case TYPE_UI8:
            case TYPE_I16:
            case TYPE_UI16:
            case TYPE_I32:
            case TYPE_UI32:
            case TYPE_I64:
            case TYPE_UI64:
            case TYPE_FLOAT:
            case TYPE_DOUBLE:
            case TYPE_STRING:
            case TYPE_LIB:
                //case TYPE_STRUCT:
            case TYPE_AGGREGATE_TYPE:
            case TYPE_FUNCTION_REF_TYPE:
            case TYPE_ARRAY:
                //case TYPE_FUNCTION:
                if (pVariable = static_cast<Type*>(*it)->FindVariableDecNode(symId)) {
                    return pVariable;
                }
                break;
            case TYPE_ERROR_VAR:
                if ((*it)->GetSymbolId() == symId) {
                    return static_cast<Variable*>(*it);
                }
                break;
            default: // continue
                break;
        }
    }
    return 0;
}

ErrorVariable *Scope::GetErrorVariable() {
    ErrorVariable *pErrorVariable = 0;
    for (ui32 i = 0; i < m_expressions.size(); ++i) {
        if (ErrorVariable *pErrorVariable = m_expressions[i]->QueryType<ErrorVariable>(TYPE_ERROR_VAR)) {
            return pErrorVariable;
        }
    }
    return 0;
}

#ifdef SCRIPT_DEBUGGER

void Scope::SetDebuggerHandler(DebuggerHandler *pDebuggerHandler) {
    assert(m_pDebuggerHandler == 0);
    m_pDebuggerHandler = pDebuggerHandler;
}

bool Scope::SetBreakpoint(ui32 fileId, ui32 line, bool set) {
    BREAKPOINT_LOCATION bl = {fileId, line};
    if (set) {
        
        pair<unordered_set<ui64>::iterator, bool> res = m_activeBreakPointLines.insert(bl.breakpoint);
    }
    else {
        m_activeBreakPointLines.erase(bl.breakpoint);
    }
    return true;
}

void Scope::RegisterBreakPointLine(Node* pNode) {
    assert(pNode);
    assert(m_pAllBreakPointLinesByFileId);

    ui32 fileId = pNode->GetFileId(),
        line = pNode->GetLine();

    assert(line != 0);

    if (m_pAllBreakPointLinesByFileId->size() <= fileId) {
        m_pAllBreakPointLinesByFileId->resize(fileId + 1);
    }
    BREAK_POINT_LINES &breakPointLines = (*m_pAllBreakPointLinesByFileId)[fileId];
    if (breakPointLines.size() < line) {
        breakPointLines.resize(line/* line is not zero based: START_POSITION_OFFSET*/);
    }
    if (!breakPointLines[line - 1]) {
        breakPointLines[line - 1] = pNode;
    } // else multiple expressions on the same line, just skip it.
}

#endif // SCRIPT_DEBUGGER

/*************************************************************/
Function::Function(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) :
    Node(pParent, symDesc, type),
    m_pRefType(new FunctionRefType(this, symDesc)) {

    if (FunctionCallbackPtr *pFunctionCallbackPtr = new FunctionCallbackPtr(this, symDesc)) {
        SetRight(pFunctionCallbackPtr);
    }
}

Function::~Function() {
    delete m_pRefType;
}

RUNTIME_RETURN Function::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = m_pLeft->Run(param);
    if (ret == RT_RETURN) {
        ret = RT_NO_ERROR;
    }
    // set default return value?
    //switch (ret) {
    //    case RT_ERROR:
    //    case RT_EXCEPTION:
    //        break;
    //    default:
    //        if (param.m_pValue->GetType() == Value::NULL_TYPE) {
    //            param.m_pStackMemory->GetRunTimeError().SetError("Function return value is not set!", m_symDesc, 0);
    //            return RT_ERROR;
    //        }
    //        break;
    //}
    return ret;
}

void Function::SerializeBody(Serializer *pSerializer) {
    assert(pSerializer);
    pSerializer->Begin(TYPE_EMPTY);
    m_pRefType->GetReturnType()->Serialize(pSerializer);
    //m_pReturnType->Serialize(pSerializer);
    pSerializer->Begin(TYPE_EMPTY);
    pSerializer->WriteSymbol(m_symDesc);
    pSerializer->Begin(TYPE_PARENTHESIS);
    for (vector<SYMBOL_DESC>::iterator it = m_argOrder.begin(); it != m_argOrder.end();) {
        VARIABLE_REF_MAP::iterator itType = m_argVariables.find((*it).m_symId);
        itType->second->Serialize(pSerializer);
        ++it;
        if (it != m_argOrder.end()) {
            pSerializer->End(TYPE_COMMA);
            pSerializer->Begin(TYPE_EMPTY);
        }
    }
    pSerializer->End(TYPE_PARENTHESIS);
    pSerializer->Begin(TYPE_EMPTY);
    if (m_pLeft) {
        m_pLeft->Serialize(pSerializer);
    }
}

bool Function::PushArgInfo(SYMBOL_DESC &argSymDesc, Type *pArgType) {
    pair<VARIABLE_REF_MAP::iterator, bool> ret = m_argVariables.insert(pair<SYMBOL_ID, Type*>(argSymDesc.m_symId, pArgType));
    if (ret.second) {
        m_argOrder.push_back(argSymDesc);
#ifdef SCRIPT_DEBUGGER
        m_pRefType->PushArgDesc(LOCAL_VAR_DESCRIPTOR(argSymDesc, pArgType));
#else // SCRIPT_DEBUGGER
        m_pRefType->PushArgDesc(pArgType);
#endif // SCRIPT_DEBUGGER
    }
    return ret.second;
}

Type* Function::GetArgumentType(SYMBOL_ID argSymId) {
    VARIABLE_REF_MAP::iterator it = m_argVariables.find(argSymId);
    return (it != m_argVariables.end()) ? (*it).second : 0;
}

bool Function::SetArgumentVariable(SYMBOL_ID symId, Variable *pVariable) {
    VARIABLE_REF_MAP::iterator it = m_argVariables.find(symId);
    if (it != m_argVariables.end()) {
        it->second->PushVariable(pVariable);
        return true;
    }
    return false;
}

Type* Function::GetReturnType() { 
    return m_pRefType->GetReturnType();
}

bool Function::IsArgument(SYMBOL_ID symId) {
    VARIABLE_REF_MAP::iterator it = m_argVariables.find(symId);
    return (it != m_argVariables.end());
}

SYMBOL_ID Function::GetArgSymId(ui32 argIdx) {
    return (argIdx < m_argOrder.size()) ? m_argOrder[argIdx].m_symId : SYMBOL_ID_MAX;
}

Variable* Function::GetArgumentNode(SYMBOL_ID symId) {
    VARIABLE_REF_MAP::iterator it = m_argVariables.find(symId);
    return (it != m_argVariables.end()) ?
           (*it).second->GetLastDecVariable()->QueryType<Variable>(TYPE_RUNABLE)
           : 0;
}

ui32 Function::GetArgDWORDCount() {
    ui32 dwordCount = 0;
    for (ui32 i = 0; i < GetArgCount(); ++i) {
        if (Type *pType = GetArgumentType(GetArgSymId(i))) {
            ui32 size = pType->GetValueSize();
            if (!size) { // if it's a dynamic array, it should be passed by reference, this should not be the case, but for safety make it 4 bytes
                size = sizeof(ui32); // 
            }
            if (size % sizeof(ui32)) {
                ++dwordCount;
            }
            dwordCount += size / sizeof(ui32);
        }
    }
    return dwordCount;
}

FunctionCallbackPtr* Function::GetFunctionCallbackPtr() {
    FunctionCallbackPtr* pFunctionCallbackPtr = static_cast<FunctionCallbackPtr*>(GetRight());
    return pFunctionCallbackPtr;
}

void* Function::GetRawPtr(Runable::Param &param) {
    FunctionCallbackPtr *pFunctionCallbackPtr = GetFunctionCallbackPtr();
    return *pFunctionCallbackPtr->GetFooStubRef(*param.m_pMemory);
}

void Function::PushParameters(Runable::Param &param, vector<Value> &values) {
    Scope *pFunctionScope = static_cast<Scope*>(GetLeft());
    assert(pFunctionScope);
    param.m_pStackMemory->PushFrame(this, pFunctionScope, pFunctionScope->GetLocalVarDescriptors());
//#ifdef SCRIPT_DEBUGGER
    param.m_pStackMemory->SetCurRunningNode(this);
//#endif // SCRIPT_DEBUGGER
    for (ui32 i = 0; i < m_argOrder.size(); ++i) {
        Type *pResolvedType = 0;
        MEMORY_OFFSET offset = ResolveArg(i, pResolvedType);
        assert(INVALID_MEMORY_OFFSET != offset);
        PushParameter(param, i, values[i], pResolvedType);
    }
}

void Function::PushParameter(Runable::Param &param, ui32 paramIdx, Value &value, Type *pType) {
    switch (pType->GetPassBy()) {
        case Type::BY_VALUE: {
            Value argValueHolder(param.m_pStackMemory->GetMemoryBlockRef(paramIdx), pType);
            argValueHolder = value;
            break;
        }
        case Type::BY_REF: {
            MemoryBlockRef *pMemoryBlockRef = value.GetMemoryBlockRef(pType);
            param.m_pStackMemory->UpdateMemoryBlock(paramIdx, pType, pMemoryBlockRef->GetOffset(), pMemoryBlockRef->GetMemoryBlock());
            break;
        }
        case Type::BY_REF_TO_REF: // pass by double reference
            param.m_pStackMemory->UpdateMemoryBlockRef(paramIdx, value.GetMemoryBlockRef(pType));
            break;
        default:
            assert(0);
            break;
    }
}

MEMORY_OFFSET Function::ResolveArg(ui32 idx, Type *&pResolvedType) {
    MEMORY_OFFSET offset = INVALID_MEMORY_OFFSET;
    pResolvedType = 0;
    Scope *pFunctionScope = static_cast<Scope*>(GetLeft());
    assert(pFunctionScope);
    SYMBOL_ID symId = GetArgSymId(idx);
    if (Node *pArgNode = GetArgumentNode(symId)) {
        if (Variable *pArgVar = pArgNode->QueryType<Variable>(TYPE_VAR)) {
            pResolvedType = pArgVar->GetTypeNode();
            offset = pArgVar->GetAlignedOffset();
        }
    }
    return offset;
}

/*************************************************************/

SizeOf::SizeOf(Node *pParent, SYMBOL_DESC &symDesc) :
    Function(pParent, symDesc, TYPE_SIZEOF) {
    SymbolStore *pSymbolStore = SymbolStore::GetSymbolStore();
    SYMBOL_ID symId;
    pSymbolStore->GetSymbolId(string("ui32"), symId);
    SYMBOL_DESC _symDesc(symId);// = { 0 };
    if (Type *pReturnType = new Type(this, _symDesc, TYPE_UI32)) {
        m_pRefType->SetReturnType(pReturnType);
        PushArgInfo(_symDesc, 0);
    }
}

SizeOf::~SizeOf() {

}

RUNTIME_RETURN SizeOf::Run(Runable::Param &param) {
    MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(0);
    //TODO("Fix it to get type size!");
    // if array only then get memory type, if it's a struct then get next member and substruct from array memory offset
    Type *pType = pMemoryBlockRef->GetReferenceTypeNode();
    ui32 value = pType->GetValueSize();
    RUNTIME_RETURN ret = RT_NO_ERROR;
_PROTECT_BEGIN
    if (pType->GetType() == TYPE_STRING) {
        value = pMemoryBlockRef->GetMemoryBlock()->GetPtr<string>(pMemoryBlockRef->GetOffset())->length();
    }
    else if (value == 0) {
        if (pType->GetType() == TYPE_ARRAY) {
            value = pMemoryBlockRef->GetMemoryBlock()->GetSize() - pMemoryBlockRef->GetOffset();
        }
    }
    //ui32 value = pMemoryBlockRef->GetMemoryBlock()->GetSize() - pMemoryBlockRef->GetOffset();
    param.m_pValue->Set(value);
_PROTECT_END(RT_ERROR)
    return ret;
}

void SizeOf::PushParameters(Runable::Param &param, vector<Value> &values) {
    vector<LOCAL_VAR_DESCRIPTOR> desc;
    Value &value = values[0];
    Type *pType = value.GetTypeNode();
    SYMBOL_DESC symDesc;
    pType->GetSymbolDesc(symDesc);
    Type *pStackType = Scope::GetDummyTypeNode(TYPE_UI32); 
    // since a place holder value gets reserved on the stack pass fake type, 
    // so in case of passing struct ref member or array ref item, UpdateMemoryBlock() handles it accordingly
#ifdef SCRIPT_DEBUGGER
    desc.push_back(LOCAL_VAR_DESCRIPTOR(symDesc, pStackType));
#else // SCRIPT_DEBUGGER
    desc.push_back(pStackType);
#endif // SCRIPT_DEBUGGER
    param.m_pStackMemory->PushFrame(this, 0, desc);
    MemoryBlockRef *pMemoryBlockRef = value.GetMemoryBlockRef(pType);
    param.m_pStackMemory->UpdateMemoryBlock(0, pType, pMemoryBlockRef->GetOffset(), pMemoryBlockRef->GetMemoryBlock());
}

bool SizeOf::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    return false;
}

void SizeOf::SerializeBody(Serializer *pSerializer) {

}

/*************************************************************/
BuiltIn1Arg::~BuiltIn1Arg() {
    delete m_pArgType;
}

void BuiltIn1Arg::PushParameters(Runable::Param &param, vector<Value> &values) {
    vector<LOCAL_VAR_DESCRIPTOR> desc;
    assert(values.size());
    assert(m_pArgType);
    SYMBOL_DESC symDesc;
    Type *pType = values[0].GetTypeNode();
    //m_pArgType->GetSymbolDesc(symDesc);
    pType->GetSymbolDesc(symDesc);
#ifdef SCRIPT_DEBUGGER
    desc.push_back(LOCAL_VAR_DESCRIPTOR(symDesc, pType/*m_pArgType*/));
#else // SCRIPT_DEBUGGER
    desc.push_back(pType/*m_pArgType*/);
#endif // SCRIPT_DEBUGGER
    param.m_pStackMemory->PushFrame(this, 0, desc);
    //PushParameter(param, 0, values[0], pType/*m_pArgType*/);
    MemoryBlockRef *pMemoryBlockRef = values[0].GetMemoryBlockRef(pType);
    param.m_pStackMemory->UpdateMemoryBlock(0, pType, pMemoryBlockRef->GetOffset(), pMemoryBlockRef->GetMemoryBlock());
}

void BuiltIn1Arg::SerializeBody(Serializer *pSerializer) {

}

/*************************************************************/

Arr2Str::Arr2Str(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc, NODE_TYPE type) :
    BuiltIn1Arg(pParent, symDesc, type) {
    //SYMBOL_DESC _symDesc = { 0 };
    SYMBOL_ID symId;
    pSymbolStore->GetSymbolId(string("string"), symId);
    SYMBOL_DESC _symDesc(symId);// = { 0 };
    Type *pReturnType = new Type(this, _symDesc, TYPE_STRING);
    pReturnType->SetLeft(new Reference(pReturnType, _symDesc));
    m_pRefType->SetReturnType(pReturnType);

    pSymbolStore->GetSymbolId(string("array"), symId);
    _symDesc.m_symId = symId;
    m_pArgType = new ArrayType(pSymbolStore, this, _symDesc);
    pSymbolStore->GetSymbolId((TYPE_ARR2STR == type) ? string("ui8") : string("ui16"), symId);
    _symDesc.m_symId = symId;

    static_cast<ArrayType*>(m_pArgType)->SetType(new Type(m_pArgType, _symDesc, (TYPE_ARR2STR == type) ? TYPE_UI8 : TYPE_UI16));
    m_pArgType->SetLeft(new Reference(m_pArgType, _symDesc));
    PushArgInfo(_symDesc, m_pArgType);
}

RUNTIME_RETURN Arr2Str::Run(Runable::Param &param) {
    // prototype: function string@ Arr2Str(array<ui8>@ arr);
    MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(0);
    MemoryBlock *pMemoryBlock = pMemoryBlockRef->GetMemoryBlock();
    RUNTIME_RETURN ret = RT_NO_ERROR;
_PROTECT_BEGIN
    if (char *pArray = pMemoryBlock->GetPtr<char>(pMemoryBlockRef->GetOffset())) {
        ArrayType *pArrayType = static_cast<ArrayType*>(pMemoryBlockRef->GetReferenceTypeNode());// static_cast<ArrayType*>(m_pArgType);
        ui32 valueSize = pArrayType->GetTypeNode()->GetValueSize();
        ui32 size = 0;
        if (ui32 arraySize = pArrayType->GetTypeSize()) {
            size = arraySize / valueSize;
        }
        else {
            size = (pMemoryBlock->GetSize() - pMemoryBlockRef->GetOffset()) / valueSize;
        }
        SetParam(param, pArray, size);
    }
    else {
        param.m_pValue->Set("", 0);
    }
_PROTECT_END(RT_ERROR)
    return ret;
}

void Arr2Str::SetParam(Runable::Param &param, void *pData, ui32 size) {
    ui32 len = strlen((char*)pData);
    param.m_pValue->Set((char*)pData, ((size > len) || (size == 0)) ? len : size);
}

/*************************************************************/

WArr2Str::WArr2Str(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc) :
    Arr2Str(pSymbolStore, pParent, symDesc, TYPE_WARR2STR) {
}

WArr2Str::~WArr2Str() {

}

void WArr2Str::SetParam(Runable::Param &param, void *pData, ui32 size) {
    ui32 len = SysStringLen((OLECHAR*)pData);
    size = ((size > len) || (size == 0)) ? len : size;
    BSTR wStr = SysAllocStringLen((OLECHAR*)pData, size);
    _bstr_t bstr(wStr, false);
    param.m_pValue->Set((char*)(LPCSTR)bstr, size);
}

/*************************************************************/

Str2Arr::Str2Arr(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc, NODE_TYPE type) :
    BuiltIn1Arg(pParent, symDesc, type) {
    //SYMBOL_DESC _symDesc = { 0 };
    SYMBOL_ID symId;
    pSymbolStore->GetSymbolId(string("array"), symId);
    SYMBOL_DESC _symDesc(symId);// = { 0 };

    ArrayType *pReturnType = new ArrayType(pSymbolStore, this, _symDesc);
    pSymbolStore->GetSymbolId((TYPE_STR2ARR == type) ? string("ui8") : string("ui16"), symId);
    _symDesc.m_symId = symId;
    pReturnType->SetType(new Type(pReturnType, _symDesc, (TYPE_STR2ARR == type) ? TYPE_UI8 : TYPE_UI16));
    pReturnType->SetLeft(new Reference(pReturnType, _symDesc));
    m_pRefType->SetReturnType(pReturnType);

    pSymbolStore->GetSymbolId(string("string"), symId);
    _symDesc.m_symId = symId;
    m_pArgType = new Type(this, _symDesc, TYPE_STRING);
    m_pArgType->SetLeft(new Reference(m_pArgType, _symDesc));
    PushArgInfo(_symDesc, m_pArgType);
}


RUNTIME_RETURN Str2Arr::Run(Runable::Param &param) {
    // prototype: function array<ui8>@ Str2Arr(string@ str);
    MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(0);
    RUNTIME_RETURN ret = RT_NO_ERROR;
_PROTECT_BEGIN
    if (string *pString = pMemoryBlockRef->GetMemoryBlock()->GetPtr<string>(pMemoryBlockRef->GetOffset())) {
        SetParam(param, pString, pString->length());
    }
    else {
        //??
    }
_PROTECT_END(RT_ERROR)
    return ret;
}

void Str2Arr::SetParam(Runable::Param &param, void *pData, ui32 size) {
    MemoryBlockRef *pOutMemoryBlockRef = Memory::Allocate(m_pRefType->GetReturnType(), 0, size + 1);
    char *p = pOutMemoryBlockRef->GetMemoryBlock()->GetPtr<char>(0);
    memcpy(p, ((string*)pData)->c_str(), size);
    p[size] = 0;
    param.m_pValue->SetRef(pOutMemoryBlockRef);
    pOutMemoryBlockRef->Release();
}

/*************************************************************/

Str2WArr::Str2WArr(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc) :
    Str2Arr(pSymbolStore, pParent, symDesc, TYPE_STR2WARR) {
}

void Str2WArr::SetParam(Runable::Param &param, void *pData, ui32 size) {
    _bstr_t bstr(((string*)pData)->c_str());
    size = SysStringByteLen(bstr.GetBSTR());
    MemoryBlockRef *pOutMemoryBlockRef = Memory::Allocate(m_pRefType->GetReturnType(), 0, size + 2);
    char *p = pOutMemoryBlockRef->GetMemoryBlock()->GetPtr<char>(0);
    memcpy(p, bstr.GetBSTR(), size);
    *(ui16*)&p[size] = 0;
    param.m_pValue->SetRef(pOutMemoryBlockRef);
    pOutMemoryBlockRef->Release();
}

/*************************************************************/

RUNTIME_RETURN Lock::Run(Runable::Param &param) {
    MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(0);
    pMemoryBlockRef->GetMemoryBlock()->Lock();
    param.m_pValue->SetRef(pMemoryBlockRef);
    return RT_NO_ERROR;
}

void Lock::PushParameters(Runable::Param &param, vector<Value> &values) {
    vector<LOCAL_VAR_DESCRIPTOR> desc;
    Value &value = values[0];
    Type *pType = value.GetTypeNode();
    SYMBOL_DESC symDesc;
    pType->GetSymbolDesc(symDesc);
    Type *pStackType = Scope::GetDummyTypeNode(TYPE_UI32);
    // since a place holder value gets reserved on the stack pass fake type, 
    // so in case of passing struct ref member or array ref item, UpdateMemoryBlock() handles it accordingly
#ifdef SCRIPT_DEBUGGER
    desc.push_back(LOCAL_VAR_DESCRIPTOR(symDesc, pStackType));
#else // SCRIPT_DEBUGGER
    desc.push_back(pStackType);
#endif // SCRIPT_DEBUGGER
    param.m_pStackMemory->PushFrame(this, 0, desc);
    MemoryBlockRef *pMemoryBlockRef = value.GetMemoryBlockRef(pType);
    param.m_pStackMemory->UpdateMemoryBlock(0, pType, pMemoryBlockRef->GetOffset(), pMemoryBlockRef->GetMemoryBlock());
}

bool Lock::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    return false;
}

/*************************************************************/
RUNTIME_RETURN UnLock::Run(Runable::Param &param) {
    MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(0);
    pMemoryBlockRef->GetMemoryBlock()->UnLock();
    param.m_pValue->SetRef(pMemoryBlockRef);
    return RT_NO_ERROR;
}

/*************************************************************/

RUNTIME_RETURN Cast::Run(Runable::Param &param) {
    Memory &memory = *param.m_pMemory;
    MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(0);
    MemoryBlock *pOutMemoryBlock = 0;
    Type *pType = pMemoryBlockRef->GetReferenceTypeNode();
    ui32 offset = pMemoryBlockRef->GetOffset();
    Type *pOutType = param.m_pValue->GetTypeNode();
    RUNTIME_RETURN ret = RT_NO_ERROR;
    assert(pOutType);

    // Get argument memory and return as out parameter type
    if (pType->IsReference()) {
_PROTECT_BEGIN
            pOutMemoryBlock = memory.GetReferencedMemoryBlock(pType, pMemoryBlockRef->GetMemoryBlock(), offset/*new offset returned*/);
_PROTECT_END(RT_ERROR)
#ifdef MEMORY_ACCESS_EXCEPTION
        if (ret == RT_ERROR) {
            return ret;
        }
#endif // MEMORY_ACCESS_EXCEPTION
    }
    else {
        pOutMemoryBlock = pMemoryBlockRef->GetMemoryBlock();
        pOutMemoryBlock->AddRef();
    }
    MemoryBlockRef *pOutMemoryBlockRef = Memory::Allocate(pOutType, offset, pOutMemoryBlock);
    param.m_pValue->SetRef(pOutMemoryBlockRef);
    pOutMemoryBlock->Release();
    pOutMemoryBlockRef->Release();
    return ret;
}

void Cast::PushParameters(Runable::Param &param, vector<Value> &values) {
    vector<LOCAL_VAR_DESCRIPTOR> desc;
    Value &value = values[0];
    Type *pType = value.GetTypeNode();
    SYMBOL_DESC symDesc;
    pType->GetSymbolDesc(symDesc);
    Type *pStackType = Scope::GetDummyTypeNode(TYPE_UI32);
    // since a place holder value gets reserved on the stack pass fake type, 
    // so in case of passing struct ref member or array ref item, UpdateMemoryBlock() handles it accordingly
#ifdef SCRIPT_DEBUGGER
    desc.push_back(LOCAL_VAR_DESCRIPTOR(symDesc, pStackType));
#else // SCRIPT_DEBUGGER
    desc.push_back(pStackType);
#endif // SCRIPT_DEBUGGER
    param.m_pStackMemory->PushFrame(this, 0, desc);
    MemoryBlockRef *pMemoryBlockRef = value.GetMemoryBlockRef(pType);
    param.m_pStackMemory->UpdateMemoryBlock(0, pType, pMemoryBlockRef->GetOffset(), pMemoryBlockRef->GetMemoryBlock());
}

bool Cast::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    return false;
}

/*************************************************************/

PrePostExpressions::~PrePostExpressions() {
    delete m_pExpression;
}

RUNTIME_RETURN PrePostExpressions::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = RunPre(param);
    if (ret != RT_NO_ERROR) {
        return ret;
    }

    ret = m_pExpression->Run(param);

    if (ret == RT_NO_ERROR) {
        ret = RunPost(param);
    }
    return ret;
}

RUNTIME_RETURN PrePostExpressions::RunPre(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    for (vector<Operator*>::iterator it = m_preExpressions.begin(); it != m_preExpressions.end(); ++it) {
        Param tempParam(0, param);
        ret = (*it)->Run(tempParam);
        if (ret != RT_NO_ERROR) {
            return ret;
        }
    }
    return ret;
}

RUNTIME_RETURN PrePostExpressions::RunPost(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    if (ret == RT_NO_ERROR) {
        for (vector<Operator*>::iterator it = m_postExpressions.begin(); it != m_postExpressions.end(); ++it) {
            Param tempParam(0, param);
            ret = (*it)->Run(tempParam);
            if (ret != RT_NO_ERROR) {
                return ret;
            }
        }
    }
    return ret;
}

bool PrePostExpressions::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    return m_pExpression->IsExpression(pReturnType, pPrePostExp, error);
}

void PrePostExpressions::PushPreExpression(Operator* pOperator) {
    m_preExpressions.push_back(pOperator);
}

void PrePostExpressions::PushPostExpression(Operator* pOperator) {
    m_postExpressions.push_back(pOperator);
}

void PrePostExpressions::Serialize(Serializer *pSerializer) {
    m_pExpression->Serialize(pSerializer);
}

/*************************************************************/

RUNTIME_RETURN Reference::Run(Runable::Param &param) {
    return RT_NO_ERROR;
}

bool Reference::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    _ERROR(INTERNAL_ERROR);
    return false;
}

/*************************************************************/

RUNTIME_RETURN Return::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_RETURN;
    if (m_pRight) {
        assert(param.m_pValue->GetType() == Value::NULL_TYPE);
        Value retValue;
        Param retParam(&retValue, param);
        ret = m_pRight->Run(retParam);
        if (ret == RT_NO_ERROR) {
            //check return by type
            if (retValue.GetType() != Value::NULL_TYPE) {
                param.m_pValue->Set(retValue);
            }
            ret = RT_RETURN;
        }
    }
    else {
        param.m_pValue->SetType(Value::NULL_TYPE, 0);
    }
    return ret;
}

bool Return::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(pReturnType == 0);

    if (!m_pRight) { // no return, will validate at runtime if result is needed
        return true;
    }

    Type *pRightType = 0;
    if (!m_pRight->IsExpression(pRightType, pPrePostExp, error)) {
        return false;
    }

    Function *pFunction = GetFunctionNode();
    pReturnType = pFunction->GetReturnType();

    switch (pReturnType->GetPassBy()) {
        case Type::BY_REF_TO_REF:
            if (Variable *pRightVariable = m_pRight->QueryType<Variable>(TYPE_VAR)) {
                Variable *pSubVariable = pRightVariable->GetSubVariable();
                if (!pSubVariable->IsFunctionCall()) {
                    if (pSubVariable->GetParent()->GetType() == TYPE_MEMBER_ACCESS) {
                        _ERROR(CANNOT_RETURN_MEMBER_AS_REFERENCE_TO_REFERENCE);
                        return false;
                    }
                    else if (pSubVariable->GetParent()->GetType() == TYPE_SUBSCRIPT) {
                        _ERROR(CANNOT_RETURN_ARRAY_ITEM_AS_REFERENCE_TO_REFERENCE);
                        return false;
                    } //fall through
                }
                else {
                    _ERROR(CANNOT_RETURN_REFERENCE_TO_REFERENCE_OF_FUNCTION_RETURN);
                    return false;
                }
            }
            else {
                _ERROR(CANNOT_RETURN_REFERENCE_TO_REFERENCE_OF_EXPRESSION_RESULT);
                return false;
            }
        case Type::BY_REF:
            switch (m_pRight->GetType()) {
                case TYPE_CONST:
                case TYPE_CONST_BOOL:
                case TYPE_CONST_NUM:
                    _ERROR(CANNOT_RETURN_REFERENCE_TO_CONST);
                    return false;
            }
            if (!pReturnType->IsEqualType(pRightType)) {
                _ERROR(TYPES_MISMATCHED);
                return false;
            }
            break;
        default:
            if (!pReturnType->IsRelatedType(pRightType)) {
                _ERROR(TYPES_MISMATCHED);
                return false;
            }
            break;
    }
    return true;
}

bool Return::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();
    if (m_pRight == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        SetRight(pPrePostExpressions);
        return true;
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

void Return::SerializeBody(Serializer *pSerializer) {
    if (m_pRight) {
        pSerializer->Begin(TYPE_EMPTY);
        m_pRight->Serialize(pSerializer);
    }
}

/*************************************************************/

RUNTIME_RETURN Break::Run(Runable::Param &param) {
    return RT_BREAK;
}

bool Break::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    _ERROR(INTERNAL_ERROR);
    return false;
}

/*************************************************************/

RUNTIME_RETURN Continue::Run(Runable::Param &param) {
    return RT_CONTINUE;
}

bool Continue::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    _ERROR(INTERNAL_ERROR);
    return false;
}

/*************************************************************/

FunctionCall::~FunctionCall() {
    Delete(m_arguments);
}

RUNTIME_RETURN FunctionCall::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    Memory &memory = *param.m_pMemory;
    Function *pFunction = 0;
    vector<Value> values;
    ret = EvalParamExpresions(param, values);
    if (ret != RT_NO_ERROR) {
        return ret;
    }
    pFunction = GetFunctionImpl(param);
    if (pFunction) {
_PROTECT_BEGIN
        pFunction->PushParameters(param, values);
        ret = pFunction->Run(param);
_PROTECT_END(RT_EXCEPTION)
        if (ret == RT_EXCEPTION) {
            ret = RT_ERROR;
        }
        param.m_pStackMemory->PopFrame();
        if (m_pRight) {
            Value _value;
_PROTECT_BEGIN
            Scope *pFunctionScope = static_cast<Scope*>(pFunction->GetLeft());
            param.m_pStackMemory->PushResultMemoryBlockRef(pFunction, pFunctionScope, param.m_pValue->GetMemoryBlockRef(pFunction->GetReturnType()));
            Runable::Param _param(&_value, param);
            ret = m_pRight->Run(_param);
            param.m_pStackMemory->PopResultMemoryBlockRef();
            param.m_pValue->Set(*_param.m_pValue);
_PROTECT_END(RT_EXCEPTION)
            if (ret == RT_EXCEPTION) {
                ret = RT_ERROR;
            }
        }
        return ret;
    }
    return RT_ERROR;
}


bool FunctionCall::IsExpression(Type *&pReturnType, PrePostExpressions *&, Error &error) {
    FunctionRefType *pFunctionRefType = GetFunctionRefTypeNode();
    assert(pFunctionRefType);
    NODE_TYPE functionType = pFunctionRefType->GetParent()->GetType();
    switch (functionType) {
        case TYPE_CAST:
        case TYPE_SIZEOF:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
            if (m_arguments.size() != 1) {
                _ERROR(EXPECTING_SINGLE_ARGUMENT);
                return false;
            }
            break;
        default:
            if (pFunctionRefType->GetArgCount() != m_arguments.size()) {
                _ERROR(UNEXPECTED_NUMBER_OF_ARGUMENTS);
                return false;
            }
            break;
    }

    switch (functionType) {
        case TYPE_CAST: {
                Node *pExpNode = m_arguments[0];
                PrePostExpressions *pPrePostExp = 0;
                if (!pExpNode->IsExpression(pReturnType, pPrePostExp, error)) {
                    return false;
                }
                if (pPrePostExp) {
                    pPrePostExp->SetExpressionNode(pExpNode);
                    if (!pExpNode->GetParent()->InjectPrePostExpressions(pPrePostExp)) {
                        SYMBOL_DESC symDesc;
                        pExpNode->GetSymbolDesc(symDesc);
                        _ERROR_EX(INTERNAL_ERROR, symDesc);
                        return false;
                    }
                }
                pReturnType = pFunctionRefType->GetReturnType();
                if (ArrayType *pArrayType = pReturnType->QueryType<ArrayType>(TYPE_ARRAY)) {
                    if (pArrayType->GetDynExpression()) {
                        _ERROR(CANNOT_TYPECAST_TO_DYNAMIC_ARRAY);
                        return false;
                    }
                }
                else if (pReturnType->GetType() == TYPE_STRING) {
                    _ERROR(CANNOT_TYPECAST_TO_STRING);
                    return false;
                }
            }
            return true;
        case TYPE_SIZEOF:
            if (Variable *pArgVariable = m_arguments[0]->QueryType<Variable>(TYPE_VAR)) {
                Variable *pSubVariable = pArgVariable->GetSubVariable();
                if (!pSubVariable->GetLeft() && !pSubVariable->IsFunctionCall()) {
                    PrePostExpressions *pPrePostExp = 0;
                    if (!pSubVariable->IsExpression(pReturnType, pPrePostExp, error)) {
                        return false;
                    }
                    pReturnType = pFunctionRefType->GetReturnType();
                    assert(pPrePostExp == 0);
                    return true;
                }
            }
            _ERROR(EXPECTING_VARIABLE);
            return false;
        case TYPE_LOCK:
        case TYPE_UNLOCK: {
            Node *pExpNode = m_arguments[0];
            PrePostExpressions *pPrePostExp = 0;
            if (!pExpNode->IsExpression(pReturnType, pPrePostExp, error)) {
                return false;
            }
            if (pPrePostExp) {
                pPrePostExp->SetExpressionNode(pExpNode);
                if (!pExpNode->GetParent()->InjectPrePostExpressions(pPrePostExp)) {
                    SYMBOL_DESC symDesc;
                    pExpNode->GetSymbolDesc(symDesc);
                    _ERROR_EX(INTERNAL_ERROR, symDesc);
                    return false;
                }
            }
            return true;
//            if (Variable *pArgVariable = m_arguments[0]->QueryType<Variable>(TYPE_VAR)) {
//                Variable *pSubVariable = pArgVariable->GetSubVariable();
//                //Type *pVarType = pSubVariable->GetTypeNode();
//                //if ((pVarType->GetType() == TYPE_ARRAY) && (pSubVariable->GetRight())) { // subscript object
//                //    pVarType = static_cast<ArrayType*>(pVarType)->GetTypeNode();
//                //}
//                if (!pSubVariable->GetLeft() && !pSubVariable->IsFunctionCall()) {
//                    PrePostExpressions *pPrePostExp = 0;
//                    if (!pSubVariable->IsExpression(pReturnType, pPrePostExp, error)) {
//                        return false;
//                    }
//                    switch (functionType) {
//                        case TYPE_LOCK:
//                        case TYPE_UNLOCK:
////                            pReturnType = pVarType;
//                            break;
//                        case TYPE_CAST:
//                            if (pReturnType->GetPassBy() == Type::BY_REF_TO_REF) {
//                                _ERROR(CANNOT_TYPECAST_REFERENCE_TO_REFERENCE);
//                                return false;
//                            }
//                            pReturnType = pFunctionRefType->GetReturnType();
//                            if (ArrayType *pArrayType = pReturnType->QueryType<ArrayType>(TYPE_ARRAY)) {
//                                if (pArrayType->GetDynExpression()) {
//                                    _ERROR(CANNOT_TYPECAST_TO_DYNAMIC_ARRAY);
//                                    return false;
//                                }
//                            }
//                            //{
//                            //    Cast *pCast = pFunctionRefType->GetParent()->QueryType<Cast>(TYPE_CAST);
//                            //    assert(pCast);
//                            //}
//                            //if (pVarType->GetPassBy() == Type::BY_REF_TO_REF) {
//                            //    _ERROR(CANNOT_TYPECAST_REFERENCE_TO_REFERENCE);
//                            //    return false;
//                            //}
//                            //else {
//                            //    pReturnType = pVarType;
//                            //}
//                            break;
//                        case TYPE_SIZEOF:
//                        default:
//                            pReturnType = pFunctionRefType->GetReturnType();
//                            break;
//                    }
//                    return true;
//                }
//            }
//
//            _ERROR(EXPECTING_VARIABLE);
//            return false;
        }
        default:
            break;
    }

    //if (pFunctionRefType->GetArgCount() != m_arguments.size()) {
    //    _ERROR(UNEXPECTED_NUMBER_OF_ARGUMENTS);
    //    return false;
    //}

    for (ui32 i = 0; i < m_arguments.size(); ++i) {
        Node *pExpNode = m_arguments[i];
        PrePostExpressions *pPrePostExp = 0;
        Type *pExpReturnType = 0,
             *pArgType = pFunctionRefType->GetArgType(i + 1);

        if (!pExpNode->IsExpression(pExpReturnType, pPrePostExp, error)) {
            if (pPrePostExp) {
                delete pPrePostExp;
            }
            return false;
        }
        if (pPrePostExp) {
            pPrePostExp->SetExpressionNode(pExpNode);
            if (!pExpNode->GetParent()->InjectPrePostExpressions(pPrePostExp)) {
                SYMBOL_DESC symDesc;
                pExpNode->GetSymbolDesc(symDesc);
                _ERROR_EX(INTERNAL_ERROR, symDesc);
                return false;
            }
        }

        switch (pArgType->GetPassBy()) {
            case Type::BY_REF_TO_REF:
                if (Variable *pRightVariable = pExpNode->QueryType<Variable>(TYPE_VAR)) {
                    Variable *pSubVariable = pRightVariable->GetSubVariable();
                    if (!pSubVariable->IsFunctionCall()) {
                        if (pSubVariable->GetParent()->GetType() == TYPE_MEMBER_ACCESS) {
                            string errorTxt(SCRIPT_SYM_TO_STR(CANNOT_PASS_MEMBER_AS_REFERENCE_TO_REFERENCE)"_ARG_" + _bstr_t(_variant_t(i)));
                            error.SetErrorEx(CANNOT_PASS_MEMBER_AS_REFERENCE_TO_REFERENCE, errorTxt.c_str(), __FILE__, __FUNCTION__, __LINE__, m_symDesc);
                            return false;
                        }
                        else if (pSubVariable->GetParent()->GetType() == TYPE_SUBSCRIPT) {
                            string errorTxt(SCRIPT_SYM_TO_STR(CANNOT_PASS_ARRAY_ITEM_AS_REFERENCE_TO_REFERENCE)"_ARG_" + _bstr_t(_variant_t(i)));
                            error.SetErrorEx(CANNOT_PASS_ARRAY_ITEM_AS_REFERENCE_TO_REFERENCE, errorTxt.c_str(), __FILE__, __FUNCTION__, __LINE__, m_symDesc);
                            return false;
                        } //fall through
                    }
                    else {
                        string errorTxt(SCRIPT_SYM_TO_STR(CANNOT_PASS_REFERENCE_TO_REFERENCE_OF_FUNCTION_RETURN)"_ARG_" + _bstr_t(_variant_t(i)));
                        error.SetErrorEx(CANNOT_PASS_REFERENCE_TO_REFERENCE_OF_FUNCTION_RETURN, errorTxt.c_str(), __FILE__, __FUNCTION__, __LINE__, m_symDesc);
                        return false;
                    }
                }
                else {
                    string errorTxt(SCRIPT_SYM_TO_STR(CANNOT_PASS_REFERENCE_TO_REFERENCE_OF_EXPRESSION_RESULT)"_ARG_" + _bstr_t(_variant_t(i)));
                    error.SetErrorEx(CANNOT_PASS_REFERENCE_TO_REFERENCE_OF_EXPRESSION_RESULT, errorTxt.c_str(), __FILE__, __FUNCTION__, __LINE__, m_symDesc);
                    return false;
                }
            case Type::BY_REF:
                switch (pExpReturnType->GetType()) {
                    case TYPE_CONST:
                    case TYPE_CONST_BOOL:
                    case TYPE_CONST_NUM:
                        string errorTxt(SCRIPT_SYM_TO_STR(CANNOT_PASS_CONST_BY_REFERENCE)"_ARG_" + _bstr_t(_variant_t(i)));
                        error.SetErrorEx(CANNOT_PASS_CONST_BY_REFERENCE, errorTxt.c_str(), __FILE__, __FUNCTION__, __LINE__, m_symDesc);
                        return false;
                }
                if (!pExpReturnType->IsEqualType(pArgType)) {
                    string errorTxt(SCRIPT_SYM_TO_STR(TYPES_MISMATCHED)"_ARG_" + _bstr_t(_variant_t(i)));
                    error.SetErrorEx(TYPES_MISMATCHED, errorTxt.c_str(), __FILE__, __FUNCTION__, __LINE__, m_symDesc);
                    return false;
                }
                break;
            default:
            case Type::BY_VALUE:
                if (!pExpReturnType->IsRelatedType(pArgType)) {
                    string errorTxt(SCRIPT_SYM_TO_STR(TYPES_MISMATCHED)"_ARG_" + _bstr_t(_variant_t(i)));
                    error.SetErrorEx(TYPES_MISMATCHED, errorTxt.c_str(), __FILE__, __FUNCTION__, __LINE__, m_symDesc);
                    return false;
                }
                break;

        }
    }

    if (m_pRight) {
        PrePostExpressions *pPrePostExp = 0;
        if (!m_pRight->IsExpression(pReturnType, pPrePostExp, error)) {
            return false;
        }
    }
    else {
        pReturnType = pFunctionRefType->GetReturnType();
    }

    return true;
}

bool FunctionCall::PushArgument(Node *pArgNode) {
    bool good = (pArgNode != 0);
    if (good) {
        m_arguments.push_back(pArgNode);
    }
    return good;
}

bool FunctionCall::UpdateArgument(Node *pArgNode) {
    if (m_arguments.size()) {
        m_arguments.pop_back();
        m_arguments.push_back(pArgNode);
        return true;
    }
    return false;

}
Node* FunctionCall::GetArgNode(ui32 idx) {
    return (m_arguments.size() > idx) ? m_arguments[idx] : 0;
}

FunctionRefType* FunctionCall::GetFunctionRefTypeNode() {
    Node *pParent = GetParent();
    FunctionRefType *pFunctionRefType = 0;
    ui32 i = 0;
    while (pParent) {
        if (pParent->GetType() == TYPE_FUNCTION_CALL) {
            ++i;
            pParent = pParent->GetParent();
        }
        else {
            // check if it's array subscript or varaible
            switch (pParent->GetType()) {
                case TYPE_VAR:
                case TYPE_LIB_VAR:
                    if (Variable *pVariable = pParent->QueryType<Variable>(TYPE_RUNABLE)) {
                        pFunctionRefType = pVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
                    }
                    break;
                case TYPE_SUBSCRIPT: {
                    ArrayType *pArrayType = static_cast<SubscriptNode*>(pParent)->GetArrayType();
                    pFunctionRefType = pArrayType->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
                    break;
                }
                default:
                    break;
            }
            if (pFunctionRefType) {
                while (i) {
                    pFunctionRefType = pFunctionRefType->GetReturnType()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
                    --i;
                }
            }
            return pFunctionRefType;
        }
    }
    return 0;
}

bool FunctionCall::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();
    for (ui32 i = 0; i < m_arguments.size(); ++i) {
        if (m_arguments[i] == pExpNode) {
            pPrePostExpressions->SetParent(this);
            pExpNode->SetParent(pPrePostExpressions);
            m_arguments[i] = pPrePostExpressions;
            return true;
        }
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

MEMORY_OFFSET FunctionCall::ResolveArg(Function *pFunction, ui32 idx, Type *&pResolvedType) {
    MEMORY_OFFSET offset = INVALID_MEMORY_OFFSET;
    pResolvedType = 0;
    Scope *pFunctionScope = static_cast<Scope*>(pFunction->GetLeft());
    assert(pFunctionScope);
    SYMBOL_ID symId = pFunction->GetArgSymId(idx);
    if (Node *pArgNode = pFunction->GetArgumentNode(symId)) {
        if (Variable *pArgVar = pArgNode->QueryType<Variable>(TYPE_VAR)) {
            pResolvedType = pArgVar->GetTypeNode();
            offset = pArgVar->GetAlignedOffset();
        }
    }
    return offset;
}

RUNTIME_RETURN FunctionCall::EvalParamExpresions(Runable::Param &param, vector<Value> &values) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    assert(values.size() == 0);
    for (ui32 i = 0; i < m_arguments.size(); ++i) {
        values.push_back(Value());
        Param _param(&values.back(), param);
        ret = m_arguments[i]->Run(_param);
        if (ret != RT_NO_ERROR) {
            break;
        }
    }
    return ret;
}

Function* FunctionCall::GetFunctionImpl(Runable::Param &param) {
    if (Variable *pVariable = GetParent()->QueryType<Variable>(TYPE_VAR)) {
        SYMBOL_ID symId;
        if (FunctionRefType *pFunctionRefType = pVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
            symId = pFunctionRefType->GetSymbolId();
        }
        else {
            param.m_pStackMemory->GetRunTimeError().SetError("No function implementation.", m_symDesc, 0, param.m_pStackMemory);
            return 0;
        }
        Memory &memory = *param.m_pMemory;
        if (pVariable->GetAlignedOffset() == INVALID_MEMORY_OFFSET) { // this is reference to a function
            if (Function *pFunction = memory.GetRootScope()->ResolveFunction(symId)) {
                return pFunction;
            }
        }
        else {
            MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(pVariable->GetVarIdx());
            ui32 offset = pMemoryBlockRef->GetOffset();
            void **pFunctionPtrMemory = pMemoryBlockRef->GetMemoryBlock()->GetPtr<void*>(offset);
            if (*pFunctionPtrMemory) {
                FunctionCallbackPtr *pFunctionCallbackPtr = FUNCTION_PTR_HEADER::GetFunctionCallbackPtr(*pFunctionPtrMemory);
                if (pFunctionCallbackPtr) {
                    return pFunctionCallbackPtr->GetParent()->QueryType<Function>(TYPE_FUNCTION);
                }
                else if (FunctionPtr *pFunctionPtr = Scope::GetFunctionPtr(*pFunctionPtrMemory)) {
                    return pFunctionPtr;
                }
                else {
                    SYMBOL_DESC symDesc;
                    pVariable->GetSymbolDesc(symDesc);
                    Function *pFunction = memory.GetRootScope()->ResolveFunction(symId);
                    assert(pFunction);
                    if (FunctionPtr *pFunctionPtr = new FunctionPtr(pFunction, symDesc, *pFunctionPtrMemory)) {
                        Scope::RegisterFunctionPtr(*pFunctionPtrMemory, pFunctionPtr);
                        return pFunctionPtr;
                    }
                    else {
                        param.m_pStackMemory->GetRunTimeError().SetError("Out of memory.", m_symDesc, 0, param.m_pStackMemory);
                        return 0;
                    }
                }
            }
            else {
                Node *pParent = pVariable->GetParent();
                if (pParent && (pParent = pParent->GetParent())) {
                    if (LibNode *pLibNode = pParent->QueryType<LibNode>(TYPE_LIB_VAR)) {
                        LibType *pLibType = pLibNode->GetTypeNode()->QueryType<LibType>(TYPE_LIB);
                        assert(pLibType);
                        FunctionRefType *pFunctionRefType = pLibType->GetFunctionRefType(symId);
                        assert(pFunctionRefType);
                        Variable *pFooVar = pFunctionRefType->FindVariableDecNode(symId);
                        assert(pFooVar);
                        FunctionLibPtr *pFunctionLibPtr = pFooVar->GetRight()->QueryType<FunctionLibPtr>(TYPE_FUNCTION_LIB_PTR);
                        assert(pFunctionLibPtr);
                        if (pFunctionLibPtr->GetFooPtr(param)) {
                            return pFunctionLibPtr;
                        }
                        return 0;// an error is already set by GetFooPtr()
                    }
                }
                if (Function *pFunction = memory.GetRootScope()->ResolveFunction(symId)) {
                    return pFunction;
                }
            }
        }
    }
    else { //read stack at idx  == 0 (it's in function's return value)
        ui32 varIdx = 0;
        Memory &memory = *param.m_pMemory;
        MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(varIdx);
        ui32 offset = pMemoryBlockRef->GetOffset();
        void **pFunctionPtrMemory = pMemoryBlockRef->GetMemoryBlock()->GetPtr<void*>(offset);
        if (*pFunctionPtrMemory) {
            FunctionCallbackPtr *pFunctionCallbackPtr = FUNCTION_PTR_HEADER::GetFunctionCallbackPtr(*pFunctionPtrMemory);
            if (pFunctionCallbackPtr) {
                return pFunctionCallbackPtr->GetParent()->QueryType<Function>(TYPE_FUNCTION);
            }
            else if (FunctionPtr *pFunctionPtr = Scope::GetFunctionPtr(*pFunctionPtrMemory)) {
                return pFunctionPtr;
            }
            else {
                SYMBOL_DESC symDesc;
                //pVariable->GetSymbolDesc(symDesc);
                Function *pFunction = memory.GetRootScope()->ResolveFunction(m_symDesc.m_symId);
                assert(pFunction);
                pFunction->GetSymbolDesc(symDesc);
                if (FunctionPtr *pFunctionPtr = new FunctionPtr(pFunction, symDesc, *pFunctionPtrMemory)) {
                    Scope::RegisterFunctionPtr(*pFunctionPtrMemory, pFunctionPtr);
                    return pFunctionPtr;
                }
                else {
                    param.m_pStackMemory->GetRunTimeError().SetError("Out of memory.", m_symDesc, 0, param.m_pStackMemory);
                    return 0;
                }
            }
        }
        else {
            if (Function *pFunction = memory.GetRootScope()->ResolveFunction(m_symDesc.m_symId)) {
                return pFunction;
            }
        }
    }

    param.m_pStackMemory->GetRunTimeError().SetError("No function implementation.", m_symDesc, 0, param.m_pStackMemory);
    return 0;
}


ui32 FunctionCall::GetArgDWORDCount() {
    ui32 dwordCount = 0;
    if (Function *pFunction = GetParent()->QueryType<Function>(TYPE_FUNCTION)) {
        return pFunction->GetArgDWORDCount();
    }
    else {
        assert(0);
    }
    return dwordCount;
}

void FunctionCall::SerializeBody(Serializer *pSerializer) {
    assert(pSerializer);

    if (Variable *pVariable = GetParent()->QueryType<Variable>(TYPE_VAR)) {
        FunctionRefType *pFunctionRefType = pVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
        if (pFunctionRefType->GetParent()->GetType() == TYPE_CAST) {
            pSerializer->Begin(TYPE_LESS);
            Type *pRetType = pFunctionRefType->GetReturnType();
            pRetType->Serialize(pSerializer);
            pSerializer->End(TYPE_GREATER);
        }
    }

    pSerializer->Begin(TYPE_PARENTHESIS);
    for (vector<Node*>::iterator it = m_arguments.begin(); it != m_arguments.end(); ++it) {
        (*it)->Serialize(pSerializer);
        if (it + 1 != m_arguments.end()) {
            pSerializer->End(TYPE_COMMA);
            pSerializer->Begin(TYPE_EMPTY);
        }
    }
    pSerializer->End(TYPE_PARENTHESIS);

    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }
}

/*************************************************************/

FunctionCallbackPtr::FunctionCallbackPtr(Node *pParent, SYMBOL_DESC &symDesc) : 
    Node(pParent, symDesc, TYPE_FUNCTION_CALLBACK_PTR), 
    m_pExecRef(&m_pExec), 
    m_pExec(0) {
}

FunctionCallbackPtr::~FunctionCallbackPtr() {
    /*if (m_pExec) {
        VirtualFree(m_pExec, 0, MEM_RELEASE);
    }*/
}

RUNTIME_RETURN FunctionCallbackPtr::Run(Runable::Param &param) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    ret = GetParent()->Run(param);
    return ret;
}

void** FunctionCallbackPtr::GetFooStubRef(Memory &memory) {
    CreateFooStub(memory);
    return &m_pExec;
}

ui32 FunctionCallbackPtr::GetArgDWORDCount() {
    return static_cast<Function*>(GetParent())->GetArgDWORDCount();
}

void FunctionCallbackPtr::CreateFooStub(Memory &memory) {
    if (m_pExec) {
        return;
    }
    m_pExec = memory.AllocFunctionStub(this);
}

bool FunctionCallbackPtr::UnMarshalArgs(Runable::Param &param, ui32 *pArgs) {
    if (!pArgs) {
        assert(0);
        return false;
    }
    Function *pFunction = GetParent()->QueryType<Function>(TYPE_FUNCTION);
    if (!pFunction) {
        assert(0);
        return false;
    }
    Scope *pFunctionScope = static_cast<Scope*>(pFunction->GetLeft());
    if (!pFunctionScope) {
        assert(0);
        return false;
    }
    Memory &memory = *param.m_pMemory;
    param.m_pStackMemory->PushFrame(this, pFunctionScope, pFunctionScope->GetLocalVarDescriptors());
//#ifdef SCRIPT_DEBUGGER
    param.m_pStackMemory->SetCurRunningNode(this);
//#endif // SCRIPT_DEBUGGER
    ui32 argIdx = 0;
    for (ui32 i = 0; i < pFunction->GetArgCount(); ++i) {
        Type *pResolvedType = 0;
        MEMORY_OFFSET offset = pFunction->ResolveArg(i, pResolvedType);
        assert(INVALID_MEMORY_OFFSET != offset);
        switch (pResolvedType->GetPassBy()) {
            case Type::BY_VALUE:
                UnMarshalByValueArg(param, pArgs, i, *pResolvedType);
                break;
            case Type::BY_REF:
                UnMarshalByRefArg(param, pArgs, i, *pResolvedType);
                break;
            case Type::BY_REF_TO_REF:
                UnMarshalByRefToRefArg(param, pArgs, i, *pResolvedType);
                break;
            default:
                assert(0);
                break;
        }
    }
    return true;
}

bool FunctionCallbackPtr::UnMarshalByValueArg(Runable::Param &param, ui32 *&pArgs, ui32 argIdx, Type &argType) {
    ui32 idx = argIdx;
    MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(idx);
    void *p = pMemoryBlockRef->GetMemoryBlock()->GetPtr(pMemoryBlockRef->GetOffset());
    switch (argType.GetType()) {
        case TYPE_BOOL:
        case TYPE_I8:
        case TYPE_UI8:
            *static_cast<ui8*>(p) = *(ui8*)(pArgs);
            ++pArgs;
            break;
        case TYPE_I16:
        case TYPE_UI16:
            *static_cast<ui16*>(p) = *(ui16*)(pArgs);
            ++pArgs;
            break;
        case TYPE_I32:
        case TYPE_UI32:
            *static_cast<ui32*>(p) = *(ui32*)(pArgs);
            ++pArgs;
            break;
        case TYPE_FLOAT:
            *static_cast<float*>(p) = *(float*)(pArgs);
            ++pArgs;
            break;
        //case  Value::NULL_TYPE:
        case TYPE_I64:
        case TYPE_UI64:
        case TYPE_DOUBLE:
            *static_cast<ui32*>(p) = *(ui32*)pArgs;
            ++pArgs;
            *(static_cast<ui32*>(p) + 1) = *(ui32*)pArgs;
            ++pArgs;
            break;
        case TYPE_STRING:
            assert(0);
            break;
        case TYPE_AGGREGATE_TYPE: { // must handle member function pointers!
            ui32 size = argType.GetTypeSize();
            memcpy(p, pArgs, size);
            pArgs += (size / 4);
            if (size % 4) {
                ++pArgs;
            }
            break;
        }
        case TYPE_FUNCTION_REF_TYPE: {
            *static_cast<ui32*>(p) = *(ui32*)(pArgs);
            ++pArgs;
            break;
        }
        default:
            assert(0);
            return false;
    }
    return true;
}

bool FunctionCallbackPtr::UnMarshalByRefArg(Runable::Param &param, ui32 *&pArgs, ui32 argIdx, Type &argType) {
    ui32 idx = argIdx;
    Memory &memory = *param.m_pMemory;
    StackMemory &stackMemory = *param.m_pStackMemory;
    MemoryBlockRef *pMemoryBlockRef = stackMemory.GetMemoryBlockRef(idx);
    void **p = pMemoryBlockRef->GetMemoryBlock()->GetPtr<void*>(pMemoryBlockRef->GetOffset());
    void **ppArg = *(void***)pArgs;
    if (argType.GetType() == TYPE_FUNCTION_REF_TYPE) {
        *p = *ppArg;
    }
    else {
        if (ppArg) {
            pMemoryBlockRef = memory.QueryMemoryBlockRef(ppArg, &argType);
        }
        else {
            pMemoryBlockRef = Memory::Allocate(&argType, 0, argType.GetTypeSize());
        }
        stackMemory.UpdateMemoryBlockRef(idx, pMemoryBlockRef);
        pMemoryBlockRef->Release(); // QueryMemoryBlockRef() and Allocate() AddREfincrement reference, UpdateMemoryBlockRef() increment reference as well, so release it
    }
    ++pArgs;
    return true;
}

bool FunctionCallbackPtr::UnMarshalByRefToRefArg(Runable::Param &param, ui32 *&pArgs, ui32 argIdx, Type &argType) {
    ui32 idx = argIdx;
    Memory &memory = *param.m_pMemory;
    StackMemory &stackMemory = *param.m_pStackMemory;
    MemoryBlockRef *pMemoryBlockRef = stackMemory.GetMemoryBlockRef(idx);
    void **p = pMemoryBlockRef->GetMemoryBlock()->GetPtr<void*>(pMemoryBlockRef->GetOffset());
    void **ppArg = *(void***)pArgs;
    if (argType.GetType() == TYPE_FUNCTION_REF_TYPE) {
        if (ppArg && *ppArg) {
            *p = **(void***)ppArg;
        }
        else {
            *p = 0;
        }
    }
    else {
        if (ppArg && *ppArg) {
            pMemoryBlockRef = memory.QueryMemoryBlockRef(*ppArg, &argType);
        }
        else {
            pMemoryBlockRef = Memory::Allocate(&argType, 0, argType.GetTypeSize());
        }
        stackMemory.UpdateMemoryBlockRef(idx, pMemoryBlockRef);
        pMemoryBlockRef->Release(); // QueryMemoryBlockRef() and Allocate() AddREfincrement reference, UpdateMemoryBlockRef() increment reference as well, so release it
    }
    ++pArgs;
    return true;
}

ui64 FunctionCallbackPtr::MarshalReturnValue(ui32 *pRetMemory, FunctionPtr::FOO_RETURN_METHOD fooReturnMethod, Value &retValue) {
    Function *pFunction = GetParent()->QueryType<Function>(TYPE_FUNCTION);
    Type *pReturnType = pFunction->GetReturnType();

    switch (fooReturnMethod) {
        case FunctionPtr::REG_EAX: {
            switch (pReturnType->GetPassBy()) {
                case Type::BY_VALUE:
                    return retValue.GetUI32();// MarshalValue(retValue);
                case Type::BY_REF:
                    return (ui32)retValue.GetRaw();//MarshalRef(retValue);
                case Type::BY_REF_TO_REF:
                    return MarshalRefToRef(retValue.GetMemoryBlockRef(pReturnType));
                default:
                    assert(0);
                    break;
            }
            break;
        }
        case FunctionPtr::REG_EAX_EDX: {
            // must take care of struct !
            return retValue.GetUI64();
        }
        case FunctionPtr::POINTER_EAX: {
            // must take care of struct !
            memcpy((void*)*pRetMemory, retValue.GetRaw(), retValue.SizeOf());
            return (ui64)*pRetMemory;
        }
        default:
            return 0;
    }
    return (ui64)pRetMemory;
}

bool FunctionCallbackPtr::MarshalArgs(Runable::Param &param, ui32 *pArgs) {
    if (!pArgs) {
        assert(0);
        return false;
    }
    Function *pFunction = GetParent()->QueryType<Function>(TYPE_FUNCTION);
    if (!pFunction) {
        assert(0);
        return false;
    }
    Scope *pFunctionScope = static_cast<Scope*>(pFunction->GetLeft());
    if (!pFunctionScope) {
        assert(0);
        return false;
    }

    ui32 argIdx = 0;
    Memory &memory = *param.m_pMemory;
    for (ui32 i = 0; i < pFunction->GetArgCount(); ++i) {
        Type *pResolvedType = 0;
        MEMORY_OFFSET offset = pFunction->ResolveArg(i, pResolvedType);
        if (INVALID_MEMORY_OFFSET == offset) {
            assert(0);
            return false;
        }
        ui32 idx = i;
        MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(idx);
        switch (pResolvedType->GetPassBy()) {
            case Type::BY_VALUE:
                ++pArgs;
                break;
            case Type::BY_REF:
                /*if (!MarshalRef(pMemoryBlockRef, *pResolvedType, pArgs)) {
                    assert(0);
                    return false;
                }*/
                ++pArgs;
                break;
            case Type::BY_REF_TO_REF:
                if (!MarshalRefToRef(memory, pMemoryBlockRef, *pResolvedType, pArgs)) {
                    assert(0);
                    return false;
                }
                break;
            default:
                assert(0);
                break;
        }
    }
    return true;
}

ui32 FunctionCallbackPtr::MarshalValue(Value  &value) {
    return value.GetUI32();
}

ui32 FunctionCallbackPtr::MarshalRef(Value  &value) {
    return (ui32)value.GetRaw();
}

ui32 FunctionCallbackPtr::MarshalRefToRef(MemoryBlockRef *pMemoryBlockRef) {
    MemoryBlock *pMemoryBlock = pMemoryBlockRef->GetMemoryBlock();
    void *p = pMemoryBlock->GetRefToMemoryPtr() + pMemoryBlockRef->GetOffset();
    return (ui32)p;
}

bool FunctionCallbackPtr::MarshalRefToRef(Memory &memory, MemoryBlockRef *pMemoryBlockRef, Type &valueType, ui32 *&pOutMemory) {
    void **pInArg = *(void***)pOutMemory;
    if (pInArg) {
        MemoryBlock *pMemoryBlock = pMemoryBlockRef->GetMemoryBlock();
        void **pMemory = pMemoryBlock->GetPtr<void*>(pMemoryBlockRef->GetOffset());

        switch (valueType.GetType()) {
            case TYPE_FUNCTION_REF_TYPE: {
                if (*pInArg) {
                    **(void***)pInArg = *pMemory;
                }
                break;
            }
            //case TYPE_AGGREGATE_TYPE:
                // unmarshal struct's function pointers !!
                //assert(0);
                //break;
            TODO("Unmarshal struct's member pointers!");
            default:
                if (*pInArg && (**(void***)pInArg != *pMemory)) {
                    memory.RegisterMemoryBlock(pMemoryBlock);
                    **(void***)pInArg = *pMemory;
                }
                break;
        }
    }
    else { // null pointer came in, nothing to do
    }
    ++pOutMemory;
    return true;
}

ui64 FunctionCallbackPtr::Callback() {
    // args at ebp + 8
    ui32 *pArgs;
    __asm {
        push eax;
        mov eax, dword ptr[ebp];
        add eax, 8;
        mov dword ptr[pArgs], eax;
        pop eax;
    }

    Value retValue;
    Memory &memory = Memory::GetInstance();
    ThreadId threadId = GetCurrentThreadId();
    Param param(&retValue, &memory, memory.GetThreadMemory(threadId));
    bool isBeginThread = param.PopulateGlobals();
    FunctionPtr::FOO_RETURN_METHOD fooReturnMethod = PrepareReturnValue(retValue);
    if (FunctionPtr::POINTER_EAX == fooReturnMethod) {
        ++pArgs;
    }
    if (UnMarshalArgs(param, pArgs)) {
        //what to do if it failed?
        StackMemory *pStackMemory = param.m_pStackMemory;
#ifdef SCRIPT_DEBUGGER
        FlowSemaphor *pFlowSemaphor = pStackMemory->GetFlowSemaphor();
        DebuggerEvents *pNotify = pFlowSemaphor->GetDebuggerNotify();
        if (isBeginThread) {
            SE_Exception::Init();
            pNotify->OnBeginThread(threadId);
        }
        else {
            ++pFlowSemaphor->m_Script2DllSwitchCounter;
        }
#endif // SCRIPT_DEBUGGER
        RUNTIME_RETURN ret;
        try {
            ret = Run(param);
        }
        catch (SE_Exception &e) {
            ret = RT_EXCEPTION;
            pStackMemory->GetRunTimeError().SetError("SE Exception", m_symDesc, e.getSeNumber(), param.m_pStackMemory);
        }
#ifdef SCRIPT_DEBUGGER
        ++pFlowSemaphor->m_Script2DllSwitchCounter;
#endif // SCRIPT_DEBUGGER
        if (!MarshalArgs(param, pArgs)) {
        
        }

        if (isBeginThread) {
#ifdef SCRIPT_DEBUGGER
            pFlowSemaphor->ResetOnEndThread();
            pNotify->OnEndThread(pStackMemory->GetThreadId());
#endif // SCRIPT_DEBUGGER
            pStackMemory->PopFrame();
            ui32 frameIdx = pStackMemory->PopFrame(); // remove globals
            if (RT_EXCEPTION == ret) {
                while (frameIdx != INVALID_STACK_FRAME_IDX) {
                    frameIdx = pStackMemory->PopFrame();
                };
            }
            assert(INVALID_STACK_FRAME_IDX == frameIdx);
            param.m_pMemory->ReleaseThreadMemory(threadId);
        }
        else {
            param.m_pStackMemory->PopFrame();
        }

        return MarshalReturnValue(--pArgs, fooReturnMethod, retValue);
    }
    return 0;
}

FunctionPtr::FOO_RETURN_METHOD FunctionCallbackPtr::PrepareReturnValue(Value &retValue) {
    Function *pFunction = GetParent()->QueryType<Function>(TYPE_FUNCTION);
    assert(pFunction);
    Type *pRetType = pFunction->GetFunctionRefTypeNode()->GetReturnType();
    ui32 retValueSize = pRetType->GetTypeSize();
    FunctionPtr::FOO_RETURN_METHOD fooReturnMethod = FunctionPtr::REG_EAX;
    retValue.SetType(pRetType->GetValueType(), pRetType);
    if (pRetType->GetPassBy() == Type::BY_VALUE) {
        switch (retValueSize) {
            case 1:
            case 2:
            case 4:
                fooReturnMethod = FunctionPtr::REG_EAX;
                break;
            case 8:
                fooReturnMethod = FunctionPtr::REG_EAX_EDX;
                break;
            default:
                fooReturnMethod = FunctionPtr::POINTER_EAX;
                break;
        }
    }
    return fooReturnMethod;
}

/*************************************************************/

FunctionPtrHeader* FunctionPtrHeader::TypeCast(void *p) { 
    return reinterpret_cast<FunctionPtrHeader*>(reinterpret_cast<ui32>(p) - sizeof(FunctionPtrHeader)); 
}

bool FunctionPtrHeader::IsValid() { 
    try {
        return (reinterpret_cast<ui32>(m_pFunctionPtrStub) == reinterpret_cast<ui32>(this) + sizeof(*this));
    }
    catch (SE_Exception &e) { // make it (...)?
        return false;
    }
}

void FunctionPtrHeader::Init(FunctionCallbackPtr *pFunctionCallbackPtr) {
    m_pFunctionPtrStub = reinterpret_cast<void*>(reinterpret_cast<ui32>(this) + sizeof(FunctionPtrHeader)); 
    m_pFunctionCallbackPtr = pFunctionCallbackPtr;
}

FunctionCallbackPtr* FunctionPtrHeader::GetFunctionCallbackPtr(void* p) {
    FunctionPtrHeader *pFunctionPtrHeader = TypeCast(p);
    return pFunctionPtrHeader->IsValid() ? pFunctionPtrHeader->m_pFunctionCallbackPtr : 0;
}

/*************************************************************/

// ************ EXAMPLE *************
//typedef WINAPI __stdcall
//typedef int (__stdcall *MY_MessageBoxA)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
//FARPROC foo = GetProcAddress(hModule, "MessageBoxA");
//char *title = "title", *text = "test 2";
//_asm {
//    push        2;
//    push        dword ptr[title]
//    push        dword ptr[text]
//    push        0
//    call        dword ptr[foo]
//    //add         esp, 10h <- in case of __cdecl this, Calling function pops the arguments from the stack
//    //cmp         esi, esp
//    //call        __RTC_CheckEsp
//    mov         dword ptr[ret], eax
//};
/*
** WARNING! This is 32 bit memory space **
*/
LIB_FOO FunctionPtr::GetFooPtr(Runable::Param &param) {
    return static_cast<LIB_FOO>(m_pFooPtr);
}

void FunctionPtr::PushParameters(Runable::Param &param, vector<Value> &values) {
    FunctionRefType *pFunctionRefType = GetFunctionRefTypeNode();
    vector<LOCAL_VAR_DESCRIPTOR> argDescs;
    pFunctionRefType->GetArgDescs(argDescs);
    Scope *pFunctionScope = 0;
    if (Node *pLeft = GetLeft()) {
        pFunctionScope = pLeft->QueryType<Scope>(TYPE_SCOPE);
    }
    param.m_pStackMemory->PushFrame(this, pFunctionScope, argDescs);
//#ifdef SCRIPT_DEBUGGER
    param.m_pStackMemory->SetCurRunningNode(this);
//#endif // SCRIPT_DEBUGGER
    ui32 argCount = pFunctionRefType->GetArgCount();
    for (ui32 i = 0; i < argCount; ++i) {
        Type *pArgType = pFunctionRefType->GetArgType(i + 1);
        PushParameter(param, i, values[i], pArgType);
    }
}

void* FunctionPtr::GetRawPtr(Runable::Param &param) {
    return GetFooPtr(param);
}

FunctionPtr::DataReference::~DataReference() {
    if (m_pMemoryBlockRef) {
        m_pMemoryBlockRef->Release();
    }
}

FunctionPtr::DataReference::DataReference(Memory &memory, Value& value, Type *pType) : m_pType(pType), m_pMemoryBlockRef(0), m_pDataRef(&m_pData), m_pData(0) {
    m_pMemoryBlockRef = value.GetMemoryBlockRef(pType);
    m_pMemoryBlockRef->AddRef();
    
    m_pMemoryBlockRef->GetMemoryBlock()->Register(memory);

    void *p = value.GetRaw();
    m_pData = (void**)p;
}

RUNTIME_RETURN FunctionPtr::MarshalArgs(Runable::Param &param, vector<Value> &args, vector<ui32> &data, DATA_REF_VECTOR &dataRef) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    FunctionRefType *pFunctionRefType = GetFunctionRefTypeNode();
    ui32 argCount = pFunctionRefType->GetArgCount();
    Memory &memory = *param.m_pMemory;
    for (ui32 i = 0; i < argCount; ++i) {
        Type *pArgType = pFunctionRefType->GetArgType(i + 1);
        ui32 idx = i;
        args.push_back(Value(param.m_pStackMemory->GetMemoryBlockRef(idx), pArgType));
        switch (pArgType->GetPassBy()) {
            case Type::BY_VALUE:
                ret = MarshalByValueArg(args[i], data, dataRef);
                break;
            case Type::BY_REF:
                ret = MarshalByRefArg(memory, param.m_pStackMemory->GetRunTimeError(), args[i], pArgType, data, dataRef);
                break;
            case Type::BY_REF_TO_REF:
                ret = MarshalByRefToRefArg(memory, param.m_pStackMemory->GetRunTimeError(), args[i], pArgType, data, dataRef);
                break;
            default:
                ret = AssertError();
                break;
        }
        if (ret != RT_NO_ERROR) {
            //assert(0);
            return ret;
        }
    }
    return ret;
}

RUNTIME_RETURN FunctionPtr::MarshalByValueArg(Value &argValue, vector<ui32> &data, DATA_REF_VECTOR &dataRef) {
    switch (argValue.GetType()) {
        case Value::BOOL_TYPE:
        case Value::I8_TYPE:
        case Value::UI8_TYPE:
        case Value::I16_TYPE:
        case Value::UI16_TYPE:
        case Value::I32_TYPE:
        case Value::UI32_TYPE:
        case Value::FLOAT_TYPE: {
            ui32 val = argValue.GetUI32();
            data.push_back(val);
            break;
        }
        case Value::NULL_TYPE:
        case Value::I64_TYPE:
        case Value::UI64_TYPE:
        case Value::DOUBLE_TYPE: {
            ui32 *p = static_cast<ui32*>(argValue.GetRaw());
            data.push_back(*p);
            ++p;
            data.push_back(*p);
            break;
        }
        case Value::STRING_TYPE: {
            char *p = static_cast<char*>(argValue.GetRaw());
            data.push_back((ui32)p);
            break;
        }
        case Value::AGGREGATE_TYPE: {
            ui32 *p = (ui32*)(argValue.GetRaw());
            ui32 size = argValue.SizeOf() / sizeof(ui32);
            for (ui32 k = 0; k < size; ++k) {
                data.push_back(p[k]);
            }
            ui32 delta = argValue.SizeOf() % sizeof(ui32);
            switch (delta) {
                case 0:
                default:
                    break;
                case 1: {
                    ui8 v8[4] = { *(ui8*)&p[size], 0, 0, 0 };
                    data.push_back(*(ui32*)v8);
                    break;
                }
                case 2: {
                    ui8 v8[4] = { *(ui8*)&p[size], ((ui8*)&p[size])[1], 0, 0 };
                    data.push_back(*(ui32*)v8);
                    break;
                }
                case 3: {
                    ui8 v8[4] = { *(ui8*)&p[size], ((ui8*)&p[size])[1], ((ui8*)&p[size])[2], 0 };
                    data.push_back(*(ui32*)v8);
                    break;
                }
            }
            break;
        }
        case Value::FUNCTION_REF_TYPE: {
            ui32 val = argValue.GetUI32();
            data.push_back(val);
            break;
        }
        default:
            return AssertError();
    }
    dataRef.push_back(0);
    return RT_NO_ERROR;
}

RUNTIME_RETURN FunctionPtr::MarshalByRefArg(Memory &memory, RunTimeError &error, Value &argValue, Type *pType, vector<ui32> &data, DATA_REF_VECTOR &dataRef) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    if (DATA_REFERENCE *pDataRef = new DATA_REFERENCE(memory, argValue, pType)) {
        data.push_back((ui32)pDataRef->m_pData);
        dataRef.push_back(pDataRef);
    }
    else {
        TODO("FIX it!");
        ret = RT_ERROR;
        //ret = error.SetError("Out of memory.", m_symDesc, 0);
    }
    return ret;
}
RUNTIME_RETURN FunctionPtr::MarshalByRefToRefArg(Memory &memory, RunTimeError &error, Value &argValue, Type *pType, vector<ui32> &data, DATA_REF_VECTOR &dataRef) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    if (DATA_REFERENCE *pDataRef = new DATA_REFERENCE(memory, argValue, pType)) {
        data.push_back((ui32)pDataRef->m_pDataRef);
        dataRef.push_back(pDataRef);
    }
    else {
        TODO("FIX it!");
        ret = RT_ERROR;
        //ret = error.SetError("Out of memory.", m_symDesc, 0);
    }
    return ret;
}

RUNTIME_RETURN FunctionPtr::UnMarshalArgs(Memory &memory, vector<Value> &args, DATA_REF_VECTOR &dataRef) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    FunctionRefType *pFunctionRefType = GetFunctionRefTypeNode();
    ui32 argCount = pFunctionRefType->GetArgCount();
    for (ui32 i = 0; i < argCount; ++i) {
        Type *pArgType = pFunctionRefType->GetArgType(i + 1);
        switch (pArgType->GetPassBy()) {
            case Type::BY_REF:
                break;
            case Type::BY_REF_TO_REF:
                ret = UnMarshalByRefToRefArg(memory, args[i], pArgType, *dataRef[i]);
                break;
            default:
                break;
        }
        if (ret != RT_NO_ERROR) {
            return ret;
        }
    }
    return ret;
}

RUNTIME_RETURN FunctionPtr::UnMarshalByRefToRefArg(Memory &memory, Value &argValue, Type *pArgType, DATA_REFERENCE &dataRef) {
    void *p = argValue.GetRaw();
    
    if ((*dataRef.m_pDataRef != p) || (*dataRef.m_pDataRef && (*(void**)*dataRef.m_pDataRef != *(void**)p))) {
        return UnMarshalByRef(memory, *dataRef.m_pDataRef, pArgType, argValue) ? RT_NO_ERROR : AssertError();
    }
    
    return RT_NO_ERROR;
}

RUNTIME_RETURN FunctionPtr::CallWrapper(ui32 *data, ui32 dwordCount, LIB_FOO foo, FOO_RETURN_METHOD fooReturnMethod, ui32 &regEAX, ui32 &regEDX) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    ui32 curESP, retESP, retEAX, retEDX;
    //__try {
    //try {
    for (ui32 i = dwordCount; i != 0;) {
        ui32 val = data[--i];
        _asm {
            push    dword ptr[val];
        }
    }
    _asm {
        mov     dword ptr[curESP], esp;
        call    dword ptr[foo];
        push    eax;
        mov     dword ptr[retEAX], esp;
        push    edx;
        mov     dword ptr[retEDX], esp;
    }
    switch (fooReturnMethod) {
        case REG_EAX:
            regEAX = *(ui32*)retEAX;
            break;
        case REG_EAX_EDX:
            regEAX = *(ui32*)retEAX;
            regEDX = *(ui32*)retEDX;
            break;
        case POINTER_EAX:
            regEAX = *(ui32*)retEAX;
            break;
        default:
            assert(0);
            break;
    }
    _asm {
        add     esp, 8;
        mov     dword ptr[retESP], esp;
    }
    if (retESP == curESP) { // workaround for now! adjust esp 
        _asm {
            imul    eax, dword ptr[dwordCount], 4;
            add     esp, eax;
        }
    }
    /*}
    catch (SE_Exception &e) {
        e.getSeNumber();
    }*/
    /*}
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ret = RT_ERROR;
    }*/
    return ret;
}

FunctionRefType* FunctionPtr::GetFunctionRefTypeNode() {
    Node* pParent = GetParent();
    if (Variable *pVariable = pParent->QueryType<Variable>(TYPE_VAR)) {
        assert(pVariable);
        FunctionRefType *pFunctionRefType = pVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
        assert(pFunctionRefType);
        return pFunctionRefType;
    }
    else if (Function *pFunction = pParent->QueryType<Function>(TYPE_FUNCTION)) {
        return pFunction->GetFunctionRefTypeNode();
    }
    //???? is it necessary?
    else if (pParent->GetType() == TYPE_FUNCTION_CALL) {
        return static_cast<FunctionCall*>(pParent)->GetFunctionRefTypeNode();
    }

    assert(0);
    return 0;
}

FunctionPtr::FOO_RETURN_METHOD FunctionPtr::PrepareReturnValue(vector<ui32> &data, Value &retValue) {
    FunctionRefType *pFunctionRefType = GetFunctionRefTypeNode();
    Type *pRetType = pFunctionRefType->GetReturnType();
    ui32 retValueSize = pRetType->GetTypeSize();
    FOO_RETURN_METHOD fooReturnMethod = REG_EAX;
    retValue.SetType(pRetType->GetValueType(), pRetType);
    if (pRetType->GetPassBy() == Type::BY_VALUE) {
        switch (retValueSize) {
            case 1:
            case 2:
            case 4:
                fooReturnMethod = REG_EAX;
                break;
            case 8:
                fooReturnMethod = REG_EAX_EDX;
                break;
            default:
                fooReturnMethod = POINTER_EAX;
                data.push_back((ui32)retValue.GetRaw());
                break;
        }
    }
    return fooReturnMethod;
}

bool FunctionPtr::UnMarshalReturnValue(Memory &memory, FOO_RETURN_METHOD fooReturnMethod, ui32 regEAX, ui32 regEDX, Value &retValue, Value &outValue) {
    Type *pRetType = GetFunctionRefTypeNode()->GetReturnType();
    switch (pRetType->GetPassBy()) {
        case Type::BY_REF: 
            return UnMarshalByRef(memory, (void*)regEAX, pRetType, outValue);
        case Type::BY_REF_TO_REF: 
            return UnMarshalRefByRef(memory, (void*)regEAX, pRetType, outValue);
        case Type::BY_VALUE:
            return UnMarshalByValue(fooReturnMethod, (void*)regEAX, pRetType, retValue, outValue);
            break;
        default:
            assert(0);
            return false;
    }
}

bool FunctionPtr::UnMarshalByValue(FOO_RETURN_METHOD fooReturnMethod, void *p, Type *pType, Value &retValue, Value &outValue) {
    switch (fooReturnMethod) {
        case REG_EAX:
            outValue.SetType(pType->GetValueType(), pType);
            *(ui32*)outValue.GetRaw() = (ui32)p;
            break;
        case REG_EAX_EDX: {
            // must unmarshal pointer to function!
            // check if if it's an aggregate type then unmarsal foo ptr if any!
            outValue.SetType(pType->GetValueType(), pType);
            ui32 *pOut = (ui32*)outValue.GetRaw();
            *pOut = (ui32)p;
            *(pOut + 1) = (ui32)p;
            break;
        }
        case POINTER_EAX: // regEAX cab be ignored, regEAX == retValue->m_pMemoryBlock->m_pMemory
            // must unmarshal all function pointers in the structure!
            outValue = retValue;
            break;
        default:
            assert(0);
            return false;
    }
    return true;
}

bool FunctionPtr::UnMarshalByRef(Memory &memory, void *p, Type *pType, Value &outValue) {
    MemoryBlockRef *pMemoryBlockRef;
    if (pType->GetType() == TYPE_FUNCTION_REF_TYPE) {
        pMemoryBlockRef = outValue.GetMemoryBlockRef(pType);
        *pMemoryBlockRef->GetMemoryBlock()->GetPtr<void*>(pMemoryBlockRef->GetOffset()) = p ? *(void**)p : 0;
    }
    else {
        if (p) {
            pMemoryBlockRef = memory.QueryMemoryBlockRef(p, pType);
        }
        else {
            pMemoryBlockRef = Memory::Allocate(pType, 0, pType->GetTypeSize());
        }
        outValue.Set(pMemoryBlockRef, pType, pMemoryBlockRef->GetOffset());
        pMemoryBlockRef->Release();// QueryMemoryBlockRef() and Allocate() AddREfincrement reference, UpdateMemoryBlockRef() increment reference as well, so release it
    }
    return true;
}

bool FunctionPtr::UnMarshalRefByRef(Memory &memory, void *p, Type *pType, Value &outValue) {
    return UnMarshalByRef(memory, p ? *(void**)p : 0, pType, outValue);
}

FunctionPtr::CleanDataRefVector::~CleanDataRefVector() {
    for (DATA_REF_VECTOR::iterator it = m_dataRef.begin(); it != m_dataRef.end(); ++it) {
        if (*it) {
            delete *it;
        }
    }
}

RUNTIME_RETURN FunctionPtr::Run(Runable::Param &param) {
    vector<Value> args;
    RUNTIME_RETURN ret = RT_NO_ERROR;
    vector<ui32> data;
    vector<DATA_REFERENCE*> dataRef;
    CleanDataRefVector _(dataRef);
    Value retValue;
    // check return type size if it's more than 4 bytes then push memory pointer 
    // for the return value first and then all the arguments!
    Memory &memory = *param.m_pMemory;
    FOO_RETURN_METHOD fooReturnMethod = PrepareReturnValue(data, retValue);

_PROTECT_BEGIN
    ret = MarshalArgs(param, args, data, dataRef);
_PROTECT_END(RT_ERROR)

    if (ret != RT_NO_ERROR) {
        return ret;
    }
    LIB_FOO foo = 0;
    switch (GetType()) {
        case TYPE_FUNCTION_LIB_PTR:
            //foo = static_cast<LIB_FOO>(Scope::GetRawFunctionPtr(this));
            //break;
        case TYPE_FUNCTION_PTR:
            foo = GetFooPtr(param);
            assert(foo);
            break;
        case TYPE_FUNCTION_CALLBACK_PTR:
            foo = static_cast<LIB_FOO>(m_pFooPtr);
            break;
        default:
            break;
    }
    if (foo) {
        ui32 dwordCount = data.size();
        ui32 regEAX = 0, regEDX = 0;
#ifdef SCRIPT_DEBUGGER
        FlowSemaphor *pFlowSemaphor = param.m_pStackMemory->GetFlowSemaphor();
        ++pFlowSemaphor->m_Script2DllSwitchCounter;
#endif // SCRIPT_DEBUGGER

        try {
            ret = CallWrapper(data.data(), dwordCount, foo, fooReturnMethod, regEAX, regEDX);
        } 
        catch (SE_Exception &e) {
            ret = RT_EXCEPTION;
            param.m_pStackMemory->GetRunTimeError().SetError("SE Exception", m_symDesc, e.getSeNumber(), param.m_pStackMemory);
        }
#ifdef SCRIPT_DEBUGGER
        ++pFlowSemaphor->m_Script2DllSwitchCounter;
#endif // SCRIPT_DEBUGGER
        if (ret == RT_NO_ERROR) {
_PROTECT_BEGIN
            if (UnMarshalReturnValue(memory, fooReturnMethod, regEAX, regEDX, retValue, *param.m_pValue)) {
                ret = UnMarshalArgs(memory, args, dataRef);
            }
            else {
                ret = AssertError();
            }
_PROTECT_END(RT_ERROR)
        }
    }
    else {
        ret = param.m_pStackMemory->GetRunTimeError().SetError("NULL function pointer.", m_symDesc, 0, param.m_pStackMemory);
    }
    return ret;
}

/*************************************************************/

FunctionLibPtr::~FunctionLibPtr() {
    /*if (m_pFooPtr) {
        Scope::UnRegisterFunctionPtr(m_pFooPtr);
    }*/
}

RUNTIME_RETURN FunctionLibPtr::Run(Runable::Param &param) {
    return FunctionPtr::Run(param);
}

LIB_FOO FunctionLibPtr::GetFooPtr(Runable::Param &param) {
    Memory &memory = *param.m_pMemory;
    LibType *pLibType = GetParent<LibType>(TYPE_LIB);
    LibNode *pLibNode = 0;
    Variable *pVariable = GetParent()->QueryType<Variable>(TYPE_VAR);
    if (pVariable && pLibType && (pLibNode = pLibType->GetLibNode())) {
        RUNTIME_RETURN ret = RT_NO_ERROR;
        MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(pLibNode->GetVarIdx());
        MemoryBlock *pMemoryBlock = pMemoryBlockRef->GetMemoryBlock();
        ui32 offset = pMemoryBlockRef->GetOffset() + pVariable->GetAlignedMemberOffset();
        void **ppFunctionPtrMemory = pMemoryBlock->GetPtr<void*>(offset);
_PROTECT_BEGIN
        if (!*ppFunctionPtrMemory) {
            string fooName;
            LIB_FOO fooPtr = 0;
            if (m_pSymbolStore->GetSymbolName(m_symDesc.m_symId, fooName)) {
                if (void *h = pLibType->GetModuleHandle()) {
                    if (fooPtr = GetProcAddress((HMODULE)h, fooName.c_str())) {
                        *ppFunctionPtrMemory = fooPtr;
                        Scope::RegisterFunctionPtr(fooPtr, this);
                    }
                    else {
                        SYMBOL_DESC symDesc;
                        GetParent()->GetParent()->GetSymbolDesc(symDesc);
                        param.m_pStackMemory->GetRunTimeError().SetError("Can't get function address.", symDesc, GetLastError(), param.m_pStackMemory);
                    }
                }
                else {
                    SYMBOL_DESC symDesc;
                    pLibType->GetSymbolDesc(symDesc);
                    param.m_pStackMemory->GetRunTimeError().SetError("Module is not loaded.", m_symDesc, 0, param.m_pStackMemory);
                }
            }
            else {
                SYMBOL_DESC symDesc;
                GetParent()->GetSymbolDesc(symDesc);
                param.m_pStackMemory->GetRunTimeError().SetError("Can't get symbol name.", symDesc, 0, param.m_pStackMemory);
            }
        }
        return static_cast<LIB_FOO>(*ppFunctionPtrMemory);
_PROTECT_END(RT_ERROR)
    }
    //assert(0);
    return 0;
}

Type* FunctionLibPtr::GetReturnType() {
    Variable *pVariable = GetParent()->QueryType<Variable>(TYPE_VAR);
    assert(pVariable);
    FunctionRefType *pFunctionRefType = static_cast<FunctionRefType*>(pVariable->GetTypeNode());
    return pFunctionRefType->GetReturnType();
}

/*************************************************************/

ConditionalNode::~ConditionalNode() {
    if (m_pCondition) {
        delete m_pCondition;
    }
};

void ConditionalNode::SetCondition(Node *pExp) {
    m_pCondition = pExp;
}

void ConditionalNode::SerializeBegin(Serializer *pSerializer) {
    pSerializer->WriteSymbol(m_symDesc);
    if (m_pCondition) {
        m_pCondition->Serialize(pSerializer);
    }
}

void ConditionalNode::SerializeBody(Serializer *pSerializer) {
    if (m_pLeft) {
        m_pLeft->Serialize(pSerializer);
        if (m_pLeft->GetType() != TYPE_SCOPE) {
            pSerializer->End(TYPE_SCOPE_END_EXPRESSION);
        }
    }
}

/*************************************************************/

RUNTIME_RETURN IfNode::Run(Runable::Param &param) {
    assert(m_pCondition);

    Value condValue;
    Param condParam(&condValue, param);
    RUNTIME_RETURN ret = m_pCondition->Run(condParam);
    if (ret == RT_NO_ERROR) {
        if (condParam.m_pValue->GetType() == Value::NULL_TYPE) {
            param.m_pStackMemory->GetRunTimeError().SetError("Condition value is NULL!", m_symDesc, 0, param.m_pStackMemory);
            return RT_ERROR;
        }
        if (condValue.GetBool()) {
#ifdef SCRIPT_DEBUGGER
            if (m_pLeft) {
                ret = CheckFlow(m_pLeft, param);
                if (RT_STOP == ret) {
                    return ret;
                }
            }
#endif // SCRIPT_DEBUGGER
            return m_pLeft ? m_pLeft->Run(param) : RT_NO_ERROR;
        }
        else {
#ifdef SCRIPT_DEBUGGER
            if (m_pRight) {
                ret = CheckFlow(m_pRight, param);
                if (RT_STOP == ret) {
                    return ret;
                }
            }
#endif // SCRIPT_DEBUGGER
            return m_pRight ? m_pRight->Run(param) : RT_NO_ERROR;
        }
    }
    return ret;
}

void IfNode::PushExpression(Node *pNode) {
    if (m_IfClause) {
#ifdef SCRIPT_DEBUGGER
        if (pNode->GetType() != TYPE_SCOPE) {
            Scope *pCurrentScope = GetParent<Scope>(TYPE_SCOPE);
            pCurrentScope->GetDebuggerHandler()->RegisterBreakPointLine(pNode);
            pCurrentScope->RegisterBreakPointLine(pNode);
        }
#endif // SCRIPT_DEBUGGER
        SetLeft(pNode);
    }
    else {
#ifdef SCRIPT_DEBUGGER
        if (pNode->GetType() != TYPE_SCOPE) {
            Scope *pCurrentScope = GetParent<Scope>(TYPE_SCOPE);
            pCurrentScope->GetDebuggerHandler()->RegisterBreakPointLine(pNode);
            pCurrentScope->RegisterBreakPointLine(pNode);
        }
#endif // SCRIPT_DEBUGGER
        SetRight(pNode);
    }
}

void IfNode::UpdateExpression(Node *pNode) {
    if (!GetLeft() && !GetRight()) {
        m_pCondition->SetRight(pNode);
    }
    else if (m_IfClause) {
        SetLeft(pNode);
    }
    else {
        SetRight(pNode);
    }
}

bool IfNode::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();
    if (m_pCondition == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        m_pCondition = pPrePostExpressions;
        return true;
    }
    else if (GetLeft() == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        SetLeft(pPrePostExpressions);
        return true;
    }
    else if (GetRight() == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        SetRight(pPrePostExpressions);
        return true;
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

void IfNode::SerializeBody(Serializer *pSerializer) {
    if (pSerializer) {
        pSerializer->Begin(TYPE_EMPTY);
        ConditionalNode::SerializeBody(pSerializer);
        if (m_pRight) {
            pSerializer->Begin(TYPE_ELSE);
            pSerializer->Begin(TYPE_EMPTY);
            m_pRight->Serialize(pSerializer);
            if (m_pRight->GetType() != TYPE_SCOPE) {
                pSerializer->End(TYPE_SCOPE_END_EXPRESSION);
            }
            pSerializer->End(TYPE_ELSE);
        }
    }

}

/*************************************************************/

ForNode::~ForNode() {
    if (m_pInitExp) {
        delete m_pInitExp;
    }
}

RUNTIME_RETURN ForNode::Run(Runable::Param &param) {
    /*        INIT_EXP    COND_EXP   LOOP_EXP  BODY_EXP
        for (m_pInitExp;m_pCondition;m_pRight) m_pLeft;
    */
    RUNTIME_RETURN ret = RT_NO_ERROR;
    Value tempValue;
    Param tempParam(&tempValue, param);
    if (m_pInitExp) {
        ret = m_pInitExp->Run(tempParam);
    }
    if (RT_NO_ERROR == ret) {
        while (true) {
            if (m_pCondition) {
                tempValue.SetType(Value::NULL_TYPE, 0);
                ret = m_pCondition->Run(tempParam);
                if (ret != RT_NO_ERROR) {
                    return ret;
                }
                if (tempParam.m_pValue->GetType() == Value::NULL_TYPE) {
                    param.m_pStackMemory->GetRunTimeError().SetError("Condition value is NULL!", m_symDesc, 0, param.m_pStackMemory);
                    return RT_ERROR;
                }

                if (!tempValue.GetBool()) {
                    break;
                }
            }

            if (m_pLeft) {
#ifdef SCRIPT_DEBUGGER
                ret = CheckFlow(m_pLeft, param);
                if (RT_STOP == ret) {
                    return ret;
                }
#endif // SCRIPT_DEBUGGER
                ret = m_pLeft->Run(param);
                switch (ret) {
                    case RT_BREAK:
                        ret = RT_NO_ERROR;
                    case RT_ERROR:
                    case RT_RETURN:
                    case RT_STOP:
                        return ret;
                    case RT_CONTINUE:
                        ret = RT_NO_ERROR;
                    case RT_NO_ERROR:
                        break;
                    default:
                        return AssertError();
                }
            }

            if (m_pRight) {
#ifdef SCRIPT_DEBUGGER
                ret = CheckFlow(m_pRight, param);
                if (RT_STOP == ret) {
                    return ret;
                }
#endif // SCRIPT_DEBUGGER
                tempValue.SetType(Value::NULL_TYPE, 0);
                ret = m_pRight->Run(tempParam);
                if (ret != RT_NO_ERROR) {
                    return ret;
                }
            }
        }
    }
    return ret;
}

void ForNode::SetInitExp(Node *pNode) {
    if (!pNode) {
        m_expType = COND_EXP;
    }
    else {
        assert(!m_pInitExp);
        m_pInitExp = pNode;
    }
}
void ForNode::SetCondExp(Node* pNode) {
    if (!pNode) {
        m_expType = LOOP_EXP;
    }
    else {
        assert(!m_pCondition);
        m_pCondition = pNode;
    }
}

void ForNode::SetLoopExp(Node *pNode) {
    if (!pNode) {
        m_expType = BODY_EXP;
    }
    else {
        assert(!m_pRight);
        SetRight(pNode);
    }
}

bool ForNode::UpdateExpression(Node *pNode) {
    switch (m_expType) {
        case INIT_EXP:
            m_pInitExp = pNode;
            break;
        case COND_EXP:
            m_pCondition = pNode;
            break;
        case LOOP_EXP:
            SetRight(pNode);
            break;
        case BODY_EXP:
#ifdef SCRIPT_DEBUGGER
            if (pNode->GetType() != TYPE_SCOPE) {
                Scope *pCurrentScope = GetParent<Scope>(TYPE_SCOPE);
                pCurrentScope->GetDebuggerHandler()->RegisterBreakPointLine(pNode);
                pCurrentScope->RegisterBreakPointLine(pNode);
            }
#endif // SCRIPT_DEBUGGER
            SetLeft(pNode);
            break;
        default:
            return false;
    }
    return true;
}

bool ForNode::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();
    if (m_pInitExp == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        m_pInitExp = pPrePostExpressions;
        return true;
    }
    else if (m_pCondition == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        m_pCondition = pPrePostExpressions;
        return true;
    }
    else if (GetRight() == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        SetRight(pPrePostExpressions);
        return true;
    }
    else if (GetLeft() == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        SetLeft(pPrePostExpressions);
        return true;
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

void ForNode::SerializeBegin(Serializer *pSerializer) {
    pSerializer->WriteSymbol(m_symDesc);
}

void ForNode::SerializeBody(Serializer *pSerializer) {
    assert(pSerializer);

    pSerializer->Begin(TYPE_PARENTHESIS);
    if (m_pInitExp) {
        m_pInitExp->Serialize(pSerializer);
    }
    pSerializer->End(TYPE_SEMICOLON);
    pSerializer->Begin(TYPE_EMPTY);
    if (m_pCondition) {
        m_pCondition->Serialize(pSerializer);
    }
    pSerializer->End(TYPE_SEMICOLON);
    pSerializer->Begin(TYPE_EMPTY);
    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }
    pSerializer->End(TYPE_PARENTHESIS);
    pSerializer->Begin(TYPE_EMPTY);
    if (m_pLeft) {
        m_pLeft->Serialize(pSerializer);
        if (m_pLeft->GetType() != TYPE_SCOPE) {
            pSerializer->End(TYPE_SCOPE_END_EXPRESSION);
        }
    }
    else {
        pSerializer->End(TYPE_SCOPE_END_EXPRESSION);
    }
}

/*************************************************************/

RUNTIME_RETURN WhileNode::Run(Runable::Param &param) {
    assert(m_pCondition);
    RUNTIME_RETURN ret = RT_NO_ERROR;
    while (true) {
        Value condValue;
        Param condParam(&condValue, param);
#ifdef SCRIPT_DEBUGGER
        ret = CheckFlow(m_pCondition, param);
        if (RT_STOP == ret) {
            return ret;
        }
#endif // SCRIPT_DEBUGGER
        ret = m_pCondition->Run(condParam);
        if (ret != RT_NO_ERROR) {
            return ret;
        }
        if (condParam.m_pValue->GetType() == Value::NULL_TYPE) {
            param.m_pStackMemory->GetRunTimeError().SetError("Condition value is NULL!", m_symDesc, 0, param.m_pStackMemory);
            return RT_ERROR;
        }
        if (!condValue.GetBool()) {
            break;
        }
        if (m_pLeft) {
#ifdef SCRIPT_DEBUGGER
            ret = CheckFlow(m_pLeft, param);
            if (RT_STOP == ret) {
                return ret;
            }
#endif // SCRIPT_DEBUGGER
            ret = m_pLeft->Run(param);
            switch (ret) {
                case RT_BREAK:
                    ret = RT_NO_ERROR;
                case RT_ERROR:
                case RT_RETURN:
                case RT_STOP:
                    return ret;
                case RT_CONTINUE:
                    ret = RT_NO_ERROR;
                case RT_NO_ERROR:
                    break;
                default:
                    return AssertError();
            }
        }
    }
    return ret;
}

void WhileNode::UpdateExpression(Node *pNode) {
    if (!GetLeft()) {
        m_pCondition->SetRight(pNode);
    }
    else {
#ifdef SCRIPT_DEBUGGER
        if (pNode->GetType() != TYPE_SCOPE) {
            Scope *pCurrentScope = GetParent<Scope>(TYPE_SCOPE);
            pCurrentScope->GetDebuggerHandler()->RegisterBreakPointLine(pNode);
            pCurrentScope->RegisterBreakPointLine(pNode);
        }
#endif // SCRIPT_DEBUGGER
        SetLeft(pNode);
    }
}

void WhileNode::PushExpression(Node *pNode) {
#ifdef SCRIPT_DEBUGGER
    if (pNode->GetType() != TYPE_SCOPE) {
        Scope *pCurrentScope = GetParent<Scope>(TYPE_SCOPE);
        pCurrentScope->GetDebuggerHandler()->RegisterBreakPointLine(pNode);
        pCurrentScope->RegisterBreakPointLine(pNode);
    }
#endif // SCRIPT_DEBUGGER
    SetLeft(pNode);
}

bool WhileNode::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();

    if (m_pCondition == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        m_pCondition = pPrePostExpressions;
        return true;
    }
    else if (GetLeft() == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        SetLeft(pPrePostExpressions);
        return true;
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

void WhileNode::SerializeBody(Serializer *pSerializer) {
    ConditionalNode::SerializeBody(pSerializer);
    if (m_pRight) {
        pSerializer->Begin(TYPE_EMPTY);
        m_pRight->Serialize(pSerializer);
    }
}
/*************************************************************/
SwitchNode::~SwitchNode() {
    vector<Node*>::iterator it;
    m_pLeft = 0; // it's referenced in m_casesFlow array, points to "default:" case
    Delete(m_casesFlow);
}

RUNTIME_RETURN SwitchNode::Run(Runable::Param &param) {
    Value caseValue;
    Param caseParam(&caseValue, param);
    RUNTIME_RETURN ret = m_pCondition->Run(caseParam);
    if (ret == RT_NO_ERROR) {
        if (caseParam.m_pValue->GetType() == Value::NULL_TYPE) {
            param.m_pStackMemory->GetRunTimeError().SetError("Switch statement value is NULL!", m_symDesc, 0, param.m_pStackMemory);
            return RT_ERROR;
        }

        CASES_MAP::iterator it = m_cases.find(caseValue.GetUI32());
        ui32 i;
        if (it != m_cases.end()) {
            i = it->second - 1;
        }
        else if (m_pLeft) { // process default case
            i = m_defaultIdx;
        }
        else { // no default case, just skip it
            return RT_NO_ERROR;
        }
        for (; i < m_casesFlow.size(); ++i) {
            ret = m_casesFlow[i]->Run(param);
            switch (ret) {
                case RT_BREAK:
                    ret = RT_NO_ERROR;
                case RT_ERROR:
                case RT_RETURN:
                case RT_STOP:
                case RT_CONTINUE:
                    return ret;
                case RT_NO_ERROR:
                    break;
                default:
                    return AssertError();
            }
        }
    }

    return ret;
}

bool SwitchNode::SetCase(ui32 value) {
    pair<CASES_MAP::iterator, bool> ret = m_cases.insert(pair<ui32, ui32>(value, m_casesFlow.size()));
    return ret.second;
}

bool SwitchNode::UpdateCaseExp(Node *pExpNode) {
    m_casesFlow.push_back(pExpNode);
    return true;
}

bool SwitchNode::SetDefault(Node *pExpNode) {
    if (!GetLeft()) {
        SetLeft(pExpNode);
        m_defaultIdx = m_casesFlow.size();
        return UpdateCaseExp(pExpNode);
    }
    return false;
}

bool SwitchNode::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();
    if (m_pCondition == pExpNode) {
        pPrePostExpressions->SetParent(this);
        pExpNode->SetParent(pPrePostExpressions);
        m_pCondition = pPrePostExpressions;
        return true;
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

void SwitchNode::SerializeBody(Serializer *pSerializer) {
    vector<Node*>::iterator it;
    for (it = m_casesFlow.begin(); it != m_casesFlow.end(); ++it) {
        (*it)->Serialize(pSerializer);
    }
}

void SwitchNode::SerializeBegin(Serializer *pSerializer) {
    ConditionalNode::SerializeBegin(pSerializer);
    pSerializer->Begin(TYPE_EMPTY);
    pSerializer->Begin(TYPE_SCOPE);
}

void SwitchNode::SerializeEnd(Serializer *pSerializer) {
    pSerializer->End(TYPE_SCOPE);
}

/*************************************************************/

bool AggregateSymbol::GetString(string &value) {
    for (vector<SYMBOL_ID>::iterator it = m_symbolIds.begin(); it != m_symbolIds.end(); ++it) {
        string str;
        if (!m_pSymbolStore->GetSymbolName(*it, str)) {
            return false;
        }
        value += str;
    }
    return true;
}

void AggregateSymbol::SerializeBody(Serializer *pSerializer) {
    for (vector<SYMBOL_ID>::iterator it = AggregateSymbol::m_symbolIds.begin(); it != AggregateSymbol::m_symbolIds.end(); ++it) {
        // for now ignore line and position of the symbol in the source code
        pSerializer->WriteSymbol(SYMBOL_DESC(*it));
    }
}
/*************************************************************/
RUNTIME_RETURN CaseNode::Run(Runable::Param &param) {
    if (m_pLeft) {
        return m_pLeft->Run(param);
    }
    return RT_NO_ERROR;
}

void CaseNode::SerializeBody(Serializer *pSerializer) {
    pSerializer->Begin(TYPE_EMPTY);
    AggregateSymbol::SerializeBody(pSerializer);
    
    pSerializer->Begin(TYPE_COLON);
    if (m_pLeft) {
        pSerializer->Begin(TYPE_EMPTY);
        pSerializer->Begin(TYPE_SCOPE);
        m_pLeft->Serialize(pSerializer);
        pSerializer->End(TYPE_SCOPE);
    }
    else {
        pSerializer->End(TYPE_NEW_LINE);
    }
}

/*************************************************************/

RUNTIME_RETURN DefaultNode::Run(Runable::Param &param) {
    if (m_pLeft) {
        return m_pLeft->Run(param);
    }
    return RT_NO_ERROR;
}

void DefaultNode::SerializeBody(Serializer *pSerializer) {
    pSerializer->Begin(TYPE_COLON);
    if (m_pLeft) {
        pSerializer->Begin(TYPE_EMPTY);
        pSerializer->Begin(TYPE_SCOPE);
        m_pLeft->Serialize(pSerializer);
        pSerializer->End(TYPE_SCOPE);
    }
    else {
        pSerializer->End(TYPE_NEW_LINE);
    }
}

/*************************************************************/

RUNTIME_RETURN Parenthesis::Run(Runable::Param &param) {
    assert(m_pRight);
    return m_pRight->Run(param);
}

bool Parenthesis::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    if (SYMBOL_ID_PARENTHESIS != m_symDesc.m_symId) {
        _ERROR(PARENTHESIS_IS_NOT_CLOSED);
    }
    else if (!m_pRight) {
        _ERROR(EMPTY_PARENTHESIS);
    }
    else {
        return m_pRight->IsExpression(pReturnType, pPrePostExp, error);
    }
    return false;
}


void Parenthesis::SerializeBody(Serializer *pSerializer) {
    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }
}

/*************************************************************/
SubscriptNode::~SubscriptNode() {
    Delete<Node>(m_subscriptExpressions);
}

RUNTIME_RETURN SubscriptNode::GetArraySubscriptOffset(Runable::Param &param, ui32 &offset) {
    RUNTIME_RETURN ret = RT_NO_ERROR;
    ArrayType *pArrayType = GetArrayType();

    assert(pArrayType);
    
    //_PROTECT_BEGIN

    ui32 jump = 1;
    ui32 valueSize = pArrayType->GetTypeNode()->GetValueSize();
    
    offset = 0;
    
    for (ui32 i = 0; i < m_subscriptExpressions.size(); ++i) {
        Value subscriptValue;
        Param subscriptParam(&subscriptValue, param);
        Node *pExp = m_subscriptExpressions[i];
        if (pExp->GetType() == TYPE_PRE_POST_EXPRESSIONS) {
            PrePostExpressions *pPrePostExpressions = static_cast<PrePostExpressions*>(pExp);
            ret = pPrePostExpressions->RunPre(subscriptParam);
            if (ret != RT_NO_ERROR) {
                return ret;
            }
            ret = pPrePostExpressions->GetExpressionNode()->Run(subscriptParam);
        }
        else {
            ret = pExp->Run(subscriptParam);
        }
        if (ret != RT_NO_ERROR) {
            return ret;
        }
        if (subscriptParam.m_pValue->GetType() == Value::NULL_TYPE) {
            param.m_pStackMemory->GetRunTimeError().SetError("Subscript value is NULL!", m_symDesc, 0, param.m_pStackMemory);
            return RT_ERROR;
        }
        ui32 idx = subscriptValue.GetI32(),
             dimensionSize = pArrayType->GetDimensionSubscript(i);

        if (pExp->GetType() == TYPE_PRE_POST_EXPRESSIONS) {
            PrePostExpressions *pPrePostExpressions = static_cast<PrePostExpressions*>(pExp);
            ret = pPrePostExpressions->RunPost(subscriptParam);
            if (ret != RT_NO_ERROR) {
                return ret;
            }
        }
        offset += idx * jump;
        jump *= dimensionSize;
    }

    offset *= valueSize;
    
    //if (!pArrayType->GetDynExpression()) {
    //    if (pArrayType->GetTypeSize() <= offset) {
    //        param.m_pStackMemory->GetRunTimeError().SetError("Array subscript is out of bounds", m_symDesc, 0, param.m_pStackMemory);
    //        return RT_ERROR;
    //    }
    //}
    //_PROTECT_END
    return ret;
}

RUNTIME_RETURN SubscriptNode::Run(Runable::Param &param) {
    ui32 arrayOffset;
    RUNTIME_RETURN ret = GetArraySubscriptOffset(param, arrayOffset);
    if (ret != RT_NO_ERROR) {
        return ret;
    }
    ArrayType *pArrayType = 0;
    ui32 varIdx = 0;
    if (Variable *pArrayVariable = GetParent()->QueryType<Variable>(TYPE_VAR)) {
        pArrayType = pArrayVariable->GetTypeNode()->QueryType<ArrayType>(TYPE_ARRAY);
        varIdx = pArrayVariable->GetVarIdx();
    }
    else { // else it's function call return
        pArrayType = GetArrayType();
    }

    assert(pArrayType);

    StackMemory     &stackMemory         = *param.m_pStackMemory;
    MemoryBlockRef *pArrayMemoryBlockRef = stackMemory.GetMemoryBlockRef(varIdx);
    MemoryBlock    *pArrayMemoryBlock    = pArrayMemoryBlockRef->GetMemoryBlock();
    ui32            offset               = pArrayMemoryBlockRef->GetOffset() + arrayOffset;

    Type *pItemType = pArrayType->GetTypeNode();

    if (pArrayMemoryBlockRef->GetMemoryBlock()->GetSize() <= offset) {
        param.m_pStackMemory->GetRunTimeError().SetError("Array subscript is out of memory bounds", m_symDesc, 0, param.m_pStackMemory);
        return RT_ERROR;
    }

    MemoryBlock *pItemMemoryBlock = 0;

    if (pItemType->GetPassBy() != Type::BY_VALUE) {
        // dereference memory 
        // this call adds reference
_PROTECT_BEGIN
        param.SetReferenceOfValue(pArrayMemoryBlock, offset);
        pItemMemoryBlock = param.m_pMemory->GetReferencedMemoryBlock(pItemType, pArrayMemoryBlock, offset/*new offset returned*/);//, pItemType->GetTypeSize());
_PROTECT_END(RT_ERROR)
#ifdef MEMORY_ACCESS_EXCEPTION
        if (ret == RT_ERROR) {
            return ret;
        }
#endif // MEMORY_ACCESS_EXCEPTION
    }
    else {
        param.ReleaseReferenceOfValue(false);
        pItemMemoryBlock = pArrayMemoryBlock;
        pItemMemoryBlock->AddRef();
    }

    if (m_pRight) {
        if (m_pRight->GetType() == TYPE_FUNCTION_CALL) {
            Value _value;
            //Scope *pFunctionScope = static_cast<Scope*>(pFunction->GetLeft());
            MemoryBlockRef *pItemMemoryBlockRef = Memory::Allocate(pItemType, offset, pItemMemoryBlock);
            stackMemory.PushResultMemoryBlockRef(0, 0, pItemMemoryBlockRef);
            //stackMemory.PushReferencedMemoryBlock(varIdx, pItemType, offset, pItemMemoryBlock);
            Runable::Param _param(&_value, param);
            ret = m_pRight->Run(_param);
            //stackMemory.PopReferencedMemoryBlock(varIdx);
            stackMemory.PopResultMemoryBlockRef();
_PROTECT_BEGIN
            param.m_pValue->Set(*_param.m_pValue);
_PROTECT_END(RT_ERROR)
            pItemMemoryBlockRef->Release();
            if (ret == RT_EXCEPTION) {
                ret = RT_ERROR;
            }
        }
        else
        {
            ui32 varIdx = GetVarIdx();
            // push memory block of dereferenced memory
            stackMemory.PushReferencedMemoryBlock(varIdx, pItemType, offset, pItemMemoryBlock);

            ret = m_pRight->Run(param);
            stackMemory.PopReferencedMemoryBlock(varIdx);
        }
    }
    else {
        MemoryBlockRef *pDataMemberMemoryBlockRef = Memory::Allocate(pItemType, offset, pItemMemoryBlock);

        param.m_pValue->Set(pDataMemberMemoryBlockRef, pItemType, offset);
        pDataMemberMemoryBlockRef->Release();
    }
    //pDataMemoryBlock->Release();
    pItemMemoryBlock->Release();

    return ret;
}

bool SubscriptNode::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(pReturnType == 0);

    if (!m_subscriptExpressions.size()) {
        _ERROR(EXPECTING_SUBSCRIPT_VALUE);
        return false;
    }
    vector<Node*>::iterator it = m_subscriptExpressions.begin();
    for (; it != m_subscriptExpressions.end(); ++it) {
        Node *pExp = (*it);
        Type *_pReturnType = 0;
        PrePostExpressions *_pPrePostExp = 0;

        if (!pExp->IsExpression(_pReturnType, _pPrePostExp, error)) {
            if (_pPrePostExp) {
                delete _pPrePostExp;
            }
            return false;
        }
        if (_pPrePostExp) {
            _pPrePostExp->SetExpressionNode(pExp);
            if (!pExp->GetParent()->InjectPrePostExpressions(_pPrePostExp)) {
                SYMBOL_DESC symDesc;
                pExp->GetSymbolDesc(symDesc);
                _ERROR_EX(INTERNAL_ERROR, symDesc);
                return false;
            }
        }
    }

    if (m_pRight) {
        return m_pRight->IsExpression(pReturnType, pPrePostExp, error);
    }
    else if (m_pLeft) { // prepost expression
        return m_pLeft->IsExpression(pReturnType, pPrePostExp, error);
    }
    else {
        ArrayType *pArrayType = GetArrayType();
        pReturnType = pArrayType->GetTypeNode();
        return true;
    }
    return false;
}

bool SubscriptNode::InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions) {
    Node *pExpNode = pPrePostExpressions->GetExpressionNode();
    for (ui32 i = 0; i < m_subscriptExpressions.size(); ++i) {
        if (m_subscriptExpressions[i] == pExpNode) {
            pPrePostExpressions->SetParent(this);
            pExpNode->SetParent(pPrePostExpressions);
            m_subscriptExpressions[i] = pPrePostExpressions;
            return true;
        }
    }
    assert(0);
    delete pPrePostExpressions;
    return false;
}

ArrayType *SubscriptNode::GetArrayType() {
    ArrayType *pArrayType = 0;
    switch (GetParent()->GetType()) {
        case TYPE_VAR:
            pArrayType = static_cast<ArrayType*>(static_cast<Variable*>(GetParent())->GetTypeNode());
            break;
        case TYPE_FUNCTION_CALL:
            pArrayType = static_cast<FunctionCall*>(GetParent())->GetFunctionRefTypeNode()->GetReturnType()->QueryType<ArrayType>(TYPE_ARRAY);
            assert(pArrayType);
            break;
        default:
            assert(0);
            break;
    }
    
    return pArrayType;
}

bool SubscriptNode::UpdateExpression(Node* pNode) {
    if (m_subscriptExpressions.size()) {
        m_subscriptExpressions.pop_back();
        m_subscriptExpressions.push_back(pNode);
        return true;
    }
    return false;
}

void SubscriptNode::SetPostOperator(Node *pPostOperator) {
    Node *pLast = this;
    while (pLast) {
        if (pLast->GetLeft()) {
            pLast = pLast->GetLeft();
        }
        else {
            break;
        }
    }
    pLast->SetLeft(pPostOperator);
    pPostOperator->SetParent(this); // duplicate?
}

void SubscriptNode::SerializeBody(Serializer *pSerializer) {
    for (ui32 i = 0; i < m_subscriptExpressions.size(); ++i) {
        m_subscriptExpressions[i]->Serialize(pSerializer);
        if (m_subscriptExpressions.size() > i + 1) {
            pSerializer->Begin(TYPE_COMMA);
        }
    }
}
void SubscriptNode::SerializeEnd(Serializer *pSerializer) {
    Runable::SerializeEnd(pSerializer);
    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }
}
/*************************************************************/

LibNode::~LibNode() {
    if (m_hModule != 0) {
        FreeLibrary((HMODULE)m_hModule);
    }
};

RUNTIME_RETURN LibNode::Run(Runable::Param &param) {
    if (m_pRight) {
        return m_pRight->Run(param);
    }
    else {
        LibType *pLibType = GetTypeNode()->QueryType<LibType>(TYPE_LIB);
        void *hModule = pLibType->GetModuleHandle();
        if (hModule == 0) {
            string path = pLibType->GetPath();
            m_hModule = LoadLibraryA(path.c_str());
            return m_hModule ? RT_NO_ERROR : param.m_pStackMemory->GetRunTimeError().SetError("Can't load library: \'" + path + "\'", m_symDesc, GetLastError(), param.m_pStackMemory);
        }
        return RT_NO_ERROR;
    }
}

void* LibNode::GetModuleHandle() {
    if (GetParent()->GetParent()->GetType() == TYPE_SCOPE) {
        return m_hModule;
    }
    else {
        LibType *pLibType = GetTypeNode()->QueryType<LibType>(TYPE_LIB);
        void *hModule = pLibType->GetModuleHandle();
        return hModule;
    }
}

bool LibNode::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    if (!m_pRight) {
        _ERROR(LIB_FUNCTION_CALL_MISSING);
    }
    else {
        return m_pRight->IsExpression(pReturnType, pPrePostExp, error);
    }
    return false;
}

void LibNode::SerializeBegin(Serializer *pSerializer) {
    pSerializer->WriteSymbol(m_symDesc);
}

void LibNode::SerializeBody(Serializer *pSerializer) {
    if (m_pRight) {
        m_pRight->Serialize(pSerializer);
    }
}

/*************************************************************/

RUNTIME_RETURN ErrorNode::Run(Runable::Param &param) {
    Scope *pScope = static_cast<Scope*>(GetLeft());
    assert(pScope);
    ErrorVariable *pErrorVariable = pScope->GetErrorVariable();
    assert(pErrorVariable);
    pErrorVariable->SetError(param, param.m_pStackMemory->GetRunTimeError());
    return GetLeft()->Run(param);
}

void ErrorNode::SerializeBody(Serializer *pSerializer) {
    pSerializer->Begin(TYPE_PARENTHESIS);
    ErrorVariable *pErrorVariable = static_cast<Scope*>(GetLeft())->GetErrorVariable();
    SYMBOL_DESC symDesc;
    pErrorVariable->GetSymbolDesc(symDesc);
    pSerializer->WriteSymbol(symDesc);
    pSerializer->End(TYPE_PARENTHESIS);
    pSerializer->Begin(TYPE_EMPTY);
    GetLeft()->Serialize(pSerializer);
}

/*************************************************************/

RUNTIME_RETURN ErrorVariable::Run(Runable::Param &param) {
    return Variable::Run(param);
}

bool ErrorVariable::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    if (m_pLeft) {
        //_ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
        return m_pLeft->IsExpression(pReturnType, pPrePostExp, error);
    }
    if (m_pRight) { // member access
        return m_pRight->IsExpression(pReturnType, pPrePostExp, error);
    }

    pReturnType = GetTypeNode();
    return true;
}

void ErrorVariable::SetError(Runable::Param &param, RunTimeError &error) {
    Memory &memory = *param.m_pMemory;
    AggregateType *pType = static_cast<AggregateType*>(GetTypeNode());
    MemoryBlockRef *pMemoryBlockRef = param.m_pStackMemory->GetMemoryBlockRef(GetVarIdx());
    SYMBOL_ID symId;

    m_pSymbolStore->GetSymbolId(string("name"), symId);
    Variable *pVariable = pType->FindMemberDecNode(symId);
    Value name;
    name.Set(pMemoryBlockRef, pVariable->GetTypeNode(), pVariable->GetAlignedMemberOffset());
    name.Set(error.GetName(), Value::STRING_TYPE, pVariable->GetTypeNode());

    m_pSymbolStore->GetSymbolId(string("line"), symId);
    pVariable = pType->FindMemberDecNode(symId);
    Value line, line0;
    line.Set(pMemoryBlockRef, pVariable->GetTypeNode(), pVariable->GetAlignedMemberOffset());
    line0.Set(error.GetSymDesc().m_line);
    line = line0;

    m_pSymbolStore->GetSymbolId(string("position"), symId);
    pVariable = pType->FindMemberDecNode(symId);
    Value position, position0;
    position.Set(pMemoryBlockRef, pVariable->GetTypeNode(), pVariable->GetAlignedMemberOffset());
    position0.Set(error.GetSymDesc().m_pos);
    position = position0;

    m_pSymbolStore->GetSymbolId(string("file"), symId);
    pVariable = pType->FindMemberDecNode(symId);
    Value file;
    file.Set(pMemoryBlockRef, pVariable->GetTypeNode(), pVariable->GetAlignedMemberOffset());
    string fileName;
    m_pSymbolStore->GetFileName(error.GetSymDesc().m_fileId, fileName);
    file.Set(fileName, Value::STRING_TYPE, pVariable->GetTypeNode());

    m_pSymbolStore->GetSymbolId(string("trace"), symId);
    pVariable = pType->FindMemberDecNode(symId);
    Value trace;
    trace.Set(pMemoryBlockRef, pVariable->GetTypeNode(), pVariable->GetAlignedMemberOffset());
    trace.Set(error.GetStackString(), Value::STRING_TYPE, pVariable->GetTypeNode());

    m_pSymbolStore->GetSymbolId(string("symbol"), symId);
    pVariable = pType->FindMemberDecNode(symId); 
    Value symbol;
    symbol.Set(pMemoryBlockRef, pVariable->GetTypeNode(), pVariable->GetAlignedMemberOffset());
    string symbolName;
    m_pSymbolStore->GetSymbolName(error.GetSymDesc().m_symId, symbolName);
    symbol.Set(symbolName, Value::STRING_TYPE, pVariable->GetTypeNode());
}

//void ErrorVariable::SerializeBegin(Serializer *pSerializer) {
//    
//}

void ErrorVariable::SerializeBody(Serializer *pSerializer) {
    if (m_pRight) {
        pSerializer->WriteSymbol(m_symDesc);
        m_pRight->Serialize(pSerializer);
    }
}

//void ErrorVariable::SerializeEnd(Serializer *pSerializer) {
//
//}

/*************************************************************/

RUNTIME_RETURN MathOperator::Run(Runable::Param &param) {
    Value leftValue;
    Param leftParam(&leftValue, param);
    RUNTIME_RETURN ret = m_pLeft->Run(leftParam);
    if (ret == RT_NO_ERROR) {
        if (leftParam.m_pValue->GetType() == Value::NULL_TYPE) {
            param.m_pStackMemory->GetRunTimeError().SetError("Left value is NULL!", m_symDesc, 0, param.m_pStackMemory);
            return RT_ERROR;
        }
        Value rightValue;
        Param rightParam(&rightValue, param);
        ret = m_pRight->Run(rightParam);
        if (ret == RT_NO_ERROR) {
            if (rightParam.m_pValue->GetType() == Value::NULL_TYPE) {
                param.m_pStackMemory->GetRunTimeError().SetError("Right value is NULL!", m_symDesc, 0, param.m_pStackMemory);
                return RT_ERROR;
            }
_PROTECT_BEGIN
            switch (m_type) {
                case TYPE_LESS:
                case TYPE_LESS_EQ:
                case TYPE_GREATER:
                case TYPE_GREATER_EQ:
                case TYPE_EQ:
                case TYPE_NOT_EQ: {
                    ERROR_TYPE err = (leftValue.*m_fnOperation)(rightValue);
                    if (EVAL_TRUE == err) {
                        //set true
                        param.m_pValue->Set(true);
                        return RT_NO_ERROR;
                    }
                    else if (EVAL_FALSE == err) {
                        param.m_pValue->Set(false);
                        return RT_NO_ERROR;
                    }
                    break;
                }
                default: {
                    *param.m_pValue = leftValue;
                    ERROR_TYPE err = (param.m_pValue->*m_fnOperation)(rightValue);
                    if (NO_ERROR == err) {
                        return RT_NO_ERROR;
                    }
                    if (DIVISION_BY_ZERO == err) {
                        param.m_pStackMemory->GetRunTimeError().SetError("Division by zero", m_symDesc, 0, param.m_pStackMemory);
                    }
                    else {
                        param.m_pStackMemory->GetRunTimeError().SetError("Runtime error: " + to_string(err), m_symDesc, 0, param.m_pStackMemory);
                    }
                    ret = RT_ERROR;
                    break;
                }
            }
_PROTECT_END(RT_ERROR)
        }
    }
    return ret;
}

bool MathOperator::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(pReturnType == 0);
    Type *pLeftType = 0, *pRightType = 0;
    if (!m_pLeft) {
        _ERROR(OPERATOR_NEEDS_LEFT_VALUE);
    }
    else if (!m_pRight) {
        _ERROR(OPERATOR_NEEDS_RIGHT_VALUE);
    }
    else if (m_pLeft->IsExpression(pLeftType, pPrePostExp, error) && m_pRight->IsExpression(pRightType, pPrePostExp, error)) {
        if (IsApplicable(pLeftType, pRightType)) {
            switch (m_type) {
                case TYPE_LESS:
                case TYPE_LESS_EQ:
                case TYPE_GREATER:
                case TYPE_GREATER_EQ:
                case TYPE_EQ:
                case TYPE_NOT_EQ:
                case TYPE_AND:
                case TYPE_OR:
                    pReturnType = Scope::GetDummyTypeNode(TYPE_BOOL);
                    break;
                default:
                    pReturnType = pLeftType;
                    break;
            }
            return true;
        }
        else {
            _ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
        }
    }
    return false;
}

/*************************************************************/

RUNTIME_RETURN MathAssignOperator::Run(Runable::Param &param) {
    if (Variable *pLeftVariable = m_pLeft->QueryType<Variable>(TYPE_VAR)) {
        Value rightValue;
        Param rightParam(&rightValue, param);
        RUNTIME_RETURN  ret = MathOperator::Run(rightParam);
_PROTECT_BEGIN
        if (ret == RT_NO_ERROR) {
            if (rightParam.m_pValue->GetType() == Value::NULL_TYPE) {
                param.m_pStackMemory->GetRunTimeError().SetError("Right value is NULL!", m_symDesc, 0, param.m_pStackMemory);
                return RT_ERROR;
            }
            ret = pLeftVariable->Run(param);
            if (ret == RT_NO_ERROR) {
                *param.m_pValue = rightValue;
            }
        }
_PROTECT_END(RT_ERROR)
        return ret;
    }
    return AssertError();
}

bool MathAssignOperator::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(pReturnType == 0);
    Type *pLeftType = 0, *pRightType = 0;
    if (!m_pLeft) {
        _ERROR(OPERATOR_NEEDS_LEFT_VALUE);
    }
    else if (!m_pRight) {
        _ERROR(OPERATOR_NEEDS_RIGHT_VALUE);
    }
    else if (m_pLeft->IsExpression(pLeftType, pPrePostExp, error) && m_pRight->IsExpression(pRightType, pPrePostExp, error)) {
        if (IsApplicable(pLeftType, pRightType)) {
            pReturnType = pLeftType;
            return true;
        }
        else {
            _ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
        }
    }
    return false;
}

/*************************************************************/

bool Operator::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    _ERROR(INTERNAL_ERROR);
    return false;
}

bool Operator::IsApplicable(Type *pLeftType, Type *pRightType) {
    assert(0);
    return false;
}

bool Operator::IsApplicableFixed(Type *pLeftType, Type *pRightType) {
    switch (pLeftType->GetType()) {
        case TYPE_I8:
        case TYPE_UI8:
        case TYPE_I16:
        case TYPE_UI16:
        case TYPE_I32:
        case TYPE_UI32:
        case TYPE_I64:
        case TYPE_UI64:
            switch (pRightType->GetType()) {
                case TYPE_I8:
                case TYPE_UI8:
                case TYPE_I16:
                case TYPE_UI16:
                case TYPE_I32:
                case TYPE_UI32:
                case TYPE_I64:
                case TYPE_UI64:
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

bool Operator::IsApplicableAnyNum(Type *pLeftType, Type *pRightType) {
    switch (pLeftType->GetType()) {
        case TYPE_I8:
        case TYPE_UI8:
        case TYPE_I16:
        case TYPE_UI16:
        case TYPE_I32:
        case TYPE_UI32:
        case TYPE_I64:
        case TYPE_UI64:
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
            switch (pRightType->GetType()) {
                case TYPE_I8:
                case TYPE_UI8:
                case TYPE_I16:
                case TYPE_UI16:
                case TYPE_I32:
                case TYPE_UI32:
                case TYPE_I64:
                case TYPE_UI64:
                case TYPE_FLOAT:
                case TYPE_DOUBLE:
                    return true;
                default:
                    return false;
            }
        default:
            return false;
    }
}

void Operator::SerializeBody(Serializer *pSerializer) {
    if (m_pLeft) {
        m_pLeft->Serialize(pSerializer);
        pSerializer->Begin(TYPE_EMPTY);
    }
    pSerializer->WriteSymbol(m_symDesc);
    if (m_pRight) {
        pSerializer->Begin(TYPE_EMPTY);
        m_pRight->Serialize(pSerializer);
    }
}

/*************************************************************/

RUNTIME_RETURN OperatorAssign::Run(Runable::Param &param) {
    if (Variable *pLeftVariable = m_pLeft->QueryType<Variable>(TYPE_VAR)) {
        Value rightValue;
        Param rightParam(&rightValue, param);
        RUNTIME_RETURN ret = m_pRight->Run(rightParam);
_PROTECT_BEGIN
        if (ret == RT_NO_ERROR) {
            if (rightParam.m_pValue->GetType() == Value::NULL_TYPE) {
                param.m_pStackMemory->GetRunTimeError().SetError("Right value is NULL!", m_symDesc, 0, param.m_pStackMemory);
                return RT_ERROR;
            }
            ret = pLeftVariable->Run(param);
            if (ret == RT_NO_ERROR) {
                *param.m_pValue = rightValue;
            }
        }
_PROTECT_END(RT_ERROR)
        return ret;
    }
    return AssertError();
}

bool OperatorAssign::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(pReturnType == 0);
    Type *pLeftType = 0, *pRightType = 0;
    if (!m_pLeft) {
        _ERROR(ASSIGNMENT_NEEDS_LEFT_VALUE);
    }
    else if (!m_pRight) {
        _ERROR(ASSIGNMENT_NEEDS_RIGHT_VALUE);
    }
    else if (Variable *pLeftVariable = m_pLeft->QueryType<Variable>(TYPE_VAR)) {
        Variable *pLeftSubVariable = pLeftVariable->GetSubVariable();
        if (pLeftSubVariable->IsFunctionCall()) {
            FunctionRefType *pFunctionRefType = pLeftSubVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
            assert(pFunctionRefType);
            if (pFunctionRefType->GetReturnType()->GetPassBy() == Type::BY_VALUE) {
                _ERROR(CANNOT_ASSIGN_TO_NOT_REFERENCE_RETURN_TYPE);
                return false;
            }
        }
        else if (pLeftSubVariable->IsConst()) {
            _ERROR(LEFT_VALUE_CANNOT_BE_CONST);
            return false;
        }

        if (m_pLeft->IsExpression(pLeftType, pPrePostExp, error) && m_pRight->IsExpression(pRightType, pPrePostExp, error)) {
            if (IsApplicable(pLeftType, pRightType)) {
                pReturnType = pLeftType;
                return true;
            }
            else {
                _ERROR(TYPES_MISMATCHED);
            }
        }
    }
    else {
        _ERROR(ASSIGNMENT_NEEDS_LEFT_VALUE);
    }
    return false;
}

bool OperatorAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    Value::VALUE_TYPE leftType = pLeftType->GetValueType(),
                      rightType = pRightType->GetValueType();

    switch (leftType) {
        case Value::FUNCTION_REF_TYPE:
            return (rightType == Value::FUNCTION_REF_TYPE) && pLeftType->IsEqualType(pRightType);
        case Value::BOOL_TYPE:
        case Value::STRING_TYPE:
            if ((rightType & Value::NUM_TYPE_FLAG) || (rightType == Value::STRING_TYPE) || (rightType == Value::BOOL_TYPE)) {
                return true;
            }
            if (rightType == Value::ARRAY_TYPE) {
                ArrayType *pArrayType = static_cast<ArrayType*>(pRightType);
                if (pArrayType->GetDynExpression()) {
                    return false;
                }
                if (pArrayType->GetDimensionCount() == 0) {
                    return false;
                }
                Type *pType = pArrayType->GetTypeNode();
                if (pType->GetPassBy() != Type::BY_VALUE) {
                    return false;
                }
                switch (pArrayType->GetTypeNode()->GetType()) {
                    case TYPE_UI8:
                    case TYPE_I8:
                        return true;
                    default:
                        break;
                }
            }
            return false;
        case Value::AGGREGATE_TYPE:
            return pLeftType->IsEqualType(pRightType);
        case Value::ARRAY_TYPE:
            if (rightType == Value::STRING_TYPE) {
                ArrayType *pArrayType = static_cast<ArrayType*>(pLeftType);
                if (pArrayType->GetDynExpression()) {
                    return false;
                }
                if (pArrayType->GetDimensionCount() == 0) {
                    return false;
                }
                Type *pType = pArrayType->GetTypeNode();
                if (pType->GetPassBy() != Type::BY_VALUE) {
                    return false;
                }
                switch (pArrayType->GetTypeNode()->GetType()) {
                    case TYPE_UI8:
                    case TYPE_I8:
                        return true;
                    default:
                        break;
                }
                return false;
            }
            return pLeftType->IsRelatedType(pRightType);
        case Value::LIB_TYPE:
            return false;
        default:
            return ((leftType & Value::NUM_TYPE_FLAG) != 0) && ((rightType & Value::NUM_TYPE_FLAG) || (rightType == Value::BOOL_TYPE));
    }
}

/*************************************************************/

RUNTIME_RETURN OperatorPrePost::Run(Runable::Param &param) {
    Variable *pVariable = ((m_pParent->GetType() == TYPE_SUBSCRIPT) ? m_pParent->GetParent() : m_pParent)->QueryType<Variable>(TYPE_VAR);
    if (!pVariable) {
        return AssertError();
    }
    pVariable = pVariable->GetRootVariableNode();
    Value value, *pValueRef = param.m_pValue ? param.m_pValue : &value;
    Param _param(pValueRef, param);
    RUNTIME_RETURN ret = pVariable->Run(_param);
    if (ret == RT_NO_ERROR) {
        Value incValue;
_PROTECT_BEGIN
        incValue.Set((ui8)1);
        ret = Operation(*pValueRef, incValue) ? RT_NO_ERROR : param.m_pStackMemory->GetRunTimeError().SetError("INCOMPATIBLE_TYPE", m_symDesc, 0, param.m_pStackMemory);
_PROTECT_END(RT_ERROR)    }
    return ret;
}

bool OperatorPrePost::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(pReturnType == 0);
    Variable *pVariable;
    switch (GetParent()->GetType()) {
        case TYPE_VAR:
            pVariable = GetParent()->QueryType<Variable>(TYPE_VAR);
            break;
        case TYPE_SUBSCRIPT:
            pVariable = GetParent()->GetParent()->QueryType<Variable>(TYPE_VAR);
            break;
        default:
            pVariable = 0;
            break;
    }
    if (pVariable) {
        Variable *pRootVariable = pVariable->GetRootVariableNode();
        assert(pRootVariable);
        if (Node *pSubNode = pRootVariable->FindFirstMemberSubscriptOrFooCall()) {
            switch (pSubNode->GetType()) {
                case TYPE_FUNCTION_CALL:
                    if (pSubNode->GetRight()) {
                        _ERROR(OPERATOR_NOT_ALLOWED_ON_FUNCTION_CALL);
                    }
                    else {
                        _ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
                    }
                    break;
                case TYPE_SUBSCRIPT:
                    _ERROR(OPERATOR_NOT_ALLOWED_ON_ARRAY);
                    break;
                default:
                    _ERROR(INTERNAL_ERROR);
                    break;
            }
        }
        else {
        //if (!pSubVariable->IsFunctionCall()) {
            //if (pSubVariable->GetRight()) {
            //    _ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
            //    return false;
            //}
            Variable *pSubVariable = pRootVariable->GetSubVariable();
            Type *pVarType = 0;
            if (pSubVariable != pVariable) {
                if (!pSubVariable->IsExpression(pVarType, pPrePostExp, error))
                {
                    return false;
                }
                pReturnType = pVarType;
            }
            else {
                pVarType = pSubVariable->GetTypeNode();
                if ((pVarType->GetType() == TYPE_ARRAY) && (pSubVariable->GetRight())) { // subscript object
                    pVarType = static_cast<ArrayType*>(pVarType)->GetTypeNode();
                }
            }
            if (IsApplicable(pVarType, 0)) {
                if (GetPrePostExpression(pPrePostExp, error)) {
                    if (GetLeft()) {
                        return GetLeft()->IsExpression(pReturnType, pPrePostExp, error);
                    }
                    pReturnType = pVarType;
                    return true;
                }
            }
            else {
                _ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
            }
        //}
        //else {
        //    _ERROR(OPERATOR_NOT_ALLOWED_ON_FUNCTION_CALL);
        }
    }
    else {
        _ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
    }
    return false;
}

bool OperatorPrePost::IsApplicable(Type *pLeftType, Type *pRightType) {
    Value::VALUE_TYPE leftType = pLeftType->GetValueType();
    return (leftType & Value::NUM_TYPE_FLAG) == Value::FIXED_NUM_TYPE_FLAG;
}

bool OperatorPrePost::GetPreExpression(PrePostExpressions *&pPreExp, Error &error) {
    if (!pPreExp) {
        pPreExp = new PrePostExpressions(m_symDesc);
        if (!pPreExp) {
            _ERROR(OUT_OF_MEMORY);
            return false;
        }
    }
    pPreExp->PushPreExpression(this);
    return true;
}

bool OperatorPrePost::GetPostExpression(PrePostExpressions *&pPostExp, Error &error) {
    if (!pPostExp) {
        pPostExp = new PrePostExpressions(m_symDesc);
        if (!pPostExp) {
            _ERROR(OUT_OF_MEMORY);
            return false;
        }
    }
    pPostExp->PushPostExpression(this);
    return true;
}

/*************************************************************/
bool OperatorInc::Operation(Value &value, Value &delta) {
    ERROR_TYPE err = value.Add(delta);
    return (NO_ERROR == err);
}

/*************************************************************/
bool OperatorDec::Operation(Value &value, Value &delta) {
    ERROR_TYPE err = value.Sub(delta);
    return (NO_ERROR == err);
}

/*************************************************************/

bool OperatorPreInc::GetPrePostExpression(PrePostExpressions *&pPrePostExp, Error &error) {
    return GetPreExpression(pPrePostExp, error);
}

/*************************************************************/
bool OperatorPostInc::GetPrePostExpression(PrePostExpressions *&pPrePostExp, Error &error) {
    return GetPostExpression(pPrePostExp, error);
}

/*************************************************************/

bool OperatorPreDec::GetPrePostExpression(PrePostExpressions *&pPrePostExp, Error &error) {
    return GetPreExpression(pPrePostExp, error);
}

/*************************************************************/
bool OperatorPostDec::GetPrePostExpression(PrePostExpressions *&pPrePostExp, Error &error) {
    return GetPostExpression(pPrePostExp, error);
}

/*************************************************************/

RUNTIME_RETURN OperatorBitNot::Run(Runable::Param &param) {
    assert(m_pRight);
    Value value;
    Param _param(&value, param);
    RUNTIME_RETURN ret = m_pRight->Run(_param);
    if (ret == RT_NO_ERROR) {
        if (_param.m_pValue->GetType() == Value::NULL_TYPE) {
            param.m_pStackMemory->GetRunTimeError().SetError("Value is NULL!", m_symDesc, 0, param.m_pStackMemory);
            return RT_ERROR;
        }

_PROTECT_BEGIN
        ERROR_TYPE err = value.BitNot(value);
        if (NO_ERROR == err) {
            if (param.m_pValue) {
                *param.m_pValue = value;
            }
            return RT_NO_ERROR;
        }
        else {
            ret = param.m_pStackMemory->GetRunTimeError().SetError("INCOMPATIBLE_TYPE", m_symDesc, 0, param.m_pStackMemory);
        }
_PROTECT_END(RT_ERROR)
    }
    return ret;
}

bool OperatorBitNot::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    assert(pReturnType == 0);
    if (m_pRight != 0) {
        if (m_pRight->IsExpression(pReturnType, pPrePostExp, error)) {
            if (IsApplicable(0, pReturnType)) {
                return true;
            }
            _ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
        }
    }
    else {
        _ERROR(OPERATOR_NEEDS_LEFT_VALUE);
    }
    return false;
}

bool OperatorBitNot::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

RUNTIME_RETURN OperatorNot::Run(Runable::Param &param) {
    Value value;
    Param _param(&value, param);
    RUNTIME_RETURN ret = m_pRight->Run(_param);
    if (ret == RT_NO_ERROR) {
        if (_param.m_pValue->GetType() == Value::NULL_TYPE) {
            param.m_pStackMemory->GetRunTimeError().SetError("Value is NULL!", m_symDesc, 0, param.m_pStackMemory);
            return RT_ERROR;
        }
_PROTECT_BEGIN
        param.m_pValue->Set(!value.GetBool());
_PROTECT_END(RT_ERROR)
        return ret;
    }
    return ret;
}

bool OperatorNot::IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error) {
    if (m_pRight != 0) {
        Node *_pReturnNode = 0;
        if (m_pRight->IsExpression(pReturnType, pPrePostExp, error)) {
            pReturnType = Scope::GetDummyTypeNode(TYPE_BOOL);
            return true;
        }
    }
    else {
        _ERROR(OPERATOR_NEEDS_LEFT_VALUE);
    }
    return false;
}

bool OperatorNot::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorEqual::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorXorAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    if (IsApplicableFixed(pLeftType, pRightType)) {
        return true;
    }
    return (pLeftType->GetType() == TYPE_BOOL) && (pRightType->GetType() == TYPE_BOOL);
}

/*************************************************************/

bool OperatorXor::IsApplicable(Type *pLeftType, Type *pRightType) {
    if (IsApplicableFixed(pLeftType, pRightType)) {
        return true;
    }
    return (pLeftType->GetType() == TYPE_BOOL) && (pRightType->GetType() == TYPE_BOOL);
}

/*************************************************************/

bool OperatorNotEqual::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorModAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorMod::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorDivAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableAnyNum(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorDiv::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableAnyNum(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorMulAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableAnyNum(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorMul::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableAnyNum(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorShiftRight::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorGreaterEq::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorGreater::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorShiftLeft::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorLessEq::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorLess::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorBitOr::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorBitOrAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorOr::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorBitAnd::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorBitAndAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableFixed(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorAnd::IsApplicable(Type *pLeftType, Type *pRightType) {
    return true;
}

/*************************************************************/

bool OperatorSubAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableAnyNum(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorSub::IsApplicable(Type *pLeftType, Type *pRightType) {
    return IsApplicableAnyNum(pLeftType, pRightType);
}

/*************************************************************/

bool OperatorAddAssign::IsApplicable(Type *pLeftType, Type *pRightType) {
    if (IsApplicableAnyNum(pLeftType, pRightType)) {
        return true;
    }
    return pLeftType->GetType() == TYPE_STRING;
}

/*************************************************************/

bool OperatorAdd::IsApplicable(Type *pLeftType, Type *pRightType) {
    if (IsApplicableAnyNum(pLeftType, pRightType)) {
        return true;
    }
    return pLeftType->GetType() == TYPE_STRING;
}

/*************************************************************/

bool MathAssignOperator::IsApplicable(Type *pLeftType, Type *pRightType) {
    return false;
}

/*************************************************************/

bool MathOperator::IsApplicable(Type *pLeftType, Type *pRightType) {
    return false;
}

/*************************************************************/

#ifdef _DEBUG
string s_line;
const char* line(Node* pNode) {
    string name;
    SymbolStore::GetSymbolStore()->GetSymbolName(pNode->GetSymbolId(), name);
    SYMBOL_DESC desc;
    pNode->GetSymbolDesc(desc);
    //s_line.clear();
    s_line = name;
    s_line += " (line: " + to_string(desc.m_line) + ", pos: " + to_string(desc.m_pos) + ")";
    return s_line.c_str();
}

#endif _DEBUG

/*************************************************************/
