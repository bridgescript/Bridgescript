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

//string GetPath(const char * const fileName) {
//    string path("C:\\Projects\\Script\\MFCApplication1");
//    return (path + "\\" + fileName);
//}

void Main(int count, char** ppArgs) {
    if (OptionRunner *pOptionRunner = OptionSelector::GetRunner(count, ppArgs)) {
        pOptionRunner->Run();
        delete pOptionRunner;
    }
}

int main(int count, char** ppArgs)
{
#ifdef DEV_MODE
    //TestReference();
    Error error;
    char *test[] = {//"refs.js",
                    ////"lib.js",
                    //"function.js",//, 
        "memleak.js" ,// "while.js", "switch.js", "serializer_test.js", 
                    //"number.js",
                    ////"global.js",
                    //"array.js",
    };

    Interpreter interpreter(error);
    //InterpreterTest interpreter(error);

    //NumTestCase(interpreter);
    //
    //return 0;

    SE_Exception::Init();
    //ui32 *pv32 = 0;
    //try {
    //    *pv32 = 10;
    //}
    //catch (SE_Exception &e) {
    //}
    for (int i = 0; i < sizeof(test) / sizeof(char*); ++i) {
        ScriptReader reader(false, interpreter, error);
        string path = GetPath(test[i]);

        printf("************ %d) BEGIN: %s ************\n", i, test[i]);

        if (reader.Read(path)) {
            static PrintF s_printf;
            error.Print(&s_printf);
#if 1
            TextSerializer ser(interpreter);
            string con("CON");
#if 1 // disable printout to console
            ser.Write(con);
            printf("\n");
#endif 
            BinarySerializer bser(interpreter);
            bser.Write(path + ".bin");
            printf("\n");
            if (!interpreter.Run()) {
                printf("Runtime ERROR!");
            }
#ifdef TRACE_OBJECTS
            interpreter.DeleteRunable();
            Memory::GetInstance().DumpObjects();
#endif // TRACE_OBJECTS
#if defined(TRACE_MEMORY) 
            PrintF printF;
            Memory::GetInstance().DumpMemoryBlocks(&printF);
            interpreter.DeleteRunable();
#endif // defined(TRACE_MEMORY) 

#if 0
            BinaryReader r(false, interpreter, error);
            r.Read(path + ".bin");
            if (!interpreter.Run()) {
                printf("Runtime ERROR!");
            }
#endif
#else
            //BinarySerializer ser(interpreter);
            //ser.Write(path + ".bin");
            BinaryReader r(false, interpreter, error);
            r.Read(path + ".bin");

            TextSerializer ser1(interpreter);
            string con("CON");
            ser1.Write(con);
            if (!interpreter.Run()) {
                printf("Runtime ERROR!");
            }
#endif
        }
        else {
//#ifdef _DEBUG
            error.SetErrorLocation(path, reader.GetLine(), reader.GetPosition());
            //printf("\n    >>>>    ERROR: \"%s\" line:%lu, pos: %lu    <<<<\n\n", path.c_str(), reader.GetLine(), reader.GetPosition());
//#else // _DEBUG
//            printf("\n    >>>>    ERROR    <<<<\n\n");
//#endif // _DEBUG
            static PrintF s_printf;
            error.Print(&s_printf);
        }
        printf("\n************ %d) END: %s ************\n\n", i, test[i]);
    }

#else // DEV_MODE
    Main(count, ppArgs);
#endif // DEV_MODE
    return 0;
}

