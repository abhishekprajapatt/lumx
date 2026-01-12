#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace LumX
{
    class WindowsRegistry
    {
    public:
        static std::wstring GetStringValue(HKEY hKey, const std::wstring &subKey, const std::wstring &valueName)
        {
            HKEY hOpenKey;
            if (RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_READ, &hOpenKey) != ERROR_SUCCESS)
            {
                return L"";
            }

            wchar_t buffer[MAX_PATH];
            DWORD bufferSize = sizeof(buffer);
            LONG result = RegQueryValueExW(hOpenKey, valueName.c_str(), nullptr, nullptr, (LPBYTE)buffer, &bufferSize);

            RegCloseKey(hOpenKey);

            if (result == ERROR_SUCCESS)
            {
                return std::wstring(buffer);
            }

            return L"";
        }

        static bool SetStringValue(HKEY hKey, const std::wstring &subKey, const std::wstring &valueName, const std::wstring &value)
        {
            HKEY hOpenKey;
            if (RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_WRITE, &hOpenKey) != ERROR_SUCCESS)
            {
                return false;
            }

            LONG result = RegSetValueExW(hOpenKey, valueName.c_str(), 0, REG_SZ, (LPBYTE)value.c_str(), (DWORD)(value.length() + 1) * sizeof(wchar_t));

            RegCloseKey(hOpenKey);

            return result == ERROR_SUCCESS;
        }

        static bool KeyExists(HKEY hKey, const std::wstring &subKey)
        {
            HKEY hOpenKey;
            if (RegOpenKeyExW(hKey, subKey.c_str(), 0, KEY_READ, &hOpenKey) == ERROR_SUCCESS)
            {
                RegCloseKey(hOpenKey);
                return true;
            }
            return false;
        }
    };
}
