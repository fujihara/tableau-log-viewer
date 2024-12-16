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

void SearchTab::on_searchButton_clicked()
{
    ui->groupResults->setVisible(false);
    emit search(searchText(), searchScope(), searchMode(), matchCase());
}

SearchScope SearchTab::searchScope()
{
    switch (ui->scopeComboBox->currentIndex()) {
    case 0:
        return SearchScope::Folder;
    case 1:
        return SearchScope::CurrentFile;
    case 2:
        return SearchScope::AllOpened;
    default:
        return SearchScope::Folder;
    }
}

SearchMode SearchTab::searchMode()
{
    switch (ui->searchTypeComboBox->currentIndex()) {
    case 0:
        return SearchMode::Contains;
    case 1:
        return SearchMode::Regex;
    case 2:
        return SearchMode::Equals;
    case 3:
        return SearchMode::StartsWith;
    case 4:
        return SearchMode::EndsWith;
    default:
        return SearchMode::Contains;
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

void SearchTab::showResults(
    QStandardItemModel &model)
{
    ui->resultsView->setModel(&model);
    ui->resultsView->expandAll();
    ui->groupResults->setVisible(true);
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
