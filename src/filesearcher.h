#ifndef FILESEARCHER_H
#define FILESEARCHER_H

#include <QObject>
#include <QFileSystemModel>
#include "singleton.h"

struct SearchResult
{
    QString fileName;
    QString filePath;
    int lineNumber;
    QString lineContent;
};

class FileSearcherImpl: public QObject
{
    Q_OBJECT

private:


public:
    explicit FileSearcherImpl(QObject *parent = nullptr);

    QStringList getFileList(const QFileSystemModel &model);
    QList<SearchResult> searchInFile(const QString &filePath, const QString &searchString);
    QList<SearchResult> searchInFiles(const QStringList &filePaths, const QString &searchString);
    QList<SearchResult> searchInFilesConcurrently(const QStringList &filePaths, const QString &searchString);

signals:
};

//Global variable
typedef Singleton<FileSearcherImpl> FileSearcher;

#endif // FILESEARCHER_H
