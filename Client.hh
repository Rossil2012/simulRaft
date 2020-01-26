#ifndef CLIENT_HH
#define CLIENT_HH

#include <QMainWindow>

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();
};
#endif // CLIENT_HH
