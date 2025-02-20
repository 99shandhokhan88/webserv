/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseGenerator.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 20:40:56 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:46:15 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "ResponseGenerator.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include <sstream> // For std::ostringstream

ResponseGenerator::ResponseGenerator() {}

ResponseGenerator::~ResponseGenerator() {}

std::string ResponseGenerator::generateResponse(const Request& request) {
    Response response;

    // Example: Generate a simple response
    response.setStatus(200);
    response.setHeader("Content-Type", "text/html");
    response.setBody("<h1>Hello, World!</h1>");

    (void)request; // Silence the unused parameter warning

    return response.generate();
}

std::string ResponseGenerator::generateErrorResponse(int status, const std::string& message) {
    Response response;
    response.setStatus(status);
    response.setHeader("Content-Type", "text/html");

    // Use std::ostringstream to convert the status to a string
    std::ostringstream oss;
    oss << "<h1>Error " << status << ": " << message << "</h1>";
    response.setBody(oss.str());

    return response.generate();
}