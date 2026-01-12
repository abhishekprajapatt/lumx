#pragma once

#include "..\include\Common.h"
#include "..\include\PerformanceController.h"
#include "..\include\DockModule.h"
#include "..\include\TopBarModule.h"
#include "..\include\WindowEffectsModule.h"
#include "..\include\ConfigManager.h"
#include "..\include\MonitorManager.h"
#include "..\include\EventManager.h"

namespace LumX
{
    class Application
    {
    public:
        Application();
        ~Application();

        HRESULT Initialize();
        void Shutdown();
        int Run();

    private:
        std::unique_ptr<PerformanceController> m_perfController;
        std::unique_ptr<DockModule> m_dockModule;
        std::unique_ptr<TopBarModule> m_topBarModule;
        std::unique_ptr<WindowEffectsModule> m_windowEffectsModule;

        HWND m_mainWindow;
        bool m_running;
        float m_deltaTime;
        std::chrono::steady_clock::time_point m_lastFrameTime;

        HRESULT RegisterWindowClass();
        HRESULT CreateMainWindow();
        void MessageLoop();
        void Update();
        void OnDisplaySettingsChange();

        static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static Application *GetInstance(HWND hwnd);
        LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

        HRESULT SetupWindowHooks();
        void CleanupWindowHooks();
    };
}
