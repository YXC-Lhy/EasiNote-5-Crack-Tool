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

// 解析 .lnk 快捷方式目标路径
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

// 从注册表获取希沃白板 5 安装路径
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

// 搜索常见安装目录
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

// 从桌面快捷方式解析路径
std::string GetEasiNote5PathFromShortcut() {
    // 要查找的桌面路径 当前用户、公用桌面
    std::vector<std::wstring> desktopPaths;

    // 当前用户桌面
    PWSTR userDesktop = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &userDesktop))) {
        desktopPaths.emplace_back(userDesktop);
        CoTaskMemFree(userDesktop);
    }
    // 公用桌面（C:\Users\Public\Desktop）
    PWSTR publicDesktop = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_PublicDesktop, 0, NULL, &publicDesktop))) {
        desktopPaths.emplace_back(publicDesktop);
        CoTaskMemFree(publicDesktop);
    }
    // 遍历两个桌面查找
    for (const auto& desktop : desktopPaths) {
        std::wstring shortcut = desktop + L"\\希沃白板 5.lnk";
        if (fs::exists(shortcut)) {
            std::string target = ResolveShortcut(shortcut);
            if (!target.empty()) {
                fs::path p(target);
                // 去掉末尾的 swenlauncher 目录（如果存在）
                fs::path parent = p.parent_path(); // 先去掉 exe
                if (parent.filename() == "swenlauncher") {
                    parent = parent.parent_path(); // 再去掉 swenlauncher
                }
                return parent.string();
            }
        }
    }
    // 没找到
    return "";
}


// 主函数：自动检测希沃白板 5 安装路径
std::string FindEasiNote5InstallPath() {
    std::string path;
    
    // 注册表 OK
    path = GetEasiNote5PathFromRegistry();
    if (!path.empty() && fs::exists(fs::path(path) / "swenlauncher" / "swenlauncher.exe"))
        return path;
    
    // 常见目录 OK
    path = SearchCommonEasiNote5Paths();
    if (!path.empty() && fs::exists(fs::path(path) / "swenlauncher" / "swenlauncher.exe"))
        return path;
    
    // 桌面快捷方式 OK
    path = GetEasiNote5PathFromShortcut();
    if (!path.empty() && fs::exists(fs::path(path) / "swenlauncher" / "swenlauncher.exe"))
        return path;

    //未找到
    return "";
}

/*
// 测试主函数
int main() {
    std::string path = FindEasiNote5InstallPath();
    if (!path.empty()) {
        std::cout << "希沃白板5安装路径：" << path << std::endl;
    }
    else {
        std::cout << "未找到希沃白板5的安装路径。" << std::endl;
    }
    return 0;
}
*/

// 解析 "EasiNote5_5.2.4.8772" → {5,2,4,8772}
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

// 比较两个版本号大小：返回 true 如果 a > b
static bool CompareVersion(const std::vector<int>& a, const std::vector<int>& b) {
    // 计算最小长度（避免使用 std::min，防止宏或模板解析问题）
    size_t minLen = (a.size() < b.size()) ? a.size() : b.size();
    for (size_t i = 0; i < minLen; ++i) {
        if (a[i] != b[i])
            return a[i] > b[i];
    }
    // 如果前面都相等，长度更长的视为更大（例如 5.2.4.1 > 5.2.4）
    return a.size() > b.size();
}

// 主函数：在 path 下找到最新版本目录名
std::string GetLatestEasiNoteVersion() {
    
    if (path.empty() || !fs::exists(path))
        return "";

    std::string latestFolder;
    std::vector<int> latestVer;

    for (const auto& entry : fs::directory_iterator(path)) {
        // 不使用 entry.is_directory()，改为 fs::is_directory(entry.path())
        if (fs::is_directory(entry.path())) {
            std::string name = entry.path().filename().string();
            if (name.rfind("EasiNote5_", 0) == 0) { // 以 EasiNote5_ 开头
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
        std::cout << "最新版本文件夹：" << latest << std::endl;
    }
    else {
        std::cout << "未找到任何版本。" << std::endl;
    }
    return 0;
}*/


#include <tlhelp32.h>

bool IsEasiNoteRunning()
{
    const std::wstring targetProcess = L"EasiNote.exe";
    bool isRunning = false;

    // 创建系统快照，包含当前所有进程
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
    MessageBoxW(NULL, L"EasiNote 正在运行！", L"提示", MB_OK | MB_ICONINFORMATION);
}
else
{
    MessageBoxW(NULL, L"EasiNote 未运行。", L"提示", MB_OK | MB_ICONWARNING);
}
*/

#include "resource.h"
// 将 std::string (假设为 UTF-8) 转为 std::wstring（若失败退回到 ANSI）
static std::wstring StringToWString(const std::string& s)
{
    if (s.empty()) return std::wstring();
    // 先尝试按 UTF-8 转换
    int needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (needed > 0) {
        std::wstring out(needed - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &out[0], needed);
        return out;
    }
    // 回退到 ANSI
    needed = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, NULL, 0);
    if (needed > 0) {
        std::wstring out(needed - 1, L'\0');
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &out[0], needed);
        return out;
    }
    return std::wstring();
}

// 主函数：替换并提取资源覆盖目标 dll
void crack()
{
    // 构造目标路径： path + "\" + latest + "\Main\SWCoreSharp.SWAuthorization.SWAuthClients.dll"
    std::string relative = "\\";
    relative += latest;
    relative += "\\Main\\SWCoreSharp.SWAuthorization.SWAuthClients.dll";
    std::string fullPathA = path + relative;

    std::wstring fullPath = StringToWString(fullPathA);

    // 检查目录存在性
    if (fullPath.empty()) {
        MessageBoxW(hWnd111, L"激活失败。\r\n安装路径为空或无法转换为宽字符串。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    // 如果文件存在就尝试删除 如果不存在直接继续
    DWORD attr = GetFileAttributesW(fullPath.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES) {
        // 文件存在，尝试删除
        if (!DeleteFileW(fullPath.c_str())) {
            // 删除失败，弹窗报错并结束
            std::wstring errMsg = L"激活失败。请尝试以管理员身份运行本工具再激活！\r\n无法删除文件：";
            errMsg += fullPath;
            errMsg += L"\r\n错误代码: ";
            wchar_t codeBuf[32];
            swprintf_s(codeBuf, L"%u", GetLastError());
            errMsg += codeBuf;
            MessageBoxW(hWnd111, errMsg.c_str(), L"错误", MB_OK | MB_ICONERROR);
            return;
        }
    }

    // 从资源提取 IDR_OTHERS2 覆盖写入
    // 尝试以 RT_RCDATA 查找资源 失败再尝试自定义类型 "BINARY"
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_OTHERS1), RT_RCDATA);
    if (!hRes) {
        // 尝试自定义类型 "OTHERS"
        hRes = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_OTHERS1), L"OTHERS");
    }
    if (!hRes) {
        std::wstring msg = L"激活失败。\r\n资源文件丢失。\r\n未能在资源中找到 IDR_OTHERS1。无法复制到：";
        msg += fullPath;
        MessageBoxW(hWnd111, msg.c_str(), L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) {
        MessageBoxW(hWnd111, L"激活失败。\r\nLoadResource 失败。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    LPVOID pData = LockResource(hData);
    DWORD dataSize = SizeofResource(NULL, hRes);
    if (!pData || dataSize == 0) {
        MessageBoxW(hWnd111, L"激活失败。\r\nLockResource 或 SizeofResource 失败/返回 0。", L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    // 创建目标文件（覆盖模式）
    HANDLE hFile = CreateFileW(fullPath.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::wstring errMsg = L"激活失败。请尝试以管理员身份运行本工具再激活！\r\n无法创建/写入文件：";
        errMsg += fullPath;
        errMsg += L"\r\n错误代码: ";
        wchar_t codeBuf[32];
        swprintf_s(codeBuf, L"%u", GetLastError());
        errMsg += codeBuf;
        MessageBoxW(hWnd111, errMsg.c_str(), L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD written = 0;
    BOOL writeOk = WriteFile(hFile, pData, dataSize, &written, NULL);
    CloseHandle(hFile);

    if (!writeOk || written != dataSize) {
        std::wstring msg = L"激活失败。请尝试以管理员身份运行本工具再激活！\r\n写入文件失败或未写入完整数据：";
        msg += fullPath;
        MessageBoxW(hWnd111, msg.c_str(), L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    // 成功
    std::wstring successMsg = L"激活版本号：";
    successMsg += StringToWString(latest);
    successMsg += L"\r\n已成功替换文件：";
    successMsg += fullPath;
    MessageBoxW(hWnd111, successMsg.c_str(), L"激活成功", MB_OK | MB_ICONINFORMATION);
}



#include <windows.h>
#include <shobjidl.h>   // IShellLink
#include <shlguid.h>    // CLSID_ShellLink, IID_IShellLink
#include <objbase.h>    // CoInitialize

#include <shlobj.h>     // SHGetSpecialFolderPath



bool CreateEasiNoteShortcut()
{
    HRESULT hr;
    CoInitialize(NULL);  // 初始化 COM

    // 快捷方式目标（例如 "C:\Program Files (x86)\Seewo\EasiNote5\swenlauncher\swenlauncher.exe"）
    std::wstring exePath = StringToWString(path) + L"\\swenlauncher\\swenlauncher.exe";

    // 快捷方式路径
    wchar_t publicDesktop[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, publicDesktop, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);

    std::wstring lnkPath = std::wstring(publicDesktop) + L"\\希沃白板 5.lnk";

    // 创建 IShellLink 实例
    IShellLinkW* pShellLink = nullptr;
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLinkW, (void**)&pShellLink);
    if (FAILED(hr))
    {
        MessageBoxW(hWnd111, L"创建失败。\r\n创建 IShellLink 实例失败。", L"错误", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return false;
    }

    // 设置目标路径和参数
    pShellLink->SetPath(exePath.c_str());
    pShellLink->SetArguments(L" -m Display -iwb");

    // 可选：设置工作目录和图标
    pShellLink->SetWorkingDirectory((StringToWString(path) + L"\\swenlauncher").c_str());
    pShellLink->SetIconLocation(exePath.c_str(), 0);

    // 查询 IPersistFile 接口
    IPersistFile* pPersistFile = nullptr;
    hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
    if (FAILED(hr))
    {
        MessageBoxW(hWnd111, L"创建失败。\r\n获取 IPersistFile 接口失败。", L"错误", MB_OK | MB_ICONERROR);
        pShellLink->Release();
        CoUninitialize();
        return false;
    }

    // 保存快捷方式
    hr = pPersistFile->Save(lnkPath.c_str(), TRUE);
    pPersistFile->Release();
    pShellLink->Release();
    CoUninitialize();

    if (SUCCEEDED(hr))
    {
        MessageBoxW(hWnd111, L"已成功创建桌面快捷方式 希沃白板 5 于公用桌面。", L"成功", MB_OK | MB_ICONINFORMATION);
        return true;
    }
    else
    {
        MessageBoxW(hWnd111, L"创建快捷方式失败，请尝试以管理员身份运行本工具！", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }
}



#include <commdlg.h>

// 复制文件并处理只读属性
bool CopyAndReplaceFileWithReadOnly(const std::wstring& src, const std::wstring& dst)
{
    SetFileAttributesW(dst.c_str(), FILE_ATTRIBUTE_NORMAL); // 取消只读
    if (!CopyFileW(src.c_str(), dst.c_str(), FALSE))
        return false;
    SetFileAttributesW(dst.c_str(), FILE_ATTRIBUTE_READONLY); // 设置只读(防止被修复为原图)
    return true;
}

// 函数：选择PNG,替换文件
bool ReplaceBannerImage()
{
    // 打开文件选择窗口
    wchar_t filePath[MAX_PATH] = { 0 };
    OPENFILENAMEW ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"PNG 文件（图片尺寸需小于于屏幕尺寸！） (*.png)\0*.png\0所有文件 (*.*)\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"请选择要使用的图片（必须为PNG格式！）（图片尺寸需小于于屏幕尺寸！）";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    ofn.hwndOwner = hWnd111;

    if (!GetOpenFileNameW(&ofn))
    {
        MessageBoxW(hWnd111, L"未选择任何PNG文件。", L"提示", MB_OK | MB_ICONWARNING);
        return false;
    }

    std::wstring srcFile = filePath;

    // 方法一：当前用户 AppData 路径
    wchar_t appdataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdataPath)))
    {
        std::wstring bannerPath = std::wstring(appdataPath) +
            L"\\Seewo\\EasiNote5\\Resources\\Banner\\Banner.png";

        if (fs::exists(bannerPath))
        {
            if (CopyAndReplaceFileWithReadOnly(srcFile, bannerPath))
            {
                std::wstring msg = L"已成功替换图片：\n";
                msg += bannerPath;
                MessageBoxW(hWnd111, msg.c_str(), L"修改成功", MB_OK | MB_ICONINFORMATION);
                return true; // 成功则直接返回
            }
        }
    }

    // 方法二（适用旧版希沃白板5）：全局路径 path + latest
    std::wstring splashPath = StringToWString(path) + L"\\" + StringToWString(latest) + L"\\Main\\Assets\\SplashScreen.png";
    if (fs::exists(splashPath))
    {
        if (CopyAndReplaceFileWithReadOnly(srcFile, splashPath))
        {
            std::wstring msg = L"已成功替换旧版本希沃白板5图片：\n";
            msg += splashPath;
            MessageBoxW(hWnd111, msg.c_str(), L"修改成功", MB_OK | MB_ICONINFORMATION);
            return true;
        }
    }

    // 两种方法均失败
    MessageBoxW(hWnd111, L"修改失败：未找到可替换的图片。\r\n请重新安装希沃白板5。", L"错误", MB_OK | MB_ICONERROR);
    return false;
}


#include <fstream>
// 从资源文件加载 PNG 并写入临时路径
std::wstring ExtractPngFromResource(int resourceID, LPCWSTR resourceType)
{
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(resourceID), resourceType);
    if (!hRes) return L"";

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return L"";

    DWORD size = SizeofResource(NULL, hRes);
    void* pData = LockResource(hData);
    if (!pData || size == 0) return L"";

    // 生成临时文件路径
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring tempFile = std::wstring(tempPath) + L"temp_banner.png";

    // 写入文件
    std::ofstream out(tempFile, std::ios::binary);
    if (!out) return L"";
    out.write(reinterpret_cast<const char*>(pData), size);
    out.close();

    return tempFile;
}

// 从资源中加载 PNG 并替换文件
bool ReplaceBannerImageFromResource()
{
    int iiikkk = 0;
    // 从资源中提取PNG
    std::wstring srcFile = ExtractPngFromResource(IDB_PNG1, L"PNG");
    if (srcFile.empty())
    {
        MessageBoxW(hWnd111, L"无法从资源文件中提取图片。请重新下载完好的本工具。", L"错误", MB_OK | MB_ICONERROR);
        return false;
    }

    // 方法一：当前用户 AppData 路径
    wchar_t appdataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appdataPath)))
    {
        std::wstring bannerFolder = std::wstring(appdataPath) +
            L"\\Seewo\\EasiNote5\\Resources\\Banner\\";
        std::wstring bannerPath = bannerFolder + L"Banner.png";

        //如果文件夹存在则复制（无论 Banner.png 是否存在）
        if (fs::exists(bannerFolder))
        {
            if (CopyAndReplaceFileWithReadOnly(srcFile, bannerPath))
            {
                iiikkk = 1;
                //std::wstring msg = L"已成功还原启动图片：\n";
                //msg += bannerPath;
                //MessageBoxW(hWnd111, msg.c_str(), L"还原成功", MB_OK | MB_ICONINFORMATION);
                //DeleteFileW(srcFile.c_str());// 用完后删除临时文件
                //return true; // 成功则直接返回
            }
        }
    }

    // 方法二（适用旧版希沃白板5）：全局路径 path + latest
    std::wstring splashFolder = StringToWString(path) + L"\\" + StringToWString(latest) + L"\\Main\\Assets\\";
    std::wstring splashPath = splashFolder + L"SplashScreen.png";

    //文件夹存在即可复制
    if (fs::exists(splashFolder))
    {
        if (CopyAndReplaceFileWithReadOnly(srcFile, splashPath))
        {
            iiikkk = 1;
            //std::wstring msg = L"已成功还原旧版本希沃白板5启动图片：\n";
            //msg += splashPath;
            //MessageBoxW(hWnd111, msg.c_str(), L"还原成功", MB_OK | MB_ICONINFORMATION);
            //DeleteFileW(srcFile.c_str());// 用完后删除临时文件
            //return true;
        }
    }

    if (iiikkk == 1) {
        std::wstring msg = L"已成功还原启动图片。\n";
        //msg += bannerPath;
        MessageBoxW(hWnd111, msg.c_str(), L"还原成功", MB_OK | MB_ICONINFORMATION);
        DeleteFileW(srcFile.c_str());// 用完后删除临时文件
        return true; // 成功则直接返回
    }

    // 两种方法均失败
    MessageBoxW(hWnd111, L"还原失败：未找到可替换的图片。\r\n请重新安装希沃白板5。", L"错误", MB_OK | MB_ICONERROR);
    // 用完后删除临时文件
    DeleteFileW(srcFile.c_str());
    return false;
}
