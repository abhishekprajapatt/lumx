#pragma once

#include "..\include\Common.h"

namespace LumX
{
    class EventManager
    {
    public:
        static EventManager &GetInstance()
        {
            static EventManager instance;
            return instance;
        }

        EventManager(const EventManager &) = delete;
        EventManager &operator=(const EventManager &) = delete;

        using EventCallback = std::function<void(WPARAM, LPARAM)>;

        enum class EventType : UINT
        {
            MouseMove = 1,
            MouseLeave = 2,
            DisplayChange = 3,
            DPIChange = 4,
            FullscreenChange = 5,
            BatteryStatusChange = 6,
            AudioVolumeChange = 7,
            NetworkStatusChange = 8,
            ActiveWindowChange = 9,
            Custom = 100
        };

        HRESULT Initialize();
        void Shutdown();

        void Subscribe(EventType eventType, const EventCallback &callback);
        void Unsubscribe(EventType eventType, const EventCallback &callback);
        void Emit(EventType eventType, WPARAM wParam = 0, LPARAM lParam = 0);

    private:
        EventManager();
        ~EventManager();

        std::map<EventType, std::vector<EventCallback>> m_subscribers;
        std::mutex m_subscriberMutex;
    };
}
