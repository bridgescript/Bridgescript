// author		: Mikhail Botcharov
// email        : mbotcharov@corp.untd.com

#ifndef SMART_PTR_H
#define SMART_PTR_H

namespace mb_thread_lib
{
    template< class _Type >
    class sp_no_free
    {
    public:
        static void free(_Type *)
        {
        }
    };

    template< class _Type >
    class sp_free
    {
    public:
        static void free(_Type *p)
        {
            delete p;
        }
    };
    template< class _Type, class _Free = sp_free< _Type > >
    class smart_ptr 
    {
        class _TypeWrap
        {
        public:
            _TypeWrap(_Type* p):m_lRefCount(1)
            {
                m_pData = p;
            }
            ~_TypeWrap()
            {
                if(m_pData)
                    _Free::free( m_pData );
            }
            LONG AddRef()
            {
#ifdef _MT
                return ::InterlockedIncrement(&m_lRefCount);
#else
                return m_lRefCount++;
#endif
            }
            void Release()
            {
#ifdef _MT
                if(!::InterlockedDecrement(&m_lRefCount))
#else
                if(!(--m_lRefCount))
#endif
                    delete this;
            }
            inline _Type* GetPtr()
            {
                return m_pData;
            }
        private:
            volatile LONG	m_lRefCount;
            _Type*			m_pData;
        };
    public:
        explicit smart_ptr(_Type *p = NULL) 
        {
            m_pTypeWrap = new _TypeWrap(p);
        }
        smart_ptr(const smart_ptr<_Type, _Free>& ref):m_pTypeWrap(NULL) 
        {
            *this = ref;
        }
        ~smart_ptr()
        {
            if(m_pTypeWrap)
                m_pTypeWrap->Release(); 
        }
        smart_ptr<_Type, _Free>& operator=(const smart_ptr<_Type, _Free>& _Right)
        {
            if(GetPtr() != _Right.GetPtr())
            {
                if(m_pTypeWrap)
                {
                    m_pTypeWrap->Release();
                }
                m_pTypeWrap = _Right.m_pTypeWrap;
                if(m_pTypeWrap)
                    m_pTypeWrap->AddRef();
            }
            return (*this); 
        }
        bool operator ==(const smart_ptr<_Type, _Free>& _Right)const
        {
            return (GetPtr() == _Right.GetPtr());
        }
        inline operator bool()const
        {
            return (GetPtr() != NULL);
        }
        inline bool operator!() const 
        {
            return (GetPtr() == NULL);
        }
        inline _Type& operator*() const throw()
        {
            return (*GetPtr()); 
        }
        inline _Type *operator->() const
        {
            return (GetPtr()); 
        }
        inline _Type *GetPtr() const
        {
            return ((m_pTypeWrap)?(m_pTypeWrap->GetPtr()):(NULL)); 
        }
    private:
        friend class smart_ptr<_Type, _Free>;
        _TypeWrap *m_pTypeWrap;
    };

#ifndef SP_NULL
    #define SP_NULL(type)	(mb_thread_lib::smart_ptr<type>(NULL))
#endif
}
#endif//SMART_PTR_H