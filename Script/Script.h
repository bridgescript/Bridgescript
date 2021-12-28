#pragma once

#include "compiler.h"

#define EXT_TEXT_SCRIPT_STRING  ".bridge"
#define EXT_BIN_SCRIPT_STRING  ".bin"

namespace script {

    typedef enum Print_Type {
        BUILD_OUT = 0,
        DEBUG_OUT
    } PRINT_TYPE;

    class StdOutInterface {
    public:
        virtual void Print(PRINT_TYPE type, string &text) = 0;
        virtual void Clear() = 0;
    };

    class PrintF : public StdOutInterface {
    public:
        virtual void Print(PRINT_TYPE type, string &text) { cout << text << endl; }
        virtual void Clear() {}
    };

    typedef enum Extention_Type {
        EXT_INVALID,
        EXT_TEXT_SCRIPT,
        EXT_BIN_SCRIPT
    } EXTENTION_TYPE;

    typedef enum Option {
        OPT_NONE = 0,
        OPT_COMPILE,
        OPT_DECOMPILE
    } OPTION;

    class OptionRunner {
    public:
        OptionRunner() {};
        virtual ~OptionRunner() {};
        void Run();
        virtual Reader* GetReader() = 0;
        virtual bool Load() = 0;
        virtual bool Exec() = 0;
#ifdef SCRIPT_DEBUGGER
        virtual void BeforeExec(DebuggerEvents *pDebuggerEvents) {};
#else // SCRIPT_DEBUGGER
        virtual void BeforeExec() {};
#endif // SCRIPT_DEBUGGER
        virtual void AfterExec() {};
    private:
        OptionRunner(const OptionRunner &);
        OptionRunner& operator= (const OptionRunner&);
    };

    /*************************************************************/

    class Help : public OptionRunner {
    public:
        Help() : OptionRunner() {};
        virtual ~Help() {};
        virtual Reader* GetReader() { return 0; };
        virtual bool Load() { return true; };
        virtual bool Exec();
    private:
        Help(const Help &);
        Help& operator= (const Help&);
    };

    /*************************************************************/

    class ScriptRunner : public OptionRunner {
    public:
        ScriptRunner(string path, StdOutInterface *pStdOutInterface) : OptionRunner(), m_pStdOutInterface(pStdOutInterface), m_pReader(0), m_interpreter(m_error, pStdOutInterface), m_path(path), m_extType(GetExtentionType()) {};
        virtual ~ScriptRunner() { delete m_pReader; };
        virtual Reader* GetReader();
        virtual bool Load();
        virtual bool Exec();
//#ifdef SCRIPT_DEBUGGER
//        virtual void BeforeExec(DebuggerEvents *pDebuggerNotify) { m_interpreter.PreRun(pDebuggerNotify); };
//#else // SCRIPT_DEBUGGER
//        virtual void BeforeExec() { m_interpreter.PreRun(); };
//#endif // SCRIPT_DEBUGGER
        //virtual void AfterExec() { m_interpreter.PostRun(); };
        inline SymbolStore& GetSymbolStore() { return m_interpreter.GetSymbolStore(); };
        inline Interpreter& GetInterpreter() { return m_interpreter; };
    private:
        ScriptRunner();
        ScriptRunner(const ScriptRunner &);
        ScriptRunner& operator= (const ScriptRunner&);
        EXTENTION_TYPE GetExtentionType();
        
    protected:
        StdOutInterface *m_pStdOutInterface;
        Error            m_error;
        Reader          *m_pReader;
        Interpreter      m_interpreter;
        string           m_path;
        EXTENTION_TYPE   m_extType;
    };

    /*************************************************************/

    class CompilerRunner : public ScriptRunner {
    public:
        CompilerRunner(string path, StdOutInterface *pStdOutInterface) : ScriptRunner(path, pStdOutInterface) {};
        virtual ~CompilerRunner() {};
        virtual bool Load();
        virtual bool Exec();
    private:
        CompilerRunner();
        CompilerRunner(const CompilerRunner &);
        CompilerRunner& operator= (const CompilerRunner&);
    };

    /*************************************************************/

    class DecompilerRunner : public ScriptRunner {
    public:
        DecompilerRunner(string path, StdOutInterface *pStdOutInterface) : ScriptRunner(path, pStdOutInterface) {};
        virtual ~DecompilerRunner() {};
        virtual bool Load();
        virtual bool Exec();
    private:
        DecompilerRunner();
        DecompilerRunner(const DecompilerRunner &);
        DecompilerRunner& operator= (const DecompilerRunner&);
    };

    /*************************************************************/

    class OptionSelector {
    public:
        static OptionRunner* GetRunner(int count, char** ppArgs);
        static OPTION TranslateParameters(string arg0, string arg1, string &path);
    private:
        OptionSelector();
        OptionSelector(const OptionSelector &);
        OptionSelector& operator= (const OptionSelector&);
    };

}