/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 11:35:01 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/27 17:54:50 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

class LocationConfig {
private:
    std::string path;
    std::string root;
    std::string cgi_extension;
    std::vector<std::string> allowed_methods;
    std::string index;
    
    std::string _cgiPath;
    bool _cgiEnabled;

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

    
};

#endif
