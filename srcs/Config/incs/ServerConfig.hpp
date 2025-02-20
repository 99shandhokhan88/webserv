/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 17:40:03 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/19 23:48:22 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <string>
#include <vector>
#include <map>

class ServerConfig {
private:
    int port;
    std::string server_name;
    size_t client_max_body_size;
    std::map<int, std::string> error_pages;
    std::vector<LocationConfig> locations;
    std::string root;
    std::string index;

public:
    ServerConfig();
    ServerConfig(const std::string& configFilePath); // ADD THIS
    void loadConfig(const std::string& configFilePath);

    // Setters
    void setPort(int port);
    void setServerName(const std::string& name);
    void setClientMaxBodySize(size_t size);
    void addErrorPage(int code, const std::string& path);
    void addLocation(const LocationConfig& location);
    void setRoot(const std::string& root);
    void setIndex(const std::string& index);

    // Getters
    int getPort() const;
    const std::string& getServerName() const;
    size_t getClientMaxBodySize() const;
    const std::map<int, std::string>& getErrorPages() const;
    const std::vector<LocationConfig>& getLocations() const;
    const std::string& getRoot() const;
    const std::string& getIndex() const;

    void parseLocationBlock(std::ifstream& configFile, const std::string& path);
};

#endif