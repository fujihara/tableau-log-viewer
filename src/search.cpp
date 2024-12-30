#include "search.h"

#include <QtConcurrent>

namespace LogSearch {



void Search::regexSearch(QTextStream &textStream, QList<Match> &matches)
{
    auto reOptions = QRegularExpression::UseUnicodePropertiesOption
                     | ((this->parameters.caseSensitive) ? QRegularExpression::NoPatternOption
                                                         : QRegularExpression::CaseInsensitiveOption);

    QRegularExpression regex(this->parameters.searchString, reOptions);
    int lineNumber = 0;

    while (!textStream.atEnd()) {
        // Check if is there a request to cancell the operation
        if (this->status == Status::Canceling) {
            break;
        }

        QString line = textStream.readLine();
        ++lineNumber;

        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            matches.append({lineNumber, line});
        }
    }
}

void Search::textSearch(QTextStream &textStream, QList<Match> &matches)
{
    Qt::CaseSensitivity cs = (this->parameters.caseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
    int lineNumber = 0;

    while (!textStream.atEnd()) {
        // Check if is there a request to cancell the operation
        if (this->status == Status::Canceling) {
            break;
        }

        QString line = textStream.readLine();
        ++lineNumber;

        switch (this->parameters.mode) {
        case Mode::Contains:
            if (line.contains(this->parameters.searchString, cs)) {
                matches.append({lineNumber, line});
            }
            break;
        case Mode::StartsWith:
            if (line.startsWith(this->parameters.searchString, cs)) {
                matches.append({lineNumber, line});
            }
            break;
        case Mode::EndsWith:
            if (line.endsWith(this->parameters.searchString, cs)) {
                matches.append({lineNumber, line});
            }
            break;
        case Mode::Equals:
            if (line.compare(this->parameters.searchString, cs) == 0) {
                matches.append({lineNumber, line});
            }
            break;
        default:
            if (line.contains(this->parameters.searchString, cs)) {
                matches.append({lineNumber, line});
            }
            break;
        }
    }
}


QSharedPointer<FileSearchResults> Search::searchInFile(const QString &filePath)
{
    QFile file(filePath);
    QFileInfo fileInfo(file.fileName());

    // Create a new instance of results
    QSharedPointer<FileSearchResults> fileResults = QSharedPointer<FileSearchResults>::create(fileInfo.fileName(), filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return fileResults; // Return the QSharedPointer directly
    }

    QTextStream in(&file);

    if (this->parameters.mode == Mode::Regex) {
        regexSearch(in, *(fileResults->matches));
    } else {
        textSearch(in, *(fileResults->matches));
    }

    file.close();
    return fileResults;
}


void Search::searchInFiles()
{
    for (int i = 0; i < this->parameters.files.length(); i++) {
        // Check if is there a request to cancell the operation
        if (this->status == Status::Canceling) {
            break;
        }

        qDebug() << "Search in:" << this->parameters.files.at(i);
        auto fileResults = searchInFile(this->parameters.files.at(i));
        this->appendResults(fileResults);

        // Update the progress
        int searchProgress = this->getProgress();
        qDebug() << "Progresss: " << searchProgress << "%";
        emit progress(searchProgress, this->sharedFromThis());
    }
}

void Search::cancel()
{
    QMutexLocker locker(&m_mutex);
    if (this->status != Status::Searching) {
        return;
    }

    this->status = Status::Canceling;
}

int Search::getProgress() const
{
    if (this->m_resultsList.length() == 0 || this->parameters.files.length() == 0)
        return 0;

    double progress = static_cast<double>(this->m_resultsList.length()) / this->parameters.files.length();
    qDebug() << "Current Progress:" << progress;
    return static_cast<int>(progress * 100);
}

int Search::getTotalMatches() const
{
    if (this->m_resultsList.length() == 0)
        return 0;

    auto fileCount = [](QSharedPointer<FileSearchResults> fileResults) -> int {
        return fileResults->matches->length();
    };

    auto reduce = [](int &result, const int &intermediate) { result += intermediate; };

    auto totalMatches = QtConcurrent::blockingMappedReduced(this->m_resultsList,
                                                            fileCount,
                                                            reduce);
    return totalMatches;
}

QList<QSharedPointer<FileSearchResults>> Search::getResultsList() const
{
    return m_resultsList;
}

void Search::appendResults(QSharedPointer<FileSearchResults> fileResults)
{
    QMutexLocker locker(&m_mutex);
    // Is the results object from the same thread?
    if (this->thread() != fileResults->thread()) {
        bool moved = fileResults->moveToThread(this->thread());
        qDebug() << "Results object moved the Search object thread:" << moved;
        assert(moved);
    }

    fileResults->setParent(this);
    m_resultsList.append(fileResults);
}

void Search::clearResultsList()
{
    QMutexLocker locker(&m_mutex);
    std::for_each(this->m_resultsList.begin(),
                  this->m_resultsList.end(),
                  [](QSharedPointer<FileSearchResults> item) { item->deleteLater(); });
    m_resultsList.clear();
}

void Search::search()
{
    if (this->status != Status::New) {
        return;
    }

    // Prepare for starting a new search
    this->status = Status::Searching;
    emit started(this->sharedFromThis());

    // Search in the file list
    this->searchInFiles();

    if (this->status == Status::Canceling) {
        this->status = Status::Canceled;
        emit canceled(this->sharedFromThis());

        // Return partial results
        return;
    }

    // Signal the finish
    emit finished(this->sharedFromThis());
}


void Search::begin()
{
    if (this->status != Status::New) {
        return;
    }

    // Clear the list
    this->clearResultsList();

    // Prepare new search
    this->status = Status::Searching;

    qDebug() << "Search::begin:" << this << "ref:" << this->sharedFromThis();
    qDebug() << "Search::begin: thread:" << QThread::currentThread() << "isMainThread:" << QThread::isMainThread();


    auto searchTask = [this](const QString &filePath) -> void {
        qDebug() << "Search::begin::searchTask" << QThread::currentThread();

        // Check for cancelation
        if (this->status == Status::Canceling) {
            qDebug() << "Search::begin::searchTask - Canceling";
            return;
        }


        QSharedPointer<FileSearchResults> fileResults = this->searchInFile(filePath);
        qDebug() << "Search::begin::searchTask -> file:" << filePath
                 << " thread:" << QThread::currentThread();

        this->appendResults(fileResults);

        // Log the progress
        int prog = this->getProgress();
        qDebug() << "Progresss: " << prog << "%";
        emit progress(prog, this->sharedFromThis());
    };

    // Create a QFutureWatcher to monitor the completion of the task
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);

    // Connect the finished signal to emit search finished
    connect(watcher, &QFutureWatcher<LogSearch::FileSearchResults*>::finished, this, [this, watcher]() {
        // Handle cancel
        if (this->status == Status::Canceling) {
            this->status = Status::Canceled;
            emit canceled(this->sharedFromThis());
            return;
        }

        qDebug() << "Search::begin::QFutureWatcher::QFutureWatcher:" << this
                 << "ref:" << this->sharedFromThis();
        qDebug() << "Search::begin::QFutureWatcher::QFutureWatcher: thread:"
                 << QThread::currentThread() << "isMainThread:" << QThread::isMainThread();

        // Search finished
        this->status = Status::Finished;
        emit finished(this->sharedFromThis());

        // Clean up the watcher
        watcher->deleteLater();
    }, Qt::QueuedConnection);

    // reate a QFutureWatcher to monitor the canceled event
    connect(watcher, &QFutureWatcher<QString>::canceled, this, [this, watcher]() {
        qDebug() << "Search::begin::QFutureWatcher::canceled" << QThread::currentThread();
        qDebug() << "Canceled by the QFutureWatcher";

        // Handle cancel
        this->status = Status::Canceled;
        emit canceled(this->sharedFromThis());

        // Clean up the watcher
        watcher->deleteLater();
    }, Qt::QueuedConnection);


    // Run the mapped function asynchronously
    QFuture<void> future = QtConcurrent::map(this->parameters.files, searchTask);
    watcher->setFuture(future);
}

QStandardItemModel *Search::toModel()
{
    // Do not work if searching or no search started.
    if (this->status == Status::New || this->status == Status::Searching)
        return nullptr;

    // Create a new model
    QStandardItemModel *model = new QStandardItemModel;
    model->setHorizontalHeaderLabels({"File", "Line Number", "Content"});

    QHash<QString, QStandardItem *> fileItems;

    for (auto fileResults : this->getResultsList()) {
        if (!fileItems.contains(fileResults->filePath)) {
            // No entry for this file yet, create a new one
            QStandardItem *fileItem = new QStandardItem(fileResults->fileName);
            fileItem->setToolTip(fileResults->filePath);
            model->appendRow(fileItem);
            fileItems[fileResults->filePath] = fileItem;
        }


        // Add each match for the file entry
        for (auto match : *(fileResults->matches)) {
            QStandardItem *fileName = new QStandardItem("");
            fileName->setToolTip(match.lineContent);
            QStandardItem *lineNumberItem = new QStandardItem(QString::number(match.lineNumber));
            lineNumberItem->setToolTip(match.lineContent);
            QStandardItem *contentItem = new QStandardItem(match.lineContent);
            contentItem->setToolTip(match.lineContent);
            QStandardItem *filePath = new QStandardItem(fileResults->filePath);

            fileItems[fileResults->filePath]->appendRow(
                {fileName, lineNumberItem, contentItem, filePath});
        }
    }
    return model;
}


// SearchManager


QSharedPointer<Search> SearchManager::search(SearchParameters params)
{
    // Overloaded method, SearchManager slots to handle the search events
    return this->search(params, nullptr, nullptr, nullptr);
}

QSharedPointer<Search> SearchManager::search(SearchParameters params,
                                             QObject *receiver,
                                             SearchProgressSignal onProgress,
                                             SearchCompleteSignal onComplete)
{
    // If it is searching, cancel the current search
    if (this->isSearching()) {
        this->getCurrentSearch()->cancel();
    }

    // Prepare a new search
    QSharedPointer<Search> search = QSharedPointer<Search>::create(params, this);
    this->m_searchQueue.enqueue(search);
    qDebug() << "SearchManager::search" << "New search obj:" << search << "ref:" << &search;


    // Handle started signals from search. There is no callback for this event
    connect(search.data(), &Search::started, this, &SearchManager::onSearchStarted, Qt::QueuedConnection);

    // If we are using a callback function, setup a lambda to handle the sinals from the search
    // and pass it as a paramenter for the manager slot. The slot will call the callback function.
    if (receiver && onProgress) {
        auto sProgress = [onProgress](const int value, const QSharedPointer<Search> srch) {
            qDebug() << "SearchManager::search::progress" << "Search progress: " << value << "% - "<< srch;

            emit SearchManager::Instance()->onSearchProgress(value, srch, onProgress);
        };

        connect(search.data(), &Search::progress, receiver, sProgress, Qt::QueuedConnection);
    } else {
        // Use the SearchManager slots
        // The connect call is using a static_cast to allow overloading of the SearchManager::onSearchProgress slot
        connect(search.data(), &Search::progress, this, static_cast<void(SearchManager::*)(const int, const QSharedPointer<Search>)>(&SearchManager::onSearchProgress), Qt::QueuedConnection);
    }


    if (receiver && onComplete) {
        auto sComplete = [onComplete](const QSharedPointer<Search> srch) {
            qDebug() << "SearchManager::search::complete" << "Search completed: " << srch;

            emit SearchManager::Instance()->onSearchFinished(srch, onComplete);
        };

        connect(search.data(), &Search::canceled, receiver, sComplete, Qt::QueuedConnection);
        connect(search.data(), &Search::finished, receiver, sComplete, Qt::QueuedConnection);
    } else {
        // Use the SearchManager slots
        // The connect call is using a static_cast to allow overloading of the
        // SearchManager::onSearchCanceled and SearchManager::onSearchFinished slots
        connect(search.data(), &Search::canceled, this, static_cast<void(SearchManager::*)(const QSharedPointer<Search>)>(&SearchManager::onSearchCanceled), Qt::QueuedConnection);
        connect(search.data(), &Search::finished, this, static_cast<void(SearchManager::*)(const QSharedPointer<Search>)>(&SearchManager::onSearchFinished), Qt::QueuedConnection);
    }

    // Begin searching
    search->begin();

    return search;
}

void SearchManager::onSearchStarted(const QSharedPointer<Search> search)
{
    qDebug() << "SearchManager::onSearchStarted" << "Starting a new search: " << search;
}

void SearchManager::onSearchCanceled(const QSharedPointer<Search> search)
{
    this->onSearchCanceled(search, nullptr);
}

void SearchManager::onSearchCanceled(const QSharedPointer<Search> search, SearchCompleteSignal onCompleteCallback)
{
    qDebug() << "SearchManager::onSearchCanceled" << "Search canceled: " << search;

    emit searchComplete(search->status, search);
    if (onCompleteCallback)
        emit onCompleteCallback(search->status, search);

    SearchManager::Instance()->removeSearchFromQueue();
}

void SearchManager::onSearchFinished(const QSharedPointer<Search> search)
{
    this->onSearchFinished(search, nullptr);
}

void SearchManager::onSearchFinished(const QSharedPointer<Search> search, SearchCompleteSignal onCompleteCallback)
{
    qDebug() << "SearchManager::onSearchFinished" << "Search finished: " << search;

    emit searchComplete(search->status, search);
    if (onCompleteCallback)
        emit onCompleteCallback(search->status, search);

    SearchManager::Instance()->removeSearchFromQueue();
}

void SearchManager::onSearchProgress(const int progress, const QSharedPointer<Search> search)
{
    this->onSearchProgress(progress, search, nullptr);
}

void SearchManager::onSearchProgress(const int progress, const QSharedPointer<Search> search, SearchProgressSignal onProgressCallback)
{
    qDebug() << "SearchManager::onSearchProgress" << "Search progress: " << progress << "% - " << search;

    emit searchProgress(progress, search);
    if (onProgressCallback)
        emit onProgressCallback(progress, search);
}

QSharedPointer<Search> SearchManager::getCurrentSearch() const
{
    if (this->m_searchQueue.empty())
        return nullptr;

    return this->m_searchQueue.head();
}

void SearchManager::cancelCurrentSearch()
{
    // Top of the queue is searching
    // Just cancel.
    if (this->isSearching()) {
        auto search = this->getCurrentSearch();
        search->cancel();
        return;
    }

    // If the top of the queue is not in searching status
    // check the next items for running search jobs.
    for (auto search : this->m_searchQueue) {
        if (search->status == Status::Searching) {
            search->cancel();
            return;
        }
    }
}

bool SearchManager::isSearching() const
{
    if (!this->getCurrentSearch())
        return false;

    if (this->getCurrentSearch()->status == Status::Searching) {
        return true;
    }

    return false;
}

void SearchManager::clear()
{
    if (this->m_searchQueue.empty())
        // nothing to do
        return;

    if (this->isSearching()) {
        // If there is a search ongoing, connect to its cancel signal,
        // and call for its cancelation.
        // Repeat the process until it is all clear.
        auto search = this->getCurrentSearch();
        QMetaObject::Connection connection = connect(search.data(), &Search::canceled, this,
            [&connection, this](const QSharedPointer<Search> search) {
                qDebug() << "SearchManager::clear::canceled"
                         << "Search canceled: " << search;
                qDebug() << "Calling clear again";

                disconnect(connection);
                this->clear();
            },
            Qt::QueuedConnection);
        this->cancelCurrentSearch();
        return;
    }

    // No searching in progress, remove each item from the queue
    while (this->m_searchQueue.length() > 1) {
        this->removeSearchFromQueue();
    }

    // Remove search always leave a single item in the queue
    // remove it.
    // Remove the top search from the queue
    auto search = this->m_searchQueue.dequeue();
    qDebug() << "Removing search:" << search;

    // Disconnect the signals
    search->disconnect();

    // Set for cleanup
    search->deleteLater();
}

void SearchManager::removeSearchFromQueue()
{
    // No search exists, nothing to do
    if (!this->getCurrentSearch())
        return;

    // Do not remove ongoing search
    if (this->isSearching())
        return;

    // Remove the s

    // Always keep the last search in the top of the queue
    if (this->m_searchQueue.length() < 2)
        return;

    // Remove the top search from the queue
    auto search = this->m_searchQueue.dequeue();
    qDebug() << "SearchManager::removeSearchFromQueue: Removing search:" << search;

    // Disconnect the signals
    search->disconnect();

    // Set for cleanup
    search->deleteLater();
}

QStringList SearchManager::getFileList(
    const QFileSystemModel &model)
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


QScopedPointer<LogSearch::SearchManager> LogSearch::SearchManager::m_instance(nullptr);


} // End namespace LogSearch
