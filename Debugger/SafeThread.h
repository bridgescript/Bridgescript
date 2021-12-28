// author		: Mikhail Botcharov
// email        : mbotcharov@corp.untd.com

#ifndef _SAFETHREAD_H_
#define _SAFETHREAD_H_
//#include ".\common.h"
#include <map>
#include "smart_ptr.h"
#include "Sync.h"

namespace mb_thread_lib
{
    typedef enum tagNOTIFY_EVENT_TYPE
    {
        BEGIN_THREAD_EVENT = 0,
        END_THREAD_EVENT,
        TERMINATE_THREAD_EVENT// when Stop() is invoked
    } NOTIFY_EVENT_TYPE;

    class  CWorkItemInterface
    {
    public:
        virtual ~CWorkItemInterface()
        {
        };
        virtual void ProcessWorkItem() = NULL;
        virtual void AbortWorkItem()= NULL;
    };
    //typedef smart_ptr< _Type, _Free > TParam;
    template < class _Type, class _Free = sp_free< _Type >, class _Param = smart_ptr< _Type, _Free > >
    class CThreadInterface
    {
    public:
        virtual ~CThreadInterface(){};//compiler does not generate default one grrrr

        virtual bool Start(_Param &Params) = NULL;
        virtual bool Stop(void) = NULL;//called in destructor, call it only when you want to reuse the object
        virtual bool Wait(void) = NULL;
        virtual bool WaitWithMessageLoop(HANDLE hEvent) = NULL;
        virtual UINT GetThreadId() = NULL;

        virtual void Thread(_Param Params) = NULL;//must implement this
        virtual void Notify(NOTIFY_EVENT_TYPE evt, _Param& param) = NULL;
    };

    template < class _Base, class _Type , class _Free = sp_free< _Type >, class _Param = smart_ptr< _Type, _Free> >
    class CSafeThread: public _Base
    {
    public:
    public:
        CSafeThread(void);
        virtual ~CSafeThread(void);

        virtual bool Start(_Param &spParams);
        virtual bool Stop(void);//called in destructor, call it only when you want to reuse the object
        virtual bool Wait(void);
        //virtual bool IsTerminating();
        inline virtual UINT GetThreadId()
        {
            return m_uThreadId;
        };
    private:
        //implement this function if you want to be notified 
        //for example you can call SetEvent(hYourApplicationEvent)
        //and inside your thread function you could have WaitForSingleObject(...)
        //or if you do not use any waitable objects then in your thread function
        //you can check if thread is terminating by calling IsTerminating
        virtual void Notify(NOTIFY_EVENT_TYPE evt, _Param& param);
        //virtual void Thread(void* pParams) = NULL;//must implement this
        void ThreadFunc();
        static unsigned __stdcall ThreadFunc( void* p );
    protected:
        bool WaitWithMessageLoop(HANDLE hEvent);
        UINT			m_uThreadId;//PostThreadMessage needs it, could be accessed by derived class
    private:
        HANDLE			m_hBeginThreadEvent;
        HANDLE			m_hThread;
        _Param			m_spThreadFuncParams;	
        volatile bool	m_bStopping;
    protected:
        CLocalCriticalSec	m_cs;
    };

    template < class _Base, class _Type , class _Free, class _Param >
    CSafeThread< _Base, _Type, _Free, _Param >::CSafeThread(void):
    m_uThreadId(0),
        m_hThread(NULL),
        m_bStopping(false)
    {
#ifdef _DEBUG
        printf("\nCSafeThread::CSafeThread()\n");
#endif
        m_hBeginThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
    template < class _Base, class _Type , class _Free, class _Param >
    CSafeThread< _Base, _Type, _Free, _Param >::~CSafeThread(void)
    {
        Stop();

        if(m_hBeginThreadEvent)
            ::CloseHandle(m_hBeginThreadEvent);

#ifdef _DEBUG
        printf("\nCSafeThread::~CSafeThread()\n\n");
#endif
    }

    template < class _Base, class _Type , class _Free, class _Param >
    bool CSafeThread< _Base, _Type, _Free, _Param >::Start(_Param &spParams)
    {
        if( (!m_hBeginThreadEvent) )
                                   return false;

        uintptr_t hThread;
        m_cs.Lock();
        if(m_hThread)
        {
            m_cs.Unlock();
            return false;
        }
        m_spThreadFuncParams = spParams;
        hThread = _beginthreadex(NULL, 0 , ThreadFunc, this, 0, &m_uThreadId);
        if( NULL == hThread )
        {
            m_cs.Unlock();
            return false;
        }
        m_hThread = (HANDLE)hThread;
        m_cs.Unlock();


        if(WaitWithMessageLoop( m_hBeginThreadEvent ))
            return true;

        m_cs.Lock();
        ::CloseHandle(m_hThread);
        m_hThread = NULL;
        m_uThreadId = 0;
        m_cs.Unlock();

        return false;
    }

    template < class _Base, class _Type , class _Free, class _Param >
    bool CSafeThread< _Base, _Type, _Free, _Param >::Stop(void)
    {
        if(m_uThreadId == ::GetCurrentThreadId())
            return false;//sould not call stop inside this thread, it will block
        m_cs.Lock();
        if(WAIT_TIMEOUT != WaitForSingleObject(m_hThread,0))
        {
            m_cs.Unlock();
            return true;
        }
        m_cs.Unlock();
        Notify(TERMINATE_THREAD_EVENT, m_spThreadFuncParams);
        WaitWithMessageLoop(m_hThread);
        return false;
    }

    template < class _Base, class _Type , class _Free, class _Param >
    bool CSafeThread< _Base, _Type, _Free, _Param >::Wait(void)
    {
        return WaitWithMessageLoop(m_hThread);
    }

    template < class _Base, class _Type , class _Free, class _Param >
    unsigned __stdcall CSafeThread< _Base, _Type, _Free, _Param >::ThreadFunc( void* p )
    {
        if(p)
        {
            CSafeThread *pThis = (CSafeThread*)p;
            pThis->ThreadFunc();
        }
        return 0;
    }

    template < class _Base, class _Type , class _Free, class _Param >
    void CSafeThread< _Base, _Type, _Free, _Param >::ThreadFunc()
    {
        Notify(BEGIN_THREAD_EVENT, m_spThreadFuncParams);

        ::SetEvent(m_hBeginThreadEvent);//allow to return from Start()

        try//must protect it, just in case
        {
            Thread(m_spThreadFuncParams);
        }
        catch(...)
        {
            int e;
            e = 0;
        }
        Notify(END_THREAD_EVENT, m_spThreadFuncParams);
    }
#pragma warning(push)
#pragma warning(disable:4127)
    template < class _Base, class _Type , class _Free, class _Param >
    bool CSafeThread< _Base, _Type, _Free, _Param >::WaitWithMessageLoop(HANDLE hEvent)
    {
        DWORD dwRet;
        MSG msg;

        while(1)
        {
            dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, INFINITE, QS_ALLINPUT);

            if(dwRet == WAIT_OBJECT_0)
                return true;    // The event was signaled

            if(dwRet != WAIT_OBJECT_0 + 1)
                return false;          // Something else happened

            // There is one or more window message available. Dispatch them
            while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
            {
                if(WM_QUIT == msg.message)
                    return false;//application terminates
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
                    return true; // Event is now signaled.
            }
        }
        return false;//never goes here
    }
#pragma warning(pop)
    template < class _Base, class _Type , class _Free, class _Param >
    void CSafeThread< _Base, _Type, _Free, _Param >::Notify(NOTIFY_EVENT_TYPE evt, _Param& param)
    {
#ifdef _DEBUG
        switch(evt)
        {
        case BEGIN_THREAD_EVENT:
            printf("\nBEGIN_THREAD_EVENT:\n");
            break;
        case END_THREAD_EVENT:
            printf("\nEND_THREAD_EVENT:\n");
            break;
        case TERMINATE_THREAD_EVENT:
            printf("\nTERMINATE_THREAD_EVENT:\n");
            break;
        default:
            break;
        }
#endif
        try
        {
            _Base::Notify(evt, param);
        }
        catch(...)
        {
        }

        if(END_THREAD_EVENT == evt)
        {
            m_cs.Lock();
            m_uThreadId = 0;
            ::CloseHandle(m_hThread);
            m_hThread = NULL;
            m_cs.Unlock();
        }
    }
}

#endif//_SAFETHREAD_H_