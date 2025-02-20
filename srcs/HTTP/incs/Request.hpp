/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 11:51:50 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:40:49 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "StringUtils.hpp"      // From Utils

#include <string>
#include <map>
#include <sstream>
#include <algorithm>


class Request
{

public:

    // Constructor
    Request();
    
    // Destructor
    ~Request();

    // Parse the request data
    void parse(const char* data, size_t length);



    // Setters
    void setMethod(const std::string& method);
    void setUri(const std::string& uri);
    void setVersion(const std::string& version);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);

    

    // Getters

    // Get the request method
    const std::string& getMethod() const;

    // Get the request URI
    const std::string& getUri() const;

    // Get the request version
    const std::string& getVersion() const;

    // Get the request headers
    const std::map<std::string, std::string>& getHeaders() const;
    
    // Get the request body
    const std::string& getBody() const;
    

private:

    // Request method
    std::string method;

    // Request URI
    std::string uri;

    // Request version
    std::string version;

    // Request headers
    std::map<std::string, std::string> headers;

    // Request body
    std::string body;

    

    // Helper functions
    
    // Parse the request line
    void parseRequestLine(const std::string& line);

    // Parse a header line
    void parseHeaderLine(const std::string& line);
    
};

#endif // REQUEST_HPP


/*

This directory (HTTP) contains the classes responsible for handling HTTP requests and responses:

Request.hpp: Header file for the HTTP request class.

Request.cpp: Implementation of the HTTP request class.

Response.hpp: Header file for the HTTP response class.

Response.cpp: Implementation of the HTTP response class.

RequestParser.hpp: Header file for the HTTP request parser.

RequestParser.cpp: Implementation of the HTTP request parser.

ResponseGenerator.hpp: Header file for the HTTP response generator.

ResponseGenerator.cpp: Implementation of the HTTP response generator.

*/
