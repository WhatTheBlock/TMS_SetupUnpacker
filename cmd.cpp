#include "widget.h"
#include "ui_widget.h"

//執行指令
void Widget::runCmd(QString cmd, int mode) {
    ui->cmdOutput->clear();
    cmd += "\n\r";
    encodedString = codec->fromUnicode(cmd);

    process = new QProcess(this);
    process->start("cmd");
    process->write(encodedString.data());
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(realTimeReadOut()));
    connect(process, &QProcess::errorOccurred, this, &Widget::processError);
    connect(process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [=]() {
            switch (mode) {
            case 1: //驗證安裝檔
                verifyInstaller();
                break;
            case 2: //遊戲安裝結束後
                afterInstallation();
                break;
            }
        });
    process->write("exit\n\r");
}

//輸出執行過程
void Widget::realTimeReadOut() {
    QProcess *p = dynamic_cast<QProcess *>(sender());
    QString temp_Output = QString::fromLocal8Bit(p -> readAllStandardOutput());

    ui->cmdOutput->append(temp_Output);

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

//捕獲錯誤訊息
void Widget::processError(QProcess::ProcessError error) {
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg(QStringLiteral("innounp啟動失敗，請重試。"));
        break;
    case QProcess::Crashed:
        errorMsg(QStringLiteral("innounp已崩潰，請確保有足夠的記憶體並重試。"));
        break;
    case QProcess::Timedout:
        errorMsg(QStringLiteral("執行逾時，如發生此問題請回報\n"
                                    "並附上工作管理員的效能狀態。"));
        break;
    case QProcess::WriteError:
        errorMsg(QStringLiteral("寫入失敗，請重試。"));
        break;
    case QProcess::ReadError:
        errorMsg(QStringLiteral("讀取失敗，請重試。"));
        break;
    case QProcess::UnknownError:
        errorMsg(QStringLiteral("發生預期外的錯誤。"));
        break;
    }
}

//驗證安裝檔
void Widget::verifyInstaller() {
    QString text = ui->cmdOutput->toPlainText();
    ui->cmdOutput->clear();

    if(text.indexOf("Files: ") != -1) {
        //取得遊戲檔案數量
        setTotalFiles(text);

        //檢查分割檔是否完整
        checkSlices(text);

        //是否為測試服
        isOT = (setupPath.right(6).mid(0,2) == "OT")?true:false;
    }
    else {
        warningMsg(QStringLiteral("錯誤的安裝檔，請重新選擇。"));
        setupPath.clear();
        ui->setSetupPath->setText(QStringLiteral("遊戲安裝檔的路徑尚未設定"));
    }
}

//遊戲安裝結束後
void Widget::afterInstallation() {
    QDir *dir = new QDir(gamePath_upLv + "{tmp}");
    QFile iss(gamePath_upLv + "install_script.iss");

    isStart = false;
    ui->setSetupPath->setEnabled(true);
    ui->setGamePath->setEnabled(true);
    ui->progress->setValue(0);
    ui->start->setEnabled(true);

    //安裝失敗
    if(ui->extracted->text() != ui->total->text()) {
        ui->extracted->setText("0");

        //移除安裝不完全的遊戲檔案
        dir->removeRecursively();
        dir->setPath(gamePath_upLv + "{app}");
        dir->removeRecursively();

        errorMsg(QStringLiteral("安裝失敗，殘餘的遊戲檔案已移除，\n"
                                "回報錯誤時請提供含有Exception字樣的畫面截圖。"));
    }
    //安裝成功
    else {
        ui->cmdOutput->clear(); //安裝成功才清除，方便失敗時查看出問題的分割檔

        //移除多餘的檔案 & 重命名目錄名稱
        dir->removeRecursively();
        dir->setPath(gamePath);
        dir->removeRecursively(); //避免重命名目錄名稱發生錯誤
        dir->rename(gamePath_upLv + "{app}", gamePath_upLv + dirName);
        iss.remove();

        //設定安裝路徑讓Beanfun可直接偵測到遊戲
        if(!isOT) {
            QSettings registryPath("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\GAMANIA\\MAPLESTORY", QSettings::NativeFormat);
            registryPath.setValue("Path", gamePath.chopped(1));
        }
        else {
            QSettings registryPath("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\GAMANIA\\MapleStory_TestServer", QSettings::NativeFormat);
            registryPath.setValue("Path", gamePath.chopped(1));
        }

        infoMsg(QStringLiteral("安裝完畢！接下來請按照不同的登入方式操作：\n\n"
                                  "1. 第三方登入器設定【%1MapleStory.exe】路徑後即可啟動遊戲\n\n"
                                  "2. Beanfun若顯示無法偵測安裝狀態請點選 [確認安裝]，"
                                  "並設定【%1MapleStory.exe】的路徑，重新整理網頁後即可啟動遊戲").arg(gamePath));
    }
}
