#ifndef SEARCHTAB_H
#define SEARCHTAB_H

#include "searchopt.h"

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

    SearchScope searchScope();
    SearchMode searchMode();
    QString searchText();
    bool matchCase();

    void showResults(QStandardItemModel& model);

private:
    Ui::SearchTab *ui;

private slots:
    void on_searchButton_clicked();
    void on_resultsView_doubleClicked(const QModelIndex &index);

signals:
    void search(const QString &text, const SearchScope &scope, const SearchMode &mode, const bool &caseSensitive);
    void resultSelected(const QString fileName, const QString filePath, const int lineNumber, const QString text);
};

#endif // SEARCHTAB_H
