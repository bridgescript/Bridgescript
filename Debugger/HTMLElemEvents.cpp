#include "StdAfx.h"
#include "HTMLElemEvents.h"
//HTML element sink

CHTMLElemEvents::CHTMLElemEvents(void)/*:m_lRef(0)*/ {
}

CHTMLElemEvents::~CHTMLElemEvents(void) {
    DispEventUnadviseAll();
}

