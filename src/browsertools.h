#ifndef BROWSERTOOLS
#define BROWSERTOOLS
#include<QString>
//设置IE主页；
int SetHomePage(QString strHomePageURL);

//设置默认浏览器；
int SetDefaultBrowser(QString strbrowserPath);

//设置IE的默认搜索引擎；
int SetSearchEngine(QString strSearchEngineName, QString strSearchEngineURL);

//设置IE右键菜单
int SetBrowserMenu(int flag, QString strMenuName, QString strFileName);

#endif // BROWSERTOOLS

