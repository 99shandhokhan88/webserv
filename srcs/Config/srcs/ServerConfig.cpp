/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 23:54:19 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/20 18:13:45 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

ServerConfig::ServerConfig() :
    port(8080),
    client_max_body_size(1048576), // 1MB default
    root(""),
    index("index.html") {}

ServerConfig::ServerConfig(const std::string& configFilePath) : port(8080), root(""), index("") {
    loadConfig(configFilePath);
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

    locations.push_back(location);
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
    locations.push_back(location);
}

void ServerConfig::setRoot(const std::string& root) {
    this->root = root;
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