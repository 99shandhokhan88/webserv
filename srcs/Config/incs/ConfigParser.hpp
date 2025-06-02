/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/25 22:33:19 by vzashev           #+#    #+#             */
/*   Updated: 2025/06/02 22:49:11 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "../../../incs/webserv.hpp"

#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

class ConfigParser
{
    
                    public:
                    
                            ConfigParser(); // Constructor
                            ~ConfigParser(); // Destructor
                            
                            // Parse the configuration file
                            void parse(const std::string& filename);
                            // Get the list of server configurations
                            const std::vector<ServerConfig>& getServers() const;
                    private:
                            
                            std::vector<ServerConfig> servers;

                            // Helper functions
                            void parseServerBlock(std::ifstream& file); // Parse a server block
void parseLocationBlock(std::ifstream& file, 
                          ServerConfig& server,
                          const std::string& path);                             void parseDirective(const std::string& line, ServerConfig& server); // Parse a directive
                            void parseDirective(const std::string& line, LocationConfig& location); // Parse a directive

};

#endif // CONFIGPARSER_HPP


/*

The ConfigParser class is responsible for parsing the configuration file
and storing the server configurations in a vector.
It provides a parse method to read the configuration file
and a getServers method to retrieve the list of server configurations.

The class contains private helper functions to parse
server blocks,
location blocks,
and directives.
These functions are used internally by the parse method
to process the configuration file.

The ConfigParser class is designed to work with the ServerConfig
and LocationConfig classes,
which represent server
and location configurations,
respectively.

The ConfigParser class is a key component of the web server,
as it is responsible for reading and interpreting the server configuration file.
By parsing the configuration file,
the ConfigParser class enables the web server to configure itself
based on the settings specified in the file.

The ConfigParser class uses a vector<ServerConfig> servers; member variable.
A vector in C++ is a dynamic array that can grow or shrink in size.
In this case,
the vector is used to store the server configurations read from the configuration file.
The vector<ServerConfig> servers; line declares a vector of ServerConfig objects
to hold the server configurations.
It is a private member of the ConfigParser class,
meaning that it can only be accessed within the class.
To explain the vector<ServerConfig> servers; line in more detail:
vector is a template class that represents a dynamic array in C++.
The <ServerConfig> part specifies that the vector will hold objects of type ServerConfig.
servers is the name of the vector,
which can be used to access and manipulate the elements stored in the vector.
The semicolon ; at the end of the line terminates the declaration statement.

*/
