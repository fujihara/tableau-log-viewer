#include "filesearcher.h"

#include <QFuture>
#include <QtConcurrent>

FileSearcherImpl::FileSearcherImpl(
    QObject *parent)
    : QObject{parent}
{}

QStringList FileSearcherImpl::getFileList(const QFileSystemModel &model)
{
    QStringList filePaths;
    QModelIndex rootIndex = model.index(model.rootPath());

    for (int i = 0; i < model.rowCount(rootIndex); ++i) {
        QModelIndex index = model.index(i, 0, rootIndex);
        if (model.isDir(index))
            continue;
        filePaths << model.filePath(index);
    }

    return filePaths;
}

QList<SearchResult> FileSearcherImpl::searchInFile(const QString &filePath, const QString &searchString)
{
    QFile file(filePath);
    QFileInfo fileInfo(file.fileName());
    QList<SearchResult> results;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return results;
    }

    QTextStream in(&file);
    int lineNumber = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        ++lineNumber;

        if (line.contains(searchString, Qt::CaseInsensitive)) {
            SearchResult result{fileInfo.fileName(), filePath, lineNumber, line};
            results.append(result);
        }
    }

    return results;
}

QList<SearchResult> FileSearcherImpl::searchInFiles(const QStringList &filePaths, const QString &searchString)
{
    QList<SearchResult> allResults;

    for (const QString& filePath : filePaths) {
        QList<SearchResult> fileResults = searchInFile(filePath, searchString);
        allResults.append(fileResults);
    }

    return allResults;
}

QList<SearchResult> FileSearcherImpl::searchInFilesConcurrently(
    const QStringList &filePaths, const QString &searchString)
{
    auto searchTask = [&searchString, this](const QString& filePath) {
        return searchInFile(filePath, searchString);
    };

    QFuture<QList<SearchResult>> results = QtConcurrent::mapped(filePaths, searchTask);
    QList<SearchResult> allResults;

    for (const QList<SearchResult>& resultList : results) {
        allResults.append(resultList);
    }

    return allResults;
}
