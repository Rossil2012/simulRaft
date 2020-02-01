#include "Client.hh"
#include "Package.hh"

#include <QApplication>
#include <QMetaType>

Q_DECLARE_METATYPE(uint32_t);
Q_DECLARE_METATYPE(uint64_t);
Q_DECLARE_METATYPE(QVector<packagePair>);

int main(int argc, char *argv[])
{
    qRegisterMetaType<uniformRPC>("uniformRPC");
    qRegisterMetaType<uniformRPC>("uniformRPC&");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<QVector<packagePair>>("QVector<packagePair>");

    QApplication a(argc, argv);
    Client w;
    w.show();
    return a.exec();
}
