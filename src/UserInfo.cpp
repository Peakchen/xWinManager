#include "UserInfo.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include "swmgrapp.h"
#include "ConfOperation.h"
#include "curl/curl.h"
#include "Windowstools.h"
#include "selfupdate.h"

QString userinfoItem[userinfoItemCount] = {
    "address",
    "cents",
    "ctime",
    "email",
    "email_valid",

    "gender",
    "id",
    "ltime",
    "mobile",
    "mobile_valid",

    "mtime",
    "nickname",
    "status"
};

UserInfo::UserInfo(QObject *parent) : QObject(parent)
{
    this->init = "0xff";
    _userDatWatcher = NULL;
}
//序列化，初始化。
void UserInfo::serializeUserInfo(bool bSerialize) {
    QJsonObject jsUserInfo;
    QByteArray fileBuf;
    QJsonDocument doc;

    QString szFile = ConfOperation::Root().getSubpathFile("Data", "user.dat");
    if (bSerialize) { // write from user.dat
        jsUserInfo["init"] = QJsonValue(this->init);
        jsUserInfo["username"] = QJsonValue(this->username);
        jsUserInfo["password"] = QJsonValue(this->password);
        jsUserInfo["token"] = QJsonValue(this->usertoken);
        for (int i = 0; i < userinfoItemCount; i++) {
            jsUserInfo[userinfoItem[i]] = userPrivateInfo.contains(userinfoItem[i]) ? userPrivateInfo[userinfoItem[i]] : QString("");
        }
        doc = QJsonDocument(jsUserInfo);
        QFile saveFile(szFile);
        if (saveFile.open(QIODevice::ReadWrite)) {
            saveFile.resize(0);
            saveFile.write(doc.toJson(QJsonDocument::Compact));
            saveFile.close();
        }
    }
    else { // read from user.dat
        QFile saveFile(szFile);
        if (saveFile.open(QIODevice::ReadWrite)) {
            fileBuf = saveFile.readAll();
            doc = QJsonDocument::fromJson(fileBuf);
            saveFile.close();
        }
        if ( doc.isEmpty() || !doc.isObject() ) {
            return;
        }
        jsUserInfo = doc.object();
        this->init = jsUserInfo.contains("init") ? jsUserInfo.value("init").toString() : QString("0");
        this->username = jsUserInfo.contains("username") ? jsUserInfo.value("username").toString() : QString("");
        this->password = jsUserInfo.contains("password") ? jsUserInfo.value("password").toString() : QString("");
        this->usertoken = jsUserInfo.contains("token") ? jsUserInfo.value("token").toString() : QString("");
        for (int i = 0; i < userinfoItemCount; i++) {
            userPrivateInfo[userinfoItem[i]] = jsUserInfo.contains(userinfoItem[i]) ? jsUserInfo.value(userinfoItem[i]).toString() : QString("");
        }
    }
}

QJsonObject UserInfo::toJsonObject() {
    QJsonObject userObject;
    userObject["init"] = QJsonValue(init);
    userObject["username"] = QJsonValue(username);
    userObject["password"] = QJsonValue(password);
    userObject["token"] = QJsonValue(usertoken);
    for (int i = 0; i < userinfoItemCount; i++) {
        userObject[userinfoItem[i]] = userPrivateInfo.contains(userinfoItem[i]) ? userPrivateInfo[userinfoItem[i]] : QString("");
    }
    return userObject;
}

void UserInfo::clean() {
    this->init = "0xff";
    this->username.clear();
    this->password.clear();
    this->usertoken.clear();
    for (int i = 0; i < userinfoItemCount; i++) {
        userPrivateInfo[userinfoItem[i]].clear();
    }
}

size_t UserInfo::LoginCallback(char *buffer, size_t size, size_t nitems, void *outstream) {
    QByteArray *response = (QByteArray *)outstream;
    if (!response) {
        return 0;
    }
    response->append(buffer, size*nitems);
    return size * nitems;
}

QString UserInfo::postMethod(std::string url,std::string cookieFile,std::string post) {
    QByteArray response;
    QString szRet;
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.data());
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.data());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, UserInfo::LoginCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        curl_easy_setopt(curl, CURLOPT_POST, (long)1);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res == CURLE_OK) {
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(response, &err);
            if (err.error == QJsonParseError::NoError) {
                szRet = doc.toJson(QJsonDocument::Compact);
            }
            else {
                szRet.append("param error");
            }
        }
        else {
            szRet.append("server error");
        }
    }
    else {
        szRet.append("please check network");
    }
    return szRet;
}

//先将密码转utf8，然后求取hash值，然后自身再加密，再求hash，得到结果
QByteArray UserInfo::cryptPassword(QString szPassword) {
    QByteArray byCrypto = szPassword.toUtf8();
    byCrypto = QCryptographicHash::hash(byCrypto, QCryptographicHash::Md5).toHex();
    byCrypto += byCrypto.mid(8, 8);
    byCrypto = QCryptographicHash::hash(byCrypto, QCryptographicHash::Sha1).toHex();
    return byCrypto;
}
//网页调用用户登录
void UserInfo::UserLogin(QString szUserName, QString szPassword) {
    QByteArray byCrypto;
    QString POSTFIELDS;
    QString szResult;
    QString szCookieFile = SwmgrApp::GetCookieFile();
    QString szUserLoginUrl = SwmgrApp::GetUserLoginUrl();

    if (szUserName.size()==0) {
        return ;
    }
    clean();
    byCrypto = UserInfo::cryptPassword(szPassword);

    this->username = szUserName;
    this->init = "0";

    POSTFIELDS=QString("%1=%2&%3=%4").arg("username").arg(this->username).arg("password").arg(QString(byCrypto));
    szResult = postMethod(szUserLoginUrl.toStdString(), szCookieFile.toStdString(), POSTFIELDS.toStdString());
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(szResult.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject resObject = doc.object();
        int nCode = resObject.value("code").toInt();
        if (nCode == 1) {
            qDebug() << L"成功登录";
            resObject = resObject.value("msg").toObject();
            this->init = "2";
            this->usertoken = resObject.value("token").toString();
            this->password = byCrypto;
            for (int i = 0; i < userinfoItemCount; i++) {
                this->userPrivateInfo[userinfoItem[i]] = resObject.contains(userinfoItem[i]) && resObject.value(userinfoItem[i]).isString() ? resObject.value(userinfoItem[i]).toString() : QString("");
            }
        }
        else {
            qDebug() << resObject.value("msg").toString();
        }
    }
    else {
        qDebug() << szResult;
    }
    StopWatcher();
    this->serializeUserInfo(true);
    // emit signalLoginUser(toJsonObject().toVariantMap());
    StartWatcher();
}

//默认登录方法
void UserInfo::UserLoginDef()
{
    QJsonObject jsUserInfo;
    QByteArray fileBuf;
    QJsonDocument doc;

    QString szFile = ConfOperation::Root().getSubpathFile("Data", "user.dat");
    QFile saveFile(szFile);
    if (saveFile.open(QIODevice::ReadWrite)) {
        fileBuf = saveFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        saveFile.close();
    }

    if ( !doc.isEmpty() && doc.isObject() ) {
        jsUserInfo = doc.object();
        this->init = "1";       //有数据就默认有密码，但是未登录
        this->username = jsUserInfo.contains("username") ? jsUserInfo.value("username").toString() : QString("");
        this->password = jsUserInfo.contains("password") ? jsUserInfo.value("password").toString() : QString("");
        this->usertoken = jsUserInfo.contains("token") ? jsUserInfo.value("token").toString() : QString("");
        for (int i = 0; i < userinfoItemCount; i++) {
            userPrivateInfo[userinfoItem[i]] = jsUserInfo.contains(userinfoItem[i]) ? jsUserInfo.value(userinfoItem[i]).toString() : QString("");
        }
    }
    else
    {
        std::string strQQ = GetUsualQQ();
        QString strUserName;
        if(strQQ.length()>0)
        {
            //用qq号码注册
            QString strUserEmail = QString::fromStdString(strQQ)+"@qq.com";
            strUserName = "QQ" + QString::fromStdString(strQQ);
            QString strMobile = QString::fromStdString(strQQ);
            RegistUser(strUserName,"123456",strUserEmail,strMobile);
        }
        else
        {
            init = "0";
            strUserName = QString::fromStdString(GetDisknumber());
            username = strUserName;
        }

    }
    if(init == "1") //（注册成功）如果有密码，则登录
    {
        QString POSTFIELDS;
        QString szResult;
        QString szCookieFile = SwmgrApp::GetCookieFile();
        QString szUserLoginUrl = SwmgrApp::GetUserLoginUrl();

        POSTFIELDS=QString("%1=%2&%3=%4").arg("username").arg(this->username).arg("password").arg(password);
        szResult = postMethod(szUserLoginUrl.toStdString(), szCookieFile.toStdString(), POSTFIELDS.toStdString());
        QJsonParseError err;
        QJsonDocument docLoginResult = QJsonDocument::fromJson(szResult.toUtf8(), &err);
        if (err.error == QJsonParseError::NoError && docLoginResult.isObject()) {
            QJsonObject resObject = docLoginResult.object();
            int nCode = resObject.value("code").toInt();
            if (nCode == 1) {
                qDebug() << L"成功登录";
                resObject = resObject.value("msg").toObject();
                this->init = "2";
                this->usertoken = resObject.value("token").toString();
                for (int i = 0; i < userinfoItemCount; i++) {
                    this->userPrivateInfo[userinfoItem[i]] = resObject.contains(userinfoItem[i]) && resObject.value(userinfoItem[i]).isString() ? resObject.value(userinfoItem[i]).toString() : QString("");
                }
            }
            else {
                qDebug() << resObject.value("msg").toString();
            }
        }
        else {
            qDebug() << szResult;
        }
        StopWatcher();
        this->serializeUserInfo(true);
        StartWatcher();
    }
    // emit signalLoginUser(toJsonObject().toVariantMap());
}
//用户注册
void UserInfo::RegistUser(QString szUserName, QString szPassword, QString szEmail,QString strMobile) {
    QByteArray byCrypto;
    QString POSTFIELDS;
    QString szResult;
    QString szCookieFile = SwmgrApp::GetCookieFile();
    QString szUserRegisteUrl = SwmgrApp::GetUserRegisteUrl();
    if (szUserName.size()==0){
        return;
    }
    clean();
    byCrypto = UserInfo::cryptPassword(szPassword);
    POSTFIELDS = QString("%1=%2&%3=%4&%5=%6&%7=%8").arg("username").arg(szUserName).arg("password").arg(QString(byCrypto)).arg("email").arg(szEmail).arg("mobile").arg(strMobile);
    szResult = postMethod(szUserRegisteUrl.toStdString(), szCookieFile.toStdString(), POSTFIELDS.toStdString());
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(szResult.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject resObject = doc.object();
        int nCode = resObject.value("code").toInt();
        if (nCode == 1) {//注册成功才会写入文件。
            qDebug() << L"注册成功";
            this->init = "1";
            this->username = szUserName;
            this->password = byCrypto;
            StopWatcher();
            this->serializeUserInfo(true);
            // emit signalRegisteUser(toJsonObject().toVariantMap());
            StartWatcher();
        }
        else {
            qDebug() << resObject.value("msg").toString();      //注册失败
            this->init="0";
            this->username = szUserName;
        }
    }
    else {
        qDebug() << szResult;
    }
}
//修改用户
void UserInfo::ModifyUserInfo(QVariantMap userInfo) {
    Q_UNUSED(userInfo);
    // emit signalModifyUserInfo(toJsonObject().toVariantMap());
}
//查询用户信息，如果查到，则登录。
void UserInfo::QueryUserInfo() {
    if(this->username.isEmpty())
        return;

    // emit signalLoginUser(toJsonObject().toVariantMap());
}
//清除内存中的数据
void UserInfo::ClearUserInfo() {
    clean();
    // emit signalLoginUser(toJsonObject().toVariantMap());
}
//开始监视数据文件变化。
void UserInfo::StartWatcher() {
    if (_userDatWatcher!=NULL) {
        return;
    }
    _userDatWatcher = new QFileSystemWatcher(QStringList()<<ConfOperation::Root().getSubpathFile("Data", "user.dat"),this);
    QObject::connect(_userDatWatcher,SIGNAL(fileChanged(QString)),this,SLOT(ChangedDat(QString)));
}
//停止监视数据文件变化。
void UserInfo::StopWatcher() {
    if (_userDatWatcher==NULL) {
        return;
    }
    QObject::disconnect(_userDatWatcher,SIGNAL(fileChanged(QString)),this,SLOT(ChangedDat(QString)));
    delete _userDatWatcher;
    _userDatWatcher = NULL;
}
//请求增值用户信息
void UserInfo::requestComfireUpdateUserInfo(QString szOldPassword, QString szNewPassword)
{
    QJsonObject jsUserInfo;
    QByteArray fileBuf;
    QJsonDocument docinfo;
    QString POSTFIELDS;
    QString szResult;
    bool isSuccess = false;
    QString szCookieFile = SwmgrApp::GetCookieFile();
    QString szUserLoginUrl = SwmgrApp::GetUserLoginUrl();
    QString szFile = ConfOperation::Root().getSubpathFile("Data", "user.dat");
    //先读取 确认密码 信息
    QFile saveFile(szFile);
    if (saveFile.open(QIODevice::ReadWrite)) {
        fileBuf = saveFile.readAll();
        docinfo = QJsonDocument::fromJson(fileBuf);
        saveFile.close();
    }
    if ( docinfo.isEmpty() || !docinfo.isObject() ) {
        return;
    }
    jsUserInfo = docinfo.object();
    QString sinit = jsUserInfo.contains("init") ? jsUserInfo.value("init").toString() : QString("0");
    QString susername = jsUserInfo.contains("username") ? jsUserInfo.value("username").toString() : QString("");
    QString susertoken = jsUserInfo.contains("token") ? jsUserInfo.value("token").toString() : QString("");

    QByteArray byCrypto = UserInfo::cryptPassword(szOldPassword);
    qDebug()<<"new passwd: "<<QString(byCrypto)<<" old passwd: "<<QString(UserInfo::cryptPassword(this->password));
    POSTFIELDS=QString("%1=%2&%3=%4").arg("username").arg(this->username).arg("password").arg(QString(byCrypto));
    szResult = postMethod(szUserLoginUrl.toStdString(), szCookieFile.toStdString(), POSTFIELDS.toStdString());
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(szResult.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject() && (QString(this->password).compare(QString(byCrypto)) == 0)) {
        QJsonObject resObject = doc.object();
        int nCode = resObject.value("code").toInt();
        if (nCode == 1) {
            // 确认 正确之后 写如新的 密码
            jsUserInfo["init"] = sinit;
            jsUserInfo["username"] = QJsonValue(susername);
            jsUserInfo["password"] = QJsonValue(szNewPassword);
            jsUserInfo["token"] = susertoken;
            for (int i = 0; i < userinfoItemCount; i++) {
                this->userPrivateInfo[userinfoItem[i]] = resObject.contains(userinfoItem[i]) && resObject.value(userinfoItem[i]).isString() ? resObject.value(userinfoItem[i]).toString() : QString("");
            }
            doc = QJsonDocument(jsUserInfo);
            QFile saveFile(szFile);
            if (saveFile.open(QIODevice::ReadWrite)) {
                saveFile.resize(0);
                saveFile.write(doc.toJson(QJsonDocument::Compact));
                saveFile.close();
                isSuccess = true;
            }
        }
    }else{
        isSuccess = false;
    }
    StopWatcher();
    this->serializeUserInfo(true);
    // emit signalComfireUpdateUserInfo(isSuccess);
    StartWatcher();
}
//更新配置文件信息到内存。
void UserInfo::ChangedDat(QString path) {
    if (path.compare(ConfOperation::Root().getSubpathFile("Data", "user.dat"), Qt::CaseInsensitive) == 0) {
        qDebug() << path;
        serializeUserInfo();
        QueryUserInfo();
    }
}
