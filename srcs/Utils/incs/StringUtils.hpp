/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StringUtils.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:54:29 by vzashev           #+#    #+#             */
/*   Updated: 2025/04/03 19:46:39 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>

class StringUtils {
public:
    // Template function for generic conversion to string
    template <typename T>
    static std::string toString(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    // Trim leading and trailing whitespace
    static std::string trim(const std::string& str);

    // Split string by delimiter
    static std::vector<std::string> split(const std::string& str, char delimiter);

    // Convert to lowercase
    static std::string toLower(const std::string& str);

    // Convert to uppercase
    static std::string toUpper(const std::string& str);
};

#endif // STRINGUTILS_HPP
