/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseGenerator.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:42:17 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:42:59 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef RESPONSEGENERATOR_HPP
#define RESPONSEGENERATOR_HPP


#include <string>
#include <sstream>
#include "Response.hpp"
#include "Request.hpp" // Include the Request header


class ResponseGenerator
{
    
public:

    ResponseGenerator();
    ~ResponseGenerator();

    // Generate a response for a given request
    std::string generateResponse(const Request& request);


        // Generate an error response
    std::string generateErrorResponse(int status, const std::string& message);

};

#endif // RESPONSEGENERATOR_HPP
