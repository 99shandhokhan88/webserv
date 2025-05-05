#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <set>

class LocationConfig {
private:
    std::string path;
    std::string root;
    std::string cgi_extension;
    std::vector<std::string> allowed_methods;
    std::string index;
    std::set<std::string> _cgi_extensions;
    std::string _cgiPath;
    bool _cgiEnabled;
    bool _auto_index;
    bool _allow_upload;
    bool _allow_delete;
    std::set<std::string> _allowed_mime_types;

public:
    // Constructor with correct initialization order
    LocationConfig() : 
        path(""),
        root(""),
        cgi_extension(""),
        allowed_methods(),
        index("index.html"),
        _cgi_extensions(),
        _cgiPath(""),
        _cgiEnabled(false),
        _auto_index(false),
        _allow_upload(false),
        _allow_delete(false),
        _allowed_mime_types()
    {}
    
    ~LocationConfig();
    
    // Allow methods
    bool getAllowUpload() const;
    bool getAllowDelete() const;
    void setAllowUpload(bool value);
    void setAllowDelete(bool value);

    // Existing methods
    void setPath(const std::string& p);
    void setRoot(const std::string& r);
    void setCgiExtension(const std::string& ext);
    void addAllowedMethod(const std::string& method);
    void setIndex(const std::string& idx);

    void addCgiExtension(const std::string& ext);
    const std::set<std::string>& getCgiExtensions() const;
    void setCgiEnabled(bool enabled) { _cgiEnabled = enabled; }
    void setCgiPath(const std::string& path) { _cgiPath = path; }

    // Getters declarations
    const std::string& getPath() const;
    const std::string& getRoot() const;
    const std::string& getCgiExtension() const;
    const std::vector<std::string>& getAllowedMethods() const;
    const std::string& getIndex() const;
    bool isCgiEnabled() const { return _cgiEnabled; }
    const std::string& getCgiPath() const { return _cgiPath; }
    bool getAutoIndex() const;
    bool isMimeTypeAllowed(const std::string& mime_type) const;
    void setAutoIndex(bool value);
    void addAllowedMimeType(const std::string& mime_type);
};

#endif