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