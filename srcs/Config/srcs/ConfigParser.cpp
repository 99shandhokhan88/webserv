/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/25 23:50:10 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/05 19:04:46 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "../../Utils/incs/StringUtils.hpp"
#include "../../Utils/incs/FileHandler.hpp"  // Added missing include


// Default Constructor
ConfigParser::ConfigParser()
{
    //Empty constructor
}


// Default Destructor
ConfigParser::~ConfigParser()
{
    //Empty destructor
}






void ConfigParser::parse(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Error: failed to open the configuration file " + filename);

    std::string line;
    while (std::getline(file, line)) {
        line = line.substr(0, line.find('#'));
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty())
            continue;
        
        if (line.find("server") != std::string::npos)
            parseServerBlock(file);
    }
}


void ConfigParser::parseServerBlock(std::ifstream& file)
{
    ServerConfig server;
    LocationConfig default_location;
    default_location.setPath("/");
        default_location.setRoot(server.getRoot());  // Inherit server root

    server.addLocation(default_location);
    std::string line;

    while (std::getline(file, line)) {
        line = line.substr(0, line.find('#'));
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty())
            continue;

        if (line.find("location") != std::string::npos) {
            parseLocationBlock(file, server);
        }
        else if (line == "}") {
            break;
        }
        else {
            parseDirective(line, server);
        }
    }
    servers.push_back(server);
}

// Parse a location block for a server
void ConfigParser::parseLocationBlock(std::ifstream& file, ServerConfig& server)
{
    
    LocationConfig location;    // Create a new location configuration object

    std::string line;   // Variable to store each line of the file

    // Read each line of the file
    while   (std::getline(file, line))
    {
        
        line = line.substr(0, line.find('#'));  // Remove comments
        
        line.erase(0, line.find_first_not_of(" \t\r\n"));   // Remove leading whitespace
        
        line.erase(line.find_last_not_of(" \t\r\n") + 1);   // Remove trailing whitespace
        

        if (line.empty())   // If the line is empty, skip it
            continue    ;   // Skip to the next line


        if (line == "}")   // If the line contains the closing bracket
        {
            break   ;   // Exit the loop
        }
        else
        {
            parseDirective(line, location); // Parse the directive
        }
        
    }

    server.addLocation(location);    // Add the location configuration to the server
    
}


void ConfigParser::parseDirective(const std::string& line, ServerConfig& server)
{
    std::istringstream iss(line);
    std::string key;
    iss >> key;

    if (key == "listen") {
        int port;
        iss >> port;
        server.setPort(port);
    }
    else if (key == "root") {
        std::string root_value;
        iss >> root_value;
        
        // C++98 compatible semicolon removal
        if (!root_value.empty() && root_value[root_value.size()-1] == ';') {
            root_value.erase(root_value.size()-1, 1);
        }
        
        server.setRoot(FileHandler::sanitizePath(root_value));
    }
    else if (key == "server_name") {
        std::string name;
        iss >> name;
        server.setServerName(name);
    }
    else if (key == "client_max_body_size") {
        size_t size;
        iss >> size;
        server.setClientMaxBodySize(size);
    }
    else if (key == "error_page") {
        int code;
        std::string path;
        iss >> code >> path;
        server.addErrorPage(code, path);
    }
}


// Updated location directive parsing with C++98 compatibility
void ConfigParser::parseDirective(const std::string& line, LocationConfig& location)
{
    std::istringstream iss(line);
    std::string key;
    iss >> key;

    if (key == "root") {
        std::string root_value;
        iss >> root_value;
        
        // C++98 compatible semicolon removal
        if (!root_value.empty() && root_value[root_value.size()-1] == ';') {
            root_value.erase(root_value.size()-1, 1);
        }
        
        location.setRoot(FileHandler::sanitizePath(root_value));
    }
    else if (key == "autoindex") {
        std::string value;
        iss >> value;
        location.setAutoIndex(value == "on");
    }
    else if (key == "allow_types") {
        std::string mime_type;
        while (iss >> mime_type) {
            location.addAllowedMimeType(mime_type);
        }
    }
    else if (key == "index")    // If the key is "index"
    {
        
        std::string index;  // Variable to store the index file
        iss >> index;   // Read the index file
        location.setIndex(index);   // Set the index file in the location configuration
        
    }
    else if (key == "allow_methods") // If the key is "allow_methods"
    {
        
        std::string method; // Variable to store the method
        while   (iss >> method)   // Read the method
        {
            location.addAllowedMethod(method);   // Add the method to the allowed methods in the location configuration
        }
        
    }
    else if (key == "cgi_extension")    // If the key is "cgi_extension"
    {
        
        std::string extension;  // Variable to store the CGI extension
        iss >> extension;   // Read the CGI extension
        location.setCgiExtension(extension);  
        //std::cout << extension;
         // Set the CGI extension in the location configuration
        
    }
    else if (key == "cgi_path") // If the key is "cgi_path"
    {
        
        std::string path;   // Variable to store the CGI path
        iss >> path;    // Read the CGI path
        location.setCgiPath(path);  // Set the CGI path in the location configuration
        
    }

}


// Get the list of server configurations
const std::vector<ServerConfig>& ConfigParser::getServers() const
{
    return  (servers); // Return the list of server configurations
}

// Path: srcs/Config/srcs/LocationConfig.cpp
