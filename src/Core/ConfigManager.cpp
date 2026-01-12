#include "..\include\ConfigManager.h"
#include <shlobj.h>

#pragma comment(lib, "shell32.lib")

namespace LumX
{
    ConfigManager::ConfigManager()
    {
    }

    ConfigManager::~ConfigManager()
    {
    }

    HRESULT ConfigManager::Initialize()
    {
        m_configPath = GetConfigPath();
        return LoadConfiguration();
    }

    void ConfigManager::Shutdown()
    {
        SaveConfiguration();
    }

    bool ConfigManager::GetBool(const std::string &key, bool defaultValue)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        auto it = m_config.find(key);
        if (it != m_config.end())
        {
            return it->second == "true" || it->second == "1";
        }
        return defaultValue;
    }

    int ConfigManager::GetInt(const std::string &key, int defaultValue)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        auto it = m_config.find(key);
        if (it != m_config.end())
        {
            try
            {
                return std::stoi(it->second);
            }
            catch (...)
            {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    float ConfigManager::GetFloat(const std::string &key, float defaultValue)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        auto it = m_config.find(key);
        if (it != m_config.end())
        {
            try
            {
                return std::stof(it->second);
            }
            catch (...)
            {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    std::string ConfigManager::GetString(const std::string &key, const std::string &defaultValue)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        auto it = m_config.find(key);
        if (it != m_config.end())
        {
            return it->second;
        }
        return defaultValue;
    }

    void ConfigManager::SetBool(const std::string &key, bool value)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        m_config[key] = value ? "true" : "false";
    }

    void ConfigManager::SetInt(const std::string &key, int value)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        m_config[key] = std::to_string(value);
    }

    void ConfigManager::SetFloat(const std::string &key, float value)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        m_config[key] = std::to_string(value);
    }

    void ConfigManager::SetString(const std::string &key, const std::string &value)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        m_config[key] = value;
    }

    HRESULT ConfigManager::LoadConfiguration()
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        m_config.clear();

        std::ifstream file(m_configPath);
        if (!file.is_open())
        {
            SetBool("dock.enabled", true);
            SetBool("topbar.enabled", true);
            SetBool("windoweffects.enabled", true);
            SetInt("dock.hideTimeout", HIDE_TIMEOUT_MS);
            SetInt("animation.duration", ANIMATION_DURATION_MS);
            return S_OK;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == ';')
                continue;

            size_t delimiterPos = line.find('=');
            if (delimiterPos == std::string::npos)
                continue;

            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            m_config[key] = value;
        }

        file.close();
        return S_OK;
    }

    HRESULT ConfigManager::SaveConfiguration()
    {
        std::lock_guard<std::mutex> lock(m_configMutex);

        std::ofstream file(m_configPath);
        if (!file.is_open())
        {
            return E_FAIL;
        }

        for (const auto &[key, value] : m_config)
        {
            file << key << "=" << value << "\n";
        }

        file.close();
        return S_OK;
    }

    std::wstring ConfigManager::GetConfigPath()
    {
        wchar_t appDataPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appDataPath)))
        {
            std::wstring configPath(appDataPath);
            configPath += L"\\LumX";

            CreateDirectoryW(configPath.c_str(), nullptr);

            configPath += L"\\config.ini";
            return configPath;
        }

        return L"config.ini";
    }
}
