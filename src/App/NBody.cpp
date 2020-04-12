//
// Main.cpp
//

#include "App/AppCore.hpp"
#include "App/App.hpp"
#include "Services/Log.hpp"

#include <imgui.h>
#include <cxxopts.hpp>
#include <shellapi.h>

#include <Mouse.h>
#include <Keyboard.h>

namespace
{
    std::unique_ptr<App> g_app;
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bool showCmd = true;

#ifdef _DEBUG
    showCmd = true;
#endif

    int num;
    auto cmd = CommandLineToArgvW(GetCommandLineW(), &num);
    char** argv = new char*[num];

    for(int i = 0; i < num; ++i)
    {
        int size = WideCharToMultiByte(CP_ACP, 0, cmd[i], -1, NULL, 0, NULL, NULL);
        argv[i] = new char[size];
        WideCharToMultiByte(CP_ACP, 0, cmd[i], -1, argv[i], size, NULL, NULL);
        LOGM(argv[i])
    }

    cxxopts::Options options("NBody Simulator", "Simulate gravitational physics");

    bool compute = false;
    int simtime = 10, particles = 4000;
    float timestep = 0.02f;
    std::string file = "";

    options.add_options()
        ("c,compute", "Precompute a simulation", cxxopts::value<bool>(compute))
        ("t,simtime", "Time to run the precomputed simulation for", cxxopts::value<int>(simtime))
        ("s,timestep", "Timestep", cxxopts::value<float>(timestep))
        ("p,particles", "Number of particles", cxxopts::value<int>(particles))
        ("f,file", "Load previous computation", cxxopts::value<std::string>(file));
    
    auto result = options.parse(num, argv);

    for(int i = 0; i < num; ++i)
        delete[] argv[i];

    delete[] argv;

    if(result.count("compute") > 0)
        showCmd = true;

    if(showCmd)
    {
        AllocConsole();

        FILE *fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        std::cout.clear();
    }

    if(result.count("compute") > 0)
    {
        App::RunSimulation(timestep * (1.0f / 60.0f), simtime, particles, file);
        return 0;
    }

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

    if (FAILED(hr))
        return 1;

    try
    {
        g_app = std::make_unique<App>();
    }
    catch (std::exception ex)
    {
        MessageBoxA(nullptr, ex.what(), "Error", MB_OK | MB_ICONEXCLAMATION);
    }

    // Register class and create window
    {
        // Register class
        WNDCLASSEXW wcex = {};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wcex.lpszClassName = L"NBodyWindowClass";
        wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
        if (!RegisterClassExW(&wcex))
            return 1;

        // Create window
        int w, h;
        g_app->GetDefaultSize(w, h);

        RECT rc = { 0, 0, static_cast<LONG>(w), static_cast<LONG>(h) };

        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        HWND hwnd = CreateWindowExW(0, L"NBodyWindowClass", L"Procedural Universe", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);

        // TODO: Change to CreateWindowExW(WS_EX_TOPMOST, L"NBodyWindowClass", L"Procedural Universe", WS_POPUP,
        // to default to fullscreen.

        if (!hwnd)
            return 1;

        ShowWindow(hwnd, nCmdShow);
        // TODO: Change nCmdShow to SW_SHOWMAXIMIZED to default to fullscreen.

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g_app.get()) );

        GetClientRect(hwnd, &rc);
        
        try
        {
            g_app->Initialize(hwnd, rc.right - rc.left, rc.bottom - rc.top);
        }
        catch (const std::exception& ex)
        {
            MessageBoxA(hwnd, ex.what(), "Error", MB_OK | MB_ICONEXCLAMATION);
            return 1;
        }
    }

    // Main message loop
    MSG msg = {};

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            g_app->Tick();
        }
    }

    g_app.reset();

    CoUninitialize();

    return (int) msg.wParam;
}

// Windows procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;
    static bool s_fullscreen = false;
    // TODO: Set s_fullscreen to true if defaulting to fullscreen.

    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    auto game = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_PAINT:
        if (s_in_sizemove && game)
        {
            game->Tick();
        }
        else
        {
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_MOVE:
        if (game)
        {
            game->OnWindowMoved();
        }
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (!s_in_suspend && game)
                    game->OnSuspending();
                s_in_suspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (s_in_suspend && game)
                game->OnResuming();
            s_in_suspend = false;
        }
        else if (!s_in_sizemove && game)
        {
            game->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
        }
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        if (game)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            game->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
        }
        break;

    case WM_GETMINMAXINFO:
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;

    case WM_ACTIVATEAPP:
        if (game)
        {
            if (wParam)
            {
                game->OnActivated();
            }
            else
            {
                game->OnDeactivated();
            }
        }

        DirectX::Mouse::ProcessMessage(message, wParam, lParam);
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
        
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend && game)
                game->OnSuspending();
            s_in_suspend = true;
            return TRUE;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (s_in_suspend && game)
                    game->OnResuming();
                s_in_suspend = false;
            }
            return TRUE;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_SYSKEYDOWN:
        if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        {
            // Implements the classic ALT+ENTER fullscreen toggle
            if (s_fullscreen)
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

                int width = 800;
                int height = 600;
                if (game)
                    game->GetDefaultSize(width, height);

                ShowWindow(hWnd, SW_SHOWNORMAL);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
            else
            {
                SetWindowLongPtr(hWnd, GWL_STYLE, 0);
                SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

                SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                ShowWindow(hWnd, SW_SHOWMAXIMIZED);
            }

            s_fullscreen = !s_fullscreen;
        }

        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);

        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
        break;

    case WM_MENUCHAR:
        // A menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);

    case WM_INPUT:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_MOUSEHOVER:
        DirectX::Mouse::ProcessMessage(message, wParam, lParam);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}