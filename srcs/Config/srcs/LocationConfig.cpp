/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 12:46:31 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/27 17:56:01 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

// Constructors and Destructor
LocationConfig::~LocationConfig() { }

// Setters
void LocationConfig::setPath(const std::string& p) { 
    path = p; 
}

void LocationConfig::setRoot(const std::string& r) { 
    root = r; 
}

void LocationConfig::setCgiExtension(const std::string& ext) { 
    cgi_extension = ext; 
}



void LocationConfig::addAllowedMethod(const std::string& method) { 
    allowed_methods.push_back(method); 
}

void LocationConfig::setIndex(const std::string& idx) { 
    index = idx; 
}

// Getters
const std::string& LocationConfig::getPath() const { 
    return path; 
}

const std::string& LocationConfig::getRoot() const { 
    return root; 
}

const std::string& LocationConfig::getCgiExtension() const { 
    return cgi_extension; 
}



const std::vector<std::string>& LocationConfig::getAllowedMethods() const { 
    return allowed_methods; 
}

const std::string& LocationConfig::getIndex() const { 
    return index; 
}
