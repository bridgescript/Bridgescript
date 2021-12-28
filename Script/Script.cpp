// Script.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Compiler.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

#include "Script.h"

using namespace script;
using namespace std;


void TestWrite(char *name, char *s) {
    string path("C:\\Projects\\Script\\Script");
    path = path + "\\" + name;

    std::ofstream file(path, std::ofstream::binary);
    //ifstream file(path);// , ios::in /*| ios::binary*/);
    file.write(s, strlen(s));
    file.close();
}

void NumTestCase(Interpreter &interpreter) {
    interpreter.TestNum(string("0"));
    interpreter.TestNum(string("123"));
    interpreter.TestNum(string("12345"));
    interpreter.TestNum(string("123456"));
    interpreter.TestNum(string("-65534"));
    interpreter.TestNum(string("65534"));
    interpreter.TestNum(string("32767"));
    interpreter.TestNum(string("-32767"));
    interpreter.TestNum(string("+32767"));
    interpreter.TestNum(string("-32768"));
    interpreter.TestNum(string("-10"));
    interpreter.TestNum(string("+.0"));
    interpreter.TestNum(string("-.0"));
    interpreter.TestNum(string("-.001"));
    interpreter.TestNum(string(".001"));
    interpreter.TestNum(string("0.001"));
    interpreter.TestNum(string("-0.002"));
    interpreter.TestNum(string("0e-10"));
    interpreter.TestNum(string("10E+3"));
    interpreter.TestNum(string("0.12000e-02"));
    interpreter.TestNum(string("-0.12000e-02"));
    interpreter.TestNum(string("001e+2"));
    interpreter.TestNum(string("0.001"));
    interpreter.TestNum(string("-0.002e+23"));
}

/*************************************************************/
void OptionRunner::Run() {
    if (Load() && Exec()) {
        //cout << "Done!" << endl;
    }
}
/*************************************************************/
bool Help::Exec() {
    PrintF  _;
    _.Print(BUILD_OUT, string("invalid number of parameters!"));

    _.Print(BUILD_OUT, string("command: script.exe script_file_path [options]"));
    _.Print(BUILD_OUT, string("options"));
    _.Print(BUILD_OUT, string("         -c - compile only"));
    _.Print(BUILD_OUT, string("         -d - decompile only"));
    return true;
}
/*************************************************************/

Reader* ScriptRunner::GetReader() {
    switch (m_extType) {
        case EXT_TEXT_SCRIPT: {
            return new ScriptReader(false, m_interpreter, m_error);
        }
        case EXT_BIN_SCRIPT: {
            return new BinaryReader(false, m_interpreter, m_error);
        }
        default:
            return 0;
    }
}

bool ScriptRunner::Load() {
    assert(m_pReader == 0);
    if (m_pReader = GetReader()) {
        ui32 win32Error = 0;
        m_interpreter.PushIncludePath(m_path, win32Error);
        if ((win32Error == S_OK) && m_pReader->Read(m_path, win32Error)) {
            m_error.Print(m_pStdOutInterface); // ?
#ifdef TRACE_OBJECTS
            m_interpreter.Loaded(true);
#endif // TRACE_OBJECTS
            return true;
        }
        else {
            if (win32Error != S_OK) {
                LPSTR errorStr = 0;
                ui32 size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                           0, win32Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorStr, 0, NULL);

                m_error.SetError(CANNOT_OPEN_FILE, errorStr, __FILE__, __FUNCTION__, __LINE__);
                //Free the buffer.
                LocalFree(errorStr);
            }
            else {
                if (m_extType == EXT_TEXT_SCRIPT) {
                    if (!m_error.IsLocationSet()) {
                        m_error.SetErrorLocation(m_path, m_pReader->GetLine(), m_pReader->GetPosition());
                    }
                }
            }
            m_error.Print(m_pStdOutInterface);
        }
    }
    else {
        m_pStdOutInterface->Print(BUILD_OUT, string("Unknown script type!"));
    }
    return false;
}

bool ScriptRunner::Exec() {
    if (m_error == NO_ERROR) {
        bool good = false;
        try {
            good = m_interpreter.Run();
        }
        catch (SE_Exception &e) {
            //SYMBOL_DESC symDesc = { 0 };
            //m_interpreter.GetMemory().GetMainStackMemory()->GetRunTimeError().SetError("SE Exception", symDesc, e.getSeNumber());
            std::stringstream ss;
            ss << "0x" << std::hex << e.getSeNumber();
            m_pStdOutInterface->Print(DEBUG_OUT, string("SE Exception! ") + ss.str());
        }
        return good;
    }
    
    return true;
}

EXTENTION_TYPE ScriptRunner::GetExtentionType() {
    string ext = m_path.substr(m_path.find_last_of("."), m_path.length());
    if (ext == EXT_TEXT_SCRIPT_STRING) {
        return EXT_TEXT_SCRIPT;
    }
    else if (ext == EXT_BIN_SCRIPT_STRING) {
        return EXT_BIN_SCRIPT;
    }
    return EXT_INVALID;
}
/*************************************************************/
bool CompilerRunner::Load() {
    if (m_extType == EXT_BIN_SCRIPT) {
        m_pStdOutInterface->Print(BUILD_OUT, string("Script already compiled!"));
        return false;
    }
    return ScriptRunner::Load();
}

bool CompilerRunner::Exec() {
    string path = m_path + EXT_BIN_SCRIPT_STRING;
    BinarySerializer bser(m_interpreter);
    if (!bser.Write(path)) {
        m_error.Print(m_pStdOutInterface);
        return false;
    }

    return true;
}
/*************************************************************/
bool DecompilerRunner::Load() {
    if (m_extType == EXT_TEXT_SCRIPT) {
        m_pStdOutInterface->Print(BUILD_OUT, string("Script already decompiled!"));
        return false;
    }

    return ScriptRunner::Load();
}

bool DecompilerRunner::Exec() {
    string path = m_path + EXT_TEXT_SCRIPT_STRING;
    TextSerializer tser(m_interpreter);
    if (!tser.Write(path)) {
        m_error.Print(m_pStdOutInterface);
        return false;
    }
    return true;
}
/*************************************************************/
OptionRunner* OptionSelector::GetRunner(int count, char** ppArgs) {
    switch (count) {
        case 1: {
            return new Help;
        }
        case 2: {
            static PrintF  s_printf;
            return new ScriptRunner(ppArgs[1], &s_printf);
        }
        case 3: {
            string path;
            OPTION option = OptionSelector::TranslateParameters(ppArgs[1], ppArgs[2], path);
            switch (option) {
                case OPT_COMPILE: {
                    static PrintF  s_printf;
                    return new CompilerRunner(path, &s_printf);
                }
                case OPT_DECOMPILE: {
                    static PrintF  s_printf;
                    return new DecompilerRunner(path, &s_printf);
                }
                default: {
                    PrintF  _;
                    _.Print(BUILD_OUT, string("Unknown option!"));
                    break;
                }
            }
            break;
        }
        default: {
            PrintF  _;
            _.Print(BUILD_OUT, string("invalid number of parameters!"));
            break;
        }
    }
    return 0;
}

OPTION OptionSelector::TranslateParameters(string arg0, string arg1, string &path) {
    OPTION opt = OPT_NONE;

    if (arg1 == "-c") { // compile
        path = arg0;
        opt = OPT_COMPILE;
    }
    else if (arg1 == "-d") { // decompile
        path = arg0;
        opt = OPT_DECOMPILE;
    }
    else if (arg0 == "-c") { // compile
        path = arg1;
        opt = OPT_COMPILE;
    }
    else if (arg0 == "-d") { // decompile
        path = arg1;
        opt = OPT_DECOMPILE;
    }

    return opt;
}
/*************************************************************/
