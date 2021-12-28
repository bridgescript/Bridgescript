#pragma once
//#include "build.h"

#define SCRIPT_DEBUG_INFO 1

#define _BRIDGE_SCRIPT_VER(v0,v1,v2,v3) SCRIPT_SYM_TO_STR(v0) "." SCRIPT_SYM_TO_STR(v1) "." SCRIPT_SYM_TO_STR(v2) "." SCRIPT_SYM_TO_STR(v3)
#define BRIDGE_SCRIPT_VER()             _BRIDGE_SCRIPT_VER(VERSION_MAJOR,VERSION_MINOR,VERSION_3,VERSION_4)

#define SCRIPT_MAJOR_VERSION  VERSION_MAJOR
#define SCRIPT_MINOR_VERSION  VERSION_MINOR
#define SCRIPT_SINATURE       "MBKB"

typedef char                i8;
typedef unsigned char       ui8;
typedef short               i16;
typedef unsigned short      ui16;
typedef long                i32;
typedef unsigned long       ui32;
typedef long long           i64;
typedef unsigned long long  ui64;
typedef                     ui32 ThreadId;

#define START_POSITION_OFFSET 1
#define TAB_SIZE              8

typedef ui32 SYMBOL_ID;
typedef ui32 PRECEDENCE;
typedef ui32 MEMORY_OFFSET;
typedef ui32 AGGREGATE_TYPE_OFFSET;

#define MEMORY_OFFSET_MASK             0x3fffffff // 0011 1111 1111 1111 1111 1111 1111 1111
#define MEMORY_OFFSET_BIT_MASK         0xC0000000 // 11xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
#define MEMORY_BLOCK_MASK              0x3fffffff // 0011 1111 1111 1111 1111 1111 1111 1111
#define MEMORY_BLOCK_BIT_MASK          0xC0000000 // 11xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx

//#define MEMORY_OFFSET_BIT_STACK        0x00000000 // 00xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
//member access or array bit
#define MEMORY_OFFSET_BIT_SUB_STACK    0x40000000 // 01xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
//global memory bit
#define MEMORY_BLOCK_BIT_GLOBAL        0x80000000 // 10xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
//#define MEMORY_OFFSET_BIT_GLOBAL       0xC0000000 // 11xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
//#define MEMORY_BLOCK_BIT_GLOBAL        0xC0000000 // 11xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx

#define GLOBAL_VARIABLE_BIT             0x80000000
#define GLOBAL_VARIABLE_MASK            0x7fffffff
#define INVALID_MEMORY_OFFSET           0xffffffff
#define INVALID_AGGREGATE_TYPE_OFFSET   0xffffffff
#define INVALID_STACK_FRAME_IDX         0xffffffff
#define INVALID_STACK_IDX               0xffffffff
#define INVALID_REF                     0xffffffff
#define INVALID_REF_PTR                 0xffffffff
#define INVALID_VAR_IDX                 0xffffffff
#define INVALID_THREAD_ID               0xffffffff

#define GET_VALUE_REFERENCE             (MemoryBlock*)0xffffffff

#define BINARY_STREAM_BYTE_MASK       0x3f       // 0011 1111
#define BINARY_STREAM_ONE_BYTE_MAX    0x3f       // 0011 1111
#define BINARY_STREAM_TWO_BYTE_MAX    0x3fff     // 0011 1111 1111 1111
#define BINARY_STREAM_THREE_BYTE_MAX  0x3fffff   // 0011 1111 1111 1111 1111 1111
#define BINARY_STREAM_FOUR_BYTE_MAX   0x3fffffff // 0011 1111 1111 1111 1111 1111 1111 1111

typedef enum Binary_Stream_Item_Size {
    ONE   = 0x00, // 0000 0000
    TWO   = 0x40, // 0100 0000
    THREE = 0x80, // 1000 0000
    FOUR  = 0xc0, // 1100 0000
} BINARY_STREAM_ITEM_SIZE;

#define SYMBOL_ID_MAX               0x3fffffff
#define PRECEDENCE_INVALID          0xffffffff
#define POS_MAX                     0xffffffff
#define LINE_MAX                    0xffffffff
#define FILE_ID_MAX                 0xffffffff

typedef enum Runtime_Return {
    RT_NO_ERROR,
    RT_ERROR,
    RT_BREAK,
    RT_RETURN,
    RT_CONTINUE,
    RT_EXCEPTION,
    RT_STOP
} RUNTIME_RETURN;

typedef struct SymbolDesc {
    SYMBOL_ID m_symId;
    SymbolDesc() : m_symId(SYMBOL_ID_MAX), m_fileId(0), m_line(0), m_pos(0) {}
//    SymbolDesc(SYMBOL_ID symId) : m_symId(symId)
//#ifdef SCRIPT_DEBUG_INFO
        //, m_line(0), m_pos(0) {}
    ui32      m_fileId;
    ui32      m_line;
    ui32      m_pos;
    SymbolDesc(SYMBOL_ID symId) : m_symId(symId), m_fileId(0), m_line(0), m_pos(0) {}
    SymbolDesc(SYMBOL_ID symId, ui32 fileId, ui32 line, ui32 pos) : m_symId(symId), m_fileId(fileId), m_line(line), m_pos(pos) {}
//#else // SCRIPT_DEBUG_INFO
    //{}
//#endif // SCRIPT_DEBUG_INFO
} SYMBOL_DESC;
//#define SYMBOL_ID ui32


#define SCRIPT_SYM_TO_STR(a)        #a
#define SCRIPT_CONCAT_0(a,b)        a##b
#define SCRIPT_CONCAT_1(a,b)        SCRIPT_CONCAT_0(a,b)
#define SCRIPT_COMPILE_ASSERT(e)    enum { SCRIPT_CONCAT_1(val, __LINE__) = 1/(int)(!!(e == 0)) };

#define _STR(s)     #s
#define STR(s)      _STR(s)
#define TODO(msg)   __pragma(message(__FILE__ "(" STR(__LINE__) "): TO DO: " msg))
//#pragma message( __FILE__ "(" STR(__LINE__) "): TO DO: Fix it to handle dynamic arrays!")

#define TRACE_OBJECTS
//#define TRACE_DELETE_OBJECTS
#define TRACE_MEMORY
//#define DEV_MODE
//#define SCRIPT_DEBUGGER
#define MEMORY_ACCESS_EXCEPTION

/************** Temporary below ****************/

#define DEL    // delete initial memory pointer regardless of offset to that memory, must test more

#define NUMBER // number handling
#define UNSAFE_STRING // don't allow string type to be used for lib calls

//#define STACK_PL // no allocations for stack variable placeholders