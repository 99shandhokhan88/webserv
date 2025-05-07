/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StringUtils.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:54:29 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/07 21:29:45 by vzashev          ###   ########.fr       */
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
static std::string urlDecode(const std::string &str) {
        std::ostringstream decoded;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '+') {
                decoded << ' ';
            } else if (str[i] == '%' && i + 2 < str.size()) {
                int hex;
                std::istringstream hexStream(str.substr(i + 1, 2));
                if (hexStream >> std::hex >> hex) {
                    decoded << static_cast<char>(hex);
                    i += 2;
                } else {
                    decoded << str[i];
                }
            } else {
                decoded << str[i];
            }
        }
        return decoded.str();
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
