/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 18:50:57 by vzashev           #+#    #+#             */
/*   Updated: 2025/06/02 22:51:01 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../incs/webserv.hpp"

#include "Request.hpp"

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




std::string Request::dechunk(const std::string& body) {
    std::string result;
    size_t pos = 0;

    while (pos < body.size()) {
        size_t chunk_size_end = body.find("\r\n", pos);
        if (chunk_size_end == std::string::npos) break;

        // Estrai dimensione chunk
        std::string chunk_size_str = body.substr(pos, chunk_size_end - pos);
        unsigned int chunk_size;
        std::istringstream iss(chunk_size_str);
        iss >> std::hex >> chunk_size;

        if (chunk_size == 0) break; // Chunk finale
        
        // Salta "\r\n" e leggi i dati
        pos = chunk_size_end + 2;
        result += body.substr(pos, chunk_size);
        pos += chunk_size + 2; // Salta "\r\n" dopo i dati
    }

    return result;
}



void Request::parse(const char* data, size_t length) {
    raw_data.append(data, length);

    // Separazione header/body
    size_t header_end = raw_data.find("\r\n\r\n");
    if (header_end == std::string::npos) return;

    // Parsing headers
    std::istringstream headers_stream(raw_data.substr(0, header_end));
    std::string header_line;
    while (std::getline(headers_stream, header_line)) {
        parseHeaderLine(header_line);
    }

    // Estrazione body
    _body = raw_data.substr(header_end + 4);

    // Gestione chunked encoding
    if (getHeader("Transfer-Encoding") == "chunked") {
        _body = dechunk(_body);
    }

    // Parsing request line (METODO/PATH/VERSIONE)
    size_t first_line_end = raw_data.find("\r\n");
    parseRequestLine(raw_data.substr(0, first_line_end));
}


// Parse a header line
void Request::parseHeaderLine(const std::string& line)
{

    size_t colon_pos = line.find(':');

    if (colon_pos != std::string::npos)
    {

        std::string key = line.substr(0, colon_pos);

        std::string value = line.substr(colon_pos + 1);

        
        // Trim leading/trailing whitespace
        key.erase(0, key.find_first_not_of(" \t\r\n")); // Erase leading whitespace
        key.erase(key.find_last_not_of(" \t\r\n") + 1); // Erase trailing whitespace
        value.erase(0, value.find_first_not_of(" \t\r\n")); // Erase leading whitespace
        value.erase(value.find_last_not_of(" \t\r\n") + 1); // Erase trailing whitespace
        
        // Add the header to the map
        _headers[key] = value;
        
    }
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
    _path = FileHandler::sanitizePath(_path);
    _uri = FileHandler::sanitizePath(_uri);
}


