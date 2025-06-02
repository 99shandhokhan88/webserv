/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/25 23:50:10 by vzashev           #+#    #+#             */
/*   Updated: 2025/06/02 22:49:29 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../../../incs/webserv.hpp"

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
            std::cout << "DEBUG: Found location directive: '" << line << "'" << std::endl;
            // Extract location path from line
            std::istringstream iss(line);
            std::string keyword, path;
            iss >> keyword >> path;  // Get "location" and path
            std::cout << "DEBUG: Parsed keyword='" << keyword << "', path='" << path << "'" << std::endl;
            
            // Remove any trailing '{' from path
            size_t brace_pos = path.find('{');
            if (brace_pos != std::string::npos) {
                path = path.substr(0, brace_pos);
            }
            
            std::cout << "DEBUG: Parsing location block for path: '" << path << "'" << std::endl;
            parseLocationBlock(file, server, path);  // Pass path to parser
            
            // Dopo aver aggiunto la location, verifichiamo se è /cgi-bin/
            if (path == "/cgi-bin/" || path == "/cgi-bin") {
                const std::vector<LocationConfig>& locations = server.getLocations();
                for (std::vector<LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
                    if (it->getPath() == "/cgi-bin") {
                        std::cout << "DEBUG: Checking /cgi-bin location after parsing" << std::endl;
                        const std::map<std::string, std::string>& cgiMap = it->getCgiInterpreters();
                        std::cout << "DEBUG: CGI interpreters in location: " << cgiMap.size() << std::endl;
                        for (std::map<std::string, std::string>::const_iterator cgi_it = cgiMap.begin(); cgi_it != cgiMap.end(); ++cgi_it) {
                            std::cout << "DEBUG: Found interpreter: '" << cgi_it->first << "' -> '" << cgi_it->second << "'" << std::endl;
                        }
                        break;
                    }
                }
            }
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
    std::cout << "DEBUG: Started parsing location block for path '" << path << "'" << std::endl;

    // Aggiungiamo manualmente gli interpreti CGI per il path /cgi-bin/
    if (path == "/cgi-bin/" || path == "/cgi-bin") {
        std::cout << "DEBUG: Manually adding CGI interpreters for /cgi-bin/ location" << std::endl;
        location.addCgiInterpreter(".py", "/usr/bin/python3");
        location.addCgiInterpreter(".sh", "/bin/bash");
        location.addCgiInterpreter(".cgi", "/usr/bin/env");
        location.addCgiInterpreter(".php", "/usr/bin/php");
        
        // Verifichiamo che siano stati aggiunti
        const std::map<std::string, std::string>& cgiMap = location.getCgiInterpreters();
        std::cout << "DEBUG: Added " << cgiMap.size() << " CGI interpreters manually" << std::endl;
        for (std::map<std::string, std::string>::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
            std::cout << "DEBUG: Manual CGI interpreter: '" << it->first << "' -> '" << it->second << "'" << std::endl;
        }
    }

    std::string line;
    while (std::getline(file, line)) {
        line = line.substr(0, line.find('#'));
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        if (line == "}") break;
        
        // Debug per vedere ogni linea analizzata
        std::cout << "DEBUG: Parsing directive in location '" << path << "': '" << line << "'" << std::endl;
        
        // Usiamo il normale parseDirective per tutte le direttive
        parseDirective(line, location);
    }
    
    // Verifichiamo gli interpreti configurati
    std::cout << "DEBUG: Finished parsing location block for path '" << path << "'" << std::endl;
    const std::map<std::string, std::string>& cgiMap = location.getCgiInterpreters();
    std::cout << "DEBUG: Number of CGI interpreters configured: " << cgiMap.size() << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
        std::cout << "DEBUG: CGI interpreter in location: '" << it->first << "' -> '" << it->second << "'" << std::endl;
    }
    
    server.addLocation(location);
    std::cout << "DEBUG: Added location to server config" << std::endl;
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
        std::string size_str;
        iss >> size_str;
        
        // Remove semicolon if present
        if (!size_str.empty() && size_str[size_str.size()-1] == ';') {
            size_str.erase(size_str.size()-1, 1);
        }
        
        size_t size = static_cast<size_t>(atoi(size_str.c_str()));
        server.setClientMaxBodySize(size);
        std::cout << "DEBUG: Set client_max_body_size to " << size << " bytes" << std::endl;
    }
    else if (key == "error_page") {
        int code;
        std::string path;
        iss >> code >> path;
        std::cerr << "DEBUG: Parsing error_page directive: code=" << code << ", path=" << path << std::endl;
        server.addErrorPage(code, path);
        std::cerr << "DEBUG: Added error page to server config" << std::endl;
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
        bool autoindex_enabled = (value == "on");
        location.setAutoIndex(autoindex_enabled);
        std::cout << "DEBUG: Set autoindex to " << (autoindex_enabled ? "true" : "false") << " for location" << std::endl;
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
        std::string extension, interpreter;
        iss >> extension >> interpreter;
        std::cout << "DEBUG: Raw cgi_extension directive: extension='" << extension << "', interpreter='" << interpreter << "'" << std::endl;
        
        if (!extension.empty() && !interpreter.empty()) {
            // Rimuove eventuali punti e virgola alla fine
            if (!interpreter.empty() && interpreter[interpreter.size()-1] == ';') {
                std::cout << "DEBUG: Removing semicolon from interpreter" << std::endl;
                interpreter.erase(interpreter.size()-1, 1);
            }
            
            // Stampiamo dettagli aggiuntivi per il debug
            std::cout << "DEBUG: Adding CGI interpreter: '" << extension << "' -> '" << interpreter << "'" << std::endl;
            location.addCgiInterpreter(extension, interpreter);
            
            // Verifichiamo immediatamente che sia stato aggiunto
            std::string interpreter_check = location.getCgiInterpreter(extension);
            if (interpreter_check == interpreter) {
                std::cout << "DEBUG: Verification successful - interpreter was added correctly" << std::endl;
            } else {
                std::cout << "DEBUG: ERROR - Interpreter was not added correctly. Got: '" << interpreter_check << "'" << std::endl;
            }
            
            // Mostriamo il contenuto aggiornato della mappa
            const std::map<std::string, std::string>& cgiMap = location.getCgiInterpreters();
            std::cout << "DEBUG: CGI Interpreters map now contains " << cgiMap.size() << " entries" << std::endl;
            for (std::map<std::string, std::string>::const_iterator it = cgiMap.begin(); it != cgiMap.end(); ++it) {
                std::cout << "DEBUG: Map entry: '" << it->first << "' -> '" << it->second << "'" << std::endl;
            }
        } else {
            std::cout << "DEBUG: Invalid CGI extension configuration. Extension: '" << extension << "', Interpreter: '" << interpreter << "'" << std::endl;
        }
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
