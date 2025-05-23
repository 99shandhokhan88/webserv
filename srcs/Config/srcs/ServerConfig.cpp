/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 23:54:19 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/15 22:33:40 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "../../Utils/incs/FileHandler.hpp"
#include "../../Config/incs/LocationConfig.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

const std::set<std::string>& ServerConfig::getCgiExtensions() const {
    return _cgi_extensions;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const {
    return _locations;  // Assuming _locations is your member variable storing locations
}

const LocationConfig& ServerConfig::getLocationForPath(const std::string& path) const {
    std::string clean_path = path;
        std::cout << "DEBUG: Available locations:\n";

    for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
         it != _locations.end(); ++it) {
        std::cout << " - '" << it->getPath() << "'\n";
    }
    // Normalize path
    if (clean_path.empty() || clean_path[0] != '/') {
        clean_path = "/" + clean_path;
    }

    const LocationConfig* best_match = NULL;
    size_t best_length = 0;

    // First pass: exact matches
    for (std::vector<LocationConfig>::const_iterator it = _locations.begin(); 
         it != _locations.end(); ++it) {
        if (it->getPath() == clean_path) {
            return *it;
        }
    }

    // Second pass: longest prefix match
    for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
         it != _locations.end(); ++it) {
        const std::string& loc_path = it->getPath();
        if (clean_path.compare(0, loc_path.length(), loc_path) == 0) {
            if (loc_path.length() > best_length) {
                best_length = loc_path.length();
                best_match = &(*it);
            }
        }
    }

    // Return best match if found
    if (best_match) {
        return *best_match;
    }

    // Explicit fallback to root location
    for (std::vector<LocationConfig>::const_iterator it = _locations.begin();
         it != _locations.end(); ++it) {
        if (it->getPath() == "/") {
            return *it;
        }
    }

    throw std::runtime_error("No matching location found for path: " + clean_path);
}

std::string ServerConfig::getFullPath(const std::string& uri) const {
    std::string path = this->root;  // Access class member
    
    // Remove trailing slash from root
    if (!path.empty() && path[path.length()-1] == '/') {
        path.erase(path.length()-1);
    }
    
    // Add sanitized URI path
    std::string uri_path = uri;
    if (!uri_path.empty() && uri_path[0] == '/') {
        uri_path.erase(0, 1);
    }
    
    return FileHandler::sanitizePath(path + "/" + uri_path);
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
    return _upload_dir;  // Return actual member variable
}

void ServerConfig::setUploadDir(const std::string& dir) {
    _upload_dir = dir;
}

void ServerConfig::loadConfig(const std::string& configFilePath)
{
    // Validation: check file extension
    if (configFilePath.length() < 5 || configFilePath.substr(configFilePath.length() - 5) != ".conf") {
        throw std::runtime_error("Configuration file must have .conf extension: " + configFilePath);
    }
    
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
    bool inServerBlock = false;
    bool hasServerBlock = false;
    bool hasValidConfig = false;
    int lineCount = 0;
    int nonEmptyLines = 0;
    
    while (std::getline(configFile, line)) {
        lineCount++;
        // Remove comments and trim whitespace
        line = line.substr(0, line.find('#'));
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty()) continue;
        
        nonEmptyLines++;
        
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "server" && line.find("{") != std::string::npos) {
            inServerBlock = true;
            hasServerBlock = true;
            std::cerr << "DEBUG: Entering server block" << std::endl;
            continue;
        } else if (line == "}") {
            inServerBlock = false;
            std::cerr << "DEBUG: Exiting server block" << std::endl;
            continue;
        }
        
        if (inServerBlock) {
            if (key == "listen") {
                iss >> port; // Estrai la porta
                if (port > 0) hasValidConfig = true;
                std::cerr << "DEBUG: Set port to " << port << std::endl;
            } else if (key == "root") {
                iss >> root; // Estrai la root directory
                // Remove trailing semicolon
                if (!root.empty() && root[root.length()-1] == ';') {
                    root.erase(root.length()-1);
                }
                std::cerr << "DEBUG: Set root to " << root << std::endl;
            } else if (key == "index") {
                iss >> index; // Estrai il file index
                // Remove trailing semicolon
                if (!index.empty() && index[index.length()-1] == ';') {
                    index.erase(index.length()-1);
                }
                std::cerr << "DEBUG: Set index to " << index << std::endl;
            } else if (key == "error_page") {
                int code;
                std::string path;
                iss >> code >> path;
                // Remove trailing semicolon
                if (!path.empty() && path[path.length()-1] == ';') {
                    path.erase(path.length()-1);
                }
                std::cerr << "DEBUG: Parsing error_page directive: code=" << code << ", path=" << path << std::endl;
                error_pages[code] = path;
                std::cerr << "DEBUG: Added error page to server config" << std::endl;
            } else if (key == "location") {
                std::string path;
                iss >> path;
                parseLocationBlock(configFile, path); // Gestisci il blocco location
            }
        }
    }

    // Validation: check if config file is empty or invalid
    if (lineCount == 0) {
        throw std::runtime_error("Configuration file is completely empty: " + configFilePath);
    }
    
    if (nonEmptyLines == 0) {
        throw std::runtime_error("Configuration file contains no valid directives (only empty lines/comments): " + configFilePath);
    }
    
    if (!hasServerBlock) {
        throw std::runtime_error("Configuration file must contain at least one 'server' block: " + configFilePath);
    }
    
    if (!hasValidConfig) {
        throw std::runtime_error("Server block must contain at least 'listen' directive with valid port: " + configFilePath);
    }
    
    if (port <= 0 || port > 65535) {
        throw std::runtime_error("Invalid port number in configuration. Port must be between 1 and 65535: " + configFilePath);
    }
    
    if (root.empty()) {
        throw std::runtime_error("Server block must contain 'root' directive: " + configFilePath);
    }

    std::cout << "Config file loaded: " << configFilePath << std::endl;
}

void ServerConfig::parseLocationBlock(std::ifstream& configFile, const std::string& path) {
    LocationConfig location;
    location.setPath(path);

    std::string line;
    while (std::getline(configFile, line)) {
        // Remove leading and trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "allow_upload") {
            std::string value;
            iss >> value;
            // Remove any trailing semicolon
            if (!value.empty() && value[value.length()-1] == ';') {
                value.erase(value.length()-1);
            }
            location.setAllowUpload(value == "on");
            std::cout << "DEBUG: Set allow_upload to " << value << std::endl;
        } 
        else if (key == "upload_dir") {
            std::string dir;
            iss >> dir;
            if (!dir.empty() && dir[dir.length()-1] == ';') {
                dir.erase(dir.length()-1);
            }            
            location.setUploadDir(dir);    
        }
        else if (key == "root") {
            std::string root;
            iss >> root;
            if (!root.empty() && root[root.length()-1] == ';') {
                root.erase(root.length()-1);
            }
            location.setRoot(root);
        }
        else if (key == "cgi_extension") {
            std::string cgi_ext;
            iss >> cgi_ext;
            if (!cgi_ext.empty() && cgi_ext[cgi_ext.length()-1] == ';') {
                cgi_ext.erase(cgi_ext.length()-1);
            }
            location.setCgiExtension(cgi_ext);
        }
        else if (key == "cgi_path") {
            std::string cgi_p;
            iss >> cgi_p;
            if (!cgi_p.empty() && cgi_p[cgi_p.length()-1] == ';') {
                cgi_p.erase(cgi_p.length()-1);
            }
            location.setCgiPath(cgi_p);
        }
        else if (key == "allow_delete") {
            std::string value;
            iss >> value;
            // Remove any trailing semicolon
            if (!value.empty() && value[value.length()-1] == ';') {
                value.erase(value.length()-1);
            }
            std::cout << "DEBUG: Processing allow_delete with value: '" << value << "'" << std::endl;
            location.setAllowDelete(value == "on");
            std::cout << "DEBUG: Set allow_delete to " << (value == "on") << std::endl;
        }
        else if (key == "allow_methods") {
            std::string method;
            while (iss >> method) {
                if (!method.empty() && method[method.length()-1] == ';') {
                    method.erase(method.length()-1);
                }
                if (!method.empty()) {
                    location.addAllowedMethod(method);
                }
            }
        }
        else if (key == "}") {
            break;
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
    // Store root as absolute path
    this->root = getAbsolutePath(root);
    
    // Ensure trailing slash
    if (!this->root.empty() && this->root[this->root.size()-1] != '/') {
        this->root += "/";
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