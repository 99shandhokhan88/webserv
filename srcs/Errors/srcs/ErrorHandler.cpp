/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:42:12 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:37:28 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorHandler.hpp"         // Local header


// Log an error message to stderr
void ErrorHandler::logError(const std::string& message)
{

    // Print the error message to stderr
    std::cerr
    << "[ERROR] "
    << message
    << std::endl;
    
}


// Throw a runtime error with the given message
void ErrorHandler::throwError(const std::string& message)
{

    // Throw a runtime error with the given message
    throw   (std::runtime_error(message));
    
}


// Handle a system error with the given context
void ErrorHandler::handleSystemError(const std::string& context)
{
    
    // Get the error message for the current errno value
    std::string error_message = context + ": " + std::strerror(errno);
    
    // Log and throw the error
    logError(error_message);

    // Throw the error
    throwError(error_message);
    
}


// Handle a custom error with the given code and message
void ErrorHandler::handleCustomError(int code, const std::string& message)
{

    std::ostringstream oss;
    
    // Create an error message with the code and message
    oss << "Error " << code << ": " << message;

    std::string error_message = oss.str(); // Convert to std::string

    // Log and throw the error
    logError(error_message);

    // Throw the error
    throwError(error_message);
    
}


/*

Key Features of the Error Implementation
Error Logging:

The logError method logs error messages to stderr with a prefix ([ERROR]).

Error Throwing:

The throwError method throws a std::runtime_error with the provided error message.

System Error Handling:

The handleSystemError method logs and throws errors related to system calls (e.g., socket, bind, poll).

It uses errno to retrieve the system error message.

Custom Error Handling:

The handleCustomError method logs and throws custom errors with a specific error code.

*/

// End of srcs/Errors/srcs/ErrorHandler.cpp
