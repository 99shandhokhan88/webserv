/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 23:54:19 by vzashev           #+#    #+#             */
/*   Updated: 2025/04/03 20:04:23 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "../../Utils/incs/FileHandler.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

const std::set<std::string>& ServerConfig::getCgiExtensions() const {
    return _cgi_extensions;
}

// Replace range-based for loops with iterator-based loops
const LocationConfig& ServerConfig::getLocationForPath(const std::string& path) const {
    
    
    
    
    
    
    std::string clean_path = path;
    
    // Normalize path
    if (clean_path.empty()) clean_path = "/";
    if (clean_path[0] != '/') clean_path = "/" + clean_path;

    const LocationConfig* best_match = NULL;
    size_t best_length = 0;

    // First pass: exact matches
    for (std::vector<LocationConfig>::const_iterator it = _locations.begin(); 
         it != _locations.end(); ++it) {
        if (it->getPath() == clean_path) {
            return *it;
        }
    }

    // Second pass: prefix matches
    for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
         it != _locations.end(); ++it) {
        const std::string& loc_path = it->getPath();
        if (clean_path.find(loc_path) == 0 && loc_path.length() > best_length) {
            best_length = loc_path.length();
            best_match = &(*it);
        }
    }

    // Fallback to default location
    if (!best_match) {
        for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
             it != _locations.end(); ++it) {
            if (it->getPath() == "/") {
                return *it;
            }
        }
        throw std::runtime_error("No matching location found for path: " + clean_path);
    }

    return *best_match;
}

// Modify string back() check for C++98 compatibility
const std::string ServerConfig::getFullPath(const std::string& uri) const {
    std::string full_path = root;
    
    // Ensure root ends with a slash
    if (!full_path.empty() && full_path[full_path.size()-1] != '/') {
        full_path += '/';
    }
    
    // Handle URI starting with slash
    if (!uri.empty() && uri[0] == '/') {
        full_path += uri.substr(1);
    } else {
        full_path += uri;
    }
    
    // Remove trailing slash for files using C++98 methods
    if (!full_path.empty() && full_path[full_path.size()-1] == '/' && 
        !FileHandler::isDirectory(full_path)) {
        full_path = full_path.substr(0, full_path.size()-1);
    }
    
    return full_path;
}


ServerConfig::ServerConfig() :
    port(8080),
    client_max_body_size(1048576), // 1MB default
    root(""),
    index("index.html") {}

ServerConfig::ServerConfig(const std::string& configFilePath) : port(8080), root(""), index("") {
    loadConfig(configFilePath);
}


// ServerConfig.cpp
const std::string& ServerConfig::getUploadDir() const {
    return upload_dir;  // Return actual member variable
}

void ServerConfig::setUploadDir(const std::string& dir) {
    upload_dir = dir;
}

void ServerConfig::loadConfig(const std::string& configFilePath)
{
    // Add debug output
    std::cerr << "Attempting to open config file: " << configFilePath << std::endl;

    std::ifstream configFile(configFilePath.c_str());
if (!configFile.is_open())
{
    std::cerr << "Failed to open file: " << configFilePath
              << ", errno: " << errno
              << " (" << strerror(errno) << ")" << std::endl;
    throw std::runtime_error("Failed to open config file: " + configFilePath);
}

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "listen") {
            iss >> port; // Estrai la porta
        } else if (key == "root") {
            iss >> root; // Estrai la root directory
        } else if (key == "index") {
            iss >> index; // Estrai il file index
        } else if (key == "location") {
            std::string path;
            iss >> path;
            parseLocationBlock(configFile, path); // Gestisci il blocco location
        }
    }

    std::cout << "Config file loaded: " << configFilePath << std::endl;
}

void ServerConfig::parseLocationBlock(std::ifstream& configFile, const std::string& path) {
    LocationConfig location;
    location.setPath(path); // Use setter

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "root") {
            std::string root;
            iss >> root;
            location.setRoot(root);
        } else if (key == "cgi_extension") {
            std::string cgi_ext;
            iss >> cgi_ext;
            location.setCgiExtension(cgi_ext);
        } else if (key == "cgi_path") {
            std::string cgi_p;
            iss >> cgi_p;
            location.setCgiPath(cgi_p);
        } else if (key == "allow_methods") {
            std::string method;
            while (iss >> method) {
                location.addAllowedMethod(method);
            }
        } else if (key == "}") {
            break; // End of location block
        }
    }

    _locations.push_back(location);
}


int ServerConfig::getPort() const {
    return port;
}



// Setters
void ServerConfig::setPort(int port) {
    this->port = port;
}

void ServerConfig::setServerName(const std::string& name) {
    this->server_name = name;
}

void ServerConfig::setClientMaxBodySize(size_t size) {
    this->client_max_body_size = size;
}

void ServerConfig::addErrorPage(int code, const std::string& path) {
    error_pages[code] = path;
}

void ServerConfig::addLocation(const LocationConfig& location) {
    _locations.push_back(location);
}

void ServerConfig::setRoot(const std::string& root) {
    if (!root.empty() && root[root.size()-1] != '/') {
        this->root = root + "/";
    } else {
        this->root = root;
    }
}



void ServerConfig::setIndex(const std::string& index) {
    this->index = index;
}

// Getters
const std::string& ServerConfig::getServerName() const {
    return server_name;
}

size_t ServerConfig::getClientMaxBodySize() const {
    return client_max_body_size;
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const {
    return error_pages;
}


// Implementazione di getRoot()
const std::string& ServerConfig::getRoot() const {
    return this->root;
}

// Implementazione di getIndex()
const std::string& ServerConfig::getIndex() const {
    return this->index;
}