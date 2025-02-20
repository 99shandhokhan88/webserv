/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 16:26:15 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 19:40:08 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP


#include <string>
#include <sstream>
#include "Request.hpp"


class RequestParser
{

public:

    RequestParser();
    
    ~RequestParser();

    // Parse the request data
    void parse(const char* data, size_t length, Request& request);

private:

    // Helper functions

    // Parse the request line
    void parseRequestLine(const std::string& line, Request& request);
    
    // Parse a header line
    void parseHeaderLine(const std::string& line, Request& request);

};

#endif // REQUESTPARSER_HPP
