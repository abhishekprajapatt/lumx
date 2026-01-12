#include "..\include\EventManager.h"

namespace LumX
{
    EventManager::EventManager()
    {
    }

    EventManager::~EventManager()
    {
    }

    HRESULT EventManager::Initialize()
    {
        return S_OK;
    }

    void EventManager::Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_subscriberMutex);
        m_subscribers.clear();
    }

    void EventManager::Subscribe(EventType eventType, const EventCallback &callback)
    {
        std::lock_guard<std::mutex> lock(m_subscriberMutex);
        m_subscribers[eventType].push_back(callback);
    }

    void EventManager::Unsubscribe(EventType eventType, const EventCallback &callback)
    {
        std::lock_guard<std::mutex> lock(m_subscriberMutex);
        auto it = m_subscribers.find(eventType);
        if (it != m_subscribers.end())
        {
            auto &callbacks = it->second;
            callbacks.erase(
                std::remove_if(callbacks.begin(), callbacks.end(),
                               [](const EventCallback &cb)
                               {
                                   return cb.target<void (*)(WPARAM, LPARAM)>() == nullptr;
                               }),
                callbacks.end());
        }
    }

    void EventManager::Emit(EventType eventType, WPARAM wParam, LPARAM lParam)
    {
        std::lock_guard<std::mutex> lock(m_subscriberMutex);
        auto it = m_subscribers.find(eventType);
        if (it != m_subscribers.end())
        {
            for (const auto &callback : it->second)
            {
                callback(wParam, lParam);
            }
        }
    }
}
