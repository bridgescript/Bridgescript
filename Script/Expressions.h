#pragma once
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <assert.h>
#include <algorithm>
#include <stack>

#include "Basic.h"
#include "Error.h"

RUNTIME_RETURN AssertError();


#define PROTECT_CLASS(TypeName) protected: TypeName() {}; TypeName(const TypeName&) {}; TypeName& operator=(const TypeName&) {};

#define VALUE_MATH_OPERATOR(name, op) ERROR_TYPE name(Value& v) {\
    if ((m_type & (NUM_TYPE_FLAG | FLOATING_TYPE_FLAG)) && (v.GetType() & (NUM_TYPE_FLAG | FLOATING_TYPE_FLAG))) {\
        switch (m_type) {\
            case I8_TYPE:\
                *GetPtr<i8>() op= v.GetI8();\
                break;\
            case UI8_TYPE:\
                *GetPtr<ui8>() op= v.GetUI8();\
                break;\
            case I16_TYPE:\
                *GetPtr<i16>() op= v.GetI16();\
                break;\
            case UI16_TYPE:\
                *GetPtr<ui16>() op= v.GetUI16();\
                break;\
            case I32_TYPE:\
                *GetPtr<i32>() op= v.GetI32();\
                break;\
            case UI32_TYPE:\
                *GetPtr<ui32>() op= v.GetUI32();\
                break;\
            case I64_TYPE:\
                *GetPtr<i64>() op= v.GetI64();\
                break;\
            case UI64_TYPE:\
                *GetPtr<ui64>() op= v.GetUI64();\
                break;\
            case FLOAT_TYPE:\
                *GetPtr<float>() op= v.GetFloat();\
                break;\
            case DOUBLE_TYPE:\
                *GetPtr<double>() op= v.GetDouble();\
                break;\
            default:\
                return INCOMPATIBLE_TYPE;\
        }\
        return NO_ERROR;\
    }\
    return INCOMPATIBLE_TYPE;\
}

#define VALUE_MATH_OPERATOR_DIV_CHECK_ZERO() ERROR_TYPE Div(Value& v) {\
    if ((m_type & (NUM_TYPE_FLAG | FLOATING_TYPE_FLAG)) && (v.GetType() & (NUM_TYPE_FLAG | FLOATING_TYPE_FLAG))) {\
        switch (m_type) {\
            case I8_TYPE:{\
                i8 _v = v.GetI8();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<i8>() /= v.GetI8();\
                break;\
            }\
            case UI8_TYPE:{\
                ui8 _v = v.GetUI8();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<ui8>() /= _v;\
                break;\
            }\
            case I16_TYPE:{\
                i16 _v = v.GetI16();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<i16>() /= _v;\
                break;\
            }\
            case UI16_TYPE:{\
                ui16 _v = v.GetUI16();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<ui16>() /= _v;\
                break;\
            }\
            case I32_TYPE:{\
                i32 _v = v.GetI32();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<i32>() /= _v;\
                break;\
            }\
            case UI32_TYPE:{\
                ui32 _v = v.GetUI32();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<ui32>() /= _v;\
                break;\
            }\
            case I64_TYPE:{\
                i64 _v = v.GetI64();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<i64>() /= _v;\
                break;\
            }\
            case UI64_TYPE:{\
                ui64 _v = v.GetUI64();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<ui64>() /= _v;\
                break;\
            }\
            case FLOAT_TYPE:{\
                float _v = v.GetFloat();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<float>() /= _v;\
                break;\
            }\
            case DOUBLE_TYPE:{\
                double _v = v.GetDouble();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<double>() /= _v;\
                break;\
            }\
            default:\
                return INCOMPATIBLE_TYPE;\
        }\
        return NO_ERROR;\
    }\
    return INCOMPATIBLE_TYPE;\
}

#define VALUE_MATH_OPERATOR_MOD_CHECK_ZERO() ERROR_TYPE Mod(Value& v) {\
    if ((m_type & (NUM_TYPE_FLAG | FLOATING_TYPE_FLAG)) && (v.GetType() & (NUM_TYPE_FLAG | FLOATING_TYPE_FLAG))) {\
        switch (m_type) {\
            case I8_TYPE:{\
                i8 _v = v.GetI8();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<i8>() %= v.GetI8();\
                break;\
            }\
            case UI8_TYPE:{\
                ui8 _v = v.GetUI8();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<ui8>() %= _v;\
                break;\
            }\
            case I16_TYPE:{\
                i16 _v = v.GetI16();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<i16>() %= _v;\
                break;\
            }\
            case UI16_TYPE:{\
                ui16 _v = v.GetUI16();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<ui16>() %= _v;\
                break;\
            }\
            case I32_TYPE:{\
                i32 _v = v.GetI32();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<i32>() %= _v;\
                break;\
            }\
            case UI32_TYPE:{\
                ui32 _v = v.GetUI32();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<ui32>() %= _v;\
                break;\
            }\
            case I64_TYPE:{\
                i64 _v = v.GetI64();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<i64>() %= _v;\
                break;\
            }\
            case UI64_TYPE:{\
                ui64 _v = v.GetUI64();\
                if (!_v) return DIVISION_BY_ZERO;\
                *GetPtr<ui64>() %= _v;\
                break;\
            }\
            default:\
                return INCOMPATIBLE_TYPE;\
        }\
        return NO_ERROR;\
    }\
    return INCOMPATIBLE_TYPE;\
}

#define VALUE_BIT_OPERATOR(name, op) ERROR_TYPE name(Value& v) {\
    if ((m_type & NUM_TYPE_FLAG) && (v.GetType() & NUM_TYPE_FLAG)) {\
        switch (m_type) {\
            case BOOL_TYPE:\
                *GetPtr<bool>() op= v.GetBool();\
                break;\
            case I8_TYPE:\
                *GetPtr<i8>() op= v.GetI8();\
                break;\
            case UI8_TYPE:\
                *GetPtr<ui8>() op= v.GetUI8();\
                break;\
            case I16_TYPE:\
                *GetPtr<i16>() op= v.GetI16();\
                break;\
            case UI16_TYPE:\
                *GetPtr<ui16>() op= v.GetUI16();\
                break;\
            case I32_TYPE:\
                *GetPtr<i32>() op= v.GetI32();\
                break;\
            case UI32_TYPE:\
                *GetPtr<ui32>() op= v.GetUI32();\
                break;\
            case I64_TYPE:\
                *GetPtr<i64>() op= v.GetI64();\
                break;\
            case UI64_TYPE:\
                *GetPtr<ui64>() op= v.GetUI64();\
                break;\
            default:\
                return INCOMPATIBLE_TYPE;\
        }\
        return NO_ERROR;\
    }\
    return INCOMPATIBLE_TYPE;\
}

#define VALUE_NUM_OPERATOR(name, op) ERROR_TYPE name(Value& v) {\
    switch (m_type) {\
        case BOOL_TYPE:\
        case I8_TYPE:\
        case UI8_TYPE:\
        case I16_TYPE:\
        case UI16_TYPE:\
        case I32_TYPE:\
        case UI32_TYPE:\
        case I64_TYPE:\
        case UI64_TYPE:\
        case FLOAT_TYPE:\
        case DOUBLE_TYPE:\
        case NULL_TYPE:\
        case STRING_TYPE:\
            Set(GetBool() op v.GetBool());\
            break;\
        default:\
            return INCOMPATIBLE_TYPE;\
    }\
    return NO_ERROR;\
}

#define VALUE_NUM_OPERATOR_NO_BOOL(name, op) ERROR_TYPE name(Value& v) {\
    if ((m_type & NUM_TYPE_FLAG) && (v.GetType() & NUM_TYPE_FLAG)) {\
        switch (m_type) {\
            case I8_TYPE:\
                Set((i8)(GetI8() op v.GetI8()));\
                break;\
            case UI8_TYPE:\
                Set((ui8)(GetUI8() op v.GetUI8()));\
                break;\
            case I16_TYPE:\
                Set((i16)(GetI16() op v.GetI16()));\
                break;\
            case UI16_TYPE:\
                Set((ui16)(GetUI16() op v.GetUI16()));\
                break;\
            case I32_TYPE:\
                Set((i32)(GetI32() op v.GetI32()));\
                break;\
            case UI32_TYPE:\
                Set((ui32)(GetUI32() op v.GetUI32()));\
                break;\
            case I64_TYPE:\
                Set((i64)(GetI64() op v.GetI64()));\
                break;\
            case UI64_TYPE:\
                Set((ui64)(GetUI64() op v.GetUI64()));\
                break;\
            default:\
                return INCOMPATIBLE_TYPE;\
        }\
        return NO_ERROR;\
    }\
    return INCOMPATIBLE_TYPE;\
}

#define VALUE_BOOL_OPERATOR(name, op) ERROR_TYPE name(Value& v) {\
    ERROR_TYPE ret = INCOMPATIBLE_TYPE;\
    if ((m_type & (NUM_TYPE_FLAG | FLOATING_TYPE_FLAG)) && (v.GetType() & (NUM_TYPE_FLAG | FLOATING_TYPE_FLAG))) {\
        switch (m_type) {\
            case BOOL_TYPE:\
                ret = (GetBool() op v.GetBool()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case I8_TYPE:\
                ret = (GetI8() op v.GetI8()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case UI8_TYPE:\
                ret = (GetUI8() op v.GetUI8()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case I16_TYPE:\
                ret = (GetI16() op v.GetI16()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case UI16_TYPE:\
                ret = (GetUI16() op v.GetUI16()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case I32_TYPE:\
                ret = (GetI32() op v.GetI32()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case UI32_TYPE:\
                ret = (GetUI32() op v.GetUI32()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case I64_TYPE:\
                ret = (GetI64() op v.GetI64()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case UI64_TYPE:\
                ret = (GetUI64() op v.GetUI64()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case FLOAT_TYPE:\
                ret = (GetFloat() op v.GetFloat()) ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            case DOUBLE_TYPE:\
                ret = (GetDouble() op v.GetDouble())  ? EVAL_TRUE : EVAL_FALSE;\
                break;\
            default:\
                break;\
        }\
    }\
    return ret;\
}

#ifdef SCRIPT_DEBUGGER
#include "Debugger.h"
#endif // SCRIPT_DEBUGGER

using namespace std;
namespace script {

    typedef class _Lock {
        _Lock();
        _Lock(const _Lock&);
        _Lock& operator=(const _Lock&);
        CRITICAL_SECTION    &m_cs;
    public:
        _Lock(CRITICAL_SECTION &cs) : m_cs(cs) { EnterCriticalSection(&m_cs); }
        ~_Lock() { LeaveCriticalSection(&m_cs); }
    } LOCK;

#ifdef SCRIPT_DEBUGGER
    class Node;
    class DebuggerHandler;
    

    typedef vector<Node*>               BREAK_POINT_LINES;
    typedef vector<BREAK_POINT_LINES>   BREAK_POINT_LINES_BY_FILE_ID;

    class FlowSemaphor : public DebuggerAction {
    public:
        typedef enum FlowType {
            FLOW_STOP = 0,
            FLOW_RUN,
            FLOW_PAUSE,
            FLOW_STEP_IN,
            FLOW_STEP_OVER,
            FLOW_STEP_OUT,
            FLOW_ERROR
        } FLOW_TYPE;

        FlowSemaphor(DebuggerHandler *pDebuggerHandler, FLOW_TYPE flowType);
        virtual ~FlowSemaphor();

        virtual void Stop();
        virtual void Run();
        virtual void Pause(PAUSE_TYPE type);
        virtual void StepIn();
        virtual void StepOver();
        virtual void StepOut();
        virtual bool SetBreakpoint(ui32 fileId, ui32 line, bool set);

        inline void SetDebugger(Debugger *pDebugger) { m_pDebugger = pDebugger; }
        inline ThreadId GetThreadId() { return m_threadId; }
        void RegisterBreakPointLine(Node* pNode);
        RUNTIME_RETURN CheckFlow(Node *pNode, ui32 stackIndex, const unordered_set<ui64> &breakLines);
        void ResetStepOver(Node *pNode, ui32 stackIndex);
        void ResetStepOut(Node *pNode, ui32 stackIndex);
        void ResetOnEndOfThread();
        void SetCurrentStackIndex(ui32 stackIndex);
        ui32 m_Script2DllSwitchCounter;
        void ResetOnEndThread();
    private:
        FlowSemaphor();
        FlowSemaphor(const FlowSemaphor&);
        FlowSemaphor& operator=(const FlowSemaphor&);

        CRITICAL_SECTION                m_cs;
        HANDLE                          m_flowTypeEvent;
        ui32                            m_stepStackIndex;
        BREAK_POINT_LINES_BY_FILE_ID    m_breakPointLines;
        FLOW_TYPE                       m_flowType;
        ThreadId                        m_threadId;
        Debugger                       *m_pDebugger;
        DebuggerHandler                *m_pDebuggerHandler;
    };

    class DebuggerHandler : public DebuggerAction {
        typedef enum FlowType {
            FLOW_STOP = 0,
            FLOW_RUN,
            FLOW_PAUSE,
            FLOW_STEP_IN,
            FLOW_STEP_OVER,
            FLOW_STEP_OUT,
            FLOW_ERROR
        } FLOW_TYPE;

    public:
        DebuggerHandler();
        virtual ~DebuggerHandler();

        virtual void Stop();
        virtual void Run();
        virtual void Pause(PAUSE_TYPE type);
        virtual void StepIn();
        virtual void StepOver();
        virtual void StepOut();
        virtual bool SetBreakpoint(ui32 fileId, ui32 line, bool set);
        
        void Wait();
        void Ready();

        void RegisterBreakPointLine(Node* pNode);
        FlowSemaphor* CreateFlowSemaphor(ThreadId threadId);
        void DeleteFlowSemaphor(ThreadId threadId);

        inline void SetDebugger(Debugger *pDebugger) { m_pDebugger = pDebugger; }
        inline Debugger* GetDebugger() { return m_pDebugger; }
    private:

        typedef void (DebuggerAction::*ACTION_HANDLER)();
        
        void Handler(ACTION_HANDLER fn);
        void StepHandler(ACTION_HANDLER fn);

        typedef vector<FlowSemaphor*>                       FLOW_SEMAPHOR_MAP;

        CRITICAL_SECTION                m_cs;
        HANDLE                          m_hReadyEvent;
        FLOW_SEMAPHOR_MAP               m_flowSemaphors;
        BREAK_POINT_LINES_BY_FILE_ID    m_breakPointLines;
        Debugger                       *m_pDebugger;
        ui32                            m_semaphorIdx;
    };
#endif // SCRIPT_DEBUGGER

    class MemoryBlockRef;

    //union typecast {
    //    bool m_b;
    //    i8  m_i8;
    //    ui8  m_ui8;
    //    i16  m_i16;
    //    ui16  m_ui16;
    //    i32  m_i32;
    //    ui32  m_ui32;
    //    i64  m_i64;
    //    ui64  m_ui64;
    //    float  m_float;
    //    double  m_double;
    //    typecast(bool v) : m_ui64(0) { m_b = v; };
    //    typecast(i8 v) : m_ui64(0) { m_i8 = v; };
    //    typecast(ui8 v) : m_ui64(0) { m_ui8 = v; };
    //    typecast(i16 v) : m_ui64(0) { m_i16 = v; };
    //    typecast(ui16 v) : m_ui64(0) { m_ui16 = v; };
    //    typecast(i32 v) : m_ui64(0) { m_i32 = v; };
    //    typecast(ui32 v) : m_ui64(0) { m_ui32 = v; };
    //    typecast(i64 v) : m_ui64(0) { m_i64 = v; };
    //    typecast(ui64 v) : m_ui64(0) { m_ui64 = v; };
    //    typecast(float v) : m_ui64(0) { m_float = v; };
    //    typecast(double v) : m_ui64(0) { m_double = v; };

    //    inline operator bool() { return m_b; };
    //    inline operator i8() { return m_i8; };
    //    inline operator ui8() { return m_ui8; };
    //    inline operator i16() { return m_i16; };
    //    inline operator ui16() { return m_ui16; };
    //    inline operator i32() { return m_i32; };
    //    inline operator ui32() { return m_ui32; };
    //    inline operator i64() { return m_i64; };
    //    inline operator ui64() { return m_ui64; };
    //    inline operator float() { return m_float; };
    //    inline operator double() { return m_double; };
    //};
#ifdef TRACE_OBJECTS
    class Runable;
#endif // TRACE_OBJECTS
    class Type;
    class Node;

    class Value {
    public:
        typedef enum {
            BASE_TYPE_MASK          = 0xff,
            FIXED_NUM_TYPE_FLAG     = 0x1000,
            FLOATING_TYPE_FLAG      = 0x2000,
            NUM_TYPE_FLAG           = FIXED_NUM_TYPE_FLAG | FLOATING_TYPE_FLAG,
            AGGREGATE_TYPE          = 0x4000,
            ARRAY_TYPE              = 0x8000,
            NULL_TYPE               = 0,
            BOOL_TYPE,
            STRING_TYPE,
            FUNCTION_REF_TYPE,
            LIB_TYPE,
            I8_TYPE                 = FIXED_NUM_TYPE_FLAG | (LIB_TYPE + 1),
            UI8_TYPE,
            I16_TYPE,
            UI16_TYPE,
            I32_TYPE,
            UI32_TYPE, 
            I64_TYPE,
            UI64_TYPE,
            FLOAT_TYPE              = FLOATING_TYPE_FLAG | (UI64_TYPE + 1),
            DOUBLE_TYPE,
        } VALUE_TYPE;
    public:
        Value();
        Value(const Value &val);
        Value(MemoryBlockRef *pMemBlockRef, Type *pType);
        virtual ~Value();

        inline bool     GetBool();
        inline i8       GetI8();
        inline ui8      GetUI8();
        inline i16      GetI16();
        inline ui16     GetUI16();
        inline i32      GetI32();
        inline ui32     GetUI32();
        inline i64      GetI64();
        inline ui64     GetUI64();
        inline float    GetFloat();
        inline double   GetDouble();
        inline Type *GetTypeNode() { return m_pType; };

        VALUE_TYPE GetType();
        ui32 SizeOf();

        Value & operator= (Value &val);
        void SetType(VALUE_TYPE type, Type *pType);
        void Set(MemoryBlockRef *pMemBlockRef, Type *pType, ui32 offset);
        void SetRef(MemoryBlockRef *pMemBlockRef);
        void Set(const Value &val);
        void SetNull() { Clear(); };
        bool Set(char *pArray, ui32 size);
        bool Set(string& str, VALUE_TYPE type, Type *pType);
        void Set(bool b);
        void Set(i8 v);
        void Set(ui8 v);
        void Set(i16 v);
        void Set(ui16 v);
        void Set(i32 v);
        void Set(ui32 v);
        void Set(i64 v);
        void Set(ui64 v);
        void Set(float v);
        void Set(double v);
        void DeepCopy(const Value &val);

        ERROR_TYPE Add(Value& v); // +, +=
        ERROR_TYPE BitNot(Value& v); // ~

        VALUE_MATH_OPERATOR(Sub, -); // -, -=
        VALUE_MATH_OPERATOR(Mul, *); // *, *=
        VALUE_MATH_OPERATOR_DIV_CHECK_ZERO(); // /, /= , must check for 0
        
        VALUE_BIT_OPERATOR(BitAnd, &); // &, &=
        VALUE_BIT_OPERATOR(BitOr, |);  // |, |=
        VALUE_BIT_OPERATOR(Xor, ^);    // ^, ^=
        
        VALUE_NUM_OPERATOR(And, &&); // &&
        VALUE_NUM_OPERATOR(Or, ||);  // ||

        VALUE_MATH_OPERATOR_MOD_CHECK_ZERO(); // %
        VALUE_NUM_OPERATOR_NO_BOOL(ShiftLeft, <<); // <<
        VALUE_NUM_OPERATOR_NO_BOOL(ShiftRight, >>); // >>

        VALUE_BOOL_OPERATOR(Less, < ); // <
        VALUE_BOOL_OPERATOR(LessEq, <= ); // <=
        VALUE_BOOL_OPERATOR(Greater, >); // >
        VALUE_BOOL_OPERATOR(GreaterEq, >=); // >=
        VALUE_BOOL_OPERATOR(Eq, ==); // ==
        VALUE_BOOL_OPERATOR(NotEq, != ); // ==

        string  GetString();
        bool    GetHexString(string &value);
        bool    GetBinaryString(string &value);
        void* const GetRaw();
        MemoryBlockRef* GetMemoryBlockRef(Type *pType);

    private:
        void Clear();

        inline static bool   S2BOOL(const string& str);
        inline static i8     S2I8(const string& str);
        inline static ui8    S2UI8(const string& str);
        inline static i16    S2I16(const string& str);
        inline static ui16   S2UI16(const string& str);
        inline static i32    S2I32(const string& str);
        inline static ui32   S2UI32(const string& str);
        inline static i64    S2I64(const string& str);
        inline static ui64   S2UI64(const string& str);
        inline static float  S2FLOAT(const string& str);
        inline static double S2DOUBLE(const string& str);

        template<class T, class STR_CONV_FOO> T GetValue(STR_CONV_FOO fnStrConv);
        template<class T> T* GetPtr();
        Type *m_pType;

        VALUE_TYPE      m_type;
        MemoryBlockRef *m_pMemoryBlockRef;
    };

    /*************************************************************/

    /*************************************************************/
        
    typedef enum Node_Type {
        TYPE_NONE,
        TYPE_RUNABLE,
        TYPE_TOKEN,
        TYPE_VAR            = TYPE_TOKEN,
        TYPE_NUM_HEX_1, // 0x..., 0X...
        TYPE_NUM_HEX_2, // ...h, ...H
        TYPE_NUM_BIN,   // ...b, ...B
        TYPE_NUM_EXP,   // ...e, ...E
        TYPE_CONST,
        TYPE_CONST_NUM,
        TYPE_OPERATOR,
        TYPE_ASSIGN,                            // = assignment operator
        TYPE_EQ,                                // == equal operator
        TYPE_ADD,                               // +
        TYPE_ADD_ASSIGN     = TYPE_ADD + 1,     // +=
        TYPE_INC_OP,                            // ++ this is invalid type, just for intermidiate parsing
        TYPE_PRE_INC_OP,                        // ++N
        TYPE_POST_INC_OP,                       // N++  
        TYPE_SUB,                               // -
        TYPE_SUB_ASSIGN     = TYPE_SUB + 1,     // -=
        TYPE_DEC_OP,                            // -- this is invalid type, just for intermidiate parsing
        TYPE_PRE_DEC_OP,                        // --N
        TYPE_POST_DEC_OP,                       // N--
        TYPE_BIT_AND,                           // &
        TYPE_BIT_AND_ASSIGN = TYPE_BIT_AND + 1, // &=
        TYPE_AND,                               // &&
        TYPE_BIT_OR,                            // |
        TYPE_BIT_OR_ASSIGN  = TYPE_BIT_OR + 1,  // |=
        TYPE_OR,                                // ||
        TYPE_LESS,                              // <
        TYPE_LESS_EQ        = TYPE_LESS + 1,    // <=
        TYPE_SHIFT_LEFT,                        // <<
        TYPE_SHIFT_LEFT_ASSIGN,                 // <<=
        TYPE_GREATER,                           // >
        TYPE_GREATER_EQ     = TYPE_GREATER + 1, // >=
        TYPE_SHIFT_RIGHT,                       // >>
        TYPE_SHIFT_RIGHT_ASSIGN,                // >>=
        TYPE_MUL,                               // *
        TYPE_MUL_ASSIGN     = TYPE_MUL + 1,     // *=
        TYPE_DIV,                               // /
        TYPE_DIV_ASSIGN     = TYPE_DIV + 1,     // /=
        TYPE_MOD,                               // %
        TYPE_MOD_ASSIGN     = TYPE_MOD + 1,     // %=
        TYPE_BIT_NOT,                           // ~
        TYPE_BIT_NOT_ASSIGN = TYPE_BIT_NOT + 1, // ~= this is invalid operation!
        TYPE_NOT,                               // !
        TYPE_NOT_EQ         = TYPE_NOT + 1,     // !=
        TYPE_XOR,                               // ^
        TYPE_XOR_ASSIGN     = TYPE_XOR + 1,     // ^=
        TYPE_REF_ASSIGN,                        // @= , operator to set reference
        TYPE_SUBSCRIPT,                         // []
        TYPE_PARENTHESIS,                       // ()
        TYPE_BASIC_SCOPE,                       // 
        TYPE_SCOPE,                             // {}
        TYPE_CASE_SCOPE,                        // scope of "case/default VAL: CASESCOPE break;"
        TYPE_DOT,                   
        TYPE_MEMBER_ACCESS  = TYPE_DOT,         // .
        TYPE_COMMA,                             // ,
        TYPE_SEMICOLON,                         // ;
        TYPE_COLON,                             // :
        TYPE_REF,                               // @
        TYPE_CONDITIONAL,                       // 
        TYPE_FOR,                               // for
        TYPE_WHILE,                             // while
        TYPE_IF,                                // if
        TYPE_ELSE,                              // else
        TYPE_SWITCH,                            // switch
        TYPE_CASE,                              // case
        TYPE_DEFAULT,                           // default
        TYPE_BREAK,                             // break
        TYPE_CONTINUE,                          // continue
        TYPE_FUNCTION_CALL,                     // foo(arg) function call
        TYPE_FUNCTION,                          // function , actual function implementation
        TYPE_FUNCTION_PTR,                      // external pointer to a C function
        TYPE_FUNCTION_LIB_PTR,                  // pointer to a C function returned by GetProcAddress
        TYPE_FUNCTION_CALLBACK_PTR,             // C pointer function stub to invoke script function
        TYPE_RETURN,                            // return
        //TYPE_NEW,                               // new
        TYPE_FUNCTION_REF_TYPE,                 // function<Foo_Name> - function type declaration
        TYPE_QUOTATION_MARK,                    // "
        // types
        TYPE_BOOL,
        TYPE_I8,
        TYPE_UI8,
        TYPE_I16,
        TYPE_UI16,
        TYPE_I32,
        TYPE_UI32,
        TYPE_I64,
        TYPE_UI64,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_CONST_BOOL,
        TYPE_LIB_VAR,
        TYPE_ERROR_VAR,
        
        // artificial types
        TYPE_SCOPE_END_EXPRESSION,              // this is a special type for TextSerializer prints ";\n"
        TYPE_SCOPE_NO_NEW_LINE,                 // special case for serializer
        TYPE_NEW_LINE,                          // \n
        TYPE_BLANK_SCOPE,                       // a scope without {}
        TYPE_EMPTY,                             // this is a hack for TextSerializer
        TYPE_AGGREGATE_SYMBOL,                  // for int and floating point numbers
        TYPE_INCLUDE,
        TYPE_LIB,
        TYPE_STRUCT,
        TYPE_ARRAY,                             // array<>
        TYPE_ERROR, // error()
        TYPE_ERROR_TYPE,
        TYPE_SIZEOF,
        TYPE_ARR2STR,
        TYPE_WARR2STR,
        TYPE_STR2ARR,
        TYPE_STR2WARR,
        TYPE_LOCK,
        TYPE_UNLOCK,
        TYPE_CAST,
        TYPE_AGGREGATE_TYPE,
        TYPE_AGGREGATE_TYPE_COLLECTION,         // stores subtypes
        TYPE_PRE_POST_EXPRESSIONS,
    } NODE_TYPE;

    /**************************************************************/
    class Scope;
    
    /*************************************************************/

#ifdef TRACE_OBJECTS
    class SymbolStore;
#endif // TRACE_OBJECTS

    /***********************************/
    class Memory;

    class MemoryBlock {
    protected:
        MemoryBlock();
        MemoryBlock(const MemoryBlock& that);
        MemoryBlock& operator=(const MemoryBlock& that);
        virtual ~MemoryBlock();
    public:
        typedef enum {
            MEM_LOCAL       = 0,
            MEM_EXTERNAL    = 1,
            MEM_REGISTERED  = 2,
            MEM_LOCAL_REGISTERED    = MEM_LOCAL | MEM_REGISTERED,
            MEM_EXTERNAL_REGISTERED = MEM_EXTERNAL | MEM_REGISTERED,
        } MEMORY_FLAG;

        MemoryBlock(Type *pType, ui32 size);
        MemoryBlock(Type *pType, ui32 size, void *pExternal);

        inline void** GetRefToMemoryPtr() { return &m_pMemory; };
        inline ui32 GetSize() { return m_size; };
        inline static MemoryBlock* GetMemoryBlock(void *pRefToMem) { return (MemoryBlock*)((ui32)pRefToMem - sizeof(MemoryBlock)); };
        inline static bool IsMemoryBlock(void *pMemory) { ui32 p = (((ui32)pMemory) - sizeof(void*)); return (*(ui32*)p == (ui32)pMemory); };
        inline MEMORY_FLAG GetMemoryFlag() { return m_flag; }
#ifdef TRACE_MEMORY
        inline ui32 GetRefCount() { return m_refCount; };
#endif // TRACE_MEMORY
        ui32 AddRef();
#ifdef DEL
        bool Release();
#else // DEL
        bool Release(ui32 offset);
#endif // DEL
        void Lock();
        void UnLock();
        bool IsLocked();
        void Clear(ui32 offset, Type *pType);
        Type *GetTypeNode() { return m_pMemoryTypeNode; };
        void Register(Memory &memory);
        template<class T = void> inline T* GetPtr(ui32 offset) { return (T*)&((ui8*)m_pMemory)[offset]; };
    protected:
        Type                    *m_pMemoryTypeNode;
        ui32                     m_refCount;
        ui32                     m_size;
        //bool                     m_registeredNonAtomic;
        MEMORY_FLAG              m_flag;
        void *                   m_pMemory;
    };

    /***********************************/
    /***********************************/

    class MemoryBlockRef {
        MemoryBlockRef();
        MemoryBlockRef(const MemoryBlockRef& that);
        MemoryBlockRef& operator=(const MemoryBlockRef& that);
        ~MemoryBlockRef();
    public:
        MemoryBlockRef(Type *pType, ui32 offset, ui32 size);
        MemoryBlockRef(Type *pType, void *pExternal, ui32 size);
        MemoryBlockRef(Type *pType, ui32 offset, MemoryBlock *pMemoryBlock);

        inline MemoryBlock* GetMemoryBlock() { return m_pMemoryBlock; };
        inline ui32 GetOffset() { return m_offset; };
        inline void SetOffset(ui32 offset) { m_offset = offset; }
        inline ui32 GetRefCount() { return m_refCount; };
        inline Type *GetReferenceTypeNode() { return m_pReferenceTypeNode; };

        ui32 AddRef();
        void Release();
        void SetMemoryBlock(Type *pType, ui32 offset, MemoryBlock *pMemoryBlock);

    private:
        ui32                m_refCount;
        ui32                m_offset;
        MemoryBlock        *m_pMemoryBlock;
        Type               *m_pReferenceTypeNode;
    };

    /***********************************/

#ifdef SCRIPT_DEBUGGER
    typedef struct LocalVarDescriptor {
    private:
        Type        *m_pType;
    public:
        SYMBOL_DESC  m_desc;
        LocalVarDescriptor(SYMBOL_DESC &symDesc, Type *pType) : m_pType(pType), m_desc(symDesc) {};
        inline operator Type* () { return m_pType; };
    } LOCAL_VAR_DESCRIPTOR;
#else // SCRIPT_DEBUGGER
    typedef Type*   LOCAL_VAR_DESCRIPTOR;
#endif // SCRIPT_DEBUGGER
    
    /***********************************/

    class FunctionCallbackPtr;
    class SymbolStore;

    class StackMemory {
        StackMemory();
        StackMemory(const StackMemory&);
        StackMemory& operator=(const StackMemory&);
    public:
        StackMemory(ThreadId threadId
#ifdef SCRIPT_DEBUGGER
            , DebuggerHandler *pDebuggerHandler
#endif // SCRIPT_DEBUGGER
            , SymbolStore     *pSymStore
            , StdOutInterface *pStdOutInterface);
        virtual ~StackMemory();

        inline ThreadId GetThreadId() { return m_threadId; };
        inline ui32 GetFrameIdx() { return m_memoryFrameIdx; };
        //inline void SetMainThread() { m_bMainThread = true; };
        //inline bool IsMainThread() { return m_bMainThread; };
#ifdef SCRIPT_DEBUGGER
        inline FlowSemaphor* GetFlowSemaphor() { return m_pFlowSemaphor; };
#endif // SCRIPT_DEBUGGER
        void SetCurRunningNode(Node *pNode);
        void PushFrame(Node *pFunctionNode, Scope *pFunctionScope, vector<LOCAL_VAR_DESCRIPTOR> &types);
        ui32 PopFrame();
        void PushResultMemoryBlockRef(Node *pFunctionNode, Scope *pFunctionScope, MemoryBlockRef *pMemoryBlockRef);
        ui32 PopResultMemoryBlockRef();

        void PushReferencedMemoryBlock(ui32 idx, Type *pType, ui32 offset, MemoryBlock *pMemoryBlock);
        void UpdateMemoryBlock(ui32 idx, Type *pType, ui32 offset, MemoryBlock *pMemoryBlock);
        void PopReferencedMemoryBlock(ui32 idx);
        void PushMemoryBlockRef(Type *pType);
        MemoryBlockRef* GetMemoryBlockRef(ui32 idx);
        MemoryBlockRef* GetMemoryBlockRef(ui32 idx, ui32 frameIdx);
        void UpdateMemoryBlockRef(ui32 idx, MemoryBlockRef *pMemoryBlockRef);
        ui32 GetCurFrameMemoryBlockRefCount();
        void SetFrameIndex(ui32 frameIdx);
        bool GetFrameFunctionAndScope(ui32 frameIdx, Node *&pFunctionNode, Scope *&pFunctionScope, Node *&pCurrentNode);
        bool GetFrameFunctionAsStrings(ui32 frameIdx, string &fooType, string &location, string &fileName, ui32 &line, ui32 &pos);
        Node* GetFrameFunctionNode(ui32 frameIdx);
        void PopulateGlobals(StackMemory *pStackMemory);
        RunTimeError& GetRunTimeError() { return m_error; }
        void GetString(string &value);
        void GetTrace(vector<string> &trace);

    private:
        typedef struct MemRefFrameSpot {
            MemoryBlockRef           *m_pMemoryBlockRef;
            vector<MemoryBlockRef*>  *m_pReferencedMemoryBlocks;
            MemRefFrameSpot(MemoryBlockRef *pMemoryBlockRef);
            MemRefFrameSpot(const MemRefFrameSpot& that);
            ~MemRefFrameSpot();

            void PushReferencedMemoryBlockRef(Type *pType, ui32 offset, MemoryBlock *pMemoryBlock/*MemoryBlockRef *pMemoryBlockRef*/);
            void UpdateReferencedMemoryBlockRef(MemoryBlockRef *pMemoryBlockRef);
            void PopReferencedMemoryBlockRef();
            MemoryBlockRef* GetReferencedMemoryBlockRef();

        private:
            MemRefFrameSpot();
        public:
            MemRefFrameSpot& operator=(const MemRefFrameSpot& that);
        } MEM_REF_FRAME_SPOT;

        typedef struct MemoryFrame : public vector<MEM_REF_FRAME_SPOT> {
            MemoryFrame(Node *pFunctionNode, Scope *pFunctionScope) : m_pFunctionNode(pFunctionNode), m_pFunctionScope(pFunctionScope), m_pCurrentNode(0) {};
            Node    *m_pFunctionNode;
            Scope   *m_pFunctionScope;
            Node    *m_pCurrentNode;
        private:
            MemoryFrame();
            MemoryFrame& operator=(const MemoryFrame& that);
        } MEM_FRAME;

        RunTimeError            m_error;
        vector<MEM_FRAME*>      m_memoryFrames;
        ui32                    m_memoryFrameIdx;
        ThreadId                m_threadId;
#ifdef SCRIPT_DEBUGGER
        DebuggerHandler        *m_pDebuggerHandler;
        FlowSemaphor           *m_pFlowSemaphor;
#endif // SCRIPT_DEBUGGER
        //bool                    m_bMainThread;
    };
    
    /***********************************/
    class InstanceHandler;

    class Memory {
        Memory();
        Memory(const Memory&);
        Memory& operator=(const Memory&);
    public:
#ifdef SCRIPT_DEBUGGER
        Memory(SymbolStore *pSymbolStore, DebuggerHandler *pDebuggerHandler, StdOutInterface *pStdOutInterface);
#else // SCRIPT_DEBUGGER
        Memory(SymbolStore *pSymbolStore, StdOutInterface *pStdOutInterface);
#endif // SCRIPT_DEBUGGER
        virtual ~Memory();
        
        static MemoryBlockRef* Allocate(Type *pType, ui32 offset, ui32 size);
        static MemoryBlockRef* Allocate(Type *pType, ui32 offset, MemoryBlock *pMemoryBlock);
        static MemoryBlockRef* AllocateWrapper(Type *pType, void *pMemory, ui32 size);
        static MemoryBlock* AllocateMemoryBlockForExternal(Type *pType, void *pMemory, ui32 size);
        static Memory& GetInstance();
        static void SetInstance(Memory *pMemory);
        
        inline InstanceHandler* GetDestroyMemoryHandler() { return m_pDestroyMemoryHandler; }
        inline InstanceHandler* GetClearMemoryHandler() { return m_pClearMemoryHandler; }

        void* AllocFunctionStub(FunctionCallbackPtr *pFunctionCallbackPtr);
        MemoryBlock* QueryMemoryBlock(void *pMemory);
        MemoryBlock* GetReferencedMemoryBlock(Type *pType, MemoryBlock *pMemoryBlock, ui32 &offset);//, ui32 size);
        MemoryBlockRef* QueryMemoryBlockRef(void *pMemory, Type *pType);
        void DereferenceMemory(Type *pType, /*[in,out] parameter, returns dereferenced memory block*/MemoryBlock *&pMemoryBlock, /*[in,out] parameter, returns offset of returned memory block*/ui32 &offset);
        void RegisterMemoryBlock(MemoryBlock *pMemoryBlock);
        void UnRegisterMemoryBlock(void* pMemory);
        StackMemory* GetThreadMemory(ThreadId threadId);
        void GetThreadIds(vector<ThreadId> &threadIds);
        StackMemory* GetMainStackMemory() { return m_pMainStackMemory; };
        void ReleaseThreadMemory(ThreadId threadId);
        Scope* GetRootScope() { return m_pRootScope; };
        void SetRootScope(Scope *prootScope) { m_pRootScope = prootScope; };
#ifdef TRACE_OBJECTS
        void AddObject(Runable* p);
        void ReleaseObject(Runable* p);
        void DumpObjects();
#endif // TRACE_OBJECTS
#ifdef TRACE_MEMORY
        void AddMemoryBlock(MemoryBlock *p);
        void ReleaseMemoryBlock(MemoryBlock *p);
        void AddMemoryBlockRef(MemoryBlockRef *p);
        void ReleaseMemoryBlockRef(MemoryBlockRef *p);
        void DumpMemoryBlocks();
#endif // TRACE_MEMORY

    private:

        typedef struct Range {
            MemoryBlock *m_pMemoryBlock;
            ui32 m_beginAddress, m_endAddress;
            explicit Range(MemoryBlock  *pMemoryBlock) : m_pMemoryBlock(pMemoryBlock), m_beginAddress((ui32)*pMemoryBlock->GetRefToMemoryPtr()), m_endAddress(m_beginAddress + pMemoryBlock->GetSize()) {};
            explicit Range(ui32 address) : m_pMemoryBlock(0), m_beginAddress(address), m_endAddress(address) {};
            Range(const Range& that) : m_pMemoryBlock(0), m_beginAddress(0), m_endAddress(0) { *this = that; };

            inline bool operator < (const Range& x) const {
                return m_endAddress < x.m_beginAddress;
            }

            inline Range& operator=(const Range &that) {
                m_beginAddress = that.m_beginAddress;
                m_endAddress   = that.m_endAddress;
                m_pMemoryBlock = that.m_pMemoryBlock;
                return *this;
            }
        } RANGE;

        typedef vector<ui8*> MEMORY;
        typedef MEMORY       STACK;
        typedef MEMORY       CONSTS;
        typedef MEMORY       HEAP;
        typedef unordered_map<MEMORY_OFFSET, ui32>     HEAP_REF;
        typedef unordered_map<void*, MEMORY_OFFSET>    MEM_REFERENCES;
        typedef unordered_map<MEMORY_OFFSET, void*>    MEM_REFERENCES_REVERSE;
        typedef unordered_map<ThreadId, StackMemory* > STACK_MEM_BY_THREAD_ID;
        typedef set<RANGE>                             BLOCK_RANGES;
        typedef map<RANGE, ui32>                       REGISTERED_MEMORY;

        void Clean(MEMORY &vec);

        STACK_MEM_BY_THREAD_ID  m_stackMemoryByThreadId;
        BLOCK_RANGES            m_memoryBlockRanges;
        REGISTERED_MEMORY       m_registeredMemory;
        CRITICAL_SECTION        m_cs;
        StackMemory            *m_pMainStackMemory;
        Scope                  *m_pRootScope;
#if defined(TRACE_OBJECTS) || defined(TRACE_MEMORY)
        CRITICAL_SECTION        m_traceCS;
#endif // defined(TRACE_OBJECTS) || defined(TRACE_MEMORY)
#ifdef TRACE_OBJECTS
        typedef set<Runable*>   OBJECTS;
        OBJECTS                 m_objects;
#endif // TRACE_OBJECTS
#ifdef TRACE_MEMORY
        typedef set<MemoryBlock*>       MEMORY_BLOCKS;
        typedef set<MemoryBlockRef*>    MEMORY_BLOCK_REFS;

        MEMORY_BLOCKS           m_memoryBlocks;
        MEMORY_BLOCK_REFS       m_memoryBlockRefs;
#endif // TRACE_MEMORY
        void                   *m_pVirtualMemory;
        ui32                    m_virtualMemoryNextOffset;
#ifdef SCRIPT_DEBUGGER
        DebuggerHandler        *m_pDebuggerHandler;
#endif // SCRIPT_DEBUGGER
        SymbolStore            *m_pSymbolStore;
        StdOutInterface        *m_pStdOutInterface;
        InstanceHandler        *m_pDestroyMemoryHandler;
        InstanceHandler        *m_pClearMemoryHandler;
    };

    /*************************************************************/
    /*************************************************************/
    class Serializer;
    class PrePostExpressions;

    class Runable {
        PROTECT_CLASS(Runable)
    public:
        class Param {
            Param();
        public:
            Param(const Param& that);
            Param(Value *pValue, const Param& that);
            Param(Value *pValue, Memory *pMemory, StackMemory *pStackMemory);
            ~Param();

            Param& operator=(const Param& that);

            inline void SetCurrentScope(Scope *pCurrentScope) { m_pCurrentScope = pCurrentScope; }
            
            void ReleaseReferenceOfValue(bool final);
            void SetReferenceOfValue(MemoryBlock *pMemoryBlock, ui32 offset);
            bool OverwriteReferenceOfValue(MemoryBlockRef *pRightMemoryBlockRef);
            bool PopulateGlobals();

            Value        *m_pValue;
            Memory       *m_pMemory;
            StackMemory  *m_pStackMemory;
            Scope        *m_pCurrentScope;
            MemoryBlock  *m_pReferenceOfValueMemoryBlock;
            ui32          m_referenceOfValueOffset;
        };

        typedef enum {
            ASSOCIATIVITY_INVALID,
            LEFT_TO_RIGHT,
            RIGHT_TO_LEFT,
        } ASSOCIATIVITY;

        Runable(SYMBOL_DESC &symDesc, const NODE_TYPE type = TYPE_RUNABLE);
        virtual ~Runable();
        virtual RUNTIME_RETURN Run(Runable::Param &param) = 0;
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual void Serialize(Serializer *pSerializer);

        static bool IsValue(NODE_TYPE type);
        static bool IsOperator(NODE_TYPE type);

        inline NODE_TYPE GetType() { return m_type; }
        inline void GetSymbolDesc(SYMBOL_DESC &symDesc) { symDesc = m_symDesc; }
        inline SYMBOL_ID GetSymbolId() { return m_symDesc.m_symId; }
        inline ui32 GetLine() { return m_symDesc.m_line; }
        inline ui32 GetPosition() { return m_symDesc.m_pos; }
        inline ui32 GetFileId() { return m_symDesc.m_fileId; }
        inline void SetSymbolId(SYMBOL_ID symId) { m_symDesc.m_symId = symId; }
        inline void SetSymbolLocation(ui32 fileId, ui32 line, ui32 pos) {
            m_symDesc.m_fileId = fileId; m_symDesc.m_line = line; m_symDesc.m_pos = pos;
        };

        template<class T> T* QueryType(NODE_TYPE type) { return CanTypecast(m_type, type) ? dynamic_cast<T*>(this) : 0; }

        PRECEDENCE GetPrecedence();
        ASSOCIATIVITY GetAssociativity();
    protected:
        SYMBOL_DESC m_symDesc;
        NODE_TYPE   m_type;
    protected:
        virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);
        virtual void SerializeEnd(Serializer *pSerializer);

        static bool CanTypecast(NODE_TYPE typeFrom, NODE_TYPE typeTo);
    };

    /*************************************************************/
    class Variable;
    class Function;

    class Node : public Runable {
        PROTECT_CLASS(Node)
        Node(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type);
        Node(Node *pParent, const NODE_TYPE type);
    public:
        virtual ~Node();
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);

        inline void SetParent(Node* pParent) { m_pParent = pParent; };
        inline void SetLeft(Node* pLeft) { m_pLeft = pLeft; };
        inline void SetRight(Node* pRight) { m_pRight = pRight; };
        inline Node* GetParent() { return m_pParent; };
        inline Node* GetLeft() { return m_pLeft; };
        inline Node* GetRight() { return m_pRight; };

        Node* GetExpressionRoot();
        Node* GetParent(NODE_TYPE type);
        Function* GetFunctionNode();
        Node* GetBreakableNode();
        Node* GetLoopNode();
        Node* GetRootValueNode();
        bool IsFunctionCall();
        bool IsAssignOp();
        Variable *FindVariableDec(SYMBOL_ID symId);

        template<class T = Node> T* GetParent(NODE_TYPE type);

    protected:
        template<class T> struct Free {
            inline void operator() (T* p) { if (p) delete p; };
        };
        template<class T> void Delete(vector<T*> &v) { for_each(v.rbegin(), v.rend(), Free<T>()); };
#ifdef SCRIPT_DEBUGGER
        RUNTIME_RETURN CheckFlow(Node *pNode, Runable::Param &param);
#endif // SCRIPT_DEBUGGER


        Node *m_pParent,
             *m_pLeft,
             *m_pRight;
    private:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    template<class T> T* Node::GetParent(NODE_TYPE type) {
        Node* pParent = m_pParent;
        while (pParent) {
            if (pParent->QueryType<T>(type)) {
                break;
            }
            pParent = pParent->GetParent();
        }
        return static_cast<T*>(pParent);
    }

    /*************************************************************/

    class FunctionRefType;
    class FunctionCallbackPtr;

    class Function : public Node {
        PROTECT_CLASS(Function)
    public:
        Function(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type = TYPE_FUNCTION);
        virtual ~Function();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual void PushParameters(Runable::Param &param, vector<Value> &values);
        virtual void* GetRawPtr(Runable::Param &param);
        virtual FunctionRefType* GetFunctionRefTypeNode() { return m_pRefType; };
        virtual Type* GetReturnType();

        inline ui32 GetArgCount() { return m_argVariables.size(); };
        inline vector<SYMBOL_DESC>& GetArgumentVector() { return m_argOrder; };

        bool PushArgInfo(SYMBOL_DESC &argSymDesc, Type *pArgType);
        Type* GetArgumentType(SYMBOL_ID argSymId);
        bool SetArgumentVariable(SYMBOL_ID symId, Variable *pVariable);
        bool IsArgument(SYMBOL_ID symId);
        SYMBOL_ID GetArgSymId(ui32 argIdx);
        Variable* GetArgumentNode(SYMBOL_ID symId);
        ui32 GetArgDWORDCount();
        FunctionCallbackPtr* GetFunctionCallbackPtr();
        MEMORY_OFFSET ResolveArg(ui32 idx, Type *&pResolvedType);
    protected:
        void PushParameter(Runable::Param &param, ui32 paramIdx, Value &value, Type *pType);
        typedef unordered_map<SYMBOL_ID, Type*> VARIABLE_REF_MAP;

        VARIABLE_REF_MAP    m_argVariables;
        vector<SYMBOL_DESC> m_argOrder;
        FunctionRefType    *m_pRefType;
    private:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/

    class SizeOf : public Function {
        PROTECT_CLASS(SizeOf)
    public:
        SizeOf(Node *pParent, SYMBOL_DESC &symDesc);
        virtual ~SizeOf();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual void PushParameters(Runable::Param &param, vector<Value> &values);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

    private:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/
    class ArrayType;

    class BuiltIn1Arg : public Function {
        PROTECT_CLASS(BuiltIn1Arg)
    public:
        BuiltIn1Arg(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) : Function(pParent, symDesc, type), m_pArgType(0) {};
        virtual ~BuiltIn1Arg();

        virtual void PushParameters(Runable::Param &param, vector<Value> &values);
        Type *GetArgType() { return m_pArgType; }
    protected:
        virtual void SerializeBody(Serializer *pSerializer);

        Type *m_pArgType;
    };

    /*************************************************************/

    class Arr2Str : public BuiltIn1Arg {
        PROTECT_CLASS(Arr2Str)
    public:
        Arr2Str(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc, NODE_TYPE type = TYPE_ARR2STR);
        virtual RUNTIME_RETURN Run(Runable::Param &param);
    protected:
        virtual void SetParam(Runable::Param &param, void *pData, ui32 size);
    };

    /*************************************************************/

    class WArr2Str : public Arr2Str {
        PROTECT_CLASS(WArr2Str)
    public:
        WArr2Str(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc);
        virtual ~WArr2Str();
    protected:
        virtual void SetParam(Runable::Param &param, void *pData, ui32 size);
    };

    /*************************************************************/

    class Str2Arr : public BuiltIn1Arg {
        PROTECT_CLASS(Str2Arr)
    public:
        Str2Arr(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc, NODE_TYPE type = TYPE_STR2ARR);
        virtual RUNTIME_RETURN Run(Runable::Param &param);
    protected:
        virtual void SetParam(Runable::Param &param, void *pData, ui32 size);
    };

    /*************************************************************/

    class Str2WArr : public Str2Arr {
        PROTECT_CLASS(Str2WArr)
    public:
        Str2WArr(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc);
        //virtual ~Str2WArr();
        //virtual RUNTIME_RETURN Run(Runable::Param &param);
    protected:
        virtual void SetParam(Runable::Param &param, void *pData, ui32 size);
    };

    /*************************************************************/

    class Lock : public Function {
        PROTECT_CLASS(Lock)
    public:
        Lock(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type = TYPE_LOCK) : Function(pParent, symDesc, type) {};
        //virtual ~Lock() {};

        virtual void PushParameters(Runable::Param &param, vector<Value> &values);
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

    protected:
        //virtual void SerializeBody(Serializer *pSerializer) {};
    };

    /*************************************************************/

    class UnLock : public Lock {
        PROTECT_CLASS(UnLock)
    public:
        UnLock(Node *pParent, SYMBOL_DESC &symDesc) : Lock(pParent, symDesc, TYPE_UNLOCK) {};
        //virtual ~UnLock() {};

        virtual RUNTIME_RETURN Run(Runable::Param &param);

    protected:
        //virtual void SerializeBody(Serializer *pSerializer) {};
    };

    /*************************************************************/

    class Cast : public Function {
        PROTECT_CLASS(Cast)
    public:
        Cast(Node *pParent, SYMBOL_DESC &symDesc) : Function(pParent, symDesc, TYPE_CAST) {};
        //virtual ~UnLock() {};

        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual void PushParameters(Runable::Param &param, vector<Value> &values);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

    protected:
        //virtual void SerializeBody(Serializer *pSerializer) {};
    };

    /*************************************************************/

    class AggregateType;
    class FunctionPtr;

    typedef struct FunctionPtrWrapper {
        FunctionPtr **m_pRefFunctionPtr;
        FunctionPtr  *m_pFunctionPtr;
        FunctionPtrWrapper(FunctionPtr *pPtrFunctionCall);
        FunctionPtrWrapper(const FunctionPtrWrapper &that);
        FunctionPtrWrapper& operator=(const FunctionPtrWrapper &that);
        ~FunctionPtrWrapper();
    private:
        FunctionPtrWrapper();
    } FUNCTION_PTR_WRAPPER;
    
    /*************************************************************/

    typedef unordered_map<SYMBOL_ID, Function*>                 FUNCTION_REF_MAP;
    typedef set<void*>                                          FUNCTION_REF_SET;
    typedef unordered_map<void*, FUNCTION_PTR_WRAPPER*>         FUNCTION_PTR_WRAPPER_MAP;
    typedef unordered_map<void*, FunctionCallbackPtr*>          FUNCTION_CALLBACK_PTR_MAP;
    typedef unordered_map<SYMBOL_ID, AggregateType*>            AGGREGATE_REF_MAP;
    
    /*************************************************************/

    class AggregateDefCollection : public Node {
        PROTECT_CLASS(AggregateDefCollection)
    public:
        AggregateDefCollection(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) : Node(pParent, symDesc, type) {};
        virtual ~AggregateDefCollection();
        virtual bool PushAggregateDef(AggregateType *pAggregateType);
        virtual AggregateType* ResolveAggregateType(SYMBOL_ID symId);
    protected:
        AGGREGATE_REF_MAP   m_aggregateDefs;
    };

    /*************************************************************/
    class ErrorVariable;

    class Scope : public AggregateDefCollection {
        PROTECT_CLASS(Scope)
    public:
        Scope(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type = TYPE_SCOPE);
        virtual ~Scope();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool PushAggregateDef(AggregateType *pAggregateType);
        virtual bool PushFunctionDef(Function *pFunction);
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);

        static FunctionPtr* GetFunctionPtr(void *pFooPtr);
        static bool RegisterFunctionPtr(void *pFooPtr, FunctionPtr* pFunctionPtr);
        static void UnRegisterFunctionPtr(void *pFooPtr);
        static Type* GetDummyTypeNode(NODE_TYPE type);

        inline MEMORY_OFFSET GetEndOfAlignedStack() { return m_unalignedEndOfStack; };
        inline vector<LOCAL_VAR_DESCRIPTOR>& GetLocalVarDescriptors() { return m_localVarTypes; };
        inline ui32 GetFunctionDefsCount() { return m_functionDefs.size(); }
#ifdef SCRIPT_DEBUGGER
        inline vector<Node*>& GetExpressionsRef() { return m_expressions; }
        inline unordered_set<ui64>& GetActiveBreakPointLines() { return m_activeBreakPointLines;  }
        inline void SetBreakPointLineByFileId(BREAK_POINT_LINES_BY_FILE_ID *pAllBreakPointLinesByFileId) { assert(pAllBreakPointLinesByFileId); m_pAllBreakPointLinesByFileId = pAllBreakPointLinesByFileId; };
        inline DebuggerHandler* GetDebuggerHandler() { return m_pDebuggerHandler; }
#endif // SCRIPT_DEBUGGER

        void PushExpression(Node* pNode);
        bool UpdateExpression(Node* pNode);
        Function* ResolveFunction(SYMBOL_ID symId);
        Variable* FindTypeVariableDeclNode(SYMBOL_ID symId);
        bool ReserveAlignedStackSopt(
#ifdef SCRIPT_DEBUGGER
            SYMBOL_DESC &symDesc, 
#endif // SCRIPT_DEBUGGER
            Type *pType, MEMORY_OFFSET &alignedStackOffset, ui32 &idx);
        ErrorVariable *GetErrorVariable();

#ifdef SCRIPT_DEBUGGER
        void RegisterBreakPointLine(Node* pNode);
        void SetDebuggerHandler(DebuggerHandler *m_pDebuggerHandler);
        bool SetBreakpoint(ui32 fileId, ui32 line, bool set);
        typedef union {
            struct {
                ui32 fileId, line;
            };
            ui64 breakpoint;
        } BREAKPOINT_LOCATION;

    private:

        unordered_set<ui64>              m_activeBreakPointLines;
        DebuggerHandler                 *m_pDebuggerHandler;
        BREAK_POINT_LINES_BY_FILE_ID    *m_pAllBreakPointLinesByFileId;
#endif // SCRIPT_DEBUGGER

    protected:
        vector<Node*>                           m_expressions;
        MEMORY_OFFSET                           m_unalignedEndOfStack;
        FUNCTION_REF_MAP                        m_functionDefs;

        static FUNCTION_PTR_WRAPPER_MAP         s_functionPtrWrapperMap;
    private:
        virtual void SerializeBody(Serializer *pSerializer);

        void InitDummyTypes();
        //void InitBuiltInFunctions();

        vector<LOCAL_VAR_DESCRIPTOR>            m_localVarTypes;
        
        static vector<Type*>                    s_dummyTypes;
    public: 
        TODO("Refactor it!");
        static SYMBOL_ID                        s_reservedFooLower,
                                                s_reservedFooUpper;
    };

    /*************************************************************/
    class Operator;

    class PrePostExpressions: public Node {
        PrePostExpressions() {};
        PrePostExpressions(const PrePostExpressions&) {};
        PrePostExpressions& operator=(const PrePostExpressions&) {};
    public:
        PrePostExpressions(SYMBOL_DESC &symDesc) : Node(0, symDesc, TYPE_PRE_POST_EXPRESSIONS), m_pExpression(0) {};
        virtual ~PrePostExpressions();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

        inline Node* GetExpressionNode() { return m_pExpression; };
        inline void SetExpressionNode(Node *pExpNode) { m_pExpression = pExpNode; };

        RUNTIME_RETURN RunPre(Runable::Param &param);
        RUNTIME_RETURN RunPost(Runable::Param &param);
        void PushPreExpression(Operator* pOperator);
        void PushPostExpression(Operator* pOperator);
    protected:
        virtual void Serialize(Serializer *pSerializer);
    private:
        vector<Operator*>   m_preExpressions;
        vector<Operator*>   m_postExpressions;
        Node               *m_pExpression;
    };

    /*************************************************************/

    class Reference: public Node {
        PROTECT_CLASS(Reference)
    public:
        Reference(Node *pParent, SYMBOL_DESC &symDesc) : Node(pParent, symDesc, TYPE_REF) {};
        virtual ~Reference() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
    };

    /*************************************************************/

    class Return : public Node {
        PROTECT_CLASS(Return)
    public:
        Return(Node *pParent, SYMBOL_DESC &symDesc) : Node(pParent, symDesc, TYPE_RETURN) {};
        virtual ~Return() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);

    protected:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/

    class Break : public Node {
        PROTECT_CLASS(Break)
    public:
        Break(Node *pParent, SYMBOL_DESC &symDesc) : Node(pParent, symDesc, TYPE_BREAK) {};
        virtual ~Break() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
    };

    /*************************************************************/

    class Continue : public Node {
        PROTECT_CLASS(Continue)
    public:
        Continue(Node *pParent, SYMBOL_DESC &symDesc) : Node(pParent, symDesc, TYPE_CONTINUE) {};
        virtual ~Continue() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
    };

    /*************************************************************/

    class ConditionalNode : public Node {
        PROTECT_CLASS(ConditionalNode)
    public:
        ConditionalNode(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) : Node(pParent, symDesc, type), m_pCondition(0) {};
        virtual ~ConditionalNode();

        void SetCondition(Node *pExp);
        Node* GetCondition() { return m_pCondition; };
    protected:
        virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);

        Node *m_pCondition;
    };

    /*************************************************************/

    class IfNode : public ConditionalNode {
        PROTECT_CLASS(IfNode)
    public:
        IfNode(Node *pParent, SYMBOL_DESC &symDesc) : ConditionalNode(pParent, symDesc, TYPE_IF), m_IfClause(true) {};
        virtual ~IfNode() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);

        void PushExpression(Node *pNode);
        void UpdateExpression(Node *pNode);
        void EndIfClause() { m_IfClause = false; };
    private:
        virtual void SerializeBody(Serializer *pSerializer);

        bool m_IfClause;
    };

    /*************************************************************/

    class BreakableNode : public ConditionalNode {
        PROTECT_CLASS(BreakableNode)
    public:
        BreakableNode(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) : ConditionalNode(pParent, symDesc, type)/*, m_break(false)*/ {};
        virtual ~BreakableNode() {};
    };

    /*************************************************************/

    class ForNode : public BreakableNode {
        PROTECT_CLASS(ForNode)
        typedef enum {
            INIT_EXP,
            COND_EXP,
            LOOP_EXP,
            BODY_EXP
        } EXP_TYPE;
    public:
        typedef void (ForNode::*APPEND_VALUE_NODE_2_FOR)(Node*);

        ForNode(Node *pParent, SYMBOL_DESC &symDesc) : BreakableNode(pParent, symDesc, TYPE_FOR), m_pInitExp(0),/* m_pCondExp(0),*/ m_expType(INIT_EXP) {};
        virtual ~ForNode();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);

        void SetInitExp(Node *pNode);
        void SetCondExp(Node *pNode);
        void SetLoopExp(Node *pNode);
        bool UpdateExpression(Node *pNode);
    private:
        virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);

        Node    *m_pInitExp;
        EXP_TYPE m_expType;
    };

    /*************************************************************/

    class WhileNode : public BreakableNode {
        PROTECT_CLASS(WhileNode)
    public:
        WhileNode(Node *pParent, SYMBOL_DESC &symDesc) : BreakableNode(pParent, symDesc, TYPE_WHILE) {};
        virtual ~WhileNode() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);

        void UpdateExpression(Node *pNode);
        void PushExpression(Node *pNode);
    private:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/

    class SwitchNode : public BreakableNode {
        PROTECT_CLASS(SwitchNode)
    public:
        SwitchNode(Node *pParent, SYMBOL_DESC &symDesc) : BreakableNode(pParent, symDesc, TYPE_SWITCH), m_defaultIdx(-1) {};
        virtual ~SwitchNode();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);

        bool SetCase(ui32 value);
        bool UpdateCaseExp(Node *pExpNode);
        bool SetDefault(Node *pExpNode);
    private:
        virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);
        virtual void SerializeEnd(Serializer *pSerializer);

        typedef unordered_map<ui32, ui32> CASES_MAP;

        CASES_MAP               m_cases; // this map stores offsets in m_casesFlow array
        vector<Node*>           m_casesFlow;
        ui32                    m_defaultIdx;
    };

    /*************************************************************/

    class AggregateSymbol {
    protected:
        AggregateSymbol() : m_pSymbolStore(0) {};
    public:
        AggregateSymbol(SymbolStore *pSymbolStore): m_pSymbolStore(pSymbolStore) {};
        virtual ~AggregateSymbol() {};
        virtual void SerializeBody(Serializer *pSerializer);

        inline void AddSymbol(SYMBOL_ID symId) { m_symbolIds.push_back(symId); };

        bool GetString(string &value);
        ui32 GetSymbolCount() { return m_symbolIds.size(); };
        SYMBOL_ID GetSymbolId(ui32 idx) { return idx < m_symbolIds.size() ? m_symbolIds[idx] : -1; };
    protected:
        vector<SYMBOL_ID>   m_symbolIds;
    private:
        SymbolStore *m_pSymbolStore;
    };

    /*************************************************************/

    class CaseNode : public AggregateSymbol, public BreakableNode {
        PROTECT_CLASS(CaseNode)
    public:
        CaseNode(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc) : AggregateSymbol(pSymbolStore), BreakableNode(pParent, symDesc, TYPE_CASE) {};
        virtual ~CaseNode() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
    private:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/

    class DefaultNode : public BreakableNode {
        PROTECT_CLASS(DefaultNode)
    public:
        DefaultNode(Node *pParent, SYMBOL_DESC &symDesc) : BreakableNode(pParent, symDesc, TYPE_DEFAULT) {};
        virtual ~DefaultNode() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
    private:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/
    class InstanceHandler {
        PROTECT_CLASS(InstanceHandler)
    public:
        InstanceHandler(Memory *pMemory) : m_pMemory(pMemory) {};
        virtual ~InstanceHandler() {}
        virtual void Run(bool reference, Type *pType, MemoryBlock *pMemoryBlock, ui32 offset);

        inline Memory* GetMemory() { return m_pMemory; }
    protected:

        Memory *m_pMemory;
    };

    /*************************************************************/

    class DestroyMemoryHandler : public InstanceHandler {
        PROTECT_CLASS(DestroyMemoryHandler)
    public:
        DestroyMemoryHandler(Memory *pMemory) : InstanceHandler(pMemory) {};
        virtual ~DestroyMemoryHandler() {}
        virtual void Run(bool reference, Type *pType, MemoryBlock *pMemoryBlock, ui32 offset);
    };

    /*************************************************************/

    class ClearMemoryHandler : public InstanceHandler {
        PROTECT_CLASS(ClearMemoryHandler)
    public:
        ClearMemoryHandler(Memory *pMemory) : InstanceHandler(pMemory) {};
        virtual ~ClearMemoryHandler() {}
        virtual void Run(bool reference, Type *pType, MemoryBlock *pMemoryBlock, ui32 offset);
    };

    /*************************************************************/

    class Type: public AggregateDefCollection {
        PROTECT_CLASS(Type)
    public:
        Type(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) : AggregateDefCollection(pParent, symDesc, type) {};
        virtual ~Type();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual ui32 GetTypeSize();
        virtual ui32 GetValueSize();
        virtual ui32 GetAlignment();
        virtual bool IsRelatedType(Type *pType);
        virtual bool IsEqualType(Type *pType);
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);
        virtual void SerializeBody(Serializer *pSerializer);
        virtual Value::VALUE_TYPE GetValueType(); 
        virtual void CopySubValues(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset);
        virtual MemoryBlock* FindMemoryBlock(const char * const path, MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem);
        virtual string GetTypeString(SymbolStore *pSymStore);
        virtual void HandleInstance(InstanceHandler *pInstanceHandler, MemoryBlock *pMemoryBlock, ui32 offset);
        virtual bool IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath);

        static NODE_TYPE ValueTypeToNodeType(Value::VALUE_TYPE type);
        
        MemoryBlock* GetMemoryBlock(MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem);
        ui32 GetVarOffset(ui32 idx);
        void PushVariable(Variable* pVariable);
        bool UpdateExpression(Node* pNode);
        Variable* FindVariableDecNode(SYMBOL_ID symId);
        Variable* GetVariable(ui32 idx);
        ui32 GetVarCount() { return m_variables.size(); };
        Node* GetLastDecVariable();
        bool Align(AGGREGATE_TYPE_OFFSET baseOffset);
        bool IsReference();
        void CopyVariableMemory(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset, ui32 variablemAlignedMemberOffset, bool reference);
        void CopyMemoryType(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset, bool reference);
        void CopyVariables(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset);
        void HandleEachInstance(bool reference, InstanceHandler *pInstanceHandler, MemoryBlock *pMemoryBlock, ui32 offset);

        typedef enum {
            BY_VALUE,
            BY_REF,
            BY_REF_TO_REF
        } PASS_BY;
        PASS_BY GetPassBy();

    protected:
        virtual void SerializeEnd(Serializer *pSerializer);

        const char * const GetSubPath(const char * path, ui32 &idx);
        MemoryBlock *GetDerefMemoryBlock(MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem);

        vector<Node*>   m_variables;
    };

    /*************************************************************/

    class AggregateType : public Type {
        PROTECT_CLASS(AggregateType)
    public:
        AggregateType(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type = TYPE_AGGREGATE_TYPE);
        virtual ~AggregateType();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual ui32 GetTypeSize();
        virtual ui32 GetValueSize();
        virtual ui32 GetAlignment();
        virtual bool IsRelatedType(Type *pType);
        virtual bool IsEqualType(Type *pType);
        virtual void CopySubValues(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset);
        virtual MemoryBlock* FindMemoryBlock(const char * const path, MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem);
        
        virtual void HandleInstance(InstanceHandler *pInstanceHandler, MemoryBlock *pMemoryBlock, ui32 offset);
        virtual bool IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath);

        void AddType(Type* pType);
        Variable* FindMemberDecNode(SYMBOL_ID symId);
        void SetTypeDefinition(AggregateType *pAggregateType);
        MEMORY_OFFSET GetNextAlignedOffset(NODE_TYPE type);
        bool AlignStruct(AGGREGATE_TYPE_OFFSET baseOffset);
        ui32 GetSubTypeCount();
        Type* GetSubType(ui32 idx);
    protected:

        virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);

        ui32 GetRawAlignment();

        vector<Type*>           m_types;
        AggregateType          *m_pTypeDefinition;
        ui32                    m_alignedSize;
        ui32                    m_alignment;
    };

    /*************************************************************/
    class LibNode;

    class LibType : public AggregateType {
        PROTECT_CLASS(LibType)
    public:
        LibType(Node *pParent, SYMBOL_DESC &symDesc);
        virtual ~LibType();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath);

        void SetPath(string &path) { m_path = path; };
        string &GetPath() { return m_path; };
        void *GetModuleHandle();
        LibNode* GetLibNode();
        FunctionRefType* GetFunctionRefType(SYMBOL_ID symId);
    private:
        virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);

        void SerializeType(Serializer *pSerializer, Type *pType);

        string  m_path;
    };

    /*************************************************************/

    class FunctionRefType : public Type {
        PROTECT_CLASS(FunctionRefType)
    public:
        FunctionRefType(Node *pParent, SYMBOL_DESC &symDesc): Type(pParent, symDesc, TYPE_FUNCTION_REF_TYPE) {};
        virtual ~FunctionRefType();
        virtual bool IsRelatedType(Type *pType);
        virtual bool IsEqualType(Type *pType);
        virtual void SerializeBegin(Serializer *pSerializer);
        virtual string GetTypeString(SymbolStore *pSymStore);
        virtual bool IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath);

        inline Type* GetReturnType() { return m_argDescs[0]; };

        void SetReturnType(Type *pType);
        void PushArgDesc(LOCAL_VAR_DESCRIPTOR &varDesc);
        ui32 GetArgCount() { return m_argDescs.size() ? m_argDescs.size() - 1 : 0; };
        Type* GetArgType(ui32 i);
        void PopulateTypes(Function *pFrom);
        void GetArgDescs(vector<LOCAL_VAR_DESCRIPTOR> &argDescs);
    private:
        vector<LOCAL_VAR_DESCRIPTOR>    m_argDescs;
    };

    /*************************************************************/
    class ArrayType : public AggregateSymbol, public Type {
        PROTECT_CLASS(ArrayType)
    public:
        ArrayType(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc) : AggregateSymbol(pSymbolStore), Type(pParent, symDesc, TYPE_ARRAY), m_pTypeNode(0), m_pDynExp(0) {};
        virtual ~ArrayType();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual ui32 GetTypeSize();
        virtual ui32 GetValueSize();
        virtual ui32 GetAlignment();
        virtual Value::VALUE_TYPE GetValueType();
        virtual bool IsRelatedType(Type *pType);
        virtual bool IsEqualType(Type *pType);
        virtual void CopySubValues(Memory &memory, MemoryBlock *pDestMemoryBlock, ui32 destOffset, MemoryBlock *pSrcMemoryBlock, ui32 srcOffset);
        virtual MemoryBlock* FindMemoryBlock(const char * const path, MemoryBlock *pMemoryBlock, ui32 offset, ui32 &offsetOut, Memory &mem);
        virtual string GetTypeString(SymbolStore *pSymStore);
        virtual void HandleInstance(InstanceHandler *pInstanceHandler, MemoryBlock *pMemoryBlock, ui32 offset);
        virtual bool IsSafeType(Error &error, vector<SYMBOL_DESC> &symbolPath);

        inline Type* GetTypeNode() { return m_pTypeNode; };
        
        bool SetType(Type *pType);
        void PushDimensionSize(ui32 size) { m_dimensions.push_back(size); };
        ui32 GetDimensionCount() { return m_dimensions.size(); };
        ui32 GetDimensionSubscript(ui32 dimension);
        void SetDynExpression(Node *pNode);
        Node* GetDynExpression();
    protected:
        //virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);
        //virtual void SerializeEnd(Serializer *pSerializer);

    private:
        vector<ui32>    m_dimensions;
        Type           *m_pTypeNode;
        Node           *m_pDynExp;
     };

    /*************************************************************/

    class ErrorType : public AggregateType {
        PROTECT_CLASS(ErrorType)
    public:
        ErrorType(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc);
        virtual ~ErrorType() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
    private:
        virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);
   
        void InitMember(string &name, NODE_TYPE type);
    private:
        SymbolStore *m_pSymbolStore;
    };

    /*************************************************************/
    class Variable : public Node {
        PROTECT_CLASS(Variable)
    public:
        Variable(Node *pParent, 
                 SYMBOL_DESC &symDesc, 
                 ui32 idx,
                 MEMORY_OFFSET alignedOffset,
                 AGGREGATE_TYPE_OFFSET alignedMemberOffset,
                 Type* pType = 0, 
                 const NODE_TYPE type = TYPE_VAR);
        virtual ~Variable() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

        inline NODE_TYPE GetResolvedType() { return  m_pType ? m_pType->GetType() : TYPE_VAR; };
        inline Type* GetTypeNode() { return m_pType; };
        inline void SetTypeNode(Type *pType) { m_pType = pType; };
        inline ui32 GetVarIdx() { return m_idx; };
        inline MEMORY_OFFSET GetAlignedOffset() { return m_alignedOffset; };
        inline AGGREGATE_TYPE_OFFSET   GetAlignedMemberOffset() { return m_alignedMemberOffset; };
        inline bool IsConst() { return (m_type == TYPE_CONST) || (m_type == TYPE_CONST_BOOL); };

        MEMORY_OFFSET GetEndOfAlignedOffset();
        bool SetAlignedMemberOffset(AGGREGATE_TYPE_OFFSET lastOffset);
        AGGREGATE_TYPE_OFFSET GetEndOfAlignedMemberOffset();
        Variable *GetRootVariableNode();
        void SetPreOperator(Node *pPreOperator, Node* pNewParent);
        void SetPostOperator(Node *pPostOperator);
        Variable* GetSubVariable();
        Node* FindFirstMemberSubscriptOrFooCall();

    private:
        virtual void SerializeBody(Serializer *pSerializer);

        Node* GetPrePostExpressionParent();
        void SerializePreExp(Node *pNode, Serializer *pSerializer);
        void SerializePostExp(Node *pNode, Serializer *pSerializer);
        ui32 m_idx;

        MEMORY_OFFSET           m_alignedOffset;
        AGGREGATE_TYPE_OFFSET   m_alignedMemberOffset;
        Type                   *m_pType;
    };

    /*************************************************************/

    class LibNode : public Variable {
        PROTECT_CLASS(LibNode)
    public:
        LibNode(Node *pParent, 
                SYMBOL_DESC &symDesc, 
                ui32 idx,
                MEMORY_OFFSET alignedOffset,
                Type *pType) : 
                Variable(pParent, 
                         symDesc, 
                         idx,
                         alignedOffset, 
                         0,
                         pType, 
                         TYPE_LIB_VAR), m_hModule(0) {};
        virtual ~LibNode();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

        inline bool HasName() { return !m_modulePath.empty(); };

        void *GetModuleHandle();
    protected:
        virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);
    private:
        string      m_modulePath;
        void       *m_hModule;
    };

    /*************************************************************/

    class ErrorVariable : public Variable {
        PROTECT_CLASS(ErrorVariable)
    public:
        ErrorVariable(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc, ui32 idx, Type *pType) :
            Variable(pParent, symDesc, idx, 0, 0, pType, TYPE_ERROR_VAR), m_pSymbolStore(pSymbolStore) {};
        virtual ~ErrorVariable() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

        void SetError(Runable::Param &param, RunTimeError &error);
    protected:
        //virtual void SerializeBegin(Serializer *pSerializer);
        virtual void SerializeBody(Serializer *pSerializer);
        //virtual void SerializeEnd(Serializer *pSerializer);
    private:
        SymbolStore *m_pSymbolStore;
    };

    /*************************************************************/

    class CastVariable : public Variable {
        PROTECT_CLASS(CastVariable)
    public:
        CastVariable(Node *pParent, SYMBOL_DESC &symDesc, ui32 idx, Function *pFunction);
        virtual ~CastVariable();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

    protected:
        //virtual void SerializeBegin(Serializer *pSerializer);
        //virtual void SerializeBody(Serializer *pSerializer);
    private:
    };

    /*************************************************************/

    class ConstVariable : public AggregateSymbol, public Variable {
        //PROTECT_CLASS(ConstVariable)
    protected: ConstVariable() ; ConstVariable(const ConstVariable&); ConstVariable& operator=(const ConstVariable&);
    public:
        ConstVariable(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc) : 
            AggregateSymbol(pSymbolStore),
            Variable(pParent, 
                     symDesc,
                     0,
                     INVALID_MEMORY_OFFSET, 
                     0,
                     0, 
                     TYPE_CONST) {};
        virtual ~ConstVariable() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);

        inline Value& GetRef() { return m_value; };
    private:
        virtual void SerializeBody(Serializer *pSerializer);

        ui64 m_holder;
        Value m_value;
    };

    /*************************************************************/

    //class ConstAlias : public ConstVariable {
    //    //PROTECT_CLASS(ConstVariable)
    //protected: ConstAlias(); ConstAlias(const ConstAlias&); ConstAlias& operator=(const ConstAlias&);
    //public:
    //    ConstAlias(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc) :
    //        ConstVariable(pParent,
    //            symDesc,
    //            0,
    //            INVALID_MEMORY_OFFSET,
    //            0,
    //            0,
    //            TYPE_CONST) {};
    //    virtual ~ConstAlias() {};
    //    //virtual RUNTIME_RETURN Run(Runable::Param &param);

    //    //inline Value& GetRef() { return m_value; };
    //private:
    //    //virtual void SerializeBody(Serializer *pSerializer);

    //    //ui64 m_holder;
    //    //Value m_value;
    //};

    /*************************************************************/

    class Operator : public Node {
        PROTECT_CLASS(Operator)
    public:
        Operator(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type = TYPE_OPERATOR) : Node(pParent, symDesc, type) {};
        virtual ~Operator() {};
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual bool IsApplicable(Type *pLeftType, Type *pRightType);
    protected:
        bool IsApplicableFixed(Type *pLeftType, Type *pRightType);
        bool IsApplicableAnyNum(Type *pLeftType, Type *pRightType);
    private:
        virtual void SerializeBegin(Serializer *pSerializer) {};
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/

    class ErrorNode : public Node {
        PROTECT_CLASS(ErrorNode)
    public:
        ErrorNode(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type = TYPE_ERROR) : Node(pParent, symDesc, type) {};
        virtual ~ErrorNode() { SetRight(0); };
        virtual RUNTIME_RETURN Run(Runable::Param &param);
    private:
        //virtual void SerializeBegin(Serializer *pSerializer) {};
        virtual void SerializeBody(Serializer *pSerializer);
    };


    /*************************************************************/

    class FunctionCall : public Node {
        PROTECT_CLASS(FunctionCall)
    public:
        FunctionCall(Node *pParent, SYMBOL_DESC &symDesc) : Node(pParent, symDesc, TYPE_FUNCTION_CALL) {};
        virtual ~FunctionCall();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual Node* GetArgNode(ui32 idx);
        virtual FunctionRefType* GetFunctionRefTypeNode();
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);
        
        inline ui32 GetArgCount() { return m_arguments.size(); };

        bool PushArgument(Node *pArgNode);
        bool UpdateArgument(Node *pArgNode);
        ui32 GetArgDWORDCount();

    protected :
        MEMORY_OFFSET ResolveArg(Function *pFunction, ui32 idx, Type *&pResolvedType);
    private:
        virtual void SerializeBody(Serializer *pSerializer);

        RUNTIME_RETURN EvalParamExpresions(Runable::Param &param, vector<Value> &values);
        Function* GetFunctionImpl(Runable::Param &param);
    protected:
        vector<Node*>    m_arguments;
    };

    /*************************************************************/

    typedef struct FunctionPtrHeader {
        FunctionCallbackPtr *m_pFunctionCallbackPtr;
        void *m_pFunctionPtrStub;
        static  FunctionPtrHeader* TypeCast(void *p);
        bool IsValid();
        void Init(FunctionCallbackPtr *pFunctionCallbackPtr);
        static FunctionCallbackPtr* GetFunctionCallbackPtr(void* p);
    private:
        FunctionPtrHeader();
        FunctionPtrHeader(const FunctionPtrHeader&);
        FunctionPtrHeader& operator=(const FunctionPtrHeader&);
    }  FUNCTION_PTR_HEADER;

    /*************************************************************/

    typedef int(__stdcall *LIB_FOO)();

    /*************************************************************/

    class FunctionPtr : public Function {
        PROTECT_CLASS(FunctionPtr)
    public:
        FunctionPtr(Node *pParent, SYMBOL_DESC &symDesc, void *pFooPtr, const NODE_TYPE type = TYPE_FUNCTION_PTR) : Function(pParent, symDesc, type), m_pFooPtr(pFooPtr) {};
        virtual ~FunctionPtr() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);

        virtual FunctionRefType* GetFunctionRefTypeNode();

        typedef enum {
            REG_EAX,
            REG_EAX_EDX,
            POINTER_EAX
        } FOO_RETURN_METHOD;

    protected:
        typedef struct DataReference {
            Type            *m_pType;
            MemoryBlockRef *m_pMemoryBlockRef;
            void **m_pDataRef;
            void  *m_pData;
            DataReference(Memory &memory, Value& value, Type *pType);
            ~DataReference();
        private:
            DataReference();
            DataReference(const DataReference&);
            DataReference &operator= (const DataReference&);
        } DATA_REFERENCE;

        typedef vector<DATA_REFERENCE*> DATA_REF_VECTOR;

        struct CleanDataRefVector {
            CleanDataRefVector(DATA_REF_VECTOR &dataRef) : m_dataRef(dataRef) {};
            ~CleanDataRefVector();
            DATA_REF_VECTOR &m_dataRef;
        private:
            CleanDataRefVector(const CleanDataRefVector&);
            CleanDataRefVector &operator= (const CleanDataRefVector&);
        };

        virtual LIB_FOO GetFooPtr(Runable::Param &param);
        virtual void PushParameters(Runable::Param &param, vector<Value> &values);
        virtual void* GetRawPtr(Runable::Param &param);

        // marshal arguments
        RUNTIME_RETURN MarshalArgs(Runable::Param &param, vector<Value> &args, vector<ui32> &data, DATA_REF_VECTOR &dataRef);
        RUNTIME_RETURN MarshalByValueArg(Value &argValue, vector<ui32> &data, DATA_REF_VECTOR &dataRef);
        RUNTIME_RETURN MarshalByRefArg(Memory &memory, RunTimeError &error, Value &argValue, Type *pType, vector<ui32> &data, DATA_REF_VECTOR &dataRef);
        RUNTIME_RETURN MarshalByRefToRefArg(Memory &memory, RunTimeError &error, Value &argValue, Type *pType, vector<ui32> &data, DATA_REF_VECTOR &dataRef);
        // unmarshal arguments
        RUNTIME_RETURN UnMarshalArgs(Memory &memory, vector<Value> &args, DATA_REF_VECTOR &dataRef);
        RUNTIME_RETURN UnMarshalByRefToRefArg(Memory &memory, Value &argValue, Type *pArgType, DATA_REFERENCE &dataRef);

        void         *m_pFooPtr;
    private:
        static RUNTIME_RETURN CallWrapper(ui32 *data, ui32 dwordCount, LIB_FOO foo, FOO_RETURN_METHOD fooReturnMethod, ui32 &regEAX, ui32 &regEDX);

        FOO_RETURN_METHOD PrepareReturnValue(vector<ui32> &data, Value &retValue);
        bool UnMarshalReturnValue(Memory &memory, FOO_RETURN_METHOD fooReturnMethod, ui32 regEAX, ui32 regEDX, Value &retValue, Value &outValue);
        bool UnMarshalByValue(FOO_RETURN_METHOD fooReturnMethod, void *p, Type *pType, Value &retValue, Value &outValue);
        bool UnMarshalByRef(Memory &memory, void *p, Type *pType, Value &outValue);
        bool UnMarshalRefByRef(Memory &memory, void *p, Type *pType, Value &outValue);
    };

    /*************************************************************/

    class FunctionLibPtr : public FunctionPtr {
        PROTECT_CLASS(FunctionLibPtr)
    public:
        FunctionLibPtr(SymbolStore *pSymbolStore, Node *pParent, SYMBOL_DESC &symDesc) : FunctionPtr(pParent, symDesc, 0, TYPE_FUNCTION_LIB_PTR), m_pSymbolStore(pSymbolStore) {};
        virtual ~FunctionLibPtr();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
    //protected:
        virtual LIB_FOO GetFooPtr(Runable::Param &param);
        virtual Type* GetReturnType();
    private:
        SymbolStore *m_pSymbolStore;
    };

    /*************************************************************/

    class FunctionCallbackPtr : public Node {
        PROTECT_CLASS(FunctionCallbackPtr)
    public:
        typedef void* (FunctionCallbackPtr::*MY_FOO)();

        FunctionCallbackPtr(Node *pParent, SYMBOL_DESC &symDesc);
        virtual ~FunctionCallbackPtr();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual ui64 Callback();

        void** GetFooStubRef(Memory &memory);
        ui32 GetArgDWORDCount();
    protected:
        FunctionPtr::FOO_RETURN_METHOD PrepareReturnValue(Value &retValue);
        void *m_pExecRef;
        void *m_pExec;
    private:
        void CreateFooStub(Memory &memory);
        bool UnMarshalArgs(Runable::Param &param, ui32 *pArgs);
        bool UnMarshalByValueArg(Runable::Param &param, ui32 *&pArgs, ui32 argIdx, Type &argType);
        bool UnMarshalByRefArg(Runable::Param &param, ui32 *&pArgs, ui32 argIdx, Type &argType);
        bool UnMarshalByRefToRefArg(Runable::Param &param, ui32 *&pArgs, ui32 argIdx, Type &argType);

        ui64 MarshalReturnValue(ui32 *pRetMemory, FunctionPtr::FOO_RETURN_METHOD fooReturnMethod, Value &retValue);
        bool MarshalArgs(Runable::Param &param, ui32 *pArgs);
        ui32 MarshalValue(Value &value);
        ui32 MarshalRef(Value &value);
        ui32 MarshalRefToRef(MemoryBlockRef *pMemoryBlockRef);
        bool MarshalRefToRef(Memory &memory, MemoryBlockRef *pMemoryBlockRef, Type &valueType, ui32 *&pOutMemory);
    };

    /*************************************************************/

    class Parenthesis : public Node {
        PROTECT_CLASS(Parenthesis)
    public:
        Parenthesis(Node *pParent, SYMBOL_DESC &symDesc) : Node(pParent, symDesc, TYPE_PARENTHESIS) {};
        virtual ~Parenthesis() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
    private:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/

    class SubscriptNode : public Node {
        PROTECT_CLASS(SubscriptNode)
    public:
        SubscriptNode(Node *pParent, SYMBOL_DESC &symDesc) : Node(pParent, symDesc, TYPE_SUBSCRIPT), m_idx(INVALID_STACK_IDX) {};
        virtual ~SubscriptNode();
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual bool InjectPrePostExpressions(PrePostExpressions *pPrePostExpressions);

        inline ui32 GetSubscriptCount() { return m_subscriptExpressions.size(); };
        inline void SetVarIdx(ui32 idx) { m_idx = idx; };
        inline ui32 GetVarIdx() { return m_idx; };
        
        ArrayType *GetArrayType();
        void SetExpression(Node* pNode) { m_subscriptExpressions.push_back(pNode); };
        bool UpdateExpression(Node* pNode);
        void SetPostOperator(Node *pPostOperator);
    private:
        virtual void SerializeBody(Serializer *pSerializer);
        virtual void SerializeEnd(Serializer *pSerializer);

        RUNTIME_RETURN GetArraySubscriptOffset(Runable::Param &param, ui32 &offset);

        vector<Node*>   m_subscriptExpressions;
        ui32            m_idx;
    };

    /*************************************************************/

    class MemberAccess : public Operator {
        PROTECT_CLASS(MemberAccess)
    public:
        MemberAccess(Node *pParent, SYMBOL_DESC &symDesc) : Operator(pParent, symDesc, TYPE_MEMBER_ACCESS) {};
        virtual ~MemberAccess() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);

        //Variable* GetVariableNode();
    private:
        virtual void SerializeBody(Serializer *pSerializer);
    };

    /*************************************************************/

    class OperatorRefAssign : public Operator {
        PROTECT_CLASS(OperatorRefAssign)
    public:
        OperatorRefAssign(Node *pParent, SYMBOL_DESC &symDesc) : Operator(pParent, symDesc, TYPE_REF_ASSIGN) {};
        virtual ~OperatorRefAssign() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        //virtual bool IsApplicable(Type *pLeftType, Type *pRightType);
    };

    /*************************************************************/

    typedef ERROR_TYPE(Value::*MATH_OPERATION)(Value& v);

    class MathOperator : public Operator {
        PROTECT_CLASS(MathOperator)
    public:
        MathOperator(Node *pParent, MATH_OPERATION fnOperation, SYMBOL_DESC &symDesc, const NODE_TYPE type) : Operator(pParent, symDesc, type), m_fnOperation(fnOperation) {};
        virtual ~MathOperator() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual bool IsApplicable(Type *pLeftType, Type *pRightType);
    protected:
        MATH_OPERATION m_fnOperation;
    };

    /*************************************************************/

    class MathAssignOperator : public MathOperator {
        PROTECT_CLASS(MathAssignOperator)
    public:
        MathAssignOperator(Node *pParent, MATH_OPERATION fnOperation, SYMBOL_DESC &symDesc, const NODE_TYPE type) : MathOperator(pParent, fnOperation, symDesc, type) {};
        virtual ~MathAssignOperator() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual bool IsApplicable(Type *pLeftType, Type *pRightType);
    };

    /*************************************************************/

#define DECLARE_MATH_OPERATOR(name, fnOp, type) \
    class name : public MathOperator {\
        PROTECT_CLASS(name)\
    public:\
        name(Node *pParent, SYMBOL_DESC &symDesc) : MathOperator(pParent, fnOp, symDesc, type) {};\
        virtual ~name() {};\
        virtual bool IsApplicable(Type *pLeftType, Type *pRightType);\
    };

#define DECLARE_MATH_ASSIGN_OPERATOR(name, fnOp, type) \
    class name : public MathAssignOperator {\
        PROTECT_CLASS(name)\
    public:\
        name(Node *pParent, SYMBOL_DESC &symDesc) : MathAssignOperator(pParent, fnOp, symDesc, type) {};\
        virtual ~name() {};\
        virtual bool IsApplicable(Type *pLeftType, Type *pRightType);\
    };

#define DECLARE_OPERATOR(name, type) \
    class name : public Operator {\
        PROTECT_CLASS(name)\
    public:\
        name(Node *pParent, SYMBOL_DESC &symDesc) : Operator(pParent, symDesc, type) {};\
        virtual ~name() {};\
        virtual RUNTIME_RETURN Run(Runable::Param &param);\
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);\
    virtual bool IsApplicable(Type *pLeftType, Type *pRightType);\
    };

    class OperatorPrePost : public Operator {
        PROTECT_CLASS(OperatorPrePost)
    public:
        OperatorPrePost(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) : Operator(pParent, symDesc, type) {};
        virtual ~OperatorPrePost() {};
        virtual RUNTIME_RETURN Run(Runable::Param &param);
        virtual bool IsExpression(Type *&pReturnType, PrePostExpressions *&pPrePostExp, Error &error);
        virtual bool IsApplicable(Type *pLeftType, Type *pRightType);
        virtual bool Operation(Value &value, Value &delta) = 0;
    protected:
        virtual bool GetPrePostExpression(PrePostExpressions *&pPrePostExp, Error &error) = 0;
        
        bool GetPreExpression(PrePostExpressions *&pPreExp, Error &error);
        bool GetPostExpression(PrePostExpressions *&pPostExp, Error &error);
    };
    
    class OperatorInc : public OperatorPrePost {
        PROTECT_CLASS(OperatorInc)
    public:
        OperatorInc(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) : OperatorPrePost(pParent, symDesc, type) {};
            virtual ~OperatorInc() {};
            virtual bool Operation(Value &value, Value &delta);
    };
    
    class OperatorDec : public OperatorPrePost {
        PROTECT_CLASS(OperatorDec)
    public:
        OperatorDec(Node *pParent, SYMBOL_DESC &symDesc, const NODE_TYPE type) : OperatorPrePost(pParent, symDesc, type) {};
        virtual ~OperatorDec() {};
        virtual bool Operation(Value &value, Value &delta);
    };

#define DECLARE_PRE_POST_OPERATOR(name, type, op) \
    class name : public Operator##op {\
        PROTECT_CLASS(name)\
    public:\
        name(Node *pParent, SYMBOL_DESC &symDesc) : Operator##op(pParent, symDesc, type) {};\
        virtual ~name() {};\
    protected:\
        virtual bool GetPrePostExpression(PrePostExpressions *&pPrePostExp, Error &error);\
    };

    /*************************************************************/
    DECLARE_MATH_OPERATOR(OperatorAdd, &Value::Add, TYPE_ADD)                     // +    
    DECLARE_MATH_ASSIGN_OPERATOR(OperatorAddAssign, &Value::Add, TYPE_ADD_ASSIGN) // += 
    DECLARE_PRE_POST_OPERATOR(OperatorPreInc, TYPE_PRE_INC_OP, Inc)               // ++N
    DECLARE_PRE_POST_OPERATOR(OperatorPostInc, TYPE_POST_INC_OP, Inc)             // N++

    DECLARE_MATH_OPERATOR(OperatorSub, &Value::Sub, TYPE_SUB)                     // - 
    DECLARE_MATH_ASSIGN_OPERATOR(OperatorSubAssign, &Value::Sub, TYPE_SUB_ASSIGN) // -=    
    DECLARE_PRE_POST_OPERATOR(OperatorPreDec, TYPE_PRE_DEC_OP, Dec)               // --N
    DECLARE_PRE_POST_OPERATOR(OperatorPostDec, TYPE_POST_DEC_OP, Dec)             // N--

    DECLARE_MATH_OPERATOR(OperatorAnd, &Value::And, TYPE_AND)                               // && 
    DECLARE_MATH_ASSIGN_OPERATOR(OperatorBitAndAssign, &Value::BitAnd, TYPE_BIT_AND_ASSIGN) // &=   
    DECLARE_MATH_OPERATOR(OperatorBitAnd, &Value::BitAnd, TYPE_BIT_AND)                     // &    

    DECLARE_MATH_OPERATOR(OperatorOr, &Value::Or, TYPE_OR)                               // ||    
    DECLARE_MATH_ASSIGN_OPERATOR(OperatorBitOrAssign, &Value::BitOr, TYPE_BIT_OR_ASSIGN) // |=    
    DECLARE_MATH_OPERATOR(OperatorBitOr, &Value::BitOr, TYPE_BIT_OR)                     // |    

    DECLARE_MATH_OPERATOR(OperatorLess, &Value::Less, TYPE_LESS)                  // <  
    DECLARE_MATH_OPERATOR(OperatorLessEq, &Value::LessEq, TYPE_LESS_EQ)           // <=  
    DECLARE_MATH_OPERATOR(OperatorShiftLeft, &Value::ShiftLeft, TYPE_SHIFT_LEFT) // <<  

    DECLARE_MATH_OPERATOR(OperatorGreater, &Value::Greater, TYPE_GREATER)           // > 
    DECLARE_MATH_OPERATOR(OperatorGreaterEq, &Value::GreaterEq, TYPE_GREATER_EQ)    // >=
    DECLARE_MATH_OPERATOR(OperatorShiftRight, &Value::ShiftRight, TYPE_SHIFT_RIGHT) // >>

    DECLARE_MATH_OPERATOR(OperatorMul, &Value::Mul, TYPE_MUL)                     // *    
    DECLARE_MATH_ASSIGN_OPERATOR(OperatorMulAssign, &Value::Mul, TYPE_MUL_ASSIGN) // *=    

    DECLARE_MATH_OPERATOR(OperatorDiv, &Value::Div, TYPE_DIV)                     // /    
    DECLARE_MATH_ASSIGN_OPERATOR(OperatorDivAssign, &Value::Div, TYPE_DIV_ASSIGN) // /=    

    DECLARE_MATH_OPERATOR(OperatorMod, &Value::Mod, TYPE_MOD)                     // %    
    DECLARE_MATH_ASSIGN_OPERATOR(OperatorModAssign, &Value::Mod, TYPE_MOD_ASSIGN) // %=    

    DECLARE_OPERATOR(OperatorBitNot, TYPE_BIT_NOT)                      // ~
    DECLARE_OPERATOR(OperatorNot, TYPE_NOT)                             // !
    DECLARE_MATH_OPERATOR(OperatorNotEqual, &Value::NotEq, TYPE_NOT_EQ) // !=

    DECLARE_MATH_OPERATOR(OperatorXor, &Value::Xor,TYPE_XOR)                      // ^    
    DECLARE_MATH_ASSIGN_OPERATOR(OperatorXorAssign, &Value::Xor, TYPE_XOR_ASSIGN) // ^=  

    DECLARE_OPERATOR(OperatorAssign, TYPE_ASSIGN)             // =
    DECLARE_MATH_OPERATOR(OperatorEqual, &Value::Eq, TYPE_EQ) // ==


    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
    /*************************************************************/
}
#ifdef _DEBUG
const char* line(script::Node* pNode);
const char* call();
#endif // _DEBUG
