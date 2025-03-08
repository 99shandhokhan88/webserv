/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 18:50:57 by vzashev           #+#    #+#             */
/*   Updated: 2025/03/09 00:45:47 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Request.hpp"              // Local header
#include "StringUtils.hpp"


// Constructor
Request::Request()
{
    // No print in constructor
}


// Destructor
Request::~Request()
{
    // No print in destructor
}


void Request::parse(const char* data, size_t length) {
    raw_data.assign(data, length);
    std::istringstream iss(raw_data);
    std::string request_line;

    // Extract request line
    std::getline(iss, request_line);
    size_t method_end = request_line.find(' ');
    size_t path_end = request_line.find(' ', method_end + 1);

    if (method_end != std::string::npos && path_end != std::string::npos) {
        // CORRECTED: Use _method and _path instead of method/path
        _method = request_line.substr(0, method_end);
        _path = request_line.substr(method_end + 1, path_end - method_end - 1);
        
        // Convert method to uppercase
        for (size_t i = 0; i < _method.length(); ++i) {
            _method[i] = toupper(_method[i]);
        }
    } else {
        throw std::runtime_error("Invalid request line");
    }

    // Parse headers
    std::string header;
    while (std::getline(iss, header) && header != "\r") {
        size_t colon = header.find(':');
        if (colon != std::string::npos) {
            std::string key = header.substr(0, colon);
            std::string value = header.substr(colon + 2, header.length() - colon - 3);
            headers[key] = value;
        }
    }

    // CORRECTED: Use _body instead of body
    size_t header_end = raw_data.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        _body = raw_data.substr(header_end + 4);
    }
}




// Parse a header line
void Request::parseHeaderLine(const std::string& line)
{

    // Find the colon character
    size_t colon_pos = line.find(':');

    // Check if the colon character was found
    if (colon_pos != std::string::npos)
    {

        // Extract the key and value
        std::string key = line.substr(0, colon_pos);

        // Extract the value
        std::string value = line.substr(colon_pos + 1);

        
        // Trim leading/trailing whitespace
        key.erase(0, key.find_first_not_of(" \t\r\n")); // Erase leading whitespace
        key.erase(key.find_last_not_of(" \t\r\n") + 1); // Erase trailing whitespace
        value.erase(0, value.find_first_not_of(" \t\r\n")); // Erase leading whitespace
        value.erase(value.find_last_not_of(" \t\r\n") + 1); // Erase trailing whitespace
        
        // Add the header to the map
        headers[key] = value;
        
    }
}




// Setters
void Request::setMethod(const std::string& _method) {
    this->_method = _method;
}

void Request::setUri(const std::string& _uri) {
    this->_uri = _uri;
}

void Request::setVersion(const std::string& _version) {
    this->_version = _version;
}

void Request::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void Request::setBody(const std::string& _body) {
    this->_body = _body;
}









void Request::parseRequestLine(const std::string& line) {
    size_t method_end = line.find(' ');
    size_t path_end = line.find(' ', method_end + 1);
    size_t version_end = line.find("\r\n");

    if (method_end != std::string::npos && path_end != std::string::npos) {
        _method = line.substr(0, method_end);
        _uri = line.substr(method_end + 1, path_end - method_end - 1);
        _version = line.substr(path_end + 1, version_end - path_end - 1);

        // Parse query parameters
        size_t query_pos = _uri.find('?');
        if (query_pos != std::string::npos) {
            _path = _uri.substr(0, query_pos);
            _query = _uri.substr(query_pos + 1);
        } else {
            _path = _uri;
        }
    }
}



/*

Key Features of the HTTP Implementation
Request Parsing:

The Request class parses HTTP requests, including the request line, headers, and body.

The RequestParser class provides helper methods for parsing.

Response Generation:

The Response class generates HTTP responses, including the status line, headers, and body.

The ResponseGenerator class provides helper methods for generating responses.

Error Handling:

The ResponseGenerator class can generate error responses (e.g., 404 Not Found, 500 Internal Server Error).

Modular Design:

Each component is modular and can be extended or modified independently.


*/