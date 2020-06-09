#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mdichild.h"
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QSignalMapper>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>
#include <QFontDialog>
#include <QColorDialog>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QDebug>
#include "find_replace.h"
#include "ui_find_replace.h"
#include <QAction>
#include <QActionGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    connect(ui->mdiArea,SIGNAL(subWindowActivated(QMdiSubWindow *)),this,SLOT(updateMenus()));
    windowMapper = new QSignalMapper(this);
    connect(windowMapper,SIGNAL(mapped(QWidget *)),this,SLOT(setActiveSubWindow(QWidget *)));
    updateWindowMenu();
    connect(ui->menu_W,SIGNAL(aboutToShow()),this,SLOT(updateWindowMenu()));
    readSettings();//读取之前的窗口设置
    initWindow();//初始化窗口
    //查找替换相关
    findReplace = new Find_Replace(this);
    connect(findReplace->ui->cancleButtonF,SIGNAL(clicked()),findReplace,SLOT(close()));
    connect(findReplace->ui->nextButtonF,SIGNAL(clicked()),this,SLOT(findNextF()));
    connect(findReplace->ui->cancleButtonR,SIGNAL(clicked()),findReplace,SLOT(close()));
    connect(findReplace->ui->nextButtonR,SIGNAL(clicked()),this,SLOT(findNextR()));
    connect(findReplace->ui->pushButtonReplace,SIGNAL(clicked()),this,SLOT(replace()));
    connect(findReplace->ui->pushButtonReplaceAll,SIGNAL(clicked()),this,SLOT(replaceAll()));
    //对齐方式
    actGrp = new QActionGroup(this);
    left = new QAction(QIcon(":/image/images/textleft.png"),tr("左对齐"),actGrp);
    left->setCheckable(true);
    right = new QAction(QIcon(":/image/images/textright.png"),tr("右对齐"),actGrp);
    right->setCheckable(true);
    center = new QAction(QIcon(":/image/images/textcenter.png"),tr("居中对齐"),actGrp);
    center->setCheckable(true);
    justify = new QAction(QIcon(":/image/images/textjustify.png"),tr("两端对齐"),actGrp);
    justify->setCheckable(true);
    toolbar = new QToolBar(this);
    this->addToolBar(toolbar);
    toolbar->addActions(actGrp->actions());
    ui->menu_O->addSeparator();
    ui->menu_O->addActions(actGrp->actions());
    connect(actGrp,&QActionGroup::triggered,this,&MainWindow::slotAligment);

    updateMenus();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionNew_triggered()
{
    MdiChild * child = createMdiChild();
    child->newFile();
    child->show();
}

MdiChild *MainWindow::activeMdiChild()
{
    if(QMdiSubWindow * activeSubWindow = ui->mdiArea->activeSubWindow())
        return qobject_cast<MdiChild *>(activeSubWindow->widget());
    return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    foreach(QMdiSubWindow *window,ui->mdiArea->subWindowList()){
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        if(mdiChild->currentFile() == canonicalFilePath)
            return window;
    }
    return 0;
}

void MainWindow::readSettings()
{
    QSettings settings("jxz","myMDI");
    QPoint pos = settings.value("pos",QPoint(200,200)).toPoint();
    QSize size = settings.value("size",QSize(400,400)).toSize();
    move(pos);
    resize(size);
}

void MainWindow::writeSettings()
{
    QSettings settings("jxz","myMDI");
    settings.setValue("pos",pos());
    settings.setValue("size",size());
}

void MainWindow::initWindow()
{
    setWindowTitle(tr("多文档编辑器"));
    ui->toolBar->setWindowTitle(tr("工具栏"));
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->statusbar->showMessage(tr("欢迎使用多文档编辑器"));
    QLabel *label = new QLabel(this);
    label->setFrameStyle(QFrame::Box|QFrame::Sunken);
    label->setText(tr("Edited By Later"));
    //label->setTextFormat(Qt::RichText);
    ui->statusbar->addPermanentWidget(label);
    ui->actionNew->setStatusTip(tr("创建一个文件"));
    ui->actionOpen->setStatusTip(tr("打开一个已有文件"));
    ui->actionExit->setToolTip(tr("退出程序"));
    ui->actionNext->setToolTip(tr("下一个窗口"));
    ui->actionPrevious->setToolTip(tr("上一个窗口"));
}

void MainWindow::doFind(QString findText)
{
    if(!senceF && !upFindF){
        if(!activeMdiChild()->find(findText)){
            QMessageBox::warning(this,tr("查找结果"),tr("未找到%1").arg(findText));
        }
    }
    else if(senceF && !upFindF){
        if(!activeMdiChild()->find(findText,QTextDocument::FindCaseSensitively)){
            QMessageBox::warning(this,tr("查找结果"),tr("未找到%1").arg(findText));
        }
    }
    else if(!senceF && upFindF){
        if(!activeMdiChild()->find(findText,QTextDocument::FindBackward)){
            QMessageBox::warning(this,tr("查找结果"),tr("未找到%1").arg(findText));
        }
    }
    else if(senceF && upFindF){
        if(!activeMdiChild()->find(findText,QTextDocument::FindCaseSensitively | QTextDocument::FindBackward)){
            QMessageBox::warning(this,tr("查找结果"),tr("未找到%1").arg(findText));
        }
    }
}

bool MainWindow::doReplaceAllFind(QString findText)
{
    if(!senceF && !upFindF){
        if(!activeMdiChild()->find(findText))
            return false;
    }
    else if(senceF && !upFindF){
        if(!activeMdiChild()->find(findText,QTextDocument::FindCaseSensitively))
            return false;
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    ui->mdiArea->closeAllSubWindows();
    if(ui->mdiArea->currentSubWindow()){
        event->ignore();
    }else{
        writeSettings();
        event->accept();
    }
}

void MainWindow::updateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
    ui->actionSave->setEnabled(hasMdiChild);
    ui->actionSaveAs->setEnabled(hasMdiChild);
    ui->actionPaste->setEnabled(hasMdiChild);
    ui->actionClose->setEnabled(hasMdiChild);
    ui->actionCloseAll->setEnabled(hasMdiChild);
    ui->actionTile->setEnabled(hasMdiChild);
    ui->actionCascade->setEnabled(hasMdiChild);
    ui->actionNext->setEnabled(hasMdiChild);
    ui->actionPrevious->setEnabled(hasMdiChild);
    actionSeparator->setVisible(hasMdiChild);
    bool hasSelection = (activeMdiChild() && activeMdiChild()->textCursor().hasSelection());
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasMdiChild);
    ui->actionUndo->setEnabled(activeMdiChild() && activeMdiChild()->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(activeMdiChild() && activeMdiChild()->document()->isRedoAvailable());
    ui->actionFont->setEnabled(hasMdiChild);
    ui->actionFontColor->setEnabled(hasMdiChild);
    ui->actionPrint->setEnabled(hasMdiChild);
    ui->actionPrintPreview->setEnabled(hasMdiChild);
    ui->actionCreatePDF->setEnabled(hasMdiChild);
    ui->actionFind->setEnabled(hasMdiChild);
    ui->actionReplace->setEnabled(hasMdiChild);
    actGrp->setEnabled(hasMdiChild);
}

MdiChild *MainWindow::createMdiChild()
{
    MdiChild *child = new MdiChild;
    ui->mdiArea->addSubWindow(child);
    connect(child,SIGNAL(copyAvailable(bool)),ui->actionCopy,SLOT(setEnabled(bool)));
    connect(child,SIGNAL(copyAvailable(bool)),ui->actionCut,SLOT(setEnabled(bool)));
    connect(child->document(),SIGNAL(undoAvailable(bool)),ui->actionUndo,SLOT(setEnabled(bool)));
    connect(child->document(),SIGNAL(redoAvailable(bool)),ui->actionRedo,SLOT(setEnabled(bool)));
    connect(child,SIGNAL(cursorPositionChanged()),this,SLOT(showTextRowAndCol()));
    return child;
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if(!window)
        return;
    ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty()){
        QMdiSubWindow *existing = findMdiChild(fileName);
        if(existing){
            ui->mdiArea->setActiveSubWindow(existing);
            return;
        }
        MdiChild *child = createMdiChild();
        if(child->loadFile(fileName)){
            ui->statusbar->showMessage(tr("打开文件成功"),2000);
            child->show();
        }else{
            child->close();
        }
    }
}

void MainWindow::updateWindowMenu()
{
    ui->menu_W->clear();
    ui->menu_W->addAction(ui->actionClose);
    ui->menu_W->addAction(ui->actionCloseAll);
    ui->menu_W->addSeparator();
    ui->menu_W->addAction(ui->actionTile);
    ui->menu_W->addAction(ui->actionCascade);
    ui->menu_W->addSeparator();
    ui->menu_W->addAction(ui->actionNext);
    ui->menu_W->addAction(ui->actionPrevious);
    QList<QMdiSubWindow *> windows = ui->mdiArea->subWindowList();
    actionSeparator->setVisible(!windows.isEmpty());
    for (int i = 0;i < windows.size();++i) {
        MdiChild *child = qobject_cast<MdiChild *>(windows.at(i)->widget());
        QString text;
        if(i < 9){
            text = tr("&%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
        }else{
            text = tr("%1 %2").arg(i + 1).arg(child->userFriendlyCurrentFile());
        }
        QAction *action = ui->menu_W->addAction(text);
        action->setCheckable(true);
        action->setChecked(child == activeMdiChild());
        connect(action,SIGNAL(triggered()),windowMapper,SLOT(map()));
        windowMapper->setMapping(action,windows.at(i));
    }
}

void MainWindow::on_actionSave_triggered()
{
    if(activeMdiChild() && activeMdiChild()->save())
        ui->statusbar->showMessage(tr("文件保存成功"),2000);
}

void MainWindow::on_actionSaveAs_triggered()
{
    if(activeMdiChild() && activeMdiChild()->saveAs())
        ui->statusbar->showMessage(tr("文件保存成功"),2000);
}

void MainWindow::on_actionUndo_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->redo();
}

void MainWindow::on_actionCut_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->cut();
}

void MainWindow::on_actionCopy_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->copy();
}

void MainWindow::on_actionPaste_triggered()
{
    if(activeMdiChild())
        activeMdiChild()->paste();
}

void MainWindow::on_actionClose_triggered()
{
    ui->mdiArea->closeActiveSubWindow();
}

void MainWindow::on_actionCloseAll_triggered()
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::on_actionTile_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actionCascade_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}

void MainWindow::on_actionNext_triggered()
{
    ui->mdiArea->activateNextSubWindow();
}

void MainWindow::on_actionPrevious_triggered()
{
    ui->mdiArea->activatePreviousSubWindow();
}

void MainWindow::on_actionExit_triggered()
{
    qApp->closeAllWindows();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox box;
    box.setWindowTitle(tr("关于"));
    box.setText(tr("这是文档编辑器练习"));
    box.addButton(tr("OK"),QMessageBox::YesRole);
    box.exec();
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this,tr("关于Qt"));
}

void MainWindow::showTextRowAndCol()
{
    if(activeMdiChild()){
        int rowNum = activeMdiChild()->textCursor().blockNumber() + 1;
        int colNum = activeMdiChild()->textCursor().columnNumber() + 1;
        ui->statusbar->showMessage(tr("%1行 %2列").arg(rowNum).arg(colNum),2000);
    }
}

void MainWindow::on_actionFont_triggered()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,activeMdiChild()->font(),this,"font");
    if(ok){
        activeMdiChild()->setCurrentFont(font);
    }
}

void MainWindow::on_actionFontColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::red,this,tr("颜色选择"));
    if(color.isValid()){
        activeMdiChild()->setTextColor(color);
    }
}

void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer;
    QPrintDialog dlg(&printer,ui->mdiArea->activeSubWindow());
    if(dlg.exec() == QDialog::Accepted){
        activeMdiChild()->print(&printer);
    }
}

void MainWindow::on_actionPrintPreview_triggered()
{
    QPrinter printer;
    QPrintPreviewDialog preview(&printer,this);
    connect(&preview,&QPrintPreviewDialog::paintRequested,this,&MainWindow::printPreview);
    preview.exec();
}

void MainWindow::printPreview(QPrinter *printer)
{
    activeMdiChild()->print(printer);
}

void MainWindow::on_actionCreatePDF_triggered()
{
    if(activeMdiChild()){
        QString fileName = QFileDialog::getSaveFileName(this, "Export PDF",activeMdiChild()->curFile + ".pdf","*.pdf");
        if (!fileName.isEmpty()) {
            if (QFileInfo(fileName).suffix().isEmpty())
                fileName.append(".pdf");
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            activeMdiChild()->print(&printer);
        }
    }
}

void MainWindow::on_actionFind_triggered()
{
    findReplace->show();
    findReplace->ui->tabWidget->setCurrentWidget(findReplace->ui->find);
    findReplace->ui->lineEditF->setFocus();
}

void MainWindow::on_actionReplace_triggered()
{
    findReplace->show();
    findReplace->ui->tabWidget->setCurrentWidget(findReplace->ui->replace);
    findReplace->ui->lineEditR->setFocus();
}

void MainWindow::findNextF()
{
    QString findText = findReplace->ui->lineEditF->text();
    senceF = findReplace->ui->checkBoxSenceF->isChecked();
    upFindF = findReplace->ui->radioButtonUpF->isChecked();
    doFind(findText);
}

void MainWindow::findNextR()
{
    QString findText = findReplace->ui->lineEditR->text();
    senceF = findReplace->ui->checkBoxSenceR->isChecked();
    upFindF = findReplace->ui->radioButtonUpR->isChecked();
    doFind(findText);
}

void MainWindow::replace()
{
    QString findText = findReplace->ui->lineEditR->text();
    QString replaceText = findReplace->ui->lineEditreplace->text();
    if(activeMdiChild()->textCursor().selectedText() == findText){
        QTextCursor textcursor = activeMdiChild()->textCursor();
        textcursor.insertText(replaceText);
    }
    findNextR();
}

void MainWindow::replaceAll()
{
    QTextCursor textcursor = activeMdiChild()->textCursor();
    textcursor.setPosition(0,QTextCursor::MoveAnchor);
    activeMdiChild()->setTextCursor(textcursor);
    findReplace->ui->radioButtonUpR->setChecked(false);
    senceF = findReplace->ui->checkBoxSenceR->isChecked();
    QString findText = findReplace->ui->lineEditR->text();
    QString replaceText = findReplace->ui->lineEditreplace->text();
    while(doReplaceAllFind(findText)){
        if(activeMdiChild()->textCursor().selectedText().toLower() == findText.toLower()){
            QTextCursor textcursor = activeMdiChild()->textCursor();
            textcursor.insertText(replaceText);
        }
    }
}

void MainWindow::slotAligment(QAction *a)
{
    if(a == left)
        activeMdiChild()->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if(a == center)
        activeMdiChild()->setAlignment(Qt::AlignHCenter);
    else if(a == right)
        activeMdiChild()->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if(a == justify)
        activeMdiChild()->setAlignment(Qt::AlignJustify);
}
