#include "CommandLine.h"
#include <iostream>

CommandLine::CommandLine(QObject *parent) : QObject(parent)
{
    bMode = false;
    _id="";
    _category="";
    _launchName="";
    _autoInstall = true;
    _status = true;
}

bool CommandLine::launchMode(){
    return bMode;
}

QString CommandLine::launchID(){
    return _id;
}

QString CommandLine::launchCategory() {
    return _category;
}

QString CommandLine::launchName() {
    return _launchName;
}

bool CommandLine::launchAutoinstall() {
    return _autoInstall;
}

bool CommandLine::getStatus() {
    return _status;
}


QVariantMap CommandLine::encodeToVariantMap() {
    QVariantMap object;
    object.insert("id",QVariant::fromValue(_id));
    object.insert("category",QVariant::fromValue(_category));
    object.insert("launchName",QVariant::fromValue(_launchName));
    object.insert("autoInstall",QVariant::fromValue(_autoInstall));
    object.insert("status",QVariant::fromValue(_status));
    return object;
}

int CommandLine::parseCommandLine(QStringList commandLine) {
    if (commandLine.size() == 2) {
        if (commandLine.at(1).trimmed().compare("launch", Qt::CaseInsensitive) == 0) {
            bMode = false;// launch application
        }
        else {
            std::cout << "arguments error!" << std::endl;
            return -1;
        }
    }
    else if (commandLine.size() < 5) {
        std::cout << "arguments error!" << std::endl;
        return -1;
    }
    else {
        if (commandLine.at(1).trimmed().compare("install",Qt::CaseInsensitive)==0) {
            QStringList szIDInfo = commandLine.at(2).split(QChar('='), QString::SkipEmptyParts, Qt::CaseInsensitive);
            QStringList szCategory  = commandLine.at(3).split(QChar('='), QString::SkipEmptyParts, Qt::CaseInsensitive);

            if (szIDInfo.size()== 2 && szIDInfo.at(0).compare("id",Qt::CaseInsensitive)==0 &&
                szCategory.size() == 2 && szCategory.at(0).compare("category", Qt::CaseInsensitive) == 0
                ) {
                _id = szIDInfo.at(1).trimmed();
                _category=szCategory.at(1).trimmed();
                _launchName = commandLine.at(4).trimmed();

                bMode = true;
            }
            else {
                std::cout << "arguments error!" << std::endl;
                return -1;
            }
        }
        else {
            std::cout << "arguments error!" << std::endl;
            return -1;
        }
    }
    return 0;
}
