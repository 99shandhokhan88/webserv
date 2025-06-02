/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 12:46:31 by vzashev           #+#    #+#             */
/*   Updated: 2025/06/02 22:49:34 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../incs/webserv.hpp"

#include "LocationConfig.hpp"


void LocationConfig::addCgiExtension(const std::string& ext) {
    _cgi_extensions.insert(ext);
}

const std::set<std::string>& LocationConfig::getCgiExtensions() const {
    return _cgi_extensions;
}

// Constructors and Destructor
LocationConfig::~LocationConfig() { }

// Setters
void LocationConfig::setPath(const std::string& p) {
    // Add leading slash if missing
    if (p.empty() || p[0] != '/') {
        path = "/" + p;
    } else {
        path = p;
    }
    
    // Remove trailing slash unless it's root
    if (path.size() > 1 && path[path.size()-1] == '/') {
        path.erase(path.size()-1);
    }
}





bool LocationConfig::getAllowDelete() const {
    return _allow_delete;
}

void LocationConfig::setUploadDir(const std::string& dir) {
    _upload_dir = dir;
}

const std::string& LocationConfig::getUploadDir() const {
    return _upload_dir;
}

void LocationConfig::setAllowDelete(bool value) {
    _allow_delete = value;
}

void LocationConfig::clearAllowedMethods() {
    allowed_methods.clear();
}

bool LocationConfig::getAutoIndex() const {
    return _auto_index;
}

bool LocationConfig::isMimeTypeAllowed(const std::string& mime_type) const {
    return _allowed_mime_types.empty() ||  // Allow all if none specified
           _allowed_mime_types.count(mime_type) > 0;
}

void LocationConfig::setAutoIndex(bool value) {
    _auto_index = value;
}

void LocationConfig::addAllowedMimeType(const std::string& mime_type) {
    _allowed_mime_types.insert(mime_type);
}

void LocationConfig::setRoot(const std::string& r) { 
    root = r; 
}

void LocationConfig::setCgiExtension(const std::string& ext) { 
    cgi_extension = ext; 
}



void LocationConfig::addAllowedMethod(const std::string& method) { 
    allowed_methods.push_back(method); 
}

void LocationConfig::setIndex(const std::string& idx) { 
    index = idx; 
}

// Getters
const std::string& LocationConfig::getPath() const { 
    return path; 
}

const std::string& LocationConfig::getRoot() const { 
    return root; 
}

const std::string& LocationConfig::getCgiExtension() const { 
    return cgi_extension; 
}



const std::vector<std::string>& LocationConfig::getAllowedMethods() const { 
    return allowed_methods; 
}

const std::string& LocationConfig::getIndex() const { 
    return index; 
}

void LocationConfig::addCgiInterpreter(const std::string& ext, const std::string& interpreter) {
    std::cout << "DEBUG: LocationConfig::addCgiInterpreter - Adding: '" << ext << "' -> '" << interpreter << "'" << std::endl;
    _cgiInterpreters[ext] = interpreter;
    
    // Verifichiamo che sia stato inserito correttamente
    std::map<std::string, std::string>::const_iterator it = _cgiInterpreters.find(ext);
    if (it != _cgiInterpreters.end()) {
        std::cout << "DEBUG: Successfully added CGI interpreter: '" << it->first << "' -> '" << it->second << "'" << std::endl;
    } else {
        std::cout << "DEBUG: ERROR - Failed to add CGI interpreter for '" << ext << "'" << std::endl;
    }
}

std::string LocationConfig::getCgiInterpreter(const std::string& ext) const {
    std::cout << "DEBUG: LocationConfig::getCgiInterpreter - Looking for: '" << ext << "'" << std::endl;
    std::map<std::string, std::string>::const_iterator it = _cgiInterpreters.find(ext);
    if (it != _cgiInterpreters.end()) {
        std::cout << "DEBUG: Found interpreter: '" << it->second << "'" << std::endl;
        return it->second;
    }
    std::cout << "DEBUG: No interpreter found for extension: '" << ext << "'" << std::endl;
    return std::string();
}

const std::map<std::string, std::string>& LocationConfig::getCgiInterpreters() const {
    std::cout << "DEBUG: LocationConfig::getCgiInterpreters - Map size: " << _cgiInterpreters.size() << std::endl;
    return _cgiInterpreters;
}
