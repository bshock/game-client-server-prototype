#include "parsemessages.h"


const std::string ParseMessages::START = "^(";
const std::string ParseMessages::END = ")$";
const std::string ParseMessages::EQUAL = ")=(";

const QString ParseMessages::QSTART = "^(";
const QString ParseMessages::QEND = ")$";
const QString ParseMessages::QEQUAL = ")=(";


ParseMessages::ParseMessages()
{
    StartLen = ParseMessages::START.length();
    EndLen = ParseMessages::END.length();
    EqualLen = ParseMessages::EQUAL.length();
}

void ParseMessages::AddString(const std::string &str)
{
    std::lock_guard<std::mutex> lock(mtx);

    message += str;
}

bool ParseMessages::NewMessageAvailable()
{
    std::lock_guard<std::mutex> lock(mtx);

    size_t index1 = message.find(ParseMessages::START);
    if (index1 != std::string::npos)
    {
        size_t index2 = message.find(ParseMessages::END, index1+StartLen);
        if (index2 != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

std::pair<std::string, std::string> ParseMessages::GetKeyValuePair()
{
    std::lock_guard<std::mutex> lock(mtx);

    std::pair<std::string, std::string> result;

    size_t index1 = message.find(ParseMessages::START);
    if (index1 != std::string::npos)
    {
        size_t index2 = message.find(ParseMessages::END, index1+StartLen);
        if (index2 != std::string::npos)
        {
            std::string segment = message.substr(index1 + StartLen, index2-index1-StartLen);
            message = message.substr(index2 + EndLen);

            std::size_t index = segment.find(ParseMessages::EQUAL);
            if (index != std::string::npos)
            {
                result.first = segment.substr(0, index);
                result.second = segment.substr(index+EqualLen, segment.length() - index - EqualLen);
            }
        }
    }
    return result;
}
