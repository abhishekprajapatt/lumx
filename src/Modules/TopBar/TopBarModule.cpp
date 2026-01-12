#include "..\include\TopBarModule.h"
#include <d2d1.h>
#include <dwrite.h>
#include <wininet.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "user32.lib")

namespace LumX
{
    static std::map<HWND, TopBarModule *> g_topBarInstances;

    TopBarModule::TopBarModule(PerformanceController *perfController)
        : m_topBarWindow(nullptr),
          m_topBarPosition(0, 0),
          m_topBarSize(0, TOPBAR_HEIGHT),
          m_enabled(true),
          m_perfController(perfController),
          m_dpi(96.0f),
          m_updateTimer(0)
    {
        ZeroMemory(&m_systemStatus, sizeof(m_systemStatus));
    }

    TopBarModule::~TopBarModule()
    {
        Shutdown();
    }

    HRESULT TopBarModule::Initialize()
    {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = TopBarWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(TopBarModule *);
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = L"LumXTopBarWindow";

        if (!RegisterClassExW(&wc))
        {
            DWORD error = GetLastError();
            if (error != ERROR_CLASS_ALREADY_EXISTS)
            {
                return HRESULT_FROM_WIN32(error);
            }
        }

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        m_topBarSize = Size2D(static_cast<float>(screenWidth), TOPBAR_HEIGHT);

        m_topBarWindow = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            L"LumXTopBarWindow",
            L"LumX TopBar",
            WS_POPUP,
            0,
            0,
            screenWidth,
            TOPBAR_HEIGHT,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

        if (!m_topBarWindow)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        g_topBarInstances[m_topBarWindow] = this;
        SetWindowLongPtrW(m_topBarWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        SetLayeredWindowAttributes(m_topBarWindow, RGB(0, 0, 0), 200, LWA_COLORKEY | LWA_ALPHA);

        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_d2dFactory)))
        {
            return E_FAIL;
        }

        if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&m_dwFactory)))
        {
            return E_FAIL;
        }

        if (FAILED(m_dwFactory->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                12.0f,
                L"en-us",
                &m_textFormat)))
        {
            return E_FAIL;
        }

        if (FAILED(CreateRenderTarget()))
        {
            return E_FAIL;
        }

        ShowWindow(m_topBarWindow, SW_SHOWNA);

        return S_OK;
    }

    void TopBarModule::Shutdown()
    {
        if (m_topBarWindow)
        {
            g_topBarInstances.erase(m_topBarWindow);
            DestroyWindow(m_topBarWindow);
            m_topBarWindow = nullptr;
        }

        m_renderTarget.Reset();
        m_d2dFactory.Reset();
        m_dwFactory.Reset();
    }

    LRESULT CALLBACK TopBarModule::TopBarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        TopBarModule *pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCTW *pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
            pThis = reinterpret_cast<TopBarModule *>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = reinterpret_cast<TopBarModule *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            return pThis->HandleMessage(msg, wParam, lParam);
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT TopBarModule::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_DESTROY:
            return 0;
        case WM_PAINT:
            RenderTopBar();
            return 0;
        case WM_DISPLAYCHANGE:
            OnDisplayChange();
            return 0;
        case WM_DPICHANGED:
            OnDPIChange();
            return 0;
        }

        return DefWindowProcW(m_topBarWindow, msg, wParam, lParam);
    }

    HRESULT TopBarModule::CreateRenderTarget()
    {
        if (!m_topBarWindow || !m_d2dFactory)
        {
            return E_INVALID_STATE;
        }

        RECT clientRect;
        GetClientRect(m_topBarWindow, &clientRect);

        D2D1_SIZE_U size = D2D1::SizeU(
            clientRect.right - clientRect.left,
            clientRect.bottom - clientRect.top);

        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(m_topBarWindow, size);

        return m_d2dFactory->CreateHwndRenderTarget(rtProps, hwndProps, &m_renderTarget);
    }

    void TopBarModule::Update(float deltaTime)
    {
        if (!m_enabled)
        {
            return;
        }

        m_updateTimer += deltaTime;
        if (m_updateTimer >= 0.5f)
        {
            UpdateSystemStatus();
            m_updateTimer = 0;
        }
    }

    void TopBarModule::OnMouseMove(const Point2D &pos)
    {
    }

    void TopBarModule::OnMouseLeave()
    {
    }

    void TopBarModule::OnDisplayChange()
    {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        m_topBarSize = Size2D(static_cast<float>(screenWidth), TOPBAR_HEIGHT);
        UpdateTopBarPosition();
    }

    void TopBarModule::OnDPIChange()
    {
        m_dpi = static_cast<float>(GetDpiForWindow(m_topBarWindow));
        if (m_renderTarget)
        {
            m_renderTarget.Reset();
            CreateRenderTarget();
        }
    }

    void TopBarModule::SetEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

    bool TopBarModule::IsEnabled() const
    {
        return m_enabled;
    }

    void TopBarModule::UpdateSystemStatus()
    {
        {
            std::lock_guard<std::mutex> lock(m_statusMutex);
            m_systemStatus.activeAppName = GetActiveWindowName();
            m_systemStatus.timeString = FormatTime();
            QueryBatteryStatus();
            QueryNetworkStatus();
            QueryAudioStatus();
        }

        RenderTopBar();
    }

    void TopBarModule::RenderTopBar()
    {
        if (!m_renderTarget)
        {
            return;
        }

        m_renderTarget->BeginDraw();
        m_renderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));

        {
            std::lock_guard<std::mutex> lock(m_statusMutex);

            D2D1_RECT_F layoutRect = D2D1::RectF(10, 5, m_topBarSize.width - 10, TOPBAR_HEIGHT - 5);

            if (m_dwFactory && m_textFormat)
            {
                ComPtr<IDWriteTextLayout> timeLayout;
                if (SUCCEEDED(m_dwFactory->CreateTextLayout(
                        m_systemStatus.timeString.c_str(),
                        m_systemStatus.timeString.length(),
                        m_textFormat.Get(),
                        m_topBarSize.width,
                        TOPBAR_HEIGHT,
                        &timeLayout)))
                {
                    if (m_renderTarget && m_textBrush)
                    {
                        m_renderTarget->DrawTextLayout(
                            D2D1::Point2F(m_topBarSize.width - 100, 5),
                            timeLayout.Get(),
                            m_textBrush.Get());
                    }
                }
            }
        }

        m_renderTarget->EndDraw();
    }

    void TopBarModule::UpdateTopBarPosition()
    {
        if (m_topBarWindow)
        {
            SetWindowPos(m_topBarWindow,
                         HWND_TOPMOST,
                         0,
                         0,
                         static_cast<int>(m_topBarSize.width),
                         TOPBAR_HEIGHT,
                         SWP_NOACTIVATE | SWP_NOCOPYBITS);
        }
    }

    std::wstring TopBarModule::GetActiveWindowName()
    {
        HWND activeWindow = GetForegroundWindow();
        if (!activeWindow)
        {
            return L"Finder";
        }

        wchar_t windowTitle[256] = {};
        GetWindowTextW(activeWindow, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));

        return windowTitle;
    }

    HRESULT TopBarModule::QueryBatteryStatus()
    {
        SYSTEM_POWER_STATUS powerStatus;
        if (!GetSystemPowerStatus(&powerStatus))
        {
            return E_FAIL;
        }

        m_systemStatus.batteryPercent = powerStatus.BatteryLifePercent;
        m_systemStatus.batteryCharging = (powerStatus.BatteryFlag & 8) != 0;

        return S_OK;
    }

    HRESULT TopBarModule::QueryNetworkStatus()
    {
        DWORD flags = 0;
        if (InternetGetConnectedState(&flags, 0))
        {
            m_systemStatus.wifiConnected = (flags & INTERNET_CONNECTION_CONFIGURED) != 0;
        }
        else
        {
            m_systemStatus.wifiConnected = false;
        }

        return S_OK;
    }

    HRESULT TopBarModule::QueryAudioStatus()
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        ComPtr<IMMDeviceEnumerator> enumerator;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&enumerator));
        if (SUCCEEDED(hr))
        {
            ComPtr<IMMDevice> device;
            hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
            if (SUCCEEDED(hr))
            {
                ComPtr<IAudioEndpointVolume> volume;
                hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (void **)&volume);
                if (SUCCEEDED(hr))
                {
                    float fLevel = 0;
                    volume->GetMasterVolumeLevelScalar(&fLevel);
                    m_systemStatus.volumePercent = static_cast<int>(fLevel * 100);

                    BOOL muted = false;
                    volume->GetMute(&muted);
                    m_systemStatus.soundMuted = (muted != 0);
                }
            }
        }

        CoUninitialize();
        return S_OK;
    }

    std::wstring TopBarModule::FormatTime()
    {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        struct tm timeinfo;
        localtime_s(&timeinfo, &time_t);

        wchar_t buffer[16];
        wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%H:%M", &timeinfo);

        return buffer;
    }
}
