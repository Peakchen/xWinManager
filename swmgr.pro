#-------------------------------------------------
#
# Project created by QtCreator 2013-10-12T14:57:24
#
#-------------------------------------------------
#CONFIG -= flat
QT     += core gui widgets network webkitwidgets
QT     += winextras
QT     += xml
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#QMAKE_LFLAGS += /MANIFESTUAC:"level='requireAdministrator'uiAccess='true'"
TARGET = xbmgr
TEMPLATE = app

INCLUDEPATH += curl/include curl/lib
LIBS += Wldap32.lib ws2_32.lib Advapi32.lib User32.lib shell32.lib

SOURCES += main.cpp\
    swmgrapp.cpp \
    globalsingleton.cpp \
    ConfOperation.cpp \
    SoftwareList.cpp \
    DataControl.cpp \
    PackageRunner.cpp \
    UserInfo.cpp \
    curl/lib/vtls/axtls.c \
    curl/lib/vtls/curl_darwinssl.c \
    curl/lib/vtls/curl_schannel.c \
    curl/lib/vtls/cyassl.c \
    curl/lib/vtls/gskit.c \
    curl/lib/vtls/gtls.c \
    curl/lib/vtls/nss.c \
    curl/lib/vtls/openssl.c \
    curl/lib/vtls/polarssl.c \
    curl/lib/vtls/polarssl_threadlock.c \
    curl/lib/vtls/qssl.c \
    curl/lib/vtls/vtls.c \
    curl/lib/asyn-ares.c \
    curl/lib/asyn-thread.c \
    curl/lib/base64.c \
    curl/lib/bundles.c \
    curl/lib/conncache.c \
    curl/lib/connect.c \
    curl/lib/content_encoding.c \
    curl/lib/cookie.c \
    curl/lib/curl_addrinfo.c \
    curl/lib/curl_fnmatch.c \
    curl/lib/curl_gethostname.c \
    curl/lib/curl_gssapi.c \
    curl/lib/curl_memrchr.c \
    curl/lib/curl_multibyte.c \
    curl/lib/curl_ntlm.c \
    curl/lib/curl_ntlm_core.c \
    curl/lib/curl_ntlm_msgs.c \
    curl/lib/curl_ntlm_wb.c \
    curl/lib/curl_rtmp.c \
    curl/lib/curl_sasl.c \
    curl/lib/curl_sspi.c \
    curl/lib/curl_threads.c \
    curl/lib/dict.c \
    curl/lib/dotdot.c \
    curl/lib/easy.c \
    curl/lib/escape.c \
    curl/lib/file.c \
    curl/lib/fileinfo.c \
    curl/lib/formdata.c \
    curl/lib/ftp.c \
    curl/lib/ftplistparser.c \
    curl/lib/getenv.c \
    curl/lib/getinfo.c \
    curl/lib/gopher.c \
    curl/lib/hash.c \
    curl/lib/hmac.c \
    curl/lib/hostasyn.c \
    curl/lib/hostcheck.c \
    curl/lib/hostip.c \
    curl/lib/hostip4.c \
    curl/lib/hostip6.c \
    curl/lib/hostsyn.c \
    curl/lib/http.c \
    curl/lib/http2.c \
    curl/lib/http_chunks.c \
    curl/lib/http_digest.c \
    curl/lib/http_negotiate.c \
    curl/lib/http_negotiate_sspi.c \
    curl/lib/http_proxy.c \
    curl/lib/idn_win32.c \
    curl/lib/if2ip.c \
    curl/lib/imap.c \
    curl/lib/inet_ntop.c \
    curl/lib/inet_pton.c \
    curl/lib/krb5.c \
    curl/lib/ldap.c \
    curl/lib/llist.c \
    curl/lib/md4.c \
    curl/lib/md5.c \
    curl/lib/memdebug.c \
    curl/lib/mprintf.c \
    curl/lib/multi.c \
    curl/lib/netrc.c \
    curl/lib/non-ascii.c \
    curl/lib/nonblock.c \
    curl/lib/nwlib.c \
    curl/lib/nwos.c \
    curl/lib/openldap.c \
    curl/lib/parsedate.c \
    curl/lib/pingpong.c \
    curl/lib/pipeline.c \
    curl/lib/pop3.c \
    curl/lib/progress.c \
    curl/lib/rawstr.c \
    curl/lib/rtsp.c \
    curl/lib/security.c \
    curl/lib/select.c \
    curl/lib/sendf.c \
    curl/lib/share.c \
    curl/lib/slist.c \
    curl/lib/smtp.c \
    curl/lib/socks.c \
    curl/lib/socks_gssapi.c \
    curl/lib/socks_sspi.c \
    curl/lib/speedcheck.c \
    curl/lib/splay.c \
    curl/lib/ssh.c \
    curl/lib/strdup.c \
    curl/lib/strequal.c \
    curl/lib/strerror.c \
    curl/lib/strtok.c \
    curl/lib/strtoofft.c \
    curl/lib/telnet.c \
    curl/lib/tftp.c \
    curl/lib/timeval.c \
    curl/lib/transfer.c \
    curl/lib/url.c \
    curl/lib/version.c \
    curl/lib/warnless.c \
    curl/lib/wildcard.c \
    curl/lib/x509asn1.c \
    curl/lib/amigaos.c \
    MainWnd.cpp \
    OSSystemWrapper.cpp \
    swmgrapp_ui.cpp \
    swmgrapp_env.cpp \
    swmgrapp_IF.cpp \
    TaskManager.cpp \
    Storage.cpp \
    UserInfoManager.cpp \
    systemregset.cpp \
    checkuninstallsoftname.cpp \
    safetyreinforce.cpp \
    systemrightmenu.cpp \
    myregedit.cpp \
    browsertools.cpp \
    selfupdate.cpp \
    appenv.cpp \
    UpgradeData.cpp \
    UpgradeHandler.cpp \
    quicksettings.cpp \
    logwriter.cpp \
    character.cpp \
    mythread.cpp \
    Uninstallsoftware.cpp \
    uninstallsoftthread.cpp \
    WindowsTools.cpp \
    hdd.cpp \
    LaunchThread.cpp \
    ThreadToPullSoftwareList.cpp \
    GetPngThread.cpp

HEADERS += \
    swmgrapp.h \
    globalsingleton.h \
    DownWrapper.h \
    xldl.h \
    ConfOperation.h \
    SoftwareList.h \
    DataControl.h \
    PackageRunner.h \
    UserInfo.h \
    curl/include/curl/curl.h \
    curl/include/curl/curlbuild.h \
    curl/include/curl/curlrules.h \
    curl/include/curl/curlver.h \
    curl/include/curl/easy.h \
    curl/include/curl/mprintf.h \
    curl/include/curl/multi.h \
    curl/include/curl/stdcheaders.h \
    curl/lib/vtls/axtls.h \
    curl/lib/vtls/curl_darwinssl.h \
    curl/lib/vtls/curl_schannel.h \
    curl/lib/vtls/cyassl.h \
    curl/lib/vtls/gskit.h \
    curl/lib/vtls/gtls.h \
    curl/lib/vtls/nssg.h \
    curl/lib/vtls/openssl.h \
    curl/lib/vtls/polarssl.h \
    curl/lib/vtls/polarssl_threadlock.h \
    curl/lib/vtls/qssl.h \
    curl/lib/vtls/vtls.h \
    curl/lib/arpa_telnet.h \
    curl/lib/asyn.h \
    curl/lib/bundles.h \
    curl/lib/config-win32.h \
    curl/lib/conncache.h \
    curl/lib/connect.h \
    curl/lib/content_encoding.h \
    curl/lib/cookie.h \
    curl/lib/curl_addrinfo.h \
    curl/lib/curl_base64.h \
    curl/lib/curl_fnmatch.h \
    curl/lib/curl_gethostname.h \
    curl/lib/curl_gssapi.h \
    curl/lib/curl_hmac.h \
    curl/lib/curl_ldap.h \
    curl/lib/curl_md4.h \
    curl/lib/curl_md5.h \
    curl/lib/curl_memory.h \
    curl/lib/curl_memrchr.h \
    curl/lib/curl_multibyte.h \
    curl/lib/curl_ntlm.h \
    curl/lib/curl_ntlm_core.h \
    curl/lib/curl_ntlm_msgs.h \
    curl/lib/curl_ntlm_wb.h \
    curl/lib/curl_rtmp.h \
    curl/lib/curl_sasl.h \
    curl/lib/curl_sec.h \
    curl/lib/curl_setup.h \
    curl/lib/curl_setup_once.h \
    curl/lib/curl_share.h \
    curl/lib/curl_sspi.h \
    curl/lib/curl_threads.h \
    curl/lib/curlx.h \
    curl/lib/dict.h \
    curl/lib/dotdot.h \
    curl/lib/easyif.h \
    curl/lib/escape.h \
    curl/lib/file.h \
    curl/lib/fileinfo.h \
    curl/lib/formdata.h \
    curl/lib/ftp.h \
    curl/lib/ftplistparser.h \
    curl/lib/getinfo.h \
    curl/lib/gopher.h \
    curl/lib/hash.h \
    curl/lib/hostcheck.h \
    curl/lib/hostip.h \
    curl/lib/http.h \
    curl/lib/http2.h \
    curl/lib/http_chunks.h \
    curl/lib/http_digest.h \
    curl/lib/http_negotiate.h \
    curl/lib/http_proxy.h \
    curl/lib/if2ip.h \
    curl/lib/imap.h \
    curl/lib/inet_ntop.h \
    curl/lib/inet_pton.h \
    curl/lib/llist.h \
    curl/lib/memdebug.h \
    curl/lib/multihandle.h \
    curl/lib/multiif.h \
    curl/lib/netrc.h \
    curl/lib/non-ascii.h \
    curl/lib/nonblock.h \
    curl/lib/parsedate.h \
    curl/lib/pingpong.h \
    curl/lib/pipeline.h \
    curl/lib/pop3.h \
    curl/lib/progress.h \
    curl/lib/rawstr.h \
    curl/lib/rtsp.h \
    curl/lib/select.h \
    curl/lib/sendf.h \
    curl/lib/sigpipe.h \
    curl/lib/slist.h \
    curl/lib/smtp.h \
    curl/lib/sockaddr.h \
    curl/lib/socks.h \
    curl/lib/speedcheck.h \
    curl/lib/splay.h \
    curl/lib/ssh.h \
    curl/lib/strdup.h \
    curl/lib/strequal.h \
    curl/lib/strerror.h \
    curl/lib/strtok.h \
    curl/lib/strtoofft.h \
    curl/lib/telnet.h \
    curl/lib/tftp.h \
    curl/lib/timeval.h \
    curl/lib/transfer.h \
    curl/lib/url.h \
    curl/lib/urldata.h \
    curl/lib/warnless.h \
    curl/lib/wildcard.h \
    curl/lib/x509asn1.h \
    curl/lib/amigaos.h \
    MainWnd.h \
    global.h \
    OSSystemWrapper.h \
    TaskManager.h \
    Storage.h \
    UserInfoManager.h \
    checkuninstallsoftname.h \
    systemregset.h \
    browsertools.h \
    myregedit.h \
    safetyreinforce.h \
    systemrightmenu.h \
    selfupdate.h \
    appenv.h \
    UpgradeData.h \
    UpgradeHandler.h \
    quicksettings.h \
    logwriter.h \
    character.h \
    DataStruct.h \
    mythread.h \
    Uninstallsoftware.h \
    uninstallsoftthread.h \
    Windowstools.h \
    hdd.h \
    LaunchThread.h \
    ThreadToPullSoftwareList.h \
    GetPngThread.h
#FORMS   +=

#######
#OTHER_FILES +=

RC_FILE += icon.rc

RESOURCES += lewang/default.qrc

#######
#MSIC PROCESS

message(----------- QT INFORMATION -----------)
message(Qt version: $$[QT_VERSION])
message(Qt is installed in $$[QT_INSTALL_PREFIX])
message(Header files: $$[QT_INSTALL_HEADERS])
message(Libraries: $$[QT_INSTALL_LIBS])
message(Binary files (executables): $$[QT_INSTALL_BINS])
message(Plugins: $$[QT_INSTALL_PLUGINS])
message(Data files: $$[QT_INSTALL_DATA])
message(Translation files: $$[QT_INSTALL_TRANSLATIONS])
message(Settings: $$[QT_INSTALL_CONFIGURATION])
message(----------- PROJECT INFORMATION -----------)
message(MAKE QRC TO BINRARY RCC FILE: $$system(makercc.bat))
#message(COPY /y \"$$PWD/*.res\" \"$$OUT_PWD/release\")
#CONFIG(debug, debug|release) {
#    message(COPY RCC FILE TO BUILD DIR: $$system(COPY /y \"$$PWD/*.res\" \"$$OUT_PWD/debug\"))
#} else {
#    message(COPY RCC FILE TO BUILD DIR: $$system(COPY /y \"$$PWD/*.res\" \"$$OUT_PWD/release\"))
#}
