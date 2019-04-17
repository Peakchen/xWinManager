#include "ConfOperation.h"

ConfOperation::ConfOperation()
{

}

ConfOperation &ConfOperation::Root() {
	static ConfOperation _root;
	return _root;
}

void ConfOperation::setRootPath(QString szRoot) {
	rootDir.mkpath(szRoot);
	rootDir.setCurrent(szRoot);
}
QString ConfOperation::getRootPath() {
	return rootDir.absolutePath();
}

void ConfOperation::initSubpath(QStringList lstItems) {
	QDir dir;

	foreach(QString item, lstItems) {
		QString szSubpath = rootDir.absolutePath();
		szSubpath.append(QDir::separator());
		szSubpath.append(item);
		szSubpath = QDir::toNativeSeparators(szSubpath);
		dir.mkpath(szSubpath);
	}
}

QString ConfOperation::getSubpath(QString szSubPath) {
	QDir _dir = rootDir;
	_dir.mkdir(szSubPath);
	_dir.cd(szSubPath);
	return _dir.absolutePath();
}

QString ConfOperation::getSubpathFile(QString szSubPath, QString szFile) {
	QString absFile = getSubpath(szSubPath);
	absFile = absFile + QDir::separator() + szFile;
	return QDir::toNativeSeparators(absFile);
}
