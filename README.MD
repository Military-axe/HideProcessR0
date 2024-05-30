# Hide Process In Ring 0

Break the node in `EPROCESS->ActiveProcessLinks` to hide the process

## Usage

### Hide Process By Break Chain

IO Control code 0x6666

```c
#define IOCTL_HIDE_BY_PID \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6666, METHOD_BUFFERED, FILE_ANY_ACCESS)
```

Send PID to Driver, then driver will hide the process.

### Hide Process By Set EPROCESS ImageFileName

```c
#define IOCTL_SET_IMAGE_NAME \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x6667, METHOD_BUFFERED, FILE_ANY_ACCESS)
```

Set the SET_IMAGE_NAME to driver, then will set **new string** to cover the old iamge file name 

```c
typedef struct SET_IMAGE_NAME {
    UCHAR bImageName[250]; // new image name string
    UINT64 pid;            // the specify process id
}SET_IMAGE_NAME, *PSET_IMAGE_NAME ;
```

## Env

This is a example use in win 21h1 version.

## Build

I think is just need win10? vs2022?

My Build env.

```
Microsoft Visual Studio Community 2022
Version 17.9.6
VisualStudio.17.Release/17.9.6+34728.123
Microsoft .NET Framework
Version 4.8.09032

Installed Version: Community

Visual C++ 2022   00482-90000-00000-AA134
Microsoft Visual C++ 2022

ASP.NET and Web Tools   17.9.199.22661
ASP.NET and Web Tools

Azure App Service Tools v3.0.0   17.9.199.22661
Azure App Service Tools v3.0.0

C# Tools   4.9.0-3.24121.1+a98c90d56455379836dd5c845b35fa932b00cfa3
C# components used in the IDE. Depending on your project type and settings, a different version of the compiler may be used.

Debugging Tools for Windows   10.0.26100.1
Integrates the Windows Debugger functionality (http://go.microsoft.com/fwlink/?linkid=223405) in Visual Studio.

Microsoft JVM Debugger   1.0
Provides support for connecting the Visual Studio debugger to JDWP compatible Java Virtual Machines

NuGet Package Manager   6.9.2
NuGet Package Manager in Visual Studio. For more information about NuGet, visit https://docs.nuget.org/

Test Adapter for Boost.Test   1.0
Enables Visual Studio's testing tools with unit tests written for Boost.Test.  The use terms and Third Party Notices are available in the extension installation directory.

Test Adapter for Google Test   1.0
Enables Visual Studio's testing tools with unit tests written for Google Test.  The use terms and Third Party Notices are available in the extension installation directory.

TypeScript Tools   17.0.30103.2001
TypeScript Tools for Microsoft Visual Studio

Visual Basic Tools   4.9.0-3.24121.1+a98c90d56455379836dd5c845b35fa932b00cfa3
Visual Basic components used in the IDE. Depending on your project type and settings, a different version of the compiler may be used.

Visual Studio IntelliCode   2.2
AI-assisted development for Visual Studio.

Windows Driver Kit   10.0.26100.1
Headers, libraries, and tools needed to develop, debug, and test Windows drivers (msdn.microsoft.com/en-us/windows/hardware/gg487428.aspx)
```