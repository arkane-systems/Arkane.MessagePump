#include "stdafx.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;

//
// General Information about an assembly is controlled through the following
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
//
[assembly:AssemblyTitleAttribute(L"Arkane.MessagePump")];
[assembly:AssemblyDescriptionAttribute(L"A message-only window interface for .NET.")];

#if DEBUG
[assembly:AssemblyConfigurationAttribute(L"Debug")];
#else
[assembly:AssemblyConfigurationAttribute(L"Release")];
#endif

[assembly:AssemblyCopyrightAttribute(L"All code within published by Alon Fliess here - http://blogs.microsoft.co.il/alon/2011/12/26/message-only-window-in-for-net-application/ - and free to use. No copyright is implied or imputed.")];

//
// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version
//      Build Number
//      Revision
//
// You can specify all the value or you can default the Revision and Build Numbers
// by using the '*' as shown below:

[assembly:AssemblyVersionAttribute("1.0.0.0")];

[assembly:ComVisible(false)];

[assembly:CLSCompliantAttribute(true)];