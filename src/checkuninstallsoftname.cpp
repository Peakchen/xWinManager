#include "checkuninstallsoftname.h"
#include "swmgrapp.h"
#include "OSSystemWrapper.h"


const QString checkUnString1[]={
//" --","PDApp.exe",    "0","1"," ",
'\0',   "msiexec /",    '7',"1"," ",
"}.sdb",".exe -u",      '4',"1"," ",
'\0',   ".exe\"",       '4',"1"," /",
'\0',   ".exe ",        '4',"3"," /, --, -",
'\0',   ".exe",         '4',"1"," ",
'\0',".dll",'4','1',' ',
'\0',".ico",'4','1',' ',
'\0',".ime",'4','1',' ',
0,      0,
};




CheckUninstallSoftName::CheckUninstallSoftName()
{
    checkUnString=(PQString)checkUnString1;

}


CheckUninstallSoftName::~CheckUninstallSoftName()
{

}



int CheckUninstallSoftName::Getidx(int m)
{

    int checknum=0;
    m--;
    while(m>=0)
    {
        checknum+=checkUnString[m*ROLNUM+3].toInt();
        m--;

    }

    return checknum;
}

void CheckUninstallSoftName::GetArgList(QStringList &arguments,QString &res_arg,QString &find_str)
{
    if(res_arg.isEmpty())return;
    int idx=res_arg.lastIndexOf(find_str);
    arguments.push_front(res_arg.right(res_arg.size()-idx).trimmed());
    if(idx<0){
        res_arg.clear();
        return;
    }
    res_arg=res_arg.left(idx);
    GetArgList(arguments,res_arg,find_str);

}


bool CheckUninstallSoftName::GetUninstallNameArg(QString &name,QStringList &arguments)
{
    int i=0,idx,movnum;
    QStringList str;
    QString argg;

    name=name.trimmed();//用以删除字符串两边的空白字符(注意，空白字符包括空格、Tab以及换行符，而不仅仅是空格)
    if(name.at(0)=='\"'){
        if(name.count('\"')<=2){
            name.remove(0,1);
        }
    }

    do
    {
        if(checkUnString[i*ROLNUM]==0)idx=0;
        else idx=name.lastIndexOf(checkUnString[i*ROLNUM]);

        if(idx>=0){
            idx=name.lastIndexOf(checkUnString[i*ROLNUM+1]);
            if(idx>=0){
                movnum=checkUnString[i*ROLNUM+2].toInt();
                if(!movnum)break;
                str=checkUnString[i*ROLNUM+4].split(QString(","), QString::SkipEmptyParts, Qt::CaseInsensitive);

                if(str.size()!=checkUnString[i*ROLNUM+3].toInt()){
                    qDebug()<<"strings split error ";

                }
                break;
            }
        }
        i++;
    }while(checkUnString[i*ROLNUM+1]!=0);

    if(idx==-1)return false;

    if(movnum){
        argg=name.right(name.size()-idx-movnum);
        name=name.left(idx+movnum);
    }else{
        return true;
    }

    for(QStringList::Iterator it=str.begin();it!=str.end();it++){
        if(argg.contains(*it)){
            GetArgList(arguments,argg,*it);
        }
    }
    return true;
}



//============================================================================CGetIconFromFile=======================
//============================================================================CGetIconFromFile=======================


bool CGetIconFromFile::finishFind=false;

void CGetIconFromFile::SetFinishFind_Finish()
{
//    finishFind=true;
}


int test;
CGetIconFromFile::CGetIconFromFile(QObject *parent) : QObject(parent)
{
    test=0;
    checkUnString=(PQString)checkUnString1;
    runRoot = QCoreApplication::applicationDirPath();
    runRoot += QObject::tr("/png/");


    if(!dir.exists(runRoot)){
        dir.mkpath(runRoot);
    }
    ItemUnstall=new QStringListMap;

    ReadSoftInfo();

}

CGetIconFromFile::~CGetIconFromFile()
{
    delete ItemUnstall;

}

void CGetIconFromFile::convertsystemdir(QString &src_name)
{
    if(src_name.contains("%windir%")){
        wchar_t windir[100];
        GetWindowsDirectory(windir,100);
        src_name.replace("%windir%",QString::fromWCharArray(windir));
    }

}

void CGetIconFromFile::remove_no_used_symbol(QString &name)
{
    name=name.trimmed();

    if(name.at(0)=='\"'){
        if(name.count('\"')<=2){

            QString::iterator it=name.end();
            it--;

            if(*it=='\"'){
                *it=0;
            }
            name.remove(0,1);
        }
    }
}

bool CGetIconFromFile::GetFilename(QString &src_name,QString &path,QString &filename,QString &suffix,QString &arg)
{
//    QFileInfo info(itExe);
//    suffix=info.completeSuffix();
//    int idx=suffix.indexOf(' ');
//    suffix=suffix.left(idx);
//    path=info.absolutePath()+QLatin1Char('/');
//    filename=info.baseName()+QLatin1Char('.')+suffix;


    int i=0,idx=0,movnum;

    src_name=src_name.trimmed();

    if(src_name.isEmpty())
        return false;
    if(test==101){
        test=101;
    }
    //有时候会有双引号，要去掉
    if(src_name.at(0)=='\"'){
        if(src_name.count('\"')<=2){
            src_name.remove(0,1);
        }
    }

    convertsystemdir(src_name);


    do
    {
        if(checkUnString[i*ROLNUM]==0)idx=0;
        else idx=src_name.lastIndexOf(checkUnString[i*ROLNUM]);

        if(idx>=0){
            idx=src_name.lastIndexOf(checkUnString[i*ROLNUM+1]);
            if(idx>=0){
                movnum=checkUnString[i*ROLNUM+2].toInt();
                if(!movnum)break;

                break;
            }
        }
        i++;
    }while(checkUnString[i*ROLNUM+1]!=0);

    if(idx==-1)return false;

    if(movnum){
        arg=src_name.right(src_name.size()-idx-movnum);
        src_name=src_name.left(idx+movnum);
        filename=src_name;
        idx=filename.lastIndexOf('.')+1;
        suffix=filename.right(filename.size()-idx);
        //filename=filename.left(idx-1);

        idx=filename.lastIndexOf('\\')+1;
        if(idx>0){
            path=filename.left(idx);
            filename=filename.right(filename.size()-idx);

        }else path="";


    }else{
        return true;
    }


    return true;

}


bool CGetIconFromFile::GetFileNameList(QVariantMap &objParameter)
{

    if(finishFind)return 1;
    QVariant &itflag=objParameter["UnIcoFlag"];
    QVariant &itId=objParameter["UnIcoID"];
    QString &itExe=objParameter["UnIcoPath"].toString();
    QString &itUnString=objParameter["UninstallString"].toString();
    bool used_unstring=false;

    QString name;
    QString unstring;
    QString path;
    QString suffix;
    QString arg;
    int flag;
    test++;

    if(test==10){
        test=10;
    }

    GetFilename(itExe,path,name,suffix,arg);
    fullpathfile=path+name;
    flag=0;
    if(!itExe.isEmpty()){
      if(!dir.exists(fullpathfile)){
        GetFilename(itUnString,path,name,suffix,arg);
        if(!itUnString.isEmpty()){
            fullpathfile=path+name;
            if(!dir.exists(fullpathfile)){
                itflag=1;
                flag=1;
                arg=arg.section(QRegExp("[{}]"),1,1,QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep);
                if(arg.isEmpty()){
                    itId="default.png";
                }else used_unstring=true;
            }else{
                used_unstring=true;
            }
        }else{
            itId="default.png";
            itflag=1;//
        }

      }else{
          //处理exe,dll ,ico文件
          mult_type_file_icon(itId,itflag ,suffix,objParameter);
      }

    }else{
        GetFilename(itUnString,path,name,suffix,arg);
        if(!itUnString.isEmpty()){
            fullpathfile=path+name;
            if(!dir.exists(fullpathfile)){
                itflag=1;
                flag=1;
                arg=arg.section(QRegExp("[{}]"),1,1,QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep);
                if(arg.isEmpty()){
                    itId="default.png";
                }else used_unstring=true;
            }else{
                used_unstring=true;
            }
        }else{
            itId="default.png";
            itflag=1;
        }
    }

    if(used_unstring){
        //qDebug()<<test<<" used_unstring"<<path+name;
        wchar_t windir[100];
        GetWindowsDirectory(windir,100);

        int ret;
        if(!flag)arg=arg.section(QRegExp("[{}]"),1,1,QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep);

        if(arg.isEmpty()){
            ret=0;
        }else{

            ret=AnalyzeDealWithAllFile(QString::fromWCharArray(windir)+"\\installer\\"+arg);
        }

        if(ret>0){//有合适的
            //处理exe,dll ,ico文件
            if(ret==1){//ico
                itflag=3;
                mult_type_file_icon(itId,itflag ,QString("ico"),objParameter);
                itflag=3;
            }else if(ret==2){//exe dll

                mult_type_file_icon(itId,itflag ,QString("exe"),objParameter);
            }

        }else {//没有合适的
            if(!flag)objParameter["UnIcoPath"]=path+name;
            else itId="default.png";
        }
    }
//    if(itflag.toInt()){
//        qDebug()<<test<<" default"<<path+name;;
//    }

    return 0;
}

void CGetIconFromFile::DealWithUnstallString(QString )
{

}
//处理文件夹中的图标文件
int CGetIconFromFile::AnalyzeDealWithAllFile(QString installer)
{
    if(dir.exists(installer)){
        dir.setPath(installer);
        dir.setFilter(QDir::AllEntries|QDir::NoDotAndDotDot);
//        QFileInfoList list= dir.entryInfoList();
        QDirIterator iter(dir,QDirIterator::Subdirectories);
        QFileInfo info;
        QString Suffix;

        while (iter.hasNext())
        {
            iter.next();
            info=iter.fileInfo();
            Suffix=info.completeSuffix();

            if(Suffix=="" || Suffix=="ico"){
                fullpathfile=info.absoluteFilePath();
                return 1;
            }else if(Suffix=="exe" || Suffix=="dll"){
                fullpathfile=info.absoluteFilePath();
                return 2;
            }
        }


    }

    return 0;
}

void CGetIconFromFile::mult_type_file_icon(QVariant &itId,QVariant &itflag ,QString &suffix,QVariantMap &objParameter)
{
    if(suffix=="exe" || suffix=="dll"){
        objParameter["UnIcoPath"]=fullpathfile;
        //qDebug()<<test<<"exe"<<fullpathfile;
        itflag=0;
    }else if(suffix=="ico"){
        //qDebug()<<test<<" ico"<<fullpathfile;

        QString imageFileRoot=runRoot+itId.toString();
        if(!dir.exists(imageFileRoot)){


            QPixmap pixmap=QPixmap(fullpathfile);
            if(pixmap.width()<128){
                file.copy(fullpathfile,imageFileRoot);
            }else{
                int ret=pixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::FastTransformation).save(imageFileRoot);
                if (!ret) {
                    //std::cout << "Error writing image file " << fileName.toStdString() << ".\n";
                    qDebug() <<    "GetFileNameList ico Save icon    failed!!";
                }
            }

        }

//      itId=name;
        itflag=2;
    }else{
        itId="default.png";
        itflag=1;
    }
}

bool CGetIconFromFile::SaveOneIcon(QVariantMap &map)
{
    if(finishFind)return 1;

    if(map["UnIcoFlag"].toInt()>0){
        return 1;
    }

    //检测文件是否存在

    QString imageFileRoot=runRoot+map["UnIcoID"].toString();//QFileInfo(sourceFile).baseName();


    if(dir.exists(imageFileRoot)){
        return 1;//exist no use save;
    }

    const QString &sourceFile = map["UnIcoPath"].toString();

      const bool large=1;

      //const UINT iconCount = ExtractIconEx((wchar_t *)sourceFile.utf16(), -1, 0, 0, 0);
      const UINT iconCount=1;

      if (!iconCount){
          std::cout << sourceFile.toStdString() << " does not appear to contain icons.\n";
          map["UnIcoID"]="default.png";
          qDebug() <<"iconCount is zero.\n";
          return 0;
      }

      QScopedArrayPointer<HICON> icons(new HICON[iconCount]);

      const UINT extractedIconCount = large ?
          ExtractIconEx((wchar_t *)sourceFile.utf16(), 0, icons.data(), 0, iconCount) :
          ExtractIconEx((wchar_t *)sourceFile.utf16(), 0, 0, icons.data(), iconCount);
      if (!extractedIconCount) {
          //qErrnoWarning("Failed to extract icons from %s", qPrintable(sourceFile));
          qDebug() <<"extractedIconCount is zero.\n";
          map["UnIcoID"]="default.png";
          return 0;
      }

      std::cout << sourceFile.toStdString() << " contains " << extractedIconCount << " icon(s).\n";



//       QString fileName;
      QPixmap pixmap;


      //for (UINT i = 0; i < extractedIconCount; ++i)
      for (UINT i = 0; i < 1; ++i)
      {
         // fileName = QString::fromLatin1("%1%2.png").arg(imageFileRoot).arg(i, 3, 10, QLatin1Char('0'));

          pixmap = QtWin::fromHICON(icons[i]);
          if (pixmap.isNull()) {
              //std::cerr << "Error converting icons.\n";
              qDebug() <<"Error converting icons.\n";
              map["UnIcoID"]="default.png";
              return 0;
          }

          if (!pixmap.save(imageFileRoot)) {
              //std::cout << "Error writing image file " << fileName.toStdString() << ".\n";
              qDebug() <<    "Save icon    failed!!";
              map["UnIcoID"]="default.png";
              return 0;
          }

          qDebug()<<imageFileRoot;
          std::cout << sourceFile.toStdString()<<   "Save icon    succeeded!!!";
          //std::cout << "Wrote image file "<< QDir::toNativeSeparators(fileName).toStdString() << ".\n";
          ::DestroyIcon(icons[i]);

      }
      return 1;
}

bool CGetIconFromFile::SaveIconFile(QVariantList &jsArray)
{
    int k=0;
    for(QVariantList::Iterator it=jsArray.begin();it!=jsArray.end();it++){
        QVariantMap &map=(*it).toMap();
        k++;
        if(map["UnIcoFlag"].toInt()>0){
            continue;
        }

        //检测文件是否存在

        QString imageFileRoot=runRoot+map["UnIcoID"].toString();//QFileInfo(sourceFile).baseName();


        if(dir.exists(imageFileRoot)){
            continue;//exist no use save;
        }

        const QString &sourceFile = map["UnIcoPath"].toString();

          const bool large=1;

          //const UINT iconCount = ExtractIconEx((wchar_t *)sourceFile.utf16(), -1, 0, 0, 0);
          const UINT iconCount=1;

          if (!iconCount){
              std::cout << sourceFile.toStdString() << " does not appear to contain icons.\n";

              qDebug() <<"iconCount is zero.\n";
              continue;
          }

          QScopedArrayPointer<HICON> icons(new HICON[iconCount]);

          const UINT extractedIconCount = large ?
              ExtractIconEx((wchar_t *)sourceFile.utf16(), 0, icons.data(), 0, iconCount) :
              ExtractIconEx((wchar_t *)sourceFile.utf16(), 0, 0, icons.data(), iconCount);
          if (!extractedIconCount) {
              //qErrnoWarning("Failed to extract icons from %s", qPrintable(sourceFile));
              qDebug() <<"extractedIconCount is zero.\n";
              continue;
          }

          std::cout << sourceFile.toStdString() << " contains " << extractedIconCount << " icon(s).\n";



   //       QString fileName;
          QPixmap pixmap;


          //for (UINT i = 0; i < extractedIconCount; ++i)
          for (UINT i = 0; i < 1; ++i)
          {
             // fileName = QString::fromLatin1("%1%2.png").arg(imageFileRoot).arg(i, 3, 10, QLatin1Char('0'));

              pixmap = QtWin::fromHICON(icons[i]);
              if (pixmap.isNull()) {
                  //std::cerr << "Error converting icons.\n";
                  qDebug() <<"Error converting icons.\n";
                  continue;
              }

              if (!pixmap.save(imageFileRoot)) {
                  //std::cout << "Error writing image file " << fileName.toStdString() << ".\n";
                  qDebug() <<    "Save icon    failed!!";
                  break;
              }
              qDebug()<<k;
              qDebug()<<imageFileRoot;
              std::cout << sourceFile.toStdString()<<   "Save icon    succeeded!!!"<<k;
              //std::cout << "Wrote image file "<< QDir::toNativeSeparators(fileName).toStdString() << ".\n";
              ::DestroyIcon(icons[i]);

          }
    }

    return 1;
}



void CGetIconFromFile::CalcFolersize1(QStringList &list)
{
    CgetSoftSize=new CGetFolderSize(list);
    thread_calc=new QThread;

    CgetSoftSize->moveToThread(thread_calc);
    QObject::connect(thread_calc, SIGNAL(started()), CgetSoftSize, SLOT(get()),Qt::QueuedConnection);
    QObject::connect(CgetSoftSize, SIGNAL(result_ready()), this, SLOT(GetFolersize()),Qt::QueuedConnection);
//    QObject::connect(thread_calc, SIGNAL(finished()), thread_calc, SLOT(deleteLater()),Qt::QueuedConnection);
    thread_calc->start();
}


void CGetIconFromFile::calcfoler(QString &foler,QString &size,QString &date,int flag)
{

     if (foler.isEmpty())
     {
         return;
     }

     dir.setPath(foler);
     dir.setFilter(QDir::AllEntries|QDir::NoDotAndDotDot);
     //       dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|
     //                          QDir::Files | QDir::Hidden | QDir::NoSymLinks);
     //dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式（后缀为.jpeg等图片格式）
     QDirIterator iter(dir,QDirIterator::Subdirectories);


     if(flag==2 || flag==3){
         QFileInfo info(foler);
         date=info.created().date().toString(Qt::DateFormat::ISODate);
     }

     if(flag==2)return;

     int i=0;
     quint64 _size=0;


     while (iter.hasNext())
     {
         iter.next();
         i++;
         _size+=iter.fileInfo().size();

     }


     size = QString("%1").arg((float)_size/1024);

}

void CGetIconFromFile::GetFolersize(QVariantMap &objParameter)
{
    QVariant &date=objParameter["InstallDate"];
    QVariant &size=objParameter["EstimatedSize"];
    QString MD5_ID=objParameter["uninstallID"].toString();
    QString InstallLocat=objParameter["InstallLocation"].toString();

    if(test==3){
        test=3;
    }

    QString tt0=objParameter["DisplayName"].toString();
//    QString tt1=objParameter["DisplayIcon"].toString();
//    QString tt2=objParameter["UninstallString"].toString();

    if(InstallLocat.isEmpty()){
        int empty=0;

        InstallLocat=objParameter["UnIcoPath"].toString();

        if(InstallLocat.isEmpty()){
            empty=1;
        }else{
            QFileInfo info(InstallLocat);
            InstallLocat=info.absoluteDir().path();
            if(InstallLocat.isEmpty()){
                empty=1;
            }
        }

        if(empty){
            InstallLocat=objParameter["UninstallString"].toString();

            if(!InstallLocat.isEmpty()){
                QFileInfo info(InstallLocat);
                InstallLocat=info.absoluteDir().path();
                if(InstallLocat.isEmpty()){
                    return;
                }
            }else{
               return;
            }
        }



    }

    remove_no_used_symbol(InstallLocat);
    convertsystemdir(InstallLocat);

    int flag=0;
    int add=0;

    if(size.isNull()){
        flag|=1;
    }

    if(date.isNull()){
        flag|=2;
    }

    if(!flag)return;


    QString _date="",_size="";


    if(!softSizeDate.isEmpty()){

        if(flag==1){//size
            int i=softSizeDate.indexOf(MD5_ID);

            if(i>=0){
                const QString &str=softSizeDate.at(i+1);
                if(str.isEmpty()){
                    calcfoler(InstallLocat,_size,_date,flag);
                    size=_size;
                    if(_size!=str){
                        softSizeDate.replace(i+1,_size);
                        saveflag=1;
                    }
                }else{
                    size=str;
                }
            }else{
                add=1;
            }
        }
        if(flag==2){
            int i=softSizeDate.indexOf(MD5_ID);
            if(i>=0){
                const QString &str=softSizeDate.at(i+2);
                if(str.isEmpty()){
                    calcfoler(InstallLocat,_size,_date,flag);
                    date=_date;
                    if(_date!=str){
                        softSizeDate.replace(i+2,_date);
                        saveflag=1;
                    }
                }else{

                    date=str;

                }
            }else{
                add=1;
            }
        }
        if(flag==3){
            int i=softSizeDate.indexOf(MD5_ID);
            if(i>=0){
                const QString &str=softSizeDate.at(i+1);
                if(str.isEmpty()){
                    calcfoler(InstallLocat,_size,_date,flag);
                    size=_size;
                    if(_size!=str){
                        softSizeDate.replace(i+1,_size);
                        saveflag=1;
                    }
                }else{
                    size=str;
                }

                const QString &str1=softSizeDate.at(i+2);
                if(str1.isEmpty()){
                    calcfoler(InstallLocat,_size,_date,flag);
                    date=_date;
                    if(_date!=str1){
                        softSizeDate.replace(i+2,_date);
                        saveflag=1;
                    }
                }else{
                    date=str1;
                }
            }else{
                add=1;
            }
        }
    }else{
        add=1;
    }

    if(add){

        if(flag&3){
            calcfoler(InstallLocat,_size,_date,flag);
        }
        if(flag&1){
            size=_size;
        }
        if(flag&2){
            date=_date;
        }

        softSizeDate.append(MD5_ID);
        softSizeDate.append(size.toString());
        softSizeDate.append(date.toString());

        saveflag=1;
    }

}

void CGetIconFromFile::ReadSoftInfo()
{
    softSizeDate.clear();

    QFile file1(runRoot+"\\filefoldersize.dat");
    if (file1.open(QIODevice::ReadOnly)){
        QDataStream in(&file1);
        in.setVersion(QDataStream::Qt_5_5);
        in >> softSizeDate;
        file1.close();
    }

    saveflag=0;
}

void CGetIconFromFile::SaveSoftInfo()
{
    //保存
    if(saveflag){
        QFile file(runRoot+"\\filefoldersize.dat");
        int ret=file.open(QIODevice::WriteOnly);
        if(ret){
            QDataStream out(&file);   // we will serialize the data into the file
            out.setVersion(QDataStream::Qt_5_5);
            out <<softSizeDate;   // serialize a string
            //out << (qint32)42;        // serialize an integer
            file.close();
        }
        saveflag=0;
    }
}

void CGetIconFromFile::GetFolersize1()
{

    //CgetSoftSize->FolderSizeMap[""];
    //保存

    QFile file(runRoot+"\\filefoldersize.dat");
    int ret=file.open(QIODevice::WriteOnly);
    if(ret){
        QDataStream out(&file);   // we will serialize the data into the file
        out.setVersion(QDataStream::Qt_5_5);
        out <<CgetSoftSize->FolderSizeMap;   // serialize a string
        //out << (qint32)42;        // serialize an integer
        file.close();
    }

    QStringMap Stringmap;

    for(ItemSize::iterator it=CgetSoftSize->FolderSizeMap.begin();it!=CgetSoftSize->FolderSizeMap.end();it++){
        for(QStringListMap::iterator item=ItemUnstall->begin();item!=ItemUnstall->end();item++){

            //(*item)[it->first]=CgetSoftSize->FolderSizeMap[it->first].tostring;
        }
    }

    thread_calc->quit();
    delete CgetSoftSize;
}
//============================================================================GetFolderSize=======================
//============================================================================GetFolderSize=======================


CGetFolderSize::CGetFolderSize(QStringList &list,QObject *parent) : QObject(parent)
{
    folderlist=list;
    stopped=false;
}

void CGetFolderSize::stop()
{
    stopped=true;
}

void CGetFolderSize::get()
{

    //读取
    QString runRoot = QCoreApplication::applicationDirPath();

    QFile file1(runRoot+"\\filefoldersize.dat");
    if (file1.open(QIODevice::ReadOnly)){
        QDataStream in(&file1);
        in.setVersion(QDataStream::Qt_5_5);
        in >> FolderSizeMap;
        file1.close();
    }

    // while(!stopped)
    //    {
    //qDebug()<<QString("this is thread %1").arg(i++);
    for(QStringList::iterator it=folderlist.begin();it!=folderlist.end();it++){
        folorSize=0;
        calc(*it);
        FolderSizeMap.insert("MD5",folorSize);
        //  QThread::msleep(10);
    }


    //    }

    emit result_ready();

    stopped=false;
};



quint64 CGetFolderSize::calc(QString &foler)
{
//    QStringList list1;
//    process->waitForStarted(100);
//    QProcess *process1=new QProcess();
//    process1->start("calc",list1);

     if (foler.isEmpty())
     {
         return 0;
     }

     dir.setPath(foler);
     dir.setFilter(QDir::AllEntries|QDir::NoDotAndDotDot);
     //       dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|
     //                          QDir::Files | QDir::Hidden | QDir::NoSymLinks);
     //dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式（后缀为.jpeg等图片格式）
     QDirIterator iter(dir,QDirIterator::Subdirectories);
     QFileInfo info;
     int i=0;

     while (iter.hasNext())
     {
         iter.next();
         i++;
         folorSize+=iter.fileInfo().size();

     }

     return folorSize;

}



