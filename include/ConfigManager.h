#pragma once

#include "..\include\Common.h"
#include <fstream>
#include <sstream>

namespace LumX
{
    class ConfigManager
    {
    public:
        static ConfigManager &GetInstance()
        {
            static ConfigManager instance;
            return instance;
        }

        ConfigManager(const ConfigManager &) = delete;
        ConfigManager &operator=(const ConfigManager &) = delete;

        HRESULT Initialize();
        void Shutdown();

        bool GetBool(const std::string &key, bool defaultValue = false);
        int GetInt(const std::string &key, int defaultValue = 0);
        float GetFloat(const std::string &key, float defaultValue = 0.0f);
        std::string GetString(const std::string &key, const std::string &defaultValue = "");

        void SetBool(const std::string &key, bool value);
        void SetInt(const std::string &key, int value);
        void SetFloat(const std::string &key, float value);
        void SetString(const std::string &key, const std::string &value);

        HRESULT LoadConfiguration();
        HRESULT SaveConfiguration();

    private:
        ConfigManager();
        ~ConfigManager();

        std::map<std::string, std::string> m_config;
        std::wstring m_configPath;
        std::mutex m_configMutex;

        std::wstring GetConfigPath();
    };
}
