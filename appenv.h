#ifndef APPENV_H
#define APPENV_H
#include<windows.h>
#include <string>


//主要是一些路径获取和字符集转换的函数
std::string GetModulePath(HMODULE hModule = NULL);
std::string GetAppdataPath(std::string szCompany="HurricaneTeam");
std::string GetProgramProfilePath(std::string name);
std::string GetFilePathFromFile(std::string szFile);

// ==========================================
//获取本地Appdata路径，这个路径下的读写，一般是不需要权限的。
std::string GetLocalAppDataPath();
void InitDir();

BOOL StringToWString(const std::string &str, std::wstring &wstr);
BOOL WStringToString(const std::wstring &wstr, std::string &str);
BOOL UTF8ToWString(const std::string &str, std::wstring &wstr);

#endif // APPENV_H
