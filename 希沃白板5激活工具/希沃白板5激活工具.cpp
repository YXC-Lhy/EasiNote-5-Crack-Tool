// 希沃白板5激活工具.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "希沃白板5激活工具.h"

#include "path.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_LOADSTRING 100
#define IDM_crack 2001
#define IDM_lnk 2002
#define IDM_1 2003
#define IDM_2 2004

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 获取屏幕尺寸
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
// 计算窗口位置使其居中
int windowWidth = 455;
int windowHeight = 245;
int posX = (screenWidth - windowWidth) / 2;
int posY = (screenHeight - windowHeight) / 2;

// 此代码模块中包含的函数的前向声明:
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

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY5, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY5));

    MSG msg;

    // 主消息循环:
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
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY5));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY5);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MY5));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   //HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      //CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   HWND hWnd = CreateWindowEx(
       NULL, szWindowClass, L"希沃白板5激活工具 v1.1.1",
       WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX ,
       posX, posY, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   hWnd111 = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        {
            // 定义字体属性
            LOGFONT lf;
            ZeroMemory(&lf, sizeof(LOGFONT));
            lstrcpy(lf.lfFaceName, L"Microsoft YaHei"); // 设置字体名称
            //lf.lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72); // 设置字体大小12点
            lf.lfWeight = FW_NORMAL; // 设置字体粗细
            // 创建字体
            HFONT hFont = CreateFontIndirect(&lf);

            HDC hdc = GetDC(NULL); // 获取整个屏幕的设备上下文
            int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
            // 定义字体属性
            LOGFONT lf2;
            ZeroMemory(&lf2, sizeof(LOGFONT));
            lstrcpy(lf2.lfFaceName, L"Microsoft YaHei"); // 设置字体名称
            lf2.lfHeight = -MulDiv(18, GetDeviceCaps(hdc, LOGPIXELSY), 72); // 设置字体大小18点
            lf2.lfWeight = FW_NORMAL; // 设置字体粗细
            // 创建字体
            HFONT bFont = CreateFontIndirect(&lf2);

            HWND Static = CreateWindow(L"STATIC",
                L"希沃白板 5",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                100, 28, 160, 50, hWnd, (HMENU)NULL,
                ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(Static, WM_SETFONT, (WPARAM)bFont, MAKELPARAM(TRUE, 0));
            HWND Static1 = CreateWindow(L"STATIC",
                L"版本号:\n安装路径:",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                20, 88, 100, 70, hWnd, (HMENU)NULL,
                ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(Static1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            path = FindEasiNote5InstallPath();//获取路径
            if (!path.empty()) {
                HWND Static2 = CreateWindowA("STATIC",
                    path.c_str(),
                    WS_CHILD | WS_VISIBLE | SS_LEFT,
                    97, 110, 600, 30, hWnd, (HMENU)NULL,
                    ((LPCREATESTRUCT)lParam)->hInstance, NULL);
                SendMessage(Static2, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            }
            else {
                MessageBox(hWnd111, L"未找到希沃白板5的安装路径。", NULL, NULL);
            }
            latest = GetLatestEasiNoteVersion();//获取版本号
            if (!latest.empty()) {
                HWND Static3 = CreateWindowA("STATIC",
                    latest.c_str(),
                    WS_CHILD | WS_VISIBLE | SS_LEFT,
                    83, 88, 600, 30, hWnd, (HMENU)NULL,
                    ((LPCREATESTRUCT)lParam)->hInstance, NULL);
                SendMessage(Static3, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            }
            else {
                HWND Static3 = CreateWindowA("STATIC",
                    "未找到任何版本。",
                    WS_CHILD | WS_VISIBLE | SS_LEFT,
                    83, 88, 600, 30, hWnd, (HMENU)NULL,
                    ((LPCREATESTRUCT)lParam)->hInstance, NULL);
                SendMessage(Static3, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            }
            HWND button = CreateWindow(L"BUTTON", L"激活\n专业版",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE/*换行样式*/ | BS_DEFPUSHBUTTON,
                240, 10, 80, 67, hWnd, (HMENU)IDM_crack,
                ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(button, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND button1 = CreateWindow(L"BUTTON", L"关于\n本工具",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE/*换行样式*/ ,
                335, 10, 80, 67, hWnd, (HMENU)IDM_ABOUT,
                ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(button1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND button2 = CreateWindow(L"BUTTON", L"修改桌面快捷方式\n为免登录打开白板",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE/*换行样式*/,
                20, 137, 161, 60, hWnd, (HMENU)IDM_lnk,
                ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(button2, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND button01 = CreateWindow(L"BUTTON", L"修改启动时\n显示的图片",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE/*换行样式*/,
                188, 137, 110, 60, hWnd, (HMENU)IDM_1,
                ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(button01, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            HWND button02 = CreateWindow(L"BUTTON", L"还原启动时\n显示的图片",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE/*换行样式*/,
                305, 137, 110, 60, hWnd, (HMENU)IDM_2,
                ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(button02, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
        }
    case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC)wParam;
            SetTextColor(hdcStatic, RGB(0, 0, 0)); // 设置文本颜色为黑色
            SetBkMode(hdcStatic, TRANSPARENT); // 设置背景模式为透明
            return (LONG)GetStockObject(HOLLOW_BRUSH);// 返回空画刷，以确保不绘制背景
        }
    case WM_NCLBUTTONDBLCLK: // 双击非客户区（例如标题栏）
        {
            // 如果双击的是标题栏，则返回0以阻止默认行为
            //if (wParam == HTCAPTION)
            {
                return 0;
            }
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                system("start https://github.com/YXC-Lhy/EasiNote-5-Crack-Tool");
                break;
            case IDM_crack:
                if (IsEasiNoteRunning())
                {
                    MessageBoxW(hWnd111, L"希沃白板5正在运行！\r\n请彻底关闭软件后再激活。", L"提示", MB_OK | MB_ICONINFORMATION);
                }
                else
                {
                    crack();
                }
                break;
            case IDM_lnk:
                CreateEasiNoteShortcut();
                break;
            case IDM_1:
                ReplaceBannerImage();
                break;
            case IDM_2:
                ReplaceBannerImageFromResource();
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
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            // 
            // 从资源加载图标
            HICON hIcon = (HICON)LoadImage(
                GetModuleHandle(NULL),         // 当前程序模块句柄
                MAKEINTRESOURCE(IDI_SEEWO),  // 图标资源ID
                IMAGE_ICON,                    // 加载图标
                64, 64,                        // 图标大小（可改）
                0                              // 无特殊标志
            );
            // 绘制图标
            if (hIcon)
                DrawIconEx(hdc, 25, 12, hIcon, 64, 64, 0, NULL, DI_NORMAL);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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
