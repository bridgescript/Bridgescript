#pragma once
#include <unordered_map>
#include <unordered_set>

using namespace std;

#ifdef SCRIPT_DEBUGGER

namespace script {
    typedef string SCRIPT_FILE_NAME;
    typedef string SCRIPT_PATH_NAME;
    typedef ui32   SCRIPT_FILE_LINE;

    typedef enum Breakpoint_Marker {
        BREAKPOINT_NONE = 0,
        BREAKPOINT_INVALID,
        BREAKPOINT_VALID
    } BREAKPOINT_MARKER;

    typedef struct BreakpointInfo {
        SCRIPT_FILE_LINE    line;
        BREAKPOINT_MARKER   marker;
        BreakpointInfo() : line(-1), marker(BREAKPOINT_NONE) {}
        BreakpointInfo(SCRIPT_FILE_LINE l, BREAKPOINT_MARKER m) : line(l), marker(m) {}
        BreakpointInfo(const BreakpointInfo& that) : line(that.line), marker(that.marker) {}
        bool operator==(const BreakpointInfo& that) const {
            return (line == that.line) && (marker == that.marker);
        }
        size_t operator()(const BreakpointInfo& k) const {
            return hash<unsigned int>()(k.line);
        }
    } BREAKPOINT_INFO;
    //struct KeyHasher {
    //    std::size_t operator()(const Key& k) const
    //    {
    //        using std::size_t;
    //        using std::hash;
    //        using std::string;
    //        return ((hash<string>()(k.first)
    //            ^ (hash<string>()(k.second) << 1)) >> 1)
    //            ^ (hash<int>()(k.third) << 1);
    //    }
    //};
    typedef unordered_map<SCRIPT_FILE_NAME, unordered_set<BREAKPOINT_INFO, BREAKPOINT_INFO> >   BREAKPOINTS;

    //typedef unordered_map<SCRIPT_PATH_NAME, unordered_map<SCRIPT_FILE_NAME, unordered_map<SCRIPT_FILE_LINE, BREAKPOINT_MARKER> > > BREAKPOINT_STORE;

    class Scope;

    typedef struct BreakHitData {
        ui32      fileId,
                  line;
        ThreadId  threadId;
        Scope    *pScope;
        BreakHitData() { memset(this, 0, sizeof(*this)); }
        BreakHitData(ui32 _fileId, ui32 _line, ThreadId _threadId, Scope *_pScope) : 
            fileId(_fileId), line(_line), threadId(_threadId), pScope(_pScope) {}
    } BREAK_HIT_DATA;

    typedef enum Debugger_States {
        DEBUGGER_STOP = 0,
        DEBUGGER_RUN,
        DEBUGGER_PAUSE,
        DEBUGGER_STEP_IN,
        DEBUGGER_STEP,
        DEBUGGER_STEP_OUT,
        DEBUGGER_MAX
    } DEBUGGER_STATES;

    class DebuggerEvents {
    public:
        virtual ~DebuggerEvents() {};

        virtual void OnStop(ThreadId threadId) = 0;
        virtual void OnRun(ThreadId threadId) = 0;
        virtual void OnPause(ThreadId threadId) = 0;
        virtual void OnStepIn(ThreadId threadId) = 0;
        virtual void OnStepOver(ThreadId threadId) = 0;
        virtual void OnStepOut(ThreadId threadId) = 0;
        virtual void OnSetBreakPoint(ui32 fileId, ui32 line, bool set) = 0;
        virtual void OnBreakHit(ui32 fileId, ui32 line, ThreadId threadId, Scope* pScope) = 0;
        virtual void OnBeginThread(ThreadId threadId) = 0;
        virtual void OnEndThread(ThreadId threadId) = 0;
    };

    typedef union PauseType {
        typedef enum Type {
            PAUSE_ALL = 0,
            PAUSE_ON_END_OF_THREAD = 0xffffffff,
        } TYPE;
        TYPE        type;
        ThreadId    threadId;
        PauseType(TYPE _type) { type = _type; }
        PauseType(ThreadId _threadId) { threadId = _threadId; }
    } PAUSE_TYPE;

    class DebuggerAction {
    public:
        DebuggerAction() : m_pNotify(0) {};
        virtual ~DebuggerAction() {};

        virtual void Stop() = 0;
        virtual void Run() = 0;
        virtual void Pause(PAUSE_TYPE type) = 0;
        virtual void StepIn() = 0;
        virtual void StepOver() = 0;
        virtual void StepOut() = 0;
        virtual bool SetBreakpoint(ui32 fileId, ui32 line, bool set) = 0;
        //virtual void OnError() = 0;
        inline void SetDebuggerNotify(DebuggerEvents *pNotify) { m_pNotify = pNotify; };
        inline DebuggerEvents* GetDebuggerNotify() { return m_pNotify; };
    protected:
        DebuggerEvents *m_pNotify;
    };

    class Debugger;

    class DebuggerState {
    public:
        //DebuggerState(DebuggerEvents *pDebuggerNotify);
        DebuggerState(Debugger *pDebugger);
        virtual ~DebuggerState();

        virtual DebuggerState* Run();
        virtual DebuggerState* Stop();
        virtual DebuggerState* Pause(PAUSE_TYPE type);
        virtual DebuggerState* StepIn();
        virtual DebuggerState* StepOver();
        virtual DebuggerState* StepOut();

        typedef void (DebuggerState::*SET_NOTIFY_FOO)(DebuggerEvents *);
        typedef void (DebuggerState::*SET_ACTION_FOO)(DebuggerAction *);

        //inline void SetDebuggerNotify(DebuggerEvents *pNotify) { m_pNotify = pNotify; };
        inline void SetAction(DebuggerAction *pAction) { m_pAction = pAction; };
        bool SetBreakpoint(ui32 fileId, ui32 line, bool set);
        /*void NotifyStop() {
            if (m_pNotify) {
                m_pNotify->OnStop(fileId);
            }
        }*/
        //void ExecAction();
    protected:
        DebuggerState();// : m_pParent(0), m_states{ 0 }, m_pDebuggerNotify(0) {};
        DebuggerState(const DebuggerState&);
        DebuggerState& operator=(const DebuggerState&);

        //DebuggerState& GetState(DEBUGGER_STATES state);
        //void PutAction(DebuggerAction *pAction);
        
        Debugger          *m_pDebugger;
        //DebuggerState  *m_pParent;
        //DebuggerEvents    *m_pNotify;
        DebuggerAction    *m_pAction;
        //DebuggerState  *m_states[DEBUGGER_MAX];
        //DebuggerEvents *m_pDebuggerNotify;
    };

    /*
        states   command   action  result state

          Stop    Stop      0        Stop
          Stop    Run       Run      Run
          Stop    Pause     0        Stop
          Stop    StepIn    StepIn   StepIn
          Stop    Step      Step     Step
          Stop    StepOut   0        Stop

          Run     Stop      Stop     Stop
          Run     Run       0        Run
          Run     Pause     Pause    Pause
          Run     StepIn    0        Run
          Run     Step      0        Run
          Run     StepOut   0        Run

          Pause   Stop      Stop     Stop
          Pause   Run       Run      Run
          Pause   Pause     0        Pause
          Pause   StepIn    StepIn   StepIn
          Pause   Step      Step     Step
          Pause   StepOut   StepOut  StepOut

          StepIn, Step and StepOut are special states

          StepIn   Stop      0       StepIn
          StepIn   Run       0       StepIn
          StepIn   Pause     Pause   Pause
          StepIn   StepIn    0       StepIn
          StepIn   Step      0       StepIn
          StepIn   StepOut   0       StepIn

          Step     Stop      0       Step
          Step     Run       0       Step
          Step     Pause     Pause   Pause
          Step     StepIn    0       Step
          Step     Step      0       Step
          Step     StepOut   0       Step

          StepOut  Stop      0       StepOut
          StepOut  Run       0       StepOut
          StepOut  Pause     Pause   Pause
          StepOut  StepIn    0       StepOut
          StepOut  Step      0       StepOut
          StepOut  StepOut   0       StepOut

    */
    class SymbolStore;
    class InterpreterIf;
    class Memory;

    class  Debugger {
    public:
        Debugger(/*DebuggerEvents *pDebuggerNotify*/);
        ~Debugger();

        void Run();
        void Stop();
        void Pause(PAUSE_TYPE type);
        void StepIn();
        void StepOver();
        void StepOut();
        
        void PropagateBreakpoints(BREAKPOINTS &breakpoints);
        bool SetBreakpoint(ui32 fileId, ui32 line, bool set);
        void SetAction(DebuggerAction *pAction);
        DebuggerState* GetState(DEBUGGER_STATES state);
        
        inline void SetInterpreter(InterpreterIf *pInterpreter) { m_pInterpreterIf = pInterpreter; }
        SymbolStore* GetSymbolStore();
        Memory* GetMemory();
    private:
        Debugger(const Debugger&);
        Debugger& operator=(const Debugger&);

        template<class FOO, class T> void Iterate(FOO foo, T t) {
            for (ui32 i = 0; i < DEBUGGER_MAX; ++i) {
                if (m_states[i]) {
                    (m_states[i]->*foo)(t);
                }
            }
        }
        
        CRITICAL_SECTION    m_cs;
        DebuggerState      *m_states[DEBUGGER_MAX];
        DebuggerState      *m_pCurrentState;
        InterpreterIf      *m_pInterpreterIf;
    };

     
    class DebuggerPause;

    class DebuggerStop : public DebuggerState {
    public:
        DebuggerStop(Debugger *pDebugger);

        virtual DebuggerState* Stop();
        virtual DebuggerState* Pause(PAUSE_TYPE type);
    private:
        DebuggerStop();
    };

    class DebuggerRun : public DebuggerState {
    public:
        DebuggerRun(Debugger *pDebugger);

        virtual DebuggerState* Run();
        virtual DebuggerState* StepIn();
        virtual DebuggerState* StepOver();
        virtual DebuggerState* StepOut();
    };

    class DebuggerPause : public DebuggerStop {
    public:
        DebuggerPause(Debugger *pDebugger);

        virtual DebuggerState* Stop();
        virtual DebuggerState* Pause(PAUSE_TYPE type);
    };

    class DebuggerStepIn : public DebuggerState {
    public:
        DebuggerStepIn(Debugger *pDebugger);

        virtual DebuggerState* Run();
        virtual DebuggerState* StepIn();
        virtual DebuggerState* StepOver();
        virtual DebuggerState* SteOut();
    };

    class DebuggerStep : public DebuggerStepIn {
    public:
        DebuggerStep(Debugger *pDebugger);
    };

    class DebuggerStepOut : public DebuggerStepIn {
    public:
        DebuggerStepOut(Debugger *pDebugger);

        virtual DebuggerState* Run();
    };
};

#endif // SCRIPT_DEBUGGER