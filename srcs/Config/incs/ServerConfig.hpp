/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/12 17:40:03 by vzashev           #+#    #+#             */
/*   Updated: 2025/06/02 22:49:18 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "../../../incs/webserv.hpp"

#include "LocationConfig.hpp"

class ServerConfig {
private:
    int port;
    std::string server_name;
    size_t client_max_body_size;
    std::map<int, std::string> error_pages;
    std::string root;
    std::string index;
    std::set<std::string> _cgi_extensions;
    std::vector<LocationConfig> _locations;


    std::string _upload_dir;  // Add this member

public:
    ServerConfig();
    ServerConfig(const std::string& configFilePath); // ADD THIS
    void loadConfig(const std::string& configFilePath);
    std::string getFullPath(const std::string& uri) const;  // Add this line

    // Setters
    void setPort(int port);
    void setServerName(const std::string& name);
    void setClientMaxBodySize(size_t size);
    void addErrorPage(int code, const std::string& path);
    void addLocation(const LocationConfig& location);
    void setRoot(const std::string& root);
    void setIndex(const std::string& index);

    // Getters
    const std::string& getUploadDir() const;
    void setUploadDir(const std::string& dir);

    const std::set<std::string>& getCgiExtensions() const;
    const LocationConfig& getLocationForPath(const std::string& path) const;
    

    
    int getPort() const;
    const std::string& getServerName() const;
    size_t getClientMaxBodySize() const;
    const std::map<int, std::string>& getErrorPages() const;
    const std::vector<LocationConfig>& getLocations() const;
    const std::string& getRoot() const;
    const std::string& getIndex() const;


    void parseLocationBlock(std::ifstream& configFile, const std::string& path);


    const LocationConfig* matchLocation(const std::string& path) const {
        for (std::vector<LocationConfig>::const_iterator it = _locations.begin(); 
             it != _locations.end(); ++it) {
            if (path.find(it->getPath()) == 0) {
                return &(*it);
            }
        }
        return NULL;
    }

};

#endif