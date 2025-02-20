/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/25 23:50:10 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:24:16 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "ConfigParser.hpp"         // Local header
#include "ServerConfig.hpp"         // Local header
#include "LocationConfig.hpp"       // Local header
#include "../../Utils/incs/StringUtils.hpp"    // From Utils


// This is the implementation of the ConfigParser class
// The ConfigParser class is responsible for parsing the configuration file
// and storing the server configurations in memory.


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


// Parse the configuration file and store the server configurations
void ConfigParser::parse(const std::string& filename)
{
    
    // Open the configuration file
    std::ifstream file(filename.c_str());

    // Check if the file was opened successfully
    if (!file.is_open())
        throw (std::runtime_error("Error: failed to open the configuration file " + filename)); // Throw an exception if the file could not be opened


    // Parse the configuration file
    std::string line; // Variable to store each line of the file
    
    // Read each line of the file
    while   (std::getline(file, line))
    {
        
        line = line.substr(0, line.find('#'));  // Remove comments
        
        line.erase(0, line.find_first_not_of(" \t\r\n"));   // Remove leading whitespace
        
        line.erase(line.find_last_not_of(" \t\r\n") + 1);   // Remove trailing whitespace
        
        
        // If the line is empty, skip it
        if (line.empty())
            continue    ;   // Skip to the next line
        
        
        // Check if the line contains the "server" keyword
        if (line.find("server") != std::string::npos)
            parseServerBlock(file); // Parse the server block
        
    }
    
}


// Parse a server block in the configuration file
void ConfigParser::parseServerBlock(std::ifstream& file)
{
    
    ServerConfig server; // Create a new server configuration object

    std::string line; // Variable to store each line of the file


    // Read each line of the file
    while   (std::getline(file, line))
    {
        
        line = line.substr(0, line.find('#'));  // Remove comments
        
        line.erase(0, line.find_first_not_of(" \t\r\n"));   // Remove leading whitespace
        
        line.erase(line.find_last_not_of(" \t\r\n") + 1);   // Remove trailing whitespace


        // If the line is empty, skip it
        if (line.empty())
            continue    ;   // Skip to the next line
        

        // Check if the line contains the "location" keyword
        if (line.find("location") != std::string::npos) // If the line contains the "location" keyword
        {
            parseLocationBlock(file, server);   // Parse the location block
        }
        else if (line == "}")   // If the line contains the closing bracket
        {
            break   ;   // Exit the loop
        }
        else
        {
            parseDirective(line, server);  // Parse the directive
        }
        
    }

    servers.push_back(server);  // Add the server configuration to the list of servers
    
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


// Parse a directive for a server block
void ConfigParser::parseDirective(const std::string& line, ServerConfig& server)
{
    
    // Create a string stream to parse the line
    std::istringstream iss(line);
    
    // Variable to store the key
    std::string key;
    
    // Read the key from the line
    iss >> key;
    
    // Check the key and parse the value
    if (key == "listen")    // If the key is "listen"
    {
        
        int port;   // Variable to store the port number
        iss >> port;    // Read the port number
        server.setPort(port);   // Set the port number in the server configuration

    }
    else if (key == "server_name")
    {
        
        std::string name;   // Variable to store the server name
        iss >> name;    // Read the server name
        server.setServerName(name); // Set the server name in the server configuration
        
    }
    else if (key == "client_max_body_size")   // If the key is "client_max_body_size"
    {

        size_t size;   // Variable to store the size
        iss >> size;   // Read the size
        server.setClientMaxBodySize(size); // Set the size in the server configuration
        
    }
    else if (key == "error_page")  // If the key is "error_page"
    {

        int code;   // Variable to store the error code
        std::string path;  // Variable to store the error path
        iss >> code >> path;    // Read the error code and path
        server.addErrorPage(code, path);    // Add the error page to the server configuration

    }
    
}


// Parse a directive for a location block
void ConfigParser::parseDirective(const std::string& line, LocationConfig& location)
{

    // Create a string stream to parse the line
    std::istringstream iss(line);

    // Variable to store the key
    std::string key;

    // Read the key from the line
    iss >> key;


    // Check the key and parse the value
    if (key == "root")  // If the key is "root"
    {
        
        std::string root;   // Variable to store the root directory
        iss >> root;    // Read the root directory
        location.setRoot(root); // Set the root directory in the location configuration
        
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
        location.setCgiExtension(extension);   // Set the CGI extension in the location configuration
        
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
