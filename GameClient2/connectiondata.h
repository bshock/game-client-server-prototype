#ifndef CONNECTIONDATA_H
#define CONNECTIONDATA_H

struct ConnectionData {
    QString ip;
    int port;
};

QDataStream& operator<<(QDataStream &stream, const ConnectionData &cd) {
    stream << cd.ip;
    stream << cd.port;
    return stream;
}

QDataStream& operator>>(QDataStream &stream, ConnectionData &cd) {
    stream >> cd.ip;
    stream >> cd.port;
    return stream;
}

#endif // CONNECTIONDATA_H
