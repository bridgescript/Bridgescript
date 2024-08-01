# BridgeScript programming language
[www.bridgescript.com](https://www.bridgescript.com/)<br>


> [!NOTE]
> This is my study project, however, it's fully functional and includes Visual Debugger.<br>
> [Bridgescript Source Code](https://github.com/bridgescript/Bridgescript)

Bridge script is statically typed, platform independent programming language. It's syntactically similar to C language. Bridge script does not have any built-in APIs in the interpreter. It provides mechanism to invoke external functions by defining a bridge (Library type) to invoke. Bridge script syntax derived from C with simplicity in mind and built-in memory management. If you are interested to get your hands on the Bridge script interpreter  [contact me ↗](http://www.linkedin.com/in/mikhail-botcharov-52362b17), otherwise you can play with it using [Bridge script debugger](#debugger).

## Contents
-  [Syntax](#syntax)
-  [Types relationship](#types_rel)
-  [Const primitive types](#prim)
-  [Structure type](#struct)
-  [Arrays](#arrays)
-  [Functions](#functions)
-  [References](#references)
-  [Bridge script function and C function relationship](#func_rel)
-  [Library type (Bridge)](#lib_type)
-  [Built-in functions](#built_in)
-  [Operators](#operators)
-  [Special operators](#spec_op)
-  [Statements](#statements)
-  [Bridge script references and C pointers relationship](#pointers)
-  [Run-time error handling](#error)
-  [Bridge script examples](#examples)
-  [CLI](#usage)
-  [Debugger GUI](#ui)
-  [Author](#aut)

<a name="syntax"/>

### Syntax
Since Bridge script syntax is similar to C syntax I am not going to repeat it here (for entertainment check wiki: [C syntax ↗](https://en.wikipedia.org/wiki/C_syntax))

<a name="types_rel"/>

### Types relationship
All basic Bridge script types have one to one mapping to C types.
|Bridge script|C|Size|
|------|------|------|
|*`bool`*|*`char`*|1 byte|
|*`i8`*|*`char`*|1 byte|
|*`ui8`*|*`unsigned char`*|1 byte|
|*`i16`*|*`short int`*|2 bytes|
|*`ui16`*|*`unsigned short int`*|2 bytes|
|*`i32`*|*`long int`*|4 bytes|
|*`ui32`*|*`unsigned long int`*|4 bytes|
|*`i64`*|*`long long int`*|8 bytes|
|*`ui64`*|*`unsigned long long int`*|8 bytes|
|*`float`*|*`float`*|4 bytes|
|*`double`*|*`double`*|8 bytes|

<a name="prim"/>

### Const primitive types
For constant integer values HEX and Binary formats are provided.
HEX prefixes: *`0X`*, *`0x`*
```c++
i32 v0 = 0xABCD1234, v1 = 0Xaaaa1111;
```
HEX postfixes: *`H`*, *`h`*
```c++
ui8 v0 = A1H, v1 = abh;
```
Binary postfixes: *`B`*, *`b`*
```c++
ui8 v0 = 001101B, v1 = 101b;
```
*`float`* and *`double`* formats are the same as in C
```c++
double d0 = -.25e-3, d1 = 0.002E-22;
float f0 = 1.5, f1 = 1.0E+3;
```
There is one built-in type which does not have corresponding C type - *`string`* type.
```c++
string str = "This is a string";
```

<a name="struct"/>

### Structure type
*`struct`* - this keyword is used to declare structure definition
```c++
struct MyStruct {
    ui32 v0;
    struct _SubStruct {
        bool _v0;
        i64 _v1;
    } v1;
    _SubStruct v2;
} var0;
MyStruct var1;
```
<a name="arrays"/>

### Arrays

Arrays declarations (where Type is any Bridge script type except *`array`* type):

*`array<Type>`* - declares array without size.

*`array<Type, dim>`* - declares array of size dim.

*`array<Type, dim0, dim1>`* - declares 2-dimensional array.

Array can be declared with any number of dimensions. Dimension of array can be an expression evaluated at run time, in this case only one-dimensional array is allowed. Expression based dimension allows dynamically allocate array at run-time.
```c++
function array<ui8>@ AllocateArray(ui32 size) {
    array<ui8, size> arr;
    return arr;
}
array<ui8> arr0 @= AllocateArray(10);
array<ui8> arr1 @= AllocateArray(20);
```
```c++
array<ui32, 10> arr;
array<ui32, 2, 2> arr2;
arr2[0, 0] = 0;
arr2[0, 1] = 1;
arr2[1, 0] = 2;
arr2[1, 1] = 3;
arr[1] = arr2[0, 1];
```
Dynamic dimension as an expression
```c++
array<i16, 2 * foo(n)> arr;
```
*`string`* variable can be assigned to *`array`* variable of type *`i8`* or *`ui8`* and vice versa, *`array`* must be static with at least one dimension.
```c++
array<ui8, 5> arr0;
string str = "Test";
arr0 = str;
array<ui8, 4> arr1;
arr1[0] = 0x41;
arr1[1] = 0x42;
arr1[2] = 0x43;
str = arr1;
```
<a name="functions"/>

### Functions
Function definition syntax:

*`function Type FooName([arguments if any]) { return value; }`*

Where Type is function return type, function can have any number of arguments and return statement is optional.
```c++
function ui32 GetSum(ui32 a0, ui32 a1) {
    return a0 + a1;
}
```
Function variable declaration
```c++
function<GetSum> fn;
array<function<GetSum>, 4> fnArr;
function<GetSum> fn0 = fn;
```
Uninitialized function variable is set to function instance by default.
Any function can be called implicitly:
-  when returned by a function call:
```c++
function ui32 F0(ui32 a) { return a + 1; }
function ui32 F1(ui32 a) { return a + 2; }
function function<F0> F(ui32 a) { if (a == 0) return F0; else return F1; }
F(0)(10);
F(1)(10);
```
-  when function is in array:
```c++
array<function<F0>, 2> fn;
fn[0] = F0;
fn[1] = F1;
fn[0](10);
fn[1](10);
```
<a name="references"/>

### References
All variables in the script are references (also see [Special operators](#spec_op)), however, to pass by reference you must use reference operator.
```c++
function ui32@ GetSum(ui32@ a0, ui32@ a1) {
    return a0 + a1;
}
```
Passing a variable by double reference allows to overwrite it's reference.
```c++
function bool GetRef(ui32@@ a0) {
    ui32 _var = 100;
    a @= _var; // reference assignment operator
    return true;
}
ui32 var = 10;
GetRef(var); // this function call will overwrite var reference by reference to _var
// now var references the same memory address as _var
```
Another way to pass a reference by reference is to declare an array of reference type of size 1 and pass it by reference.
```c++
struct ST {
    array<ui32@, 4>@ arr;
    ui64 v64;
};
array<ST@, 1> refSt;
refSt[0].v64 = 0x0000000087654321;
function ui32 GetRef(array<ST@, 1> @ref) {
    ST st;
    st.v64 = 0x1234567800000000;
    ui32 i = 0;
    for (; i < sizeof(st.arr) / sizeof(i); ++i) {
        st.arr[i] = (i + 1) * 10;
    }
    ref[0] @= st;
    return 0;
}
GetRef(refSt);
```

<a name="func_rel"/>

### Bridge script function and C function relationship

|Bridge script|C|
|---|---|
|*`function i32@ Foo(i32 arg) {}`*|*`long int* Foo(long int arg){ return NULL;}`*<br>*`typedef long int* (*Foo_Type)(long int);`*|
|**Function variable declaration**|**Function variable declaration**|
|*`function<Foo> fn;`*|*`Foo_Type fn = Foo;`*|
|**Function invocation**|**Function invocation**|
|*`i32 var @= fn(10);`*|*`i32 *p = fn(10);`*|


<a name="lib_type"/>

### Library type (Bridge)
Library type allows dynamic linking with external library. If external function return type is void then use ui32 as return type for library function description. To describe library function arguments only types of arguments are allowed.
Either library name or library path is allowed.
```c++
lib ("kernel32.dll") {
    ui32 GetModuleHandleA(array<ui8>@); // HMODULE GetModuleHandleA(LPCSTR lpModuleName);
} kernel;
array<ui8> moduleName @= str2arr("user32.dll");
ui32 handle = kernel.GetModuleHandleA(moduleName);
```

<a name="built_in"/>

### Built-in functions
*`array<ui8>@ str2arr(string@)`* - this function converts script's string to array of bytes.

*`array<ui16>@ str2warr(string@)`* - this function converts script's string to array of words (as Unicode).

*`string@ arr2str(array<ui8>@)`* - this function converts array of bytes to string.

*`string@ warr2str(array<ui16>@)`* - this function converts array of words (as unicode) to string.


<a name="operators"/>

### Operators
These operators have the same functionality as in C.
Arithmetic operators:
*`+`*, *`++`*, *`+=`*, *`-`*, *`--`*, *`-=`*, *`*`*, *`*=`*, *`/`*, *`/=`*, *`%`*, *`%=`*, *`=`*

Logical operators:
`||`, *`&&`*, `!`

Bitwise operators:
*`&`*, `|`, *`~`*, *`^`*, *`^=`*, *`>>`*, *`<<`*

Comparison operators:
*`==`*, `!=`, *`<`*, *`<=`*, *`>`*, *`>=`*

<a name="spec_op"/>

### Special operators
*`@=`* - this is reference assignment operator.
```c++
ui32 v0 = 10;
ui32 v1 @= v0; // now both variables point to the same memory
```
*`@`* - this operator is used to pass/return a variable to/from a function and to declare structure member variables.
```c++
function ui32@ Get(ui32 a0, ui32@a1, ui32@@ a2) {
    a2 @= a1; 
    return a0;
}
struct A {
    ui32@ v32;
    i8@   v0, v1, v2;
};
```
*`lock()`* - this operator locks memory which variable is referenced to. This operator should be used when you are passing local variable to asynchronous C function and caller goes out of scope. For example calling CreateThread() and passing local data as a parameter to the thread releases local variable when caller goes out of scope, hence you need to lock it and then unlock it in the thread.

*`unlock()`* - this operator unlocks memory locked by *`lock()`*
```c++
function ui32 Callback(array<ui8>@ a) {
    return a[0];
}
struct MyData {
    ui32 v0;
    function<Callback> fn;
    array<ui8, 4> v1;
};
function ui32 ThreadFoo(MyData@ arg) {
    unlock(arg);
    return arg.fn(arg.v1);
}
struct SECURITY_ATTRIBUTES {
    ui32   nLength;
    ui32   lpSecurityDescriptor;
    bool   bInheritHandle;
};
lib("kernel32.dll") {
    ui32 CreateThread(SECURITY_ATTRIBUTES@, ui32, function<ThreadFoo>, MyData@, ui32, ui32@);
    ui32 WaitForMultipleObjects(ui32, array<ui32>@, ui32, ui32);
    ui32 CloseHandle(ui32);
} kernel;
SECURITY_ATTRIBUTES sa;
MyData data;
ui32 threadId;
data.v1[0] = 10;
array<ui32, 1> handle;
handle[0] = kernel.CreateThread(sa, 0, ThreadFoo, lock(data), 0, threadId);
kernel.WaitForMultipleObjects(1, handle, true, -1);
kernel.CloseHandle(handle[0]);
```
*`sizeof()`* - this operator returns size of memory allocated for that variable, it's useful to get size of a struct and an array, if array is allocated externally this operator will return 0.

*`cast<Type>()`* - this is typecast operator. This is very powerful operator and it breaks memory constrains, when you typecast a variable static type paradigm is broken, as a result you might have memory leaks and memory corruption.
```c++
struct A {
    ui32 v0;
    ui32 v1;
} a;
a.v0 = 10;
a.v1 = 20;
ui64 v @= cast<ui64>(a);
array<ui8, 4> arr1dim;
array<ui8, 2, 2> arr2dim;
ui32 i = 0;
for(; i < 4; ++i) {
    arr1dim[i] = 1;
}
struct Typecasted {
    ui8 v0, v1, v2, v3;
} tc;
tc @= cast<Typecasted>(arr1dim); // now both variables points to the same memory
arr2dim = arr1dim; // copy by value
array<ui8> arrNodim @= arr1dim; // now arrNodim points to the same memory
```

<a name="statements"/>

### Statements
*`if`*, *`else`*, *`for`*, *`while`*, *`break`*, *`continue`*, *`switch`*, *`case`*, *`return`* - these statements have the same meaning as in C.

include statement:
```c++
include "script_to_inlcude.bridge";
```
*`include`* statement path is relative to the current script.

<a name="pointers"/>

### Bridge script references and C pointers relationship
|Bridge script|C|
|-------|-------|
|*`struct A {   `*<br>*`  i8@ v0, v1;`*<br>*`}           ;`*<br>*`struct B {`*<br>*`  A@ v0;    `*<br>*`};        `*<br>*`struct C {       `*<br>*`  array<i8@>@ v0;`*<br>*`};               `*|*`struct A {      `*<br>*`  char *v0, *v1;`*<br>*`};              `*<br>*`struct  B {`*<br>*`  A *v0;     `*<br>*`};         `*<br>*`struct C {  `*<br>*`  char **v0;`*<br>*`};          `*|


<a name="error"/>

### Run-time error handling
If a run-time error occurs an error object is thrown, to catch it use *`error`* statement - *`error(e){}`*

Error object data members:

*`name`* - name of the error, type is *`string`*.

*`line`* - line number where error occurred, type is *`ui32`*.

*`position`* - first character position of the symbol, type is *`ui32`*.

*`file`* - file path, type is *`string`*.

*`trace`* - stack trace, type is *`string`* (new line separated strings).

*`symbol`* - symbol name, type is *`string`*.

```c++
function i8 f0() { 
    ui32 v0 = 100, v1 = 0; 
    v0 = v0 / v1;
}
function i8 f1() {
    f0();
}
f1();
error (e) {
    string err = e.name;
    string trace = e.trace;
}
```

##
<a name="examples"/>

### Bridge script examples

-    [Hello World! ↗](https://github.com/bridgescript/Hello-World)
-    [Threads ↗](https://github.com/bridgescript/Threads)
-    [Window ↗](https://github.com/bridgescript/Window)

##
<a name="usage"/>

### CLI
To run bridge script:
```cmd
script.exe script_file_path
```
Compile or decompile script:
```cmd
script.exe script_file_path [options]
```
`[options]`
-    `-c` - compile only
-    `-d` - decompile only

##
<a name="debugger"/>

### Bridge script debugger
Bridge script interpreter provides debugging facilities through debugging API. I created [Bridge debugger](Release/BridgeDebugger.exe) for Microsoft Windows(c) platform. It exposes all standard debugging features using GUI. Bridge debugger has built-in interpreter and provides such features as:

-    compile/decompile script
-    edit script (this feature is limited to simple editing, it does not replace editing tool, but rather nice to have if you need to fix your code)
-    file view/browser
-    debugger actions: run, step over, step in, step out, pause and stop
-    set/delete breakpoints
-    breakpoints view
-    function view/browser (available when script is running)
-    global variables view (available when script is running)
-    threads view (available when script is running)
-    thread stack and variables view for each runnig thread (available when script is running)
-    build and debug output views
-    Compiled Bridge script can be debugged without decompiling it.

<a name="ui"/>

### Debugger GUI
In order to run a script you must set start up script using pop up file menu, Compile/Decompile menu items available as well. To execute debugger actions: run, step over, step in, step out, pause and stop you can use toolbar, application menu or keyboard shortcuts. Following views are clickable: Function Browser, Build, Debug, Breakpoints, Threads, and Stack view of Thread tab.
#### Breakpoints
![Breakpoints](https://www.bridgescript.com/debugger_view.jpg)
#### threads
![threads](https://www.bridgescript.com/debugger_view_2.jpg)
#### file menu
![file menu](https://www.bridgescript.com/file_menu.jpg)
#### global variables
![global variables](https://www.bridgescript.com/global_vars.jpg)
#### variables view
![variables view](https://www.bridgescript.com/global_vars_2.jpg)
#### thread view
![thread view](https://www.bridgescript.com/thread_view.jpg)
#### stack view
![stack view](https://www.bridgescript.com/stack_view.jpg)

##
<a name="aut"/>

### Author

[Mikhail Botcharov ↗](http://www.linkedin.com/in/mikhail-botcharov-52362b17)


***Copyright 2019 Mikhail Botcharov. All Rights Reserved.***

<!--
**bridgescript/Bridgescript** is a ✨ _special_ ✨ repository because its `README.md` (this file) appears on your GitHub profile.

Here are some ideas to get you started:

- 🔭 I’m currently working on ...
- 🌱 I’m currently learning ...
- 👯 I’m looking to collaborate on ...
- 🤔 I’m looking for help with ...
- 💬 Ask me about ...
- 📫 How to reach me: ...
- 😄 Pronouns: ...
- ⚡ Fun fact: ...
-->
