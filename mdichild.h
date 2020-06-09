#ifndef MDICHILD_H
#define MDICHILD_H
#include <QTextEdit>

class MdiChild : public QTextEdit
{
    Q_OBJECT
public:
    explicit MdiChild(QWidget * parent = nullptr);
    void newFile();     //新建文件
    bool loadFile(const QString &fileName);     //加载文件
    bool save();        //保存
    bool saveAs();      //另存为
    bool saveFile(const QString &fileName);     //保存文件
    QString userFriendlyCurrentFile();      //提取文件名
    QString currentFile(){return curFile;}
    QString curFile;    //保存当前文件路径

private:
    bool maybeSave();   //是否是需要保存
    void setCurrentFile(const QString &fileName);   //设置当前文件

    bool isUntitled;    //标志当前文件是否保存到硬盘上

protected:
    void closeEvent(QCloseEvent * event);   //关闭事件
    void contextMenuEvent(QContextMenuEvent * e);   //右键菜单

private slots:
    void documentWasModified(); //文件被更改时，窗口显示被更改标志
};

#endif // MDICHILD_H
