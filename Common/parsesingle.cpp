#include "parsesingle.h"
#include "parsemessages.h"


ParseSingle::ParseSingle(const std::string& msg):message(msg)
{
    StartLen = ParseMessages::START.length();
    EndLen = ParseMessages::END.length();
    EqualLen = ParseMessages::EQUAL.length();
}

bool ParseSingle::HasStart()
{
    return (!message.empty() && message.find(ParseMessages::START) != std::string::npos);
}

bool ParseSingle::HasEnd()
{
    return (!message.empty() && message.find(ParseMessages::END) != std::string::npos);
}

bool ParseSingle::IsComplete()
{
    return (HasStart() && HasEnd());
}

std::pair<std::string, std::string> ParseSingle::GetKeyValuePair()
{
    std::pair<std::string, std::string> result;
    if (IsComplete()) {
        size_t index1 = message.find(ParseMessages::START);
        if (index1 != std::string::npos)
        {
            size_t index2 = message.find(ParseMessages::END, index1+StartLen);
            if (index2 != std::string::npos)
            {
                std::string segment = message.substr(index1 + StartLen, index2-index1-StartLen);
                std::size_t index = segment.find(ParseMessages::EQUAL);
                if (index != std::string::npos)
                {
                    result.first = segment.substr(0, index);
                    result.second = segment.substr(index+EqualLen, segment.length() - index - EqualLen);
                }
            }
        }
    }

    return result;
}
