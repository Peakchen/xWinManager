#ifndef SELFUPDATE
#define SELFUPDATE
void StartSelfUpdate();
void MyHttpGet(const std::string &url, int timeout, std::string &rspData);
void MyHttpPost(const std::string &url, int timeout, const std::string &reqBody, std::string &rspData);
void MyHttpDownload(const std::string &url, int timeout, const std::string &path);
BOOL IsNeedUpdate(std::string strUpdateUrl, std::string strReqData, std::string &strRspData, std::string &strFileSize, std::string &strFileVersion);
void KillProgressByName(std::string strProName);
std::string GetVersion(std::string strFileName);
void StartSC();
void ShowProgressBar(int i);

#endif // SELFUPDATE

