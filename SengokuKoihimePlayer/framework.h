
#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

/*CommCtrl*/
#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#pragma comment(lib, "Comctl32.lib")

/*Media Foundation*/
#pragma comment (lib,"Mfplat.lib")

/*WIC*/
#pragma comment (lib,"Windowscodecs.lib")

/*Direct2D*/
#pragma comment (lib,"D2d1.lib")
#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"dxguid.lib")

/*DirectWrite*/
#pragma comment (lib,"Dwrite.lib")

/*Windows Shell*/
#pragma comment(lib, "Shlwapi.lib")
