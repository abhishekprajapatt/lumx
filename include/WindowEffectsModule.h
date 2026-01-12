#pragma once

#include "..\include\Common.h"

namespace LumX
{
    class WindowEffectsModule : public IModule
    {
    public:
        WindowEffectsModule();
        ~WindowEffectsModule();

        HRESULT Initialize() override;
        void Shutdown() override;
        void Update(float deltaTime) override;
        void OnMouseMove(const Point2D &pos) override;
        void OnMouseLeave() override;
        void OnDisplayChange() override;
        void OnDPIChange() override;
        void SetEnabled(bool enabled) override;
        bool IsEnabled() const override;

        void ApplyWindowEffects(HWND hwnd);
        void RemoveWindowEffects(HWND hwnd);

    private:
        struct WindowAnimation
        {
            HWND hwnd;
            float progress;
            bool minimizing;
            std::chrono::steady_clock::time_point startTime;
        };

        std::vector<WindowAnimation> m_activeAnimations;
        std::map<HWND, ComPtr<IDCompositionVisual>> m_visualMap;
        ComPtr<IDCompositionDevice> m_dcompDevice;
        ComPtr<IDCompositionTarget> m_dcompTarget;
        bool m_enabled;
        float m_dpi;
        std::mutex m_animationMutex;
        std::atomic<bool> m_dwmEnabled;

        HRESULT ApplyRoundedCorners(HWND hwnd);
        HRESULT ApplyShadow(HWND hwnd);
        HRESULT SetupWindowAnimation(HWND hwnd, bool minimizing);
        void UpdateAnimations(float deltaTime);
        void ApplyAnimationFrame(const WindowAnimation &anim);
        float EaseInOutCubic(float t);
    };
}
