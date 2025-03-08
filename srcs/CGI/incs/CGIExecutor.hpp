/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIExecutor.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 11:38:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/27 17:52:40 by vzashev          ###   ########.fr       */
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

class CGIExecutor {
    const Request& _request;
    const LocationConfig& _location;
    std::string _output;

public:
    CGIExecutor(const Request& req, const LocationConfig& loc);
    ~CGIExecutor();
    
    std::string execute();
};

#endif


