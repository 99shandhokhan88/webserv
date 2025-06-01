#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "StringUtils.hpp"
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include "../../Utils/incs/FileHandler.hpp"

class Request {
public:
    Request();
    ~Request();

    bool isValid() const {
        return !_method.empty() && !_path.empty() && !raw_data.empty();
    }

    void parse(const char* data, size_t length);
    std::string dechunk(const std::string& body);

    // Setters
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);

    // Getters

    const std::string& getHeader(const std::string& name) const {
        std::map<std::string, std::string>::const_iterator it = _headers.find(name);
        if (it != _headers.end()) return it->second;
        static const std::string empty;
        return empty;
    }
    
    bool hasHeader(const std::string& name) const {
        return _headers.find(name) != _headers.end();
    }

    const std::string& getMethod() const { return _method; }
    const std::string& getPath() const { return _path; }
    const std::string& getVersion() const { return _version; }

    const std::string& getQuery() const { return _query; }
    const std::string& getUri() const { return _uri; }
    const std::map<std::string, std::string>& getHeaders() const { return _headers; }
    const std::string& getBody() const { return _body; }
    size_t getBodySize() const { return _body.size(); }
    const std::string& getQueryString() const { return _query; }

    const std::string& getContentType() const {
        static std::string empty;
        std::map<std::string, std::string>::const_iterator it = _headers.find("Content-Type");
        return (it != _headers.end()) ? it->second : empty;
    }



private:

std::map<std::string, std::string> _headers;

std::string _method;
std::string _path;
std::string _version;
    std::string _uri;
    std::string _query;
    std::string _body;
    std::string raw_data;  // Add this member

    void parseRequestLine(const std::string& line);
    void parseHeaderLine(const std::string& line);
};

#endif // REQUEST_HPP