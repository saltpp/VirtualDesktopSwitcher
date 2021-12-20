/*
The MIT License (MIT)

Copyright (c) 2021 Tomoki Shiono

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// VDS.cpp : アプリケーションのエントリ ポイントを定義します。
//

//-- {
// Virtual Desktop Swicher
// 
// modified areas are marked with "//--"
// 
// referred to the following sites, thank you so much
// - https://www.cyberforum.ru/blogs/105416/blog3671.html
// - https://bitbucket.org/sstregg/windows-10-virtual-desktops-api/src
// - https://www.wabiapp.com/WabiSampleSource/windows/is_window_on_current_virtual_desktop.html
// - https://docs.microsoft.com/ja-jp/windows/win32/api/shobjidl_core/nf-shobjidl_core-ivirtualdesktopmanager-getwindowdesktopid
// - http://grabacr.net/archives/5701
// - https://social.msdn.microsoft.com/Forums/sqlserver/en-US/fd52fc39-d1f3-4da8-8b68-debceab75ffd/requesting-api-calls-for-windows-10s-virtual-desktops?forum=windowsgeneraldevelopmentissues
// - https://techcommunity.microsoft.com/t5/report-an-issue/virtual-desktop-api-does-not-work-any-more-for-notifications/m-p/2614296
//
//-- }


#include "framework.h"
#include "VDS.h"

//-- {
//#include <tchar.h>
//#include <windows.h>
//#include <ShObjIdl.h>
//#include <WinUser.h>
#include <shellapi.h>
#include "VirtualDesktopsAPI.h"

EXTERN_C const GUID CLSID_ImmersiveShell = { 0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39 };

#define ARRAY_SIZE_OF(a) (sizeof(a) / sizeof(a[0]))

#define ID_TIMER (1000)
#define ID_TRAYICON (1001)
#define WM_TASKTRAY (WM_APP + 1) 

#define LINE_BUFFER_SIZE (0x100)
#define MAX_LINE_LOG (20)
static WCHAR l_szLog[MAX_LINE_LOG][LINE_BUFFER_SIZE];
static int l_nLogIndex = 0;
static int l_nLogIndexPrev;     // to redraw

static int l_nInterval = 200;
static int l_nIntervalCount = 4;    // 200ms x 4
static int l_nEdgeLeft = 0;
static int l_nEdgeRight = 2560 - 1;
static int l_nEdgeCount;
static bool l_bNeedShift;
static bool l_bNeedControl;
static bool l_bShow;
static HMENU l_menuTaskTray;

static VirtualDesktops::API::IVirtualDesktopManagerInternal* l_pVirtualDesktopManagerInternal = NULL;
//-- }

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VDS, szWindowClass, MAX_LOADSTRING);

    //-- {
    // check if already launched
    HWND hWnd = FindWindow(szWindowClass, szTitle);
    if (hWnd) {
        SetForegroundWindow(hWnd);
        return TRUE;
    }
    //-- }

    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VDS));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VDS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VDS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}



//-- {
void _Log(const WCHAR* szLog) {
    wcscpy_s(l_szLog[l_nLogIndex], szLog);
    ++l_nLogIndex;
    if (l_nLogIndex == MAX_LINE_LOG) {
        l_nLogIndex = 0;
    }
}

void Log(const WCHAR* fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    WCHAR szBuf[LINE_BUFFER_SIZE];
    vswprintf(szBuf, ARRAY_SIZE_OF(szBuf), fmt, arg_ptr);
    va_end(arg_ptr);

    _Log(szBuf);
}

void ReadIniFile(void)
{
    // get filename of exe
    WCHAR szBuf[_MAX_PATH];
    GetModuleFileName(GetModuleHandle(NULL), szBuf, ARRAY_SIZE_OF(szBuf));

    // remove filename
    WCHAR* p = wcsrchr(szBuf, L'\\');
    if (p) {
        *(p + 1) = L'\0';
    }

    // concatenate filename of ini
    wcscat_s(szBuf, L"VDS.ini");

    #define INI_SECTION_NAME L"Virtual Desktop Switcher"
    l_nInterval = GetPrivateProfileIntW(INI_SECTION_NAME, L"Interval", 200, szBuf);
    l_nIntervalCount = GetPrivateProfileIntW(INI_SECTION_NAME, L"IntervalCount", 4, szBuf);
    l_bNeedShift = GetPrivateProfileIntW(INI_SECTION_NAME, L"NeedShift", 0, szBuf);
    l_bNeedControl = GetPrivateProfileIntW(INI_SECTION_NAME, L"NeedControl", 0, szBuf);
}

void AddTaskTrayIcon(HINSTANCE hInstance, HWND hWnd) {
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP);
    nid.hWnd = hWnd;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VDS));
    nid.uID = ID_TRAYICON;
    nid.uCallbackMessage = WM_TASKTRAY;
    wcscpy_s(nid.szTip, 64, szTitle);
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void DeleteTaskTrayIcon(HWND hWnd) {
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = ID_TRAYICON;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void LoadMenuTaskTray(HINSTANCE hInstance)
{
    l_menuTaskTray = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_TASKTRAY));
    l_menuTaskTray = GetSubMenu(l_menuTaskTray, 0);
}

int Init(HINSTANCE hInstance, HWND hWnd) {

    // read ini file
    ReadIniFile();
    Log(L"duration=%d..%d", l_nInterval * l_nIntervalCount, l_nInterval * (l_nIntervalCount + 1));

    // for virtual desktop api
    IServiceProvider* pServiceProvider = NULL;
    HRESULT hResult = ::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID*)&pServiceProvider);
    if (FAILED(hResult)) {
        return hResult;
    }
    hResult = pServiceProvider->QueryService(VirtualDesktops::API::CLSID_VirtualDesktopAPI_Unknown, &l_pVirtualDesktopManagerInternal);
    if (FAILED(hResult)) {
        return hResult;
    }
    pServiceProvider->Release();

    // get edge position
    RECT rect;
    rect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    rect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    rect.right = rect.left + GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1;
    rect.bottom = rect.top + GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1;
#ifdef _DEBUG
    Log(L"(%d, %d)-(%d, %d)",
        rect.left,
        rect.top,
        rect.right,
        rect.bottom);
#endif // _DEBUG
    l_nEdgeLeft = rect.left;
    l_nEdgeRight = rect.right;
    Log(L"left=%d, right=%d", l_nEdgeLeft, l_nEdgeRight);

    // load menu for tasktray
    LoadMenuTaskTray(hInstance);

    // add tasktray icon
    AddTaskTrayIcon(hInstance, hWnd);

    // set timer
    SetTimer(hWnd, ID_TIMER, l_nInterval, (TIMERPROC)NULL);

    return S_OK;
}

void Uninit(HWND hWnd) {

    // for virtual desktop api
    if (l_pVirtualDesktopManagerInternal) {
        l_pVirtualDesktopManagerInternal->Release();
    }

    // kill timer
    KillTimer(hWnd, ID_TIMER);

    // delete tasktray icon
    DeleteTaskTrayIcon(hWnd);

    // destroy menu for tasktray
    DestroyMenu(l_menuTaskTray);

}

// returns 0 if error
int VD_GetCount(void) {
    if (l_pVirtualDesktopManagerInternal) {
        UINT count;
        l_pVirtualDesktopManagerInternal->GetCount(&count);
        return count;
    }
    return 0;
}

//-- }


//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   //-- HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
   //--    CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    //-- {
#ifdef _DEBUG
    #define WINDOW_WIDTH CW_USEDEFAULT
    #define WINDOW_HEIGHT CW_USEDEFAULT
#else // _DEBUG
    #define WINDOW_WIDTH (320)
    #define WINDOW_HEIGHT (128)
#endif // _DEBUG
    HWND hWnd = CreateWindowExW(
                       WS_EX_TOOLWINDOW,
                       szWindowClass, szTitle,
                       WS_OVERLAPPED | WS_THICKFRAME,
                       CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
                       nullptr, nullptr, hInstance, nullptr);
    //-- }

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

    //-- {
    Init(hInstance, hWnd);

    l_bShow = false;
    ShowWindow(hWnd, SW_HIDE);
    //-- }

   return TRUE;
}

//-- {

VirtualDesktops::API::IVirtualDesktop* GetDesktopLeftRight(bool bLeft) {

    VirtualDesktops::API::IVirtualDesktop* pDesktop = nullptr;
    HRESULT hr = l_pVirtualDesktopManagerInternal->GetCurrentDesktop(&pDesktop);
    if (FAILED(hr)) {
        return NULL;
    }

    VirtualDesktops::API::IVirtualDesktop* pAdjacentDesktop = nullptr;
    hr = l_pVirtualDesktopManagerInternal->GetAdjacentDesktop(
        pDesktop,
        bLeft ? VirtualDesktops::API::AdjacentDesktop::LeftDirection : VirtualDesktops::API::AdjacentDesktop::RightDirection,
        &pAdjacentDesktop);

    pDesktop->Release();

    if (FAILED(hr)) {
        return NULL;
    }

    GUID guid;
    hr = pAdjacentDesktop->GetID(&guid);
    if (SUCCEEDED(hr)) {
        return pAdjacentDesktop;        // need to release
    }

    pAdjacentDesktop->Release();
    return NULL;
}


#define BIT_PRESSED (0x8000)

bool SwitchDesktopLeftRight(bool bLeft, POINT pointCursor) {

    VirtualDesktops::API::IVirtualDesktop* pAdjacentDesktop = GetDesktopLeftRight(bLeft);
    if (pAdjacentDesktop != NULL) {

        HWND hWndForeground = NULL;
        long lExStyle = 0;
        RECT rect;
        if (GetKeyState(VK_LBUTTON) & BIT_PRESSED) {
            hWndForeground = GetForegroundWindow();

            GetWindowRect(hWndForeground, &rect);
            #define WIDTH_FRAME (16)
            if (rect.left + WIDTH_FRAME < pointCursor.x
                    && rect.right - WIDTH_FRAME > pointCursor.x
                    && rect.top < pointCursor.y
                    && rect.bottom > pointCursor.y) {
                Log(L"hWndForeground=%08X", hWndForeground);
                lExStyle = GetWindowLong(hWndForeground, GWL_EXSTYLE);
                SetWindowLong(hWndForeground, GWL_EXSTYLE, WS_EX_TOOLWINDOW);   // change ex-style to move desktop
            }
            else {
                hWndForeground = NULL;
            }
        }

        // switch desktop
        l_pVirtualDesktopManagerInternal->SwitchDesktop(pAdjacentDesktop);
        pAdjacentDesktop->Release();

        // move cursor and window
        int nCursorX = bLeft ? (l_nEdgeRight - 1) : (l_nEdgeLeft + 1);
        int dx = pointCursor.x - nCursorX;

        if (hWndForeground != NULL) {
            SetWindowLong(hWndForeground, GWL_EXSTYLE, lExStyle);   // restore ex-style
            SetWindowPos(hWndForeground, HWND_TOP, rect.left - dx, rect.top, 0, 0, SWP_NOSIZE); 
        }
        SetCursorPos(nCursorX, pointCursor.y);


        return TRUE;
    }
    else {
        return FALSE;
    }
}


void wm_timer(HWND hWnd) {


    // draw logs
    if (l_nLogIndex != l_nLogIndexPrev) {
        InvalidateRect(hWnd, NULL, true);
        l_nLogIndexPrev = l_nLogIndex;
    }

    // check mouse button
    if (GetKeyState(VK_RBUTTON) & BIT_PRESSED
        || GetKeyState(VK_MBUTTON) & BIT_PRESSED) {
        l_nEdgeCount = 0;
        return;
    }

    // check shift key if needed
    if (l_bNeedShift && !(GetKeyState(VK_SHIFT) & BIT_PRESSED)) {
        l_nEdgeCount = 0;
        return;
    }

    // check control key if needed
    if (l_bNeedControl && !(GetKeyState(VK_CONTROL) & BIT_PRESSED)) {
        l_nEdgeCount = 0;
        return;
    }

    // get cursor position
    POINT pt;
    GetCursorPos(&pt);

    //Log(L"pt.x=%d", pt.x);




       // check if cursor is placed on the edge of the screen
    if (pt.x == l_nEdgeLeft || pt.x == l_nEdgeRight || pt.x == l_nEdgeRight + 1) {  // +1 is needed on several case
        ++l_nEdgeCount;

#ifdef _DEBUG
        Log(L"l_nEdgeCount=%d", l_nEdgeCount);
#endif // _DEBUG

        if (l_nEdgeCount >= l_nIntervalCount) {
            l_nEdgeCount = 0;
            if (pt.x == l_nEdgeLeft) {
                if (SwitchDesktopLeftRight(true, pt)) {
                    Log(L"switched to the left desktop");
                }
            }
            else /* if (pt.x == l_nEdgeRight || pt.x == l_nEdgeRight + 1) */ {
                if (SwitchDesktopLeftRight(false, pt)) {
                    Log(L"switched to the right desktop");
                }
            }
        }
    }
    else {
        l_nEdgeCount = 0;
    }


}

//-- }


//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...

            //-- {
            TEXTMETRIC tm;
            GetTextMetrics(hdc, &tm);
            int y = 0;
            for (int i = 0; i < MAX_LINE_LOG; ++i) {
                if (i == l_nLogIndex - 1 || (i == MAX_LINE_LOG - 1 && l_nLogIndex == 0)) {
                    SetTextColor(hdc, RGB(0x00, 0x00, 0x00));
                }
                else {
                    SetTextColor(hdc, RGB(0xa0, 0xa0, 0xa0));
                }
                TextOut(hdc, 0, y, l_szLog[i], _tcslen(l_szLog[i]));
                y += tm.tmHeight;
            }
            //-- }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        //-- {
        Uninit(hWnd);
        //-- }
        PostQuitMessage(0);
        break;
 
    //-- {
    case WM_TIMER:
        wm_timer(hWnd);
        break;

    case WM_TASKTRAY:
        if (wParam == ID_TRAYICON) {
            switch (lParam) {
            case WM_LBUTTONDBLCLK:
                if (l_bShow) {
                    l_bShow = false;
                    ShowWindow(hWnd, SW_HIDE);
                }
                else {
                    l_bShow = true;
                    ShowWindow(hWnd, SW_SHOW);
                    SetForegroundWindow(hWnd);
                    SetFocus(hWnd);
                }
                break;
            case WM_RBUTTONUP:
            {
                POINT point;
                GetCursorPos(&point);
                SetForegroundWindow(hWnd);
                SetFocus(hWnd);
                TrackPopupMenu(l_menuTaskTray, TPM_BOTTOMALIGN | TPM_RIGHTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, NULL);
            }
            break;
            default:
                break;
            }
        }
        break;
    //-- }


    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
