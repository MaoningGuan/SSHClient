#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_download_clicked();

    int download_file();

    int upload_file();

    int execute_cmd();

    void on_pushButton_upload_clicked();

    void on_pushButton_clear_2_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
