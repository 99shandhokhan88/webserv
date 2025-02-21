#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP

#include <string>
#include <map>

class MimeTypes {
private:
    static std::map<std::string, std::string> _types;  // Nome corretto
    static void initTypes();  // Nome corretto

public:
    static std::string getType(const std::string& filename);  // Nome corretto
};

#endif