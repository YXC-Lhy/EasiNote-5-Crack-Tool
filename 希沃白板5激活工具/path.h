#pragma once

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <iostream>
#include <string>
#include <experimental/filesystem>

#include <regex>
#include <vector>
#include <algorithm>
std::string path;
std::string latest;
HWND hWnd111 = NULL;
namespace fs = std::experimental::filesystem;

// ���� .lnk ��ݷ�ʽĿ��·��
std::string ResolveShortcut(const std::wstring& shortcutPath) {
    CoInitialize(NULL);
    IShellLinkW* pShellLink = nullptr;
    IPersistFile* pPersistFile = nullptr;
    wchar_t targetPath[MAX_PATH] = { 0 };
    std::string result;

    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLinkW, (void**)&pShellLink))) {
        if (SUCCEEDED(pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile))) {
            if (SUCCEEDED(pPersistFile->Load(shortcutPath.c_str(), STGM_READ))) {
                if (SUCCEEDED(pShellLink->GetPath(targetPath, MAX_PATH, NULL, SLGP_UNCPRIORITY))) {
                    result = std::string(targetPath, targetPath + wcslen(targetPath));
                }
            }
            pPersistFile->Release();
        }
        pShellLink->Release();
    }

    CoUninitialize();
    return result;
}

// ��ע����ȡϣ�ְװ� 5 ��װ·��
std::string GetEasiNote5PathFromRegistry() {
    HKEY hKey;
    const char* keys[] = {
        "SOFTWARE\\Seewo\\EasiNote5",
        "SOFTWARE\\WOW6432Node\\Seewo\\EasiNote5"
    };

    for (auto key : keys) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char value[512];
            DWORD len = sizeof(value);
            if (RegQueryValueExA(hKey, "path", nullptr, nullptr, (LPBYTE)value, &len) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return std::string(value);
            }
            RegCloseKey(hKey);
        }
    }
    return "";
}

// ����������װĿ¼
std::string SearchCommonEasiNote5Paths() {
    std::string paths[] = {
        "C:\\Program Files\\Seewo\\EasiNote5",
        "C:\\Program Files (x86)\\Seewo\\EasiNote5",
        "C:\\Seewo\\EasiNote5",
        "D:\\Program Files\\Seewo\\EasiNote5",
        "D:\\Program Files (x86)\\Seewo\\EasiNote5",
        "D:\\Seewo\\EasiNote5"
    };

    for (auto& path : paths) {
        if (fs::exists(path)) return path;
    }
    return "";
}

// �������ݷ�ʽ����·��
std::string GetEasiNote5PathFromShortcut() {
    // Ҫ���ҵ�����·�� ��ǰ�û�����������
    std::vector<std::wstring> desktopPaths;

    // ��ǰ�û�����
    PWSTR userDesktop = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &userDesktop))) {
        desktopPaths.emplace_back(userDesktop);
        CoTaskMemFree(userDesktop);
    }
    // �������棨C:\Users\Public\Desktop��
    PWSTR publicDesktop = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_PublicDesktop, 0, NULL, &publicDesktop))) {
        desktopPaths.emplace_back(publicDesktop);
        CoTaskMemFree(publicDesktop);
    }
    // ���������������
    for (const auto& desktop : desktopPaths) {
        std::wstring shortcut = desktop + L"\\ϣ�ְװ� 5.lnk";
        if (fs::exists(shortcut)) {
            std::string target = ResolveShortcut(shortcut);
            if (!target.empty()) {
                fs::path p(target);
                // ȥ��ĩβ�� swenlauncher Ŀ¼��������ڣ�
                fs::path parent = p.parent_path(); // ��ȥ�� exe
                if (parent.filename() == "swenlauncher") {
                    parent = parent.parent_path(); // ��ȥ�� swenlauncher
                }
                return parent.string();
            }
        }
    }
    // û�ҵ�
    return "";
}


// ���������Զ����ϣ�ְװ� 5 ��װ·��
std::string FindEasiNote5InstallPath() {
    std::string path;
    
    // ע��� OK
    path = GetEasiNote5PathFromRegistry();
    if (!path.empty() && fs::exists(fs::path(path) / "swenlauncher" / "swenlauncher.exe"))
        return path;
    
    // ����Ŀ¼ OK
    path = SearchCommonEasiNote5Paths();
    if (!path.empty() && fs::exists(fs::path(path) / "swenlauncher" / "swenlauncher.exe"))
        return path;
    
    // �����ݷ�ʽ OK
    path = GetEasiNote5PathFromShortcut();
    if (!path.empty() && fs::exists(fs::path(path) / "swenlauncher" / "swenlauncher.exe"))
        return path;

    //δ�ҵ�
    return "";
}

/*
// ����������
int main() {
    std::string path = FindEasiNote5InstallPath();
    if (!path.empty()) {
        std::cout << "ϣ�ְװ�5��װ·����" << path << std::endl;
    }
    else {
        std::cout << "δ�ҵ�ϣ�ְװ�5�İ�װ·����" << std::endl;
    }
    return 0;
}
*/

// ���� "EasiNote5_5.2.4.8772" �� {5,2,4,8772}
std::vector<int> ParseVersion(const std::string& name) {
    std::vector<int> v;
    std::regex re(R"((\d+)\.(\d+)\.(\d+)\.(\d+))");
    std::smatch m;
    if (std::regex_search(name, m, re)) {
        for (size_t i = 1; i < m.size(); ++i)
            v.push_back(std::stoi(m[i].str()));
    }
    return v;
}

// �Ƚ������汾�Ŵ�С������ true ��� a > b
static bool CompareVersion(const std::vector<int>& a, const std::vector<int>& b) {
    // ������С���ȣ�����ʹ�� std::min����ֹ���ģ��������⣩
    size_t minLen = (a.size() < b.size()) ? a.size() : b.size();
    for (size_t i = 0; i < minLen; ++i) {
        if (a[i] != b[i])
            return a[i] > b[i];
    }
    // ���ǰ�涼��ȣ����ȸ�������Ϊ�������� 5.2.4.1 > 5.2.4��
    return a.size() > b.size();
}

// ���������� path ���ҵ����°汾Ŀ¼��
std::string GetLatestEasiNoteVersion() {
    
    if (path.empty() || !fs::exists(path))
        return "";

    std::string latestFolder;
    std::vector<int> latestVer;

    for (const auto& entry : fs::directory_iterator(path)) {
        // ��ʹ�� entry.is_directory()����Ϊ fs::is_directory(entry.path())
        if (fs::is_directory(entry.path())) {
            std::string name = entry.path().filename().string();
            if (name.rfind("EasiNote5_", 0) == 0) { // �� EasiNote5_ ��ͷ
                auto ver = ParseVersion(name);
                if (!ver.empty() && (latestFolder.empty() || CompareVersion(ver, latestVer))) {
                    latestFolder = name;
                    latestVer = ver;
                }
            }
        }
    }

    return latestFolder;
}
/*
std::string path = "C:\\Program Files (x86)\\Seewo\\EasiNote5";

int main() {
    std::string latest = GetLatestEasiNoteVersion();
    if (!latest.empty()) {
        std::cout << "���°汾�ļ��У�" << latest << std::endl;
    }
    else {
        std::cout << "δ�ҵ��κΰ汾��" << std::endl;
    }
    return 0;
}*/


#include <tlhelp32.h>

bool IsEasiNoteRunning()
{
    const std::wstring targetProcess = L"EasiNote.exe";
    bool isRunning = false;

    // ����ϵͳ���գ�������ǰ���н���
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(hSnapshot, &pe))
    {
        do
        {
            if (_wcsicmp(pe.szExeFile, targetProcess.c_str()) == 0)
            {
                isRunning = true;
                break;
            }
        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return isRunning;
}
/*
if (IsEasiNoteRunning())
{
    MessageBoxW(NULL, L"EasiNote �������У�", L"��ʾ", MB_OK | MB_ICONINFORMATION);
}
else
{
    MessageBoxW(NULL, L"EasiNote δ���С�", L"��ʾ", MB_OK | MB_ICONWARNING);
}
*/

#include "resource.h"
// �� std::string (����Ϊ UTF-8) תΪ std::wstring����ʧ���˻ص� ANSI��
static std::wstring StringToWString(const std::string& s)
{
    if (s.empty()) return std::wstring();
    // �ȳ��԰� UTF-8 ת��
    int needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (needed > 0) {
        std::wstring out(needed - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &out[0], needed);
        return out;
    }
    // ���˵� ANSI
    needed = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, NULL, 0);
    if (needed > 0) {
        std::wstring out(needed - 1, L'\0');
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &out[0], needed);
        return out;
    }
    return std::wstring();
}

// ���������滻����ȡ��Դ����Ŀ�� dll
void crack()
{
    // ����Ŀ��·���� path + "\" + latest + "\Main\SWCoreSharp.SWAuthorization.SWAuthClients.dll"
    std::string relative = "\\";
    relative += latest;
    relative += "\\Main\\SWCoreSharp.SWAuthorization.SWAuthClients.dll";
    std::string fullPathA = path + relative;

    std::wstring fullPath = StringToWString(fullPathA);

    // ���Ŀ¼������
    if (fullPath.empty()) {
        MessageBoxW(hWnd111, L"����ʧ�ܡ�\r\n��װ·��Ϊ�ջ��޷�ת��Ϊ���ַ�����", L"����", MB_OK | MB_ICONERROR);
        return;
    }

    // ����ļ����ھͳ���ɾ�� ���������ֱ�Ӽ���
    DWORD attr = GetFileAttributesW(fullPath.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) {
        // �ļ����ڣ�����ɾ��
        if (!DeleteFileW(fullPath.c_str())) {
            // ɾ��ʧ�ܣ�������������
            std::wstring errMsg = L"����ʧ�ܡ��볢���Թ���Ա������б������ټ��\r\n�޷�ɾ���ļ���";
            errMsg += fullPath;
            errMsg += L"\r\n�������: ";
            wchar_t codeBuf[32];
            swprintf_s(codeBuf, L"%u", GetLastError());
            errMsg += codeBuf;
            MessageBoxW(hWnd111, errMsg.c_str(), L"����", MB_OK | MB_ICONERROR);
            return;
        }
    }

    // ����Դ��ȡ IDR_OTHERS2 ����д��
    // ������ RT_RCDATA ������Դ ʧ���ٳ����Զ������� "BINARY"
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_OTHERS1), RT_RCDATA);
    if (!hRes) {
        // �����Զ������� "OTHERS"
        hRes = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_OTHERS1), L"OTHERS");
    }
    if (!hRes) {
        std::wstring msg = L"����ʧ�ܡ�\r\n��Դ�ļ���ʧ��\r\nδ������Դ���ҵ� IDR_OTHERS1���޷����Ƶ���";
        msg += fullPath;
        MessageBoxW(hWnd111, msg.c_str(), L"����", MB_OK | MB_ICONERROR);
        return;
    }

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        MessageBoxW(hWnd111, L"����ʧ�ܡ�\r\nLoadResource ʧ�ܡ�", L"����", MB_OK | MB_ICONERROR);
        return;
    }

    LPVOID pData = LockResource(hData);
    DWORD dataSize = SizeofResource(NULL, hRes);
    if (!pData || dataSize == 0) {
        MessageBoxW(hWnd111, L"����ʧ�ܡ�\r\nLockResource �� SizeofResource ʧ��/���� 0��", L"����", MB_OK | MB_ICONERROR);
        return;
    }

    // ����Ŀ���ļ�������ģʽ��
    HANDLE hFile = CreateFileW(fullPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::wstring errMsg = L"����ʧ�ܡ��볢���Թ���Ա������б������ټ��\r\n�޷�����/д���ļ���";
        errMsg += fullPath;
        errMsg += L"\r\n�������: ";
        wchar_t codeBuf[32];
        swprintf_s(codeBuf, L"%u", GetLastError());
        errMsg += codeBuf;
        MessageBoxW(hWnd111, errMsg.c_str(), L"����", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD written = 0;
    BOOL writeOk = WriteFile(hFile, pData, dataSize, &written, NULL);
    CloseHandle(hFile);

    if (!writeOk || written != dataSize) {
        std::wstring msg = L"����ʧ�ܡ��볢���Թ���Ա������б������ټ��\r\nд���ļ�ʧ�ܻ�δд���������ݣ�";
        msg += fullPath;
        MessageBoxW(hWnd111, msg.c_str(), L"����", MB_OK | MB_ICONERROR);
        return;
    }

    // �ɹ�
    std::wstring successMsg = L"����汾�ţ�";
    successMsg += StringToWString(latest);
    successMsg += L"\r\n�ѳɹ��滻�ļ���";
    successMsg += fullPath;
    MessageBoxW(hWnd111, successMsg.c_str(), L"����ɹ�", MB_OK | MB_ICONINFORMATION);
}



#include <windows.h>
#include <shobjidl.h>   // IShellLink
#include <shlguid.h>    // CLSID_ShellLink, IID_IShellLink
#include <objbase.h>    // CoInitialize

#include <shlobj.h>     // SHGetSpecialFolderPath



bool CreateEasiNoteShortcut()
{
    HRESULT hr;
    CoInitialize(NULL);  // ��ʼ�� COM

    // ��ݷ�ʽĿ�꣨���� "C:\Program Files (x86)\Seewo\EasiNote5\swenlauncher\swenlauncher.exe"��
    std::wstring exePath = StringToWString(path) + L"\\swenlauncher\\swenlauncher.exe";

    // ��ݷ�ʽ·��
    wchar_t publicDesktop[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, publicDesktop, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);

    std::wstring lnkPath = std::wstring(publicDesktop) + L"\\ϣ�ְװ� 5.lnk";

    // ���� IShellLink ʵ��
    IShellLinkW* pShellLink = nullptr;
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLinkW, (void**)&pShellLink);
    if (FAILED(hr))
    {
        MessageBoxW(hWnd111, L"����ʧ�ܡ�\r\n���� IShellLink ʵ��ʧ�ܡ�", L"����", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return false;
    }

    // ����Ŀ��·���Ͳ���
    pShellLink->SetPath(exePath.c_str());
    pShellLink->SetArguments(L" -m Display -iwb");

    // ��ѡ�����ù���Ŀ¼��ͼ��
    pShellLink->SetWorkingDirectory((StringToWString(path) + L"\\swenlauncher").c_str());
    pShellLink->SetIconLocation(exePath.c_str(), 0);

    // ��ѯ IPersistFile �ӿ�
    IPersistFile* pPersistFile = nullptr;
    hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
    if (FAILED(hr))
    {
        MessageBoxW(hWnd111, L"����ʧ�ܡ�\r\n��ȡ IPersistFile �ӿ�ʧ�ܡ�", L"����", MB_OK | MB_ICONERROR);
        pShellLink->Release();
        CoUninitialize();
        return false;
    }

    // �����ݷ�ʽ
    hr = pPersistFile->Save(lnkPath.c_str(), TRUE);
    pPersistFile->Release();
    pShellLink->Release();
    CoUninitialize();

    if (SUCCEEDED(hr))
    {
        MessageBoxW(hWnd111, L"�ѳɹ����������ݷ�ʽ ϣ�ְװ� 5 �ڹ������档", L"�ɹ�", MB_OK | MB_ICONINFORMATION);
        return true;
    }
    else
    {
        MessageBoxW(hWnd111, L"������ݷ�ʽʧ�ܣ��볢���Թ���Ա������б����ߣ�", L"����", MB_OK | MB_ICONERROR);
        return false;
    }
}
