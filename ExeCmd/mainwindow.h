#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QProcess>
#include <QFile>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    void readLine();
    void createCommandProcessor();
    void deleteCommandProcessor();
    ~MainWindow();
    
private slots:
    void on_executeLineButton_clicked();
    void on_readFileButton_clicked();
    void on_command_ready();
    void on_command_results();
    void on_command_error();

private:
    Ui::MainWindow *ui;

    char* mMainBuffer;

    QFile* mFile;

    QProcess* mCommandProcessor;

    QString mOutputString;
};

#endif // MAINWINDOW_H
