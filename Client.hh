#ifndef CLIENT_HH
#define CLIENT_HH

#include "Cluster.hh"

#include <QMainWindow>
#include <QMetaType>
#include <QtWidgets>

class Client : public QMainWindow
{
    Q_OBJECT
private:
    uint64_t writeCount = 0;
    Cluster cluster;
    QTimer timedWrite;
    QTimer timedRefresh;

    QLabel *lab_leader;
    QTextEdit *text_leader;
    QLabel *lab_packageLoss;
    QTextEdit *text_packageLoss;
    QLabel *lab_wrtieCount;
    QTextEdit *text_writeCount;
    QLabel *lab_commitIndex;
    QTextEdit *text_commitIndex;
    QLabel *lab_autoWrite;
    QCheckBox *check_autoWrite;


public:
    explicit Client(QWidget *parent = nullptr);
    ~Client();
public slots:
    void handleAutoWrite();
    void autoWrite();
    void write(const char *command);

    void startDataRefresh(int msec);
    void stopDataRefresh();
    void dataRefresh();
};
#endif // CLIENT_HH
