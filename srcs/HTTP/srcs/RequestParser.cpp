/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 20:31:29 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:45:31 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "RequestParser.hpp"

RequestParser::RequestParser() {}

RequestParser::~RequestParser() {}

void RequestParser::parse(const char* data, size_t length, Request& request) {
    // Convert the request data to a string
    std::string request_data(data, length);
    
    // Create a string stream from the request data
    std::istringstream iss(request_data);
    
    // Line buffer
    std::string line;

    // Parse the request line
    std::getline(iss, line);

    // Parse the request line
    parseRequestLine(line, request);

    // Parse the headers
    while (std::getline(iss, line)) { // Read each line
        if (line.empty() || line == "\r") {  // Check for empty line
            break; // End of headers, break
        }
        parseHeaderLine(line, request); // Parse the header line
    }

    // Parse the body (if any)
    std::ostringstream body_stream; // Body stream
    body_stream << iss.rdbuf(); // Read the remaining data into the body stream
    request.setBody(body_stream.str()); // Get the body as a string
}

// Parse the request line
void RequestParser::parseRequestLine(const std::string& line, Request& request) {
    // Create a string stream from the request line
    std::istringstream iss(line);

    // Parse the request line
    std::string method, uri, version;

    // Extract the method, URI, and version
    iss >> method >> uri >> version;

    // Set the request method, URI, and version
    request.setMethod(method);

    // Set the request URI
    request.setUri(uri);

    // Set the request version
    request.setVersion(version);
}

// Parse a header line
void RequestParser::parseHeaderLine(const std::string& line, Request& request) {
    // Find the colon character
    size_t colon_pos = line.find(':');

    // Check if the colon character was found
    if (colon_pos != std::string::npos) {
        // Extract the key and value
        std::string key = line.substr(0, colon_pos);

        // Extract the value
        std::string value = line.substr(colon_pos + 1);
        
        // Trim leading/trailing whitespace
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        // Set the header in the request
        request.setHeader(key, value);
    }
}
