#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDir>

const qint64 MaxLine = 10000;
#define MessageBoxTitle "ExeCmd file error"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    mFile = 0;
    mOutputString = "";
    ui->setupUi(this);
    mMainBuffer = new char[MaxLine];
    ui->commandFileName->setText("SampleCommand.execmd");

}

MainWindow::~MainWindow()
{
    if(mFile != 0)
    {
      mFile->close();
      delete mFile;
      mFile = 0;
    }
    if(mCommandProcessor != 0)
    {
        deleteCommandProcessor();
    }

    delete ui;
    delete mMainBuffer;

    deleteCommandProcessor();
}

void MainWindow::on_executeLineButton_clicked()
{
    mOutputString = ""; //clear the output
    QString textToExecute = ui->LineDisplay->text();

    if(textToExecute.size() > 0)
    {
        // the QT process does not handle "detonate" correctly (at least not in Windows)
        // so we treat it specially
        if(textToExecute.contains("detonate"))
        {
            mCommandProcessor->terminate(); //MUST use terminate "close" crashes!
            mCommandProcessor = 0;
            ui->executeLineButton->setEnabled(false);
            mOutputString = "Command file terminated";
            ui->outputWindow->setPlainText(mOutputString);
            return;
        }
        QStringList commandList;
        commandList = textToExecute.split(" ",QString::SkipEmptyParts,Qt::CaseInsensitive);
        mCommandProcessor->write(textToExecute.toLatin1());
    }
    else{
        QMessageBox::warning(0,MessageBoxTitle,"Text contains characters that are not allowed");
    }

    readLine();
}
void MainWindow::on_readFileButton_clicked()
{
    if(mMainBuffer == 0)
        return;
    mOutputString = "";
    ui->outputWindow->setPlainText(mOutputString);
    QString path = QDir::currentPath();
    QString filename = ui->commandFileName->toPlainText();
    QString fullFileName = path + "/" + filename;
    mFile = new QFile(fullFileName);
    if(mFile->exists()){
        ui->executeLineButton->setEnabled(true);
        qint64 size = mFile->size();
        if(size > 0 && mFile->open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QStringList commandList;
            commandList.append("");
            if(mCommandProcessor == 0)
            {
                createCommandProcessor();
            }

            mCommandProcessor->start("cmd.exe",commandList);
            mCommandProcessor->waitForStarted();

            readLine();
        }
        else{
            deleteCommandProcessor();
            QString errorMessage = mFile->errorString();
            QMessageBox::warning(0,MessageBoxTitle,"File read error: " + errorMessage);
        }
    }
    else {
        mFile = 0;
        ui->executeLineButton->setEnabled(false);
        QMessageBox::warning(0,MessageBoxTitle,"No such file!");
    }

}

void MainWindow::on_command_ready(){
    QString newOutputString = mCommandProcessor->readAllStandardOutput();
    if(!newOutputString.contains("C:\\")){
        // let's not repeatedly write the path string!
        mOutputString += newOutputString;
        ui->outputWindow->setPlainText(mOutputString);
    }
}

void MainWindow::on_command_results(){
    ui->outputWindow->setPlainText(mCommandProcessor->readLine());
}

void MainWindow::on_command_error(){
    switch(mCommandProcessor->error())
    {
    case QProcess::FailedToStart:
        QMessageBox::information(0,MessageBoxTitle,"FailedToStart");
        break;
    case QProcess::Crashed:
        QMessageBox::information(0,MessageBoxTitle,"Crashed");
        break;
    case QProcess::Timedout:
        QMessageBox::information(0,MessageBoxTitle,"FailedToStart");
        break;
    case QProcess::WriteError:
        QMessageBox::information(0,MessageBoxTitle,"Timedout");
        break;
    case QProcess::ReadError:
        QMessageBox::information(0,MessageBoxTitle,"ReadError");
        break;
    case QProcess::UnknownError:
        QMessageBox::information(0,MessageBoxTitle,"UnknownError");
        break;
    default:
        QMessageBox::information(0,MessageBoxTitle,"default");
        break;
    }
    ui->executeLineButton->setEnabled(false);
}

void MainWindow::readLine(){
    if(mMainBuffer == 0 || mFile == 0 )
        return;

    qint64 lineSize = 0;

    qint64 count = 0;
    while((lineSize = mFile->readLine(mMainBuffer,MaxLine)) == -1 || count > 1000){ //don't loop forever, even if nothings happening
        if(lineSize == 0){
            QMessageBox::warning(0,MessageBoxTitle,"End of file");
            return;
        }
    }
    if(lineSize > 0){
        ui->LineDisplay->setText(mMainBuffer);
        mFile->read(1); // need to remove the "\n"
    }
    else {
        QMessageBox::warning(0,MessageBoxTitle,"End of file reached");
    }
}

void MainWindow::createCommandProcessor()
{
    mCommandProcessor = new QProcess;

    connect(mCommandProcessor,SIGNAL(readyRead()),this,SLOT(on_command_ready()));
    connect(mCommandProcessor,SIGNAL(error(QProcess::ProcessError)),this,SLOT(on_command_error()));
}
void MainWindow::deleteCommandProcessor()
{
    disconnect(mCommandProcessor,SIGNAL(readyRead()),this,SLOT(on_command_ready()));
    disconnect(mCommandProcessor,SIGNAL(error(QProcess::ProcessError)),this,SLOT(on_command_error()));

    delete mCommandProcessor;
    mCommandProcessor = 0;

}
