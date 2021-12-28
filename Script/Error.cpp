#include "stdafx.h"
#include "Script.h"
#include "Error.h"
#include <cstdarg>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace script;

//RunTimeError *RunTimeError::s_error = 0;

Error::Error() :
    m_errorStr(""),
    m_errorFile(""),
    m_errorFunction(""),
    m_errorLine(0),
    m_error(NO_ERROR),
    m_line(START_POSITION_OFFSET),
    m_position(START_POSITION_OFFSET) {
    memset(&m_symDesc, 0, sizeof(m_symDesc));
}

Error::~Error()
{
}

//Error& Error::Instance() {
//    static Error err;
//    return err;
//}

void Error::SetError(ERROR_TYPE error, const char * const errorStr, const char *const file, const char * const function, ui32 const line) {
    m_error = error;
    m_errorStr = errorStr;
    ui32 len = strlen(file);
    m_errorFile = 0;
    while (--len) {
        if (file[len] == '\\') {
            m_errorFile = &file[len + 1];
            break;
        }
    }
    if (!m_errorFile) {
        m_errorFile = file;
    }
    m_errorFunction = function;
    m_errorLine = line;
}

void Error::SetErrorEx(ERROR_TYPE error, const char * const errorStr, const char *const file, const char * const function, ui32 const line, SYMBOL_DESC &symDesc) {
    m_symDesc = symDesc;
    SetError(error, errorStr, file, function, line);
}
void Error::SetErrorLocation(string &file, ui32 line, ui32 pos) {
    m_file = file;
    if (m_symDesc.m_symId == 0) {
        m_line = line;
        m_position = pos;
    }
    else {
        m_line = m_symDesc.m_line;
        m_position = m_symDesc.m_pos;
    }
}

void Error::Set(int pos, int line, char* szFile, ERROR_TYPE error, const char *const szFormat, ...) {
    //int _Result;
    va_list _ArgList;
    va_start(_ArgList, szFormat);
    char buf[1024];
    sprintf_s(buf, szFormat, _ArgList);
    //m_strError = buf;
}

static string GetScriptLine(string &path, ui32 line) {
    ifstream file(path, ios::in | ios::binary);
    string script;
    //ui8 *pData = 0;
    //ui32 len = 0;
    file.seekg(0, std::ios::end);
    //script.reserve((size_t)file.tellg());
    //len = (size_t)file.tellg() + 1;
    //pData = new ui8[len];
    //pData[len] = 0;
    file.seekg(0, std::ios::beg);
    string s;
    for (ui32 i = START_POSITION_OFFSET; i < line; ++i) {
        getline(file, s);
    }
    getline(file, s);
    //file.read((char*)pData, len - 1);
    //file.readsome((char*)pData, len - 1);
    script.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return s;
}

void Error::Print(StdOutInterface *pStdOutInterface) {
#ifdef SCRIPT_DEBUGGER
    stringstream ss;
    if (NO_ERROR != m_error) {

        //for (ui32 k = 0; k <= 0xff; ++k) {
        //    printf("%d:%c ", k,k);
        //}
        //printf("\n");
        //printf("%s(%lu): %s ERROR: %s, line: %lu, pos: %lu \n", m_errorFile, m_errorLine, m_errorFunction, m_errorStr/*, m_line, m_pos*/);
        //string s = GetScriptLine(m_file, m_line);
        //s.pop_back();
        //ss << (char)218;
        //printf("%c", 218);
        //for (ui32 i = 0; i < s.length() + 2; ++i) {
        //    printf("%c", 196);
        //    ss << (char)196;
        //}
        //printf("%c", 191);
        //ss << (char)191;
        //pStdOutInterface->Print(ss.str());
        //ss.str("");
        //ss.clear();
        //printf("\n%c %s %c\n", 179, s.c_str(), 179);
        //ss << (char)179 << s.c_str() << (char)179;
        //pStdOutInterface->Print(ss.str());
        //pStdOutInterface->Print(s);
        //ss.str("");

        //printf("%c", 179);
        //ss << (char)179;
        /*string pointer;
        for (ui32 i = 0; i < m_position; ++i) {
            pointer.push_back(' ');
        }
        pointer.push_back(24);
        pStdOutInterface->Print(pointer);*/
        //printf("%s", pointer.c_str());
        //ss << pointer.c_str();

        //pointer.clear();
        //for (ui32 i = m_position; i < s.length() + 1; ++i) {
        //    pointer.push_back(' ');
        //}
        //pointer.push_back((char)179);
        //ss << (char)179;
        //printf(pointer.c_str());
        //ss << pointer.c_str();
        //pStdOutInterface->Print(ss.str());
        //ss.str("");

        //printf("\n%c", 192);
        //ss << (char)192;

        //for (ui32 i = 0; i < s.length() + 2; ++i) {
        //    printf("%c", 196);
        //    ss << (char)196;
        //}
        //printf("%c", 217);
        //ss << (char)217;
        //pStdOutInterface->Print(ss.str());
        //ss.str("");
        ss.str(string());
        //printf("\n\nERROR: %s in \"%s\", line: %lu, pos: %lu \n", m_errorStr, m_file.c_str(), m_line, m_position);
        ss << "ERROR: " << m_errorStr << " in \"" << m_file.c_str() << "\", line: " << m_line << ", pos: " << m_position;
        pStdOutInterface->Print(BUILD_OUT, ss.str());
        
        if (m_trace.size()) {
            pStdOutInterface->Print(BUILD_OUT, string("TRACE:"));
        }
        for (ui32 i = 0; i < m_trace.size(); ++i) {
            pStdOutInterface->Print(BUILD_OUT, m_trace[i]);
        }

        ss.str(string());

        //printf("       %s(%lu): %s\n", m_errorFile, m_errorLine, m_errorFunction);
        ss << "INTERNAL: " << m_errorFile << "(" << m_errorLine << "): " << m_errorFunction;
        pStdOutInterface->Print(BUILD_OUT, ss.str());
        //ss.str("");

    }
    else {
        //printf("\nBuild: succeeded\n\n");
        ss << "Build succeeded.";
        pStdOutInterface->Print(BUILD_OUT, ss.str());
    }

#else // DEBUUGER
    stringstream ss;
    if (NO_ERROR != m_error) {

        //for (ui32 k = 0; k <= 0xff; ++k) {
        //    printf("%d:%c ", k,k);
        //}
        //printf("\n");
        //printf("%s(%lu): %s ERROR: %s, line: %lu, pos: %lu \n", m_errorFile, m_errorLine, m_errorFunction, m_errorStr/*, m_line, m_pos*/);
        string s = GetScriptLine(m_file, m_line);
        s.pop_back();
        ss << (char)218;
        printf("%c", 218);
        for (ui32 i = 0; i < s.length() + 2; ++i) {
            printf("%c", 196);
            ss << (char)196;
        }
        printf("%c", 191);
        ss << (char)191;
        pStdOutInterface->Print(BUILD_OUT, ss.str());
        ss.str("");
        //ss.clear();
        printf("\n%c %s %c\n", 179, s.c_str(), 179);
        ss << (char)179 << s.c_str() << (char)179;
        pStdOutInterface->Print(BUILD_OUT, ss.str());
        ss.str("");

        printf("%c", 179);
        ss << (char)179;
        string pointer;
        for (ui32 i = 0; i < m_position; ++i) {
            pointer.push_back(' ');
        }
        pointer.push_back(24);
        printf("%s", pointer.c_str());
        ss << pointer.c_str();

        pointer.clear();
        for (ui32 i = m_position; i < s.length() + 1; ++i) {
            pointer.push_back(' ');
        }
        pointer.push_back((char)179);
        ss << (char)179;
        printf(pointer.c_str());
        ss << pointer.c_str();
        pStdOutInterface->Print(BUILD_OUT, ss.str());
        ss.str("");

        printf("\n%c", 192);
        ss << (char)192;

        for (ui32 i = 0; i < s.length() + 2; ++i) {
            printf("%c", 196);
            ss << (char)196;
        }
        printf("%c", 217);
        ss << (char)217;
        pStdOutInterface->Print(BUILD_OUT, ss.str());
        ss.str("");

        printf("\n\nERROR: %s in \"%s\", line: %lu, pos: %lu \n", m_errorStr.c_str(), m_file.c_str(), m_line, m_position);
        ss << "ERROR: " << m_errorStr << " in \"" << m_file.c_str() << "\", line: " << m_line << ", pos: " << m_position;
        pStdOutInterface->Print(BUILD_OUT, ss.str());
        ss.str("");

        printf("       %s(%lu): %s\n", m_errorFile, m_errorLine, m_errorFunction);
        ss << "       " << m_errorFile << "(" << m_errorLine << "): " << m_errorFunction;
        pStdOutInterface->Print(BUILD_OUT, ss.str());
        ss.str("");

    }
    else {
        printf("\nBuild: succeeded\n\n");
        ss << "\nBuild: succeeded\n\n";
    }
    pStdOutInterface->Print(BUILD_OUT, ss.str());
#endif // SCRIPT_DEBUGGER
}

/*********************************/

RUNTIME_RETURN RunTimeError::SetError(string name, SYMBOL_DESC &symDesc, ui32 seNumber, StackMemory *pStackMemory) {
    // revisit it later to make sure it belogns to the current thread!
    m_name = name;
    m_symDesc = symDesc;
    m_seNumber = seNumber;
    vector<string> trace;
    pStackMemory->GetTrace(trace);
    pStackMemory->GetString(m_stackString);
    if (m_pStdOutInterface) {
        ui32 threadId = GetCurrentThreadId();
        //string s = error.GetName();
        //if (s.length()) {
            //SYMBOL_DESC symDesc = error.GetSymDesc();
        string sline = to_string(symDesc.m_line);
        string spos = to_string(symDesc.m_pos);
        //SymbolStore &symStore = GetSymbolStore();
        string path;
        m_pSymStore->GetFileName(symDesc.m_fileId, path);
        string symName;
        m_pSymStore->GetSymbolName(symDesc.m_symId, symName);
        if (seNumber) {
            std::stringstream ss;
            ss << "0x" << std::hex << seNumber;
            string s = "thread: " + to_string(threadId) +  ", RUNTIME ERROR: " + m_name + " error:" + ss.str() + " in \"" + path + "\", line: " + sline + " ,pos: " + spos + ", sym: " + symName;
            m_pStdOutInterface->Print(DEBUG_OUT, s);
        }
        else {
            string s = "thread: " + to_string(threadId) + ", RUNTIME ERROR: " + m_name + " in \"" + path + "\", line: " + sline + " ,pos: " + spos + ", sym: " + symName;
            m_pStdOutInterface->Print(DEBUG_OUT, s);
        }
        if (trace.size()) {
            m_pStdOutInterface->Print(DEBUG_OUT, string("TRACE:"));
            for (ui32 i = 0; i < trace.size(); ++i) {
                m_pStdOutInterface->Print(DEBUG_OUT, trace[i]);
            }
        }
    }
    return RT_ERROR;
}
