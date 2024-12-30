#ifndef SEARCHTAB_H
#define SEARCHTAB_H

#include "search.h"

#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class SearchTab;
}

class SearchTab : public QWidget
{
    Q_OBJECT

public:
    explicit SearchTab(QWidget *parent = nullptr);
    ~SearchTab();

    LogSearch::Scope searchScope();
    LogSearch::Mode searchMode();
    QString searchText();
    bool matchCase();

    void showResults(QStandardItemModel& model);

private:
    Ui::SearchTab *ui;

    void setReadyToSearch();
    void setSearching();

private slots:
    void on_searchButton_clicked();
    void on_resultsView_doubleClicked(const QModelIndex &index);

signals:
    void search(const QString &text, const LogSearch::Scope scope, const LogSearch::Mode mode, const bool caseSensitive);
    void cancelSearch();
    void resultSelected(const QString fileName, const QString filePath, const int lineNumber, const QString text);
};

#endif // SEARCHTAB_H
