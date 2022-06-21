﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "util/cipherutil.h"
#include "passworddialog.h"
#include "newpassworddialog.h"
#include "changepassworddialog.h"
#include "entrydetaildialog.h"
#include "longinfodialog.h"

#include <QMenuBar>
#include <QMap>
#include <QMenu>
#include <QLabel>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    cipherBook(nullptr),
    filepath(nullptr),
    key(nullptr),
    groupList(nullptr),
    isSaved(true)
{
    ui->setupUi(this);
    mySetupUi();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete cipherBook;
    delete filepath;
    delete key;
    delete groupList;
}

void MainWindow::mySetupUi() {
    setWindowIcon(QPixmap(":/icons/icon.png"));

    QWidget* oldTitle = ui->sideBar->titleBarWidget();
    ui->sideBar->setTitleBarWidget(new QWidget(this));
    delete oldTitle;

    this->ui->searchTreeView->setHeaderHidden(true);

    setupActions();
}

void MainWindow::setupActions() {
    //文件菜单
    connect(ui->actionNew, ui->actionNew->triggered, this, newFile);
    connect(ui->actionOpenCipher, ui->actionOpenCipher->triggered, this, openCipher);
    connect(ui->actionOpenPlain, ui->actionOpenPlain->triggered, this, openPlain);
    connect(ui->actionSave, ui->actionSave->triggered, this, save);
    connect(ui->actionSaveAsCipher, ui->actionSaveAsCipher->triggered, this, saveAsCipher);
    connect(ui->actionSaveAsPlain, ui->actionSaveAsPlain->triggered, this, saveAsPlain);
    connect(ui->actionClose, ui->actionClose->triggered, this, closeFile);
    connect(ui->actionExit, ui->actionExit->triggered, this, exitExe);
    //编辑菜单
    connect(ui->actionChangePassword, ui->actionChangePassword->triggered, this, changePassword);
    connect(ui->actionAdd, ui->actionAdd->triggered, this, addEntry);
    connect(ui->actionAlter, ui->actionAlter->triggered, this, alterEntry);
    connect(ui->actionDelete, ui->actionDelete->triggered, this, deleteEntry);
    //帮助菜单
    connect(ui->actionHelp, ui->actionHelp->triggered, this, help);
    connect(ui->actionAbout, ui->actionAbout->triggered, this, about);
    //左侧边栏上面三个按钮
    connect(ui->addButton, ui->addButton->clicked, this, addEntry);
    connect(ui->alterButton, ui->alterButton->clicked, this, alterEntry);
    connect(ui->deleteButton, ui->deleteButton->clicked, this, deleteEntry);
    //左侧边栏下面两个按钮
    connect(ui->saveButton, ui->saveButton->clicked, this, save);
    connect(ui->closeButton, ui->closeButton->clicked, this, closeFile);
    //搜索按钮
    connect(ui->searchButton, ui->searchButton->clicked, this, search);
    //清空按钮
    connect(ui->clearButton, ui->clearButton->clicked, this, clear);
    //复制按钮
    connect(ui->usernameCopyButton, ui->usernameCopyButton->clicked, this, copyUsername);
    connect(ui->passwordCopyButton, ui->passwordCopyButton->clicked, this, copyPassword);
    //右侧列表点击事件
    connect(ui->searchTreeView, ui->searchTreeView->clicked, this, clickedAtIndex);

    alterActionsOnCloseFile();
}

void MainWindow::alterActionsOnOpenFile() {
    ui->actionSave->setEnabled(true);
    ui->actionSaveAsCipher->setEnabled(true);
    ui->actionSaveAsPlain->setEnabled(true);
    ui->actionClose->setEnabled(true);

    ui->saveButton->setEnabled(true);
    ui->closeButton->setEnabled(true);

    ui->actionChangePassword->setEnabled(true);
    ui->actionAdd->setEnabled(true);
    ui->addButton->setEnabled(true);

    ui->searchButton->setEnabled(true);
}
void MainWindow::alterActionsOnCloseFile() {
    ui->actionSave->setEnabled(false);
    ui->actionSaveAsCipher->setEnabled(false);
    ui->actionSaveAsPlain->setEnabled(false);
    ui->actionClose->setEnabled(false);

    ui->saveButton->setEnabled(false);
    ui->closeButton->setEnabled(false);

    ui->actionChangePassword->setEnabled(false);
    ui->actionAdd->setEnabled(false);
    ui->addButton->setEnabled(false);

    ui->searchButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
    ui->clearButton->setStyleSheet("color:grey;");

    clearSearchTreeView();
    clear();
}

void MainWindow::clearSearchTreeView() {
    delete ui->searchTreeView->model();
    ui->searchTreeView->setModel(nullptr);
}

void MainWindow::alterActionsOnSelect() {
    ui->actionAlter->setEnabled(true);
    ui->actionDelete->setEnabled(true);
    ui->alterButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
}
void MainWindow::alterActionsOnClear() {
    ui->actionAlter->setEnabled(false);
    ui->actionDelete->setEnabled(false);
    ui->alterButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
}

void MainWindow::clear(){
    ui->nameLabel->clear();
    ui->otherNameLabel->clear();
    ui->groupLabel->clear();
    ui->usernameBox->setHidden(true);
    ui->passwordBox->setHidden(true);
    ui->remarksEdit->clear();
    ui->remarksBox->setHidden(true);

    ui->searchTreeView->clearSelection();

    ui->clearButton->setEnabled(false);
    ui->clearButton->setStyleSheet("color:grey;");
    alterActionsOnClear();
}


void MainWindow::newFile() {
    closeFile();

    NewPasswordDialog dialog;
    if (!dialog.exec()) return;
    QString key = dialog.getPassword();

    this->isSaved = false;
    this->key = new QByteArray(CipherUtil::getSHA256(key.toLocal8Bit()));
    this->cipherBook = new CipherBook();
    this->groupList = new QStringList();
    alterActionsOnOpenFile();
}

void MainWindow::openCipher() {
    closeFile();

    QString filename = QFileDialog::getOpenFileName(this, "打开密码本文件", ".", "*.cipherbook");
    if (!filename.length()) return;

    PasswordDialog dialog;
    if (!dialog.exec()) return;
    QString key = dialog.getPassword();
    if (key.length() == 0) return;

    QByteArray key256 = CipherUtil::getSHA256(key.toLocal8Bit());
    QJsonObject json = CipherUtil::readBinaryFileAndDecode(filename, key256);

    CipherBook* newCipherBook = new CipherBook(json);
    if (newCipherBook->globalId == 0) {
        QMessageBox::critical(this, "错误", "密码输入错误或文件损坏！");
        delete newCipherBook;
        return;
    }

    this->cipherBook = newCipherBook;
    this->filepath = new QString(filename);
    this->key = new QByteArray(key256);
    this->groupList = new QStringList();
    renewGroupList();
    this->isSaved = true;

    alterActionsOnOpenFile();
}

void MainWindow::openPlain() {
    closeFile();

    QString filename = QFileDialog::getOpenFileName(this, "打开明文文件", ".", "*.json");
    if (!filename.length()) return;

    QJsonObject json = CipherUtil::readPlainFileAndDecode(filename);

    CipherBook* newCipherBook = new CipherBook(json);
    if (newCipherBook->globalId == 0) {
        QMessageBox::critical(this, "错误", "文件内容格式有误！");
        delete newCipherBook;
        return;
    }

    NewPasswordDialog dialog;
    if (!dialog.exec()) return;
    QString key = dialog.getPassword();

    this->cipherBook = newCipherBook;
    this->key = new QByteArray(CipherUtil::getSHA256(key.toLocal8Bit()));
    this->groupList = new QStringList();
    renewGroupList();
    this->isSaved = false;

    alterActionsOnOpenFile();
}

void MainWindow::renewGroupList() {
    QSet<QString> qset;
    for (auto it=cipherBook->book->begin(); it!=cipherBook->book->end(); it++) {
        if (it.value()->getGroup().length() > 0) {
            qset.insert(it.value()->getGroup());
        }
    }
    for (auto it=qset.begin(); it!=qset.end(); it++) {
        groupList->append(*it);
    }
}
void MainWindow::renewGroupList(const QString& group) {
    if (group.length() == 0) return;
    for (auto it=groupList->begin(); it!=groupList->end(); it++) {
        if (it->compare(group) == 0) return;
    }
    groupList->append(group);
}

void MainWindow::save(){
    if (isSaved) return;
    if (filepath != nullptr) {
        QJsonObject* json = cipherBook->toJSON();
        CipherUtil::encodeAndSave(json, *key, *filepath);
        QMessageBox::information(this, "信息", "保存成功！");
        isSaved = true;
        delete json;
    } else {
        saveAsCipher();
    }
}
void MainWindow::saveAsCipher(){
    QString filename = QFileDialog::getSaveFileName(this, "保存密码本文件", ".", "*.cipherbook");
    if (!filename.length()) return;

    QJsonObject* json = cipherBook->toJSON();
    CipherUtil::encodeAndSave(json, *key, filename);
    if (filepath == nullptr) {
        filepath = new QString(filename);
        isSaved = true;
    } else if (filepath->compare(filename) == 0) {
        isSaved = true;
    }
    QMessageBox::information(this, "信息", "保存成功！");

    delete json;
}
void MainWindow::saveAsPlain() {
    QString filename = QFileDialog::getSaveFileName(this, "导出明文", ".", "*.json");
    if (!filename.length()) return;

    QJsonObject* json = this->cipherBook->toJSON();
    CipherUtil::savePlainJson(json, filename);

    QMessageBox::information(this, "信息", "明文导出成功！");
    delete json;
}
void MainWindow::closeFile(){
    if (!isSaved) {
        int ret = QMessageBox::question(this, "警告", "密码本内容尚未保存，是否保存？");
        if (ret == QMessageBox::StandardButton::Yes) {
            save();
        }
    }

    delete cipherBook;
    delete filepath;
    delete key;
    delete groupList;
    cipherBook = nullptr;
    filepath = nullptr;
    key = nullptr;
    groupList = nullptr;

    isSaved = true;
    alterActionsOnCloseFile();
}
void MainWindow::exitExe(){
    if (cipherBook != nullptr) {
        closeFile();
    }
    close();
}

void MainWindow::closeEvent(QCloseEvent*) {
    exitExe();
}

void MainWindow::changePassword(){
    ChangePasswordDialog dialog(*key ,this);

    if (!dialog.exec()) return;
    QString newKeyStr = dialog.getPassword();

    QByteArray* newKey = new QByteArray(CipherUtil::getSHA256(newKeyStr.toLocal8Bit()));
    key = newKey;
    QMessageBox::information(this, "信息", "密码修改成功！");
    isSaved = false;
}
void MainWindow::addEntry() {
    EntryDetailDialog dialog(cipherBook->globalId, groupList, this);
    if (!dialog.exec()) return;

    CipherEntry* entry = dialog.getEntry();
    cipherBook->add(entry);

    QVector<CipherEntry*>* list = new QVector<CipherEntry*>();
    list->append(entry);
    renewGroupList(entry->getGroup());
    updateSearchTreeView(list);
    clear();
    isSaved = false;
}
void MainWindow::alterEntry() {
    QModelIndex index = ui->searchTreeView->currentIndex();
    auto model = ui->searchTreeView->model();
    int id = *(int*)(model->data(index, Qt::UserRole + 1).data());
    EntryDetailDialog dialog(cipherBook->getById(id), groupList, this);
    if (!dialog.exec()) return;

    CipherEntry* entry = dialog.getEntry();
    cipherBook->modify(id, entry);

    QVector<CipherEntry*>* list = new QVector<CipherEntry*>();
    list->append(entry);
    renewGroupList(entry->getGroup());
    updateSearchTreeView(list);
    isSaved = false;
}
void MainWindow::deleteEntry(){
    QModelIndex index = ui->searchTreeView->currentIndex();
    int id = *(int*)(ui->searchTreeView->model()->data(index, Qt::UserRole + 1).data());
    CipherEntry* tmp = cipherBook->getById(id);

    int ret = QMessageBox::question(this, "警告", QString("确认删除：%1（%2）吗？")
                                                .arg(tmp->getName())
                                                .arg(tmp->getOtherName()));
    if (ret == QMessageBox::StandardButton::No) return;

    cipherBook->remove(id);
    ui->searchTreeView->model()->removeRow(index.row());
    isSaved = false;
    clear();
}

void MainWindow::help(){
    LongInfoDialog dialog("帮助", this);
    dialog.exec();
}
void MainWindow::about(){
    LongInfoDialog dialog("关于我", this);
    dialog.exec();
}

void MainWindow::copyUsername(){
    if (ui->usernameEdit->text().length() == 0) return;
    QApplication::clipboard()->setText(ui->usernameEdit->text());
    QMessageBox::information(this, "信息", "用户名复制成功！");
}
void MainWindow::copyPassword(){
    if (ui->passwordEdit->text().length() == 0) return;
    QApplication::clipboard()->setText(ui->passwordEdit->text());
    QMessageBox::information(this, "信息", "密码复制成功！");
}
void MainWindow::search() {
    QString str = this->ui->searchEdit->displayText().trimmed();
    if (str.length() > 0) {
        QVector<CipherEntry*>* list = this->cipherBook->search(str);
        updateSearchTreeView(list);
    } else {
        clearSearchTreeView();
    }
    ui->searchEdit->clear();
    clear();
}
void MainWindow::updateSearchTreeView(QVector<CipherEntry*>* list) {
    QMap<QString, QStandardItem*> qmap;
    QString noGroupStr("未分组");

    for (int i=0; i<list->length(); i++) {
        QString group = list->at(i)->getGroup();
        if (group.length() == 0) {
            group = QString(noGroupStr);
        }
        if (!qmap.contains(group)) {
            QStandardItem* groupItem = new QStandardItem(group);
            QStandardItem* entryItem = new QStandardItem(list->at(i)->getName());
            entryItem->setData(list->at(i)->getId());
            groupItem->setChild(groupItem->rowCount(), entryItem);
            qmap.insert(group, groupItem);
        } else {
            QStandardItem* entryItem = new QStandardItem(list->at(i)->getName());
            entryItem->setData(list->at(i)->getId());
            QStandardItem* groupItem = qmap.find(group).value();
            groupItem->setChild(groupItem->rowCount(), entryItem);
        }
    }

    QStandardItemModel* treeModel = new QStandardItemModel(this);

    for (auto it=qmap.begin(); it!=qmap.end(); it++) {
        treeModel->appendRow(it.value());
    }
    auto oldModel = ui->searchTreeView->model();
    ui->searchTreeView->setModel(treeModel);
    ui->searchTreeView->expandAll();
    delete list;
    delete oldModel;
}

void MainWindow::clickedAtIndex(const QModelIndex& index) {
    QVariant v = index.data(Qt::UserRole + 1);
    if (v.isNull()) return;
    int id = *(int*)(v.data());
    CipherEntry* entry = cipherBook->getById(id);

    ui->nameLabel->setText(entry->getName());
    ui->otherNameLabel->setText(entry->getOtherName());
    ui->groupLabel->setText(entry->getGroup());
    ui->usernameBox->setHidden(false);
    ui->usernameEdit->setText(entry->getUsername());
    ui->passwordBox->setHidden(false);
    ui->passwordEdit->setText(entry->getPassword());
    ui->remarksEdit->setPlainText(entry->getRemarks());
    ui->remarksBox->setHidden(false);

    ui->clearButton->setEnabled(true);
    ui->clearButton->setStyleSheet("color:red;");

    alterActionsOnSelect();
}
