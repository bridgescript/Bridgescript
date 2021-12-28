#include "stdafx.h"
#include "Compiler.h"
#include <assert.h>
#include "Script.h"
#include "Expressions.h"
#include "build.h"

#ifdef ERROR
#undef ERROR
#endif // ERROR
#define ERROR(e) m_error.SetError(e, SCRIPT_SYM_TO_STR(e), __FILE__, __FUNCTION__, __LINE__)

using namespace script;

const char* const s_basicSymbols[BASIC_SYMBOL_COUNT] = { "", "()", "(", "[]", "[" };

SCRIPT_COMPILE_ASSERT(sizeof(s_basicSymbols) / sizeof(s_basicSymbols[0]) - BASIC_SYMBOL_COUNT);

/**************************************************************/
SymbolStore::SymbolStore() :
    m_lastBaseSymbolId(0) {
    s_pSymbolStore = this;
}

//SymbolStore::~SymbolStore() {
//}

bool SymbolStore::Reset() {
    m_NameToSymbolIdMap.clear();
    m_SymboIdToName.clear();

    SYMBOL_ID symId = UpdateSymbolMap(string(s_basicSymbols[SYMBOL_ID_BLANK])); // 0
    assert(SYMBOL_ID_BLANK == symId);

    symId = UpdateSymbolMap(string(s_basicSymbols[SYMBOL_ID_PARENTHESIS])); // 1
    assert(SYMBOL_ID_PARENTHESIS == symId);

    symId = UpdateSymbolMap(string(s_basicSymbols[SYMBOL_ID_OPENPARENTHESIS])); // 2
    assert(SYMBOL_ID_OPENPARENTHESIS == symId);

    symId = UpdateSymbolMap(string(s_basicSymbols[SYMBOL_ID_SUBSCRIPT])); // 3
    assert(SYMBOL_ID_SUBSCRIPT == symId);

    symId = UpdateSymbolMap(string(s_basicSymbols[SYMBOL_ID_OPENSUBSCRIPT])); // 4
    assert(SYMBOL_ID_OPENSUBSCRIPT == symId);

    InitCoreSymbolsMap();
    return true;
}

void SymbolStore::InitCoreSymbolsMap() {
    //must rearragne to get most of the compressed int
    UpdateSymbolMap(string(")"));
    UpdateSymbolMap(string("{"));
    UpdateSymbolMap(string("}"));
    //UpdateSymbolMap(string("["));
    UpdateSymbolMap(string("]"));
    // operators: (+,++,+=) (-,--,-=) (&,&&,&=) (|,||,|=) (<,<<,<=,<<=) (>,>>,>=,>>=) (*,*=) (/,/=) (%,%=) (~,~= invalid) (^,^=) (!,!=) (=,==) @=
    UpdateSymbolMap(string("+"));
    UpdateSymbolMap(string("++"));
    UpdateSymbolMap(string("+="));
    UpdateSymbolMap(string("-"));
    UpdateSymbolMap(string("--"));
    UpdateSymbolMap(string("-="));
    UpdateSymbolMap(string("&"));
    UpdateSymbolMap(string("&&"));
    UpdateSymbolMap(string("&="));
    UpdateSymbolMap(string("|"));
    UpdateSymbolMap(string("||"));
    UpdateSymbolMap(string("|="));
    UpdateSymbolMap(string("<"));
    UpdateSymbolMap(string("<<"));
    UpdateSymbolMap(string("<="));
    UpdateSymbolMap(string("<<="));
    UpdateSymbolMap(string(">"));
    UpdateSymbolMap(string(">>"));
    UpdateSymbolMap(string(">="));
    UpdateSymbolMap(string(">>="));
    UpdateSymbolMap(string("*"));
    UpdateSymbolMap(string("*="));
    UpdateSymbolMap(string("/"));
    UpdateSymbolMap(string("/="));
    UpdateSymbolMap(string("%"));
    UpdateSymbolMap(string("%="));
    UpdateSymbolMap(string("~"));
    UpdateSymbolMap(string("^"));
    UpdateSymbolMap(string("^="));
    UpdateSymbolMap(string("!"));
    UpdateSymbolMap(string("!="));
    UpdateSymbolMap(string("="));
    UpdateSymbolMap(string("=="));
    UpdateSymbolMap(string("@="));

    UpdateSymbolMap(string(";"));
    UpdateSymbolMap(string(","));
    UpdateSymbolMap(string("."));
    UpdateSymbolMap(string(":"));
    UpdateSymbolMap(string("@"));
    UpdateSymbolMap(string("bool"));
    UpdateSymbolMap(string("i8"));
    UpdateSymbolMap(string("ui8"));
    UpdateSymbolMap(string("i16"));
    UpdateSymbolMap(string("ui16"));
    UpdateSymbolMap(string("i32"));
    UpdateSymbolMap(string("ui32"));
    UpdateSymbolMap(string("i64"));
    UpdateSymbolMap(string("ui64"));
    UpdateSymbolMap(string("float"));
    UpdateSymbolMap(string("double"));
    UpdateSymbolMap(string("string"));
    UpdateSymbolMap(string("true"));
    UpdateSymbolMap(string("false"));

    UpdateSymbolMap(string("function"));
    UpdateSymbolMap(string("return"));
    UpdateSymbolMap(string("for"));
    UpdateSymbolMap(string("if"));
    UpdateSymbolMap(string("else"));
    UpdateSymbolMap(string("while"));
    UpdateSymbolMap(string("break"));
    UpdateSymbolMap(string("switch"));
    UpdateSymbolMap(string("case"));
    UpdateSymbolMap(string("default"));
    UpdateSymbolMap(string("continue"));

    UpdateSymbolMap(string("include"));
    UpdateSymbolMap(string("lib"));
    UpdateSymbolMap(string("struct"));
    UpdateSymbolMap(string("array"));
    UpdateSymbolMap(string("error"));
    //UpdateSymbolMap(string("sizeof"));
    //UpdateSymbolMap(string("arr2str"));
    //UpdateSymbolMap(string("warr2str"));
    //UpdateSymbolMap(string("str2arr"));
    //UpdateSymbolMap(string("str2warr"));
    //UpdateSymbolMap(string("lock"));
    //UpdateSymbolMap(string("unlock"));
    //UpdateSymbolMap(string("cast"));
    // error members
    UpdateSymbolMap(string("name"));
    UpdateSymbolMap(string("line"));
    UpdateSymbolMap(string("position"));
    UpdateSymbolMap(string("trace"));
    UpdateSymbolMap(string("symbol"));

    m_lastBaseSymbolId = m_SymboIdToName.size() - 1;
}

SYMBOL_ID SymbolStore::UpdateSymbolMap(string& name) {
    pair<SYMBOL_MAP::iterator, bool> res = m_NameToSymbolIdMap.insert(pair<string, SYMBOL_ID>(name, 0));
    if (res.second) {
        res.first->second = m_SymboIdToName.size();
        m_SymboIdToName.push_back(res.first);
    }
    return res.first->second;
}

bool SymbolStore::GetSymbolName(SYMBOL_ID symId, string &name) {
    SYMBOL_ID maxSymId = m_SymboIdToName.size();
    if (maxSymId && (maxSymId > symId)) {
        name = m_SymboIdToName[symId]->first;
        return true;
    }
    return false;
}

bool SymbolStore::GetSymbolName(Node * const pNode, string &name) {
    if (pNode) {
        SYMBOL_DESC symDesc;
        pNode->GetSymbolDesc(symDesc);
        return GetSymbolName(symDesc.m_symId, name);
    }
    return false;
}

bool SymbolStore::GetSymbolId(string &name, SYMBOL_ID &symId) {
    SYMBOL_MAP::iterator it = m_NameToSymbolIdMap.find(name);
    if (it != m_NameToSymbolIdMap.end()) {
        symId = it->second;
        return true;
    }
    return false;
}

ui32 SymbolStore::GetFileId(string fileName) {
    pair<FILE_ID_MAP::iterator, bool> res = m_FileNameToIdMap.insert(pair<string, ui32>(fileName, 0));
    if (res.second) {
        res.first->second = m_FileIdToName.size();
        m_FileIdToName.push_back(res.first);
    }
    return res.first->second;
}

bool SymbolStore::GetFileName(ui32 fileId, string &name) {
    SYMBOL_ID maxFileId = m_FileIdToName.size();
    if (maxFileId && (maxFileId > fileId)) {
        name = m_FileIdToName[fileId]->first;
        return true;
    }
    return false;
}

void SymbolStore::GetFileNames(vector<string> &fileNames) {
    fileNames.clear();
    FILE_ID_MAP::iterator it = m_FileNameToIdMap.begin();
    for (; it != m_FileNameToIdMap.end(); ++it) {
        fileNames.push_back(it->first);
    }
}

bool SymbolStore::Dump(Serializer &serializer) {
    for (ui32 i = m_lastBaseSymbolId + 1; i < m_SymboIdToName.size(); ++i) {
        serializer.WriteRaw((ui8*)m_SymboIdToName[i]->first.c_str(), m_SymboIdToName[i]->first.length() + 1);
    }
    serializer.WriteRaw((ui8*)"", 1);
    return true;
}

/**************************************************************/
BinaryHeader::BinaryHeader() :
    m_scriptSignature(SCRIPT_SINATURE), 
    m_timeStatmp(__TIMESTAMP__), 
    m_scriptMajorVersion(SCRIPT_MAJOR_VERSION), 
    m_scriptMinorVersion(SCRIPT_MINOR_VERSION) {
}

bool BinaryHeader::Load(ui8 *&pData, ui32 &size) {
    if (size < m_scriptSignature.length()) {
        return false;
    }
    string sig;
    sig.append((char*)pData, m_scriptSignature.length());
    if (sig != m_scriptSignature) {
        return false;
    }
    size -= m_scriptSignature.length();
    pData += m_scriptSignature.length();

    if ((size < sizeof(ui16)) || (SCRIPT_MAJOR_VERSION < (ui16)*pData)) {
        return false;
    }
    m_scriptMajorVersion = (ui16)*pData;
    size -= sizeof(ui16);
    pData += sizeof(ui16);

    if ((size < sizeof(ui16)) || (SCRIPT_MINOR_VERSION < (ui16)*pData)) {
        return false;
    }
    m_scriptMinorVersion = (ui16)*pData;
    size -= sizeof(ui16);
    pData += sizeof(ui16);

    if (size < m_timeStatmp.length()) {
        return false;
    }
    ui32 len = m_timeStatmp.length();
    m_timeStatmp.clear();
    m_timeStatmp.append((char*)pData, len);
    size -= m_timeStatmp.length();
    pData += m_timeStatmp.length();

    return true;
}

void BinaryHeaderSave(Serializer &serializer) {

}
/**************************************************************/
bool InterpreterTest::DispatchEvent(NODE_TYPE type, string &token) {
    static ui32 prevLine = 1;
#ifdef _DEBUG
    if (m_pReader->GetLine() > prevLine) {
        printf("\n");
        prevLine = m_pReader->GetLine();
    }
    printf("%s[%d,%d] ", token.c_str(), m_pReader->GetLine(), m_pReader->GetPosition());
#else // _DEBUG
    printf("%s ", token.c_str());
#endif // _DEBUG
    return true;
}
/**************************************************************/
string Interpreter::s_semicolon = ";";
string Interpreter::s_empty     = "";
//#ifdef _DEBUG
//static SymbolStore *s_pSymbolStore;
//SymbolStore *GetSymbolStore() {
//    return s_pSymbolStore;
//}
//#endif // _DEBUG
SymbolStore *SymbolStore::s_pSymbolStore = 0;

Interpreter::Interpreter(Error &error, StdOutInterface *pStdOutInterface) : InterpreterIf(error, pStdOutInterface),
    m_pRunableRoot(0),
    m_pRunableCurrent(0)
#ifdef TRACE_OBJECTS
    ,m_loaded(false) 
#endif // TRACE_OBJECTS
{
    Memory::SetInstance(&m_Memory);
//#ifdef _DEBUG
    //change it later!!
//    s_pSymbolStore = &m_symbolStore;
//#endif // _DEBUG
}

Interpreter::~Interpreter() {
    if (m_pRunableRoot) {
        delete m_pRunableRoot;
    }
#ifdef TRACE_OBJECTS
    //interpreter.DeleteRunable();
    if (m_loaded) {
        Memory::GetInstance().DumpObjects();
    }
#endif // TRACE_OBJECTS
    Memory::SetInstance(0);
}

#ifdef SCRIPT_DEBUGGER

void Interpreter::PreRun(DebuggerEvents *pDebuggerEvents) {
    m_debugger.SetInterpreter(this);
    m_debugger.SetAction(&m_debuggerHandler);
    m_debuggerHandler.SetDebugger(&m_debugger);
    m_debuggerHandler.SetDebuggerNotify(pDebuggerEvents);
}

void Interpreter::PostRun() {
    m_debugger.SetAction(0);
    m_debugger.SetInterpreter(0);
}
#else // SCRIPT_DEBUGGER
#endif // SCRIPT_DEBUGGER

bool Interpreter::Run() {
    SE_Exception::Init();

    if (m_pRunableRoot) {
        Value value;
        ThreadId threadId = GetCurrentThreadId();
        Runable::Param param(&value, &m_Memory, m_Memory.GetThreadMemory(threadId));
        StackMemory *pStackMemory = param.m_pStackMemory;
        m_Memory.SetRootScope(m_pRunableRoot);
        if (Scope *pScope = m_pRunableRoot->QueryType<Scope>(TYPE_SCOPE)) {
            pStackMemory->PushFrame(0, pScope, pScope->GetLocalVarDescriptors());
            pStackMemory->SetCurRunningNode(m_pRunableRoot);
#ifdef SCRIPT_DEBUGGER
            FlowSemaphor *pFlowSemaphor = pStackMemory->GetFlowSemaphor();
            DebuggerEvents *pNotify = pFlowSemaphor->GetDebuggerNotify();
            m_debuggerHandler.Ready();
            pNotify->OnBeginThread(threadId);
            pNotify->OnRun(threadId); // just to disable main thread tab
#endif // SCRIPT_DEBUGGER
            RUNTIME_RETURN ret;
            ui32 se = 0;
#ifdef SCRIPT_DEBUGGER
            ret = pFlowSemaphor->CheckFlow(pScope, 0, unordered_set<ui64>());
            if (RT_STOP == ret) {
                pFlowSemaphor->ResetOnEndThread();
                pNotify->OnEndThread(threadId);
                pFlowSemaphor->Stop();
                ui32 frameIdx = pStackMemory->PopFrame();
                assert(INVALID_STACK_FRAME_IDX == frameIdx);
                m_Memory.ReleaseThreadMemory(threadId);
                return ret != RT_ERROR;
            }
#endif // SCRIPT_DEBUGGER
            try {
                ret = m_pRunableRoot->Run(param);
            }
            catch (SE_Exception &e) {
                ret = RT_EXCEPTION;
                se = e.getSeNumber();
            }
#ifdef SCRIPT_DEBUGGER
            TODO("Must kill all treads?");
            pFlowSemaphor->ResetOnEndThread();
            pNotify->OnEndThread(threadId);
            pFlowSemaphor->Stop();
#endif // SCRIPT_DEBUGGER
            ui32 frameIdx = pStackMemory->PopFrame();
            if (RT_EXCEPTION == ret) {
                while (frameIdx != INVALID_STACK_FRAME_IDX) {
                    frameIdx = pStackMemory->PopFrame();
                };
            }
            assert(INVALID_STACK_FRAME_IDX == frameIdx);
            m_Memory.ReleaseThreadMemory(threadId);
            if (RT_EXCEPTION == ret) {
                throw(SE_Exception(se));
            }
            return ret != RT_ERROR;
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(NULL_SCRIPT);
    }
    return false;
}

bool Interpreter::Init() {
    m_stateEventStack.clear();
    m_stateEventStack.push_back(&Interpreter::ScopeEvent);
    m_symbolStore.Reset();
    /******************************************/
    // testing symbols
    //string token;
    //for (ui32 i = 0; i < 0xffff; ++i) {
    //    token = "q" + to_string(i);
    //    m_symbolStore.UpdateSymbolMap(token);
    //}
    /******************************************/

    DeleteRunable();

    m_pRunableRoot = CreateNode<Scope>(s_empty);
#ifdef SCRIPT_DEBUGGER
    m_pRunableRoot->SetDebuggerHandler(&m_debuggerHandler);
    m_pRunableRoot->SetBreakPointLineByFileId(&m_breakPointLines);
#endif // SCRIPT_DEBUGGER
    m_pRunableCurrent = m_pRunableRoot;
    m_Memory.SetRootScope(m_pRunableRoot);
    GetBuiltInFunction(TYPE_CAST, string("cast"));
    GetBuiltInFunction(TYPE_LOCK, string("lock"));
    GetBuiltInFunction(TYPE_UNLOCK, string("unlock"));
    GetBuiltInFunction(TYPE_SIZEOF, string("sizeof"));
    GetBuiltInFunction(TYPE_ARR2STR, string("arr2str"));
    GetBuiltInFunction(TYPE_WARR2STR, string("warr2str"));
    GetBuiltInFunction(TYPE_STR2ARR, string("str2arr"));
    GetBuiltInFunction(TYPE_STR2WARR, string("str2warr"));
    return (m_pRunableCurrent != 0);
}

void Interpreter::DeleteRunable() {
    m_pRunableCurrent = 0;

    if (m_pRunableRoot) {
        delete m_pRunableRoot;
        m_pRunableRoot = 0;
        m_Memory.SetRootScope(0);
    }
}

bool Interpreter::IsValidEndState() {
    if (m_stateEventStack.back() == &Interpreter::ElseEvent) {
        Node *pNode = m_pRunableCurrent;
        if (pNode->GetType() == TYPE_IF) {
            while (pNode) {
                switch (pNode->GetType()) {
                    case TYPE_IF:
                    case TYPE_FOR:
                    case TYPE_WHILE:
                        pNode = pNode->GetParent();
                        break;
                    default:
                        return (m_pRunableRoot == pNode);
                }
            }
        }
        return (m_pRunableRoot == pNode);
    }
    else {
        return (m_pRunableRoot == m_pRunableCurrent);
    }
    // check states as well?
    //switch (m_stateEventStack.size()) {
    //    case 0:
    //        return false;
    //    //case 2:
    //    //    if (m_stateEventStack.back() != &Interpreter::BeginScopeExpression) {
    //    //        return false;
    //    //    }
    //    case 1:
    //        //return (m_stateEventStack[0] == &Interpreter::BeginGlobalScopeEvent);
    //        return (m_stateEventStack[0] == &Interpreter::ScopeEvent);
    //    default: {
    //        /* valid stack:
    //            1) BeginScopeExpression
    //            2) BeginScopeExpression, IncludeEvent, ...
    //            ...
    //            ElseEvent, IfEvent, and stack above
    //            or
    //            ElseEvent, IfEvent, [IfBodyEvent, IfEvent]..., and stack above
    //        */
    //        ui32 size = m_stateEventStack.size();
    //        if (m_stateEventStack[size - 1] == &Interpreter::BeginScopeExpression) {
    //            if (m_stateEventStack[size - 1] == &Interpreter::IncludeEvent) {
    //                return true;
    //            }
    //            return false;
    //        }
    //        if (m_stateEventStack[size - 1] == &Interpreter::ElseEvent) {
    //            if (size > 2) {
    //                if (m_stateEventStack[size - 2] == &Interpreter::IfEvent) {
    //                    if (m_stateEventStack[size - 3] == &Interpreter::BeginScopeExpression) {
    //                        if (size == 3) {
    //                            return true;
    //                        }
    //                        if (m_stateEventStack[size - 4] == &Interpreter::IncludeEvent) {
    //                            return true;
    //                        }
    //                        return false;
    //                    }
    //                    
    //                    if (m_stateEventStack[size - 3] == &Interpreter::IfBodyEvent) {
    //                        if (size > 3) {
    //                            if (m_stateEventStack[size - 4] == &Interpreter::IfEvent) {
    //                                if (size > 4) {
    //                                    if (m_stateEventStack[size - 5] == &Interpreter::BeginScopeExpression) {
    //                                        if (size == 6) {
    //                                            return true;
    //                                        }
    //                                        if (m_stateEventStack[size - 6] == &Interpreter::IncludeEvent) {
    //                                            return true;
    //                                        }
    //                                    }
    //                                }
    //                            }
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //        return false;
    //    }
    //}
}

bool Interpreter::DispatchEvent(NODE_TYPE type, string &token) {
    return (this->*m_stateEventStack.back())(type, token);
}

void Interpreter::DumpHeader(Serializer &serializer) {
    ui16 val;
    serializer.WriteRaw((ui8*)SCRIPT_SINATURE, sizeof(SCRIPT_SINATURE) - 1);
    val = SCRIPT_MAJOR_VERSION;
    serializer.WriteRaw((ui8*)&val, sizeof(ui16));
    val = SCRIPT_MINOR_VERSION;
    serializer.WriteRaw((ui8*)&val, sizeof(ui16)); 
    char *c = __TIMESTAMP__;
    serializer.WriteRaw((ui8*)c, strlen(c));
}

void Interpreter::DumpSymbols(Serializer &serializer) {
    serializer.SetSymbolStore(&m_symbolStore);
    m_symbolStore.Dump(serializer);
}

void Interpreter::DumpConsts(Serializer &serializer) {
}

void Interpreter::DumpScript(Serializer &serializer) {
    serializer.SetSymbolStore(&m_symbolStore);
    if (m_pRunableRoot) {
        m_pRunableRoot->Serialize(&serializer);
    }
}

template<class T> T* Interpreter::CreateNode(SYMBOL_ID symId) {
    SYMBOL_DESC symDesc(symId
        //#ifdef _DEBUG
        , m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
        //#endif // _DEBUG
        );
    return new T(m_pRunableCurrent, symDesc);
}

template<class T> T* Interpreter::CreateNode(string &token) {
    SYMBOL_ID   symId = m_symbolStore.UpdateSymbolMap(token);
    return CreateNode<T>(symId);
//    SYMBOL_DESC symDesc(symId
////#ifdef _DEBUG
//        ,m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
////#endif // _DEBUG
//        );
//    return new T(m_pRunableCurrent, symDesc);
}

Function* Interpreter::CreateBuiltInFunction(SymbolStore *pSymbolStore, NODE_TYPE type, SYMBOL_ID symId) {
    SYMBOL_DESC symDesc(symId
//#ifdef _DEBUG
        ,FILE_ID_MAX, LINE_MAX, POS_MAX
//#endif // _DEBUG
        );
    Function *pBuiltInFunction = 0;
    switch (type) {
        case TYPE_SIZEOF:
            pBuiltInFunction = new SizeOf(m_pRunableCurrent, symDesc);
            break;
        case TYPE_ARR2STR:
            pBuiltInFunction = new Arr2Str(&m_symbolStore, m_pRunableRoot, symDesc);
            break;
        case TYPE_WARR2STR:
            pBuiltInFunction = new WArr2Str(&m_symbolStore, m_pRunableRoot, symDesc);
            break;
        case TYPE_STR2ARR:
            pBuiltInFunction = new Str2Arr(&m_symbolStore, m_pRunableRoot, symDesc);
            break;
        case TYPE_STR2WARR:
            pBuiltInFunction = new Str2WArr(&m_symbolStore, m_pRunableRoot, symDesc);
            break;
        case TYPE_LOCK:
            pBuiltInFunction = new Lock(m_pRunableRoot, symDesc);
            break;
        case TYPE_UNLOCK:
            pBuiltInFunction = new UnLock(m_pRunableRoot, symDesc);
            break;
        case TYPE_CAST:
            pBuiltInFunction = new Cast(m_pRunableCurrent, symDesc);
            break;
        default:
            break;
    }
    if (pBuiltInFunction) {
        pBuiltInFunction->SetSymbolLocation(FILE_ID_MAX, LINE_MAX, POS_MAX);
        pBuiltInFunction->SetParent(m_pRunableRoot);
        if (!m_pRunableRoot->PushFunctionDef(pBuiltInFunction)) {
            delete pBuiltInFunction;
            pBuiltInFunction = 0;
            ERROR(DUPLICATE_FUNCTION_DEFINITION);
        }
    }
    else {
        ERROR(OUT_OF_MEMORY);
    }

    return pBuiltInFunction;
}

template<class T> T* Interpreter::CreateNode(SymbolStore *pSymbolStore, string &token) {
    SYMBOL_ID   symId = m_symbolStore.UpdateSymbolMap(token);
    SYMBOL_DESC symDesc(symId,
//#ifdef _DEBUG
        m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#else // _DEBUG
//        0, 0
//#endif // _DEBUG
        );
    return new T(pSymbolStore, m_pRunableCurrent, symDesc);
}
template<class T> T* Interpreter::CreateNode(NODE_TYPE type, string &token) {
    Node *pNode = 0;

    switch (type) {
        case TYPE_ASSIGN:
            pNode = CreateNode<OperatorAssign>(token);
            break;
        case TYPE_EQ:
            pNode = CreateNode<OperatorEqual>(token);
            break;
        case TYPE_SCOPE:
            pNode = CreateNode<Scope>(token);
#ifdef SCRIPT_DEBUGGER
            static_cast<Scope*>(pNode)->SetDebuggerHandler(&m_debuggerHandler);
            static_cast<Scope*>(pNode)->SetBreakPointLineByFileId(&m_breakPointLines);
#endif // SCRIPT_DEBUGGER
            break;
        case TYPE_LIB: {
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            SYMBOL_DESC symDesc(symId
//#ifdef _DEBUG
                ,m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#endif // _DEBUG
                );
            pNode = new LibType(m_pRunableCurrent, symDesc);
            break;
        }
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
        case TYPE_STRING: {
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            SYMBOL_DESC symDesc(symId
//#ifdef _DEBUG
                ,m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#endif // _DEBUG
                );
            pNode = new Type(m_pRunableCurrent, symDesc, type);
            break;
        }
        case TYPE_AGGREGATE_TYPE: 
            pNode = CreateNode<AggregateType>(token);
            break;
        case TYPE_FUNCTION_REF_TYPE:
            pNode = CreateNode<FunctionRefType>(token);
            break;
        case TYPE_ARRAY:
            pNode = CreateNode<ArrayType>(&m_symbolStore, token);
            break;
        case TYPE_VAR: {// check if it's type of TYPE_AGGREGATE_TYPE
             if (!(pNode = CreateVariableNode(0, token))) {
                return 0;
            }
            break;
        }
        case TYPE_MEMBER_ACCESS:
            pNode = CreateNode<MemberAccess>(token);
            break;
        case TYPE_CONST_BOOL: {
            pNode = CreateNode<ConstVariable>(&m_symbolStore, token);
            if (ConstVariable *pConstVariable = static_cast<ConstVariable*>(pNode)) {
                Type *pType = Scope::GetDummyTypeNode(TYPE_BOOL);
                //pConstVariable->SetSymbolId(m_symbolStore.UpdateSymbolMap(token));
                pConstVariable->SetTypeNode(pType);
                if (!pConstVariable->GetRef().Set(token, Value::BOOL_TYPE, pType)) {
                    ERROR(INTERNAL_ERROR);
                    delete pNode;
                    return false;
                }
            }
            break;
        }
        case TYPE_CONST: {
            //case TYPE_CONST_NUM:
            pNode = CreateNode<ConstVariable>(&m_symbolStore, token);
            if (ConstVariable *pConstVariable = static_cast<ConstVariable*>(pNode)) {
                Type *pType = Scope::GetDummyTypeNode(TYPE_STRING);
                //pConstVariable->SetSymbolId(m_symbolStore.UpdateSymbolMap(token));
                pConstVariable->SetTypeNode(pType);
                if (token.length() >= 2) { // revisit this later!
                    pConstVariable->GetRef().Set(token.substr(1, token.length() - 2), Value::STRING_TYPE, pType);
                }
            }
            break;
        }
        case TYPE_REF_ASSIGN:
            pNode = CreateNode<OperatorRefAssign>(token);
            break;
        case TYPE_ERROR:
            pNode = CreateNode<ErrorNode>(token);
            break;
        case TYPE_REF:
            pNode = CreateNode<Reference>(token);
            break;
        case TYPE_ADD:
            pNode = CreateNode<OperatorAdd>(token);
            break;
        case TYPE_ADD_ASSIGN:
            pNode = CreateNode<OperatorAddAssign>(token);
            break;
        case TYPE_INC_OP:
            switch(m_pRunableCurrent->GetType()) {
                case TYPE_VAR:
                    pNode = CreateNode<OperatorPostInc>(token);
                    break;
                case TYPE_CONST:
                case TYPE_CONST_BOOL:
                    ERROR(OPERATOR_NEEDS_VARIABLE);
                    return 0;
                case TYPE_LIB_VAR:
                    ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
                    return 0;
                case TYPE_FUNCTION_CALL:
                default:
                    pNode = CreateNode<OperatorPreInc>(token);
                    break;
            }
            if (pNode) {
                type = pNode->GetType();
            }
            break;
        case TYPE_SUB:
            pNode = CreateNode<OperatorSub>(token);
            break;
        case TYPE_SUB_ASSIGN:
            pNode = CreateNode<OperatorSubAssign>(token);
            break;
        case TYPE_DEC_OP:
            switch (m_pRunableCurrent->GetType()) {
                case TYPE_VAR:
                    pNode = CreateNode<OperatorPostDec>(token);
                    break;
                case TYPE_CONST:
                case TYPE_CONST_BOOL:
                    ERROR(OPERATOR_NEEDS_VARIABLE);
                    return 0;
                case TYPE_LIB_VAR:
                    ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
                    return 0;
                case TYPE_FUNCTION_CALL:
                default:
                    pNode = CreateNode<OperatorPreDec>(token);
                    break;
            }
            if (pNode) {
                type = pNode->GetType();
            }
            break;
        case TYPE_AND:
            pNode = CreateNode<OperatorAnd>(token);
            break;
        case TYPE_BIT_AND_ASSIGN:
            pNode = CreateNode<OperatorBitAndAssign>(token);
            break;
        case TYPE_BIT_AND:
            pNode = CreateNode<OperatorBitAnd>(token);
            break;
        case TYPE_OR:
            pNode = CreateNode<OperatorOr>(token);
            break;
        case TYPE_BIT_OR_ASSIGN:
            pNode = CreateNode<OperatorBitOrAssign>(token);
            break;
        case TYPE_BIT_OR:
            pNode = CreateNode<OperatorBitOr>(token);
            break;
        case TYPE_LESS:
            pNode = CreateNode<OperatorLess>(token);
            break;
        case TYPE_LESS_EQ:
            pNode = CreateNode<OperatorLessEq>(token);
            break;
        case TYPE_SHIFT_LEFT:
            pNode = CreateNode<OperatorShiftLeft>(token);
            break;
        case TYPE_GREATER:
            pNode = CreateNode<OperatorGreater>(token);
            break;
        case TYPE_GREATER_EQ:
            pNode = CreateNode<OperatorGreaterEq>(token);
            break;
        case TYPE_SHIFT_RIGHT:
            pNode = CreateNode<OperatorShiftRight>(token);
            break;
        case TYPE_MUL:
            pNode = CreateNode<OperatorMul>(token);
            break;
        case TYPE_MUL_ASSIGN:
            pNode = CreateNode<OperatorMulAssign>(token);
            break;
        case TYPE_DIV:
            pNode = CreateNode<OperatorDiv>(token);
            break;
        case TYPE_DIV_ASSIGN:
            pNode = CreateNode<OperatorDivAssign>(token);
            break;
        case TYPE_MOD:
            pNode = CreateNode<OperatorMod>(token);
            break;
        case TYPE_MOD_ASSIGN:
            pNode = CreateNode<OperatorModAssign>(token);
            break;
        case TYPE_BIT_NOT:
            pNode = CreateNode<OperatorBitNot>(token);
            break;
        case TYPE_NOT:
            pNode = CreateNode<OperatorNot>(token);
            break;
        case TYPE_NOT_EQ:
            pNode = CreateNode<OperatorNotEqual>(token);
            break;
        case TYPE_XOR:
            pNode = CreateNode<OperatorXor>(token);
            break;
        case TYPE_XOR_ASSIGN:
            pNode = CreateNode<OperatorXorAssign>(token);
            break;
        case TYPE_SUBSCRIPT:
            pNode = CreateNode<SubscriptNode>(token);
            break;
        case TYPE_PARENTHESIS:
            pNode = CreateNode<Parenthesis>(token);
            break;
        case TYPE_FOR:
            pNode = CreateNode<ForNode>(token);
            break;
        case TYPE_WHILE:
            pNode = CreateNode<WhileNode>(token);
            break;
        case TYPE_IF:
            pNode = CreateNode<IfNode>(token);
            break;
        case TYPE_RETURN:
            pNode = CreateNode<Return>(token);
            break;
        case TYPE_SWITCH:
            pNode = CreateNode<SwitchNode>(token);
            break;
        case TYPE_CASE:
            pNode = CreateNode<CaseNode>(&m_symbolStore, token);
            break;
        case TYPE_DEFAULT:
            pNode = CreateNode<DefaultNode>(token);
            break;
        case TYPE_CONTINUE:
            pNode = CreateNode<Continue>(token);
            break;
        case TYPE_BREAK:
            pNode = CreateNode<Break>(token);
            break;
        case TYPE_NONE:
        case TYPE_RUNABLE:
        case TYPE_OPERATOR:
        default:
            ERROR(INVALID_NODE_TYPE);
            return 0;
    }

    if (pNode) {
        T *pTypedNode = pNode->QueryType<T>(type);
        if (pTypedNode) {
            return pTypedNode;
        }
        ERROR(INVALID_TYPE);
        delete pNode;
    }
    else {
        ERROR(OUT_OF_MEMORY);
    }
    return 0;
}

Variable* Interpreter::CreateVariableNode(Type *pType, string &token) {
    Variable *pVariable = 0;
    SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
    Function *pFunction = 0;
    Type *pResolvedType = pType;
    NODE_TYPE type;

    MEMORY_OFFSET alignedOffset = INVALID_MEMORY_OFFSET;
    ui32 idx = 0xffffffff;
    SYMBOL_DESC symDesc(symId
//#ifdef _DEBUG
        ,m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#endif // _DEBUG
        );

    if (pResolvedType) {
        alignedOffset = ReserveAlignedStackSpotIfNotDeclared(symDesc, pResolvedType, idx);
        if (!(m_error == NO_ERROR)) {
            return 0;
        }
        type = pResolvedType->GetType();
    }
    else {
        alignedOffset = ResolveAlignedVariable(symId, pResolvedType, idx);
        if (!pResolvedType) {
            // this coud be a function name, search for function definitions
            if (pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                alignedOffset = INVALID_MEMORY_OFFSET;
                pResolvedType = pFunction->GetFunctionRefTypeNode();
                type = pFunction->GetType();
            }
            else {
                ERROR(UNDEFINED_SYMBOL);
                return 0;
            }
        }
        else {
            type = pResolvedType->GetType();
        }
    }

    assert(pResolvedType);
    
    switch (type) {
        case TYPE_LIB:
            pVariable = new LibNode(m_pRunableCurrent, symDesc, idx, alignedOffset, pResolvedType);
            break;
        case TYPE_ERROR_TYPE:
            pVariable = new ErrorVariable(&m_symbolStore, m_pRunableCurrent, symDesc, idx, Scope::GetDummyTypeNode(TYPE_ERROR_TYPE));
            break;
        case TYPE_CAST:
            pVariable = new CastVariable(m_pRunableCurrent, symDesc, idx, pFunction);
            break;
        default:
            pVariable = new Variable(m_pRunableCurrent, symDesc, idx, alignedOffset, 0, pResolvedType);
            break;
    }

    if (!pVariable) {
        ERROR(OUT_OF_MEMORY);
    }

    return pVariable;
}

Variable* Interpreter::CreateArgNode(string &token) {
    Variable *pVariable = 0;
    return pVariable;
}

Variable* Interpreter::CreateMemberVariableNode(string &token) {
    Variable *pVariable = 0;
    SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
    // get root variable node and member offset
    Variable *pParentVariable = 0;
    ui32 idx = 0;
    Type *pParentType = 0;
    Node *pParent = m_pRunableCurrent->GetParent();
    
    switch (pParent->GetType()) {
        case TYPE_VAR:
        case TYPE_LIB_VAR:
        case TYPE_ERROR_VAR:
            if (pParentVariable = static_cast<Variable*>(pParent)) {
                pParentType = pParentVariable->GetTypeNode();
                idx = pParentVariable->GetVarIdx();
                idx |= MEMORY_OFFSET_BIT_SUB_STACK;
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
        case TYPE_SUBSCRIPT: {
            SubscriptNode *pSubscriptNode = static_cast<SubscriptNode*>(pParent);
            pParentType = pSubscriptNode->GetArrayType()->GetTypeNode();
            idx = pSubscriptNode->GetVarIdx();
            break;
        }
        case TYPE_FUNCTION_CALL: {
            FunctionCall *pFunctionCall = pParent->QueryType<FunctionCall>(TYPE_FUNCTION_CALL);
            pParentType = pFunctionCall->GetFunctionRefTypeNode()->GetReturnType();
            idx = 0;// pParentVariable->GetVarIdx();
            idx |= MEMORY_OFFSET_BIT_SUB_STACK;
            break;
        }
        default:
            ERROR(INTERNAL_ERROR);
            break;
    }

    if (pParentType) {
        switch (pParentType->GetType()) {
            case TYPE_LIB:
            case TYPE_AGGREGATE_TYPE:
            case TYPE_ERROR_TYPE:
                if (Variable *pVariableDec = static_cast<AggregateType*>(pParentType)->FindMemberDecNode(symId)) {
                    SYMBOL_DESC symDesc(symId
//#ifdef _DEBUG
                        ,m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#endif // _DEBUG
                        );
                    MEMORY_OFFSET alignedOffset = pVariableDec->GetAlignedMemberOffset();
                    pVariable = new Variable(m_pRunableCurrent, symDesc, idx, alignedOffset, alignedOffset, pVariableDec->GetTypeNode());
                    if (!pVariable) {
                        ERROR(OUT_OF_MEMORY);
                    }
                }
                else {
                    ERROR(UNKNOWN_MEMBER_NAME);
                }
                break;
            default:
                ERROR(INTERNAL_ERROR);
                break;
        }
    }

    return pVariable;
}

Variable* Interpreter::CreateMemberDecNode(string &token) {
    Variable *pVariable = 0;
    SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
    if (AggregateType *pAggregateType = m_pRunableCurrent->GetParent()->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
        if (pAggregateType->FindMemberDecNode(symId)) {
            ERROR(DUPLICATE_DECLARATION);
        }
        else {
            Node *pRootType = 0, *pNode = m_pRunableCurrent;
            while (pNode) { // get root Type definition
                if (pNode->QueryType<Type>(TYPE_RUNABLE)) {
                    pRootType = pNode;
                    pNode = pNode->GetParent();
                }
                else {
                    break;
                }
            }
            if (AggregateType *pRootAggregateType = pRootType->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
                SYMBOL_DESC symDesc(symId
//#ifdef _DEBUG
                    ,m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#endif // _DEBUG
                    );
                if (pVariable = new Variable(m_pRunableCurrent, symDesc, 0xffffffff, INVALID_MEMORY_OFFSET, 0, static_cast<Type*>(m_pRunableCurrent))) {
                }
                else {
                    ERROR(OUT_OF_MEMORY);
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }
    return pVariable;
}

Function* Interpreter::GetBuiltInFunction(NODE_TYPE type, string &token) {
    Function *pFunction = 0;
    switch (type) {
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST: {
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            pFunction = m_pRunableRoot->ResolveFunction(symId);
            if (!pFunction) {
                if (Scope::s_reservedFooLower == SYMBOL_ID_MAX) {
                    Scope::s_reservedFooLower = symId;
                }
                else {
                    Scope::s_reservedFooUpper = symId;
                }
                pFunction = CreateBuiltInFunction(&m_symbolStore, type, symId);
            }
            break;
        }
        default:
            ERROR(UNDEFINED_SYMBOL);
            break;
    }
    return pFunction;
}

FunctionLibPtr* Interpreter::CreateFunctionLibPtr(Node* pParent, string &token) {
    FunctionLibPtr *pFunctionLibPtr = 0;
    SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
    SYMBOL_DESC symDesc(symId
//#ifdef _DEBUG
        ,m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#endif // _DEBUG
        );
    pFunctionLibPtr = new FunctionLibPtr(&m_symbolStore, pParent, symDesc);
    if (!pFunctionLibPtr) {
        ERROR(OUT_OF_MEMORY);
    }
    return pFunctionLibPtr;
}

bool Interpreter::ScopeEvent(NODE_TYPE type, string &token) {
    return BeginFullExpression(type, token);
}

Type *Interpreter::GetCallerType(ui32 &varIdx) {
    Type *pCallerType = 0;
    varIdx = 0;
    if (Variable *pVariable = m_pRunableCurrent->QueryType<Variable>(TYPE_TOKEN)) {
        assert(pVariable->GetTypeNode() != pVariable->GetParent());// { // this is variable decalration: T v() <- wrong!
        pCallerType = pVariable->GetTypeNode();
        varIdx = pVariable->GetVarIdx();
    }
    else if (FunctionCall *pFunctionCall = m_pRunableCurrent->QueryType<FunctionCall>(TYPE_FUNCTION_CALL)) {
        pCallerType = pFunctionCall->GetFunctionRefTypeNode()->GetReturnType();
    }
    else if (SubscriptNode *pSubscript = m_pRunableCurrent->QueryType<SubscriptNode>(TYPE_SUBSCRIPT)) {
        ArrayType *pArrayType = 0;
        switch (pSubscript->GetParent()->GetType()) {
            case TYPE_VAR:
                pArrayType = static_cast<ArrayType*>(static_cast<Variable*>(pSubscript->GetParent())->GetTypeNode());
                break;
            case TYPE_FUNCTION_CALL:
                pArrayType = static_cast<FunctionCall*>(pSubscript->GetParent())->GetFunctionRefTypeNode()->GetReturnType()->QueryType<ArrayType>(TYPE_ARRAY);
                assert(pArrayType);
                break;
            default:
                assert(0);
                break;
        }

        pCallerType = pArrayType->GetTypeNode();
        //pCallerType = static_cast<ArrayType*>(pSubscript->GetParent())->GetTypeNode();
    }
    return pCallerType;
}

bool Interpreter::OpenSubscriptEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    if ((type == TYPE_SUBSCRIPT) && (token == "[")) {
        ui32 idx;
        Type *pType = GetCallerType(idx);
        assert(pType->GetType() == TYPE_ARRAY);
        assert(!m_pRunableCurrent->GetRight());
        if (SubscriptNode *pSubscriptNode = CreateNode<SubscriptNode>(token)) {
            idx |= MEMORY_OFFSET_BIT_SUB_STACK;
            pSubscriptNode->SetVarIdx(idx);
            m_pRunableCurrent->SetRight(pSubscriptNode);
            m_pRunableCurrent = pSubscriptNode;
            if (Node *pLeft = m_pRunableCurrent->GetLeft()) {
                pSubscriptNode->SetLeft(pLeft);
                m_pRunableCurrent->SetLeft(0);
                while (pLeft) {
                    pLeft->SetParent(pSubscriptNode);
                    pLeft = pLeft->GetLeft();
                }
            }
            m_stateEventStack.push_back(&Interpreter::ArrayBeginSubscriptEvent);
        }
    }
    else {
        Node *pRootValueNode = m_pRunableCurrent->GetRootValueNode();
        m_pRunableCurrent = pRootValueNode;
        return DispatchEvent(type, token);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::BeginMemberEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();// remove BeginMemberEvent
    if (type == TYPE_MEMBER_ACCESS) {
        if (MemberAccess *pMemberAccess = CreateNode<MemberAccess>(type, token)) {
            m_pRunableCurrent->SetRight(pMemberAccess);
            m_pRunableCurrent = pMemberAccess;
            m_stateEventStack.push_back(&Interpreter::MemberAccessEvent);
        }
    }
    else {
        Node *pRootValueNode = m_pRunableCurrent->GetRootValueNode();
        m_pRunableCurrent = pRootValueNode;
        return DispatchEvent(type, token);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::TypeInitEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();

    switch (type) {
        case TYPE_ASSIGN:
        case TYPE_REF_ASSIGN:
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
        case TYPE_COMMA:
        case TYPE_SEMICOLON:
            return DispatchEvent(type, token);
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::ExpEvent(NODE_TYPE type, string &token) {

    NODE_TYPE curType = m_pRunableCurrent->GetType();
    Node *pDestNode = 0;
    switch (type) {
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN:
        case TYPE_NUM_EXP:
            return ExpNumberTokenEvent(type, token);
#else // NUMBER
        case TYPE_NONE:
            return UnknownTokenEvent(type, token);
#endif // NUMBER
        case TYPE_SEMICOLON:
            // handle it if it's an expression in the scope or in the for(;;) statement
            if (Node *pExpNode = m_pRunableCurrent->GetExpressionRoot()) {
                Type *pRetType = 0;//Node *pNode = 0;
                PrePostExpressions *pPrePostExp = 0;
                if (!pExpNode->IsExpression(pRetType, pPrePostExp, m_error)) {
                    if (pPrePostExp) {
                        delete pPrePostExp;
                    }
                    break;
                }
                m_pRunableCurrent = pExpNode->GetParent();
                if (!ApplyPrePostExpressions((pExpNode->GetType() == TYPE_RETURN) ? pExpNode->GetRight() : pExpNode, pPrePostExp)) {
                    break;
                }
            }
            m_stateEventStack.pop_back();
            return DispatchEvent(type, token);
        case TYPE_COMMA:
            // handle if it's an argument expression
            m_stateEventStack.pop_back();
            return DispatchEvent(type, token);
        case TYPE_PARENTHESIS:
            if (token[0] == '(') {
                return OpenParenthesis(token);
            }
            else { // ')'
                return CloseParenthesis(token);
            }
            break;
        case TYPE_SUBSCRIPT:
            if (token[0] == '[') {
                m_stateEventStack.push_back(&Interpreter::OpenSubscriptEvent);
                return DispatchEvent(type, token);
            }
            else {
                m_stateEventStack.pop_back();
                return DispatchEvent(type, token);
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
            ERROR(SYNTAX_ERROR);
            break;
        case TYPE_INC_OP:
        case TYPE_DEC_OP:
            if (curType == TYPE_VAR) { // this is post inc/dec operator
                if (Operator *pNewOperator = CreateNode<Operator>(type, token)) {
                    if (!m_pRunableCurrent->IsFunctionCall()) {
                        if (m_pRunableCurrent->GetRight()) {
                            if (SubscriptNode *pSubscriptNode = m_pRunableCurrent->GetRight()->QueryType<SubscriptNode>(TYPE_SUBSCRIPT)) {
                                pSubscriptNode->SetPostOperator(pNewOperator);
                                break;
                            }
                        }
                        static_cast<Variable*>(m_pRunableCurrent)->SetPostOperator(pNewOperator);
                    }
                    else {
                        delete pNewOperator;
                        ERROR(OPERATOR_NOT_ALLOWED_ON_FUNCTION_CALL);
                    }
                }
                break;
            }
            else if (curType == TYPE_PARENTHESIS) {
                if (m_pRunableCurrent->GetSymbolId() != SYMBOL_ID_OPENPARENTHESIS) {
                    ERROR(OPERATOR_NEEDS_VARIABLE);
                    break;
                }
            }
        default:
            if (Runable::IsValue(curType) || 
                (m_pRunableCurrent->GetSymbolId() == SYMBOL_ID_PARENTHESIS) || (curType == TYPE_LIB_VAR)) {
                if (Runable::IsOperator(type)) {
                    // add operator to the tree, check operator precedence
                    // parent types: scope, if, for, while, switch, fucntion_call, operator
                    //  v can be parenthesis node () or (
                    //         1)                 2) op < newop     3) newop >= op
                    //   v* => op* ,   op   =>   op(2)    or =>  newop(2)*
                    //         /      / \        / \                /
                    //        v      v0 v1*    v0 newop(1)*       op(1)
                    //                             /              / \
                    //                            v1             v0 v1
                    //if (IsAggregateType(m_pRunableCurrent) && !((type == TYPE_ASSIGN) || (type == TYPE_REF_ASSIGN))) {
                    //    ERROR(AGGREGATE_MATH_OPERATOR_NOT_SUPPORTED);
                    //    return false;
                    //}
                    if (curType == TYPE_LIB_VAR) {
                        if (!m_pRunableCurrent->GetRight()) {
                            ERROR(OPERATOR_NOT_ALLOWED_ON_TYPE);
                            return false;
                        }
                    }

                    Node *pRootValueNode = m_pRunableCurrent->GetRootValueNode();//->QueryType<Variable>(TYPE_RUNABLE);
                    Node *pAnchor = pRootValueNode->GetParent();
                    m_pRunableCurrent = pRootValueNode;

                    if (Operator *pNewOperator = CreateNode<Operator>(type, token)) {
                        switch (NODE_TYPE anchorType = pAnchor->GetType()) {
                            case TYPE_AGGREGATE_TYPE:
                                //ERROR(SYNTAX_ERROR);
                                //delete pNewOperator;
                                //break;
                            case TYPE_FUNCTION_REF_TYPE:
                                if ((type != TYPE_ASSIGN) && (type != TYPE_REF_ASSIGN)) {
                                    delete pNewOperator;
                                    ERROR(ILLEGAL_OPERATION_ON_TYPE);
                                    break;
                                }
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
                            case TYPE_STRING: {
                                Type *pType = static_cast<Type*>(pAnchor);
                                if (pType->UpdateExpression(pNewOperator)) {
                                    pNewOperator->SetParent(pAnchor);
                                    m_pRunableCurrent->SetParent(pNewOperator);
                                    pNewOperator->SetLeft(m_pRunableCurrent);
                                    m_pRunableCurrent = pNewOperator;
                                }
                                else {
                                    delete pNewOperator;
                                    ERROR(INTERNAL_ERROR);
                                }
                                break;
                            }
                            case TYPE_SCOPE: {
                                //  CreateNode ->  v  rotate -> op
                                //                /             /
                                //               op            v 
                                Scope *pScope = static_cast<Scope*>(pAnchor);
                                if (pScope->UpdateExpression(pNewOperator)) {
                                    pNewOperator->SetParent(pAnchor);
                                    m_pRunableCurrent->SetParent(pNewOperator);
                                    pNewOperator->SetLeft(m_pRunableCurrent);
                                    m_pRunableCurrent = pNewOperator;
                                }
                                else {
                                    delete pNewOperator;
                                    ERROR(INTERNAL_ERROR);
                                }
                                break;
                            }
                            case TYPE_FUNCTION_CALL: {
                                FunctionCall *pFunctionCall = static_cast<FunctionCall*>(pAnchor);
                                if (pFunctionCall->UpdateArgument(pNewOperator)) {
                                    pNewOperator->SetParent(pAnchor);
                                    m_pRunableCurrent->SetParent(pNewOperator);
                                    pNewOperator->SetLeft(m_pRunableCurrent);
                                    m_pRunableCurrent = pNewOperator;
                                }
                                else {
                                    delete pNewOperator;
                                    ERROR(INTERNAL_ERROR);
                                }
                                break;
                            }
                            case TYPE_IF:
                                pNewOperator->SetParent(pAnchor);
                                m_pRunableCurrent->SetParent(pNewOperator);
                                pNewOperator->SetLeft(m_pRunableCurrent);
                                static_cast<IfNode*>(pAnchor)->PushExpression(pNewOperator);
                                m_pRunableCurrent = pNewOperator;
                                break;
                            case TYPE_WHILE:
                                pNewOperator->SetParent(pAnchor);
                                m_pRunableCurrent->SetParent(pNewOperator);
                                pNewOperator->SetLeft(m_pRunableCurrent);
                                //pAnchor->SetLeft(pNewOperator);
                                static_cast<WhileNode*>(pAnchor)->PushExpression(pNewOperator);
                                m_pRunableCurrent = pNewOperator;
                                break;
                            case TYPE_SWITCH:
                                pNewOperator->SetParent(pAnchor);
                                m_pRunableCurrent->SetParent(pNewOperator);
                                pNewOperator->SetLeft(m_pRunableCurrent);
                                pAnchor->SetLeft(pNewOperator);
                                m_pRunableCurrent = pNewOperator;
                                break;
                            case TYPE_FOR:
                                pNewOperator->SetParent(pAnchor);
                                m_pRunableCurrent->SetParent(pNewOperator);
                                pNewOperator->SetLeft(m_pRunableCurrent);
                                static_cast<ForNode*>(pAnchor)->UpdateExpression(pNewOperator);
                                m_pRunableCurrent = pNewOperator;
                                break;
                            case TYPE_CASE:
                            case TYPE_DEFAULT:
                                pNewOperator->SetParent(pAnchor);
                                m_pRunableCurrent->SetParent(pNewOperator);
                                pNewOperator->SetLeft(m_pRunableCurrent);
                                pAnchor->SetLeft(pNewOperator);
                                m_pRunableCurrent = pNewOperator;
                                break;
                            case TYPE_PARENTHESIS:
                            case TYPE_RETURN:
                                // anchor can be open parenthesis node '('
                                //    (         (
                                //     \    =>   \
                                //     v0       newop 
                                //               / 
                                //              v0 
                                pNewOperator->SetParent(pAnchor);
                                pNewOperator->SetLeft(pAnchor->GetRight());
                                pAnchor->SetRight(pNewOperator);
                                pNewOperator->GetLeft()->SetParent(pNewOperator);
                                m_pRunableCurrent = pNewOperator;
                                break;
                            case TYPE_SUBSCRIPT: {
                                SubscriptNode *pSubscriptNode = static_cast<SubscriptNode*>(pAnchor);
                                if (pSubscriptNode->UpdateExpression(pNewOperator)) {
                                    pNewOperator->SetParent(pAnchor);
                                    m_pRunableCurrent->SetParent(pNewOperator);
                                    pNewOperator->SetLeft(m_pRunableCurrent);
                                    m_pRunableCurrent = pNewOperator;
                                }
                                else {
                                    delete pNewOperator;
                                    ERROR(INTERNAL_ERROR);
                                }
                                break;
                            }
                            case TYPE_ARRAY: {
                                if (m_stateEventStack.back() == &Interpreter::ArrayDimExpEvent) {
                                    ArrayType *pArrayType = static_cast<ArrayType*>(pAnchor);
                                    pArrayType->SetDynExpression(pNewOperator);
                                    pNewOperator->SetParent(pAnchor);
                                    m_pRunableCurrent->SetParent(pNewOperator);
                                    pNewOperator->SetLeft(m_pRunableCurrent);
                                    m_pRunableCurrent = pNewOperator;
                                }
                                else {
                                    Type *pType = static_cast<Type*>(pAnchor);
                                    if (pType->UpdateExpression(pNewOperator)) {
                                        pNewOperator->SetParent(pAnchor);
                                        m_pRunableCurrent->SetParent(pNewOperator);
                                        pNewOperator->SetLeft(m_pRunableCurrent);
                                        m_pRunableCurrent = pNewOperator;
                                    }
                                    else {
                                        delete pNewOperator;
                                        ERROR(INTERNAL_ERROR);
                                    }
                                }
                                break;
                            }
                            default:
                                if (Runable::IsOperator(anchorType)) {
                                    PRECEDENCE anchorPrecedence = pAnchor->GetPrecedence(),
                                        newOperatorPrecedence = pNewOperator->GetPrecedence();
                                    if (newOperatorPrecedence < anchorPrecedence) {
                                        // case 2)
                                        // 
                                        // anchor      anchor(2)           
                                        //  / \   =>   /  \          
                                        // v0 v1      v0 newop(1)*
                                        //                /
                                        //               v1
                                        // insert below anchor
ASSOCIATIVITY_RIGHT_TO_LEFT:
                                        pNewOperator->SetParent(pAnchor);
                                        pNewOperator->SetLeft(pAnchor->GetRight());
                                        pAnchor->SetRight(pNewOperator);
                                        pNewOperator->GetLeft()->SetParent(pNewOperator);
                                        m_pRunableCurrent = pNewOperator;
                                    }
                                    else if (newOperatorPrecedence == anchorPrecedence) {
                                        Runable::ASSOCIATIVITY a = pNewOperator->GetAssociativity();
                                        // a) RIGHT_TO_LEFT
                                        // anchor      anchor(2)           
                                        //  / \   =>   /  \          
                                        // v0 v1      v0 newop(1)*
                                        //                /
                                        //               v1

                                        // b) LEFT_TO_RIGHT
                                        //  anchor      newop(2)*
                                        //   / \   =>    /
                                        //  v0 v1     anchor(1) 
                                        //             / \
                                        //            v0 v1
                                        if (a == Runable::RIGHT_TO_LEFT) {
                                            goto ASSOCIATIVITY_RIGHT_TO_LEFT;
                                        }
                                        else if (a == Runable::LEFT_TO_RIGHT) {
                                            goto ASSOCIATIVITY_LEFT_TO_RIGHT;
                                        }
                                        else {
                                            ERROR(INTERNAL_ERROR);
                                        }
                                    }
                                    else {
                                        // case 3)
                                        //
                                        //  anchor      newop(2)*
                                        //   / \   =>    /
                                        //  v0 v1     anchor(1) 
                                        //             / \
                                        //            v0 v1
                                        // find new anchor where newOperatorPrecedence >= anchorPrecedence
                                        // insert above anchor
                                        // Node* pAnchorParent = pAnchor->GetParent();
ASSOCIATIVITY_LEFT_TO_RIGHT:
                                        NODE_TYPE anchorType;
                                        while (pAnchor) {
                                            anchorType = pAnchor->GetType();
                                            anchorPrecedence = pAnchor->GetPrecedence();
                                            if (Runable::IsOperator(anchorType)) {
                                                if (newOperatorPrecedence >= anchorPrecedence) {
                                                    if (Runable::IsOperator(pAnchor->GetParent()->GetType())) {
                                                        pAnchor = pAnchor->GetParent();
                                                        continue;
                                                    }
                                                    // else done
                                                }
                                                else {
                                                    // done
                                                    pAnchor = pAnchor->GetRight();
                                                }
                                            }
                                            else if (TYPE_PARENTHESIS == anchorType) {
                                                // done
                                                pAnchor = pAnchor->GetLeft();
                                            }
                                            else {
                                                // done
                                                pAnchor = pAnchor->GetLeft();
                                            }
                                            break;
                                        }
                                        if (pAnchor) {
                                            pNewOperator->SetParent(pAnchor->GetParent());//1
                                            pNewOperator->SetLeft(pAnchor);//2

                                            switch (pNewOperator->GetParent()->GetType()) { // 3
                                                case TYPE_SWITCH:
                                                    ERROR(INTERNAL_ERROR);
                                                    break;
                                                case TYPE_IF:
                                                    static_cast<IfNode*>(pNewOperator->GetParent())->PushExpression(pNewOperator);
                                                    break;
                                                case TYPE_FOR:
                                                    static_cast<ForNode*>(pNewOperator->GetParent())->UpdateExpression(pNewOperator);
                                                    break;
                                                //case TYPE_PARENTHESIS:
                                                case TYPE_WHILE:
                                                case TYPE_CASE:
                                                case TYPE_DEFAULT:
                                                    pNewOperator->GetParent()->SetLeft(pNewOperator);
                                                    break;
                                                case TYPE_FUNCTION_CALL: {
                                                    FunctionCall *pFunctionCall = static_cast<FunctionCall*>(pNewOperator->GetParent());
                                                    pFunctionCall->UpdateArgument(pNewOperator);
                                                    break;
                                                }
                                                case TYPE_SUBSCRIPT: {
                                                    SubscriptNode *pSubscriptNode = static_cast<SubscriptNode*>(pNewOperator->GetParent());
                                                    pSubscriptNode->UpdateExpression(pNewOperator);
                                                    break;
                                                }
                                                default:
                                                    pNewOperator->GetParent()->SetRight(pNewOperator);//3
                                                    break;
                                            }
                                            pAnchor->SetParent(pNewOperator);//4
                                            m_pRunableCurrent = pNewOperator;
                                        }
                                        else {
                                            delete pNewOperator;
                                            ERROR(INTERNAL_ERROR);
                                        }

                                    }
                                }
                                //else if ((TYPE_PARENTHESIS == anchorType) || (TYPE_RETURN == anchorType)) {
                                //    // anchor can be open parenthesis node '('
                                //    //    (         (
                                //    //     \    =>   \
                                //    //     v0       newop 
                                //    //               / 
                                //    //              v0 
                                //    pNewOperator->SetParent(pAnchor);
                                //    pNewOperator->SetLeft(pAnchor->GetRight());
                                //    pAnchor->SetRight(pNewOperator);
                                //    pNewOperator->GetLeft()->SetParent(pNewOperator);
                                //    m_pRunableCurrent = pNewOperator;
                                //}
                                else {
                                    delete pNewOperator;
                                    (anchorType == TYPE_ARRAY) ? ERROR(SYNTAX_ERROR) :ERROR(INTERNAL_ERROR);
                                }
                                break;
                        }
                    }
                }
                else if (TYPE_MEMBER_ACCESS == type) {
                    // member access
                    if (MemberAccess *pMemberAccess = CreateNode<MemberAccess>(type, token)) {
                        if (Variable *pVariable = m_pRunableCurrent->QueryType<Variable>(TYPE_VAR)) {
                            switch (pVariable->GetTypeNode()->GetType()) {
                                case TYPE_ARRAY:
                                    if (pVariable->GetRight()) {
                                        pVariable->GetRight()->SetRight(pMemberAccess);
                                        pMemberAccess->SetParent(pVariable->GetRight());
                                    }
                                    else {
                                        delete pMemberAccess;
                                        ERROR(SYNTAX_ERROR);
                                    }
                                    break;
                                case TYPE_AGGREGATE_TYPE:
                                case TYPE_LIB:
                                case TYPE_ERROR_TYPE:
                                    m_pRunableCurrent->SetRight(pMemberAccess);
                                    break;
                                default:
                                    delete pMemberAccess;
                                    ERROR(SYNTAX_ERROR);
                                    break;
                            }
                            m_pRunableCurrent = pMemberAccess;
                            m_stateEventStack.push_back(&Interpreter::MemberAccessEvent);
                            //if (pVariable->GetTypeNode()->GetType() == TYPE_ARRAY) {
                            //    if (pVariable->GetRight()) {
                            //        pVariable->GetRight()->SetRight(pMemberAccess);
                            //        pMemberAccess->SetParent(pVariable->GetRight());
                            //    }
                            //    else {
                            //        delete pMemberAccess;
                            //        ERROR(SYNTAX_ERROR);
                            //    }
                            //}
                            //else {
                            //    m_pRunableCurrent->SetRight(pMemberAccess);
                            //}
                            //m_pRunableCurrent = pMemberAccess;
                        }
                        else {
                            delete pMemberAccess;
                            ERROR(INTERNAL_ERROR);
                        }
                    }
                    //m_stateEventStack.push_back(&Interpreter::MemberAccessEvent);
                }
                else {
                    if (m_error == NO_ERROR) {
                        ERROR(SYNTAX_ERROR);
                    }
                }
            }
            else if (Runable::IsOperator(curType) || (m_pRunableCurrent->GetSymbolId() == SYMBOL_ID_OPENPARENTHESIS)) {
                // allowed operators as begining of the expression: +,-,++,--, function_call()
                // add value node to the tree, check operator precedence
                //  op*   =>  op*           
                //  /         /\      
                // v0        v0 v1    
                // handle ~, +,-,++,-- and function_call()
                return Append2Op(type, token);
            }
            else if (m_pRunableCurrent->GetSymbolId() == SYMBOL_ID_PARENTHESIS) {
                ERROR(INTERNAL_ERROR);
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
    }

    return (m_error == NO_ERROR);
}

/******************** BEGIN expression helpers ********************/
bool Interpreter::AppendValueNode2Op(Node* pNode) {
    // allowed operators as begining of the expression: +,-,++,--, function_call()
    // add value node to the tree, check operator precedence
    //     1)           2)
    //   op* => op* ,  op*   =>  op*           
    //          /      /         /\      
    //         v      v0        v0 v1    

    //handle case 1)
    bool good = (pNode != 0);
    if (good) {
        NODE_TYPE curType = m_pRunableCurrent->GetType();
        switch (curType) {
            case TYPE_PRE_INC_OP:
            case TYPE_PRE_DEC_OP:
                if (Variable *pVariable = pNode->QueryType<Variable>(TYPE_VAR)) {
                    if (pVariable->IsConst()/* || (pVariable->GetType() == TYPE_LIB_VAR)*/) {
                        ERROR(OPERATOR_NEEDS_VARIABLE);
                        delete pNode;
                        return false;
                    }
                    Node *pParent = m_pRunableCurrent;
                    Node *pRootPrePostOp = 0;
                    do {
                        pRootPrePostOp = pParent;
                        pParent = pParent->GetParent();
                    } while ((pParent->GetType() == TYPE_PRE_INC_OP) || (pParent->GetType() == TYPE_PRE_DEC_OP));
                    
                    switch (pParent->GetType()) {
                        case TYPE_RETURN:
                        case TYPE_PARENTHESIS:
                            pVariable->SetPreOperator(pRootPrePostOp, pParent);
                            pParent->SetRight(pVariable);
                            break;
                        case TYPE_IF: {
                            IfNode *pIfNode = static_cast<IfNode*>(pParent);
                            pIfNode->UpdateExpression(pVariable);
                            pVariable->SetPreOperator(pRootPrePostOp, pParent);
                            break;
                        }
                        case TYPE_FOR: {
                            ForNode *pForNode = static_cast<ForNode*>(pParent);
                            pForNode->UpdateExpression(pVariable);
                            pVariable->SetPreOperator(pRootPrePostOp, pParent);
                            break;
                        }              
                        case TYPE_WHILE: {
                            WhileNode *pWhileNode = static_cast<WhileNode*>(pParent);
                            pWhileNode->UpdateExpression(pVariable);
                            pVariable->SetPreOperator(pRootPrePostOp, pParent);
                            break;
                        }
                        case TYPE_SWITCH:
                            assert(0);
                            break;
                        case TYPE_SCOPE: {
                            Scope *pScope = static_cast<Scope*>(pParent);
                            pScope->UpdateExpression(pVariable);
                            pVariable->SetPreOperator(pRootPrePostOp, pParent);
                            break;
                        }
                        case TYPE_FUNCTION_CALL: {
                            FunctionCall *pFunctionCall = static_cast<FunctionCall*>(pParent);
                            Node *pArgNode = pFunctionCall->GetArgNode(pFunctionCall->GetArgCount() - 1);
                            pFunctionCall->UpdateArgument(pVariable);
                            pVariable->SetPreOperator(pRootPrePostOp, pParent);
                            break;
                        }
                        case TYPE_SUBSCRIPT: {
                            SubscriptNode *pSubscriptNode = static_cast<SubscriptNode*>(pParent);
                            pSubscriptNode->UpdateExpression(pVariable);
                            pVariable->SetPreOperator(pRootPrePostOp, pParent);
                            break;
                        }
                        case TYPE_ARRAY: {
                            ArrayType *pArrayType = static_cast<ArrayType*>(pParent);
                            if (m_stateEventStack.back() == &Interpreter::ArrayDimExpEvent) {
                                pArrayType->SetDynExpression(pVariable);
                                pVariable->SetPreOperator(pRootPrePostOp, pParent);
                                break;
                            }
                            else {
                                pArrayType->UpdateExpression(pVariable);
                                pVariable->SetPreOperator(pRootPrePostOp, pParent);
                            }
                            break;
                        }
                        default: {
                            if (Operator *pOperator = pParent->QueryType<Operator>(TYPE_RUNABLE)) {
                                pVariable->SetPreOperator(pRootPrePostOp, pParent);
                                pOperator->SetRight(pVariable);
                            }
                            else if (Type *pType = pParent->QueryType<Type>(TYPE_RUNABLE)) {
                                assert(0);
                            }
                            else if (FunctionCall *pFunctionCall  = m_pRunableCurrent->QueryType<FunctionCall>(TYPE_FUNCTION_CALL)) {
                                assert(0);
                            }
                            else {
                                assert(0);
                            }
                            break;
                        }
                    }
                    m_pRunableCurrent = pVariable;
                }
                else {
                    if ((pNode->GetType() == TYPE_PRE_INC_OP) || (pNode->GetType() == TYPE_PRE_DEC_OP)) {
                        m_pRunableCurrent->SetLeft(pNode);
                        m_pRunableCurrent = pNode;
                    }
                    else {
                        ERROR(INTERNAL_ERROR);
                        delete pNode;
                        good = false;
                    }
                }
                break;
            default:
                if (Runable::IsOperator(curType) || 
                    (m_pRunableCurrent->GetSymbolId() == SYMBOL_ID_OPENPARENTHESIS) ||
                    (curType == TYPE_RETURN)) {
                    m_pRunableCurrent->SetRight(pNode);
                    m_pRunableCurrent = pNode;
                }
                else {
                    ERROR(SYNTAX_ERROR);
                    good = false;
                }
                break;
        }
    }
    return good;
}

bool Interpreter::AppendPreOpNode2Op(Node* pNode) {
    bool good = (pNode != 0);
    if (good) {
        m_pRunableCurrent->SetRight(pNode);
        m_pRunableCurrent = pNode;
    }

    return good;
}

bool Interpreter::Append2Op(NODE_TYPE type, string &token) {
    bool processNumber = false;
    NODE_TYPE _type = type;
    switch (type) {
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST:
            _type = TYPE_TOKEN;
        case TYPE_CONST_BOOL:
        case TYPE_CONST:
        case TYPE_TOKEN:
        case TYPE_INC_OP:
        case TYPE_DEC_OP:
        case TYPE_BIT_NOT:
        case TYPE_NOT:
            AppendValueNode2Op(CreateNode<Node>(_type, token));
            if (type == TYPE_CAST) {
                m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
            }
            break;
        case TYPE_ADD:
        case TYPE_SUB:
            if (AppendValueNode2Op(CreateNode<Node>(TYPE_CONST, s_empty))) {
                m_stateEventStack.push_back(&Interpreter::NumberSignEvent);
                return DispatchEvent(type, token);
            }
            break;
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN: {
            //string value;
            //if (!GetFixedNumber(type, token, value)) {
            //    return false;
            //}
            //token = value;
            //type = TYPE_CONST_NUM;
            processNumber = true;
        }
        case TYPE_NUM_EXP:
#else // NUMBER
        case TYPE_NONE: {
            ui64 value64 = 0;
            if (!UnknownToken2Number(token, value64)) {
                return false;
            }
            token = to_string(value64);
            type = TYPE_CONST_NUM;
            processNumber = true;
            //fall through
        }
#endif // NUMBER
        case TYPE_DOT:
        case TYPE_CONST_NUM:
            if (AppendValueNode2Op(CreateNode<Node>(TYPE_CONST, s_empty))) {
                m_stateEventStack.push_back(&Interpreter::NumberEvent);
                if (DispatchEvent(type, token)) {
                    return processNumber ? ProcessNumber() : true;
                }
                return false;
            }
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AppendValueNode2Arg(Node* pNode) {
    if (pNode) {
        if (FunctionCall *pFunctionCall = m_pRunableCurrent->QueryType<FunctionCall>(TYPE_FUNCTION_CALL)) {
            pFunctionCall->PushArgument(pNode);
            m_pRunableCurrent = pNode;
            return true;
        }
        ERROR(SYNTAX_ERROR);
        delete pNode;
    }

    return false;
}

bool Interpreter::AppendValueNode2Subscript(Node* pNode) {
    if (pNode) {
        if (SubscriptNode *pSubscriptNode = m_pRunableCurrent->QueryType<SubscriptNode>(TYPE_SUBSCRIPT)) {
            pSubscriptNode->SetExpression(pNode);
            m_pRunableCurrent = pNode;
            return true;
        }
        ERROR(SYNTAX_ERROR);
        delete pNode;
    }
    return false;
}

/******************** END expression helpers ********************/

bool Interpreter::FunctionBeginRefDecEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_TOKEN) {
        // search for function definition
        SYMBOL_ID symId;
        if (m_symbolStore.GetSymbolId(token, symId)) {
            if (Function *pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                if (FunctionRefType *pFunctionRefType = CreateNode<FunctionRefType>(TYPE_FUNCTION_REF_TYPE, token)) {
                    pFunctionRefType->PopulateTypes(pFunction);
                    m_pRunableCurrent = pFunctionRefType;
                    m_stateEventStack.pop_back();
                    m_stateEventStack.push_back(&Interpreter::FunctionEndRefDecEvent);
                    return true;
                }
                else {
                    return false;
                }
            }
        }
    }
    ERROR(EXPECTING_FUNCTION_REF_TYPE);

    return false;
}

bool Interpreter::FunctionEndRefDecEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_GREATER) {
        m_stateEventStack.pop_back();
        m_stateEventStack.push_back(&Interpreter::TypeDecEvent);
        if (Scope *pScope = m_pRunableCurrent->GetParent<Scope>(TYPE_SCOPE)) {
            pScope->PushExpression(m_pRunableCurrent);
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionTypeEventHandler(NODE_TYPE type, string &token, INTERPRETER_STATE_EVENT fnEvent) {
    m_stateEventStack.pop_back(); // remove FunctionTypeEvent
    switch (type) {
        case TYPE_LESS:// this is function type: function<xxx>
            m_stateEventStack.push_back(&Interpreter::FunctionBeginRefDecEvent);
            break;
        case TYPE_FUNCTION:
            if (!m_pRunableCurrent->GetParent()) {
                m_stateEventStack.push_back(&Interpreter::FunctionFunctionReturnTypeEvent);
            }
            else if (m_pRunableCurrent->GetType() == TYPE_FUNCTION) {
                m_stateEventStack.push_back(&Interpreter::FunctionFunctionArgTypeEvent);
            }
            else {
                ERROR(SYNTAX_ERROR);
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
            if (Type* pType = CreateNode<Type>(type, token)) {
                m_pRunableCurrent = pType;
                m_stateEventStack.push_back(fnEvent);
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
            ERROR(EXPECTING_TYPE_NAME);
            break;
        case TYPE_TOKEN: {
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            if (AggregateType *pResolvedType = FindTypeDefinition(symId)) {
                if (AggregateType* pAggregateType = CreateNode<AggregateType>(TYPE_AGGREGATE_TYPE, token)) {
                    pAggregateType->SetTypeDefinition(pResolvedType);
                    m_pRunableCurrent = pAggregateType;
                    m_stateEventStack.push_back(fnEvent);
                }
            }
            else {
                ERROR(UNDEFINED_TYPE);
            }
            break;
        }
        case TYPE_ARRAY:
            if (ArrayType* pArrayType = CreateNode<ArrayType>(type, token)) {
                m_pRunableCurrent = pArrayType;
                m_stateEventStack.push_back(fnEvent);
                m_stateEventStack.push_back(&Interpreter::ArrayBeginEvent);
            }
            break;
        default:
            m_stateEventStack.push_back(fnEvent);
            return DispatchEvent(type, token);
    }
    return (m_error == NO_ERROR);
}
// return
bool Interpreter::FunctionFunctionReturnTypeEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_LESS) {
        m_stateEventStack.pop_back();
        m_stateEventStack.push_back(&Interpreter::FunctionBeginFunctionRefReturnEvent);
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionBeginFunctionRefReturnEvent(NODE_TYPE type, string &token) {
    if (FunctionBeginRefDecEvent(type, token)) {
        //replace event
        m_stateEventStack.pop_back();
        m_stateEventStack.push_back(&Interpreter::FunctionEndFunctionRefReturnEvent);
        return true;
    }
    return false;
}

bool Interpreter::FunctionEndFunctionRefReturnEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_GREATER) {
        m_stateEventStack.pop_back();
        m_stateEventStack.push_back(&Interpreter::FunctionNameEvent);
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}
// arg
bool Interpreter::FunctionFunctionArgTypeEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_LESS) {
        m_stateEventStack.pop_back();
        m_stateEventStack.push_back(&Interpreter::FunctionBeginFunctionRefArgEvent);
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionBeginFunctionRefArgEvent(NODE_TYPE type, string &token) {
    if (FunctionBeginRefDecEvent(type, token)) {
        //replace event
        m_stateEventStack.pop_back();
        m_stateEventStack.push_back(&Interpreter::FunctionEndFunctionRefArgEvent);
        return true;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionEndFunctionRefArgEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_GREATER) {
        m_stateEventStack.pop_back();
        m_stateEventStack.push_back(&Interpreter::FunctionArgEvent);
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}
//
bool Interpreter::FunctionTypeEvent(NODE_TYPE type, string &token) {
    return FunctionTypeEventHandler(type, token, &Interpreter::FunctionNameEvent);
}

bool Interpreter::FunctionArgTypeEvent(NODE_TYPE type, string &token) {
    return FunctionTypeEventHandler(type, token, &Interpreter::FunctionArgEvent);
}


bool Interpreter::FunctionNameEvent(NODE_TYPE type, string &token) {
    // attach type node to the function node
    switch (type) {
        case TYPE_REF:
            if (m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) {
                if (Reference *pReference = CreateNode<Reference>(type, token)) {
                    if (!m_pRunableCurrent->GetLeft()) {
                        m_pRunableCurrent->SetLeft(pReference);
                    }
                    else if (!m_pRunableCurrent->GetRight()) {
                        m_pRunableCurrent->SetRight(pReference);
                    }
                    else {
                        delete pReference;
                        ERROR(SYNTAX_ERROR);
                    }
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
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
            ERROR(SYNTAX_ERROR);
            break;
        default:
            if ((m_pRunableCurrent->GetType() == TYPE_ARRAY) && static_cast<ArrayType*>(m_pRunableCurrent)->GetDynExpression()) {
                ERROR(DYNAMIC_ARRAY_MUST_BE_DECLARED_IN_SCOPE);
                break;
            }
            if (Type *pType = m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) {
                if (ErrorIfDynamicArray(pType)) {
                    return false;
                }
                if (Scope *pScope = pType->GetParent()->QueryType<Scope>(TYPE_SCOPE)) {
                    if (pScope->GetParent()) {
                        ERROR(FUNCTION_DEFINITION_MUST_BE_GLOBAL);
                    }
                    else if (Function *pFunction = CreateNode<Function>(token)) {
                        if (pScope->PushFunctionDef(pFunction)) {
                            pFunction->SetParent(pType->GetParent());
                            pType->SetParent(pFunction);
                            pFunction->GetFunctionRefTypeNode()->SetReturnType(pType);
                            //pFunction->SetReturnType(pType);
                            m_pRunableCurrent = pFunction;
                            m_stateEventStack.push_back(&Interpreter::FunctionDecEvent);
                        }
                        else {
                            delete pFunction;
                            ERROR(DUPLICATE_FUNCTION_DEFINITION);
                        }
                    }
                    else {
                        ERROR(OUT_OF_MEMORY);
                    }
                }
                else {
                    ERROR(UNEXPECTED_FUNCTION_DEFINITION);
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;

    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionDecEvent(NODE_TYPE type, string &token) {
    if ((type == TYPE_PARENTHESIS) && (token[0] == '(')) {
        return OpenParenthesis(token);
    }
    else {
        ERROR(EXPECTING_PARENTHESIS);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionArgEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_REF:
            if (Type *pArgType = m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) {
                if (Reference *pReference = CreateNode<Reference>(type, token)) {
                    if (!pArgType->GetLeft()) {
                        pArgType->SetLeft(pReference);
                    }
                    else if (!pArgType->GetRight()) {
                        pArgType->SetRight(pReference);
                    }
                    else {
                        delete pReference;
                        ERROR(SYNTAX_ERROR);
                    }
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
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
            ERROR(EXPECTING_VARIABLE_NAME);
            break;
        case TYPE_TOKEN:
            if (Type *pArgType = m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) {
                if (ErrorIfDynamicArray(pArgType)) {
                    return false;
                }
                if (Function *pFunction = m_pRunableCurrent->GetParent()->QueryType<Function>(TYPE_FUNCTION)) {
                    SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
                    SYMBOL_DESC symDesc(symId,
//#ifdef _DEBUG
                            m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#endif // _DEBUG
                            );
                    if (pFunction->PushArgInfo(symDesc, pArgType)) {
                        m_pRunableCurrent->SetParent(pFunction);
                        m_pRunableCurrent = pFunction;
                        m_stateEventStack.pop_back(); // remove FunctionArgEvent
                        m_stateEventStack.push_back(&Interpreter::FunctionNextArgEvent);
                    }
                    else {
                        ERROR(DUPLICATE_ARGUMENT_NAME);
                    }
                }
                else {
                    ERROR(INTERNAL_ERROR);
                }
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        case TYPE_PARENTHESIS:
            if (token[0] == ')') {
                if (m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) {
                    ERROR(SYNTAX_ERROR);
                }
                else {
                    return CloseParenthesis(token);
                }
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        default:
            ERROR(UNEXPECTED_CHARACTER);
            break;
    }

    return (m_error == NO_ERROR);
}
bool Interpreter::FunctionNextArgEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_COMMA: {
            if (Function *pFunction = m_pRunableCurrent->QueryType<Function>(TYPE_FUNCTION)) {
                if (pFunction->GetArgCount()) {
                    m_stateEventStack.pop_back(); // remove FunctionNextArgEvent
                    m_stateEventStack.push_back(&Interpreter::FunctionArgTypeEvent);
                }
                else {
                    ERROR(SYNTAX_ERROR);
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
        }
        case TYPE_PARENTHESIS:
            if (token[0] == ')') {
                return CloseParenthesis(token);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionBodyEvent(NODE_TYPE type, string &token) {

    if (TYPE_SCOPE == type) {
        if (token[0] == '{') {
            if (Function *pFunction = m_pRunableCurrent->QueryType<Function>(TYPE_FUNCTION)) {
                assert(!pFunction->GetLeft());
                if (Scope *pScope = CreateNode<Scope>(token)) {
#ifdef SCRIPT_DEBUGGER
                    pScope->SetDebuggerHandler(&m_debuggerHandler);
                    pScope->SetBreakPointLineByFileId(&m_breakPointLines);
#endif // SCRIPT_DEBUGGER
                    pFunction->SetLeft(pScope);
                    m_pRunableCurrent = pScope;
                    ResolveArguments(pFunction);
                    m_stateEventStack.pop_back();//remove FunctionBodyEvent
                    m_stateEventStack.push_back(&Interpreter::ScopeEvent);
                }
                else {
                    ERROR(OUT_OF_MEMORY);
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
        }
        else {
            ERROR(UNEXPECTED_CHARACTER);
        }
    }
    else {
        ERROR(UNEXPECTED_CHARACTER);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionCallEvent(NODE_TYPE type, string &token) {
    if ((type == TYPE_PARENTHESIS) && (token[0] == '(')) {
        if (m_pRunableCurrent->GetRight()) {
            ERROR(SYNTAX_ERROR);
            return false;
        }
        ui32 varIdx = 0;
        Type *pCallerType = GetCallerType(varIdx);

        assert(pCallerType);

        if (pCallerType->GetType() == TYPE_FUNCTION_REF_TYPE) {
            if (FunctionCall *pFunctionCall = CreateNode<FunctionCall>(pCallerType->GetSymbolId())) {
                m_pRunableCurrent->SetRight(pFunctionCall);
                m_pRunableCurrent = pFunctionCall;
                m_stateEventStack.push_back(&Interpreter::FunctionCallArgEvent);
            }
            else {
                ERROR(OUT_OF_MEMORY);
            }
        }
        else {
            ERROR(SYNTAX_ERROR);
        }
    }
    else {
        Node *pRootValueNode = m_pRunableCurrent->GetRootValueNode();
        m_pRunableCurrent = pRootValueNode;
        m_stateEventStack.pop_back();
        return DispatchEvent(type, token);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionCallArgEvent(NODE_TYPE type, string &token) {
    INTERPRETER_STATE_EVENT fnEvent = 0;
    bool processNumber = false;
    NODE_TYPE _type = type;
    switch (type) {
        case TYPE_COMMA: {
                Node *pExpNode = m_pRunableCurrent->GetExpressionRoot();
                if (!pExpNode) {
                    ERROR(INTERNAL_ERROR);
                    return false;
                }
                FunctionCall *pFunctionCall = pExpNode->GetParent()->QueryType<FunctionCall>(TYPE_RUNABLE);
                if (!pFunctionCall) {
                    ERROR(INTERNAL_ERROR);
                    return false;
                }
                m_pRunableCurrent = pFunctionCall;

                m_stateEventStack.pop_back(); // remove FunctionCallArgEvent
                m_stateEventStack.push_back(&Interpreter::FunctionCallNextArgEvent);
                return true;
            }
        case TYPE_PARENTHESIS: {
            FunctionCall *pFunctionCall = m_pRunableCurrent->GetParent()->QueryType<FunctionCall>(TYPE_FUNCTION_CALL);
            if (!pFunctionCall) {
                pFunctionCall = m_pRunableCurrent->QueryType<FunctionCall>(TYPE_FUNCTION_CALL);
            }
            if (pFunctionCall) {
                if (FunctionRefType *pFunctionRefType = pFunctionCall->GetFunctionRefTypeNode()) {
                    m_stateEventStack.push_back(&Interpreter::ExpEvent);
                    return DispatchEvent(type, token);
                }
                else {
                    ERROR(INTERNAL_ERROR);
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
        }
/****************************/
        case TYPE_SIZEOF:
            if (IsSizeOf(m_pRunableCurrent)) {
                ERROR(EXPECTING_VARIABLE_NAME);
                break;
            }
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST:
            _type = TYPE_TOKEN;
        case TYPE_CONST_BOOL:
        case TYPE_CONST:
        case TYPE_TOKEN:
        case TYPE_INC_OP:
        case TYPE_DEC_OP:
        case TYPE_BIT_NOT:
            if (AppendValueNode2Arg(CreateNode<Node>(_type, token))) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                if (type == TYPE_CAST) {
                    m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                }
            }
            break;
        case TYPE_ADD:
        case TYPE_SUB:
            fnEvent = &Interpreter::NumberSignEvent;
            break;
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN: {
            //string value;
            //if (!GetFixedNumber(type, token, value)) {
            //    return false;
            //}
            //token = value;
            //type = TYPE_CONST_NUM;
            processNumber = true;
        }
        case TYPE_NUM_EXP:
#else // NUMBER
        case TYPE_NONE: {
            ui64 value64 = 0;
            if (!UnknownToken2Number(token, value64)) {
                return false;
            }
            token = to_string(value64);
            type = TYPE_CONST_NUM;
            processNumber = true;
            //fall through
        }
#endif // NUMBER
        case TYPE_DOT:
        case TYPE_CONST_NUM:
            fnEvent = &Interpreter::NumberEvent;
            break;
/****************************/
        default:
            if (IsSizeOf(m_pRunableCurrent)) {
                if (type == TYPE_TOKEN) { // try to resolve it 
                    SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
                    Type *pResolvedType = 0;
                    ui32 idx = 0;
                    ResolveAlignedVariable(symId, pResolvedType, idx);
                    if (!pResolvedType) { // this coud be a function name, search for function definitions
                        if (!m_pRunableRoot->ResolveFunction(symId)) {
                            ERROR(UNDEFINED_SYMBOL);
                            break;
                        }
                    }

                    if (AppendValueNode2Arg(CreateNode<Node>(type, token))) {
                        m_stateEventStack.push_back(&Interpreter::ExpEvent);
                    }
                    break;
                }
                else {
                    ERROR(EXPECTING_VARIABLE_NAME);
                    break;
                }
            }
            ERROR(SYNTAX_ERROR);
            break;
    }

    if (fnEvent && AppendValueNode2Arg(CreateNode<Node>(TYPE_CONST, s_empty))) {
        m_stateEventStack.push_back(&Interpreter::ExpEvent);
        m_stateEventStack.push_back(fnEvent);
        if (DispatchEvent(type, token)) {
            return processNumber ? ProcessNumber() : true;
        }
        return false;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FunctionCallNextArgEvent(NODE_TYPE type, string &token) {
    INTERPRETER_STATE_EVENT fnEvent = 0;
    bool processNumber = false;
    NODE_TYPE _type = type;
    m_stateEventStack.pop_back(); // remove FunctionCallNextArgEvent
    switch (type) {
        case TYPE_PARENTHESIS:
            if (token[0] == '(') {
                m_stateEventStack.push_back(&Interpreter::FunctionCallArgEvent);
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                return DispatchEvent(type, token);
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
/****************************/
        case TYPE_SIZEOF:
            if (IsSizeOf(m_pRunableCurrent)) {
                ERROR(UNEXPECTED_NUMBER_OF_ARGUMENTS);
                break;
            }
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST:
            _type = TYPE_TOKEN;
        case TYPE_CONST_BOOL:
        case TYPE_CONST:
        case TYPE_TOKEN:
        case TYPE_INC_OP:
        case TYPE_DEC_OP:
        case TYPE_BIT_NOT:
            if (AppendValueNode2Arg(CreateNode<Node>(_type, token))) {
                m_stateEventStack.push_back(&Interpreter::FunctionCallArgEvent);
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                if (type == TYPE_CAST) {
                    m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                }
            }
            break;
        case TYPE_ADD:
        case TYPE_SUB:
            fnEvent = &Interpreter::NumberSignEvent;
            break;
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN: {
            //string value;
            //if (!GetFixedNumber(type, token, value)) {
            //    return false;
            //}
            //token = value;
            //type = TYPE_CONST_NUM;
            processNumber = true;
        }
        case TYPE_NUM_EXP:
#else // NUMBER
        case TYPE_NONE: {
            ui64 value64 = 0;
            if (!UnknownToken2Number(token, value64)) {
                return false;
            }
            token = to_string(value64);
            type = TYPE_CONST_NUM;
            processNumber = true;
            //fall through
        }
#endif // NUMBER
        case TYPE_DOT:
        case TYPE_CONST_NUM:
            fnEvent = &Interpreter::NumberEvent;
            break;
/****************************/
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    if (fnEvent && AppendValueNode2Arg(CreateNode<Node>(TYPE_CONST, s_empty))) {
        m_stateEventStack.push_back(&Interpreter::FunctionCallArgEvent);
        m_stateEventStack.push_back(&Interpreter::ExpEvent);
        m_stateEventStack.push_back(fnEvent);
        if (DispatchEvent(type, token)) {
            return processNumber ? ProcessNumber() : true;
        }
        return false;
    }

    return (m_error == NO_ERROR);
}

//array

bool Interpreter::ArrayDecEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    if (type == TYPE_ARRAY) {
        if (ArrayType* pArrayType = CreateNode<ArrayType>(type, token)) {
            m_stateEventStack.push_back(&Interpreter::ArrayBeginEvent);
            m_pRunableCurrent = pArrayType;
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayBeginEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    switch (type) {
        case TYPE_LESS:
            m_stateEventStack.push_back(&Interpreter::ArrayTypeEvent);
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayTypeEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    m_stateEventStack.push_back(&Interpreter::ArrayPostTypeEvent);
    switch (type) {
        case TYPE_FUNCTION:
            m_stateEventStack.push_back(&Interpreter::ArrayBeginFunctionTypeEvent);
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
        case TYPE_TOKEN:
            if (ArrayType *pArrayType = m_pRunableCurrent->QueryType<ArrayType>(TYPE_ARRAY)) {
                if (Type* pType = CreateNode<Type>((type == TYPE_TOKEN) ? TYPE_AGGREGATE_TYPE : type, token)) {
                    if (type == TYPE_TOKEN) {
                        if (AggregateType *pResolvedType = FindTypeDefinition(pType->GetSymbolId())) {
                            static_cast<AggregateType*>(pType)->SetTypeDefinition(pResolvedType);
                        }
                        else {
                            delete pType;
                            ERROR(UNDEFINED_TYPE);
                            break;
                        }
                    }
                    if (!pArrayType->SetType(pType)) {
                        delete pType;
                        ERROR(ARRAY_TYPE_ALREADY_DECLARED);
                    }
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
        default:
            ERROR(EXPECTING_TYPE_NAME);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayPostTypeEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    switch (type) {
        case TYPE_REF:
            if (ArrayType *pArrayType = m_pRunableCurrent->QueryType<ArrayType>(TYPE_ARRAY)) {
                if (!pArrayType->GetDimensionCount()) {
                    if (Type* pType = pArrayType->GetTypeNode()) {
                        if (pType->GetPassBy() == Type::PASS_BY::BY_VALUE) {
                            if (Reference *pReference = CreateNode<Reference>(type, token)) {
                                pType->SetLeft(pReference);
                                m_stateEventStack.push_back(&Interpreter::ArrayEndTypeEvent);
                            }
                        }
                        else {
                            ERROR(SYNTAX_ERROR);
                        }
                        break;
                    }
                }
                else {
                    ERROR(SYNTAX_ERROR);
                    break;
                }
            }
            break;
        case TYPE_COMMA:
        case TYPE_GREATER:
            m_stateEventStack.push_back(&Interpreter::ArrayEndTypeEvent);
            return DispatchEvent(type, token);
        case TYPE_SHIFT_RIGHT:
            m_stateEventStack.push_back(&Interpreter::ArrayEndTypeEvent);
            if (DispatchEvent(TYPE_GREATER, string(">"))) {
                return DispatchEvent(TYPE_GREATER, string(">"));
            }
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayEndTypeEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    switch (type) {
        case TYPE_COMMA:
            m_stateEventStack.push_back(&Interpreter::ArrayDimensionEvent);
            break;
        case TYPE_GREATER:
            m_stateEventStack.push_back(&Interpreter::ArrayCloseEvent);
            return DispatchEvent(type, token);
        case TYPE_SHIFT_RIGHT:
            m_stateEventStack.push_back(&Interpreter::ArrayCloseEvent);
            if (DispatchEvent(TYPE_GREATER, string(">"))) {
                return DispatchEvent(TYPE_GREATER, string(">"));
            }

        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayDimensionEvent(NODE_TYPE type, string &token) {
    bool processNumber = false;
    m_stateEventStack.pop_back();
    m_stateEventStack.push_back(&Interpreter::ArrayDimensionEndEvent);

    assert(m_pRunableCurrent->GetType() == TYPE_ARRAY);

    ArrayType *pArrayType = m_pRunableCurrent->QueryType<ArrayType>(TYPE_ARRAY);
    if (pArrayType->GetDynExpression()) {
        ERROR(DYNAMIC_ARRAY_MUST_HAVE_ONE_DIMESION);
        return false;
    }

    switch (type) {
        case TYPE_GREATER:
        case TYPE_GREATER_EQ:
        case TYPE_LESS:
        case TYPE_LESS_EQ:
        case TYPE_EQ:
        case TYPE_NOT:
        case TYPE_NOT_EQ:
        case TYPE_OR:
        case TYPE_AND:
        case TYPE_CONST_BOOL:
            //... all boolean opeartors
            ERROR(DYNAMIC_ARRAY_DIMENSION_CANNOT_HAVE_BOOLEAN_EXPRESSION);
            return false;
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN: {
            //string value;
            //if (!GetFixedNumber(type, token, value)) {
            //    return false;
            //}
            //token = value;
            //type = TYPE_CONST_NUM;
            processNumber = true;
        }
        case TYPE_NUM_EXP:
#else // NUMBER
        case TYPE_NONE: {
            ui64 value64 = 0;
            if (UnknownToken2Number(token, value64)) {
                if (value64 > 0xffffffff) {
                    ERROR(ARRAY_CANNOT_BE_LARGER_32BIT_INT);
                    return false;
                }
            }
            else {
                return false;
            }
            token = to_string(value64);
            type = TYPE_CONST_NUM;
            processNumber = true;
            //fall through
        }
#endif // NUMBER
        case TYPE_CONST_NUM: {
            if (pArrayType->GetDimensionCount()) {
                if (ui32 size = atol(token.c_str())) {
                    SYMBOL_ID symId;
                    symId = m_symbolStore.UpdateSymbolMap(token);
                    pArrayType->AddSymbol(symId);
                    pArrayType->PushDimensionSize(size);
                }
                else {
                    ERROR(EXPECTING_NONZERO_ARRAY_DIMENSION);
                }
            }
            else { // else create an expression and then we might convert it to a dimension of fixed array
                Node *pNode = CreateNode<Node>(TYPE_CONST, s_empty);
                pArrayType->SetDynExpression(pNode);
                m_pRunableCurrent = pNode;
                m_stateEventStack.push_back(&Interpreter::ArrayDimExpEvent);
                m_stateEventStack.push_back(&Interpreter::NumberEvent);
                if (DispatchEvent(type, token)) {
                    return processNumber ? ProcessNumber() : true;
                }
                return false;
            }
            break;
        }
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_TOKEN:
        case TYPE_CAST: {
            if (pArrayType->GetDimensionCount()) {
                ERROR(DYNAMIC_ARRAY_MUST_HAVE_ONE_DIMESION);
                break;
            }
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            if (FindTypeDefinition(symId)) { // types are not allowed
                ERROR(SYNTAX_ERROR);
                break;
            }

            if (m_pRunableCurrent->FindVariableDec(symId)) {
                if (pArrayType->GetParent()->GetType() != TYPE_SCOPE) {
                    ERROR(DYNAMIC_ARRAY_MUST_BE_DECLARED_IN_SCOPE);
                    break;
                }
                if (Node *pNode = CreateNode<Node>(TYPE_TOKEN, token)) {
                    pArrayType->SetDynExpression(pNode);
                    m_pRunableCurrent = pNode;
                    m_stateEventStack.push_back(&Interpreter::ArrayDimExpEvent);
                    if (type == TYPE_CAST) {
                        m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                    }
                } // else error
                break;
            }

            if (Function *pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                if (pArrayType->GetParent()->GetType() != TYPE_SCOPE) {
                    ERROR(DYNAMIC_ARRAY_MUST_BE_DECLARED_IN_SCOPE);
                    break;
                }
                if (Variable *pVariable = CreateVariableNode(0, token)) {
                    pArrayType->SetDynExpression(pVariable);
                    m_pRunableCurrent = pVariable;
                    m_stateEventStack.push_back(&Interpreter::ArrayDimExpEvent);
                    if (type == TYPE_CAST) {
                        m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                    }
                } // else error
            }
            else {
                ERROR(UNDEFINED_SYMBOL);
            }
            break;
        }
        default:
            if (Runable::IsOperator(type) || (type == TYPE_PARENTHESIS)) {
                m_stateEventStack.push_back(&Interpreter::ArrayDimExpEvent);
                return DispatchEvent(type, token);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
    }
    
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayDimensionEndEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    switch (type) {
        case TYPE_COMMA:
        case TYPE_GREATER:
            if (ArrayType *pArrayType = m_pRunableCurrent->QueryType<ArrayType>(TYPE_ARRAY)) {
                assert(!pArrayType->GetDynExpression());
                assert(pArrayType->GetDimensionCount());
                if (type == TYPE_COMMA) {
                    m_stateEventStack.push_back(&Interpreter::ArrayDimensionEvent);
                }
                else {
                    m_stateEventStack.push_back(&Interpreter::ArrayCloseEvent);
                    return DispatchEvent(type, token);
                }
            }
            else if (Node *pExpRoot = m_pRunableCurrent->GetExpressionRoot()) {
                // check expression if its' a fixed size then check if it's a multi dim array and add new dimension
                ArrayType *pArrayType = pExpRoot->GetParent()->QueryType<ArrayType>(TYPE_ARRAY);
                assert(pArrayType);
                if (pExpRoot->GetType() == TYPE_CONST) {
                    assert(!pExpRoot->GetRight() && !pExpRoot->GetLeft());
                    ConstVariable *pConstVariable = static_cast<ConstVariable*>(pExpRoot);
                    switch (pConstVariable->GetTypeNode()->GetType()) {
                        case TYPE_FLOAT:
                        case TYPE_DOUBLE:
                        case TYPE_BOOL:
                        case TYPE_STRING:
                            ERROR(EXPECTING_NUMERIC_ARRAY_SIZE);
                            break;
                        default:
                            if (ui32 size = pConstVariable->GetRef().GetUI32()) {
                                SYMBOL_ID symId;
                                symId = m_symbolStore.UpdateSymbolMap(to_string(size));
                                pArrayType->AddSymbol(symId);
                                pArrayType->PushDimensionSize(size);
                                pArrayType->SetDynExpression(0);
                                delete pExpRoot;
                                m_pRunableCurrent = pArrayType;
                                if (type == TYPE_COMMA) {
                                    m_stateEventStack.push_back(&Interpreter::ArrayDimensionEvent);
                                }
                                else {
                                    m_stateEventStack.push_back(&Interpreter::ArrayCloseEvent);
                                    return DispatchEvent(type, token);
                                }
                            }
                            else {
                                ERROR(EXPECTING_NONZERO_ARRAY_SIZE);
                            }
                            break;
                    }
                }
                else {
                    if (pArrayType->GetDimensionCount() || (type == TYPE_COMMA)) { // this is dynamic, all previous dimensions are fixed
                        ERROR(DYNAMIC_ARRAY_MUST_HAVE_ONE_DIMESION);
                    }
                    else {
                        assert(pArrayType->GetDynExpression());
                        Type *pRetType = 0;//Node *pDestNode = Scope::GetDummyTypeNode(TYPE_UI32);
                        PrePostExpressions *pPrePostExp = 0;
                        if (pExpRoot->IsExpression(pRetType, pPrePostExp, m_error)) {
                            if (!ApplyPrePostExpressions(pExpRoot, pPrePostExp)) {
                                ERROR(INTERNAL_ERROR);
                            }
                            else {
                                m_pRunableCurrent = pArrayType;
                                m_stateEventStack.push_back(&Interpreter::ArrayCloseEvent);
                                if (type == TYPE_GREATER) {
                                    return DispatchEvent(type, token);
                                }
                            }
                        } // else error
                    }
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
            //case TYPE_REF:
            //    if (ArrayType *pArrayType = m_pRunableCurrent->QueryType<ArrayType>(TYPE_ARRAY)) {
            //        if (!pArrayType->GetDimensionCount()) {
            //            if (Type* pType = pArrayType->GetTypeNode()) {
            //                if (pType->GetPassBy() == Type::PASS_BY::BY_VALUE) {
            //                    if (Reference *pReference = CreateNode<Reference>(type, token)) {
            //                        pType->SetLeft(pReference);
            //                        m_stateEventStack.push_back(&Interpreter::ArrayDimensionEndEvent);
            //                    }
            //                }
            //                else {
            //                    ERROR(SYNTAX_ERROR);
            //                }
            //                break;
            //            }
            //        }
            //        else {
            //            ERROR(SYNTAX_ERROR);
            //            break;
            //        }
            //    }
            //    ERROR(INTERNAL_ERROR);
            //    break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayCloseEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    if (TYPE_GREATER == type) {
        if (ArrayType *pArrayType = m_pRunableCurrent->QueryType<ArrayType>(TYPE_ARRAY)) {
            // this could be declaration of dimensionless array as: array<some_type>
            //if (pArrayType->GetDimensionCount()) {
            INTERPRETER_STATE_EVENT fnEvent = m_stateEventStack.back();
            if (fnEvent == &Interpreter::ScopeEvent) {
                m_stateEventStack.push_back(&Interpreter::TypeDecEvent);
            }
            else if (pArrayType->GetDynExpression()) {
                // array cannot be dynamic: array<ui32, i*f>
                ERROR(DYNAMIC_ARRAY_MUST_BE_DECLARED_IN_SCOPE);
            }
            else if (fnEvent == &Interpreter::LibFunctionNextArgTypeEvent) {
                m_pRunableCurrent = m_pRunableCurrent->GetParent();
            }
            else if (fnEvent == &Interpreter::AggregateBodyEvent) {
                if (AggregateType *pAggregateType = m_pRunableCurrent->GetParent()->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
                    pAggregateType->AddType(static_cast<ArrayType*>(m_pRunableCurrent));
                }
                m_stateEventStack.push_back(&Interpreter::AggregateMemberDecEvent);
            }
            else if (fnEvent == &Interpreter::CastCloseTypeExpression) {
                m_pRunableCurrent = m_pRunableCurrent->GetParent();
            }
            else if (fnEvent == &Interpreter::FunctionArgEvent) {
            }
            else if (fnEvent == &Interpreter::LibFunctionNameEvent) {
            }
            else if (fnEvent == &Interpreter::FunctionNameEvent) {
            }
            else {
                //assert(0);
                ERROR(INTERNAL_ERROR);
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayDimExpEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_GREATER:
            //m_stateEventStack.push_back(&Interpreter::ArrayCloseEvent);
            m_stateEventStack.pop_back();
            return DispatchEvent(type, token);
        case TYPE_SHIFT_RIGHT:
            m_stateEventStack.pop_back();
            if (DispatchEvent(TYPE_GREATER, string(">"))) {
                return DispatchEvent(TYPE_GREATER, string(">"));
            }
            return false;
        case TYPE_CONST_BOOL:
        case TYPE_GREATER_EQ:
        case TYPE_LESS:
        case TYPE_LESS_EQ:
        case TYPE_EQ:
        case TYPE_NOT:
        case TYPE_NOT_EQ:
        case TYPE_OR:
        case TYPE_AND:
            //... all boolean opeartors
            ERROR(DYNAMIC_ARRAY_DIMENSION_CANNOT_HAVE_BOOLEAN_EXPRESSION);
            return false;
        default:
            return ExpEvent(type, token);
    }
}

bool Interpreter::ArrayBeginFunctionTypeEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    switch (type) {
        case TYPE_LESS:
            m_stateEventStack.push_back(&Interpreter::ArrayFunctionTypeEvent);
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayFunctionTypeEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_TOKEN) {
        // search for function definition
        SYMBOL_ID symId;
        if (m_symbolStore.GetSymbolId(token, symId)) {
            if (Function *pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                if (FunctionRefType *pFunctionRefType = CreateNode<FunctionRefType>(TYPE_FUNCTION_REF_TYPE, token)) {
                    pFunctionRefType->PopulateTypes(pFunction);
                    if (ArrayType *pArrayType = m_pRunableCurrent->QueryType<ArrayType>(TYPE_ARRAY)) {
                        if (pArrayType->SetType(pFunctionRefType)) {
                            m_stateEventStack.pop_back();
                            m_stateEventStack.push_back(&Interpreter::ArrayEndFunctionTypeEvent);
                        }
                        else {
                            delete pFunctionRefType;
                            ERROR(ARRAY_TYPE_ALREADY_DECLARED);
                        }
                    }
                    else {
                        ERROR(INTERNAL_ERROR);
                    }
                }
            }
            else {
                ERROR(UNDEFINED_FUNCTION);
            }
        }
        else {
            ERROR(UNDEFINED_FUNCTION);
        }
    }
    else {
        ERROR(EXPECTING_FUNCTION_REF_TYPE);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayEndFunctionTypeEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_GREATER:
            m_stateEventStack.pop_back();
            break;
        case TYPE_SHIFT_RIGHT:
            m_stateEventStack.pop_back();
            return DispatchEvent(TYPE_GREATER, string(">"));
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayBeginSubscriptEvent(NODE_TYPE type, string &token) {
    INTERPRETER_STATE_EVENT fnEvent = 0;
    bool processNumber = false;
    NODE_TYPE _type = type;
    m_stateEventStack.pop_back(); // remove ArrayBeginSubscriptEvent
    m_stateEventStack.push_back(&Interpreter::ArrayEndSubscriptEvent);
    switch (type) {
        case TYPE_PARENTHESIS: {
            if (token[0] == '(') {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                return DispatchEvent(type, token);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
        }
        /****************************/
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST:
            _type = TYPE_TOKEN;
        case TYPE_CONST_BOOL:
        case TYPE_CONST:
        case TYPE_TOKEN:
        case TYPE_INC_OP:
        case TYPE_DEC_OP:
        case TYPE_BIT_NOT:
            if (AppendValueNode2Subscript(CreateNode<Node>(_type, token))) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                if (type == TYPE_CAST) {
                    m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                }
            }
            break;
        case TYPE_ADD:
        case TYPE_SUB:
            fnEvent = &Interpreter::NumberSignEvent;
            break;
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN: {
            //string value;
            //if (!GetFixedNumber(type, token, value)) {
            //    return false;
            //}
            //token = value;
            //type = TYPE_CONST_NUM;
            processNumber = true;
        }
        case TYPE_NUM_EXP:
#else // NUMBER
        case TYPE_NONE: {
            ui64 value64 = 0;
            if (!UnknownToken2Number(token, value64)) {
                return false;
            }
            token = to_string(value64);
            type = TYPE_CONST_NUM;
            processNumber = true;
            //fall through
        }
#endif // NUMBER
        case TYPE_DOT:
        case TYPE_CONST_NUM:
            fnEvent = &Interpreter::NumberEvent;
            break;
        /****************************/
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    if (fnEvent && AppendValueNode2Subscript(CreateNode<Node>(TYPE_CONST, s_empty))) {
        m_stateEventStack.push_back(&Interpreter::ExpEvent);
        m_stateEventStack.push_back(fnEvent);
        if (DispatchEvent(type, token)) {
            return processNumber ? ProcessNumber() : true;
        }
        return false;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ArrayEndSubscriptEvent(NODE_TYPE type, string &token) {
    INTERPRETER_STATE_EVENT fnEvent = 0;
    bool processNumber = false;
    NODE_TYPE _type = type;
    m_stateEventStack.pop_back(); // remove ArrayEndSubscriptEvent
    m_stateEventStack.push_back(&Interpreter::ArrayBeginSubscriptEvent);
    switch (type) {
        case TYPE_COMMA:
            if (SubscriptNode *pSubscriptNode = m_pRunableCurrent->GetParent<SubscriptNode>(TYPE_SUBSCRIPT)) {
                m_pRunableCurrent = pSubscriptNode;
                break;
            }
            ERROR(INTERNAL_ERROR);
            return false;
        case TYPE_SUBSCRIPT:
            if (token[0] == ']') {
                if (SubscriptNode *pSubscriptNode = m_pRunableCurrent->GetParent<SubscriptNode>(TYPE_SUBSCRIPT)) {
                    m_pRunableCurrent = pSubscriptNode;
                    return CloseSubscript();
                }
                ERROR(INTERNAL_ERROR);
                return false;
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        case TYPE_PARENTHESIS:
            if (token[0] == '(') {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                return DispatchEvent(type, token);
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
        /****************************/
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST:
            _type = TYPE_TOKEN;
        case TYPE_CONST_BOOL:
        case TYPE_CONST:
        case TYPE_TOKEN:
        case TYPE_INC_OP:
        case TYPE_DEC_OP:
        case TYPE_BIT_NOT:
            if (AppendValueNode2Subscript(CreateNode<Node>(_type, token))) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                if (type == TYPE_CAST) {
                    m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                }
            }
            break;
        case TYPE_ADD:
        case TYPE_SUB:
            fnEvent = &Interpreter::NumberSignEvent;
            break;
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN: {
            //string value;
            //if (!GetFixedNumber(type, token, value)) {
            //    return false;
            //}
            //token = value;
            //type = TYPE_CONST_NUM;
            processNumber = true;
        }
        case TYPE_NUM_EXP:
#else // NUMBER
        case TYPE_NONE: {
            ui64 value64 = 0;
            if (!UnknownToken2Number(token, value64)) {
                return false;
            }
            token = to_string(value64);
            type = TYPE_CONST_NUM;
            processNumber = true;
            //fall through
        }
#endif // NUMBER
        case TYPE_DOT:
        case TYPE_CONST_NUM:
            fnEvent = &Interpreter::NumberEvent;
            break;
        /****************************/
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    if (fnEvent && AppendValueNode2Subscript(CreateNode<Node>(TYPE_CONST, s_empty))) {
        m_stateEventStack.push_back(&Interpreter::ExpEvent);
        m_stateEventStack.push_back(fnEvent);
        if (DispatchEvent(type, token)) {
            return processNumber ? ProcessNumber() : true;
        }
        return false;
    }

    return (m_error == NO_ERROR);
}

//if
bool Interpreter::IfEvent(NODE_TYPE type, string &token) {
    return ConditionalDecEvent(type, token);
}

bool Interpreter::IfBodyEvent(NODE_TYPE type, string &token) {
    return BeginFullExpression(type, token);
}

bool Interpreter::ElseEvent(NODE_TYPE type, string &token) {
    if (m_pRunableCurrent->GetType() == TYPE_IF) {
        switch (type) {
            case TYPE_ELSE:
                m_stateEventStack.pop_back();// remove 'else' scope state
                m_stateEventStack.push_back(&Interpreter::ElseBodyEvent);
                static_cast<IfNode*>(m_pRunableCurrent)->EndIfClause();
                break;
            default:
                m_pRunableCurrent = m_pRunableCurrent->GetParent();
                m_stateEventStack.pop_back();// remove ElseEvent
                m_stateEventStack.pop_back();// remove IfEvent
                return DispatchEvent(type, token);
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::ElseBodyEvent(NODE_TYPE type, string &token) {
    return BeginFullExpression(type, token);
}

bool Interpreter::ForEvent(NODE_TYPE type, string &token) {
    if (TYPE_PARENTHESIS == type) {
        if (token[0] == '(') {
            m_stateEventStack.push_back(&Interpreter::ForInitEvent);
        }
        else {
            ERROR(SYNTAX_ERROR);
        }
    }
    else {
        ERROR(EXPECTING_PARENTHESIS);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::ForInitEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SEMICOLON:
            m_stateEventStack.pop_back(); // remove ForInitEvent
            m_stateEventStack.push_back(&Interpreter::ForCondEvent);
            if (m_pRunableCurrent->GetType() != TYPE_FOR) {
                m_pRunableCurrent = m_pRunableCurrent->GetExpressionRoot()->GetParent();
            }
            static_cast<ForNode*>(m_pRunableCurrent)->SetInitExp(0);
            break;
        default: {
            ForNode *pForNode = static_cast<ForNode*>(m_pRunableCurrent);
            return AppendValueNode2For(type, token, &ForNode::SetInitExp);
        }
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ForCondEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SEMICOLON:
            m_stateEventStack.pop_back(); // remove ForCondEvent
            m_stateEventStack.push_back(&Interpreter::ForLoopEvent);
            if (m_pRunableCurrent->GetType() != TYPE_FOR) {
                m_pRunableCurrent = m_pRunableCurrent->GetExpressionRoot()->GetParent();
            }
            static_cast<ForNode*>(m_pRunableCurrent)->SetCondExp(0);
            break;
        default: {
            ForNode *pForNode = static_cast<ForNode*>(m_pRunableCurrent);
            return AppendValueNode2For(type, token, &ForNode::SetCondExp);
        }
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ForLoopEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_PARENTHESIS:
            if (token[0] == ')') {
                m_stateEventStack.pop_back(); // remove ForLoopEvent
                m_stateEventStack.push_back(&Interpreter::ForBodyEvent);
                if (m_pRunableCurrent->GetType() != TYPE_FOR) {
                    m_pRunableCurrent = m_pRunableCurrent->GetExpressionRoot()->GetParent();
                }
                static_cast<ForNode*>(m_pRunableCurrent)->SetLoopExp(0);
                break;
            }
        default: {
            ForNode *pForNode = static_cast<ForNode*>(m_pRunableCurrent);
            m_stateEventStack.pop_back(); // remove ForLoopEvent
            return AppendValueNode2For(type, token, &ForNode::SetLoopExp);
        }
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AppendValueNode2For(NODE_TYPE type, string &token, ForNode::APPEND_VALUE_NODE_2_FOR fnAppendValueNode) {
    INTERPRETER_STATE_EVENT fnEvent = 0;
    bool processNumber = false;
    NODE_TYPE _type = type;
    switch (type) {
/****************************/
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST:
            _type = TYPE_TOKEN;
        case TYPE_CONST_BOOL:
        case TYPE_CONST:
        case TYPE_TOKEN:
        case TYPE_INC_OP:
        case TYPE_DEC_OP:
        case TYPE_BIT_NOT:
        case TYPE_PARENTHESIS:
            if (token[0] == ')') {
                break;
            }
            if (Node *pNode = CreateNode<Node>(_type, token)) {
                ForNode *pForNode = static_cast<ForNode*>(m_pRunableCurrent);
                (pForNode->*fnAppendValueNode)(pNode);
                m_pRunableCurrent = pNode;
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                if (type == TYPE_CAST) {
                    m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                }
            }
            break;
        case TYPE_ADD:
        case TYPE_SUB:
            fnEvent = &Interpreter::NumberSignEvent;
            break;
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN: {
            //string value;
            //if (!GetFixedNumber(type, token, value)) {
            //    return false;
            //}
            //token = value;
            //type = TYPE_CONST_NUM;
            processNumber = true;
        }
        case TYPE_NUM_EXP:
#else // NUMBER
        case TYPE_NONE: {
            ui64 value64 = 0;
            if (!UnknownToken2Number(token, value64)) {
                return false;
            }
            token = to_string(value64);
            type = TYPE_CONST_NUM;
            processNumber = true;
            //fall through
        }
#endif // NUMBER
        case TYPE_DOT:
        case TYPE_CONST_NUM:
            fnEvent = &Interpreter::NumberEvent;
            break;
/****************************/
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    if (fnEvent) {
        if (Node *pNode = CreateNode<Node>(TYPE_CONST, s_empty)) {
            ForNode *pForNode = static_cast<ForNode*>(m_pRunableCurrent);
            (pForNode->*fnAppendValueNode)(pNode);
            m_pRunableCurrent = pNode;
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            m_stateEventStack.push_back(fnEvent);
            if (DispatchEvent(type, token)) {
                return processNumber ? ProcessNumber() : true;
            }
            return false;
        }
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ForBodyEvent(NODE_TYPE type, string &token) {
    return BeginFullExpression(type, token);
}

bool Interpreter::WhileEvent(NODE_TYPE type, string &token) {
    return ConditionalDecEvent(type, token);
}

bool Interpreter::WhileBodyEvent(NODE_TYPE type, string &token) {
    return BeginFullExpression(type, token);
}

bool Interpreter::ReturnEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back(); // remove ReturnEvent
    if (Function *pFunction = m_pRunableCurrent->GetParent<Function>(TYPE_FUNCTION)) {
        if (type == TYPE_SEMICOLON) {
            // allow to return NULL? -> "return;"
            m_pRunableCurrent = m_pRunableCurrent->GetParent();
            return DispatchEvent(type, token);
        }
        else {
            INTERPRETER_STATE_EVENT fnEvent = 0;
            NODE_TYPE _type = type;
            bool processNumber = false;
            switch (type) {
                case TYPE_PARENTHESIS:
                    if (token[0] == '(') {
                        m_stateEventStack.push_back(&Interpreter::ExpEvent);
                        return DispatchEvent(type, token);
                    }
                    else {
                        ERROR(INTERNAL_ERROR);
                    }
                    break;
/****************************/
                case TYPE_SIZEOF:
                case TYPE_ARR2STR:
                case TYPE_WARR2STR:
                case TYPE_STR2ARR:
                case TYPE_STR2WARR:
                case TYPE_LOCK:
                case TYPE_UNLOCK:
                case TYPE_CAST:
                    _type = TYPE_TOKEN;
                case TYPE_CONST_BOOL:
                case TYPE_CONST:
                case TYPE_TOKEN:
                case TYPE_INC_OP:
                case TYPE_DEC_OP:
                case TYPE_BIT_NOT:
                    if (AppendValueNode2Op(CreateNode<Node>(_type, token))) {
                        m_stateEventStack.push_back(&Interpreter::ExpEvent);
                        if (type == TYPE_CAST) {
                            m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                        }
                    }
                    break;
                case TYPE_ADD:
                case TYPE_SUB:
                    fnEvent = &Interpreter::NumberSignEvent;
                    break;
#ifdef NUMBER
                case TYPE_NUM_HEX_1:
                case TYPE_NUM_HEX_2:
                case TYPE_NUM_BIN: {
                    //string value;
                    //if (!GetFixedNumber(type, token, value)) {
                    //    return false;
                    //}
                    //token = value;
                    //type = TYPE_CONST_NUM;
                    processNumber = true;
                }
                case TYPE_NUM_EXP:
#else // NUMBER
                case TYPE_NONE: {
                    ui64 value64 = 0;
                    if (!UnknownToken2Number(token, value64)) {
                        return false;
                    }
                    token = to_string(value64);
                    type = TYPE_CONST_NUM;
                    processNumber = true;
                    //fall through
                }
#endif // NUMBER
                case TYPE_DOT:
                case TYPE_CONST_NUM:
                    fnEvent = &Interpreter::NumberEvent;
                    break;
/****************************/
                default:
                    ERROR(SYNTAX_ERROR);
                    break;
            }

            if (fnEvent && AppendValueNode2Op(CreateNode<Node>(TYPE_CONST, s_empty))) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                m_stateEventStack.push_back(fnEvent);
                if (DispatchEvent(type, token)) {
                    return processNumber ? ProcessNumber() : true;
                }
                return false;
            }
        }
    }
    else {
        ERROR(UNEXPECTED_RETURN);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::BreakEvent(NODE_TYPE type, string &token) {
    if (TYPE_SEMICOLON == type) {
        m_stateEventStack.pop_back(); // remove BreakEvent
        m_pRunableCurrent = m_pRunableCurrent->GetParent();
        return DispatchEvent(type, token);
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ContinueEvent(NODE_TYPE type, string &token) {
    if (TYPE_SEMICOLON == type) {
        m_stateEventStack.pop_back(); // remove BreakEvent
        m_pRunableCurrent = m_pRunableCurrent->GetParent();
        return DispatchEvent(type, token);
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::SwitchEvent(NODE_TYPE type, string &token) {
    return ConditionalDecEvent(type, token);
}

bool Interpreter::SwitchBodyEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SCOPE:
            if (token[0] == '{') {// begin switch body
                m_stateEventStack.push_back(&Interpreter::SwitchCasesEvent);
            }
            else {
                m_pRunableCurrent = m_pRunableCurrent->GetParent();
                m_stateEventStack.pop_back(); // remove SwitchBodyEvent
                m_stateEventStack.pop_back(); // remove SwitchEvent
                return DispatchEvent(TYPE_SEMICOLON, s_semicolon);
            }
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::SwitchCasesEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_CASE:
            if (CaseNode *pCaseNode = CreateNode<CaseNode>(TYPE_CASE, token)) {
                static_cast<SwitchNode*>(m_pRunableCurrent)->UpdateCaseExp(pCaseNode);
                m_pRunableCurrent = pCaseNode;
                m_stateEventStack.push_back(&Interpreter::CaseEvent);
            }
            break;
        case TYPE_DEFAULT: {
            SwitchNode *pSwitchNode = static_cast<SwitchNode*>(m_pRunableCurrent);
            if (!pSwitchNode->GetLeft()) {
                if (DefaultNode *pDefaultNode = CreateNode<DefaultNode>(TYPE_DEFAULT, token)) {
                    pSwitchNode->SetDefault(pDefaultNode);
                    m_pRunableCurrent = pDefaultNode;
                    m_stateEventStack.push_back(&Interpreter::DefaultEvent);
                }
            }
            else {
                ERROR(DUPLICATE_DEFAULT);
            }
            break;
        }
        case TYPE_SCOPE:
            if (token[0] == '}') {
                m_stateEventStack.pop_back(); // remove SwitchCasesEvent
                switch (m_pRunableCurrent->GetType()) {
                    case TYPE_CASE:
                    case TYPE_DEFAULT:
                        m_pRunableCurrent = m_pRunableCurrent->GetParent();
                        return DispatchEvent(type, token);
                    default:
                        ERROR(SYNTAX_ERROR);
                        break;
                }
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::CaseEvent(NODE_TYPE type, string &token) {
    switch (type) {
/****************************/
        //case TYPE_SIZEOF:
        //    assert(0);
        //    break;
        case TYPE_CONST:
        case TYPE_TOKEN:
            ERROR(EXPECTING_32BIT_NUMBER);
            break;
        case TYPE_ADD:
        case TYPE_SUB:
            AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token));
            m_stateEventStack.push_back(&Interpreter::CaseValueEvent);
            break;
#ifdef NUMBER
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN: {
            string value;
            if (GetFixedNumber(type, token, value)) {
                m_stateEventStack.push_back(&Interpreter::CaseValueEvent);
                return DispatchEvent(TYPE_CONST_NUM, value);
            }
            return false;
        }
#else // NUMBER
        case TYPE_NONE: {
            ui64 value64 = 0;
            if (!UnknownToken2Number(token, value64)) {
                return false;
            }
            token = to_string(value64);
            type = TYPE_CONST_NUM;
            //fall through
        }
#endif // NUMBER
        case TYPE_CONST_NUM:
            m_stateEventStack.push_back(&Interpreter::CaseValueEvent);
            return DispatchEvent(type, token);

            /****************************/
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::CaseValueEvent(NODE_TYPE type, string &token) {
    if (TYPE_CONST_NUM == type) {
        string value;
        if (AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token)) && dynamic_cast<AggregateSymbol*>(m_pRunableCurrent)->GetString(value)) {
            FLOAT_NUM fn;
            if (fn.Set(value)) {
                switch (fn.GetType()) {
                    case Value::I8_TYPE:
                    case Value::UI8_TYPE:
                    case Value::I16_TYPE:
                    case Value::UI16_TYPE:
                    case Value::I32_TYPE:
                    case Value::UI32_TYPE:
                        if (static_cast<SwitchNode*>(m_pRunableCurrent->GetParent())->SetCase(fn.GetUI32())) {
                            m_stateEventStack.pop_back(); // remove CaseValueEvent;
                            m_stateEventStack.push_back(&Interpreter::CaseColonEvent);
                        }
                        else {
                            ERROR(DUPLICATE_CASE);
                        }
                        break;
                    default:
                        ERROR(EXPECTING_32BIT_NUMBER);
                        break;
                }
            }
            else {
                ERROR(EXPECTING_32BIT_NUMBER);
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    
    return (m_error == NO_ERROR);
}

bool Interpreter::CaseColonEvent(NODE_TYPE type, string &token) {
    if (TYPE_COLON == type) {
        m_stateEventStack.pop_back(); // remove CaseColonEvent;
        m_stateEventStack.push_back(&Interpreter::CaseBodyEvent);
    }
    else {
        ERROR(EXPECTING_COLON);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::CaseBodyEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_CASE:
        case TYPE_DEFAULT:
            switch (m_pRunableCurrent->GetType()) {
                case TYPE_DEFAULT:
                case TYPE_CASE:
                    m_stateEventStack.pop_back(); // remove CaseEvent;
                    m_stateEventStack.pop_back(); // remove CaseBodyEvent or FefaultBodyEvent
                    m_pRunableCurrent = m_pRunableCurrent->GetParent();
                    return DispatchEvent(type, token);
                case TYPE_SWITCH:
                    m_stateEventStack.pop_back(); // remove CaseBodyEvent;
                    break;
                case TYPE_SCOPE:
                    if (static_cast<Scope*>(m_pRunableCurrent)->GetSymbolId() == SYMBOL_ID_BLANK) {
                        // end this scope
                        m_stateEventStack.pop_back(); // remove CaseEvent;
                        m_stateEventStack.pop_back(); // remove CaseBodyEvent or FefaultBodyEvent
                        m_pRunableCurrent = m_pRunableCurrent->GetParent()->GetParent();
                        return DispatchEvent(type, token);
                    }
                    else {
                        ERROR(INTERNAL_ERROR);
                    }
                    break;
                default:
                    ERROR(INTERNAL_ERROR);
                    break;
            }
            return (m_error == NO_ERROR);
        case TYPE_SCOPE:
            if (token[0] == '}') {
                switch (m_pRunableCurrent->GetType()) {
                    case TYPE_SCOPE:
                        if (static_cast<Scope*>(m_pRunableCurrent)->GetSymbolId() == SYMBOL_ID_BLANK) {
                            m_pRunableCurrent = m_pRunableCurrent->GetParent();
                        }
                        else {
                            break;
                        }
                    case TYPE_DEFAULT:
                    case TYPE_CASE:
                        m_stateEventStack.pop_back(); // remove CaseBodyEvent;
                        m_stateEventStack.pop_back(); // remove CaseEvent;
                        return DispatchEvent(type, token);
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }

    switch (m_pRunableCurrent->GetType()) {
        case TYPE_SCOPE:
            if (m_pRunableCurrent->GetSymbolId() != SYMBOL_ID_BLANK) {
                ERROR(INTERNAL_ERROR);
            }
            break;
        case TYPE_CASE:
        case TYPE_DEFAULT:
            if (Scope *pCaseScope = CreateNode<Scope>(TYPE_SCOPE, s_empty)) {
                m_pRunableCurrent->SetLeft(pCaseScope);
                m_pRunableCurrent = pCaseScope;
            }
            break;
        default:
            ERROR(INTERNAL_ERROR);
            break;
    }

    if (m_error == NO_ERROR) {
        return BeginFullExpression(type, token);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::DefaultEvent(NODE_TYPE type, string &token) {
    if (TYPE_COLON == type) { // set default of switch node
        m_stateEventStack.push_back(&Interpreter::DefaultBodyEvent);
    }
    else {
        ERROR(EXPECTING_COLON);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::DefaultBodyEvent(NODE_TYPE type, string &token) {
    return CaseBodyEvent(type, token);
}

bool Interpreter::ConditionalDecEvent(NODE_TYPE type, string &token) {
    if (TYPE_PARENTHESIS == type) {
        if (token[0] == '(') {
            m_stateEventStack.push_back(&Interpreter::ExpEvent); // stack: ...,ScopeEvent,[IfEvent|WhileEvent|SwitchEvent|ForEvent],ExpEvent,
            return DispatchEvent(type, token);
        }
        else {
            ERROR(UNEXPECTED_PARENTHESIS);
        }
    }
    else {
        ERROR(EXPECTING_PARENTHESIS);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_TOKEN) {
        if (AggregateDefCollection *pAggregateDefCollection = m_pRunableCurrent->QueryType<AggregateDefCollection>(TYPE_AGGREGATE_TYPE_COLLECTION)) {
            if (AggregateType *pAggregateType = CreateNode<AggregateType>(TYPE_AGGREGATE_TYPE, token)) {
                pAggregateDefCollection->PushAggregateDef(pAggregateType);
                if (AggregateType *pParentAggregateType = m_pRunableCurrent->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
                    pParentAggregateType->AddType(pAggregateType);
                }
                m_stateEventStack.pop_back(); // remove AggregateEvent
                m_stateEventStack.push_back(&Interpreter::AggregateBeginEvent);
                m_pRunableCurrent = pAggregateType;
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(EXPECTING_STRUCT_NAME);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateBeginEvent(NODE_TYPE type, string &token) {
    if ((type == TYPE_SCOPE) && (token[0] == '{')) {
        m_stateEventStack.pop_back(); // remove AggregateBeginEvent
        m_stateEventStack.push_back(&Interpreter::AggregateBodyEvent);
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateBodyEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_FUNCTION:
            m_stateEventStack.push_back(&Interpreter::AggregateFunctionTypeEvent);
            break;
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST:
            ERROR(EXPECTING_TYPE_NAME);
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
        case TYPE_TOKEN:
            if (AggregateType *pAggregateType = m_pRunableCurrent->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
                if (Type* pType = CreateNode<Type>((type == TYPE_TOKEN) ? TYPE_AGGREGATE_TYPE : type, token)) {
                    if (type == TYPE_TOKEN) {
                        if (AggregateType *pResolvedType = FindTypeDefinition(pType->GetSymbolId())) {
                            static_cast<AggregateType*>(pType)->SetTypeDefinition(pResolvedType);
                        }
                        else {
                            ERROR(UNDEFINED_TYPE);
                            break;
                        }
                    }
                    pAggregateType->AddType(pType);
                    m_pRunableCurrent = pType;
                    m_stateEventStack.push_back(&Interpreter::AggregateMemberDecEvent);
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
        case TYPE_AGGREGATE_TYPE:
            break;
        case TYPE_STRUCT:
            m_stateEventStack.push_back(&Interpreter::AggregateMemberDecEvent);
            m_stateEventStack.push_back(&Interpreter::AggregateEvent);
            break;
        case TYPE_ARRAY:
            m_stateEventStack.push_back(&Interpreter::ArrayDecEvent);
            return DispatchEvent(type, token);
        case TYPE_SCOPE:
            if (token[0] == '}') {
                m_stateEventStack.pop_back(); // remove AggregateBodyEvent
                m_stateEventStack.push_back(&Interpreter::AggregateEndEvent);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateEndEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back(); // remove AggregateEndEvent
    if (AlignAggregateType()) {
        switch (type) {
            case TYPE_SEMICOLON:
                m_pRunableCurrent = m_pRunableCurrent->GetParent();
                m_stateEventStack.pop_back(); // remove AggregateVarDecEvent
                if (m_pRunableCurrent->GetType() != TYPE_SCOPE) {
                    return true;
                }
            case TYPE_TOKEN:
                return DispatchEvent(type, token);
            case TYPE_SIZEOF:
            case TYPE_ARR2STR:
            case TYPE_WARR2STR:
            case TYPE_STR2ARR:
            case TYPE_STR2WARR:
            case TYPE_LOCK:
            case TYPE_UNLOCK:
            case TYPE_CAST:
                ERROR(EXPECTING_VARIABLE_NAME);
                break;
            case TYPE_REF:
                if (m_pRunableCurrent->GetParent()->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
                    if (Reference *pReference = CreateNode<Reference>(type, token)) {
                        if (!m_pRunableCurrent->GetLeft()) {
                            m_pRunableCurrent->SetLeft(pReference);
                            return true;
                        }
                        else {
                            delete pReference;
                            ERROR(SYNTAX_ERROR);
                        }
                    }
                }
                else {
                    ERROR(SYNTAX_ERROR);
                }
                break;
            default:
                ERROR(SYNTAX_ERROR);
                break;
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }
    return false;
}

bool Interpreter::AggregateVarDecEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_TOKEN) {
        if (Type *pType = m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) { // any type works, so query for TYPE_RUNABLE
            if (Variable *pVariable = CreateVariableNode(pType, token)) {
                m_stateEventStack.pop_back(); // remove AggregateVarDecEvent
                m_stateEventStack.push_back(&Interpreter::AggregateVarDecNextEvent);
                pType->PushVariable(pVariable);
                m_pRunableCurrent = pVariable;
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateVarDecNextEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_COMMA:
            m_pRunableCurrent = m_pRunableCurrent->GetParent();
            m_stateEventStack.pop_back(); // remove AggregateVarDecNextEvent
            m_stateEventStack.push_back(&Interpreter::AggregateVarDecEvent);
            break;
        case TYPE_SEMICOLON:
            m_pRunableCurrent = m_pRunableCurrent->GetParent()->GetParent();
            m_stateEventStack.pop_back(); // remove AggregateVarDecNextEvent
            return DispatchEvent(type, token);
                                          //m_stateEventStack.pop_back(); // remove EndScopeExpression
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateFunctionTypeEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back(); // remove FunctionTypeEvent
    switch (type) {
        case TYPE_LESS:// this is function type: function<xxx>
            m_stateEventStack.push_back(&Interpreter::AggregateFunctionBeginRefDecEvent);
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateFunctionBeginRefDecEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_TOKEN) {
        // search for function definition
        SYMBOL_ID symId;
        if (m_symbolStore.GetSymbolId(token, symId)) {
            if (Function *pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                if (FunctionRefType *pFunctionRefType = CreateNode<FunctionRefType>(TYPE_FUNCTION_REF_TYPE, token)) {
                    pFunctionRefType->PopulateTypes(pFunction);
                    m_pRunableCurrent = pFunctionRefType;
                    m_stateEventStack.pop_back();
                    m_stateEventStack.push_back(&Interpreter::AggregateFunctionEndRefDecEvent);
                    return true;
                }
                else {
                    return false;
                }
            }
        }
    }
    ERROR(EXPECTING_FUNCTION_REF_TYPE);

    return false;

}

bool Interpreter::AggregateFunctionEndRefDecEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_GREATER) {
        m_stateEventStack.pop_back();
        if (AggregateType *pAggregateType = m_pRunableCurrent->GetParent()->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
            if (FunctionRefType* pFunctionRefType = m_pRunableCurrent->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
                pAggregateType->AddType(pFunctionRefType);
                m_stateEventStack.push_back(&Interpreter::AggregateMemberDecEvent);
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateMemberDecEvent(NODE_TYPE type, string &token) {

    if (type == TYPE_REF) {
        if (m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) {
            if (Reference *pReference = CreateNode<Reference>(type, token)) {
                if (!m_pRunableCurrent->GetLeft()) {
                    m_pRunableCurrent->SetLeft(pReference);
                }
                else {
                    delete pReference;
                    ERROR(SYNTAX_ERROR);
                }
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else if (type == TYPE_TOKEN) {
        if (Type *pType = m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) { // any type works, so query for TYPE_RUNABLE
            if (ErrorIfDynamicArray(pType)) {
                return false;
            }
            if (Variable *pVariable = CreateMemberDecNode(token)) {
                m_stateEventStack.pop_back(); // remove AggregateMemberDecEvent
                m_stateEventStack.push_back(&Interpreter::AggregateMemberDecNextEvent);
                pType->PushVariable(pVariable);
                m_pRunableCurrent = pVariable;
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(EXPECTING_VARIABLE_NAME);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::AggregateMemberDecNextEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_COMMA:
            m_pRunableCurrent = m_pRunableCurrent->GetParent();
            m_stateEventStack.pop_back(); // remove AggregateMemberDecNextEvent
            m_stateEventStack.push_back(&Interpreter::AggregateMemberDecEvent);
            break;
        case TYPE_SEMICOLON:
            m_pRunableCurrent = m_pRunableCurrent->GetParent()->GetParent();
            m_stateEventStack.pop_back(); // remove AggregateMemberDecNextEvent
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

// error related events
bool Interpreter::ErrorBeginEvent(NODE_TYPE type, string &token) {
    if (m_pRunableCurrent->GetType() == TYPE_ERROR) {
        if ((type == TYPE_PARENTHESIS) && (token[0] == '(')) {
            m_stateEventStack.pop_back(); // remove ErrorBeginEvent
            m_stateEventStack.push_back(&Interpreter::ErrorInstanceEvent);
        }
        else {
            ERROR(SYNTAX_ERROR);
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ErrorInstanceEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_VAR) {
        if (ErrorNode *pErrorNode = m_pRunableCurrent->QueryType<ErrorNode>(TYPE_ERROR)) {
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            SYMBOL_DESC symDesc(symId
//#ifdef _DEBUG
                ,m_pReader->GetFileId(), m_pReader->GetLine(), m_pReader->GetPosition()
//#endif // _DEBUG
                );
            assert(!pErrorNode->GetLeft());
            if (Scope *pScope = CreateNode<Scope>(string("{"))) {
#ifdef SCRIPT_DEBUGGER
                pScope->SetDebuggerHandler(&m_debuggerHandler);
                pScope->SetBreakPointLineByFileId(&m_breakPointLines);
#endif // SCRIPT_DEBUGGER
                pErrorNode->SetLeft(pScope);
                ui32 idx;
                MEMORY_OFFSET alignedOffset;
                Type *pType = Scope::GetDummyTypeNode(TYPE_ERROR_TYPE);
                alignedOffset = ReserveAlignedStackSpotIfNotDeclared(symDesc, pType, idx);
                if (ErrorVariable *pErrorVariable = new ErrorVariable(&m_symbolStore, pErrorNode, symDesc, idx, pType)) {
                    pScope->PushExpression(pErrorVariable);
                }
            }
            else {
                ERROR(OUT_OF_MEMORY);
            }
            m_stateEventStack.pop_back(); // remove ErrorInstanceEvent
            m_stateEventStack.push_back(&Interpreter::ErrorEndEvent);
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(EXPECTING_ERROR_VARIABLE_NAME);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ErrorEndEvent(NODE_TYPE type, string &token) {
    if (m_pRunableCurrent->GetType() == TYPE_ERROR) {
        if ((type == TYPE_PARENTHESIS) && (token[0] == ')')) {
            m_stateEventStack.pop_back(); // remove ErrorEndEvent
            m_stateEventStack.push_back(&Interpreter::ErrorBodyEvent);
        }
        else {
            ERROR(SYNTAX_ERROR);
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ErrorBodyEvent(NODE_TYPE type, string &token) {
    if (TYPE_SCOPE == type) {
        if (token[0] == '{') {
            if (ErrorNode *pErrorNode = m_pRunableCurrent->QueryType<ErrorNode>(TYPE_ERROR)) {
                m_pRunableCurrent = pErrorNode->GetLeft();
                m_stateEventStack.pop_back();//remove ErrorBodyEvent
                m_stateEventStack.push_back(&Interpreter::ScopeEvent);
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
        }
        else {
            ERROR(UNEXPECTED_CHARACTER);
        }
    }
    else {
        ERROR(UNEXPECTED_CHARACTER);
    }
    return (m_error == NO_ERROR);
}

// type events
bool Interpreter::TypeDecEvent(NODE_TYPE type, string &token) {
    if (TYPE_TOKEN == type) {
        SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
        if (GetDefinedSymbolNode(symId)) {
            ERROR(DUPLICATE_DECLARATION);
        }
        else if (Type *pType = m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) { // any type works, so query for TYPE_RUNABLE
            if (Variable *pVariable = CreateVariableNode(pType, token)) {
                m_stateEventStack.pop_back(); // remove TypeDecEvent
                m_stateEventStack.push_back(&Interpreter::TypeDecNextEvent);
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                m_stateEventStack.push_back(&Interpreter::TypeInitEvent);
                pType->PushVariable(pVariable);
                m_pRunableCurrent = pVariable;
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(EXPECTING_SYMBOL);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::TypeDecNextEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_COMMA: {
            Node *pNode = m_pRunableCurrent->GetExpressionRoot();
            Type *pRetType = 0;//Node *pDestNode = 0;
            PrePostExpressions *pPrePostExp = 0;
            if (pNode->IsExpression(pRetType, pPrePostExp, m_error)) {
                m_pRunableCurrent = pNode->GetParent();
                if (!ApplyPrePostExpressions(pNode, pPrePostExp)) {
                    ERROR(INTERNAL_ERROR);
                    return false;
                }
                m_stateEventStack.pop_back(); // remove TypeDecNextEvent
                m_stateEventStack.push_back(&Interpreter::TypeDecEvent);
            }
            else if (pPrePostExp) {
                delete pPrePostExp;
            }
            break;
        }
        case TYPE_SEMICOLON: 
            m_stateEventStack.pop_back(); // remove TypeDecNextEvent
            m_pRunableCurrent = m_pRunableCurrent->GetParent();
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::NumberSignEvent(NODE_TYPE type, string &token) {
    // expecting ± sign only
    m_stateEventStack.pop_back(); // remove NumberSignEvent
    switch (type) {
        case TYPE_ADD:
        case TYPE_SUB:
            if (AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token))) {
                m_stateEventStack.push_back(&Interpreter::NumberEvent);
            }
            break;
        default:
            ERROR(INVALID_NUMBER_FORMAT);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::GetUI64Number(string &str, int begin, int end, ui64 &result) {
    result = 0;
    for (int i = begin; i <= end; ++i) {
        char ch = str[i];
        if (ch >= '0') {
            if (ch <= '9') {
                result = (result << 4) | (ch - '0');
                continue;
            }
            else if (ch >= 'A') {
                if (ch <= 'F') {
                    result = (result << 4) | (ch - 'A' + 10);
                    continue;
                }
                else if (ch >= 'a') {
                    if (ch <= 'f') {
                        result = (result << 4) | (ch - 'a' + 10);
                        continue;
                    }
                }
            }
        }
        ERROR(INVALID_HEX_NUMBER);
        return false;
    }
    return true;
}
//
//bool Interpreter::UnknownToken2Number(string &token, ui64 &value64) {
//    if (token.length() < 2) {
//        ERROR(UNEXPECTED_TOKEN);
//        return false;
//    }
//
//    value64 = 0;
//
//    if ((token[0] == '0') && (token[1] == 'x' || token[1] == 'X')) { // hex number
//        if ((token.length() < 3) || (token.length() > 18)) {
//            ERROR(INVALID_HEX_NUMBER);
//            return false;
//        }
//
//        if (!GetUI64Number(token, 2, token.length() - 1, value64)) {
//            return false;
//        }
//    }
//    else {
//        //check last char
//        char id = token[token.length() - 1];
//
//        switch (id) {
//            case 'h':
//            case 'H': {
//                if (token.length() > 17) {
//                    ERROR(INVALID_HEX_NUMBER);
//                    return false;
//                }
//                if (!GetUI64Number(token, 0, token.length() - 2, value64)) {
//                    return false;
//                }
//                break;
//            }
//            case 'b':
//            case 'B': {
//                if (token.length() > 65) {
//                    ERROR(INVALID_BINARY_NUMBER);
//                    return false;
//                }
//
//                for (size_t i = 0; i < token.length() - 1; ++i) {
//                    char ch = token[i];
//                    if (ch == '0') {
//                        value64 <<= 1;
//                        continue;
//                    }
//                    else if (ch == '1') {
//                        value64 = (value64 << 1) | 1;
//                        continue;
//                    }
//                    ERROR(INVALID_BINARY_NUMBER);
//                    return false;
//                }
//                break;
//            }
//            case 'e':
//            case 'E':
//                for (ui32 i = 0; i < token.length() - 1; ++i) {
//                    char ch = token[i];
//                    if (ch >= '0' && ch <= '9') {
//                        value64 = (value64 * 10) + (ch - '0');
//                        continue;
//                    }
//                    ERROR(SYNTAX_ERROR);
//                    return false;
//                }
//                break;
//            default:
//                ERROR(UNEXPECTED_TOKEN);
//                return false;
//        }
//    }
//    
//    return true;
//}

#ifdef NUMBER
bool Interpreter::GetHex1Number(string &token, string &value) {
    ui64 value64 = 0;
    if (GetUI64Number(token, 2, token.length() - 1, value64)) {
        value = to_string(value64);
        return true;
    }
    return false;
}

bool Interpreter::GetHex2Number(string &token, string &value) {
    ui64 value64 = 0;
    if (GetUI64Number(token, 0, token.length() - 2, value64)) {
        value = to_string(value64);
        return true;
    }
    return false;
}

bool Interpreter::GetBinNumber(string &token, string &value) {
    ui64 value64 = 0;
    for (size_t i = 0; i < token.length() - 1; ++i) {
        switch (token[i]) {
            case '0':
                value64 <<= 1;
                break;
            case '1':
                value64 = (value64 << 1) | 1;
                break;
            default:
                ERROR(INVALID_BINARY_NUMBER);
                return false;
        }
    }
    value = to_string(value64);
    return true;
}

bool Interpreter::GetFixedNumber(NODE_TYPE type, string &token, string &value) {
    switch (type) {
        case TYPE_NUM_HEX_1:
            return GetHex1Number(token, value);
        case TYPE_NUM_HEX_2:
            return GetHex2Number(token, value);
        case TYPE_NUM_BIN:
            return GetBinNumber(token, value);
        default:
            ERROR(INTERNAL_ERROR);
            return false;
    }
}

bool Interpreter::IsExpNumber(string &token) {
    for (ui32 i = 0; i < token.length() - 1; ++i) {
        char ch = token[i];
        if (ch >= '0' && ch <= '9') {
            continue;
        }
        ERROR(SYNTAX_ERROR);
        return false;
    }
    return true;
}

bool Interpreter::ExpNumberTokenEvent(NODE_TYPE type, string &token) {
    string value;
    bool good = true;
    switch (type) {
        case TYPE_NUM_HEX_1:
            //good = GetHex1Number(token, value);
            //break;
        case TYPE_NUM_HEX_2:
            //good = GetHex2Number(token, value);
            //break;
        case TYPE_NUM_BIN:
            //good = GetBinNumber(token, value);
            value = token;
            break;
        case TYPE_NUM_EXP:
            if (good = IsExpNumber(token)) {
                value = token;
                m_stateEventStack.push_back(&Interpreter::ExponentSignEvent);
            }
            break;
        default:
            good = false;
            ERROR(INTERNAL_ERROR);
            break;
    }

    if (!good) {
        return false;
    }

    if (!AppendValueNode2Op(CreateNode<Node>(TYPE_CONST, s_empty))) {
        return false;
    }

    if (!AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(value))) {
        return false;
    }

    return (type != TYPE_NUM_EXP) ? ProcessNumber(): true;
}

#else // NUMBER

bool Interpreter::UnknownTokenEvent(NODE_TYPE type, string &token) {
    // check if its a hex number: 0x..., 0X..., ...h, ...H (example: 0x1234ABCD )
    // check if its a binary number ...b or ...B (example: 10110001b)
    ui64 value64 = 0;
    if (!UnknownToken2Number(token, value64)) {
        return false;
    }

    if (!AppendValueNode2Op(CreateNode<Node>(TYPE_CONST, s_empty))) {
        return false;
    }
    token = to_string(value64);
    type = TYPE_CONST_NUM;
    //m_stateEventStack.push_back(&Interpreter::ExponentSignEvent);
    m_stateEventStack.push_back(&Interpreter::NumberEvent);
    return DispatchEvent(type, token) && ProcessNumber();

    //if (!AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(value))) {
    //    return false;
    //}

    //return ProcessNumber();
}
#endif // NUMBER

bool Interpreter::NumberEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back(); // remove NumberEvent
    switch (type) {
        case TYPE_NUM_HEX_1:
        case TYPE_NUM_HEX_2:
        case TYPE_NUM_BIN:
            return AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token));
        case TYPE_CONST_NUM:
            if (AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token))) {
                m_stateEventStack.push_back(&Interpreter::DecimalPointEvent);
            }
            break;
        case TYPE_DOT:
            m_stateEventStack.push_back(&Interpreter::DecimalPointEvent);
            return DispatchEvent(type, token);
#ifdef NUMBER
        case TYPE_NUM_EXP:
            if (IsExpNumber(token) && AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token))) {
                m_stateEventStack.push_back(&Interpreter::ExponentSignEvent);
            }
            break;
#else // NUMBER
        case TYPE_NONE: {
            char last = token.back();
            if ((last == 'e') || (last == 'E')) {
                bool good = false;
                for (ui32 i = 0; i < token.length() - 1; ++i) {
                    char ch = token[i];
                    good = (ch >= '0' && ch <= '9');
                    if (good) {
                        continue;
                    }
                    ERROR(SYNTAX_ERROR);
                    break;
                }
                if (good && AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token))) {
                    m_stateEventStack.push_back(&Interpreter::ExponentSignEvent);
                }
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        }
#endif // NUMBER
        default:
            ERROR(EXPECTING_NUMBER);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::DecimalPointEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back(); // remove DecimalPointEvent
    switch (type) {
        case TYPE_DOT:
            if (AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token))) {
                m_stateEventStack.push_back(&Interpreter::FractionEvent);
            }
            break;
        default:
            if (ProcessNumber()) {
                return DispatchEvent(type, token);
            }
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FractionEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back(); // remove FractionEvent
    switch (type) {
        case TYPE_CONST_NUM:
            if (AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token))) {
                return ProcessNumber();
            }
            break;
#ifdef NUMBER
        case TYPE_NUM_EXP:
#else // NUMBER
        case TYPE_NONE: 
#endif // NUMBER
            m_stateEventStack.push_back(&Interpreter::NumberEvent);
            return DispatchEvent(type, token);
        default:
            ERROR(INVALID_NUMBER_FORMAT);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ExponentSignEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back(); // remove ExponentSignEvent
    switch (type) {
        case TYPE_ADD:
        case TYPE_SUB:
            if (AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token))) {
                m_stateEventStack.push_back(&Interpreter::ExponentEvent);
            }
            break;
        default:
            ERROR(INVALID_NUMBER_FORMAT);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::ExponentEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back(); // remove ExponentEvent
    if (type == TYPE_CONST_NUM) {
        if (AddAggregateSymbol(m_symbolStore.UpdateSymbolMap(token))) {
            ProcessNumber();
        }
    }
    else {
        ERROR(INVALID_NUMBER_FORMAT);
    }
    return(m_error == NO_ERROR);
}

bool Interpreter::PushIncludePath(string &path, ui32 &win32Error) {
    char buffer[4096], **lppPart = { NULL };
    win32Error = 0;
    if (GetFullPathNameA(path.c_str(), 4096, buffer, lppPart)) {
        pair<unordered_set<string>::iterator, bool> res = m_includedFiles.insert(buffer);
        return res.second;
    }
    win32Error = GetLastError();
    return false;
}

bool Interpreter::IncludeEvent(NODE_TYPE type, string &token) {
    // keep IncludeEvent on the stack as marker for IsValidEndState()
    m_stateEventStack.push_back(&Interpreter::ScopeEvent);
    if ((type == TYPE_CONST) && (token.length() > 2)){
        Reader *pReader = GetReader();
        ScriptReader reader(true, *this, m_error);
        string filePath = pReader->GetFilePath();
        ui32 pos = filePath.rfind('\\');
        std::replace(token.begin(), token.end(), '/', '\\');
        string path = filePath.substr(0, pos + 1) + token.substr(1, token.length() - 2);
        ui32 win32Error = 0;
        if (!PushIncludePath(path, win32Error)) {
            if (win32Error == S_OK) {
                SetReader(pReader);
                m_stateEventStack.pop_back(); // remove ScopEvent
                m_stateEventStack.pop_back(); // remove IncludeEvent
                return true;
            }
        }
        if ((win32Error != S_OK) || !reader.Read(path, win32Error)) {
//#if _DEBUG
            if (win32Error != S_OK) {
                LPSTR errorStr = 0;
                ui32 size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                           0, win32Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorStr, 0, NULL);

                m_error.SetError(CANNOT_OPEN_FILE, errorStr, __FILE__, __FUNCTION__, __LINE__);
                //Free the buffer.
                LocalFree(errorStr);
                m_error.SetErrorLocation(filePath, pReader->GetLine(), pReader->GetPosition());
            }
            else {
                if (!m_error.IsLocationSet()) {
                    m_error.SetErrorLocation(path, reader.GetLine(), reader.GetPosition());
                }
            }
            //static PrintF s_printf;
            //m_error.Print(&s_printf);
//#else // _DEBUG
//#endif // _DEBUG
            //ERROR(INCLUDE_ERROR);
        }
        SetReader(pReader);
        //return reader.Read(path);
    }
    else {
        ERROR(EXPECTING_SCRIPT_NAME);
    }

    if (m_error == NO_ERROR) {
        // reset in case of "if" waiting fo "else" on the stack
        while (m_stateEventStack.back() != &Interpreter::IncludeEvent) {
            m_stateEventStack.pop_back();
        }
        m_stateEventStack.pop_back(); // remove IncludeEvent

        m_pRunableCurrent = m_pRunableRoot; // reset to global scope
    }

    return (m_error == NO_ERROR);
}


bool Interpreter::LibDecEvent(NODE_TYPE type, string &token) {
     if ((TYPE_PARENTHESIS == type) && (token[0] == '(')) {
        if (Scope *pScope = m_pRunableCurrent->QueryType<Scope>(TYPE_SCOPE)) {
            m_stateEventStack.pop_back(); // remove LibDecEvent
            m_stateEventStack.push_back(&Interpreter::LibPathEvent);
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::LibPathEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_CONST:
            if (token.length() > 2) {
                string path = token.substr(1, token.length() - 2);
                if (Scope *pScope = m_pRunableCurrent->QueryType<Scope>(TYPE_SCOPE)) {
                    if (LibType *pLibType = CreateNode<LibType>(TYPE_LIB, token)) {
                        pLibType->SetPath(path);
                        pScope->PushExpression(pLibType);
                        m_pRunableCurrent = pLibType;
                    }
                }
                else {
                    ERROR(INTERNAL_ERROR);
                }
            }
            else {
                ERROR(EXPECTING_MODULE_NAME);
            }
            break;
        case TYPE_PARENTHESIS:
            if (token[0] == ')') {
                if (m_pRunableCurrent->QueryType<LibType>(TYPE_LIB)) {
                    m_stateEventStack.pop_back();//remove LibPathEvent
                    m_stateEventStack.push_back(&Interpreter::LibBeginBodyEvent);
                }
                else {
                    ERROR(SYNTAX_ERROR);
                }
            }
            else {
                ERROR(UNEXPECTED_CHARACTER);
            }
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }    
    return (m_error == NO_ERROR);
}

bool Interpreter::LibBeginBodyEvent(NODE_TYPE type, string &token) {
    if (TYPE_SCOPE == type) {
        if (token[0] == '{') {
            if (m_pRunableCurrent->QueryType<LibType>(TYPE_LIB)) {
                m_stateEventStack.pop_back();//remove LibPathEvent
                m_stateEventStack.push_back(&Interpreter::LibBodyEvent);
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
        }
        else {
            ERROR(SYNTAX_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

// function declaration format: 
// 1) RETURN_TYPE FUNCTION_NAME();
// 2) RETURN_TYPE FUNCTION_NAME( ARG_TYPE_0, ..., ARG_TYPE_N);
bool Interpreter::LibBodyEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SEMICOLON:
            if (m_pRunableCurrent->GetType() == TYPE_LIB) {
                ERROR(SYNTAX_ERROR);
            }
            else if (FunctionRefType *pFunctionRefType = m_pRunableCurrent->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
                string fooName;
                if (m_symbolStore.GetSymbolName(m_pRunableCurrent, fooName)) {
                    if (Variable *pVariable = CreateMemberDecNode(fooName)) {
                        pFunctionRefType->PushVariable(pVariable);
                        if (AggregateType *pParentAggregateType = m_pRunableCurrent->GetParent()->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
                            pParentAggregateType->AddType(pFunctionRefType);
                            m_pRunableCurrent = m_pRunableCurrent->GetParent();
                            if (m_pRunableCurrent->GetType() == TYPE_LIB) {
                                if (FunctionLibPtr *pFunctionLibPtr = CreateFunctionLibPtr(pVariable, fooName)) {
                                    pVariable->SetRight(pFunctionLibPtr);
                                }
                            }
                        }
                        else {
                            ERROR(INTERNAL_ERROR);
                        }
                    }
                }
                else {
                    ERROR(INTERNAL_ERROR);
                }
            }
            else {
                ERROR(INTERNAL_ERROR);
            }
            break;
        case TYPE_SCOPE:
            if (token[0] == '}') {
                if (AlignAggregateType()) {
#ifdef UNSAFE_STRING
                    Type *pType = m_pRunableCurrent->QueryType<Type>(TYPE_LIB);
                    vector<SYMBOL_DESC> symbolPath;
                    if (!pType->IsSafeType(m_error, symbolPath)) {
                        vector<SYMBOL_DESC>::reverse_iterator rit = symbolPath.rbegin();
                        for (; rit != symbolPath.rend(); ++rit) {
                            string err;
                            string name, file;
                            m_symbolStore.GetSymbolName((*rit).m_symId, name);
                            if (name[0] == '\"') {
                                name = name.substr(1, name.length() - 2);
                            }
                            m_symbolStore.GetFileName((*rit).m_fileId, file);
                            err += "        " + name +": \"" + file + "\"" +
                                   ", line: " + to_string((*rit).m_line) +
                                   ", pos: " + to_string((*rit).m_pos);
                            m_error.PusTrace(err);
                        }
                    }
                    else 
#endif // UNSAFE_STRING
                    {
                        m_stateEventStack.pop_back();
                        m_stateEventStack.push_back(&Interpreter::LibVarDecEvent);
                    }
                }
                else {
                    ERROR(INTERNAL_ERROR);
                }
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        default:
            m_stateEventStack.pop_back();
            m_stateEventStack.push_back(&Interpreter::LibFunctionNameEvent);
            return LibFunctionTypeEventHandler(type, token);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::LibFunctionTypeEventHandler(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_FUNCTION:
            switch (m_pRunableCurrent->GetType()) {
                case TYPE_LIB:
                    m_stateEventStack.push_back(&Interpreter::LibFunctionFunctionTypeEvent);
                    break;
                case TYPE_FUNCTION_REF_TYPE:
                    m_stateEventStack.push_back(&Interpreter::LibFunctionFunctionTypeEvent);
                    break;
                default:
                    ERROR(INTERNAL_ERROR);
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
            if (Type* pType = CreateNode<Type>(type, token)) {
                if (LibType *pLibType = m_pRunableCurrent->QueryType<LibType>(TYPE_LIB)) {
                    m_pRunableCurrent = pType;
                }
                else if (FunctionRefType *pFunctionRefType = m_pRunableCurrent->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
#ifdef SCRIPT_DEBUGGER
                    pFunctionRefType->PushArgDesc(LOCAL_VAR_DESCRIPTOR(SYMBOL_DESC(), pType));
#else // SCRIPT_DEBUGGER
                    pFunctionRefType->PushArgDesc(pType);
#endif // SCRIPT_DEBUGGER
                }
                else {
                    ERROR(INTERNAL_ERROR);
                }
            }
            break;
        case TYPE_ARRAY:
            if (ArrayType* pArrayType = CreateNode<ArrayType>(type, token)) {
                if (LibType *pLibType = m_pRunableCurrent->QueryType<LibType>(TYPE_LIB)) {
                    m_pRunableCurrent = pArrayType;
                }
                else if (FunctionRefType *pFunctionRefType = m_pRunableCurrent->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
#ifdef SCRIPT_DEBUGGER
                    pFunctionRefType->PushArgDesc(LOCAL_VAR_DESCRIPTOR(SYMBOL_DESC(), pArrayType));
#else // SCRIPT_DEBUGGER
                    Type *_pArrayType = pArrayType;
                    pFunctionRefType->PushArgDesc(_pArrayType);
#endif // SCRIPT_DEBUGGER
                    m_pRunableCurrent = pArrayType;
                }
                else {
                    ERROR(INTERNAL_ERROR);
                }
                m_stateEventStack.push_back(&Interpreter::ArrayBeginEvent);
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
            ERROR(EXPECTING_TYPE_NAME);
            break;
        case TYPE_TOKEN: {
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            if (AggregateType *pResolvedType = FindTypeDefinition(symId)) {
                if (AggregateType* pAggregateType = CreateNode<AggregateType>(TYPE_AGGREGATE_TYPE, token)) {
                    pAggregateType->SetTypeDefinition(pResolvedType);
                    if (LibType *pLibType = m_pRunableCurrent->QueryType<LibType>(TYPE_LIB)) {
                        m_pRunableCurrent = pAggregateType;
                    }
                    else if (FunctionRefType *pFunctionRefType = m_pRunableCurrent->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
#ifdef SCRIPT_DEBUGGER
                        pFunctionRefType->PushArgDesc(LOCAL_VAR_DESCRIPTOR(SYMBOL_DESC(), pAggregateType));
#else // SCRIPT_DEBUGGER
                        Type *_pAggregateType = pAggregateType;
                        pFunctionRefType->PushArgDesc(_pAggregateType);
#endif // SCRIPT_DEBUGGER
                    }
                    else {
                        ERROR(INTERNAL_ERROR);
                    }
                }
            }
            else {
                ERROR(UNDEFINED_TYPE);
            }
            break;
        }
        default:
            return DispatchEvent(type, token);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::LibFunctionFunctionTypeEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_LESS) {
        switch (m_pRunableCurrent->GetType()) {
            case TYPE_LIB:
            case TYPE_FUNCTION_REF_TYPE:
                m_stateEventStack.pop_back();
                m_stateEventStack.push_back(&Interpreter::LibFunctionBeginRefDecEvent);
                break;
            default:
                ERROR(INTERNAL_ERROR);
                break;
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::LibFunctionBeginRefDecEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_TOKEN) {
        // search for function definition
        SYMBOL_ID symId;
        if (m_symbolStore.GetSymbolId(token, symId)) {
            if (Function *pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                if (FunctionRefType *pFunctionRefType = CreateNode<FunctionRefType>(TYPE_FUNCTION_REF_TYPE, token)) {
                    pFunctionRefType->PopulateTypes(pFunction);
                    if (m_pRunableCurrent->GetType() == TYPE_LIB) {
                        m_pRunableCurrent = pFunctionRefType;
                    }
                    else {
                        if (FunctionRefType *pFunctionDec = m_pRunableCurrent->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
#ifdef SCRIPT_DEBUGGER
                            pFunctionDec->PushArgDesc(LOCAL_VAR_DESCRIPTOR(SYMBOL_DESC(),pFunctionRefType));
#else // SCRIPT_DEBUGGER
                            Type *_pFunctionRefType = pFunctionRefType;
                            pFunctionDec->PushArgDesc(_pFunctionRefType);
#endif // SCRIPT_DEBUGGER
                        }
                        else {
                            ERROR(INTERNAL_ERROR);
                            return false;
                        }
                    }
                    m_stateEventStack.pop_back();
                    m_stateEventStack.push_back(&Interpreter::LibFunctionEndRefDecEvent);
                    return true;
                }
                else {
                    return false;
                }
            }
        }
    }
    ERROR(EXPECTING_FUNCTION_REF_TYPE);
    return (m_error == NO_ERROR);
}

bool Interpreter::LibFunctionEndRefDecEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    if (type != TYPE_GREATER) {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::LibFunctionNameEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_REF) {
        if (m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) {
            if (Reference *pReference = CreateNode<Reference>(type, token)) {
                if (!m_pRunableCurrent->GetLeft()) {
                    m_pRunableCurrent->SetLeft(pReference);
                }
                else if (!m_pRunableCurrent->GetRight()) {
                    m_pRunableCurrent->SetRight(pReference);
                }
                else {
                    ERROR(SYNTAX_ERROR);
                }
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else if (Type *pType = m_pRunableCurrent->QueryType<Type>(TYPE_RUNABLE)) {
        if (ErrorIfDynamicArray(pType)) {
            return false;
        }
        // attach type node it to the function node
        m_pRunableCurrent = m_pRunableCurrent->GetParent();
        if (FunctionRefType *pFunctionRefType = CreateNode<FunctionRefType>(TYPE_FUNCTION_REF_TYPE, token)) {
            pFunctionRefType->SetReturnType(pType);
            pType->SetParent(pFunctionRefType);
            m_pRunableCurrent = pFunctionRefType;
            m_stateEventStack.pop_back();
            m_stateEventStack.push_back(&Interpreter::LibFunctionDecEvent);
            return true;
        }
        else {
            return false;
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::LibFunctionDecEvent(NODE_TYPE type, string &token) {
    if ((type == TYPE_PARENTHESIS) && (token[0] == '(')) {
        m_stateEventStack.pop_back();
        m_stateEventStack.push_back(&Interpreter::LibFunctionArgTypeEvent);
    }
    else {
        ERROR(EXPECTING_PARENTHESIS);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::LibFunctionArgTypeEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    m_stateEventStack.push_back(&Interpreter::LibFunctionNextArgTypeEvent);
    return LibFunctionTypeEventHandler(type, token);
}

bool Interpreter::LibFunctionNextArgTypeEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    if (FunctionRefType *pFunctionRefType = m_pRunableCurrent->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE)) {
        if (Type *pArgType = pFunctionRefType->GetArgType(pFunctionRefType->GetArgCount())) {
            switch (type) {
                case TYPE_REF:
                    if (Reference *pReference = CreateNode<Reference>(type, token)) {
                        if (!pArgType->GetLeft()) {
                            pArgType->SetLeft(pReference);
                        }
                        else if (!pArgType->GetRight()) {
                            pArgType->SetRight(pReference);
                        }
                        else {
                            delete pReference;
                            ERROR(SYNTAX_ERROR);
                        }
                    }
                    m_stateEventStack.push_back(&Interpreter::LibFunctionNextArgTypeEvent);
                    break;
                case TYPE_COMMA:
                    if (!ErrorIfDynamicArray(pArgType)) {
                        m_stateEventStack.push_back(&Interpreter::LibFunctionArgTypeEvent);
                    }
                    break;
                case TYPE_PARENTHESIS:
                    if (token[0] == ')') {
                        if (!ErrorIfDynamicArray(pArgType)) {
                            m_stateEventStack.push_back(&Interpreter::LibBodyEvent);
                        }
                    }
                    else {
                        ERROR(SYNTAX_ERROR);
                    }
                    break;
                default:
                    ERROR(SYNTAX_ERROR);
                    break;
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::LibVarDecEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    if (type == TYPE_TOKEN) {
        Type *pType = m_pRunableCurrent->QueryType<Type>(TYPE_LIB);
        Scope *pScope = m_pRunableCurrent->GetParent()->QueryType<Scope>(TYPE_SCOPE);
        if (pType && pScope) {
            if (Variable *pVariable = CreateVariableNode(m_pRunableCurrent->QueryType<Type>(TYPE_LIB), token)) {
                pType->PushVariable(pVariable);
                m_pRunableCurrent = pScope;
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::LibFunctionCallEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_PARENTHESIS) {
        m_stateEventStack.push_back(&Interpreter::FunctionCallArgEvent);
    }
    else {
        ERROR(EXPECTING_PARENTHESIS);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::MemberAccessEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_TOKEN) {
        m_stateEventStack.pop_back();//remove MemberAccessEvent
        switch (m_pRunableCurrent->GetType()) {
            case TYPE_LIB_VAR:
                ERROR(INTERNAL_ERROR);
                break;
            case TYPE_MEMBER_ACCESS: {
                Node *pParentNode = m_pRunableCurrent->GetParent();
                if (Variable *pDataMember = CreateMemberVariableNode(token)) {
                    m_pRunableCurrent->SetRight(pDataMember);
                    m_pRunableCurrent = pDataMember;
                    if (Node *pLeft = pParentNode->GetLeft()) {
                        pDataMember->SetLeft(pLeft);
                        pParentNode->SetLeft(0);
                        while (pLeft) {
                            pLeft->SetParent(pDataMember);
                            pLeft = pLeft->GetLeft();
                        }
                    }
                }
                break;
            }
            default:
                ERROR(INTERNAL_ERROR);
                break;
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

/******************** BEGIN expression handlers ********************/
bool Interpreter::BeginFullExpression(NODE_TYPE type, string &token) {
    NODE_TYPE curType = m_pRunableCurrent->GetType();
    Node *pNode = 0;
    switch (curType) {
        case TYPE_SCOPE:
            return BeginScopeExpression(type, token);
        case TYPE_FUNCTION:
            ERROR(NOT_IMPLEMENTED);
            break;
        case TYPE_IF:
            return BeginIfExpression(type, token);
        case TYPE_SWITCH:
            return BeginSwitchExpression(type, token);
        case TYPE_FOR:
            return BeginForExpression(type, token);
        case TYPE_WHILE:
            return BeginWhileExpression(type, token);
        case TYPE_FUNCTION_CALL:
            return BeginFunctionCallExpression(type, token);
        case TYPE_RETURN:
            return BeginReturnExpression(type, token);
        case TYPE_CASE:
        case TYPE_DEFAULT:
            //return BeginCaseDefaultExpression(type, token);
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    if (m_error == NO_ERROR) {
        m_pRunableCurrent = pNode;
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::BeginScopeExpression(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SEMICOLON:
            return true;
        case TYPE_SCOPE:
            if (token[0] == '}') {
                return EndScope();
            }
            m_stateEventStack.push_back(&Interpreter::ScopeEvent);
            break;
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
        case TYPE_CAST: {
            // next: FunctionCallEvent
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            if (Function *pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                if (Variable *pVariable = CreateVariableNode(0, token)) {
                    static_cast<Scope*>(m_pRunableCurrent)->PushExpression(pVariable);
                    m_pRunableCurrent = pVariable;
                    m_stateEventStack.push_back(&Interpreter::ExpEvent);
                    m_stateEventStack.push_back(&Interpreter::FunctionCallEvent);
                    if (type == TYPE_CAST) {
                        m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
                    }
                    return true;
                }
                else {
                    ERROR(OUT_OF_MEMORY);
                }
            }
            else {
                ERROR(UNDEFINED_SYMBOL);
            }
            break;
        }
        case TYPE_TOKEN: {
            SYMBOL_ID symId = m_symbolStore.UpdateSymbolMap(token);
            if (AggregateType *pResolvedType = FindTypeDefinition(symId)) {
                if (Type* pType = CreateNode<Type>(TYPE_AGGREGATE_TYPE, token)) {
                    static_cast<AggregateType*>(pType)->SetTypeDefinition(pResolvedType);
                    static_cast<Scope*>(m_pRunableCurrent)->PushExpression(pType);
                    m_pRunableCurrent = pType;
                    m_stateEventStack.push_back(&Interpreter::TypeDecEvent);
                    return true;
                }
            }
            else if (Variable *pVariable = m_pRunableCurrent->FindVariableDec(symId)) {
                Type *pType = pVariable->GetTypeNode();
                switch (pType->GetType()) {
                    case TYPE_FUNCTION_REF_TYPE:
                        m_stateEventStack.push_back(&Interpreter::ExpEvent);
                        m_stateEventStack.push_back(&Interpreter::FunctionCallEvent);
                        break;
                    case TYPE_ARRAY:
                        m_stateEventStack.push_back(&Interpreter::ExpEvent);
                        m_stateEventStack.push_back(&Interpreter::OpenSubscriptEvent);
                        break;
                    case TYPE_AGGREGATE_TYPE:
                    case TYPE_LIB:
                        m_stateEventStack.push_back(&Interpreter::ExpEvent);
                        m_stateEventStack.push_back(&Interpreter::BeginMemberEvent);//FunctionCallReturnAggregateEvent); // rename it
                        break;
                    default:
                        m_stateEventStack.push_back(&Interpreter::ExpEvent);
                        break;
                }
            }
            else if (Function *pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                if (Variable *pVariable = CreateVariableNode(0, token)) {
                    static_cast<Scope*>(m_pRunableCurrent)->PushExpression(pVariable);
                    m_pRunableCurrent = pVariable;
                    m_stateEventStack.push_back(&Interpreter::ExpEvent);
                    m_stateEventStack.push_back(&Interpreter::FunctionCallEvent);
                    return true;
                }
                else {
                    ERROR(OUT_OF_MEMORY);
                }
            }
            else {
                ERROR(UNDEFINED_SYMBOL);
            }
            break;
        }
#ifdef NUMBER
#else // NUMBER
        case TYPE_CONST:
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            break;
#endif // NUMBER
        case TYPE_FUNCTION:
            m_stateEventStack.push_back(&Interpreter::FunctionTypeEvent);
            return true;
        case TYPE_ARRAY:
            m_stateEventStack.push_back(&Interpreter::ArrayBeginEvent);
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
            m_stateEventStack.push_back(&Interpreter::TypeDecEvent);
            break;
        case TYPE_LIB:
            m_stateEventStack.push_back(&Interpreter::LibDecEvent);
            return true;
        case TYPE_STRUCT:
            m_stateEventStack.push_back(&Interpreter::AggregateVarDecEvent);
            m_stateEventStack.push_back(&Interpreter::AggregateEvent);
            return true;
        case TYPE_IF:
            m_stateEventStack.push_back(&Interpreter::IfEvent);
            break;
        case TYPE_FOR:
            m_stateEventStack.push_back(&Interpreter::ForEvent);
            break;
        case TYPE_WHILE:
            m_stateEventStack.push_back(&Interpreter::WhileEvent);
            break;
        case TYPE_SWITCH:
            m_stateEventStack.push_back(&Interpreter::SwitchEvent);
            break;
        case TYPE_RETURN:
            if (m_pRunableCurrent->GetFunctionNode()) {
                m_stateEventStack.push_back(&Interpreter::ReturnEvent);
            }
            else {
                ERROR(UNEXPECTED_RETURN);
            }
            break;
        case TYPE_CONTINUE:
            if (m_pRunableCurrent->GetLoopNode()) {
                m_stateEventStack.push_back(&Interpreter::ContinueEvent);
            }
            else {
                ERROR(ILLEGAL_CONTINUE);
            }
            break;
        case TYPE_BREAK:
            if (m_pRunableCurrent->GetBreakableNode()) {
                m_stateEventStack.push_back(&Interpreter::BreakEvent);
            }
            else {
                ERROR(ILLEGAL_BREAK);
            }
            break;
        case TYPE_CASE:
            ERROR(ILLEGAL_CASE);
            break;
        case TYPE_DEFAULT:
            ERROR(ILLEGAL_DEFAULT);
            break;
        case TYPE_INCLUDE:
            m_stateEventStack.push_back(&Interpreter::IncludeEvent);
            return true;
        case TYPE_ERROR:
            m_stateEventStack.push_back(&Interpreter::ErrorBeginEvent);
            break;
#ifdef NUMBER
        //case TYPE_ADD:
        //case TYPE_SUB:
        //    if (Node *pNode = CreateNode<Node>(TYPE_CONST, s_empty)) {
        //        m_stateEventStack.push_back(&Interpreter::NumberSignEvent);
        //        static_cast<Scope*>(m_pRunableCurrent)->PushExpression(pNode);
        //        m_pRunableCurrent = pNode;
        //        return DispatchEvent(type, token);
        //    }
        //    break;
#endif // NUMBER
        default:
            //if (Runable::IsOperator(type)) {
            //    m_stateEventStack.push_back(&Interpreter::ExpEvent);
            //}
            //else {
                ERROR(SYNTAX_ERROR);
            //}
            break;
    }
    if (m_error == NO_ERROR) {
        if (Node *pNode = CreateNode<Node>(type, token)) {
            static_cast<Scope*>(m_pRunableCurrent)->PushExpression(pNode);
            m_pRunableCurrent = pNode;
        }
        else {
            m_stateEventStack.pop_back();
        }
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::BeginIfExpression(NODE_TYPE type, string &token) {
    if (m_stateEventStack.back() == &Interpreter::IfBodyEvent) {
        if (m_pRunableCurrent->GetLeft()) { // if body already set, process
            if (TYPE_SEMICOLON == type) {
                m_stateEventStack.pop_back(); // remove IfBodyEvent
                m_stateEventStack.push_back(&Interpreter::ElseEvent);
                return true;
            }
            else {
                if (m_pRunableCurrent->GetLeft()->GetType() == TYPE_IF) {
                    m_pRunableCurrent = m_pRunableCurrent->GetParent();
                    m_stateEventStack.pop_back(); // remove IfBodyEvent
                    m_stateEventStack.pop_back(); // remove IfEvent
                    return DispatchEvent(type, token);
                }
                ERROR(EXPECTING_SEMICOLON);
                return false;
            }
        }
    }
    else { // it's ElseBodyEvent
        if (m_pRunableCurrent->GetRight()) {
            if (TYPE_SEMICOLON == type) {
                m_pRunableCurrent = m_pRunableCurrent->GetParent();
                m_stateEventStack.pop_back(); // remove ElseBodyEvent
                m_stateEventStack.pop_back(); // remove IfEvent
                return DispatchEvent(type, token);
            }
            else {
                ERROR(EXPECTING_SEMICOLON);
                return false;
            }
        }
    }

    switch (type) {
        case TYPE_SEMICOLON:
            if (m_stateEventStack.back() == &Interpreter::IfBodyEvent) {
                m_stateEventStack.pop_back(); // remove IfBodyEvent
                m_stateEventStack.push_back(&Interpreter::ElseEvent);
            }
            else { // it's ElseBodyEvent
                m_pRunableCurrent = m_pRunableCurrent->GetParent();
                m_stateEventStack.pop_back(); // remove ElseBodyEvent
                m_stateEventStack.pop_back(); // remove IfEvent
                return DispatchEvent(type, token);
            }
            return true;
        case TYPE_SCOPE:
            m_stateEventStack.push_back(&Interpreter::ScopeEvent);
            break;
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
            type = TYPE_TOKEN;
        case TYPE_TOKEN:
        case TYPE_CONST:
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            break;
        case TYPE_CAST:
            type = TYPE_TOKEN;
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
            break;
        case TYPE_IF:
            m_stateEventStack.push_back(&Interpreter::IfEvent);
            break;
        case TYPE_FOR:
            m_stateEventStack.push_back(&Interpreter::ForEvent);
            break;
        case TYPE_WHILE:
            m_stateEventStack.push_back(&Interpreter::WhileEvent);
            break;
        case TYPE_SWITCH:
            m_stateEventStack.push_back(&Interpreter::SwitchEvent);
            break;
        case TYPE_RETURN:
            if (m_pRunableCurrent->GetFunctionNode()) {
                m_stateEventStack.push_back(&Interpreter::ReturnEvent);
            }
            else {
                ERROR(UNEXPECTED_RETURN);
            }
            break;
        case TYPE_CONTINUE:
            if (m_pRunableCurrent->GetLoopNode()) {
                m_stateEventStack.push_back(&Interpreter::ContinueEvent);
            }
            else {
                ERROR(ILLEGAL_CONTINUE);
            }
            break;
        case TYPE_BREAK:
            if (m_pRunableCurrent->GetBreakableNode()) {
                m_stateEventStack.push_back(&Interpreter::BreakEvent);
            }
            else {
                ERROR(ILLEGAL_BREAK);
            }
            break;
        case TYPE_CASE:
            ERROR(ILLEGAL_CASE);
            break;
        case TYPE_DEFAULT:
            ERROR(ILLEGAL_DEFAULT);
            break;
        default:
            if (Runable::IsOperator(type)) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
    }
    if (m_error == NO_ERROR) {
        if (Node *pNode = CreateNode<Node>(type, token)) {
            static_cast<IfNode*>(m_pRunableCurrent)->PushExpression(pNode);
            m_pRunableCurrent = pNode;
        }
        else {
            m_stateEventStack.pop_back();
        }
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::BeginSwitchExpression(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SEMICOLON:
            if (m_stateEventStack.back() == &Interpreter::IfBodyEvent) {
                m_stateEventStack.pop_back(); // remove IfBodyEvent
                m_stateEventStack.push_back(&Interpreter::ElseEvent);
            }
            else { // it's ElseBodyEvent
                m_pRunableCurrent = m_pRunableCurrent->GetParent();
                m_stateEventStack.pop_back(); // remove ElseBodyEvent
                m_stateEventStack.pop_back(); // remove IfEvent
                return DispatchEvent(type, token);
            }
            return true;
        case TYPE_SCOPE:
            m_stateEventStack.push_back(&Interpreter::ScopeEvent);
            break;
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
            type = TYPE_TOKEN;
        case TYPE_TOKEN:
        case TYPE_CONST:
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            break;
        case TYPE_CAST:
            type = TYPE_TOKEN;
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
            break;
        case TYPE_IF:
            m_stateEventStack.push_back(&Interpreter::IfEvent);
            break;
        case TYPE_FOR:
            m_stateEventStack.push_back(&Interpreter::ForEvent);
            break;
        case TYPE_WHILE:
            m_stateEventStack.push_back(&Interpreter::WhileEvent);
            break;
        case TYPE_SWITCH:
            m_stateEventStack.push_back(&Interpreter::SwitchEvent);
            break;
        case TYPE_RETURN:
            if (m_pRunableCurrent->GetFunctionNode()) {
                m_stateEventStack.push_back(&Interpreter::ReturnEvent);
            }
            else {
                ERROR(UNEXPECTED_RETURN);
            }
            break;
        case TYPE_CONTINUE:
            if (m_pRunableCurrent->GetLoopNode()) {
                m_stateEventStack.push_back(&Interpreter::ContinueEvent);
            }
            else {
                ERROR(ILLEGAL_CONTINUE);
            }
            break;
        case TYPE_BREAK:
            if (Node *pExpNode = m_pRunableCurrent->GetExpressionRoot()) {
            }
            else {
                m_stateEventStack.pop_back(); // remove CaseBodyEvent
                m_stateEventStack.pop_back(); // remove CaseEvent
            }
            break;
        case TYPE_CASE:
            ERROR(ILLEGAL_CASE);
            break;
        case TYPE_DEFAULT:
            ERROR(ILLEGAL_DEFAULT);
            break;
        default:
            if (Runable::IsOperator(type)) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
    }
    if (m_error == NO_ERROR) {
        if (Node *pNode = CreateNode<Node>(type, token)) {
            static_cast<SwitchNode*>(m_pRunableCurrent)->UpdateCaseExp(pNode);
            m_pRunableCurrent = pNode;
        }
        else {
            m_stateEventStack.pop_back();
        }
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::BeginForExpression(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SEMICOLON:
            m_pRunableCurrent = m_pRunableCurrent->GetParent();
            m_stateEventStack.pop_back();
            m_stateEventStack.pop_back();
            return DispatchEvent(type, token);
        case TYPE_SCOPE:
            m_stateEventStack.push_back(&Interpreter::ScopeEvent);
            break;
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
            type = TYPE_TOKEN;
        case TYPE_TOKEN:
        case TYPE_CONST:
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            break;
        case TYPE_CAST:
            type = TYPE_TOKEN;
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
            break;
        case TYPE_IF:
            m_stateEventStack.push_back(&Interpreter::IfEvent);
            break;
        case TYPE_FOR:
            m_stateEventStack.push_back(&Interpreter::ForEvent);
            break;
        case TYPE_WHILE:
            m_stateEventStack.push_back(&Interpreter::WhileEvent);
            break;
        case TYPE_SWITCH:
            m_stateEventStack.push_back(&Interpreter::SwitchEvent);
            break;
        case TYPE_RETURN:
            if (m_pRunableCurrent->GetFunctionNode()) {
                m_stateEventStack.push_back(&Interpreter::ReturnEvent);
            }
            else {
                ERROR(UNEXPECTED_RETURN);
            }
            break;
        case TYPE_CONTINUE:
            m_stateEventStack.push_back(&Interpreter::ContinueEvent);
            break;
        case TYPE_BREAK:
            m_stateEventStack.push_back(&Interpreter::BreakEvent);
            break;
        case TYPE_CASE:
            ERROR(ILLEGAL_CASE);
            break;
        case TYPE_DEFAULT:
            ERROR(ILLEGAL_DEFAULT);
            break;
        default:
            if (Runable::IsOperator(type)) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
    }
    if (m_error == NO_ERROR) {
        if (Node *pNode = CreateNode<Node>(type, token)) {
            static_cast<ForNode*>(m_pRunableCurrent)->UpdateExpression(pNode);
            m_pRunableCurrent = pNode;
        }
        else {
            m_stateEventStack.pop_back();
        }
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::BeginWhileExpression(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SEMICOLON:
            m_pRunableCurrent = m_pRunableCurrent->GetParent();
            m_stateEventStack.pop_back();
            m_stateEventStack.pop_back();
            return DispatchEvent(type, token);
        case TYPE_SCOPE:
            m_stateEventStack.push_back(&Interpreter::ScopeEvent);
            break;
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
            type = TYPE_TOKEN;
        case TYPE_TOKEN:
        case TYPE_CONST:
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            break;
        case TYPE_CAST:
            type = TYPE_TOKEN;
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
            break;
        case TYPE_IF:
            m_stateEventStack.push_back(&Interpreter::IfEvent);
            break;
        case TYPE_FOR:
            m_stateEventStack.push_back(&Interpreter::ForEvent);
            break;
        case TYPE_WHILE:
            m_stateEventStack.push_back(&Interpreter::WhileEvent);
            break;
        case TYPE_SWITCH:
            m_stateEventStack.push_back(&Interpreter::SwitchEvent);
            break;
        case TYPE_RETURN:
            if (m_pRunableCurrent->GetFunctionNode()) {
                m_stateEventStack.push_back(&Interpreter::ReturnEvent);
            }
            else {
                ERROR(UNEXPECTED_RETURN);
            }
            break;
        case TYPE_CONTINUE:
            m_stateEventStack.push_back(&Interpreter::ContinueEvent);
            break;
        case TYPE_BREAK:
            m_stateEventStack.push_back(&Interpreter::BreakEvent);
            break;
        case TYPE_CASE:
            ERROR(ILLEGAL_CASE);
            break;
        case TYPE_DEFAULT:
            ERROR(ILLEGAL_DEFAULT);
            break;
        default:
            if (Runable::IsOperator(type)) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
    }
    if (m_error == NO_ERROR) {
        if (Node* pNode = CreateNode<Node>(type, token)) {
            static_cast<WhileNode*>(m_pRunableCurrent)->PushExpression(pNode);
            m_pRunableCurrent = pNode;
        }
        else {
            m_stateEventStack.pop_back();
        }
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::BeginFunctionCallExpression(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
            type = TYPE_TOKEN;
        case TYPE_CONST_BOOL:
        case TYPE_TOKEN:
        case TYPE_CONST:
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            break;
        case TYPE_CAST:
            type = TYPE_TOKEN;
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
            break;
        case TYPE_PARENTHESIS:
            if (token[0] == '(') {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                return DispatchEvent(type, token);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        case TYPE_IF:
        case TYPE_FOR:
        case TYPE_WHILE:
        case TYPE_SWITCH:
        case TYPE_RETURN:
        case TYPE_BREAK:
        case TYPE_CASE:
        case TYPE_DEFAULT:
            ERROR(SYNTAX_ERROR);
            break;
        default:
            if (Runable::IsOperator(type)) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
    }
    if (m_error == NO_ERROR) {
        if (Node *pNode = CreateNode<Node>(type, token)) {
            static_cast<FunctionCall*>(m_pRunableCurrent)->PushArgument(pNode);
            m_pRunableCurrent = pNode;
        }
        else {
            m_stateEventStack.pop_back();
        }
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::BeginReturnExpression(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_SEMICOLON:
            ERROR(NOT_IMPLEMENTED);
            break;
        case TYPE_SIZEOF:
        case TYPE_ARR2STR:
        case TYPE_WARR2STR:
        case TYPE_STR2ARR:
        case TYPE_STR2WARR:
        case TYPE_LOCK:
        case TYPE_UNLOCK:
            type = TYPE_TOKEN;
        case TYPE_CONST_BOOL:
        case TYPE_TOKEN:
        case TYPE_CONST:
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            break;
        case TYPE_CAST:
            type = TYPE_TOKEN;
            m_stateEventStack.push_back(&Interpreter::ExpEvent);
            m_stateEventStack.push_back(&Interpreter::CastOpenTypeExpression);
            break;
        case TYPE_PARENTHESIS:
            if (token[0] == '(') {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
                return DispatchEvent(type, token);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
        case TYPE_IF:
        case TYPE_FOR:
        case TYPE_WHILE:
        case TYPE_SWITCH:
        case TYPE_RETURN:
        case TYPE_BREAK:
        case TYPE_CASE:
        case TYPE_DEFAULT:
            ERROR(SYNTAX_ERROR);
            break;
        default:
            if (Runable::IsOperator(type)) {
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
            break;
    }
    if (m_error == NO_ERROR) {
        if (Node *pNode = CreateNode<Node>(type, token)) {
            static_cast<Return*>(m_pRunableCurrent)->SetLeft(pNode);
            m_pRunableCurrent = pNode;
        }
        else {
            m_stateEventStack.pop_back();
        }
    }

    return (m_error == NO_ERROR);
}

// cast event handlers
bool Interpreter::CastOpenTypeExpression(NODE_TYPE type, string &token) { // <
    m_stateEventStack.pop_back(); // remove CastOpenTypeExpression
    if (type == TYPE_LESS) {
        m_stateEventStack.push_back(&Interpreter::CastCloseTypeExpression);
        m_stateEventStack.push_back(&Interpreter::CastBeginTypeExpression);
    }
    else {
        ERROR(SYNTAX_ERROR);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::CastBeginTypeExpression(NODE_TYPE type, string &token) { // expectiong type
    Variable *pVariable = m_pRunableCurrent->QueryType<Variable>(TYPE_VAR);
    assert(pVariable);
    FunctionRefType *pFunctionRefType = pVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
    m_stateEventStack.pop_back(); // remove CastBeginTypeExpression
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
            if (Type *pType = CreateNode<Type>(type, token)) {
                pFunctionRefType->SetReturnType(pType);
            }
            break;
        case TYPE_ARRAY:
            if (ArrayType* pArrayType = CreateNode<ArrayType>(type, token)) {
                pFunctionRefType->SetReturnType(pArrayType);
                m_stateEventStack.push_back(&Interpreter::ArrayBeginEvent);
                m_pRunableCurrent = pArrayType;
            }
            break;
        case TYPE_FUNCTION:
            m_stateEventStack.push_back(&Interpreter::CastBeginFunctionTypeEvent);
            break;
        case TYPE_TOKEN:
            if (AggregateType *pResolvedType = FindTypeDefinition(m_symbolStore.UpdateSymbolMap(token))) {
                if (AggregateType* pAggregateType = CreateNode<AggregateType>(TYPE_AGGREGATE_TYPE, token)) {
                    pAggregateType->SetTypeDefinition(pResolvedType);
                    pFunctionRefType->SetReturnType(pAggregateType);
                }
            }
            else {
                ERROR(UNDEFINED_TYPE);
            }
            break;
        default:
            ERROR(EXPECTING_TYPE_NAME);
            break;
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::CastBeginFunctionTypeEvent(NODE_TYPE type, string &token) {
    m_stateEventStack.pop_back();
    switch (type) {
        case TYPE_LESS:
            m_stateEventStack.push_back(&Interpreter::CastFunctionTypeEvent);
            break;
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::CastFunctionTypeEvent(NODE_TYPE type, string &token) {
    if (type == TYPE_TOKEN) {
        // search for function definition
        SYMBOL_ID symId;
        if (m_symbolStore.GetSymbolId(token, symId)) {
            if (Function *pFunction = m_pRunableRoot->ResolveFunction(symId)) {
                if (FunctionRefType *pFunctionRefType = CreateNode<FunctionRefType>(TYPE_FUNCTION_REF_TYPE, token)) {
                    pFunctionRefType->PopulateTypes(pFunction);
                    Variable *pVariable = m_pRunableCurrent->QueryType<Variable>(TYPE_VAR);
                    assert(pVariable);
                    FunctionRefType *pCastFunctionRefType = pVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
                    pCastFunctionRefType->SetReturnType(pFunctionRefType);

                    m_stateEventStack.pop_back();
                    m_stateEventStack.push_back(&Interpreter::CastEndFunctionTypeEvent);
                }
            }
            else {
                ERROR(UNDEFINED_FUNCTION);
            }
        }
        else {
            ERROR(UNDEFINED_FUNCTION);
        }
    }
    else {
        ERROR(EXPECTING_FUNCTION_REF_TYPE);
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::CastEndFunctionTypeEvent(NODE_TYPE type, string &token) {
    switch (type) {
        case TYPE_GREATER:
            m_stateEventStack.pop_back();
            break;
        case TYPE_SHIFT_RIGHT:
            m_stateEventStack.pop_back();
            return DispatchEvent(TYPE_GREATER, string(">"));
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::CastCloseTypeExpression(NODE_TYPE type, string &token) { // >
    switch (type) {
        case TYPE_GREATER:
            m_stateEventStack.pop_back(); // remove CastCloseTypeExpression
            break;
        case TYPE_SHIFT_RIGHT:
            m_stateEventStack.pop_back();
            return DispatchEvent(TYPE_GREATER, string(">"));
        default:
            ERROR(SYNTAX_ERROR);
            break;
    }

    return (m_error == NO_ERROR);
}

/******************** END expression handlers ********************/
//
bool Interpreter::ProcessNumber() {
    bool good = true;
    if (m_pRunableCurrent->GetType() != TYPE_SWITCH) {
        if (ConstVariable *pConstVariable = m_pRunableCurrent->QueryType<ConstVariable>(TYPE_CONST)) {
            string value;
            pConstVariable->GetString(value);

            Value::VALUE_TYPE valueType = Value::NULL_TYPE;
            string res;

            if ((value[0] == '0') && (value[1] == 'x' || value[1] == 'X')) { // hex number
                if ((value.length() < 3) || (value.length() > 18)) {
                    ERROR(INVALID_HEX_NUMBER);
                    return false;
                }
                if ((value.length() > 18) || !GetHex1Number(value, res)) {
                    ERROR(INVALID_HEX_NUMBER);
                    return false;
                }
                if (value.length() <= 4) {
                    valueType = Value::UI8_TYPE;
                }
                else if (value.length() <= 6) {
                    valueType = Value::UI16_TYPE;
                }
                else if (value.length() <= 10) {
                    valueType = Value::UI32_TYPE;
                }
                else if (value.length() <= 18) {
                    valueType = Value::UI64_TYPE;
                }
                value = res;
            }
            else {
                //check last char
                char id = value[value.length() - 1];
                switch (id) {
                    case 'h':
                    case 'H':
                        if ((value.length() > 17) || !GetHex2Number(value, res)) {
                            ERROR(INVALID_HEX_NUMBER);
                            return false;
                        }
                        if (value.length() <= 3) {
                            valueType = Value::UI8_TYPE;
                        }
                        else if (value.length() <= 5) {
                            valueType = Value::UI16_TYPE;
                        }
                        else if (value.length() <= 9) {
                            valueType = Value::UI32_TYPE;
                        }
                        else if (value.length() <= 17) {
                            valueType = Value::UI64_TYPE;
                        }
                        value = res;
                        break;
                    case 'b':
                    case 'B':
                        if ((value.length() > 65) || !GetBinNumber(value, res)) {
                            ERROR(INVALID_BINARY_NUMBER);
                            return false;
                        }
                        if (value.length() <= 9) {
                            valueType = Value::UI8_TYPE;
                        }
                        else if (value.length() <= 17) {
                            valueType = Value::UI16_TYPE;
                        }
                        else if (value.length() <= 33) {
                            valueType = Value::UI32_TYPE;
                        }
                        else if (value.length() <= 65) {
                            valueType = Value::UI64_TYPE;
                        }
                        value = res;
                        break;
                    case 'e':
                    case 'E':
                        ERROR(INVALID_NUMBER_FORMAT);
                        return false;
                    default: {
                        FLOAT_NUM fn;
                        if (valueType == Value::NULL_TYPE) {
                            if (!fn.Set(value)) {
                                ERROR(INVALID_NUMBER_FORMAT);
                                return false;
                            }
                            valueType = fn.GetType();
                        }
                        break;
                    }
                }
            }

            NODE_TYPE nodeType = Type::ValueTypeToNodeType(valueType);
            Type *pType = 0;
            pType = Scope::GetDummyTypeNode(nodeType);
            //pConstVariable->SetSymbolId(m_symbolStore.UpdateSymbolMap(value));
            pConstVariable->SetTypeNode(pType);
            if (!pConstVariable->GetRef().Set(value, valueType, pType)) {
                ERROR(INTERNAL_ERROR);
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    return (m_error == NO_ERROR);
}

bool Interpreter::FLOAT_NUM::Set(string &value) {
    enum {
        SIGN,
        BEGIN_NUMBER,
        NUMBER_BODY,
        BEGIN_FRACTION,
        FRACTION,
        EXP_SIGN,
        BEGIN_EXP,
        EXP,
        ERROR
    } state;
    Clear();
    state = SIGN;
    for (ui32 i = 0; i < value.length(); ++i) {
        switch (state) {
            case SIGN:
                switch (value[i]) {
                    case '-':
                        sign = NEGATIVE;
                    case '+':
                        state = BEGIN_NUMBER;
                        continue;
                    default:
                        break;
                }
            case BEGIN_NUMBER:
                switch (value[i]) {
                    case '.':
                        state = BEGIN_FRACTION;
                        continue;
                    default:
                        if (IsNum(value[i])) {
                            number = value[i] - '0';
                            state = NUMBER_BODY;
                            continue;
                        }
                        break;
                }
                state = ERROR;
                break;
            case NUMBER_BODY:
                switch (value[i]) {
                    case 'e':
                    case 'E':
                        state = EXP_SIGN;
                        continue;
                    case '.':
                        state = BEGIN_FRACTION;
                        continue;
                    default:
                        if (IsNum(value[i])) {
                            number = (number * 10) + (value[i] - '0');
                        }
                        break;
                }
                break;
            case BEGIN_FRACTION:
                if (value[i] == '0') {
                    ++fractionForwardZeros;
                    continue;
                }
                else if (IsNum(value[i])) {
                    state = FRACTION;
                }
            case FRACTION:
                switch (value[i]) {
                    case 'e':
                    case 'E':
                        state = EXP_SIGN;
                        continue;
                    default:
                        if (IsNum(value[i])) {
                            fraction = (fraction * 10) + (value[i] - '0');
                        }
                        break;
                }
                break;
            case EXP_SIGN:
                switch (value[i]) {
                    case '-':
                        sign = (sign == NEGATIVE) ? NEGATIVE_NEGATIVE_EXP : NEGATIVE_POSITIVE_EXP;
                        state = BEGIN_EXP;
                        continue;
                    case '+':
                        sign = (sign == NEGATIVE) ? NEGATIVE_POSITIVE_EXP : NEGATIVE_NEGATIVE_EXP;
                        state = BEGIN_EXP;
                        continue;
                    default:
                        break;
                }
                state = ERROR;
                break;
            case BEGIN_EXP:
                if (value[i] == '0') {
                    continue;
                }
                exponent = 0;
            case EXP:
                if (IsNum(value[i])) {
                    exponent = (exponent * 10) + (value[i] - '0');
                    state = EXP;
                }
                else {
                    state = ERROR;
                }
                break;
            default:
                return false;
        }
    }
    while (fraction) {
        if (!(fraction % 10)) {
            fraction = fraction / 10;
        }
        else {
            break;
        }
    }
    return (state != ERROR);
}

Value::VALUE_TYPE Interpreter::FLOAT_NUM::GetType() {
    // valid number format (±)(N)(.)(N)(e±N)
    //  SIGN, NUMBER_BODY, DECIMAL_POINT, FRACTION, EXPONENT_IDENT, EXPONENT_SIGN, EXPONENT
    //    ±     N           .           N            e                ±           N  
    /*
    Size	           | Range	                            | Precision
    -------------------+------------------------------------+----------------------------------------
    4 bytes	           | ±1.18 x 10^-38 to ±3.4 x 10^38	    | 6-9 significant digits, typically 7
    8 bytes	           | ±2.23 x 10^-308 to ±1.80 x 10^308	| 15-18 significant digits, typically 16
    80-bits (12 bytes) | ±3.36 x 10^-4932 to ±1.18 x 10^4932| 18-21 significant digits
    16 bytes	       | ±3.36 x 10^-4932 to ±1.18 x 10^4932| 33-36 significant digits

    Name	 |Base|Significand   |Decimal |Exponent|Decimal|Exponent  |E min|E max|Notes
    |    |Bits[a]/Digits|digits  |bits    |E max  |bias      |     |     |
    ---------+----+--------------+--------+--------+-------+----------+-----+-----+---------
    binary32 |2   |24            |7.22    |8       |38.23  |27-1=127  |-126 |+127 |Single precision
    binary64 |2   |53            |15.95   |11      |307.95 |210-1=1023|-1022|+1023|Double precision
    decimal32|10  |7             |7	      |7.58	   |96	   |101       |-95  |+96  |not basic
    decimal64|10  |16            |16      |9.58	   |384	   |398       |-383 |+384 |
    */
    Value::VALUE_TYPE type = Value::NUM_TYPE_FLAG;
    if (exponent == -1) {
        if (fraction == 0) { // this is an integer number
            if (sign & NEGATIVE) {
                if (number <= 0x7f) { // byte
                    type = Value::I8_TYPE;
                }
                else if (number <= 0x7fff) { // short
                    type = Value::I16_TYPE;
                }
                else if (number <= 0x7fffffff) { // long
                    type = Value::I32_TYPE;
                }
                else { // long long
                    type = Value::I64_TYPE;
                }
            }
            else {
                if (number <= 0xff) { // byte
                    type = Value::UI8_TYPE;
                }
                else if (number <= 0xffff) { // short
                    type = Value::UI16_TYPE;
                }
                else if (number <= 0xffffffff) { // long
                    type = Value::UI32_TYPE;
                }
                else { // long long
                    type = Value::UI64_TYPE;
                }
            }
        }
        else { // double
            type = Value::DOUBLE_TYPE;
        }
    }
    else { // double
        type = Value::DOUBLE_TYPE;
    }

    return type;
}

void Interpreter::TestNum(string &val) {
    FLOAT_NUM num;
    char *type;
    num.Set(val);
    switch (num.GetType()) {
        case Value::I8_TYPE:
            type = "I8";
            break;
        case Value::UI8_TYPE:
            type = "UI8";
            break;
        case Value::I16_TYPE:
            type = "I16";
            break;
        case Value::UI16_TYPE:
            type = "UI16";
            break;
        case Value::I32_TYPE:
            type = "I32";
            break;
        case Value::UI32_TYPE:
            type = "UI32";
            break;
        case Value::I64_TYPE:
            type = "I32";
            break;
        case Value::UI64_TYPE:
            type = "UI64";
            break;
        case Value::FLOAT_TYPE:
            type = "FLOAT";
            break;
        case Value::DOUBLE_TYPE:
            type = "DOUBLE";
            break;
        default:
            type = "ERROR";
            break;
    }
    printf("%s: %s\n", val.c_str(), type);
}

bool Interpreter::FLOAT_NUM::GetValue(Value &value) {
    bool good = true;

    switch (GetType()) {
        case Value::I8_TYPE:
            value.Set(static_cast<i8>(number));
            break;
        case Value::UI8_TYPE:
            value.Set(static_cast<ui8>(number));
            break;
        case Value::I16_TYPE:
            value.Set(static_cast<i16>(number));
            break;
        case Value::UI16_TYPE:
            value.Set(static_cast<ui16>(number));
            break;
        case Value::I32_TYPE:
            value.Set(static_cast<i32>(number));
            break;
        case Value::UI32_TYPE:
            value.Set(static_cast<ui32>(number));
            break;
        case Value::I64_TYPE:
            value.Set(static_cast<i64>(number));
            break;
        case Value::UI64_TYPE:
            value.Set(static_cast<ui64>(number));
            break;
        case Value::FLOAT_TYPE:
            break;
        case Value::DOUBLE_TYPE:
            break;
        default:
            //type = "ERROR";
            break;
    }
    if (exponent == -1) { // no exponent
        if (fraction == 0) { // this is an integer number
            if (sign & NEGATIVE) {
                if (number <= 0x7f) { // byte
                    value.Set(static_cast<i8>(number));
                }
                else if (number <= 0x7fff) { // short
                    value.Set(static_cast<i16>(number));
                }
                else if (number <= 0x7fffffff) { // long
                    value.Set(static_cast<i32>(number));
                }
                else { // long long
                    value.Set(static_cast<i64>(number));
                }
            }
            else {
                if (number <= 0xff) { // byte
                    value.Set(static_cast<ui8>(number));
                }
                else if (number <= 0xffff) { // short
                    value.Set(static_cast<ui16>(number));
                }
                else if (number <= 0xffffffff) { // long
                    value.Set(static_cast<ui32>(number));
                }
                else { // long long
                    value.Set(static_cast<ui64>(number));
                }
            }
        }
        else { // float or double value
            string str, temp;
            str.push_back((sign & NEGATIVE) ? '-' : '+');
            while(number) {
                temp.push_back((number % 10) + 0x30);
                number = number / 10;
            }
            for (i32 i = temp.length() - 1; i >= 0; --i) {
                str.push_back(temp[i]);
            }
            temp.clear();
            str.push_back('.');
            while (fractionForwardZeros) {
                str.push_back('0');
                --fractionForwardZeros;
            }
            while (fraction) {
                temp.push_back((fraction % 10) + 0x30);
                fraction = fraction / 10;
            }
            for (i32 i = temp.length() - 1; i >= 0; --i) {
                str.push_back(temp[i]);
            }
            str.push_back('e');
            str.push_back((sign & POSITIVE_EXP) ? '+' : '-');
            while (fraction) {
                temp.push_back((fraction % 10) + 0x30);
                fraction = fraction / 10;
            }
            for (i32 i = temp.length() - 1; i >= 0; --i) {
                str.push_back(temp[i]);
            }
        }
    }
    else {

    }

    return good;
}

float Interpreter::FLOAT_NUM::GetFloat() {
    return 0;
}

double Interpreter::FLOAT_NUM::GetDouble() {
    return 0;
}

bool Interpreter::EndScope() {

    Node* pParent = m_pRunableCurrent->GetParent();
    m_stateEventStack.pop_back(); // remove ScopeEvent
    if (pParent) {
        NODE_TYPE type = pParent->GetType();

        switch (type) {
            case TYPE_FUNCTION:
            case TYPE_SWITCH:
                m_stateEventStack.pop_back(); // remove FunctionEvent or SwitchEvent
                m_pRunableCurrent = pParent->GetParent();
                break;
            case TYPE_FOR:
            case TYPE_WHILE: {
                m_pRunableCurrent = pParent;
                return DispatchEvent(TYPE_SEMICOLON, s_semicolon);
            }
            case TYPE_SCOPE:
                m_pRunableCurrent = pParent;
                break;
            case TYPE_IF:
                if (IfNode* pIfNode = pParent->QueryType<IfNode>(TYPE_IF)) {
                    m_pRunableCurrent = pParent;
                    return DispatchEvent(TYPE_SEMICOLON, s_semicolon);
                }
                else {
                    ERROR(INTERNAL_ERROR);
                }
                break;
            case TYPE_CASE:
            case TYPE_DEFAULT:
                assert(m_pRunableCurrent->GetType() == TYPE_SCOPE);
                assert(m_pRunableCurrent->GetSymbolId() == SYMBOL_ID_BLANK);// this is end of the switch
                m_pRunableCurrent = pParent;
                break;
            case TYPE_ERROR:
                m_pRunableCurrent = pParent->GetParent();
                break;
            default:
                ERROR(INTERNAL_ERROR);
                break;
        }
    }
    else {
        ERROR(UNEXPECTED_END_OF_SCOPE);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::OpenParenthesis(string &token) {
    assert(token[0] == '(');

    NODE_TYPE type = m_pRunableCurrent->GetType();

    switch (type) {
        case TYPE_SIZEOF:
            ERROR(INTERNAL_ERROR);
            break;
        case TYPE_TOKEN:
            m_stateEventStack.push_back(&Interpreter::FunctionCallEvent);
            return DispatchEvent(TYPE_PARENTHESIS, token);

        case TYPE_SCOPE:
            if (Parenthesis *pParenthesis = CreateNode<Parenthesis>(token)) {
                Scope *pScope = static_cast<Scope*>(m_pRunableCurrent);
                pScope->PushExpression(pParenthesis);
                m_pRunableCurrent = pParenthesis;
                m_stateEventStack.push_back(&Interpreter::ExpEvent);
            }
            else {
                ERROR(OUT_OF_MEMORY);
            }
            break;
        case TYPE_IF:
        case TYPE_SWITCH:
        case TYPE_WHILE:
            //case TYPE_FOR:
            if (Parenthesis *pParenthesis = CreateNode<Parenthesis>(token)) {
                ConditionalNode *pConditionalNode = m_pRunableCurrent->QueryType<ConditionalNode>(TYPE_CONDITIONAL);
                pConditionalNode->SetCondition(pParenthesis);
                m_pRunableCurrent = pParenthesis;
            }
            else {
                ERROR(OUT_OF_MEMORY);
            }
            break;
        case TYPE_FUNCTION:
            if (token[0] == '(') {
                m_stateEventStack.pop_back();//remove FunctionDecEvent
                m_stateEventStack.push_back(&Interpreter::FunctionArgTypeEvent);
            }
            else {
                ERROR(UNEXPECTED_CHARACTER);
            }
            break;
        case TYPE_FUNCTION_CALL:
            ERROR(SYNTAX_ERROR);
            //TODO("check function return type before creating Parenthesis");
            //if (Parenthesis *pParenthesis = CreateNode<Parenthesis>(token)) {
            //    FunctionCall *pFunctionCall = static_cast<FunctionCall*>(m_pRunableCurrent);
            //    pFunctionCall->PushArgument(pParenthesis);
            //    m_pRunableCurrent = pParenthesis;
            //}
            //else {
            //    ERROR(OUT_OF_MEMORY);
            //}
            break;
        case TYPE_PARENTHESIS:
            if (m_pRunableCurrent->GetSymbolId() == SYMBOL_ID_OPENPARENTHESIS) {
                if (Parenthesis *pParenthesis = CreateNode<Parenthesis>(token)) {
                    m_pRunableCurrent->SetRight(pParenthesis);
                    m_pRunableCurrent = pParenthesis;
                }
                else {
                    ERROR(OUT_OF_MEMORY);
                }
            }
            else {
                // is it a function call?
                ERROR(SYNTAX_ERROR);
            }
            break;
        case TYPE_LIB:
            ERROR(UNEXPECTED_CHARACTER);
            break;
        case TYPE_PRE_INC_OP:
        case TYPE_PRE_DEC_OP:
            ERROR(EXPECTING_VARIABLE);
            break;
        case TYPE_SUBSCRIPT:
            if (Parenthesis *pParenthesis = CreateNode<Parenthesis>(token)) {
                SubscriptNode *pSubscriptNode = static_cast<SubscriptNode*>(m_pRunableCurrent);
                pSubscriptNode->SetExpression(pParenthesis);
                m_pRunableCurrent = pParenthesis;
            }
            else {
                ERROR(OUT_OF_MEMORY);
            }
            break;
        case TYPE_ARRAY:
            if (Parenthesis *pParenthesis = CreateNode<Parenthesis>(token)) {
                ArrayType *pArrayType = static_cast<ArrayType*>(m_pRunableCurrent);
                pArrayType->SetDynExpression(pParenthesis);
                m_pRunableCurrent = pParenthesis;
            }
            else {
                ERROR(OUT_OF_MEMORY);
            }
            break;
        default:
            if (Runable::IsOperator(type) || (TYPE_RETURN == type)) {
                //      OP 
                //       \
                //        ()
                if (Parenthesis *pParenthesis = CreateNode<Parenthesis>(token)) {
                    m_pRunableCurrent->SetRight(pParenthesis);
                    m_pRunableCurrent = pParenthesis;
                }
                else {
                    ERROR(OUT_OF_MEMORY);
                }
            }
            else {
                // expression
                ERROR(SYNTAX_ERROR);
            }
            break;
    }

    return (m_error == NO_ERROR);
}
/*
allowable parenthesis:
1) close function call: token()
stack: ...,FunctionEvent, ExpEvent, POP ExpEvent
2) close keywords: for(), while(), switch(), if()
stack: ScopeEvent,CinditionalDecEvent, ExpEvent, POP ExpEvent PUSH ConditionalBodyEvent
3) close expression parenthesis: (expression)
stack ...,ExpEvent, POP ExpEvent
4) close function definition: function foo()
stack: ScopeEvent,FunctionEvent,FunctionArgEvent, POP FunctionArgEvent, PUSH FunctionBodyEvent

*/
bool Interpreter::CloseParenthesis(string &token) {
    assert(token[0] == ')');

    Node *pParent = m_pRunableCurrent->GetParent();
    if (!pParent) {
        ERROR(UNEXPECTED_PARENTHESIS);
        return false;
    }

    if (Node *pNode = GetOpenParenthesisNode()) {
        NODE_TYPE type = pNode->GetType();
        switch (type) {
            case TYPE_PARENTHESIS: {
                Parenthesis *pParenthesis = static_cast<Parenthesis*>(pNode);
                pParenthesis->SetSymbolId(SYMBOL_ID_PARENTHESIS);
                m_pRunableCurrent = pParenthesis;
                Type *pRetType = 0;//Node *pDestNode = 0;
                PrePostExpressions *pPrePostExp = 0;
                Node *pExpRoot = 0;
                // stack: ScopeEvent,ConditionEvent,ExpEvent
                switch (m_pRunableCurrent->GetParent()->GetType()) {
                    case TYPE_SWITCH:
                        if (m_pRunableCurrent->IsExpression(pRetType, pPrePostExp, m_error)) {
                            pExpRoot = m_pRunableCurrent;
                            //if (IsAggregateType(m_pRunableCurrent->GetRight())) {
                            //    ERROR(AGGREGATE_AS_VALUE_NOT_SUPPORTED);
                            //    break;
                            //}
                            m_stateEventStack.pop_back(); // remove ExpEvent
                            m_stateEventStack.push_back(&Interpreter::SwitchBodyEvent);
                            m_pRunableCurrent = m_pRunableCurrent->GetParent();
                            if (!ApplyPrePostExpressions(pExpRoot, pPrePostExp)) {
                                ERROR(INTERNAL_ERROR);
                                return false;
                            }
                        }
                        else if (pPrePostExp) {
                            delete pPrePostExp;
                        }
                        break;
                    case TYPE_WHILE:
                        if (m_pRunableCurrent->IsExpression(pRetType, pPrePostExp, m_error)) {
                            pExpRoot = m_pRunableCurrent;
                            //if (IsAggregateType(m_pRunableCurrent->GetRight())) {
                            //    ERROR(AGGREGATE_AS_VALUE_NOT_SUPPORTED);
                            //    break;
                            //}
                            m_stateEventStack.pop_back(); // remove ExpEvent
                            m_stateEventStack.push_back(&Interpreter::WhileBodyEvent);
                            m_pRunableCurrent = m_pRunableCurrent->GetParent();
                            if (!ApplyPrePostExpressions(pExpRoot, pPrePostExp)) {
                                ERROR(INTERNAL_ERROR);
                                return false;
                            }
                        }
                        else if (pPrePostExp) {
                            delete pPrePostExp;
                        }
                        break;
                    case TYPE_IF:
                        if (m_pRunableCurrent->IsExpression(pRetType, pPrePostExp, m_error)) {
                            pExpRoot = m_pRunableCurrent;
                            //if (IsAggregateType(m_pRunableCurrent->GetRight())) {
                            //    ERROR(AGGREGATE_AS_VALUE_NOT_SUPPORTED);
                            //    break;
                            //}
                            m_stateEventStack.pop_back(); // remove ExpEvent
                            m_stateEventStack.push_back(&Interpreter::IfBodyEvent);
                            m_pRunableCurrent = m_pRunableCurrent->GetParent();
                            if (!ApplyPrePostExpressions(pExpRoot, pPrePostExp)) {
                                ERROR(INTERNAL_ERROR);
                                return false;
                            }
                        }
                        else if (pPrePostExp) {
                            delete pPrePostExp;
                        }
                        break;
                    case TYPE_FOR:
                        m_pRunableCurrent = pNode;
                        break;
                    default:
                        break;
                }
                break;
            }
            case TYPE_FUNCTION_CALL: {
                m_stateEventStack.pop_back(); // remove ExpEvent
                m_stateEventStack.pop_back(); // remove FunctionCallArgEvent
                m_stateEventStack.pop_back(); // remove FunctionCallEvent
                {
                    Type *pRetType = 0;
                    if (Function *pFunction = m_pRunableRoot->ResolveFunction(pNode->GetSymbolId())) {
                        switch (pFunction->GetType()) {
                            case TYPE_CAST:
                            case TYPE_SIZEOF:
                            case TYPE_ARR2STR:
                            case TYPE_WARR2STR:
                            case TYPE_STR2ARR:
                            case TYPE_STR2WARR:
                            case TYPE_LOCK:
                            case TYPE_UNLOCK:
                                m_pRunableCurrent = pNode->GetParent<Variable>(TYPE_VAR);
                                return true;
                            default:
                                pRetType = pFunction->GetReturnType();
                                break;
                        }
                    }
                    else {
                        if (Variable *pVariable = pNode->GetParent()->QueryType<Variable>(TYPE_VAR)) {
                            FunctionRefType *pFunctionRefType = pVariable->GetTypeNode()->QueryType<FunctionRefType>(TYPE_FUNCTION_REF_TYPE);
                            pRetType = pFunctionRefType->GetReturnType();
                        }
                        else {
                            ERROR(INTERNAL_ERROR);
                            return false;
                        }
                    }

                    m_pRunableCurrent = pNode;

                    switch (pRetType->GetType()) {
                        case TYPE_AGGREGATE_TYPE:
                            m_stateEventStack.push_back(&Interpreter::BeginMemberEvent);// FunctionCallReturnAggregateEvent);
                            break;
                        case TYPE_ARRAY:
                            m_stateEventStack.push_back(&Interpreter::OpenSubscriptEvent);// FunctionCallReturnArrayEvent);
                            break;
                        case TYPE_FUNCTION_REF_TYPE:
                            m_stateEventStack.push_back(&Interpreter::FunctionCallEvent);
                            break;
                        default:
                            m_pRunableCurrent = pNode->GetRootValueNode();
                            break;
                    }
                }
                break;
            }
            case TYPE_SWITCH:
            case TYPE_WHILE:
            case TYPE_IF: {
                assert(0);
                //ConditionalNode *pConditionalNode = static_cast<ConditionalNode*>(pNode);
                //if (Node* pCondition = pConditionalNode->GetCondition()) {
                //    static_cast<Parenthesis*>(pCondition)->SetSymbolId(SYMBOL_ID_PARENTHESIS);
                //    m_pRunableCurrent = pConditionalNode;
                //    Type *pRetType = 0;// Node *pDestNode = 0;
                //    PrePostExpressions *pPrePostExp = 0;
                //    if (pCondition->IsExpression(pRetType, pPrePostExp, m_error)) {
                //        if (!ApplyPrePostExpressions(pCondition, pPrePostExp)) {
                //            ERROR(INTERNAL_ERROR);
                //            return false;
                //        }
                //        // stack: ScopeEvent,ConditionEvent,ExpEvent
                //        m_stateEventStack.pop_back(); // remove ExpEvent
                //        switch (type) {
                //            case TYPE_SWITCH:
                //                m_stateEventStack.push_back(&Interpreter::SwitchBodyEvent);
                //                break;
                //            case TYPE_WHILE:
                //                m_stateEventStack.push_back(&Interpreter::WhileBodyEvent);
                //                break;
                //            case TYPE_IF:
                //                m_stateEventStack.push_back(&Interpreter::IfBodyEvent);
                //                break;
                //            default:
                //                break;
                //        }
                //    }
                //    else if (pPrePostExp) {
                //        delete pPrePostExp;
                //    }
                //}
                //else {
                //    ERROR(EXPECTING_CONDITION);
                //}
                break;
            }
            case TYPE_FOR: {
                // if valid for statements for(1;2;3) then add m_stateEventStack.push_back(&Compiler::IfBodyEvent);
                Node *pExpNode = m_pRunableCurrent->GetExpressionRoot();
                Type *pRetType = 0;// Node *pDestNode = 0;
                if (pExpNode) {
                    PrePostExpressions *pPrePostExp = 0;
                    if (pExpNode->IsExpression(pRetType, pPrePostExp, m_error)) {
                        if (!ApplyPrePostExpressions(pExpNode, pPrePostExp)) {
                            ERROR(INTERNAL_ERROR);
                            return false;
                        }
                        m_pRunableCurrent = pNode;
                        m_stateEventStack.pop_back(); // remove ExpEvent
                        m_stateEventStack.push_back(&Interpreter::ForLoopEvent);
                        return DispatchEvent(TYPE_PARENTHESIS, token);
                    }
                    else if (pPrePostExp) {
                        delete pPrePostExp;
                    }
                    break;
                }
                ERROR(SYNTAX_ERROR);
                break;
            }
            case TYPE_FUNCTION:
                m_stateEventStack.pop_back(); // remove FunctionArgEvent
                m_stateEventStack.push_back(&Interpreter::FunctionBodyEvent);
                break;
            default:
                ERROR(INTERNAL_ERROR);
                break;
        }
    }
    else {
        ERROR(UNEXPECTED_PARENTHESIS);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::OpenSubscript() {
    if (Variable *pVariable = m_pRunableCurrent->QueryType<Variable>(TYPE_TOKEN)) {
        Type *pType = pVariable->GetTypeNode();
        if (pType->GetType() == TYPE_ARRAY) {
            if (!pVariable->GetRight()) {
                if (SubscriptNode *pSubscriptNode = CreateNode<SubscriptNode>(string("["))) {
                    ui32 idx = pVariable->GetVarIdx();

                    idx |= MEMORY_OFFSET_BIT_SUB_STACK;
                    pSubscriptNode->SetVarIdx(idx);
                    m_pRunableCurrent->SetRight(pSubscriptNode);
                    m_pRunableCurrent = pSubscriptNode;
                    if (Node *pLeft = pVariable->GetLeft()) {
                        pSubscriptNode->SetLeft(pLeft);
                        pVariable->SetLeft(0);
                        while (pLeft) {
                            pLeft->SetParent(pSubscriptNode);
                            pLeft = pLeft->GetLeft();
                        }
                    }
                    m_stateEventStack.push_back(&Interpreter::ArrayBeginSubscriptEvent);
                }
            }
            else {
                ERROR(SYNTAX_ERROR);
            }
        }
        else {
            ERROR(SYNTAX_ERROR);
        }
    }
    else {
        ERROR(SYNTAX_ERROR);
    }

    return (m_error == NO_ERROR);
}

bool Interpreter::CloseSubscript() {
    m_stateEventStack.pop_back();
    if (SubscriptNode * pSubscriptNode = m_pRunableCurrent->QueryType<SubscriptNode>(TYPE_SUBSCRIPT)) {
        if (pSubscriptNode->GetSymbolId() == SYMBOL_ID_OPENSUBSCRIPT) {
            pSubscriptNode->SetSymbolId(SYMBOL_ID_SUBSCRIPT);
            ArrayType *pArrayType = 0;
            switch (pSubscriptNode->GetParent()->GetType()) {
                case TYPE_VAR:
                    pArrayType = static_cast<ArrayType*>(static_cast<Variable*>(pSubscriptNode->GetParent())->GetTypeNode());
                    break;
                case TYPE_FUNCTION_CALL:
                    pArrayType = static_cast<FunctionCall*>(pSubscriptNode->GetParent())->GetFunctionRefTypeNode()->GetReturnType()->QueryType<ArrayType>(TYPE_ARRAY);
                    assert(pArrayType);
                    break;
                default:
                    ERROR(INTERNAL_ERROR);
                    return false;
            }

            m_pRunableCurrent = pSubscriptNode;

            switch (pArrayType->GetTypeNode()->GetType()) {
                case TYPE_AGGREGATE_TYPE:
                    m_stateEventStack.push_back(&Interpreter::BeginMemberEvent);// FunctionCallReturnAggregateEvent);
                    break;
                case TYPE_ARRAY:
                    m_stateEventStack.push_back(&Interpreter::OpenSubscriptEvent);// FunctionCallReturnArrayEvent);
                    break;
                case TYPE_FUNCTION_REF_TYPE:
                    m_stateEventStack.push_back(&Interpreter::FunctionCallEvent);
                    break;
                default:
                    break;
            }
        }
        else {
            ERROR(INTERNAL_ERROR);
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }

    return (m_error == NO_ERROR);
}

//bool Interpreter::FinalFunctionCall() {
//    assert(m_pRunableCurrent->GetRight() == 0);
//    Type *pCallerType = 0;
//    if (Variable *pVariable = m_pRunableCurrent->QueryType<Variable>(TYPE_TOKEN)) {
//        pCallerType = pVariable->GetTypeNode();
//    }
//    else if (FunctionCall *pFunctionCall = m_pRunableCurrent->QueryType<FunctionCall>(TYPE_FUNCTION_CALL)) {
//        pCallerType = pFunctionCall->GetFunctionRefTypeNode()->GetReturnType();
//    }
//    else if (SubscriptNode *pSubscript = m_pRunableCurrent->QueryType<SubscriptNode>(TYPE_SUBSCRIPT)) {
//        pCallerType = static_cast<ArrayType*>(pSubscript->GetParent())->GetTypeNode();
//    }
//    else {
//        ERROR(INTERNAL_ERROR);
//        return true;
//    }
//    assert(pCallerType);
//    if (pCallerType->GetType() == TYPE_FUNCTION_REF_TYPE) {
//        if (FunctionCall *pFunctionCall = CreateNode<FunctionCall>(pCallerType->GetSymbolId())) {
//            m_pRunableCurrent->SetRight(pFunctionCall);
//            m_pRunableCurrent = pFunctionCall;
//            m_stateEventStack.push_back(&Interpreter::FunctionCallArgEvent);
//            return false;
//        }
//        else {
//            ERROR(OUT_OF_MEMORY);
//        }
//    }
//
//    return true;
//}
//
Node* Interpreter::GetOpenParenthesisNode() {
    Node* pNode = m_pRunableCurrent;
    while (pNode) {
        switch (pNode->GetType()) {
            case TYPE_FUNCTION:
            case TYPE_FUNCTION_CALL:
            case TYPE_IF:
            case TYPE_WHILE:
            case TYPE_FOR:
            case TYPE_SWITCH:
                return pNode;
            case TYPE_SCOPE:
                return 0;
            default:
                if (pNode->GetSymbolId() == SYMBOL_ID_OPENPARENTHESIS) {
                    return pNode;
                }
                break;
        }
        pNode = pNode->GetParent();
    }

    return 0;
}

MEMORY_OFFSET Interpreter::ReserveAlignedStackSpotIfNotDeclared(SYMBOL_DESC &symDesc, Type *pType, ui32 &idx) {
    MEMORY_OFFSET offset = INVALID_MEMORY_OFFSET;

    Scope *pScope = static_cast<Scope*>((m_pRunableCurrent->GetType() == TYPE_SCOPE) ? m_pRunableCurrent : m_pRunableCurrent->GetParent<Scope>(TYPE_SCOPE));
    if (pScope) {
        if (!pScope->FindTypeVariableDeclNode(symDesc.m_symId)) {
            if (Function *pFunction = pScope->GetFunctionNode()) {
                pScope = static_cast<Scope*>(pFunction->GetLeft());
            }
            else {
                pScope = m_pRunableRoot;
            }
            // now reserve stack spot for this variable
#ifdef SCRIPT_DEBUGGER
            if (!pScope->ReserveAlignedStackSopt(symDesc, pType, offset, idx)) {
                ERROR(UNKNOWN_VALUE_TYPE);
            }
#else // SCRIPT_DEBUGGER
            if (!pScope->ReserveAlignedStackSopt(pType, offset, idx)) {
                ERROR(UNKNOWN_VALUE_TYPE);
            }
#endif // SCRIPT_DEBUGGER
        }
        else {
            ERROR(DUPLICATE_DECLARATION);
        }
    }
    else {
        ERROR(INTERNAL_ERROR);
    }

    return offset;
}

MEMORY_OFFSET Interpreter::ResolveAlignedVariable(SYMBOL_ID symId, Type *&pResolvedType, ui32 &idx) {
    // resolve local if not resolved then resolve global, if not resolved add local variable    
    pResolvedType = 0;
    if (Variable *pVariableDec = m_pRunableCurrent->FindVariableDec(symId)) {
        pResolvedType = pVariableDec->GetTypeNode();
        idx = pVariableDec->GetVarIdx();
        return pVariableDec->GetAlignedOffset();
    }
    return INVALID_MEMORY_OFFSET;
}

bool Interpreter::AlignAggregateType() {
    if (AggregateType *pAggregateType = m_pRunableCurrent->QueryType<AggregateType>(TYPE_AGGREGATE_TYPE)) {
        return pAggregateType->AlignStruct(0);
    }
    return false;
}

bool Interpreter::ResolveArguments(Function * pFunction) {
    Scope *pScope = static_cast<Scope*>(m_pRunableCurrent);
    Type *pReturnType = pFunction->GetReturnType();
    pReturnType->SetParent(pScope);
    vector<SYMBOL_DESC> &arguments = pFunction->GetArgumentVector();
    for (ui32 i = 0; i < arguments.size(); ++i) {
        string token;
        SYMBOL_ID argSymId = arguments[i].m_symId;
        if (!m_symbolStore.GetSymbolName(argSymId, token)) {
            ERROR(INTERNAL_ERROR);
            break;
        }
        else {
            Type *pResolvedType = pFunction->GetArgumentType(argSymId);

            MEMORY_OFFSET alignedOffset;
            ui32 idx;
#ifdef SCRIPT_DEBUGGER
            pScope->ReserveAlignedStackSopt(arguments[i], pResolvedType, alignedOffset, idx);
#else // SCRIPT_DEBUGGER
            pScope->ReserveAlignedStackSopt(pResolvedType, alignedOffset, idx);
#endif // SCRIPT_DEBUGGER
            if (Variable* pVariableDec = new Variable(pResolvedType, arguments[i], idx, alignedOffset, 0, pResolvedType)) {
                pResolvedType->SetParent(pScope);
                pScope->PushExpression(pResolvedType);
                if (!pFunction->SetArgumentVariable(argSymId, pVariableDec)) {
                    ERROR(INTERNAL_ERROR);
                    break;
                }
            }
            else {
                ERROR(OUT_OF_MEMORY);
            }
        }
    }
    return (m_error == NO_ERROR);
}

AggregateType* Interpreter::FindTypeDefinition(SYMBOL_ID symId) {
    Node* pNode = m_pRunableCurrent;
    //// search within structure
    //while (pNode->GetType() == TYPE_AGGREGATE_TYPE) {
    //    AggregateDefCollection *pAggregateDefCollection = pNode->QueryType<AggregateDefCollection>(TYPE_AGGREGATE_TYPE_COLLECTION);
    //    assert(pAggregateDefCollection);
    //    if (AggregateType *pResolved = pAggregateDefCollection->ResolveAggregateType(symId)) {
    //        return pResolved;
    //    }
    //    pNode = pNode->GetParent();
    //}

    //if (pNode->GetType() == TYPE_SCOPE) { // search this scope
    //    AggregateDefCollection *pAggregateDefCollection = pNode->QueryType<AggregateDefCollection>(TYPE_AGGREGATE_TYPE_COLLECTION);
    //    assert(pAggregateDefCollection);
    //    if (AggregateType *pResolved = pAggregateDefCollection->ResolveAggregateType(symId)) {
    //        return pResolved;
    //    }
    //}
    //// go up to the function and then global scope and search again
    //if (Function *pFunction = pNode->GetParent<Function>(TYPE_FUNCTION)) {
    //    AggregateDefCollection *pAggregateDefCollection = pFunction->GetLeft()->QueryType<AggregateDefCollection>(TYPE_AGGREGATE_TYPE_COLLECTION);
    //    assert(pAggregateDefCollection);
    //    if (AggregateType *pResolved = pAggregateDefCollection->ResolveAggregateType(symId)) {
    //        return pResolved;
    //    }
    //}

    //AggregateDefCollection *pAggregateDefCollection = m_pRunableRoot->QueryType<AggregateDefCollection>(TYPE_AGGREGATE_TYPE_COLLECTION);
    //assert(pAggregateDefCollection);
    //if (AggregateType *pResolved = pAggregateDefCollection->ResolveAggregateType(symId)) {
    //    return pResolved;
    //}

    while (pNode) {
        switch (pNode->GetType()) {
            case TYPE_SCOPE:
                //break;
            case TYPE_AGGREGATE_TYPE: { // this is a structure, so search type within this structure
                AggregateDefCollection *pAggregateDefCollection = pNode->QueryType<AggregateDefCollection>(TYPE_AGGREGATE_TYPE_COLLECTION);
                assert(pAggregateDefCollection);
                // this is a type definition
                if (AggregateType *pResolved = pAggregateDefCollection->ResolveAggregateType(symId)) {
                    return pResolved;
                }
                break;
            }
            case TYPE_FUNCTION: { // stop local search and search global scope
                AggregateType *pResolved = m_pRunableRoot->ResolveAggregateType(symId);
                return pResolved;
            }
            default:
                break;
        }
        pNode = pNode->GetParent();
    }

    return 0;
}

Node* Interpreter::GetDefinedSymbolNode(SYMBOL_ID symId) {
    Scope *pScope = m_pRunableCurrent->GetParent<Scope>(TYPE_SCOPE);
    assert(pScope);
    Variable *pVariable = pScope->FindTypeVariableDeclNode(symId);

    if (pVariable) {
        return pVariable;
    }
    
    Function *pFunction = pScope->ResolveFunction(symId);
    return pFunction;
}


bool Interpreter::AddAggregateSymbol(SYMBOL_ID symId) {
    if (AggregateSymbol *pAggregateSymbol = m_pRunableCurrent->QueryType<AggregateSymbol>(TYPE_AGGREGATE_SYMBOL)) {
        pAggregateSymbol->AddSymbol(symId);
    }
    else {
        ERROR(INTERNAL_ERROR);
    }
    return (m_error == NO_ERROR);
}

//bool Interpreter::ValidateArraySubscript(bool lastSubscript) {
//    Node *pExpNode = m_pRunableCurrent;
//    while (pExpNode) {
//        if (Variable *pVariable = pExpNode->QueryType<Variable>(TYPE_VAR)) {
//            pExpNode = pVariable->GetRootVariableNode();
//        }
//        if (pExpNode->GetParent()->GetType() == TYPE_SUBSCRIPT) {
//            break;
//        }
//        pExpNode = pExpNode->GetParent();
//    }
//    if (pExpNode) {
//        if (SubscriptNode *pSubscriptNode = pExpNode->GetParent<SubscriptNode>(TYPE_SUBSCRIPT)) {
//            m_pRunableCurrent = pSubscriptNode;
//            ArrayType *pArrayType = static_cast<ArrayType*>(static_cast<Variable*>(pSubscriptNode->GetParent())->GetTypeNode());
//            // check if subscript is not out of bounds
//            if (lastSubscript) {
//                if (ui32 dim = pArrayType->GetDimensionCount()) { // or runtime validation and RT_ERROR based signal
//                    if (dim != pSubscriptNode->GetSubscriptCount()) {
//                        ERROR(UNEXPECTED_NUMBER_OF_SUBSCRIPTS);
//                        return false;
//                    }
//                }
//            }
//            else {
//                if (ui32 dim = pArrayType->GetDimensionCount()) { // or runtime validation and RT_ERROR based signal
//                    if (dim < pSubscriptNode->GetSubscriptCount()) {
//                        ERROR(UNEXPECTED_NUMBER_OF_SUBSCRIPTS);
//                        return false;
//                    }
//                }
//                else {
//                    ERROR(NO_DIMENSION_ARRAY_MUST_HAVE_ONE_DIMESION_SUBSCRIPT);
//                    return false;
//                }
//            }
//            Type *pRetType = 0;// Node *pDestNode = 0;
//            PrePostExpressions *pPrePostExp = 0;
//            if (pExpNode->IsExpression(pRetType, pPrePostExp, m_error)) {
//                if (!ApplyPrePostExpressions(pExpNode, pPrePostExp)) {
//                    ERROR(INTERNAL_ERROR);
//                    return false;
//                }
//            }
//            else if (pPrePostExp) {
//                delete pPrePostExp;
//            }
//        }
//        else {
//            ERROR(INTERNAL_ERROR);
//        }
//    }
//    else {
//        ERROR(INTERNAL_ERROR);
//    }
//
//    return (m_error == NO_ERROR);
//}

bool Interpreter::ApplyPrePostExpressions(Node *pExpRootNode, PrePostExpressions *pRePostExp) {
    if (pRePostExp) {
        pRePostExp->SetExpressionNode(pExpRootNode);
        return pExpRootNode->GetParent()->InjectPrePostExpressions(pRePostExp);
    }
    return true;
}

bool Interpreter::IsSizeOf(Node *pNode) {
    if (!pNode || !pNode->GetParent()) {
        return false;
    }

    if (Variable *pVariable = pNode->GetParent()->QueryType<Variable>(TYPE_TOKEN)) {
        if (Type *pType = pVariable->GetTypeNode()) {
            if (pType->GetParent()->GetType() == TYPE_SIZEOF) {
                return true;
            }
        }
    }
    return false;
}

bool Interpreter::ErrorIfDynamicArray(Type *pType) {
    if (ArrayType *pArrayType = pType->QueryType<ArrayType>(TYPE_ARRAY)) {
        if (!pArrayType->GetDimensionCount() && (pArrayType->GetPassBy() == Type::BY_VALUE)) {
            ERROR(EXPECTING_NONZERO_ARRAY_DIMENSION);
            return true;
        }
    }
    return false;
}

/**************************************************************/
Reader::Reader(bool recursiveReader, InterpreterIf &interpreter, Error &error) :
    m_error(error),
    m_Interpreter(interpreter),
    m_line(START_POSITION_OFFSET),
    m_pos(START_POSITION_OFFSET),
    m_recursiveReader(recursiveReader) {
//#ifdef _DEBUG
    m_Interpreter.SetReader(this);
//#endif // _DEBUG
    m_keywords.insert(pair<const char*, NODE_TYPE>("function", TYPE_FUNCTION));
    m_keywords.insert(pair<const char*, NODE_TYPE>("if", TYPE_IF));
    m_keywords.insert(pair<const char*, NODE_TYPE>("else", TYPE_ELSE));
    m_keywords.insert(pair<const char*, NODE_TYPE>("for", TYPE_FOR));
    //m_keywords.insert(pair<const char*, NODE_TYPE>("new", TYPE_NEW));
    m_keywords.insert(pair<const char*, NODE_TYPE>("switch", TYPE_SWITCH));
    m_keywords.insert(pair<const char*, NODE_TYPE>("case", TYPE_CASE));
    m_keywords.insert(pair<const char*, NODE_TYPE>("break", TYPE_BREAK));
    m_keywords.insert(pair<const char*, NODE_TYPE>("continue", TYPE_CONTINUE));
    m_keywords.insert(pair<const char*, NODE_TYPE>("default", TYPE_DEFAULT));
    m_keywords.insert(pair<const char*, NODE_TYPE>("while", TYPE_WHILE));
    m_keywords.insert(pair<const char*, NODE_TYPE>("return", TYPE_RETURN));
    m_keywords.insert(pair<const char*, NODE_TYPE>("bool", TYPE_BOOL));
    m_keywords.insert(pair<const char*, NODE_TYPE>("i8", TYPE_I8));
    m_keywords.insert(pair<const char*, NODE_TYPE>("ui8", TYPE_UI8));
    m_keywords.insert(pair<const char*, NODE_TYPE>("i16", TYPE_I16));
    m_keywords.insert(pair<const char*, NODE_TYPE>("ui16", TYPE_UI16));
    m_keywords.insert(pair<const char*, NODE_TYPE>("i32", TYPE_I32));
    m_keywords.insert(pair<const char*, NODE_TYPE>("ui32", TYPE_UI32));
    m_keywords.insert(pair<const char*, NODE_TYPE>("i64", TYPE_I64));
    m_keywords.insert(pair<const char*, NODE_TYPE>("ui64", TYPE_UI64));
    m_keywords.insert(pair<const char*, NODE_TYPE>("float", TYPE_FLOAT));
    m_keywords.insert(pair<const char*, NODE_TYPE>("double", TYPE_DOUBLE));
    m_keywords.insert(pair<const char*, NODE_TYPE>("string", TYPE_STRING));
    m_keywords.insert(pair<const char*, NODE_TYPE>("true", TYPE_CONST_BOOL));
    m_keywords.insert(pair<const char*, NODE_TYPE>("false", TYPE_CONST_BOOL));
    m_keywords.insert(pair<const char*, NODE_TYPE>("include", TYPE_INCLUDE));
    m_keywords.insert(pair<const char*, NODE_TYPE>("lib", TYPE_LIB));
    m_keywords.insert(pair<const char*, NODE_TYPE>("struct", TYPE_STRUCT));
    m_keywords.insert(pair<const char*, NODE_TYPE>("array", TYPE_ARRAY));
    m_keywords.insert(pair<const char*, NODE_TYPE>("error", TYPE_ERROR));
    m_keywords.insert(pair<const char*, NODE_TYPE>("sizeof", TYPE_SIZEOF));
    m_keywords.insert(pair<const char*, NODE_TYPE>("arr2str", TYPE_ARR2STR));
    m_keywords.insert(pair<const char*, NODE_TYPE>("warr2str", TYPE_WARR2STR));
    m_keywords.insert(pair<const char*, NODE_TYPE>("str2arr", TYPE_STR2ARR));
    m_keywords.insert(pair<const char*, NODE_TYPE>("str2warr", TYPE_STR2WARR));
    m_keywords.insert(pair<const char*, NODE_TYPE>("lock", TYPE_LOCK));
    m_keywords.insert(pair<const char*, NODE_TYPE>("unlock", TYPE_UNLOCK));
    m_keywords.insert(pair<const char*, NODE_TYPE>("cast", TYPE_CAST));

    m_operators.insert(pair<const char*, NODE_TYPE>("+", TYPE_ADD));
    m_operators.insert(pair<const char*, NODE_TYPE>("+=", TYPE_ADD_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("++", TYPE_INC_OP));
    m_operators.insert(pair<const char*, NODE_TYPE>("-", TYPE_SUB));
    m_operators.insert(pair<const char*, NODE_TYPE>("-=", TYPE_SUB_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("--", TYPE_DEC_OP));
    m_operators.insert(pair<const char*, NODE_TYPE>("&", TYPE_BIT_AND));
    m_operators.insert(pair<const char*, NODE_TYPE>("&=", TYPE_BIT_AND_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("&&", TYPE_AND));
    m_operators.insert(pair<const char*, NODE_TYPE>("|", TYPE_BIT_OR));
    m_operators.insert(pair<const char*, NODE_TYPE>("|=", TYPE_BIT_OR_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("||", TYPE_OR));
    m_operators.insert(pair<const char*, NODE_TYPE>("<", TYPE_LESS));
    m_operators.insert(pair<const char*, NODE_TYPE>("<=", TYPE_LESS_EQ));
    m_operators.insert(pair<const char*, NODE_TYPE>("<<", TYPE_SHIFT_LEFT));
    m_operators.insert(pair<const char*, NODE_TYPE>("<<=", TYPE_SHIFT_LEFT_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>(">", TYPE_GREATER));
    m_operators.insert(pair<const char*, NODE_TYPE>(">=", TYPE_GREATER_EQ));
    m_operators.insert(pair<const char*, NODE_TYPE>(">>", TYPE_SHIFT_RIGHT));
    m_operators.insert(pair<const char*, NODE_TYPE>(">>=", TYPE_SHIFT_RIGHT_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("*", TYPE_MUL));
    m_operators.insert(pair<const char*, NODE_TYPE>("*=", TYPE_MUL_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("/", TYPE_DIV));
    m_operators.insert(pair<const char*, NODE_TYPE>("/=", TYPE_DIV_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("%", TYPE_MOD));
    m_operators.insert(pair<const char*, NODE_TYPE>("%=", TYPE_MOD_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("~", TYPE_BIT_NOT));
    m_operators.insert(pair<const char*, NODE_TYPE>("!", TYPE_NOT));
    m_operators.insert(pair<const char*, NODE_TYPE>("!=", TYPE_NOT_EQ));
    m_operators.insert(pair<const char*, NODE_TYPE>("^", TYPE_XOR));
    m_operators.insert(pair<const char*, NODE_TYPE>("^=", TYPE_XOR_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("[", TYPE_SUBSCRIPT));
    m_operators.insert(pair<const char*, NODE_TYPE>("]", TYPE_SUBSCRIPT));
    m_operators.insert(pair<const char*, NODE_TYPE>("(", TYPE_PARENTHESIS));
    m_operators.insert(pair<const char*, NODE_TYPE>(")", TYPE_PARENTHESIS));
    m_operators.insert(pair<const char*, NODE_TYPE>("{", TYPE_SCOPE));
    m_operators.insert(pair<const char*, NODE_TYPE>("}", TYPE_SCOPE));
    m_operators.insert(pair<const char*, NODE_TYPE>(".", TYPE_MEMBER_ACCESS));
    m_operators.insert(pair<const char*, NODE_TYPE>(",", TYPE_COMMA));
    m_operators.insert(pair<const char*, NODE_TYPE>(";", TYPE_SEMICOLON));
    m_operators.insert(pair<const char*, NODE_TYPE>(":", TYPE_COLON));
    m_operators.insert(pair<const char*, NODE_TYPE>("=", TYPE_ASSIGN));
    m_operators.insert(pair<const char*, NODE_TYPE>("==", TYPE_EQ));
    m_operators.insert(pair<const char*, NODE_TYPE>("@", TYPE_REF));
    m_operators.insert(pair<const char*, NODE_TYPE>("@=", TYPE_REF_ASSIGN));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));
    //m_operators.insert(pair<const char*, NODE_TYPE>("", ));

}

bool Reader::Read(string path, ui32 &win32Error) {
    win32Error = S_OK;
    ifstream file(path , ios::in | ios::binary);
    if (file.is_open()) {
        string script;
        m_filePath = path;
        file.seekg(0, std::ios::end);
        file.seekg(0, std::ios::beg);
        script.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        ui8* pData = (ui8*)script.c_str();
        ui32 len = script.length();
        return OnInit(pData, len) && OnRead(pData, len);
    }
    else {
        win32Error = GetLastError();
        return false;
    }
}

bool Reader::OnInit(ui8 *pData, ui32 size) {
    ERROR(NO_ERROR);
    m_line = m_pos = START_POSITION_OFFSET;

    if (!pData) {
        ERROR(INVALID_POINTER);
        return false;
    }

    return m_recursiveReader ? true : m_Interpreter.Init();
}

bool Reader::OnToken(NODE_TYPE type, string &token) {
    return m_Interpreter.DispatchEvent(type, token);
}

bool Reader::IsValidNum(string &token) {
    const char *pch = token.c_str();
    bool good = *pch != 0;

    while (*pch) {
        if (!IsNum(*pch)) {
            good = false;
            break;
        }
        ++pch;
    }

    return good;
}

/**************************************************************/
ScriptReader::ScriptReader(bool recursiveReader, InterpreterIf &interpreter, Error &error) : Reader(recursiveReader, interpreter, error) {
}

bool ScriptReader::OnRead(ui8 *pData, ui32 size) {
    char *pScript = (char*)pData;
    ui32 nextLine = START_POSITION_OFFSET, nextPos = START_POSITION_OFFSET;
    string token;

    while (*pScript) {
        char ch = *pScript;
        if (IsChar(ch) || IsNum(ch)) { // check if collected token is not an operator
            token.push_back(ch);
            ++nextPos;
        }
        else {
            if (!token.empty()) {
                if (!ProcessToken(token)) {
                    return false;
                }
                continue;
            }
            else {
                switch (ch) {
                    case '\n':
                        ++nextLine;
                        nextPos = START_POSITION_OFFSET;
                        break;
                    case '\r':
                        nextPos = START_POSITION_OFFSET;
                        break;
                    case '\t':
                        nextPos += TAB_SIZE;
                        break;
                    case ' ':
                        ++nextPos;
                        break;
                    default:
                        if (!ProcessOperator(pScript, token)) {
                            return false;
                        }
                        nextPos = m_pos;
                        nextLine = m_line;
                        continue;
                }
            }
        }
        m_pos = nextPos;
        m_line = nextLine;
        ++pScript;
    }

    if (token.empty()) {
        if (((Interpreter&)m_Interpreter).IsValidEndState()) {
            return true;
        }
    }
    ERROR(UNEXPECTED_END_OF_SCRIPT);
    return false;
}

bool ScriptReader::ProcessOperator(char *&pScript, string &token) {
    NODE_TYPE type = TYPE_NONE;
    ui32 posOffset = 0;
    char nextChar = *(pScript + 1);
    assert(token.empty());
    token.push_back(*pScript);
    // operators: (+,++,+=) (-,--,-=) (&,&&,&=) (|,||,|=) (<,<<,<=,<<=) (>,>>,>=,>>=) (*,*=) (/,/=) (%,%=) (~,~= invalid) (^,^=) (!,!=) (=,==)
    switch (*pScript) {
        case '+':
            type = TYPE_ADD;
            break;
        case '-':
            type = TYPE_SUB;
            break;
        case '&':
            type = TYPE_BIT_AND;
            break;
        case '|':
            type = TYPE_BIT_OR;
            break;
        case '<':
            type = TYPE_LESS;
            break;
        case '>':
            type = TYPE_GREATER;
            break;
        case '*':
            type = TYPE_MUL;
            break;
        case '/':
            if (nextChar == '*') { // long comment
                token.clear();
                ++m_pos;
                ++pScript;
                //find end of the comment
                return SkipLongComment(pScript);
            }
            else if (nextChar == '/') { // short comment
                token.clear();
                SkipShortComment(pScript);
                return true;
            }
            type = TYPE_DIV;
            break;
        case '%':
            type = TYPE_MOD;
            break;
        case '~':
            type = TYPE_BIT_NOT;
            break;
        case '^':
            type = TYPE_XOR;
            break;
        case '!':
            type = TYPE_NOT;
            break;
        case '=':
            type = TYPE_ASSIGN;
            break;
        case '(':
        case ')':
            type = TYPE_PARENTHESIS;
            break;
        case '[':
        case ']':
            type = TYPE_SUBSCRIPT;
            break;
        case '{':
        case '}':
            type = TYPE_SCOPE;
            break;
        case '.':
            type = TYPE_MEMBER_ACCESS;
            break;
        case ',':
            type = TYPE_COMMA;
            break;
        case ';':
            type = TYPE_SEMICOLON;
            break;
        case ':':
            type = TYPE_COLON;
            break;
        case '@':
            type = TYPE_REF;
if (nextChar == '=') {
    token.push_back(nextChar);
    type = TYPE_REF_ASSIGN;
    ++posOffset;
    ++pScript;
    goto NEXT;
}
break;
        case '\'':
        case '\"':
            if (!EndConstString(pScript, token, *pScript)) {
                return false;
            }
            m_pos -= token.length();
            if (!OnToken(TYPE_CONST, token)) {
                return false;
            }
            m_pos += token.length();
            token.clear();
            return true;
        default:
            ERROR(UNEXPECTED_CHARACTER);
            return false;
    }

    if (nextChar == '=') {
        token.push_back(nextChar);
        type = (NODE_TYPE)(type + 1);
        ++posOffset;
        ++pScript;
    }
    else if ((type <= TYPE_GREATER) && (nextChar == *pScript)) {
        token.push_back(nextChar);
        type = (NODE_TYPE)(type + 1 + 1);
        ++posOffset;
        ++pScript;

        switch (type) {
            case TYPE_BIT_NOT_ASSIGN:
                ERROR(INVALID_OPERATOR);
                return false;
            case TYPE_SHIFT_RIGHT:
            case TYPE_SHIFT_LEFT:
                nextChar = *(pScript + 1);
                if (!nextChar) {
                    ERROR(UNEXPECTED_END_OF_SCRIPT);
                    return false;
                }
                else if (nextChar == '=') {
                    token.push_back(nextChar);
                    type = (NODE_TYPE)(type + 1);
                    ++posOffset;
                    ++pScript;
                }
                break;
            default:
                break;
        }
    }

    if (type == TYPE_NONE) {
        ERROR(UNEXPECTED_CHARACTER);
        return false;
    }
    else if (type == TYPE_BIT_NOT_ASSIGN) {
        ERROR(INVALID_OPERATOR);
        return false;
    }

NEXT:
    ++posOffset;
    ++pScript;

    if (!OnToken(type, token)) {
        return false;
    }
    m_pos += posOffset;
    token.clear();
    return true;
}

bool ScriptReader::ProcessToken(string& token) {
    NODE_TYPE type = TYPE_NONE;
    ui32 tokenLen = token.length();

    assert(!token.empty());

    m_pos -= tokenLen;
    //process it...
    KEYWORDS::iterator it = m_keywords.find(token);

    if (it != m_keywords.end()) {
        type = it->second;
    }
    else {
        if (IsChar(*token.c_str())) {
            type = TYPE_VAR;
        }
#ifdef NUMBER
        else {
            if ((token[0] == '0') && (token[1] =='x' || token[1] == 'X')) { // hex number
                if ((token.length() < 3) || (token.length() > 18)) {
                    ERROR(INVALID_HEX_NUMBER);
                    return false;
                }
                type = TYPE_NUM_HEX_1;
            }
            else {
                //check last char
                char id = token[token.length() - 1];

                switch (id) {
                    case 'h':
                    case 'H':
                        if (token.length() > 17) {
                            ERROR(INVALID_HEX_NUMBER);
                            return false;
                        }
                        type = TYPE_NUM_HEX_2;
                        break;
                    case 'b':
                    case 'B':
                        if (token.length() > 65) {
                            ERROR(INVALID_BINARY_NUMBER);
                            return false;
                        }
                        type = TYPE_NUM_BIN;
                        break;
                    case 'e':
                    case 'E':
                        type = TYPE_NUM_EXP;
                        break;
                    default:
                        if (IsValidNum(token)) {
                            type = TYPE_CONST_NUM;
                        }
                        break;
                }
            }
        }
#else // NUMBER
        else if (IsValidNum(token)) { // also valid number format is (±)(N)(.)(N)(e±N) will be processed in NumberEvent()
            type = TYPE_CONST_NUM;
        }
#endif // NUMBER
    }

    if (!OnToken(type, token)) {
        return false;
    }

    token.clear();
    m_pos += tokenLen;

    return true;
}

bool ScriptReader::SkipLongComment(char *&pScript) {
    ERROR(UNEXPECTED_END_OF_SCRIPT);
    ++pScript;
    ++m_pos;
    char chPrev = 0;
    while (*pScript) {
        char ch = *pScript;
        if (ch == '/') {
            if (chPrev == '*') {//done
                ERROR(NO_ERROR);
                ++pScript;
                ++m_pos;
                break;
            }
        }
        else if (ch == '\n') {
            ++m_line;
            m_pos = START_POSITION_OFFSET;
        }
        else if (ch == '\r') {
            m_pos = START_POSITION_OFFSET;
        }
        else {
            ++m_pos;
        }
        chPrev = *pScript;
        ++pScript;
    }

    return (m_error == NO_ERROR);
}

void ScriptReader::SkipShortComment(char *&pScript) {
    ++pScript;
    ++m_pos;

    while (*pScript) {
        if (*pScript == '\n') {
            ++pScript;
            m_pos = START_POSITION_OFFSET;
            ++m_line;
            break;
        }
        else if (*pScript == '\r') {
            m_pos = START_POSITION_OFFSET;
        }
        else {
            ++m_pos;
        }
        ++pScript;
    }
}

bool ScriptReader::EndConstString(char *&pScript, string &out, const char quotationMark) {
    char chPrev = 0;
    ERROR(UNEXPECTED_END_OF_SCRIPT);
    ++pScript;
    ++m_pos;
    while (*pScript) {
        switch (*pScript) {
            case '\\':
                char hex;
                ++pScript;
                ++m_pos;
                switch (*pScript) {
                    case 'a':
                        hex = 0x07;
                        break;
                    case 'b':
                        hex = 0x08;
                        break;
                    case 'f':
                        hex = 0x0c;
                        break;
                    case 'n':
                        hex = 0x0a;
                        break;
                    case 'r':
                        hex = 0x0d;
                        break;
                    case 't':
                        hex = 0x09;
                        break;
                    case 'v':
                        hex = 0x0b;
                        break;
                    case '\\':
                    case '\'':
                    case '\"':
                        hex = *pScript;
                        break;
                    default:
                        if (*pScript) {
                            ERROR(INVALID_ESCAPE_CHARACTER);
                        }
                        return false;
                }
                out.push_back(hex);
                break;
            case '\'':
            case '\"':
                if (quotationMark == *pScript) {
                    out.push_back(*pScript);
                    ++pScript;
                    ++m_pos;
                    ERROR(NO_ERROR);
                    return true;
                }
                else {
                    ERROR((quotationMark == '\'') ? EXPECTING_SINGLE_QUOTATION_MARK : EXPECTING_DOUBLE_QUOTATION_MARK);
                    return false;
                }
                //case '\r':
                //case '\n':
            case '\0':
                ERROR(UNEXPECTED_CHARACTER);
                return false;
            default:
                out.push_back(*pScript);
                break;
        }
        ++pScript;
        ++m_pos;
    }

    if (m_error == NO_ERROR) {
        ERROR(UNEXPECTED_END_OF_SCRIPT);
    }
    return false;
}

/**************************************************************/
void BinaryReader::EscapeString(string &token) {
    string s;
    char ch = token[0];
    if ((ch == '\"') || (ch == '\'')) {
        s.push_back(token[0]);
        for (ui32 i = 1; i < token.size() - 1; ++i) {
            switch (token[i]) {
                case '\a':
                    s.push_back('\\');
                    s.push_back('a');
                    break;
                case '\b':
                    s.push_back('\\');
                    s.push_back('b');
                    break;
                case '\f':
                    s.push_back('\\');
                    s.push_back('f');
                    break;
                case '\n':
                    s.push_back('\\');
                    s.push_back('n');
                    break;
                case '\r':
                    s.push_back('\\');
                    s.push_back('r');
                    break;
                case '\t':
                    s.push_back('\\');
                    s.push_back('t');
                    break;
                case '\v':
                    s.push_back('\\');
                    s.push_back('v');
                    break;
                case '\\':
                    s.push_back('\\');
                    s.push_back('\\');
                    break;
                case '\'':
                    s.push_back('\\');
                    s.push_back('\'');
                    break;
                case '\"':
                    s.push_back('\\');
                    s.push_back('\"');
                    break;
                default:
                    s.push_back(token[i]);
                    break;
            }
        }
        s.push_back(token[token.size() - 1]);
        token = s;
    }
}

void BinaryReader::UnEscapeString(string &token) {
    string s;
    char ch = token[0];
    if ((ch == '\"') || (ch == '\'')) {
        s.push_back(ch);
        for (ui32 i = 1; i < token.size(); ++i) {
            if (token[i] == '\\') {
                ++i;
                switch (token[i]) {
                    case 'a':
                        ch = 0x07;
                        break;
                    case 'b':
                        ch = 0x08;
                        break;
                    case 'f':
                        ch = 0x0c;
                        break;
                    case 'n':
                        ch = 0x0a;
                        break;
                    case 'r':
                        ch = 0x0d;
                        break;
                    case 't':
                        ch = 0x09;
                        break;
                    case 'v':
                        ch = 0x0b;
                        break;
                    case '\\':
                    case '\'':
                    case '\"':
                        ch = token[i];
                        break;
                    default:
                        break;
                }
            }
            else {
                ch = token[i];
            }
            s.push_back(ch);
        }
        token = s;
    }
}

bool BinaryReader::OnRead(ui8 *pData, ui32 size) {
    // revisit, consider byte order!
    printf("\n");
    if (pData && OnHeader(pData, size) && OnSymbols(pData, size)) {
        while (size) {
            ui32 bytes;
            ui8 byte = *pData & FOUR;
            SYMBOL_ID symId;
            switch (byte) {
                case ONE:
                    symId = *pData & BINARY_STREAM_ONE_BYTE_MAX;
                    ++pData;
                    --size;
                    break;
                case TWO:
                    if (size >= 2) {
                        symId = ((*pData) & BINARY_STREAM_BYTE_MASK) << 8;
                        symId |= pData[1];
                        pData += (bytes = 2);
                        size -= bytes;
                    }
                    else {
                        ERROR(CORRUPTED_FILE);
                    }
                    break;
                case THREE:
                    if (size >= 3) {
                        symId = ((*pData) & BINARY_STREAM_BYTE_MASK) << 16;
                        symId |= pData[1] << 8;
                        symId |= pData[2];
                        pData += (bytes = 3);
                        size -= bytes;
                    }
                    else {
                        ERROR(CORRUPTED_FILE);
                    }
                    break;
                case FOUR:
                    if (size >= 4) {
                        symId = ((*pData) & BINARY_STREAM_BYTE_MASK) << 24;
                        symId |= pData[1] << 16;
                        symId |= pData[2] << 8;
                        symId |= pData[3];
                        pData += (bytes = 4);
                        size -= bytes;
                    }
                    else {
                        ERROR(CORRUPTED_FILE);
                    }
                    break;
                default:
                    ERROR(INTERNAL_ERROR);
                    return false;
            }
            if (m_error == NO_ERROR) {
                string token;
                if (m_Interpreter.GetSymbolStore().GetSymbolName(symId, token)) {
                    // process token
                    NODE_TYPE type = GetTokenType(token);
#if 1
                    //string s;
                    if (type == TYPE_CONST) { // replace  escape character
                        //EscapeString(token);
                        //if (token[0] == '\"') {
                        //    s.push_back(token[0]);
                        //    for (ui32 i = 1; i < token.size() - 1; ++i) {
                        //        switch (token[i]) {
                        //            case '\a':
                        //                s.push_back('\\');
                        //                s.push_back('a');
                        //                break;
                        //            case '\b':
                        //                s.push_back('\\');
                        //                s.push_back('b');
                        //                break;
                        //            case '\f':
                        //                s.push_back('\\');
                        //                s.push_back('f');
                        //                break;
                        //            case '\n':
                        //                s.push_back('\\');
                        //                s.push_back('n');
                        //                break;
                        //            case '\r':
                        //                s.push_back('\\');
                        //                s.push_back('r');
                        //                break;
                        //            case '\t':
                        //                s.push_back('\\');
                        //                s.push_back('t');
                        //                break;
                        //            case '\v':
                        //                s.push_back('\\');
                        //                s.push_back('v');
                        //                break;
                        //            case '\\':
                        //                s.push_back('\\');
                        //                s.push_back('\\');
                        //                break;
                        //            case '\'':
                        //                s.push_back('\\');
                        //                s.push_back('\'');
                        //                break;
                        //            case '\"':
                        //                s.push_back('\\');
                        //                s.push_back('\"');
                        //                break;
                        //            default:
                        //                s.push_back(token[i]);
                        //                break;
                        //        }
                        //    }
                        //    s.push_back(token[token.size() - 1]);
                        //}
                        //token = s;
                    }
                    //else {
                    //    s = token;
                    //}
                    //if (token == ";") {
                    //    printf("%s\n", token.c_str());
                    //}
                    //else {
                    //    printf("%s ", token.c_str());
                    //}
#endif // 0
                    if (!OnToken(type, token)) {
                        return false;
                    }
                }
                else {
                    ERROR(SYMBOL_NOT_FOUND);
                }
            }
        }
    }
    else {
        ERROR(CORRUPTED_FILE);
    }
    
    return (m_error == NO_ERROR);
}

NODE_TYPE BinaryReader::GetTokenType(string &token) {
    KEYWORDS::iterator itK = m_keywords.find(token);
    if (itK != m_keywords.end()) {
        return itK->second;
    }
    
    OPERATORS::iterator itO = m_operators.find(token);
    if (itO != m_operators.end()) {
        return itO->second;
    }

    if (IsChar(*token.c_str())) {
        return TYPE_VAR;
    }
#ifdef NUMBER
    else {
        if ((token[0] == '0') && (token[1] == 'x' || token[1] == 'X')) { // hex number
            if ((token.length() < 3) || (token.length() > 18)) {
                //ERROR(INVALID_HEX_NUMBER);
                return TYPE_NONE;
            }
            return TYPE_NUM_HEX_1;
        }
        else {
            //check last char
            char id = token[token.length() - 1];

            switch (id) {
                case 'h':
                case 'H':
                    if (token.length() > 17) {
                        //ERROR(INVALID_HEX_NUMBER);
                        return TYPE_NONE;
                    }
                    return TYPE_NUM_HEX_2;
                    break;
                case 'b':
                case 'B':
                    if (token.length() > 65) {
                        //ERROR(INVALID_BINARY_NUMBER);
                        return TYPE_NONE;
                    }
                    return TYPE_NUM_BIN;
                    break;
                case 'e':
                case 'E':
                    return TYPE_NUM_EXP;
                    break;
                default:
                    if (IsValidNum(token)) {
                        return TYPE_CONST_NUM;
                    }
                    break;
            }
        }
    }
#else // NUMBER
    if (IsValidNum(token)) { // also valid number format is (±)(N)(.)(N)(e±N) will be processed in NumberEvent()
        return TYPE_CONST_NUM;
    }
#endif // NUMBER
    return ((token[0] == '\"') || (token[0] == '\'')) ? TYPE_CONST : TYPE_NONE;
}

bool BinaryReader::OnHeader(ui8 *&pData, ui32 &size) {
    if (!m_header.Load(pData, size)) {
        ERROR(CORRUPTED_FILE);
    }

    return (m_error == NO_ERROR);
}

bool BinaryReader::OnSymbols(ui8 *&pData, ui32 &size) {
    string symbol;
    while (*pData && size) {
        if (*pData) {
            symbol.push_back((char)*pData);
        }
        ++pData;
        --size;
        if (!*pData) {
            //EscapeString(symbol);
            m_Interpreter.GetSymbolStore().UpdateSymbolMap(symbol);
            symbol.clear();
            if (!size) {
                return false;
            }
            ++pData;
            --size;
        }
    }
    
    if (*pData && (size == 0)) {
        ERROR(CORRUPTED_FILE);
    }
    else {
        ++pData;
        --size;
    }
    
    return (m_error == NO_ERROR);
}

/**************************************************************/
bool Serializer::Write(string path) {
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
    m_fileStream.open(path, ofstream::binary | ios::out);
    if (path == "CON") {
        m_fileStream.set_rdbuf(cout.rdbuf());
    }

    OnDumpHeader();
    OnDumpSymbols();
    OnDumpConsts();
    OnDumpScript();

    m_fileStream.close();
    return true;
}

void Serializer::OnDumpHeader() {
    m_Interpreter.DumpHeader(*this);
}

void Serializer::OnDumpSymbols() {
    m_Interpreter.DumpSymbols(*this);
}

void Serializer::OnDumpConsts() {
    m_Interpreter.DumpConsts(*this);
}

void Serializer::OnDumpScript() {
    m_Interpreter.DumpScript(*this);
}

bool Serializer::TranslateBegin(NODE_TYPE type, string &value) {
    switch (type) {
        case TYPE_SCOPE:
            value = "{";
            break;
        //case TYPE_BLANK_SCOPE:
        //    break;
        case TYPE_SUBSCRIPT:
            value = "[";
            break;
        case TYPE_PARENTHESIS:
            value = "(";
            break;
        case TYPE_COMMA:
            value = ",";
            break;
        case TYPE_MEMBER_ACCESS:
            value = ".";
            break;
        case TYPE_FUNCTION_REF_TYPE:
        case TYPE_FUNCTION:
            value = "function";
            break;
        case TYPE_ELSE:
            value = "else";
            break;
        case TYPE_RETURN:
            value = "return";
            break;
        case TYPE_COLON:
            value = ":";
            break;
        case TYPE_EMPTY:
            value = " ";
            break;
        case TYPE_AGGREGATE_TYPE:
            value = "struct";
            break;
        case TYPE_ARRAY:
            value = "array";
            break;
        case TYPE_LIB:
            value = "lib";
            break;
        case TYPE_BOOL:
            value = "bool";
            break;
        case TYPE_I8:
            value = "i8";
            break;
        case TYPE_UI8:
            value = "ui8";
            break;
        case TYPE_I16:
            value = "i16";
            break;
        case TYPE_UI16:
            value = "ui16";
            break;
        case TYPE_I32:
            value = "i32";
            break;
        case TYPE_UI32:
            value = "ui32";
            break;
        case TYPE_I64:
            value = "i64";
            break;
        case TYPE_UI64:
            value = "ui64";
            break;
        case TYPE_FLOAT:
            value = "float";
            break;
        case TYPE_DOUBLE:
            value = "double";
            break;
        case TYPE_STRING:
            value = "string";
            break;
        case TYPE_LESS:
            value = "<";
            break;
        case TYPE_QUOTATION_MARK:
            value = "\"";
            break;
        case TYPE_ERROR:
            value = "error";
            break;
        default:
            return false;
    }
    return true;
}

bool Serializer::TranslateEnd(NODE_TYPE type, string &value) {
    switch (type) {
        case TYPE_SCOPE_NO_NEW_LINE:
        case TYPE_SCOPE:
            value = "}";
            break;
        //case TYPE_BLANK_SCOPE:
        //    break;
        //case TYPE_NEW_LINE:
        //    //value = "\n";
        //    break;
        case TYPE_SUBSCRIPT:
            value = "]";
            break;
        case TYPE_PARENTHESIS:
            value = ")";
            break;
        case TYPE_COMMA:
            value = ",";
            break;
        case TYPE_SEMICOLON:
            value = ";";
            break;
        case TYPE_SCOPE_END_EXPRESSION:
            value = ";";//"; \n";
            break;
        case TYPE_GREATER:
            value = ">";
            break;
        case TYPE_COLON:
            value = ":";
            break;
        default:
            return false;
    }
    return true;
}

/**************************************************************/
bool TextSerializer::EscapeString(ui8 *pData, ui32 size, string &out) {
    //string s;
    char ch = *pData;
    if ((ch == '\"') || (ch == '\'')) {
        out.push_back(ch);
        for (ui32 i = 1; i < size - 1; ++i) {
            switch (pData[i]) {
                case '\a':
                    out.push_back('\\');
                    out.push_back('a');
                    break;
                case '\b':
                    out.push_back('\\');
                    out.push_back('b');
                    break;
                case '\f':
                    out.push_back('\\');
                    out.push_back('f');
                    break;
                case '\n':
                    out.push_back('\\');
                    out.push_back('n');
                    break;
                case '\r':
                    out.push_back('\\');
                    out.push_back('r');
                    break;
                case '\t':
                    out.push_back('\\');
                    out.push_back('t');
                    break;
                case '\v':
                    out.push_back('\\');
                    out.push_back('v');
                    break;
                case '\\':
                    out.push_back('\\');
                    out.push_back('\\');
                    break;
                case '\'':
                    out.push_back('\\');
                    out.push_back('\'');
                    break;
                case '\"':
                    out.push_back('\\');
                    out.push_back('\"');
                    break;
                default:
                    out.push_back(pData[i]);
                    break;
            }
        }
        out.push_back(pData[size - 1]);
        return true;
    }
    return false;
}

bool TextSerializer::WriteRaw(ui8* pData, ui32 size) {
    if (size == 1) {
        m_fileStream.write((char*)pData, size);
    }
    else {
        string s;
        if (EscapeString(pData, size, s)) {
            m_fileStream.write(s.c_str(), s.length());
        }
        else {
            m_fileStream.write((char*)pData, size);
        }
    }
    return true; 
}

//bool TextSerializer::Write(string path) {
//    if (m_fileStream.is_open()) {
//        m_fileStream.close();
//    }
//    m_fileStream.open(path, ofstream::binary);
//
//    m_Interpreter.DumpScript(*this);
//
//    m_fileStream.close();
//    return true;
//}

bool TextSerializer::WriteSymbol(SYMBOL_DESC &symDesc) {
    if (m_pSymbolStore) {
        string symbol;
        if (m_pSymbolStore->GetSymbolName(symDesc.m_symId, symbol)) {
            if (m_newLine) {
                WriteRaw((ui8*)"\n", 1);
                WriteRaw((ui8*)m_tabs.c_str(), m_tabs.length());
                m_newLine = false;
            }

            WriteRaw((ui8*)symbol.c_str(), symbol.length());
        }
    }
    return true;
}

bool TextSerializer::Begin(NODE_TYPE type) {
    string value;
    if (TranslateBegin(type, value)) {
        if (m_newLine) {
            WriteRaw((ui8*)"\n", 1);
            WriteRaw((ui8*)m_tabs.c_str(), m_tabs.length());
            m_newLine = false;
        }
        WriteRaw((ui8*)value.c_str(), value.length());
        if (type == TYPE_SCOPE) {
            m_tabs += "    ";
            m_newLine = true;
        }
    }
    return true;
}

bool TextSerializer::End(NODE_TYPE type) {
    string value;
    if (TranslateEnd(type, value)) {
        if (m_newLine && (type != TYPE_SCOPE_END_EXPRESSION)) {
            WriteRaw((ui8*)"\n", 1);
            if ((type == TYPE_SCOPE) || (type == TYPE_SCOPE_NO_NEW_LINE)) {
                m_tabs.erase(m_tabs.length() - 4, 4);
            }
            WriteRaw((ui8*)m_tabs.c_str(), m_tabs.length());
            m_newLine = false;
        }
        switch (type) {
            case TYPE_SCOPE:
                m_newLine = true;
                break;
            case TYPE_SCOPE_NO_NEW_LINE:
                m_newLine = false;
                break;
            case TYPE_SCOPE_END_EXPRESSION:
                m_newLine = true;
                break;
            case TYPE_COLON:
                m_tabs += "    ";
                m_newLine = true;
                break;
            case TYPE_NEW_LINE:
                m_newLine = true;
                break;
            default:
                break;
        }
        WriteRaw((ui8*)value.c_str(), value.length());
    }
    return true;
}

/*************************************************************/
BinarySerializer::BinarySerializer(InterpreterIf &interpreter) :
    Serializer(interpreter) {
}

BinarySerializer::~BinarySerializer() {
    m_fileStream.close();
}

//bool BinarySerializer::Write(string path) {
//    if (m_fileStream.is_open()) {
//        m_fileStream.close();
//    }
//    m_fileStream.open(path, ofstream::binary);
//    
//    m_Interpreter.Dump(*this);
//
//    m_fileStream.close();
//    return true;
//}

bool BinarySerializer::WriteRaw(ui8* pData, ui32 size) {
    if (pData) {
        m_fileStream.write((char*)pData, size);
    }
    return true;
}

bool BinarySerializer::WriteSymbol(SYMBOL_DESC &symDesc) {
    // must revisit, consider byte order!
    ui8 pData[sizeof(ui32)];
    ui32 size;
    if (m_pSymbolStore) {
        if (symDesc.m_symId <= BINARY_STREAM_ONE_BYTE_MAX) {
            pData[0] = (ui8)symDesc.m_symId;
            size = sizeof(ui8);
        }
        else if (symDesc.m_symId <= BINARY_STREAM_TWO_BYTE_MAX) {
            pData[0] = TWO | (ui8)(symDesc.m_symId >> 8);
            pData[1] = (ui8)symDesc.m_symId;
            size = sizeof(ui8) * 2;
        }
        else if (symDesc.m_symId <= BINARY_STREAM_THREE_BYTE_MAX) {
            pData[0] = THREE | (ui8)(symDesc.m_symId >> 16);
            pData[1] = (ui8)(symDesc.m_symId >> 8);
            pData[2] = (ui8)symDesc.m_symId;
            size = sizeof(ui8) * 3;
        }
        else if (symDesc.m_symId <= BINARY_STREAM_FOUR_BYTE_MAX) {
            pData[0] = THREE | (ui8)(symDesc.m_symId >> 24);
            pData[1] = (ui8)(symDesc.m_symId >> 16);
            pData[2] = (ui8)(symDesc.m_symId >> 8);
            pData[3] = (ui8)symDesc.m_symId;
            size = sizeof(ui8) * 4;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
    WriteRaw(pData, size);
    return true;
}

bool BinarySerializer::Begin(NODE_TYPE type) {
    SYMBOL_ID symId;
    string value;
    if ((type != TYPE_EMPTY) && TranslateBegin(type, value) && m_pSymbolStore && m_pSymbolStore->GetSymbolId(value, symId)) {
        WriteSymbol(SYMBOL_DESC(symId));
    }
    return true;
}

bool BinarySerializer::End(NODE_TYPE type) {
    SYMBOL_ID symId;
    string value;
    if (TranslateEnd((type == TYPE_SCOPE_END_EXPRESSION) ? TYPE_SEMICOLON : type, value) && m_pSymbolStore && m_pSymbolStore->GetSymbolId(value, symId)) {
        WriteSymbol(SYMBOL_DESC(symId));
    }
    return false;
}

/*************************************************************/

/*

template<class T0> struct N1 {
T0 v0;
};
template<class T0, class T1> struct N2 {
T0 v0;
T1 v1;
};
template<class T0, class T1, class T2 > struct N3 {
T0 v0;
T1 v1;
T2 v2;
};
template<class T0, class T1, class T2, class T3 > struct N4 {
T0 v0;
T1 v1;
T2 v2;
T3 v3;
};

template<class T> T Get1() {
T v = {1};
return v;
}

template<class T> T Get2() {
T v = { 1 , 2};
return v;
}

template<class T> T Get3() {
T v = { 1 , 2, 3 };
return v;
}
template<class T> T Get4() {
T v = { 1 , 2, 3 , 4};
return v;
}

struct T5 {
char v0;
char v1;
char v2;
char v3;
char v4;
};
T5 Get5() {
T5 v = { 1,2,3,4,5 };
return v;
}
struct T6 {
char v0;
char v1;
char v2;
char v3;
char v4;
char v5;
};
T6 Get6() {
T6 v = { 1,2,3,4,5,6 };
return v;
}

N2<char, char> v11;
N2<char, short> v12;
N2<short, char> v21;
N2<short, short> v22;
N2<char, long> v14;
N2<long, char> v41;
N2<short, long> v24;
N2<long, short> v42;

N3<char, char, char> v111;
N3<char, char, short> v112;
N3<char, short, char> v121;
N3<char, short, short> v122;
N3<short, char, char> v211;
N3<short, char, short> v212;
N3<short, short, char> v221;
N3<short, short, short> v222;
void TestGetStructAll() {
//  TT(CCC, char, char, short, char) v0;
//N4<char, char, short, char> v1121;
N1<char> v1;
N1<short> v2;
N1<long> v4;
N1<double> v8;

N2<char, char> v11;
N2<char, short> v12;
N2<short, char> v21;
N2<short, short> v22;
N2<char, long> v14;
N2<long, char> v41;
N2<short, long> v24;
N2<long, short> v42;

N3<char, char, char> v111;
N3<char, char, short> v112;
N3<char, short, char> v121;
N3<char, short, short> v122;
N3<short, char, char> v211;
N3<short, char, short> v212;
N3<short, short, char> v221;
N3<short, short, short> v222;

N4<char, char, char, char> v1111;
N4<char, char, char, short> v1112;
N4<char, char, short, char> v1121;
N4<char, char, short, short> v1122;
N4<char, short, char, char> v1211;
N4<char, short, char, short> v1212;
N4<char, short, short, char> v1221;
N4<char, short, short, short> v1222;
N4<short, char, char, char> v2111;
N4<short, char, char, short> v2112;
N4<short, char, short, char> v2121;
N4<short, char, short, short> v2122;
N4<short, short, char, char> v2211;
N4<short, short, char, short> v2212;
N4<short, short, short, char> v2221;
N4<short, short, short, short> v2222;
//N4<long, long, long, long> v4444;
T5 v11111;
T6 v111111;
v8 = Get1<N1<double> >();// eax == HI, edx == LO, size == 8
v11111 = Get5();//   push eax, at eax: 01 02 03 04 05,    size == 5
v111111 = Get6();//  push eax, at eax: 01 02 03 04 05 06, size == 6
v1 = Get1<N1<char> >();  //        eax == 0xcccccc01, size == 1
v2 = Get1<N1<short> >(); //        eax == 0x00000001, size == 2
v4 = Get1<N1<long> >();  //        eax == 0x00000001, size == 4

v11 = Get2<N2<char, char> >(); //  eax == 0xcccc0201, size == 2
v12 = Get2<N2<char, short> >(); // eax == 0x0002cc01, size == 4
v21 = Get2<N2<short, char> >(); // eax == 0xcc020001, size == 4
v22 = Get2<N2<short, short> >();// eax == 0x00020001, size == 4
v14 = Get2<N2<char, long> >(); //  eax == 0xcccccc01 edx == 0x00000002, size == 8
v41 = Get2<N2<long, char> >(); //  eax == 0x00000001 edx == 0xcccccc02, size == 8
v24 = Get2<N2<short, long> >();//  eax == 0xcccc0001 edx == 0x00000002, size == 8
v42 = Get2<N2<long, short> >();//  eax == 0x00000001 edx == 0xcccc0002, size == 8

v111 = Get3<N3<char, char, char> >(); //  push eax , at eax: 01 02 03,          size == 3
v112 = Get3<N3<char, char, short> >();//            eax == 0x00030201,          size == 4
v121 = Get3<N3<char, short, char> >(); // push eax, at eax: 01 cc 02 00 03,     size == 6
v122 = Get3<N3<char, short, short> >();// push eax, at eax: 01 cc 02 00 03 00,  size == 6
v211 = Get3<N3<short, char, char> >(); //           eax == 0x03020001,          size == 4
v212 = Get3<N3<short, char, short> >();// push eax, at eax: 01 00 02 cc 03 00,  size == 6
v221 = Get3<N3<short, short, char> >();// push eax, at eax: 01 00 02 00 03,     size == 6
v222 = Get3<N3<short, short, short> >();//push eax, at eax: 01 00 02 00 03 00,  size == 6

//v4444 = Get4<N4<long, long, long, long> >();//             at eax:
v1111 = Get4<N4<char, char, char, char> >();//             eax == 0x04030201,            size == 4
v1112 = Get4<N4<char, char, char, short> >();//  push eax, at eax: 01 02 03 cc 04 00,    size == 6
v1121 = Get4<N4<char, char, short, char> >(); // push eax, at eax: 01 02 03 00 04,       size == 6
v1122 = Get4<N4<char, char, short, short> >();// push eax, at eax: 01 02 03 00 04 00,    size == 6
v1211 = Get4<N4<char, short, char, char> >(); // push eax, at eax: 01 cc 02 00 03 04,    size == 6
v1212 = Get4<N4<char, short, char, short> >();//  eax == 0x0002cc01, edx == 0x0004cc03,  size == 8
v1221 = Get4<N4<char, short, short, char> >();//  eax == 0x0002cc01, edx == 0xcc040003,  size == 8
v1222 = Get4<N4<char, short, short, short> >();// eax == 0x0002cc01 , edx == 0x00040003, size == 8
v2111 = Get4<N4<short, char, char, char> >();// push eax, at eax: 01 00 02 03 04,        size == 6
v2112 = Get4<N4<short, char, char, short> >();//push eax, at eax: 01 00 02 03 04 00,     size == 6
v2121 = Get4<N4<short, char, short, char> >();//  eax == 0xcc020001, edx == 0xcc040003,  size == 8
v2122 = Get4<N4<short, char, short, short> >();// eax == 0xcc020001, edx == 0x00040003,  size == 8
v2211 = Get4<N4<short, short, char, char> >();//push eax, at eax: 01 00 02 00 03 04,     size == 6
v2212 = Get4<N4<short, short, char, short> >();// eax == 0x00020001, edx == 0x0004cc03,  size == 8
v2221 = Get4<N4<short, short, short, char> >();// eax == 0x00020001, edx == 0xcc040003,  size == 8
v2222 = Get4<N4<short, short, short, short> >();//eax == 0x00020001, edx == 0x00040003,  size == 8

}

*/