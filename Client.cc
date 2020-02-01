#include "Client.hh"

Client::Client(QWidget *parent)
    : QMainWindow(parent), cluster(15)
{
    cluster.run();

    lab_leader = new QLabel;
    text_leader = new QTextEdit;
    lab_packageLoss = new QLabel;
    text_packageLoss = new QTextEdit;
    lab_wrtieCount = new QLabel;
    text_writeCount = new QTextEdit;
    lab_commitIndex = new QLabel;
    text_commitIndex = new QTextEdit;
    lab_autoWrite = new QLabel;
    check_autoWrite = new QCheckBox;

    startDataRefresh(1000);
    connect(&timedWrite, &QTimer::timeout, this, &Client::autoWrite);
    connect(&timedRefresh, &QTimer::timeout, this, &Client::dataRefresh);

    QWidget *centralWidget = new QWidget;
    QHBoxLayout **h_layout = new QHBoxLayout *[5];
    for (int i = 0; i < 5; ++i)
    {
        h_layout[i] = new QHBoxLayout;
    }

    lab_leader->setText(tr("Leader's ID: "));
    lab_leader->setFixedSize(100, 30);
    text_leader->setText(tr("UNKNOWN"));
    text_leader->setFixedSize(100, 30);
    text_leader->setReadOnly(true);
    h_layout[0]->addWidget(lab_leader);
    h_layout[0]->addWidget(text_leader);

    lab_packageLoss->setText(tr("Package Loss Rate"));
    lab_packageLoss->setFixedSize(100, 30);
    text_packageLoss->setText(tr("0"));
    text_packageLoss->setFixedSize(100, 30);
    text_packageLoss->setReadOnly(true);
    h_layout[1]->addWidget(lab_packageLoss);
    h_layout[1]->addWidget(text_packageLoss);

    lab_wrtieCount->setText(tr("Entries Number"));
    lab_wrtieCount->setFixedSize(100, 30);
    text_writeCount->setText(tr("0"));
    text_writeCount->setFixedSize(100, 30);
    text_writeCount->setReadOnly(true);
    h_layout[2]->addWidget(lab_wrtieCount);
    h_layout[2]->addWidget(text_writeCount);

    lab_commitIndex->setText(tr("Commited Index"));
    lab_commitIndex->setFixedSize(100, 30);
    text_commitIndex->setText(tr("0"));
    text_commitIndex->setFixedSize(100, 30);
    text_commitIndex->setReadOnly(true);
    h_layout[3]->addWidget(lab_commitIndex);
    h_layout[3]->addWidget(text_commitIndex);

    lab_autoWrite->setText(tr("Auto Write Entries"));
    lab_autoWrite->setFixedSize(100, 30);
    check_autoWrite->setCheckState(Qt::Unchecked);
    connect(check_autoWrite, &QCheckBox::stateChanged, this, &Client::handleAutoWrite);
    h_layout[4]->addWidget(lab_autoWrite);
    h_layout[4]->addWidget(check_autoWrite);

    QVBoxLayout *layout = new QVBoxLayout;

    for (int i = 0; i < 5; ++i)
    {
        layout->addLayout(h_layout[i]);
    }

    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    delete []h_layout;

}

Client::~Client()
{
}


void Client::handleAutoWrite()
{
    if (check_autoWrite->checkState() == Qt::Checked)
    {
        timedWrite.start(3000);
    }
    else
    {
        timedWrite.stop();
    }
}

void Client::autoWrite()
{
    cluster.writeEntries("123");
    ++writeCount;
    qDebug() << "Send" << writeCount;
}

void Client::write(const char *command)
{
    cluster.writeEntries(command);
    ++writeCount;
    qDebug() << "Send" << writeCount;
}

void Client::startDataRefresh(int msec)
{
    timedRefresh.start(msec);
}

void Client::stopDataRefresh()
{
    timedRefresh.stop();
}

void Client::dataRefresh()
{
#ifdef DEBUG
    qDebug() << "Package Loss Rate:" << QString("%1").arg(100 * double(cluster.packageLoss) / double(cluster.packageSum), 0, 'f', 5) + "%"<< "Leader's ID:" << cluster.leaderID << "Commited Log Index:" << cluster.commitIndex;
#endif
    text_leader->setText(QString::number(cluster.leaderID));
    text_packageLoss->setText(QString("%1").arg(100 * double(cluster.packageLoss) / double(cluster.packageSum), 0, 'f', 5) + "%");
    text_writeCount->setText(QString::number(writeCount));
    text_commitIndex->setText(QString::number(cluster.commitIndex));

}


