#ifndef PARSEMESSAGES_H
#define PARSEMESSAGES_H

#include <string>
#include <utility>
#include <mutex>
#include <QString>


//Adds all new string segments to a running string
//if there are '<' and '>' characters in that order,
//GetKeyValuePair delivers the kvp between the first set of < >
//and removes that segment from the running string
class ParseMessages
{
public:
    static const std::string START;
    static const std::string END;
    static const std::string EQUAL;

    static const QString QSTART;
    static const QString QEND;
    static const QString QEQUAL;


private:
    size_t StartLen;
    size_t EndLen;
    size_t EqualLen;
    std::mutex mtx;
    std::string message;
public:
    ParseMessages();

    void AddString(const std::string &str);
    bool NewMessageAvailable();
    std::pair<std::string, std::string> GetKeyValuePair();
};

#endif // PARSEMESSAGES_H
