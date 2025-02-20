/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StringUtils.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:54:29 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 19:55:11 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

class   StringUtils
{

public:

    // Trim leading and trailing whitespace from a string
    static std::string trim(const std::string& str);

    // Split a string by a delimiter
    static std::vector<std::string> split(const std::string& str, char delimiter);

    // Convert a string to lowercase
    static std::string toLower(const std::string& str);

    // Convert a string to uppercase
    static std::string toUpper(const std::string& str);
    
};

#endif // STRINGUTILS_HPP
