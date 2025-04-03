/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 11:35:01 by vzashev           #+#    #+#             */
/*   Updated: 2025/04/02 23:29:22 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
    std::set<std::string> _allowed_mime_types;

public:
    // Constructors and Destructor declarations
    LocationConfig() : _cgiEnabled(false) {}
    ~LocationConfig();

    // Setters declarations
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
