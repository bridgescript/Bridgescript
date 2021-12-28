' VBScript source code
WScript.Echo "************************************************************************" 
WScript.Echo "*                   Script Started: " & WScript.ScriptName&"                 *"
WScript.Echo "************************************************************************" 

Dim Ver1,Ver2,Ver3,Ver4
Dim fso, ScriptDirectory, BuildH, NewBuildH
Set fso = CreateObject("Scripting.FileSystemObject")
ScriptDirectory = Split(WScript.ScriptFullName,WScript.ScriptName)(0)

Set BuildH = fso.OpenTextFile(ScriptDirectory&"build.h",1,False)
Set NewBuildH = fso.CreateTextFile(ScriptDirectory&"build.txt",True)

Dim Line, NewLine
WScript.Echo "---------------------  Generating ""Build.h"" ... ---------------------"

on error resume next
do
    Line = BuildH.ReadLine()
    if err.number <> 0 then
        exit do
    end if
    
    NewLine = Line
    if InStr(Line,"VERSION_MAJOR") <> 0 then
        NewLine = GetVer1(Line)
    end if
    if InStr(Line,"VERSION_MINOR") <> 0 then
        NewLine = GetVer2(Line)
    end if
    if InStr(Line,"VERSION_3") <> 0 then
        NewLine = GetVer3(Line)
    end if
    if InStr(Line,"VERSION_4") <> 0 then
        NewLine = GetVer4(Line)
    end if
    if InStr(Line,"BUILD_DATE") <> 0 then
        NewLine = GetBuild(Line)
    end if
    err.Clear()
    WScript.Echo NewLine
    NewBuildH.WriteLine NewLine
    
Loop while True
BuildH.Close
NewBuildH.Close
fso.DeleteFile ScriptDirectory&"build.h",True
fso.MoveFile ScriptDirectory&"build.txt",ScriptDirectory&"build.h" 
WScript.Echo "---------------------            Done            ---------------------"


WScript.Echo "************************************************************************" 
WScript.Echo "*                   Script Executed: " & WScript.ScriptName&"                *"
WScript.Echo "************************************************************************" 

'*******************    helper functions   ********************
Function GetVer1(Line)
    if Left(Line, 22) = "#define VERSION_MAJOR " then
        WScript.Echo Right(Line,Len(Line) - 21)
        Ver1 = CStr(CLng(Right(Line,Len(Line) - 21)))
        GetVer1 = "#define VERSION_MAJOR " & Ver1
    end if
End Function

Function GetVer2(Line)
    if Left(Line, 22) = "#define VERSION_MINOR " then
        Ver2 = CStr(CLng(Right(Line,Len(Line) - 22)))
        GetVer2 = "#define VERSION_MINOR " & Ver2
    end if
End Function

Function GetVer3(Line)
    Ver3 = CStr(CLng(Date))
    GetVer3 = "#define VERSION_3 "&Ver3
End Function

Function GetVer4(Line)
    Ver4 = CStr(CLng(CDbl(Time) * 50000))
    GetVer4 = "#define VERSION_4 "&Ver4
End Function

Function GetBuild(Line)
    GetBuild = "#define BUILD_DATE """&CStr(Date)&" "&CStr(Time)&""""
End Function

