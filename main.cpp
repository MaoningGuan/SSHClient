#include "widget.h"

#include <QApplication>

#include "sftp_get.h"

// 测试从SFtp,上传、下载文件
int Upload2DownloadFile_SFtp()
{
    SftpGet fg;
    int ret = fg.Connect("172.22.129.14", 22, "gmn", "187020");
    if (ret)
    {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        return -1;
    }

    int len = 1024 * 1024;
    char buf[1024 * 1024] = { 0 };
    ret = fg.SFtpRead("/home/gmn/test.txt", buf, &len);
    if (ret)
    {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        return -1;
    }
    qDebug() << buf;

    ret = fg.Download("/home/gmn/test.txt", "./test2.txt");
    if (ret)
    {
        QString errmsg = fg.GetLastError();
        qDebug() << errmsg;
        return -1;
    }
    return 0;
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Upload2DownloadFile_SFtp();
    Widget w;
    w.show();
    return a.exec();
}
