#ifndef CONFOPERATION_H
#define CONFOPERATION_H
#include <QString>
#include <QDir>

class ConfOperation
{
protected:
	ConfOperation();
public:
	static ConfOperation &Root();
public:
	void setRootPath(QString);
	QString getRootPath();

	void initSubpath(QStringList);
	QString getSubpath(QString);
	QString getSubpathFile(QString,QString);
protected:
	QDir rootDir;
};

#endif // CONFOPERATION_H
