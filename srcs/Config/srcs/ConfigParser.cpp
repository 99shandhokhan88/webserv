/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/25 23:50:10 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/15 22:29:50 by vzashev          ###   ########.fr       */
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
            // Extract location path from line
            std::istringstream iss(line);
            std::string keyword, path;
            iss >> keyword >> path;  // Get "location" and path
            
            // Remove any trailing '{' from path
            size_t brace_pos = path.find('{');
            if (brace_pos != std::string::npos) {
                path = path.substr(0, brace_pos);
            }
            
            parseLocationBlock(file, server, path);  // Pass path to parser
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
void ConfigParser::parseLocationBlock(std::ifstream& file, 
                                    ServerConfig& server,
                                    const std::string& path) {
    LocationConfig location;
    location.setPath(path);  // Now using the passed path parameter

    std::string line;
    while (std::getline(file, line)) {
        line = line.substr(0, line.find('#'));
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        if (line == "}") break;
        
        parseDirective(line, location);
    }
    server.addLocation(location);
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
        
        size_t semicolon_pos;
        while ((semicolon_pos = root_value.find(';')) != std::string::npos) {
            root_value.erase(semicolon_pos, 1);
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
else if (key == "allow_delete") {
    std::string value;
    iss >> value;
    location.setAllowDelete(value == "on");
    std::cout << "DEBUG: Set allow_delete to " << value << std::endl;
}
    else if (key == "allow_methods") {
        location.clearAllowedMethods();
        std::string method;
        while (iss >> method) {

size_t invalid_char = method.find_first_of(";{}");
            if (invalid_char != std::string::npos) {
                method = method.substr(0, invalid_char);
            }

 if (!method.empty()) {
                location.addAllowedMethod(method);
                std::cout << "DEBUG: Metodo aggiunto: '" 
                         << method << "' a " 
                         << location.getPath() << std::endl;
            }
           
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
