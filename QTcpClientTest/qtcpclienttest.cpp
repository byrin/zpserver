﻿#include "qtcpclienttest.h"
#include <QSettings>
QTcpClientTest::QTcpClientTest(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    //Paramenters
    QSettings settings("goldenhawking club","QTcpClientTest",this);
    ui.lineEdit_ip->setText(settings.value("ip","localhost").toString());
    ui.lineEdit_Port->setText(settings.value("port","23").toString());
    ui.dial->setValue(settings.value("clientNum","2").toInt());
    ui.lcdNumber->display(settings.value("clientNum","2").toInt());
    ui.horizontalSlider->setValue(settings.value("Payload","4096").toInt());
    ui.label_load->setText(QString("Payload = %1").arg(settings.value("Payload","4096").toInt()));
    ui.listView_msg->setModel(&model);
}

QTcpClientTest::~QTcpClientTest()
{

}
void QTcpClientTest::on_horizontalSlider_valueChanged(int value)
{
    ui.label_load->setText(QString("Payload = %1").arg(value));
}
void QTcpClientTest::on_action_Connect_triggered(bool bConn)
{
    //connect to the server
    QSettings settings("goldenhawking club","QTcpClientTest",this);
    settings.setValue("ip",ui.lineEdit_ip->text());
    settings.setValue("port",ui.lineEdit_Port->text());
    settings.setValue("clientNum",ui.dial->value());
    settings.setValue("Payload",ui.horizontalSlider->value());
    if (bConn==true)
    {
        nTimer = startTimer(300);
    }
    else
        killTimer(nTimer);
}
void QTcpClientTest::on_client_trasferred(qint64 dtw)
{
    QGHTcpClient * pSock = qobject_cast<QGHTcpClient*>(sender());
    if (pSock)
    {
        displayMessage(QString("client %1 Transferrd %2 bytes.").arg((quintptr)pSock).arg(dtw));
    }

}
void QTcpClientTest::on_client_connected()
{
    QGHTcpClient * pSock = qobject_cast<QGHTcpClient*>(sender());
    if (pSock)
    {
        displayMessage(QString("client %1 disconnected.").arg((quintptr)pSock));
        pSock->SendData(QByteArray(qrand()%1024+1024,qrand()%(128-32)+32));
    }

}
void QTcpClientTest::on_client_disconnected()
{
    QGHTcpClient * pSock = qobject_cast<QGHTcpClient*>(sender());
    if (pSock)
    {
        displayMessage(QString("client %1 disconnected.").arg((quintptr)pSock));
        //disconnect the signal immediately so that the system resource could be freed.
        disconnect(pSock, SIGNAL(readyRead()),this, SLOT(new_data_recieved()));
        disconnect(pSock, SIGNAL(connected()),this, SLOT(on_client_connected()));
        disconnect(pSock, SIGNAL(disconnected()),this,SLOT(on_client_disconnected()));
        disconnect(pSock, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(displayError(QAbstractSocket::SocketError)));
        disconnect(pSock, SIGNAL(bytesWritten(qint64)), this, SLOT(on_client_trasferred(qint64)));
        m_clients.remove(pSock);
        pSock->deleteLater();
    }
}
void QTcpClientTest::displayError(QAbstractSocket::SocketError err)
{
    QGHTcpClient * sock = qobject_cast<QGHTcpClient *> (sender());
    if (sock)
        displayMessage(QString("client %1 error msg:").arg((quintptr)sock)+sock->errorString());
}
void QTcpClientTest::new_data_recieved()
{
    QTcpSocket * pSock = qobject_cast<QTcpSocket*>(sender());
    if (pSock)
    {
        QByteArray array =pSock->readAll();
        //in this example, we just do nothing but to display the byte size.
        displayMessage(QString("client %1 Recieved %2 bytes.").arg((quintptr)pSock).arg(array.size()));
    }
}

void QTcpClientTest::timerEvent(QTimerEvent * evt)
{
    if (evt->timerId()==nTimer)
    {
        int nTotalClients = ui.dial->value();
        QList<QGHTcpClient*> listObj = m_clients.keys();
        foreach(QGHTcpClient * sock,listObj)
        {
            if (rand()%10<3)
                //3/10 possibility to send a data block to server
                sock->SendData(QByteArray(qrand()%1024+1024,qrand()%(128-32)+32));
        }
        //
        if (rand()%10 <1)
        {
            //1/10 chance to make new connections.
            if (m_clients.size()>nTotalClients)
            {
                int nDel = m_clients.size()-nTotalClients;
                QList<QGHTcpClient*> listObj = m_clients.keys();
                for (int i=0;i<nDel;i++)
                {
                    listObj.at(i)->disconnectFromHost();
                }
            }
            QGHTcpClient * client = new QGHTcpClient(this,ui.horizontalSlider->value());
            client->connectToHost(ui.lineEdit_ip->text(),ui.lineEdit_Port->text().toUShort());
            m_clients[client] = QDateTime::currentDateTime();
            connect(client, SIGNAL(readyRead()),this, SLOT(new_data_recieved()));
            connect(client, SIGNAL(connected()),this, SLOT(on_client_connected()));
            connect(client, SIGNAL(disconnected()),this,SLOT(on_client_disconnected()));
            connect(client, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(displayError(QAbstractSocket::SocketError)));
            connect(client, SIGNAL(bytesWritten(qint64)), this, SLOT(on_client_trasferred(qint64)));
        }
    }
}
void QTcpClientTest::displayMessage(const QString &str)
{
    model.insertRow(0,new QStandardItem(str));
    while (model.rowCount()>=256)
        model.removeRow(model.rowCount()-1);
}