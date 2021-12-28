#pragma once
#include "Basic.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

namespace script {

    //typedef ui32 SYMBOL_ID;

    //class SymbolStore {
    //    typedef unordered_map<string, SYMBOL_ID> SYMBOL_REFS;
    //    typedef vector<SYMBOL_REFS::iterator>    SYMBOL_VEC;

    //    class SymbolScope {
    //        typedef unordered_set<SYMBOL_ID> SYMBOL_REFS;
    //        typedef vector<SymbolScope*>     SYMBOL_SCOPES;
    //    public:
    //        SymbolScope(SymbolScope *pParent) : m_pParent(pParent) {};
    //        virtual ~SymbolScope();
    //        inline SymbolScope *GetParent() { return m_pParent; };
    //        SymbolScope* AddScope();
    //        void AddSymbol(SYMBOL_ID symbolId);
    //    private:
    //        SymbolScope();
    //        SymbolScope(const SymbolScope&);
    //        SYMBOL_SCOPES m_children;
    //        SYMBOL_REFS   m_symbolRefs;
    //        SymbolScope  *m_pParent;
    //    };

    //public:
    //    SymbolStore();
    //    virtual ~SymbolStore();
    //    SYMBOL_ID AddSymbol(string &symbolName);
    //    SymbolScope* GetCurrentScope();//?

    //private:
    //    void PushScope();
    //    void PopScope();

    //    SYMBOL_REFS  m_symbolRefs;//SYMBOL_ID is index to SYMBOL_VEC
    //    SYMBOL_VEC   m_symbolVec;
    //    SymbolScope  m_rootSymbolScope;
    //    SymbolScope *m_pCurrentScope;
    //};
}

