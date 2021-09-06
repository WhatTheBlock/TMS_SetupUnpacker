#include "widget.h"
#include "ui_widget.h"

//初始化設定
Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget) {
    QApplication::setStyle(QStyleFactory::create("Fusion")); //設定UI介面
    ui->setupUi(this);
    ui->background->viewport()->setCursor(Qt::ArrowCursor); //設定TextEdit的鼠標為標準箭頭
    ui->cmdOutput->viewport()->setCursor(Qt::ArrowCursor); //設定TextEdit的鼠標為標準箭頭

    QFile innounp(qApp->applicationDirPath() + "/innounp.exe");
    if(innounp.exists()) {
        innounpPath = innounp.fileName().replace("/","\\");
    }
    else {
        warningMsg(QStringLiteral("未偵測到innounp.exe，請勿擅自更改檔名或移動檔案。"));
        exit(EXIT_FAILURE);
    }
}

Widget::~Widget() {
    delete ui;
}

//提醒訊息
void Widget::warningMsg(QString msg) {
    QMessageBox::warning(this, QStringLiteral("注意！"), msg);
}

//設定遊戲安裝檔的路徑
void Widget::on_setSetupPath_clicked() {
    QString temp = QFileDialog::getOpenFileName(this, QStringLiteral("選擇安裝檔"), "/", "MapleStoryVxxx.exe (*.exe)");

    if(!temp.isEmpty()) {
        setupPath = temp.replace("/","\\");
        cmd = "\"" + innounpPath + "\" \"" + setupPath + "\"";
        runCmd(cmd, 1);
    }
    else {
        warningMsg(QStringLiteral("未選擇安裝檔！"));
        setupPath.clear();
        ui->setSetupPath->setText(QStringLiteral("遊戲安裝檔的路徑尚未設定"));
    }
}

//設定遊戲安裝的路徑
void Widget::on_setGamePath_clicked() {
    QDir dirTmp;
    QDir dirApp;
    QDir dirGame;
    QFile iss;
    QString temp = QFileDialog::getExistingDirectory(this, QStringLiteral("選擇遊戲安裝的路徑"), "/", QFileDialog::ShowDirsOnly);

    if(temp.length() > 3) {
        gamePath_upLv = temp.mid(0, temp.lastIndexOf("/") + 1);
        dirTmp.setPath(gamePath_upLv + "{tmp}");
        dirApp.setPath(gamePath_upLv + "{app}");
        dirGame.setPath(temp);
        iss.setFileName(gamePath_upLv + "install_script.iss");

        if(dirGame.isEmpty()) {
            if(!dirTmp.exists() && !dirApp.exists() && !iss.exists()) {
                dirName = temp.mid(gamePath_upLv.length());
                gamePath = temp.replace("/","\\") + "\\";
                gamePath_upLv = gamePath_upLv.replace("/","\\");
                ui->setGamePath->setText(gamePath);
            }
            else {
                warningMsg(QStringLiteral("安裝路徑的上層不可含有{tmp}、{app}目錄\n與install_script.iss檔案。"));
                gamePath.clear();
                ui->setGamePath->setText(QStringLiteral("遊戲安裝的路徑尚未設定"));
            }
        }
        else {
            warningMsg(QStringLiteral("%1目錄下不可包含檔案").arg(dirGame.path().replace("/","\\")));
            gamePath.clear();
            ui->setGamePath->setText(QStringLiteral("遊戲安裝的路徑尚未設定"));
        }
    }
    //安裝路徑設定根目錄則自動加上MapleStory
    else if(temp.length() == 3) {
        dirTmp.setPath(temp + "{tmp}");
        dirApp.setPath(temp + "{app}");
        dirGame.setPath(temp + "MapleStory");
        iss.setFileName(temp + "install_script.iss");

        if(!dirGame.exists() || dirGame.isEmpty()) {
            if(!dirTmp.exists() && !dirApp.exists() && !iss.exists()) {
                dirName = "MapleStory";
                gamePath = temp.replace("/","\\");
                gamePath_upLv = gamePath;
                gamePath += dirName + "\\";
                ui->setGamePath->setText(gamePath);
            }
            else {
                warningMsg(QStringLiteral("根目錄不可含有{tmp}、{app}目錄\n與install_script.iss檔案。"));
                gamePath.clear();
                ui->setGamePath->setText(QStringLiteral("遊戲安裝的路徑尚未設定"));
            }
        }
        else {
            warningMsg(QStringLiteral("%1必須為空或不存在").arg(dirGame.path().replace("/","\\")));
            gamePath.clear();
            ui->setGamePath->setText(QStringLiteral("遊戲安裝的路徑尚未設定"));
        }
    }
    else {
        warningMsg(QStringLiteral("未選擇遊戲安裝的路徑！"));
        gamePath.clear();
        ui->setGamePath->setText(QStringLiteral("遊戲安裝的路徑尚未設定"));
    }
}

//執行指令
void Widget::runCmd(QString cmd, int mode) {
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
            switch (mode) {
            case 1: //驗證安裝檔並取得檔案數量
                {
                    QString text = ui->cmdOutput->toPlainText();
                    ui->cmdOutput->clear();

                    if(text.indexOf("Files: ") != -1) {
                        //支援的遊戲檔案數量為100~9999 (64bit客戶端若檔案拆分的很徹底將有可能超過9999)
                        text = text.mid(text.indexOf("Files: ") + 7, 4).replace(" ", "");
                        ui->extracted->setText("0");
                        ui->total->setText(text);
                        ui->progress->setMaximum(text.toInt());
                        ui->setSetupPath->setText(setupPath);
                    }
                    else {
                        warningMsg(QStringLiteral("錯誤的安裝檔，請重新選擇。"));
                        setupPath.clear();
                        ui->setSetupPath->setText(QStringLiteral("遊戲安裝檔的路徑尚未設定"));
                    }
                }
                break;
            case 2: //遊戲安裝結束後
                {
                    isStart = false;
                    ui->cmdOutput->clear();
                    ui->setSetupPath->setEnabled(true);
                    ui->setGamePath->setEnabled(true);
                    ui->start->setEnabled(true);

                    QDir *dir = new QDir(gamePath_upLv + "{tmp}");
                    dir->removeRecursively();
                    dir->setPath(gamePath);
                    dir->removeRecursively();
                    dir->rename(gamePath_upLv + "{app}", gamePath_upLv + dirName);
                    QFile iss(gamePath_upLv + "install_script.iss");
                    iss.remove();
                }
                break;
            }
        });

    process->write("exit\n\r");
}

//輸出執行過程
void Widget::realTimeReadOut() {
    QProcess *p = dynamic_cast<QProcess *>(sender());
    QString temp_Output = QString::fromLocal8Bit(p->readAllStandardOutput());
    //QString temp_Error = QString::fromLocal8Bit(p->readAllStandardError());
    //qDebug() << temp_Output;

    ui->cmdOutput->append(temp_Output);
    //ui->cmdOutput->append(temp_Error);

    if(isStart) {
        if(temp_Output.indexOf("#") != -1) {
            QString temp = temp_Output.mid(temp_Output.indexOf("#") + 1, temp_Output.indexOf(" ") - 1);
            ui->extracted->setText(temp);
            ui->progress->setValue(temp.toInt());
        }
        else if(temp_Output.indexOf("; Version") != -1) {
            ui->cmdOutput->clear();
        }
    }
}

//開始安裝
void Widget::on_start_clicked() {
    if(!setupPath.isEmpty() && !gamePath.isEmpty()) {
        cmd = "\"" + innounpPath + "\" -x -d\"" + gamePath_upLv + "\" -a -y \"" + setupPath + "\"";
        ui->setSetupPath->setEnabled(false);
        ui->setGamePath->setEnabled(false);
        ui->start->setEnabled(false);
        isStart = true;
        runCmd(cmd, 2);
    }
    else {
        warningMsg(QStringLiteral("請檢查安裝檔與安裝路徑是否均設定。"));
    }
}

