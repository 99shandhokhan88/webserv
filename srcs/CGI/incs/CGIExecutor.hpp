/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIExecutor.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 11:38:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/04/02 19:42:47 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include "../../HTTP/incs/Request.hpp"
#include "../../Config/incs/LocationConfig.hpp"

#include <map>
#include <string>
#include <exception>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <iostream>
#include <cerrno>

class CGIException : public std::exception {
    std::string _msg;
public:
    explicit CGIException(const std::string& message) : _msg(message) {}
    virtual ~CGIException() throw() {}
    virtual const char* what() const throw() { return _msg.c_str(); }
};

class CGIExecutor
{
    private:
     const Request& _request;
     const LocationConfig& _location;
     LocationConfig _config;
     char** _env;
    std::string _output;
    void setupEnvironment();
    char** createExecArgs() const;

public:
CGIExecutor(const Request& request, const LocationConfig& config);
~CGIExecutor();
std::string execute();
    
};

#endif


