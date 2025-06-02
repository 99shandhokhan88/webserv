/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:34:31 by vzashev           #+#    #+#             */
/*   Updated: 2025/06/02 22:51:05 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../incs/webserv.hpp"


#include "Response.hpp"             // Local header

// Constructor, status code is 200 by default
Response::Response() : status(200)
{
    // No print in constructor
}


// Destructor
Response::~Response()
{
    // No print in destructor
}




// Setters


// Set the response status code
void Response::setStatus(int status)
{
    
    this->status = status;  // Set the status code
    
}


// Set a response header
void Response::setHeader(const std::string& key, const std::string& value)
{
    
    _headers[key] = value;   // Set the header
    
}


// Set the response body
void Response::setBody(const std::string& _body)
{
    
    this->_body = _body;  // Set the body
    
}


// Generate the HTTP response
std::string Response::generate() const
{
    
    // Output stream
    std::ostringstream oss;

    // Status line
    oss
    << "HTTP/1.1 "
    << status
    << " OK\r\n";   // Status code and reason phrase

    // Headers, including Content-Length and Connection headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        oss
        << it->first
        << ": "
        << it->second
        << "\r\n";  // Header key and value
    }

    // End of headers
    oss << "\r\n";

    // Body
    oss << _body;

    // Return the response as a string
    return  (oss.str());
}
