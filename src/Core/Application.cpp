#include "..\include\Application.h"
#include <iostream>
#include <shellapi.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

namespace LumX
{
    static std::map<HWND, Application *> g_appInstances;

    Application::Application()
        : m_mainWindow(nullptr),
          m_running(false),
          m_deltaTime(0.0f)
    {
    }

    Application::~Application()
    {
        Shutdown();
    }

    HRESULT Application::Initialize()
    {
        if (FAILED(RegisterWindowClass()))
        {
            return E_FAIL;
        }

        m_perfController = std::make_unique<PerformanceController>();
        if (FAILED(m_perfController->Initialize()))
        {
            return E_FAIL;
        }

        m_dockModule = std::make_unique<DockModule>(m_perfController.get());
        if (FAILED(m_dockModule->Initialize()))
        {
            return E_FAIL;
        }

        m_topBarModule = std::make_unique<TopBarModule>(m_perfController.get());
        if (FAILED(m_topBarModule->Initialize()))
        {
            return E_FAIL;
        }

        m_windowEffectsModule = std::make_unique<WindowEffectsModule>();
        if (FAILED(m_windowEffectsModule->Initialize()))
        {
            return E_FAIL;
        }

        if (FAILED(CreateMainWindow()))
        {
            return E_FAIL;
        }

        if (FAILED(SetupWindowHooks()))
        {
            return E_FAIL;
        }

        m_lastFrameTime = std::chrono::steady_clock::now();
        m_running = true;

        return S_OK;
    }

    void Application::Shutdown()
    {
        m_running = false;

        CleanupWindowHooks();

        if (m_windowEffectsModule)
        {
            m_windowEffectsModule->Shutdown();
        }

        if (m_topBarModule)
        {
            m_topBarModule->Shutdown();
        }

        if (m_dockModule)
        {
            m_dockModule->Shutdown();
        }

        if (m_perfController)
        {
            m_perfController->Shutdown();
        }

        if (m_mainWindow)
        {
            DestroyWindow(m_mainWindow);
            m_mainWindow = nullptr;
        }

        g_appInstances.clear();
    }

    int Application::Run()
    {
        MSG msg = {};
        while (m_running)
        {
            if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    m_running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            else
            {
                Update();
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        }

        return static_cast<int>(msg.wParam);
    }

    HRESULT Application::RegisterWindowClass()
    {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = MainWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(Application *);
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = L"LumXMainWindow";

        if (!RegisterClassExW(&wc))
        {
            DWORD error = GetLastError();
            if (error != ERROR_CLASS_ALREADY_EXISTS)
            {
                return HRESULT_FROM_WIN32(error);
            }
        }

        return S_OK;
    }

    HRESULT Application::CreateMainWindow()
    {
        m_mainWindow = CreateWindowExW(
            0,
            L"LumXMainWindow",
            L"LumX",
            WS_POPUP,
            0,
            0,
            1,
            1,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

        if (!m_mainWindow)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        g_appInstances[m_mainWindow] = this;
        SetWindowLongPtrW(m_mainWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        return S_OK;
    }

    LRESULT CALLBACK Application::MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Application *pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCTW *pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
            pThis = reinterpret_cast<Application *>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = reinterpret_cast<Application *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            return pThis->HandleMessage(msg, wParam, lParam);
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    Application *Application::GetInstance(HWND hwnd)
    {
        auto it = g_appInstances.find(hwnd);
        if (it != g_appInstances.end())
        {
            return it->second;
        }
        return nullptr;
    }

    LRESULT Application::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_MOUSEMOVE:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            Point2D pos(static_cast<float>(x), static_cast<float>(y));

            if (m_dockModule)
            {
                m_dockModule->OnMouseMove(pos);
            }
            if (m_topBarModule)
            {
                m_topBarModule->OnMouseMove(pos);
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            if (m_dockModule)
            {
                m_dockModule->OnMouseLeave();
            }
            if (m_topBarModule)
            {
                m_topBarModule->OnMouseLeave();
            }
            return 0;
        case WM_DISPLAYCHANGE:
            OnDisplaySettingsChange();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(m_mainWindow, msg, wParam, lParam);
    }

    void Application::Update()
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFrameTime);
        m_deltaTime = duration.count() / 1000.0f;
        m_lastFrameTime = now;

        if (m_perfController)
        {
            m_perfController->Update(m_deltaTime);
        }

        if (m_dockModule && m_dockModule->IsEnabled())
        {
            m_dockModule->Update(m_deltaTime);
        }

        if (m_topBarModule && m_topBarModule->IsEnabled())
        {
            m_topBarModule->Update(m_deltaTime);
        }

        if (m_windowEffectsModule && m_windowEffectsModule->IsEnabled())
        {
            m_windowEffectsModule->Update(m_deltaTime);
        }
    }

    void Application::OnDisplaySettingsChange()
    {
        if (m_dockModule)
        {
            m_dockModule->OnDisplayChange();
        }
        if (m_topBarModule)
        {
            m_topBarModule->OnDisplayChange();
        }
    }

    HRESULT Application::SetupWindowHooks()
    {
        HWND mouseHook = CreateWindowExW(
            WS_EX_TRANSPARENT,
            L"LumXMainWindow",
            L"LumX Mouse Hook",
            WS_POPUP,
            0, 0, 1, 1,
            m_mainWindow,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

        return S_OK;
    }

    void Application::CleanupWindowHooks()
    {
    }
}

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
