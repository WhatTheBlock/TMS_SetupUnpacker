#include "widget.h"
#include "ui_widget.h"

//初始化設定
Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    QApplication::setStyle(QStyleFactory::create("Fusion")); //設定UI介面
    ui->setupUi(this);
    ui->background->viewport()->setCursor(Qt::ArrowCursor); //設定TextEdit的鼠標為標準箭頭
    ui->cmdOutput->viewport()->setCursor(Qt::ArrowCursor); //設定TextEdit的鼠標為標準箭頭
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_setInnounpPath_clicked()
{
    innounpPath = QFileDialog::getOpenFileName(this, QStringLiteral("選擇innounp.exe"), "/", "innounp (innounp.exe)");
    if(innounpPath != "") {
        innounpPath = innounpPath.replace("/","\\");
        ui->setInnounpPath->setText(innounpPath);
    }
    else {

    }
}


void Widget::on_setSetupPath_clicked()
{
    setupPath = QFileDialog::getOpenFileName(this, QStringLiteral("選擇TMS安裝檔"), "/", "MapleStoryVxxx.exe (*.exe)");
    if(setupPath != "") {
        setupPath = setupPath.replace("/","\\");
        ui->setSetupPath->setText(setupPath);
    }
    else {

    }
}


void Widget::on_setGameSetupPath_clicked()
{
    gameSetupPath = QFileDialog::getExistingDirectory(this, QStringLiteral("選擇遊戲安裝的路徑"), "/", QFileDialog::ShowDirsOnly);
    if(gameSetupPath != "") {
        gameSetupPath = gameSetupPath.replace("/","\\");
        ui->setGameSetupPath->setText(gameSetupPath);
    }
    else {

    }
}

void Widget::runCmd(QString cmd, bool startInstall) {
    ui->cmdOutput->clear();
    cmd += "\n\r";
    encodedString = codec->fromUnicode(cmd);

    process = new QProcess(this);
    process->start("cmd");
    process->write(encodedString.data());
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(realTimeReadOut()));
    //connect(process, SIGNAL(readyReadStandardError()), this, SLOT(realTimeReadOut()));
    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus exitStatus)
        {
            if(startInstall) {
                ui->cmdOutput->clear();
            }
            else {
                QString text = ui->cmdOutput->toPlainText();
                text = text.mid(text.indexOf("Files: ") + 7, 3);
                //qDebug() << text;
                ui->total->setText(text);
                ui->cmdOutput->clear();
                ui->progress->setMaximum(text.toInt());
            }
        });

    process->write("exit\n\r");
}

void Widget::realTimeReadOut() {
    QProcess *p = dynamic_cast<QProcess *>(sender());
    QString temp_Output = QString::fromLocal8Bit(p->readAllStandardOutput());
    //QString temp_Error = QString::fromLocal8Bit(p->readAllStandardError());
    //qDebug() << temp_Output;

    ui->cmdOutput->append(temp_Output);
    //ui->cmdOutput->append(temp_Error);

    if(temp_Output.indexOf("#") != -1) {
        if(temp_Output.indexOf("install_script.iss") != -1) {
            ui->extracted->setText(ui->total->text());
            ui->progress->setValue(ui->progress->maximum());
        }
        else {
            QString temp = temp_Output.mid(temp_Output.indexOf("#") + 1, temp_Output.indexOf(" {") - 1);
            ui->extracted->setText(temp);
            ui->progress->setValue(temp.toInt());
        }

    }
}


void Widget::on_start_clicked()
{
    QString cmd;

    //get total files
    cmd = "\"" + innounpPath + "\" \"" + setupPath + "\"";
    runCmd(cmd, false);

    //start install
    cmd = "\"" + innounpPath + "\" -x -d\"" + gameSetupPath + "\\\" -a \"" + setupPath + "\"";
    runCmd(cmd, true);
}

