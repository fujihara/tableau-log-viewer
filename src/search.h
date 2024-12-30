#ifndef SEARCH_H
#define SEARCH_H

#pragma once

#include <QtCore>
#include <QFileSystemModel>
#include <QStandardItemModel>

namespace LogSearch {


enum Scope : short { Folder, CurrentFile, AllOpened };

enum Mode : short { Equals, Contains, StartsWith, EndsWith, Regex };

enum Status : short { New, Searching, Canceling, Canceled, Finished, Error };

struct SearchParameters
{
    QString searchString;
    Mode mode;
    Scope scope;
    QStringList files;
    bool caseSensitive = false;
};

struct Match
{
    int lineNumber;
    QString lineContent;
};

class FileSearchResults : public QObject
{
    Q_OBJECT

public:
    explicit FileSearchResults(
        QString fileName, QString filePath, QObject *parent = nullptr)
        : QObject{parent}
        , fileName(fileName)
        , filePath(filePath)
        , matches(new QList<Match>())
    {
        connect(this, &QObject::destroyed, this, [](QObject *obj) { qDebug() << "Results::destroyed" << obj; }, Qt::QueuedConnection);
    };

    ~FileSearchResults() override
    {
        matches->clear();
        delete matches;
    };

    QString fileName;
    QString filePath;
    QList<Match> *matches;
};


class Search : public QObject, public QEnableSharedFromThis<Search>
{
    Q_OBJECT
public:
    explicit Search(SearchParameters params, QObject *parent = nullptr)
        : QObject{parent}
        , parameters(params)
    {
        setObjectName(
            QString("Search: %1 :: #files: %2").arg(params.searchString).arg(params.files.length()));
    };

    const SearchParameters parameters;
    Status status = Status::New;


    void search();
    void begin();
    void cancel();

    int getProgress() const;
    int getTotalMatches() const;
    QList<QSharedPointer<FileSearchResults>> getResultsList() const;
    QStandardItemModel *toModel();



signals:
    void started(const QSharedPointer<Search> search);
    void canceled(const QSharedPointer<Search> search);
    void finished(const QSharedPointer<Search> search);
    void progress(const int progress, const QSharedPointer<Search> search);

private:
    mutable QMutex m_mutex;
    QList<QSharedPointer<FileSearchResults>> m_resultsList;

    void appendResults(QSharedPointer<FileSearchResults> fileResults);
    void clearResultsList();

    void regexSearch(QTextStream &textStream, QList<Match> &matches);
    void textSearch(QTextStream &textStream, QList<Match> &matches);
    QSharedPointer<FileSearchResults> searchInFile(const QString &filePath);
    void searchInFiles();
};



// Search Manager

class SearchManager : public QObject
{
    Q_OBJECT


public:
    using SearchCompleteSignal = std::function<void(const Status, const QSharedPointer<Search>)>;
    using SearchProgressSignal = std::function<void(int, const QSharedPointer<Search>)>;

    QSharedPointer<Search> search(SearchParameters params);
    QSharedPointer<Search> search(SearchParameters params,
                                  QObject *receiver,
                                  SearchProgressSignal onProgress,
                                  SearchCompleteSignal onComplete);


    QSharedPointer<Search> getCurrentSearch() const;
    void cancelCurrentSearch();
    bool isSearching() const;
    void clear();

    static QStringList getFileList(const QFileSystemModel &model);

    // Singleton instance of the SearchManager
    static SearchManager *Instance()
    {
        //static QMutex mutex;
        if (!SearchManager::m_instance) {
            //QMutexLocker locker(&mutex);
            if (!SearchManager::m_instance) {
                SearchManager::m_instance.reset(new SearchManager);
            }
        }
        assert(SearchManager::m_instance != NULL);
        return SearchManager::m_instance.data();
    }

signals:
    void searchComplete(const Status status, const QSharedPointer<Search> search);
    void searchProgress(const int progress, const QSharedPointer<Search> search);

private slots:
    void onSearchStarted(const QSharedPointer<Search> search);
    void onSearchCanceled(const QSharedPointer<Search> search);
    void onSearchCanceled(const QSharedPointer<Search> search, SearchCompleteSignal onCompleteCallback);
    void onSearchFinished(const QSharedPointer<Search> search);
    void onSearchFinished(const QSharedPointer<Search> search, SearchCompleteSignal onCompleteCallback);
    void onSearchProgress(const int progress, const QSharedPointer<Search> search);
    void onSearchProgress(const int progress, const QSharedPointer<Search> search, SearchProgressSignal onProgressCallback);

private:
    SearchManager(
        QObject *parent = nullptr)
        : QObject{parent}
    {
        qDebug() << "LogSearch::SearchManager created";
    };
    // ~SearchManager() override
    // {
    //     qDebug() << "LogSearch::SearchManager destroyed";
    // }

    // Singleton instance of the manager class
    static QScopedPointer<SearchManager> m_instance;
    QQueue<QSharedPointer<Search>> m_searchQueue;
    void removeSearchFromQueue();

    Q_DISABLE_COPY_MOVE(SearchManager)
};

}

#endif // SEARCH_H
