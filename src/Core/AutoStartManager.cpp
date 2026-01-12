#include "..\include\AutoStartManager.h"

namespace LumX
{
    const wchar_t *AutoStartManager::REGISTRY_RUN_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    const wchar_t *AutoStartManager::APP_NAME = L"LumX";

    HRESULT AutoStartManager::EnableAutoStart(const std::wstring &appPath)
    {
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_KEY, 0, KEY_SET_VALUE, &hKey);
        if (result != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(result);
        }

        result = RegSetValueExW(hKey, APP_NAME, 0, REG_SZ, (LPBYTE)appPath.c_str(), (DWORD)(appPath.length() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);

        if (result != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(result);
        }

        return S_OK;
    }

    HRESULT AutoStartManager::DisableAutoStart()
    {
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_KEY, 0, KEY_SET_VALUE, &hKey);
        if (result != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(result);
        }

        result = RegDeleteValueW(hKey, APP_NAME);
        RegCloseKey(hKey);

        if (result != ERROR_SUCCESS && result != ERROR_FILE_NOT_FOUND)
        {
            return HRESULT_FROM_WIN32(result);
        }

        return S_OK;
    }

    bool AutoStartManager::IsAutoStartEnabled()
    {
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_KEY, 0, KEY_QUERY_VALUE, &hKey);
        if (result != ERROR_SUCCESS)
        {
            return false;
        }

        wchar_t buffer[MAX_PATH];
        DWORD bufferSize = sizeof(buffer);
        result = RegQueryValueExW(hKey, APP_NAME, nullptr, nullptr, (LPBYTE)buffer, &bufferSize);
        RegCloseKey(hKey);

        return result == ERROR_SUCCESS;
    }
}
