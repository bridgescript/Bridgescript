// author		: Mikhail Botcharov
// email        : mbotcharov@corp.untd.com

#ifndef SYNC_H
#define SYNC_H

namespace mb_thread_lib
{
    typedef enum CRITICAL_SEC_TYPE
    {
        CS_LOCAL = 0,
        CS_GLOBAL
    } CRITICAL_SEC_TYPE;

    class CCriticalSecDisposer
    {
        CCriticalSecDisposer(const CCriticalSecDisposer& ){}
        CCriticalSecDisposer(){}
        ~CCriticalSecDisposer(){}
    public:

        static void Dispose(_bstr_t &bstrName);
    };
    class CCriticalSecInterface
    {
        _bstr_t	m_bstrName;
    public:
        CCriticalSecInterface(BSTR wsName = NULL):m_bstrName(wsName){}
        virtual ~CCriticalSecInterface()
        {
        };
        virtual void Lock(void) = NULL;
        virtual void Unlock(void) = NULL;
        //	virtual bool TryLock() = NULL;
        _bstr_t GetName(){return m_bstrName;}
    };

    class CLocalCriticalSec: public CCriticalSecInterface
    {
        CLocalCriticalSec(const CLocalCriticalSec& ){/*no cpoy ctor!!*/}
    public:
        CLocalCriticalSec(BSTR wsName = NULL):CCriticalSecInterface(wsName){::InitializeCriticalSection(&m_cs);}
        virtual ~CLocalCriticalSec(){::DeleteCriticalSection(&m_cs);}
        virtual void Lock(void){::EnterCriticalSection(&m_cs);}
        virtual void Unlock(void){::LeaveCriticalSection(&m_cs);}
        //	virtual bool TryLock(){return ::TryEnterCriticalSection(&m_cs);}
    private:
        CRITICAL_SECTION	m_cs;
    };

    class CGlobalCriticalSec: public CCriticalSecInterface
    {
        CGlobalCriticalSec(const CGlobalCriticalSec& ){}
    public:
        CGlobalCriticalSec(BSTR wsName = NULL):CCriticalSecInterface(wsName),m_hMutex(NULL)
        {
            m_hMutex = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, wsName);
            if(!m_hMutex)
            {
                m_hMutex = ::CreateMutexW(NULL, FALSE, wsName);
            }
        }
        ~CGlobalCriticalSec()
        {
            CloseHandle(m_hMutex);
        }
        virtual void Lock(void)
        {
            ::WaitForSingleObject(m_hMutex, INFINITE);
        }
        virtual void Unlock(void)
        {
            ::ReleaseMutex(m_hMutex);
        }

    protected:
        HANDLE	m_hMutex;
    };

    class CGlobalCriticalSecWnd: public CGlobalCriticalSec

    {
    public:
        CGlobalCriticalSecWnd(BSTR wsName = NULL):CGlobalCriticalSec(wsName){}
        virtual void Lock(void)
        {
            WaitWithMessageLoop();
        }
    private:
        inline bool WaitWithMessageLoop();
    };

    class CAutoCriticalSec
    {
    public:
        CAutoCriticalSec(smart_ptr<CCriticalSecInterface> &spCS):m_spCS(spCS)
        {
            m_spCS->Lock();
        }
        ~CAutoCriticalSec()
        {
            m_spCS->Unlock();
        }

    private:
        smart_ptr<CCriticalSecInterface>	m_spCS;
    };

    class CCriticalSecFactory
    {
        CCriticalSecFactory(){}
        CCriticalSecFactory(const CCriticalSecFactory& ){}
        ~CCriticalSecFactory(){}
    public:
        static smart_ptr<CCriticalSecInterface> Query(_bstr_t &bstrName, CRITICAL_SEC_TYPE Type);
    private:
        friend class CCriticalSecDisposer;
        static CLocalCriticalSec								m_cs;
        static std::map<_bstr_t, smart_ptr<CCriticalSecInterface> >  m_mapCriticalSec;
    };


}
#endif//SYNC_H