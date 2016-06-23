#ifndef PARSESINGLE_H
#define PARSESINGLE_H

#include <string>
#include <utility>

class ParseSingle
{
    size_t StartLen;
    size_t EndLen;
    size_t EqualLen;
    std::string message;
public:
    ParseSingle(const std::string& msg);
    bool HasStart();
    bool HasEnd();
    bool IsComplete();
    std::pair<std::string, std::string> GetKeyValuePair();
};

#endif // PARSESINGLE_H
