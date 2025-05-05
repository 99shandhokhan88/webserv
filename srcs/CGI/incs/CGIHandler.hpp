/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/05 16:55:48 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/05 16:55:55 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// CGIHandler.hpp
#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <map>

class CGIHandler {
public:
    static std::string executeCGI(const std::string &scriptPath, const std::string &method,
                                  const std::map<std::string, std::string> &env,
                                  const std::string &body);
};

#endif
