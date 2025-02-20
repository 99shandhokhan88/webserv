/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 18:50:57 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:41:32 by vzashev          ###   ########.fr       */
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


// Parse the request data
void    Request::parse(const char* data, size_t length)
{
    
    // Convert the request data to a string
    std::string request_data(data, length);

    // Create a string stream from the request data
    std::istringstream iss(request_data);

    // Line buffer
    std::string line;
    

    // Get the request line
    std::getline(iss, line);

    // Parse the request line
    parseRequestLine(line);
    

    // Parse the headers
    while (std::getline(iss, line)  && !line.empty())
    {
        parseHeaderLine(line);
    }


    // Parse the body (if any)
    std::ostringstream body_stream; // Body stream
    body_stream << iss.rdbuf(); // Read the remaining data into the body stream
    body = body_stream.str();   // Get the body as a string
    
}


// Parse the request line
void Request::parseRequestLine(const std::string& line)
{
    
    // Create a string stream from the request line
    std::istringstream iss(line);

    // Parse the request line
    iss >> method >> uri >> version;
    
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
void Request::setMethod(const std::string& method) {
    this->method = method;
}

void Request::setUri(const std::string& uri) {
    this->uri = uri;
}

void Request::setVersion(const std::string& version) {
    this->version = version;
}

void Request::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void Request::setBody(const std::string& body) {
    this->body = body;
}



// Getters

// Get the request method
const std::string& Request::getMethod() const
{
    
    return  (method);  // Return the request method

}


// Get the request URI
const std::string& Request::getUri() const
{
    
    return  (uri);  // Return the request URI

}


// Get the request version
const std::string& Request::getVersion() const
{
    return  (version);  // Return the request version
}


// Get the request headers
const std::map<std::string, std::string>& Request::getHeaders() const
{
    
    return  (headers);  // Return the request headers
    
}


// Get the request body
const std::string& Request::getBody() const
{
    
    return  (body); // Return the request body
    
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