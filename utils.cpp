#include "widget.h"
#include "ui_widget.h"

//提醒訊息
void Widget::warningMsg(QString msg) {
    QMessageBox::warning(this, QStringLiteral("OAO"), msg);
}

//錯誤訊息
void Widget::errorMsg(QString msg) {
    QMessageBox::critical(this, QStringLiteral("QnQ"), msg);
}

//普通訊息
void Widget::infoMsg(QString msg) {
    QMessageBox::information(this, QStringLiteral("OuO"), msg);
}

//檢查分割檔是否完整
void Widget::checkSlices() {
    for (int i = 1; i <= slices; i++) {
        if(!QFile::exists(setupPath.chopped(4).append("-%1").arg(i).append(".bin"))) {
            warningMsg(QStringLiteral("請檢查安裝分割檔(*.bin)是否有%1個，\n"
                                      "並勿擅自更改檔名或移動檔案。").arg(slices));
            setupPath.clear();
            ui->total->setText("0");
            ui->setSetupPath->setText(QStringLiteral("遊戲安裝檔的路徑尚未設定"));
            break;
        }
    }
}
