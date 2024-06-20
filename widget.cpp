#include "widget.h"
#include "ui_widget.h"
#include "sfp.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_download_clicked()
{
    this->download_file();
}

// 下载文件
int Widget::download_file()
{
    Sftp fg;

    QString ip = ui->lineEdit_IP->text();
    QString port = ui->lineEdit_Port->text();
    QString username = ui->lineEdit_User->text();
    QString password = ui->lineEdit_pwd->text();

    int ret = fg.Connect(ip.toStdString().c_str(), port.toInt(), username.toStdString().c_str(),
                        password.toStdString().c_str());
    if (ret)
    {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        ui->textEdit->setText((errmsg));
        return -1;
    }

    QString src = ui->lineEdit_src->text();
    QString dest = ui->lineEdit_dest->text();
    ret = fg.Download(src.toStdString().c_str(), dest.toStdString().c_str());
    if (ret)
    {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        ui->textEdit->setText((errmsg));
        return -1;
    }

    ui->textEdit->setText(QString("Download file %1 to %2 successfully!").arg(src, dest));

    return 0;
}

// 上传文件
int Widget::upload_file()
{
    Sftp fg;

    QString ip = ui->lineEdit_IP->text();
    QString port = ui->lineEdit_Port->text();
    QString username = ui->lineEdit_User->text();
    QString password = ui->lineEdit_pwd->text();

    int ret = fg.Connect(ip.toStdString().c_str(), port.toInt(), username.toStdString().c_str(),
                         password.toStdString().c_str());
    if (ret)
    {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        ui->textEdit->setText((errmsg));
        return -1;
    }

    QString src = ui->lineEdit_src->text();
    QString dest = ui->lineEdit_dest->text();
    ret = fg.Upload(src.toStdString().c_str(), dest.toStdString().c_str());
    if (ret)
    {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        ui->textEdit->setText((errmsg));
        return -1;
    }

    ui->textEdit->setText(QString("Upload file %1 to %2 successfully!").arg(dest, src));

    return 0;
}

int Widget::execute_cmd()
{
    QString cmd = ui->textEdit->toPlainText();

    Sftp fg;

    QString ip = ui->lineEdit_IP->text();
    QString port = ui->lineEdit_Port->text();
    QString username = ui->lineEdit_User->text();
    QString password = ui->lineEdit_pwd->text();

    int ret = fg.Connect(ip.toStdString().c_str(), port.toInt(), username.toStdString().c_str(),
                         password.toStdString().c_str());
    if (ret) {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        ui->textEdit->setText(errmsg);
        return -1;
    }

    ret = fg.ExecuteCommad(cmd.toStdString().c_str());
    if (ret) {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        ui->textEdit->setText(errmsg);
        return -1;
    }

    QString output = cmd.append("\n").append(fg.GetOutputText());
    ui->textEdit->setText(output);

    return 0;
}


void Widget::on_pushButton_upload_clicked()
{
    this->upload_file();
}


void Widget::on_pushButton_clear_2_clicked()
{
    this->execute_cmd();
}

