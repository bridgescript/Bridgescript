#pragma once
#include <Windows.h>
#include <eh.h>
#include <string>
#include "Basic.h"
//#include "Script.h"
//using namespace script;
using namespace std;
#ifdef NO_ERROR
#undef NO_ERROR
#endif // NO_ERROR
typedef enum Error_Type {
    NO_ERROR,
    NOT_IMPLEMENTED,
    INVALID_POINTER,
    OUT_OF_MEMORY,
    INTERNAL_ERROR,
    DUPLICATE_SYMBOL,
    DUPLICATE_DECLARATION,
    DUPLICATE_FUNCTION_DEFINITION,
    FUNCTION_DEFINITION_MUST_BE_GLOBAL,
    INCLUDE_ERROR,

    UNEXPECTED_END_OF_SCRIPT,
    UNEXPECTED_CONST_STRING,
    UNEXPECTED_CHARACTER,
    UNEXPECTED_SCOPE,
    UNEXPECTED_OPERATOR,
    UNEXPECTED_END_OF_SCOPE,
    UNEXPECTED_PARENTHESIS,
    UNEXPECTED_FUNCTION_DEFINITION,
    UNEXPECTED_TYPE_DECLARATION,
    UNEXPECTED_RETURN,
    UNEXPECTED_NUMBER_OF_ARGUMENTS,
    UNEXPECTED_NUMBER_OF_SUBSCRIPTS,
    UNEXPECTED_TOKEN,
    UNEXPECTED_ERROR,

    EXPECTING_SINGLE_QUOTATION_MARK,
    EXPECTING_DOUBLE_QUOTATION_MARK,
    EXPECTING_SYMBOL,
    EXPECTING_OPERATOR,
    EXPECTING_RIGHT_VALUE,
    EXPECTING_FUNCTION_NAME,
    EXPECTING_FUNCTION_TYPE,
    EXPECTING_PARENTHESIS,
    EXPECTING_SCOPE,
    EXPECTING_COMMA,
    EXPECTING_COLON,
    EXPECTING_CONDITION,
    EXPECTING_CONST,
    EXPECTING_NUMBER,
    EXPECTING_POSITIVE_NUMBER,
    EXPECTING_32BIT_NUMBER,
    EXPECTING_SCRIPT_NAME,
    EXPECTING_MODULE_NAME,
    EXPECTING_STRUCT_NAME,
    EXPECTING_TYPE_NAME,
    EXPECTING_FUNCTION_REF_TYPE,
    EXPECTING_VARIABLE,
    EXPECTING_VARIABLE_NAME,
    EXPECTING_ERROR_VARIABLE_NAME,
    EXPECTING_SINGLE_ARGUMENT,
    EXPECTING_SEMICOLON,
    EXPECTING_EXPRESSION,
    EXPECTING_SUBSCRIPT_VALUE,

    UNKNOWN_KEYWORD,
    UNKNOWN_NODE_TYPE,
    UNKNOWN_TYPE_NAME,
    UNRELATED_TYPES,

    SYNTAX_ERROR,

    INVALID_ESCAPE_CHARACTER,
    INVALID_NODE_TYPE,
    INVALID_IDENTIFIER,
    INVALID_FUNCTION_NAME,
    INVALID_SYMBOL_NAME,
    INVALID_SYMBOL_ID,
    INVALID_TOKEN,
    INVALID_NUMBER_FORMAT,
    INVALID_TYPE,
    INVALID_OPERATOR,
    INVALID_ARGUMENT_NAME,
    INVALID_EXPRESSION,
    INVALID_HEX_NUMBER,
    INVALID_BINARY_NUMBER,
    INVALID_INCLUDE_PATH,
    DUPLICATE_ARGUMENT_NAME,
    DUPLICATE_CASE,
    DUPLICATE_DEFAULT,
    ILLEGAL_CASE,
    ILLEGAL_BREAK,
    ILLEGAL_CONTINUE,
    ILLEGAL_DEFAULT,
    ILLEGAL_OPERATION_ON_TYPE,
    AGGREGATE_MATH_OPERATOR_NOT_SUPPORTED,
    AGGREGATE_AS_VALUE_NOT_SUPPORTED,
    ASSIGNMENT_NEEDS_LEFT_VALUE,
    ASSIGNMENT_NEEDS_RIGHT_VALUE,
    OPERATOR_NEEDS_LEFT_VALUE,
    OPERATOR_NEEDS_RIGHT_VALUE,
    LEFT_VALUE_CANNOT_BE_CONST,
    REFERENCE_TYPES_MISMATCHED,
    TYPES_MISMATCHED,
    PARENTHESIS_IS_NOT_CLOSED,
    EMPTY_PARENTHESIS,
    OPERATOR_NOT_ALLOWED_ON_TYPE,
    OPERATOR_NOT_ALLOWED_ON_FUNCTION_CALL,
    OPERATOR_NOT_ALLOWED_ON_CONST,
    OPERATOR_NOT_ALLOWED_ON_STRUCT,
    OPERATOR_NOT_ALLOWED_ON_ARRAY,
    OPERATOR_NEEDS_VARIABLE,
    LIB_FUNCTION_CALL_MISSING,
    REFERENCE_ASSIGNMENT_NEEDS_LEFT_VALUE,
    REFERENCE_ASSIGNMENT_NEEDS_LEFT_VALUE_AS_VARAIBLE,
    REFERENCE_ASSIGNMENT_NEEDS_RIGHT_VALUE,
    REFERENCE_ASSIGNMENT_NEEDS_LEFT_VALUE_AS_REFERENCE,
    CANNOT_PASS_MEMBER_BY_REFERENCE,
    CANNOT_ASSIGN_REFERENCE_TO_MEMBER,
    CANNOT_ASSIGN_TO_NOT_REFERENCE_RETURN_TYPE,
    CANNOT_ASSIGN_REFERENCE_TO_ARRAY_OF_NOT_REFERENCES,
    CANNOT_PASS_MEMBER_AS_REFERENCE_TO_REFERENCE,
    CANNOT_RETURN_MEMBER_AS_REFERENCE_TO_REFERENCE,
    //CANNOT_PASS_MEMBER_AS_REFERENCE_TO_REFERENCE,
    CANNOT_RETURN_ARRAY_ITEM_AS_REFERENCE_TO_REFERENCE,
    CANNOT_PASS_ARRAY_ITEM_AS_REFERENCE_TO_REFERENCE,
    CANNOT_RETURN_REFERENCE_TO_REFERENCE,
    CANNOT_RETURN_REFERENCE_TO_REFERENCE_OF_FUNCTION_RETURN,
    CANNOT_PASS_REFERENCE_TO_REFERENCE_OF_FUNCTION_RETURN,
    CANNOT_RETURN_REFERENCE_TO_REFERENCE_OF_EXPRESSION_RESULT,
    CANNOT_PASS_REFERENCE_TO_REFERENCE_OF_EXPRESSION_RESULT,
    CANNOT_RETURN_REFERENCE_TO_CONST,
    CANNOT_TYPECAST_REFERENCE_TO_REFERENCE,
    CANNOT_TYPECAST_TO_DYNAMIC_ARRAY,
    CANNOT_TYPECAST_TO_STRING,
    //CANNOT_PASS_ARRAY_ITEM_AS_REFERENCE_TO_REFERENCE,
    CANNOT_PASS_CONST_BY_REFERENCE,
    CANNOT_LOCK_CONST,
    ARRAY_TYPE_ALREADY_DECLARED,
    EXPECTING_ARRAY_SIZE,
    EXPECTING_NUMERIC_ARRAY_SIZE,
    EXPECTING_NONZERO_ARRAY_SIZE,
    EXPECTING_NONZERO_ARRAY_DIMENSION,
    EXPECTING_STATIC_OR_ONE_DIMENSION_ARRAY,
    ARRAYS_DIMENSIONS_MISMATCH,
    ARRAYS_SUBSCRIPT_MISMATCH,
    ARRAY_CANNOT_BE_LARGER_32BIT_INT,
    NO_DIMENSION_ARRAY_MUST_HAVE_ONE_DIMESION_SUBSCRIPT,
    DYNAMIC_ARRAY_MUST_HAVE_ONE_DIMESION,
    DYNAMIC_ARRAY_MUST_BE_DECLARED_IN_SCOPE,
    DYNAMIC_ARRAY_DIMENSION_CANNOT_HAVE_BOOLEAN_EXPRESSION,
    UNRELATED_ARRAY_TYPES_TYPES,
    STRING_IS_NOT_SAFE_EXTERNAL_TYPE,

    CANNOT_OPEN_FILE,
    CORRUPTED_FILE,
    SYMBOL_NOT_FOUND,
    //run time errors?
    NULL_SCRIPT,
    UNKNOWN_VALUE_TYPE,
    UNKNOWN_MEMBER_NAME,
    UNDEFINED_TYPE,
    UNDEFINED_SYMBOL,
    UNDEFINED_LIB_SYMBOL,
    UNDEFINED_FUNCTION,
    INCOMPATIBLE_TYPE,
    DIVISION_BY_ZERO,
    EVAL_TRUE,
    EVAL_FALSE,
} ERROR_TYPE;

namespace script {
    class StdOutInterface;

    class Error
    {
    public:
        Error();
        virtual ~Error();
        
        //static Error& Instance();
        
        void SetError(ERROR_TYPE error, const char * const errorStr, const char *const file, const char * const function, ui32 const line);
        void SetErrorEx(ERROR_TYPE error, const char * const errorStr, const char *const file, const char * const function, ui32 const line, SYMBOL_DESC &symDesc);
        void SetErrorLocation(string &file, ui32 line, ui32 pos);
        inline ERROR_TYPE GetError() { return m_error; }
        inline bool operator==(ERROR_TYPE type) { return (m_error == type); };
        inline void operator= (ERROR_TYPE type) { m_error = type; };
        /*virtual */void Set(int pos, int line, char* szFile, ERROR_TYPE error, const char* const szFormat, ...);
        ///*virtual */string& Get() { return m_errorStrEx; };
        void PusTrace(string str) { m_trace.push_back(str); };
        void Print(StdOutInterface *pStdOutInterface);
        bool IsLocationSet() { return m_file.size() != 0; };
    private:
        Error(const Error&);
        vector<string> m_trace;
        //string m_errorStrEx;

        SYMBOL_DESC   m_symDesc;
        string        m_errorStr;
        const char    *m_errorFile,
                      *m_errorFunction;
        ui32           m_errorLine;
        ERROR_TYPE     m_error;
        string         m_file;
        ui32           m_line,
                       m_position;
    };

    class SymbolStore;
    class StackMemory;

    class RunTimeError {
        RunTimeError();
    public:
        RunTimeError(SymbolStore *pSymStore, StdOutInterface *pStdOutInterface) : m_pSymStore(pSymStore), m_pStdOutInterface(pStdOutInterface) {};
        ~RunTimeError() {};
        
        RUNTIME_RETURN SetError(string name, SYMBOL_DESC &symDesc, ui32 seNumber, StackMemory *pStackMemory);
        //static RunTimeError *GetError() { return s_error; };
        string GetName() { return m_name; };
        SYMBOL_DESC& GetSymDesc() { return m_symDesc; };
        ui32 GetSeNumber() { return m_seNumber; }
        SymbolStore* GetSymbolStore() { return m_pSymStore; }
        string GetStackString() { return m_stackString; }
    private:
        RunTimeError(const RunTimeError&);
        RunTimeError& operator==(RunTimeError&);

        SymbolStore     *m_pSymStore;
        StdOutInterface *m_pStdOutInterface;
        string           m_name;
        string           m_stackString;
        SYMBOL_DESC      m_symDesc;
        ui32             m_seNumber;

        //static RunTimeError *s_error;
    };


    class SE_Exception {
        ui32 nSE;
    public:
        SE_Exception() : nSE{ 0 } {}
        SE_Exception(ui32 n) : nSE{ n } {}
        ui32 getSeNumber() { return nSE; }

        static void trans_func(unsigned int u, EXCEPTION_POINTERS* pExPtrs) {
            throw SE_Exception(u);
        }

        static _se_translator_function Init() {
            return _set_se_translator(trans_func);
        }
        
    };
}
