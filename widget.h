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
    void on_setInnounpPath_clicked();

    void on_setSetupPath_clicked();

    void on_setGamePath_clicked();

    void realTimeReadOut();

    void on_start_clicked();

private:
    Ui::Widget *ui;

    QString innounpPath;
    QString setupPath;
    QString gamePath;

    int files;

    //統一命令編碼
    QTextCodec *codec = QTextCodec::codecForName("Big5");
    QByteArray encodedString;

    QProcess *process;

    void runCmd(QString, bool);
};
#endif // WIDGET_H
