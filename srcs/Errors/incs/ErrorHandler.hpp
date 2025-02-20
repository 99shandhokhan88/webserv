/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:31:05 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:32:52 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP


#include <string>
#include <stdexcept>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sstream>


class   ErrorHandler
{
    
public:

        // Log an error message
        static void logError(const std::string& message);

        // Throw a runtime error with a message
        static void throwError(const std::string& message);

        // Handle a system error (e.g., from errno)
        static void handleSystemError(const std::string& context);

        // Handle a custom error with a specific error code
        static void handleCustomError(int code, const std::string& message);

};

#endif // ERRORHANDLER_HPP

/*

The ErrorHandler class is a utility class for handling errors in the server code.
It provides static methods for logging errors, throwing exceptions, and handling system errors.
The logError method prints an error message to stderr.
The throwError method throws a runtime error with a given message.
The handleSystemError method logs and throws a system error with a context message.
The handleCustomError method logs and throws a custom error with a specific error code and message.

*/
