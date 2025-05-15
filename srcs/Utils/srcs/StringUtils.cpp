/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StringUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 02:54:42 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/15 19:25:27 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "StringUtils.hpp"          // Local header
#include <algorithm>                // Standard library
#include <sstream> // For std::istringstream
#include "../../Config/incs/ServerConfig.hpp"


// Trim leading and trailing whitespace from a string
std::string StringUtils::trim(const std::string& str)
{

    size_t first = str.find_first_not_of(" \t\r\n");
    
    if (first == std::string::npos)
        return  ("");

    size_t last = str.find_last_not_of(" \t\r\n");

    return  (str.substr(first, last - first + 1));
    
}


// Split a string by a delimiter
std::vector<std::string> StringUtils::split(const std::string& str, char delimiter)
{
    
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(str); // Requires <sstream>

    while (std::getline(iss, token, delimiter))
        tokens.push_back(token);

    return  (tokens);
    
}


// Convert a string to lowercase
std::string StringUtils::toLower(const std::string& str)
{
    std::string result = str;
    
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    
    return  (result);
}


// Convert a string to uppercase
std::string StringUtils::toUpper(const std::string& str)
{

    std::string result = str;
    
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    
    return  (result);
    
}





// Modify the extractBoundary method to be more robust
std::string extractBoundary(const std::string& contentType) {
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) {
        throw std::runtime_error("No boundary found in Content-Type");
    }
    
    pos += 9; // Move past "boundary="
    
    // Boundary might be quoted or not
    if (pos < contentType.size() && contentType[pos] == '"') {
        pos++; // Skip opening quote
        size_t end = contentType.find('"', pos);
        if (end == std::string::npos) {
            return contentType.substr(pos);
        }
        return contentType.substr(pos, end - pos);
    } else {
        // Unquoted boundary ends at first semicolon or end of string
        size_t end = contentType.find(';', pos);
        if (end == std::string::npos) {
            return contentType.substr(pos);
        }
        return contentType.substr(pos, end - pos);
    }
}