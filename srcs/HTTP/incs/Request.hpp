#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "StringUtils.hpp"
#include <string>
#include <map>
#include <sstream>
#include <algorithm>

class Request {
public:
    Request();
    ~Request();

    bool isValid() const {
        return !_method.empty() && !_path.empty() && !raw_data.empty();
    }

    void parse(const char* data, size_t length);

    // Setters
    void setMethod(const std::string& method);
    void setUri(const std::string& uri);
    void setVersion(const std::string& version);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);

    // Getters
    const std::string& getMethod() const { return _method; }
    const std::string& getQuery() const { return _query; }
    const std::string& getUri() const { return _uri; }
    const std::string& getVersion() const { return _version; }
    const std::map<std::string, std::string>& getHeaders() const { return headers; }
    const std::string& getPath() const { return _path; }
    const std::string& getBody() const { return _body; }
    size_t getBodySize() const { return _body.size(); }
    const std::string& getQueryString() const { return _query; }

private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::map<std::string, std::string> headers;
    std::string _path;
    std::string _query;
    std::string _body;
    std::string raw_data;  // Add this member

    void parseRequestLine(const std::string& line);
    void parseHeaderLine(const std::string& line);
};

#endif // REQUEST_HPP