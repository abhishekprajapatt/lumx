#include "..\include\WindowEffectsModule.h"
#include <dwmapi.h>
#include <math.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "user32.lib")

namespace LumX
{
    WindowEffectsModule::WindowEffectsModule()
        : m_enabled(true),
          m_dpi(96.0f),
          m_dwmEnabled(true)
    {
    }

    WindowEffectsModule::~WindowEffectsModule()
    {
        Shutdown();
    }

    HRESULT WindowEffectsModule::Initialize()
    {
        DWORD dwmEnabled = 0;
        if (SUCCEEDED(DwmIsCompositionEnabled(&dwmEnabled)) && dwmEnabled)
        {
            m_dwmEnabled = true;
        }

        return S_OK;
    }

    void WindowEffectsModule::Shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(m_animationMutex);
            m_activeAnimations.clear();
            m_visualMap.clear();
        }
        m_dcompDevice.Reset();
        m_dcompTarget.Reset();
    }

    void WindowEffectsModule::Update(float deltaTime)
    {
        UpdateAnimations(deltaTime);
    }

    void WindowEffectsModule::OnMouseMove(const Point2D &pos)
    {
    }

    void WindowEffectsModule::OnMouseLeave()
    {
    }

    void WindowEffectsModule::OnDisplayChange()
    {
    }

    void WindowEffectsModule::OnDPIChange()
    {
    }

    void WindowEffectsModule::SetEnabled(bool enabled)
    {
        m_enabled = enabled;
    }

    bool WindowEffectsModule::IsEnabled() const
    {
        return m_enabled;
    }

    void WindowEffectsModule::ApplyWindowEffects(HWND hwnd)
    {
        if (!m_enabled || !m_dwmEnabled)
        {
            return;
        }

        ApplyRoundedCorners(hwnd);
        ApplyShadow(hwnd);
    }

    void WindowEffectsModule::RemoveWindowEffects(HWND hwnd)
    {
        std::lock_guard<std::mutex> lock(m_animationMutex);
        m_visualMap.erase(hwnd);
    }

    HRESULT WindowEffectsModule::ApplyRoundedCorners(HWND hwnd)
    {
        if (!m_dwmEnabled)
        {
            return E_FAIL;
        }

        HRGN hrgn = CreateRoundRectRgn(0, 0, 500, 500, 8, 8);
        if (!hrgn)
        {
            return E_FAIL;
        }

        DWM_WINDOW_CORNER_PREFERENCE corner = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));

        return S_OK;
    }

    HRESULT WindowEffectsModule::ApplyShadow(HWND hwnd)
    {
        if (!m_dwmEnabled)
        {
            return E_FAIL;
        }

        DWORD MARGINS = MARGINS{-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(hwnd, &MARGINS);

        return S_OK;
    }

    HRESULT WindowEffectsModule::SetupWindowAnimation(HWND hwnd, bool minimizing)
    {
        if (!m_enabled)
        {
            return S_OK;
        }

        std::lock_guard<std::mutex> lock(m_animationMutex);

        WindowAnimation anim;
        anim.hwnd = hwnd;
        anim.progress = 0.0f;
        anim.minimizing = minimizing;
        anim.startTime = std::chrono::steady_clock::now();

        m_activeAnimations.push_back(anim);

        return S_OK;
    }

    void WindowEffectsModule::UpdateAnimations(float deltaTime)
    {
        std::lock_guard<std::mutex> lock(m_animationMutex);

        auto now = std::chrono::steady_clock::now();

        for (auto &anim : m_activeAnimations)
        {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - anim.startTime).count();
            anim.progress = std::min(1.0f, static_cast<float>(elapsed) / ANIMATION_DURATION_MS);

            ApplyAnimationFrame(anim);

            if (anim.progress >= 1.0f)
            {
                anim.progress = 1.0f;
            }
        }

        m_activeAnimations.erase(
            std::remove_if(m_activeAnimations.begin(), m_activeAnimations.end(),
                           [](const WindowAnimation &anim)
                           { return anim.progress >= 1.0f; }),
            m_activeAnimations.end());
    }

    void WindowEffectsModule::ApplyAnimationFrame(const WindowAnimation &anim)
    {
        float easeProgress = EaseInOutCubic(anim.progress);

        RECT windowRect;
        if (GetWindowRect(anim.hwnd, &windowRect))
        {
            int width = windowRect.right - windowRect.left;
            int height = windowRect.bottom - windowRect.top;

            if (anim.minimizing)
            {
                float scale = 1.0f - (easeProgress * 0.3f);
                float opacityFade = 1.0f - easeProgress;

                SetWindowPos(anim.hwnd, nullptr,
                             windowRect.left + static_cast<int>(width * (1 - scale) / 2),
                             windowRect.top + static_cast<int>(height * (1 - scale) / 2),
                             static_cast<int>(width * scale),
                             static_cast<int>(height * scale),
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
            else
            {
                float scale = 0.7f + (easeProgress * 0.3f);
                float opacityGain = easeProgress;

                SetWindowPos(anim.hwnd, nullptr,
                             windowRect.left + static_cast<int>(width * (1 - scale) / 2),
                             windowRect.top + static_cast<int>(height * (1 - scale) / 2),
                             static_cast<int>(width * scale),
                             static_cast<int>(height * scale),
                             SWP_NOZORDER | SWP_NOACTIVATE);
            }
        }
    }

    float WindowEffectsModule::EaseInOutCubic(float t)
    {
        if (t < 0.5f)
        {
            return 4.0f * t * t * t;
        }
        else
        {
            float f = 2.0f * t - 2.0f;
            return 0.5f * f * f * f + 1.0f;
        }
    }
}
