/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:18:13 by vzashev           #+#    #+#             */
/*   Updated: 2025/04/02 19:47:43 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "MimeTypes.hpp"        // From Utils

#include <string>
#include <map>
#include <sstream>

#include "../../Utils/incs/MimeTypes.hpp"      // From Utils


class Response
{
                public:
                
                    // Constructor
                    Response();

                    // Destructor
                    ~Response();



                    // Setters

                    // Set the response status code
                    void setStatus(int status);

                    // Set a response header
                    void setHeader(const std::string& key, const std::string& value);
                
                    // Set the response body
                    void setBody(const std::string& _body);

                
                    // Generate the HTTP response
                    std::string generate() const;

                private:

                    // Member variables

                    //  Status code
                    int status;

                    //  Response headers
                    std::map<std::string, std::string> _headers;

                    //  Response body
                    std::string _body;
                    
};

#endif // RESPONSE_HPP
