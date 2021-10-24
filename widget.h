#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <QStyleFactory>
#include <QProcess>
#include <QTextCodec>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_setSetupPath_clicked();

    void on_setGamePath_clicked();

    void realTimeReadOut();

    void on_start_clicked();

private:
    Ui::Widget *ui;

    QString toolPath;
    QString setupPath;
    QString gamePath, gamePath_upLv;
    QString dirName;
    QString cmd;

    int files;
    int slices;

    bool isStart;

    //統一命令編碼
    QTextCodec *codec = QTextCodec::codecForName("Big5");
    QByteArray encodedString;

    QProcess *process;

    void runCmd(QString, int);

    void warningMsg(QString);
};
#endif // WIDGET_H
