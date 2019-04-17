#include <Windows.h>
#include <Shlobj.h>
#include <string>
#include"Windowstools.h"
#include "hdd.h";
#pragma comment( lib, "shell32.lib")
int GetFolderSize(const char *szPath)//获取文件夹大小
{
    int iSum = 0;
    WIN32_FIND_DATAA FindData;
    HANDLE hError;
    int FileCount = 0;
    char FilePathName[256] = { 0 };
    // 构造路径
    char FullPathName[256] = { 0 };
    strcpy_s(FilePathName, szPath);
    strcat_s(FilePathName, "\\*.*");
    hError = FindFirstFileA(FilePathName, &FindData);
    if (hError == INVALID_HANDLE_VALUE)
    {
        //printf("搜索失败!\n");
        return iSum;
    }
    while (::FindNextFileA(hError, &FindData))
    {
        // 过虑.和..
        if (strcmp(FindData.cFileName, ".") == 0 || strcmp(FindData.cFileName, "..") == 0)
        {
            continue;
        }
        // 构造完整路径
        sprintf_s(FullPathName, "%s\\%s", szPath, FindData.cFileName);
        FileCount++;
        // 获取文件信息
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //printf("%d  %s\n  ", FileCount, FullPathName);
            //printf("<文件夹>");
            iSum += GetFolderSize(FullPathName);
        }
        else
        {
            iSum += FindData.nFileSizeLow;
        }

    }
    return iSum;
}
std::string GetUsualQQ()//获取最常用qq号码
{
    int iSum = 0;
    std::string strQQ;
    WIN32_FIND_DATAA FindData;
    HANDLE hError;
    char szPath[MAX_PATH] = { 0 };
    std::string sR;
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, szPath)))
    {
        sR = szPath;
        sR += "\\Tencent Files";
    }
    char FilePathName[256] = { 0 };
    // 构造路径
    char FullPathName[256] = { 0 };
    strcpy_s(FilePathName, sR.data());
    strcat_s(FilePathName, "\\*.*");
    hError = FindFirstFileA(FilePathName, &FindData);
    if (hError == INVALID_HANDLE_VALUE)
    {
        //printf("搜索失败!\n");
        return strQQ;
    }
    while (::FindNextFileA(hError, &FindData))
    {
        // 过虑.和..
        if (strcmp(FindData.cFileName, ".") == 0 || strcmp(FindData.cFileName, "..") == 0)
        {
            continue;
        }
        // 构造完整路径
        sprintf_s(FullPathName, "%s\\%s", sR.data(), FindData.cFileName);
        // 获取文件信息
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //printf("<文件夹>");
            char *p = FindData.cFileName;
            if (48 < *p && *p < 58)//此处判断是因为存在一个All Users文件夹
            {
                int iLinshiSum = 0;
                iLinshiSum += GetFolderSize(FullPathName);
                if (iLinshiSum > iSum)
                {

                    iSum = iLinshiSum;
                    strQQ = FindData.cFileName;
                }
            }
        }
    }

    return strQQ;
}

std::string GetDisknumber()
{
    char szModelNumber[64] = { 0 };
    char szSnBuf[64] = { 0 };
    char szFwBuf[64] = { 0 };
    GetDiskModelNumber(0, szModelNumber, szSnBuf, szFwBuf);
    return szSnBuf;
}
