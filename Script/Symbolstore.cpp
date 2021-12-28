#include "stdafx.h"
#include "SymbolStore.h"

using namespace script;

//SymbolStore::SymbolScope::~SymbolScope() {
//    SYMBOL_SCOPES::iterator it;
//    for (it = m_children.begin(); it != m_children.end(); ++it) {
//        delete (*it);
//    }
//}
//
//SymbolStore::SymbolScope* SymbolStore::SymbolScope::AddScope() {
//    SymbolScope *pScope = new SymbolScope(this);
//    m_children.push_back(pScope);
//    return pScope;
//}
//
//void SymbolStore::SymbolScope::AddSymbol(SYMBOL_ID symbolId) {
//    pair<SYMBOL_REFS::iterator, bool> res = m_symbolRefs.insert(symbolId);
//    if (!res.second) {//duplicate
//
//    }
//}
//
///*************************************************/
//SymbolStore::SymbolStore() : m_rootSymbolScope(0), m_pCurrentScope (0) {
//    m_pCurrentScope = &m_rootSymbolScope;
//}
//
//SymbolStore::~SymbolStore() {
//}
//
//SYMBOL_ID SymbolStore::AddSymbol(string &symbolName) {
//    pair<SYMBOL_REFS::iterator, bool> res = m_symbolRefs.insert(pair<string, SYMBOL_ID>());
//
//    if (res.second) {
//        res.first->second = m_symbolVec.size();
//        m_symbolVec.push_back(res.first);
//    }
//    m_pCurrentScope->AddSymbol(res.first->second);
//    return res.first->second;
//}
//
//void SymbolStore::PushScope() {
//    m_pCurrentScope = m_pCurrentScope->AddScope();
//}
//
//void SymbolStore::PopScope() {
//    m_pCurrentScope = m_pCurrentScope->GetParent();
//}
//
//SymbolStore::SymbolScope* SymbolStore::GetCurrentScope() {
//    return m_pCurrentScope;
//}
