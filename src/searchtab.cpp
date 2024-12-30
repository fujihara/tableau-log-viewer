#include "searchtab.h"
#include "ui_searchtab.h"

#include <QDebug>
#include <QLineEdit>
#include <QStandardItemModel>

SearchTab::SearchTab(
    QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchTab)
{
    ui->setupUi(this);
    ui->searchLineEdit->setText("");
    ui->groupResults->setVisible(false);

    // Enter on edit have the same effect as clicking the search button
    connect(ui->searchLineEdit, &QLineEdit::returnPressed, this, &SearchTab::on_searchButton_clicked);
}

SearchTab::~SearchTab()
{
    delete ui;
}

void SearchTab::setReadyToSearch()
{
    // Enable the search controls
    for (QObject *obj : ui->groupSearchControls->children()) {
        QWidget *wd = qobject_cast<QWidget *>(obj);
        if (wd) {
            wd->setEnabled(true);
        }
    }
    ui->searchButton->setText("Search");
}

void SearchTab::setSearching()
{
    // Disable the search controls
    for (QObject *obj : ui->groupSearchControls->children()) {
        QWidget *wd = qobject_cast<QWidget *>(obj);
        if (wd) {
            wd->setEnabled(false);
        }
    }
    ui->groupResults->setVisible(false);
    ui->searchButton->setText("Cancel");
    ui->searchButton->setEnabled(true);
}


void SearchTab::on_searchButton_clicked()
{
    // Button is set as "Cancel"
    if (ui->searchButton->text() == "Cancel") {
        emit cancelSearch();
        this->setReadyToSearch();
        return;
    }

    // New search
    this->setSearching();
    emit search(searchText(), searchScope(), searchMode(), matchCase());
}

LogSearch::Scope SearchTab::searchScope()
{
    switch (ui->scopeComboBox->currentIndex()) {
    case 0:
        return LogSearch::Scope::Folder;
    case 1:
        return LogSearch::Scope::CurrentFile;
    case 2:
        return LogSearch::Scope::AllOpened;
    default:
        return LogSearch::Scope::Folder;
    }
}

LogSearch::Mode SearchTab::searchMode()
{
    switch (ui->searchTypeComboBox->currentIndex()) {
    case 0:
        return LogSearch::Mode::Contains;
    case 1:
        return LogSearch::Mode::Regex;
    case 2:
        return LogSearch::Mode::Equals;
    case 3:
        return LogSearch::Mode::StartsWith;
    case 4:
        return LogSearch::Mode::EndsWith;
    default:
        return LogSearch::Mode::Contains;
    }
}

QString SearchTab::searchText()
{
    return ui->searchLineEdit->text();
}

bool SearchTab::matchCase()
{
    return ui->checkBoxCase->isChecked();
}

void SearchTab::showResults(QStandardItemModel &model)
{
    // Get the previous used model
    auto *prevModel = ui->resultsView->model();

    ui->resultsView->setModel(&model);
    ui->resultsView->expandAll();
    ui->groupResults->setVisible(true);

    // Clear prev model memory
    if (prevModel)
        prevModel->deleteLater();

    // Set ready to search again
    this->setReadyToSearch();
}

void SearchTab::on_resultsView_doubleClicked(const QModelIndex &index)
{
    // File name row - do nothing
    if (!index.parent().isValid()) {
        qDebug() << "File name row. Do nothing.";
        return;
    }

    // Thet the file name
    auto fileName = index.parent().data(Qt::DisplayRole).toString();

    // Get the from the clicked row
    auto itemModel = index.model();
    auto line = itemModel->itemData(index.siblingAtColumn(1))[0].toString();
    auto text = itemModel->itemData(index.siblingAtColumn(2))[0].toString();
    auto filePath = itemModel->itemData(index.siblingAtColumn(3))[0].toString();

    qDebug() << "File: " << fileName;
    qDebug() << "Path: " << filePath;
    qDebug() << "Line: " << line;
    qDebug() << "Text: " << text;

    emit resultSelected(fileName, filePath, line.toInt(), text);
}
