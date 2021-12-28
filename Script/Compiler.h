#pragma once
#include <unordered_map>
#include <stack>
#include <string>
#include <iostream>
#include <fstream>

#include "Error.h"
#include "Basic.h"
#include "Expressions.h"

using namespace std;


namespace script {

    typedef enum BasicSymbolIds {
        SYMBOL_ID_BLANK = 0,
        SYMBOL_ID_PARENTHESIS,
        SYMBOL_ID_OPENPARENTHESIS,
        SYMBOL_ID_SUBSCRIPT,
        SYMBOL_ID_OPENSUBSCRIPT,
        BASIC_SYMBOL_COUNT
    } BASIC_SYMBOL_IDS;

    /**************************************************************/
    class SymbolStore {
        SymbolStore(const SymbolStore&);
        SymbolStore& operator=(const SymbolStore&);

    public:
        SymbolStore();
        virtual ~SymbolStore() { s_pSymbolStore = 0; };
    
        static SymbolStore* GetSymbolStore() { return s_pSymbolStore; }

        bool Reset();
        SYMBOL_ID UpdateSymbolMap(string& name);
        bool GetSymbolName(SYMBOL_ID symId, string &name);
        bool GetSymbolName(Node * const pNode, string &name);
        bool GetSymbolId(string &name, SYMBOL_ID &symId);
        ui32 GetFileId(string fileName);
        bool GetFileName(ui32 fileId, string &name);
        void GetFileNames(vector<string> &fileNames);
        bool Dump(Serializer &serializer);
        //static SymbolStore& GetInstance();

    protected:
        typedef unordered_map<string, SYMBOL_ID>    SYMBOL_MAP;
        typedef vector<SYMBOL_MAP::iterator>        SYMBOL_MAP_REVERSED;
        typedef unordered_map<string, ui32>         FILE_ID_MAP;
        typedef vector<SYMBOL_MAP::iterator>        FILE_ID_MAP_REVERSED;

        static SymbolStore     *s_pSymbolStore;
        // symbol lookup containers
        SYMBOL_MAP              m_NameToSymbolIdMap;
        SYMBOL_MAP_REVERSED     m_SymboIdToName;
        FILE_ID_MAP             m_FileNameToIdMap;
        FILE_ID_MAP_REVERSED    m_FileIdToName;
        SYMBOL_ID               m_lastBaseSymbolId;
    private:
        void InitCoreSymbolsMap();
    };
  /**************************************************************/
    class Reader;

    class BinaryHeader {
        BinaryHeader(const BinaryHeader&);
        BinaryHeader& operator=(const BinaryHeader&);
    public:
        BinaryHeader();
        ~BinaryHeader() {};
        bool Load(ui8 *&pData, ui32 &size);
        void Save(Serializer &serializer);

    protected:
        string m_scriptSignature,
               m_timeStatmp;
        ui16   m_scriptMajorVersion,
               m_scriptMinorVersion;
    };
    /**************************************************************/

    class InterpreterIf {
    public:
        InterpreterIf(Error &error, StdOutInterface *pStdOutInterface) : m_error(error), 
            m_Memory(&m_symbolStore,
#ifdef SCRIPT_DEBUGGER
            &m_debuggerHandler,
#endif // SCRIPT_DEBUGGER
            pStdOutInterface) {};
        virtual ~InterpreterIf() {};
        virtual bool Init() = 0;
        virtual bool DispatchEvent(NODE_TYPE type, string &token) = 0;
        virtual void DumpHeader(Serializer &serializer) = 0;
        virtual void DumpSymbols(Serializer &serializer) = 0;
        virtual void DumpConsts(Serializer &serializer) = 0;
        virtual void DumpScript(Serializer &serializer) = 0;
        virtual bool Run() = 0;
        virtual void PreRun() {};
        virtual void PostRun() {};
        virtual Node* GetRootNode() = 0;

        inline SymbolStore& GetSymbolStore() { return m_symbolStore; };
        inline Memory& GetMemory() { return m_Memory; };
#ifdef SCRIPT_DEBUGGER
        inline Debugger& GetDebugger() { return m_debugger; };
        inline DebuggerHandler& GetDebuggerHandler() { return m_debuggerHandler; };
#endif // SCRIPT_DEBUGGER

//#ifdef _DEBUG
        void SetReader(Reader *pReader) { m_pReader = pReader; };
        Reader* GetReader() { return m_pReader; };
//#endif // _DEBUG
    protected:
        Error        &m_error;
        SymbolStore  m_symbolStore;
        Memory       m_Memory;
//#ifdef _DEBUG
        Reader      *m_pReader;
//#endif // _DEBUG
#ifdef SCRIPT_DEBUGGER
        Debugger          m_debugger;
        DebuggerHandler   m_debuggerHandler;
#endif // SCRIPT_DEBUGGER
    };
    /**************************************************************/
    class InterpreterTest : public InterpreterIf {
        InterpreterTest();
        InterpreterTest(const InterpreterTest&);
        InterpreterTest& operator=(const InterpreterTest&);

    public:
        InterpreterTest(Error &error) : InterpreterIf(error, 0) {};
        virtual ~InterpreterTest() {};
        virtual bool Init() { return true; };
        virtual void DumpHeader(Serializer &serializer) {};
        virtual void DumpSymbols(Serializer &serializer) {};
        virtual void DumpConsts(Serializer &serializer) {};
        virtual void DumpScript(Serializer &serializer) {};
        virtual bool Run() { return true; };
        virtual Node* GetRootNode() { return 0; };
    protected:
        virtual bool DispatchEvent(NODE_TYPE type, string &token);
    };
    /**************************************************************/
    class Type;

    class Interpreter : public InterpreterIf {
        Interpreter();
        Interpreter(const Interpreter&);
        Interpreter& operator=(const Interpreter&);

    public:
        Interpreter(Error &error, StdOutInterface *pStdOutInterface);
        virtual ~Interpreter();
        virtual void DumpHeader(Serializer &serializer);
        virtual void DumpSymbols(Serializer &serializer);
        virtual void DumpConsts(Serializer &serializer);
        virtual void DumpScript(Serializer &serializer);
        virtual bool Run();
#ifdef SCRIPT_DEBUGGER
        virtual void PreRun(DebuggerEvents *pDebuggerEvents);
        virtual void PostRun();
#else // SCRIPT_DEBUGGER
#endif // SCRIPT_DEBUGGER
        virtual Node* GetRootNode() { return m_pRunableRoot; };

        bool Init();
        void TestNum(string &val);
        void DeleteRunable();
        bool IsValidEndState();
        bool PushIncludePath(string &path, ui32 &win32Error);
#ifdef TRACE_OBJECTS
        void Loaded(bool val) { m_loaded = val; }
#endif // TRACE_OBJECTS
    protected:
        virtual bool DispatchEvent(NODE_TYPE type, string &token);

    private:
        typedef bool(Interpreter::*INTERPRETER_STATE_EVENT)(NODE_TYPE, string&);
        typedef vector<INTERPRETER_STATE_EVENT>             STATE_EVENTS;
        typedef MEMORY_OFFSET(Interpreter::*RESOLVE_VARIABLE_FOO)(SYMBOL_ID, NODE_TYPE, NODE_TYPE&);
        typedef MEMORY_OFFSET(Interpreter::*RESERVE_STACK_SOPT_IF_NOT_DECALRED)(SYMBOL_ID, MEMORY_OFFSET);

        // factory methods
        Function* CreateBuiltInFunction(SymbolStore *pSymbolStore, NODE_TYPE type, SYMBOL_ID symId);
        template<class T> T* CreateNode(SYMBOL_ID symId);
        template<class T = Node> T* CreateNode(string &token);
        template<class T = Node> T* CreateNode(SymbolStore *pSymbolStore, string &token);
        template<class T = Node> T* CreateNode(NODE_TYPE type, string &token);

        Variable* CreateVariableNode(Type *pType, string &token);
        Variable* CreateArgNode(string &token);
        Variable* CreateMemberVariableNode(string &token);
        FunctionLibPtr *CreateFunctionLibPtr(Node* pParent, string &token);
        Variable* CreateMemberDecNode(string &token);
        Function* GetBuiltInFunction(NODE_TYPE type, string &token);

        //state events

        bool ScopeEvent(NODE_TYPE type, string &token);
        bool ExpEvent(NODE_TYPE type, string &token);
        // function
        bool FunctionTypeEventHandler(NODE_TYPE type, string &token, INTERPRETER_STATE_EVENT fnEvent);
        bool FunctionTypeEvent(NODE_TYPE type, string &token);
        bool FunctionArgTypeEvent(NODE_TYPE type, string &token);
        bool FunctionBeginRefDecEvent(NODE_TYPE type, string &token);
        bool FunctionEndRefDecEvent(NODE_TYPE type, string &token);
        // return is a function<>
        bool FunctionFunctionReturnTypeEvent(NODE_TYPE type, string &token);
        bool FunctionBeginFunctionRefReturnEvent(NODE_TYPE type, string &token);
        bool FunctionEndFunctionRefReturnEvent(NODE_TYPE type, string &token);
        //array
        bool ArrayDecEvent(NODE_TYPE type, string &token);
        bool ArrayBeginEvent(NODE_TYPE type, string &token);
        bool ArrayTypeEvent(NODE_TYPE type, string &token);
        bool ArrayPostTypeEvent(NODE_TYPE type, string &token);
        bool ArrayEndTypeEvent(NODE_TYPE type, string &token);
        bool ArrayCloseEvent(NODE_TYPE type, string &token);
        bool ArrayDimensionEvent(NODE_TYPE type, string &token);
        bool ArrayDimExpEvent(NODE_TYPE type, string &token);
        bool ArrayDimensionEndEvent(NODE_TYPE type, string &token);
        bool ArrayBeginFunctionTypeEvent(NODE_TYPE type, string &token);
        bool ArrayEndFunctionTypeEvent(NODE_TYPE type, string &token);
        bool ArrayFunctionTypeEvent(NODE_TYPE type, string &token);
        bool ArrayBeginSubscriptEvent(NODE_TYPE type, string &token);
        bool ArrayEndSubscriptEvent(NODE_TYPE type, string &token);
        bool OpenSubscriptEvent(NODE_TYPE type, string &token);
        // arg is a function<>
        bool FunctionFunctionArgTypeEvent(NODE_TYPE type, string &token);
        bool FunctionBeginFunctionRefArgEvent(NODE_TYPE type, string &token);
        bool FunctionEndFunctionRefArgEvent(NODE_TYPE type, string &token);
        bool FunctionNameEvent(NODE_TYPE type, string &token);
        bool FunctionDecEvent(NODE_TYPE type, string &token);
        bool FunctionArgEvent(NODE_TYPE type, string &token);
        bool FunctionNextArgEvent(NODE_TYPE type, string &token);
        bool FunctionBodyEvent(NODE_TYPE type, string &token);
        bool FunctionCallEvent(NODE_TYPE type, string &token);
        bool FunctionCallArgEvent(NODE_TYPE type, string &token);
        bool FunctionCallNextArgEvent(NODE_TYPE type, string &token);

        // conditional
        bool ConditionalDecEvent(NODE_TYPE type, string &token);
        // if
        bool IfEvent(NODE_TYPE type, string &token);
        bool IfBodyEvent(NODE_TYPE type, string &token);
        bool ElseEvent(NODE_TYPE type, string &token);
        bool ElseBodyEvent(NODE_TYPE type, string &token);
        // for
        bool ForEvent(NODE_TYPE type, string &token);
        bool ForInitEvent(NODE_TYPE type, string &token);
        bool ForCondEvent(NODE_TYPE type, string &token);
        bool ForLoopEvent(NODE_TYPE type, string &token);
        bool ForBodyEvent(NODE_TYPE type, string &token);
        // switch
        bool SwitchEvent(NODE_TYPE type, string &token);
        bool SwitchBodyEvent(NODE_TYPE type, string &token);
        bool SwitchCasesEvent(NODE_TYPE type, string &token);
        bool CaseEvent(NODE_TYPE type, string &token);
        bool CaseValueEvent(NODE_TYPE type, string &token);
        bool CaseColonEvent(NODE_TYPE type, string &token);
        bool CaseBodyEvent(NODE_TYPE type, string &token);
        bool DefaultEvent(NODE_TYPE type, string &token);
        bool DefaultBodyEvent(NODE_TYPE type, string &token);
        // while
        bool WhileEvent(NODE_TYPE type, string &token);
        bool WhileBodyEvent(NODE_TYPE type, string &token);
        bool BreakEvent(NODE_TYPE type, string &token);
        bool ContinueEvent(NODE_TYPE type, string &token);
        // return
        bool ReturnEvent(NODE_TYPE type, string &token);
        // include
        bool IncludeEvent(NODE_TYPE type, string &token);
        // lib events
        bool LibDecEvent(NODE_TYPE type, string &token);
        bool LibPathEvent(NODE_TYPE type, string &token);
        bool LibBeginBodyEvent(NODE_TYPE type, string &token);
        bool LibBodyEvent(NODE_TYPE type, string &token);
        bool LibFunctionTypeEventHandler(NODE_TYPE type, string &token);
        bool LibFunctionNameEvent(NODE_TYPE type, string &token);
        bool LibFunctionDecEvent(NODE_TYPE type, string &token);
        bool LibFunctionArgTypeEvent(NODE_TYPE type, string &token);
        bool LibFunctionNextArgTypeEvent(NODE_TYPE type, string &token);
        bool LibVarDecEvent(NODE_TYPE type, string &token);
        bool LibFunctionCallEvent(NODE_TYPE type, string &token);
        bool LibFunctionFunctionTypeEvent(NODE_TYPE type, string &token);
        bool LibFunctionBeginRefDecEvent(NODE_TYPE type, string &token);
        bool LibFunctionEndRefDecEvent(NODE_TYPE type, string &token);
        // member access
        bool MemberAccessEvent(NODE_TYPE type, string &token);
        bool BeginMemberEvent(NODE_TYPE type, string &token);
        // aggregate type events
        bool AggregateEvent(NODE_TYPE type, string &token);
        bool AggregateBeginEvent(NODE_TYPE type, string &token);
        bool AggregateBodyEvent(NODE_TYPE type, string &token);
        bool AggregateEndEvent(NODE_TYPE type, string &token);
        bool AggregateMemberDecEvent(NODE_TYPE type, string &token);
        bool AggregateMemberDecNextEvent(NODE_TYPE type, string &token);
        bool AggregateVarDecEvent(NODE_TYPE type, string &token);
        bool AggregateVarDecNextEvent(NODE_TYPE type, string &token);
        bool AggregateFunctionTypeEvent(NODE_TYPE type, string &token);
        bool AggregateFunctionBeginRefDecEvent(NODE_TYPE type, string &token);
        bool AggregateFunctionEndRefDecEvent(NODE_TYPE type, string &token);
        // error related events
        bool ErrorBeginEvent(NODE_TYPE type, string &token);
        bool ErrorInstanceEvent(NODE_TYPE type, string &token);
        bool ErrorEndEvent(NODE_TYPE type, string &token);
        bool ErrorBodyEvent(NODE_TYPE type, string &token);
        //type related
        bool TypeInitEvent(NODE_TYPE type, string &token);
        bool TypeDecEvent(NODE_TYPE type, string &token);
        bool TypeDecNextEvent(NODE_TYPE type, string &token);
        //number related
#ifdef NUMBER
        bool GetHex1Number(string &token, string &value);
        bool GetHex2Number(string &token, string &value);
        bool GetBinNumber(string &token, string &value);
        bool GetFixedNumber(NODE_TYPE type, string &token, string &value);
        bool IsExpNumber(string &token);
        bool ExpNumberTokenEvent(NODE_TYPE type, string &token);
#else // NUMBER
        bool UnknownTokenEvent(NODE_TYPE type, string &token);
#endif // NUMBER
        bool NumberSignEvent(NODE_TYPE type, string &token);
        bool NumberEvent(NODE_TYPE type, string &token);
        bool DecimalPointEvent(NODE_TYPE type, string &token);
        bool FractionEvent(NODE_TYPE type, string &token);
        bool ExponentSignEvent(NODE_TYPE type, string &token);
        bool ExponentEvent(NODE_TYPE type, string &token);
        // begin expression handlers
        bool BeginFullExpression(NODE_TYPE type, string &token);
        bool BeginScopeExpression(NODE_TYPE type, string &token);

        bool BeginIfExpression(NODE_TYPE type, string &token);
        bool BeginSwitchExpression(NODE_TYPE type, string &token);
        bool BeginForExpression(NODE_TYPE type, string &token);
        bool BeginWhileExpression(NODE_TYPE type, string &token);
        bool BeginFunctionCallExpression(NODE_TYPE type, string &token);
        bool BeginReturnExpression(NODE_TYPE type, string &token);
        // cast event handlers
        bool CastOpenTypeExpression(NODE_TYPE type, string &token); // <
        bool CastBeginTypeExpression(NODE_TYPE type, string &token); // expectiong type
        bool CastBeginFunctionTypeEvent(NODE_TYPE type, string &token);
        bool CastFunctionTypeEvent(NODE_TYPE type, string &token);
        bool CastEndFunctionTypeEvent(NODE_TYPE type, string &token);
        bool CastCloseTypeExpression(NODE_TYPE type, string &token); // >

        //helpers
        Type* GetCallerType(ui32 &varIdx);
        bool EndScope();
        bool OpenParenthesis(string &token);
        bool CloseParenthesis(string &token);
        bool OpenSubscript();
        bool CloseSubscript();
        //bool FinalFunctionCall();
        Node* GetOpenParenthesisNode();
        bool ProcessNumber();
        bool AppendValueNode2Op(Node* pNode);
        bool AppendPreOpNode2Op(Node* pNode);
        bool Append2Op(NODE_TYPE type, string &token);
        bool AppendValueNode2Arg(Node* pNode);
        bool AppendValueNode2For(NODE_TYPE type, string &token, ForNode::APPEND_VALUE_NODE_2_FOR fnAppendValueNode);
        bool AppendValueNode2Subscript(Node* pNode);
        MEMORY_OFFSET ReserveAlignedStackSpotIfNotDeclared(SYMBOL_DESC &symDesc, Type *pType, ui32 &idx);
        MEMORY_OFFSET ResolveAlignedVariable(SYMBOL_ID symId, Type *&pResolvedType, ui32& idx);
        bool AlignAggregateType();
        bool ResolveArguments(Function *pFunction);
        //ErrorType* GetErrorTypeNode();
        //bool ResolveErrorArgument(ErrorNode *pErrorNode);
        AggregateType* FindTypeDefinition(SYMBOL_ID symId);
        Node* GetDefinedSymbolNode(SYMBOL_ID symId);
        bool AddAggregateSymbol(SYMBOL_ID symId);
        //bool IsAggregateType(Node* pNode);
        //Variable* GetFunctionCallLastArg(FunctionCall *pFunctionCall);
        //bool ValidateArraySubscript(bool lastSubscript);
        bool ApplyPrePostExpressions(Node *pExpRootNode, PrePostExpressions *pRePostExp);
        bool IsSizeOf(Node *pNode);
        bool ErrorIfDynamicArray(Type *pType);
        bool GetUI64Number(string &str, int begin, int end, ui64 &result);
        //bool UnknownToken2Number(string &token, ui64 &value64);
        // states
        STATE_EVENTS     m_stateEventStack;
        //runable references
        Scope           *m_pRunableRoot;
        Node            *m_pRunableCurrent;
        unordered_set<string> m_includedFiles;
#ifdef TRACE_OBJECTS
        bool            m_loaded;
#endif // TRACE_OBJECTS
        typedef struct _SIGN {
            typedef enum {
                POSITIVE,
                NEGATIVE     = 1,
                POSITIVE_EXP = 2,
                NEGATIVE_EXP = 4,
                POSITIVE_POSITIVE_EXP = POSITIVE | POSITIVE_EXP,
                NEGATIVE_POSITIVE_EXP = NEGATIVE | POSITIVE_EXP,
                POSITIVE_NEGATIVE_EXP = POSITIVE | NEGATIVE_EXP,
                NEGATIVE_NEGATIVE_EXP = NEGATIVE | NEGATIVE_EXP,
            } SIGN;

            bool Set(string &value);
            Value::VALUE_TYPE GetType();
            inline ui32 GetUI32() { return (NEGATIVE & sign) ? -(i32)number : (ui32)number; };
            bool GetValue(Value &value);
            float GetFloat();
            double GetDouble();
        private:
            SIGN sign;
            ui64 number;
            ui32 fraction;
            ui32 fractionForwardZeros;
            ui32 exponent;
            inline bool IsNum(char ch) { return (ch >= '0') && (ch <= '9'); };
            void Clear() { memset(this, 0, sizeof(*this)); exponent = -1; };
        } FLOAT_NUM;

#ifdef SCRIPT_DEBUGGER
        //FlowSemaphor    m_FlowSemaphor;
        BREAK_POINT_LINES_BY_FILE_ID    m_breakPointLines;
#endif // SCRIPT_DEBUGGER

        static string     s_semicolon;
        static string     s_empty;
    };
    /**************************************************************/
    class Reader {
        Reader();
        Reader(const Reader&);
        Reader& operator=(const Reader&);

    public:
        Reader(bool recursiveReader, InterpreterIf &interpreter, Error &error);
        virtual ~Reader() {};
        virtual bool Read(string path, ui32 &win32Error);
//#ifdef _DEBUG
        inline ui32 GetLine() { return m_line; };
        inline ui32 GetPosition() { return m_pos; };
        inline ui32 GetFileId() { return m_Interpreter.GetSymbolStore().GetFileId(m_filePath); };
//#endif // _DEBUG
        string GetFilePath() { return m_filePath; }
    protected:
        virtual bool OnInit(ui8 *pData, ui32 size);
        virtual bool OnRead(ui8 *pData, ui32 size) = 0;
        virtual bool OnToken(NODE_TYPE type, string &token);

        inline bool IsChar(char ch) { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_'); };
        inline bool IsNum(char ch) { return (ch >= '0' && ch <= '9'); };

        bool IsValidNum(string &token);

        typedef unordered_map<string, NODE_TYPE>    KEYWORDS;
        typedef KEYWORDS                            OPERATORS;

        Error         &m_error;
        InterpreterIf &m_Interpreter;
        KEYWORDS       m_keywords;
        OPERATORS      m_operators;

        string         m_filePath;
        ui32           m_line,
                       m_pos;
        bool           m_recursiveReader;
    };
    /**************************************************************/
    class ScriptReader : public Reader {
        ScriptReader();
        ScriptReader(const ScriptReader&);
        ScriptReader& operator=(const ScriptReader&);

    public:
        ScriptReader(bool recursiveReader, InterpreterIf &interpreter, Error &error);
        virtual ~ScriptReader() {};

    protected:
        virtual bool OnRead(ui8 *pData, ui32 size);
    private:
        bool ProcessOperator(char *&pScript, string &token);
        bool ProcessToken(string& token);
        bool SkipLongComment(char *&pScript);
        void SkipShortComment(char *&pScript);
        bool EndConstString(char *&pScript, string &out, const char quotationMark);

    };
    /**************************************************************/
    class BinaryReader : public Reader {
        BinaryReader();
        BinaryReader(const BinaryReader&);
        BinaryReader& operator=(const BinaryReader&);

    public:
        BinaryReader(bool recursiveReader, InterpreterIf &interpreter, Error &error) : Reader(recursiveReader, interpreter, error) {};
        virtual ~BinaryReader() {};

    protected:
        //virtual bool OnInit(ui8 *pData, ui32 size);
        virtual bool OnRead(ui8 *pData, ui32 size);
    private:
        bool OnHeader(ui8 *&pData, ui32 &size);
        bool OnSymbols(ui8 *&pData, ui32 &size);
        NODE_TYPE GetTokenType(string &token);
        void EscapeString(string &token);
        void UnEscapeString(string &token);

        BinaryHeader    m_header;
    };
    /**************************************************************/

    class Serializer {
        Serializer();
        Serializer(const Serializer&);
        Serializer& operator=(const Serializer&);

    public:
        Serializer(InterpreterIf &interpreter): m_pSymbolStore(0), m_Interpreter(interpreter) {};
        virtual ~Serializer() {};
        virtual bool Write(string path);
        virtual bool WriteRaw(ui8* pData, ui32 size) = 0;
        virtual bool WriteSymbol(SYMBOL_DESC &symDesc) = 0;
        virtual bool Begin(NODE_TYPE type) = 0;
        virtual bool End(NODE_TYPE type) = 0;

        void SetSymbolStore(SymbolStore *pSymbolStore) { m_pSymbolStore = pSymbolStore; };
    protected:
        virtual void OnDumpHeader();
        virtual void OnDumpSymbols();
        virtual void OnDumpConsts();
        virtual void OnDumpScript();

        bool TranslateBegin(NODE_TYPE type, string &value);
        bool TranslateEnd(NODE_TYPE type, string &value);

        SymbolStore   *m_pSymbolStore;
        InterpreterIf &m_Interpreter;
        ofstream       m_fileStream;
    };
    /**************************************************************/
    class TextSerializer : public Serializer {
        TextSerializer();
        TextSerializer(const TextSerializer&);
        TextSerializer& operator=(const TextSerializer&);

    public:
        TextSerializer(InterpreterIf &interpreter) : Serializer(interpreter), m_newLine(false){};
        virtual ~TextSerializer() {};
        //virtual bool Write(string path);
        virtual bool WriteRaw(ui8* pData, ui32 size);
        virtual bool WriteSymbol(SYMBOL_DESC &symDesc);
        virtual bool Begin(NODE_TYPE type);
        virtual bool End(NODE_TYPE type);

    private:
        virtual void OnDumpHeader() {};
        virtual void OnDumpSymbols() {};
        virtual void OnDumpConsts() {};
        bool EscapeString(ui8 *pData, ui32 size, string &out);

        string m_tabs;
        bool   m_newLine;
    };
    /**************************************************************/
    class BinarySerializer : public Serializer {
        BinarySerializer();
        BinarySerializer(const BinarySerializer&);
        BinarySerializer& operator=(const BinarySerializer&);

    public:
        BinarySerializer(InterpreterIf &interpreter);
        virtual ~BinarySerializer();
        //virtual bool Write(string path);
        virtual bool WriteRaw(ui8* pData, ui32 size);
        virtual bool WriteSymbol(SYMBOL_DESC &symDesc);
        virtual bool Begin(NODE_TYPE type);
        virtual bool End(NODE_TYPE type);

    private:
    };
    /*************************************************************/
}

//#ifdef _DEBUG
//class SymbolStore;
//script::SymbolStore* GetSymbolStore();
//#endif // _DEBUG

//
//void* CreateFooStub(void *pThis, unsigned char fooOffset, unsigned int ArgDWORDCount) {
//    unsigned char code[] = {
//        0x55,//push ebp 
//        0x8B, 0xEC,//mov ebp,esp
//        0x68, 0x00, 0x00, 0x00, 0x00,//push 0x00000000, will set to pThis
//                                     //store register we are going to use
//        0x51,//push ecx
//        0x52,//push edx
//             // propagate parameters in reverse order
//             // argument loop
//        0xB8, 0x00, 0x00, 0x00, 0x00,//mov eax,0; i is stored in EAX, set to iArgCount
//                                     //for (int i = iArgCount; i > 0; --i) {
//        0xEB, 0x03,//jmp EIP + 2 + 03h                ->   
//        0x83, 0xE8, 0x01,//sub eax,1                    |
//        0x83, 0xF8, 0x00,//cmp eax,0                  <-   <- 
//        0x7E, 0x0E,//jle EIP + XX function call section->    |
//        0x8B, 0xC8,//mov ecx,eax                         |   |
//        0xC1, 0xE1, 0x02,//shl ecx,4                     |   |
//        0x03, 0xCD,//add ecx,ebp                         |   |
//        0x83, 0xC1, 0x04,//add ecx,4                     |   |
//        0xFF, 0x31,//push dword ptr[ecx]                 |   |
//        0xEB, 0xEA,//jmp - 22                            | ->
//                   //}                                              |
//                   //function call a,b,c,d,e,f                      |
//        0x8B, 0x4D, 0xFC,//mov ecx,dword ptr[ebp - 4]  <-   get pThis
//        0x8B, 0x11,//mov edx,dword ptr[ecx]                 get virtual table pointer
//        0x8B, 0x42, 0x08,//mov eax,dword ptr[edx + 8]       function address is at offset (8 / sizeof(DWORD))
//        0xFF, 0xD0,//call eax
//                   // end of function call
//                   // restore registers
//        0x5A,//pop edx
//        0x59,//pop ecx
//        0x8B, 0xE5,//mov esp,ebp
//        0x5D,//pop ebp
//        0xC3,//ret
//    };
//
//    // set pThis
//    long pc = (long)pThis;
//    code[4] = pc & 0xff;
//    code[5] = (pc >> 8) & 0xff;
//    code[6] = (pc >> 16) & 0xff;
//    code[7] = (pc >> 24) & 0xff;
//
//    // ArgDWORDCount
//    code[11] = ArgDWORDCount & 0xff;
//    code[12] = (ArgDWORDCount >> 8) & 0xff;
//    code[13] = (ArgDWORDCount >> 16) & 0xff;
//    code[14] = (ArgDWORDCount >> 24) & 0xff;
//
//    // set fooOffset
//    code[46] = fooOffset;
//
//    HANDLE handle = GetCurrentProcess();
//
//    if (void *pExec = VirtualAllocEx(handle, 0, 40, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE)) {
//        if (WriteProcessMemory(handle, pExec, code, sizeof(code), 0)) {
//            return pExec;
//        }
//    }
//
//    return 0;
//}
//
//template<class T> void* type_cast(T fn) {
//    union TYPE_CAST {
//        void *p;
//        T     v;
//    } u;
//    u.v = fn;
//    return u.p;
//}
//
//template<class T> unsigned char GetFooOffset(T fn) {
//    union TYPE_CAST {
//        long *p;
//        T     v;
//    } u;
//    u.v = fn;
//    return *(unsigned char*)(u.p + 1);
//}
//
//void TestFooCall() {
//
//    class B {
//    public:
//        virtual int Foo(int i, int k) {
//            return 0;
//        }
//        virtual int Foo4(int i1, int i2, int i3, int i4) {
//            return i4;
//        }
//        virtual int Foo5(int i1, double d2, int i3, int i4) {
//            return 0;
//        }
//        virtual int Foo0() {
//            return 0;
//        };
//    };
//
//    //typedef int (B::*F5)(int, double, int, int);
//    //typedef int(B::*F4)(int i1, int i2, int i3, int i4);
//
//    class A : public B {
//        int m_i1;
//        double m_d2;
//        int m_i3, m_i4;
//    public:
//        virtual int Foo5(int i1, double d2, int i3, int i4) {
//            m_i1 = i1;
//            m_d2 = d2;
//            m_i3 = i3;
//            m_i4 = i4;
//            return m_i1 + (int)m_d2 + m_i3 + m_i4;
//        }
//        virtual int Foo(int i, int k) {
//            i = i + k - 0xff;
//            return i + 3;
//        }
//        virtual int Foo0() {
//            return 10;
//        };
//    };
//
//    B * b = new A();
//    int k = 10;
//    k = k * 4;
//    int i = 0;
//    i = i + 0xff;
//    i = b->Foo(i, k);
//    i = b->Foo0();
//    typedef void(*FFF)();
//
//
//    //F5 pf5 = &B::Foo5;
//    //(b->*pf5)(1,2.2,3,4);
//
//    /****************************************************/
//    typedef int (B::*F5)(int, double, int, int);
//    typedef int(B::*F4)(int i1, int i2, int i3, int i4);
//
//    typedef int(*FOO4)(int i1, int i2, int i3, int i4);
//    typedef int(*FOO5)(int i1, double d2, int i3, int i4);
//
//    void *p = CreateFooStub(b, GetFooOffset<F5>(&B::Foo5), 5);
//    FOO5 fn5 = (FOO5)p;
//    int res = fn5(1, 2.0002e-22, 3, 4);
//    VirtualFreeEx(GetCurrentProcess(), p, 0, MEM_RELEASE);
//
//    p = CreateFooStub(b, GetFooOffset<F4>(&B::Foo4), 5);
//    FOO4 fn4 = (FOO4)p;
//    res = fn4(1, 2, 3, 4);
//
//    VirtualFreeEx(GetCurrentProcess(), p, 0, MEM_RELEASE);
//    /****************************************************/
//
//
//    i = b->Foo0();
//    int argc = 4;
//    for (int i = argc; i > 0; --i) {
//        __asm {
//            push dword ptr[ebp + 12]; //param 2
//        }
//    }
//    __asm {
//        mov edx, dword ptr[ecx];
//        mov eax, ecx;
//        add eax, 4;
//        mov eax, dword ptr[ecx + 4];
//        add esp, ecx;
//        mov ecx, esp;
//        sub ecx, 11;
//        mov ecx, dword ptr[ecx];
//        add esp, 4;
//        push eax;
//        add ecx, 1;
//        mov eax, 4;
//        cmp eax, 0;
//        mov ecx, eax;
//        shl ecx, 4;
//        add ecx, ebp;
//        push dword ptr[ecx];
//
//        mov dword ptr[ebp - 18h], 0;
//        push dword ptr[ebp + 4];
//        push dword ptr[ebp + 12]; //param 2
//        push dword ptr[ebp + 16]; //param 3
//        mov eax, esi;
//        add eax, 3;
//        jmp dword ptr[esi + 22];
//        push eax;
//        push ecx;
//        push edx;
//        pop edx;
//        pop ecx;
//        pop eax;
//        //mov edx, dword ptr[p];
//        //push dword ptr[b];
//        //pop ebx;
//        //        call dword ptr[pExec];
//        //jmp p;
//    }
//
//    /*
//    pseudo code:
//    jmp my_code;
//    my_code:
//    mov pointer ot Function object;
//    set ebx to point to Function Object
//    call Function::Run()
//    */
//}
