# BridgeScript programming language
https://www.bridgescript.com/

> [!NOTE]
> This is my study project, however, it's fully functional and includes Visual Debugger.

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

<a name="syntax"/>

### Syntax
Since Bridge script syntax is similar to C syntax I am not going to repeat it here (for entertainment check wiki: [C syntax ↗](https://en.wikipedia.org/wiki/C_syntax))

<a name="types_rel"/>

### Types relationship
All basic Bridge script types have one to one mapping to C types.
|Bridge script|C|Size|
|------|------|------|
|*bool*|*char*|1 byte|
|*i8*|*char*|1 byte|
|*ui8*|*unsigned char*|1 byte|
|*i16*|*short int*|2 bytes|
|*ui16*|*unsigned short int*|2 bytes|
|*i32*|*long int*|4 bytes|
|*ui32*|*unsigned long int*|4 bytes|
|*i64*|*long long int*|8 bytes|
|*ui64*|*unsigned long long int*|8 bytes|
|*float*|*float*|4 bytes|
|*double*|*double*|8 bytes|

<a name="prim"/>

### Const primitive types
For constant integer values HEX and Binary formats are provided.
HEX prefixes: *0X*, *0x*
```c++
i32 v0 = 0xABCD1234, v1 = 0Xaaaa1111;
```
HEX postfixes: *H*, *h*
```c++
ui8 v0 = A1H, v1 = abh;
```
Binary postfixes: *B*, *b*
```c++
ui8 v0 = 001101B, v1 = 101b;
```
*float* and *double* formats are the same as in C
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
*struct* - this keyword is used to declare structure definition
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

Arrays declarations (where Type is any Bridge script type except array):

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

<a name="debugger"/>

### Bridge script debugger

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
