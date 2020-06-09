#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class MdiChild;
class QMdiSubWindow;
class QSignalMapper;
class QPrinter;
class Find_Replace;
class QActionGroup;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QAction * actionSeparator;
    MdiChild *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
    QSignalMapper *windowMapper;
    Find_Replace *findReplace;//查找替换对话框
    //对齐方式
    QAction *left;
    QAction *right;
    QAction *center;
    QAction *justify;
    QActionGroup *actGrp;
    QToolBar *toolbar;

    void readSettings();
    void writeSettings();
    void initWindow();
    //查找替换
    bool senceF;//是否区分大小写
    bool upFindF;//是否向后查找
    void doFind(QString findText);//执行查找
    bool doReplaceAllFind(QString findText);//执行全部替换

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void updateMenus();//更新菜单可用状态
    MdiChild *createMdiChild();//创建文档子类
    void setActiveSubWindow(QWidget *window);//设置活动窗口
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void updateWindowMenu();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionClose_triggered();
    void on_actionCloseAll_triggered();
    void on_actionTile_triggered();
    void on_actionCascade_triggered();
    void on_actionNext_triggered();
    void on_actionPrevious_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();
    void showTextRowAndCol();
    void on_actionFont_triggered();
    void on_actionFontColor_triggered();
    void on_actionPrint_triggered();
    void on_actionPrintPreview_triggered();
    void printPreview(QPrinter *printer);
    void on_actionCreatePDF_triggered();
    void on_actionFind_triggered();
    void on_actionReplace_triggered();
    void findNextF();
    void findNextR();
    void replace();
    void replaceAll();
    void slotAligment(QAction *a);//对齐方式指令集合
};
#endif // MAINWINDOW_H
