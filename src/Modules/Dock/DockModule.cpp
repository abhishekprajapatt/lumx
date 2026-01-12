#include "..\include\DockModule.h"
#include <shellapi.h>
#include <shlwapi.h>
#include <psapi.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "psapi.lib")

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

namespace LumX
{
    static std::map<HWND, DockModule *> g_dockInstances;

    DockModule::DockModule(PerformanceController *perfController)
        : m_dockWindow(nullptr),
          m_dockPosition(0, 0),
          m_dockSize(0, 0),
          m_lastMousePos(0, 0),
          m_hideTimer(0),
          m_isHidden(false),
          m_isVisible(true),
          m_enabled(true),
          m_perfController(perfController),
          m_dpi(96.0f)
    {
    }

    DockModule::~DockModule()
    {
        Shutdown();
    }

    HRESULT DockModule::Initialize()
    {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = DockWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(DockModule *);
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = L"LumXDockWindow";

        if (!RegisterClassExW(&wc))
        {
            DWORD error = GetLastError();
            if (error != ERROR_CLASS_ALREADY_EXISTS)
            {
                return HRESULT_FROM_WIN32(error);
            }
        }

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        m_dockPosition = Point2D(screenWidth / 2.0f - 200, screenHeight - DOCK_HEIGHT - 10);
        m_dockSize = Size2D(400, DOCK_HEIGHT);

        m_dockWindow = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            L"LumXDockWindow",
            L"LumX Dock",
            WS_POPUP,
            static_cast<int>(m_dockPosition.x),
            static_cast<int>(m_dockPosition.y),
            static_cast<int>(m_dockSize.width),
            static_cast<int>(m_dockSize.height),
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

        if (!m_dockWindow)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        g_dockInstances[m_dockWindow] = this;
        SetWindowLongPtrW(m_dockWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        SetLayeredWindowAttributes(m_dockWindow, RGB(0, 0, 0), 0, LWA_COLORKEY | LWA_ALPHA);

        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_d2dFactory)))
        {
            return E_FAIL;
        }

        if (FAILED(CreateRenderTarget()))
        {
            return E_FAIL;
        }

        if (FAILED(LoadApplications()))
        {
            return E_FAIL;
        }

        ShowWindow(m_dockWindow, SW_SHOWNA);

        return S_OK;
    }

    void DockModule::Shutdown()
    {
        if (m_dockWindow)
        {
            g_dockInstances.erase(m_dockWindow);
            DestroyWindow(m_dockWindow);
            m_dockWindow = nullptr;
        }

        {
            std::lock_guard<std::mutex> lock(m_iconMutex);
            for (auto &icon : m_icons)
            {
                if (icon->iconHandle)
                {
                    DestroyIcon(icon->iconHandle);
                }
            }
            m_icons.clear();
        }

        m_renderTarget.Reset();
        m_d2dFactory.Reset();
    }

    LRESULT CALLBACK DockModule::DockWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        DockModule *pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCTW *pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
            pThis = reinterpret_cast<DockModule *>(pCreate->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = reinterpret_cast<DockModule *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            return pThis->HandleMessage(msg, wParam, lParam);
        }

        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

    LRESULT DockModule::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
            RenderDock();
            return 0;
        case WM_MOUSEMOVE:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            OnMouseMove(Point2D(static_cast<float>(x), static_cast<float>(y)));
            return 0;
        }
        case WM_MOUSELEAVE:
            OnMouseLeave();
            return 0;
        case WM_DISPLAYCHANGE:
            OnDisplayChange();
            return 0;
        case WM_DPICHANGED:
            OnDPIChange();
            return 0;
        }

        return DefWindowProcW(m_dockWindow, msg, wParam, lParam);
    }

    HRESULT DockModule::CreateRenderTarget()
    {
        if (!m_dockWindow || !m_d2dFactory)
        {
            return E_INVALID_STATE;
        }

        RECT clientRect;
        GetClientRect(m_dockWindow, &clientRect);

        D2D1_SIZE_U size = D2D1::SizeU(
            clientRect.right - clientRect.left,
            clientRect.bottom - clientRect.top);

        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps = D2D1::HwndRenderTargetProperties(m_dockWindow, size);

        return m_d2dFactory->CreateHwndRenderTarget(rtProps, hwndProps, &m_renderTarget);
    }

    HRESULT DockModule::LoadApplications()
    {
        m_icons.clear();

        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        {
            return E_FAIL;
        }

        wchar_t startMenuPath[MAX_PATH];
        DWORD pathSize = sizeof(startMenuPath);
        if (RegQueryValueExW(hKey, L"Programs", nullptr, nullptr, (LPBYTE)startMenuPath, &pathSize) == ERROR_SUCCESS)
        {
            std::wstring basePath(startMenuPath);

            WIN32_FIND_DATAW findData;
            HANDLE findHandle = FindFirstFileW((basePath + L"\\*.lnk").c_str(), &findData);

            if (findHandle != INVALID_HANDLE_VALUE)
            {
                int index = 0;
                do
                {
                    if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        auto icon = std::make_unique<DockIcon>();
                        icon->appName = findData.cFileName;
                        icon->appPath = basePath + L"\\" + findData.cFileName;
                        icon->index = index;
                        icon->scale = 1.0f;
                        icon->targetScale = 1.0f;
                        icon->pinned = true;
                        icon->appHwnd = nullptr;
                        icon->iconHandle = nullptr;

                        ExtractAppIcon(icon->appPath, icon->iconHandle);

                        m_icons.push_back(std::move(icon));
                        index++;

                        if (index >= 10)
                            break;
                    }
                } while (FindNextFileW(findHandle, &findData));

                FindClose(findHandle);
            }
        }

        RegCloseKey(hKey);

        UpdateIconPositions();
        return S_OK;
    }

    HRESULT DockModule::ExtractAppIcon(const std::wstring &appPath, HICON &icon)
    {
        SHFILEINFOW shFileInfo;
        DWORD_PTR result = SHGetFileInfoW(
            appPath.c_str(),
            0,
            &shFileInfo,
            sizeof(SHFILEINFOW),
            SHGFI_ICON | SHGFI_LARGEICON);

        if (result)
        {
            icon = shFileInfo.hIcon;
            return S_OK;
        }

        return E_FAIL;
    }

    void DockModule::Update(float deltaTime)
    {
        if (!m_enabled || !m_dockWindow)
        {
            return;
        }

        if (m_perfController && m_perfController->IsFullscreenAppActive())
        {
            if (!m_isHidden)
            {
                HideDock();
            }
            return;
        }

        m_hideTimer += deltaTime;
        if (m_hideTimer > (HIDE_TIMEOUT_MS / 1000.0f))
        {
            if (!m_isHidden)
            {
                HideDock();
            }
        }

        UpdateIconAnimations(deltaTime);
    }

    void DockModule::OnMouseMove(const Point2D &pos)
    {
        if (!m_enabled)
        {
            return;
        }

        m_lastMousePos = pos;

        if (IsMouseNearDock(pos))
        {
            m_hideTimer = 0;
            if (m_isHidden)
            {
                ShowDock();
            }
        }
    }

    void DockModule::OnMouseLeave()
    {
        if (!m_enabled)
        {
            return;
        }

        m_hideTimer = 0;
    }

    void DockModule::OnDisplayChange()
    {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        m_dockPosition = Point2D(screenWidth / 2.0f - 200, screenHeight - DOCK_HEIGHT - 10);
        UpdateDockPosition();
    }

    void DockModule::OnDPIChange()
    {
        m_dpi = static_cast<float>(GetDpiForWindow(m_dockWindow));
        if (m_renderTarget)
        {
            m_renderTarget.Reset();
            CreateRenderTarget();
        }
    }

    void DockModule::SetEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

    bool DockModule::IsEnabled() const
    {
        return m_enabled;
    }

    void DockModule::UpdateDockPosition()
    {
        if (m_dockWindow)
        {
            SetWindowPos(m_dockWindow,
                         HWND_TOPMOST,
                         static_cast<int>(m_dockPosition.x),
                         static_cast<int>(m_dockPosition.y),
                         static_cast<int>(m_dockSize.width),
                         static_cast<int>(m_dockSize.height),
                         SWP_NOACTIVATE | SWP_NOCOPYBITS);
        }
    }

    void DockModule::UpdateIconPositions()
    {
        std::lock_guard<std::mutex> lock(m_iconMutex);

        if (m_icons.empty())
        {
            return;
        }

        float totalWidth = 0;
        for (size_t i = 0; i < m_icons.size(); ++i)
        {
            totalWidth += DOCK_ICON_SIZE + DOCK_PADDING;
        }

        float startX = (m_dockSize.width - totalWidth) / 2.0f;
        float centerY = m_dockSize.height / 2.0f;

        for (size_t i = 0; i < m_icons.size(); ++i)
        {
            m_icons[i]->position = Point2D(
                startX + i * (DOCK_ICON_SIZE + DOCK_PADDING),
                centerY - DOCK_ICON_SIZE / 2.0f);
        }
    }

    void DockModule::UpdateIconAnimations(float deltaTime)
    {
        std::lock_guard<std::mutex> lock(m_iconMutex);

        for (auto &icon : m_icons)
        {
            float scale = CalculateZoomScale(m_lastMousePos, icon->index);
            icon->targetScale = scale;

            float diff = icon->targetScale - icon->scale;
            if (std::abs(diff) > 0.01f)
            {
                float animSpeed = 8.0f;
                if (m_perfController && m_perfController->ShouldReduceAnimations())
                {
                    animSpeed = 2.0f;
                }
                icon->scale += diff * animSpeed * deltaTime;
            }
            else
            {
                icon->scale = icon->targetScale;
            }
        }
    }

    void DockModule::ShowDock()
    {
        m_isHidden = false;
        if (m_dockWindow)
        {
            ShowWindow(m_dockWindow, SW_SHOWNA);
        }
    }

    void DockModule::HideDock()
    {
        m_isHidden = true;
        if (m_dockWindow)
        {
            ShowWindow(m_dockWindow, SW_HIDE);
        }
    }

    void DockModule::RenderDock()
    {
        if (!m_renderTarget)
        {
            return;
        }

        m_renderTarget->BeginDraw();
        m_renderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));

        std::lock_guard<std::mutex> lock(m_iconMutex);
        for (const auto &icon : m_icons)
        {
            Point2D scaledPos = icon->position;
            float scaledSize = DOCK_ICON_SIZE * icon->scale;
            float offsetX = (scaledSize - DOCK_ICON_SIZE) / 2.0f;
            float offsetY = (scaledSize - DOCK_ICON_SIZE) / 2.0f;

            D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                D2D1::Point2F(icon->position.x + DOCK_ICON_SIZE / 2.0f, icon->position.y + DOCK_ICON_SIZE / 2.0f),
                scaledSize / 2.0f,
                scaledSize / 2.0f);
        }

        m_renderTarget->EndDraw();
    }

    void DockModule::RefreshIconLayout()
    {
        UpdateIconPositions();
    }

    Point2D DockModule::CalculateIconPosition(int index, float scale)
    {
        if (index < 0 || index >= static_cast<int>(m_icons.size()))
        {
            return Point2D(0, 0);
        }
        return m_icons[index]->position;
    }

    float DockModule::CalculateZoomScale(const Point2D &mousePos, int iconIndex)
    {
        if (iconIndex < 0 || iconIndex >= static_cast<int>(m_icons.size()))
        {
            return 1.0f;
        }

        const auto &icon = m_icons[iconIndex];
        Point2D iconCenter = icon->position + Point2D(DOCK_ICON_SIZE / 2.0f, DOCK_ICON_SIZE / 2.0f);
        float distance = mousePos.distance(iconCenter);

        float maxDistance = DOCK_ICON_SIZE * 3.0f;
        if (distance > maxDistance)
        {
            return 1.0f;
        }

        float scale = 1.0f + (ZOOM_SCALE - 1.0f) * (1.0f - distance / maxDistance);
        return scale;
    }

    bool DockModule::IsMouseNearDock(const Point2D &pos)
    {
        float dockTop = m_dockPosition.y;
        float proximityZone = 100.0f;
        return pos.y > (dockTop - proximityZone);
    }

    void DockModule::RefreshApplicationList()
    {
        LoadApplications();
    }
}
