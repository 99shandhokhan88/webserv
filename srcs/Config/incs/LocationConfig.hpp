/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 11:35:01 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/19 23:52:10 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

class LocationConfig {
private:
    std::string path;
    std::string root;
    std::string cgi_extension;
    std::string cgi_path;
    std::vector<std::string> allowed_methods;
    std::string index;

public:
    // Constructors and Destructor declarations
    LocationConfig();
    ~LocationConfig();

    // Setters declarations
    void setPath(const std::string& p);
    void setRoot(const std::string& r);
    void setCgiExtension(const std::string& ext);
    void setCgiPath(const std::string& p);
    void addAllowedMethod(const std::string& method);
    void setIndex(const std::string& idx);

    // Getters declarations
    const std::string& getPath() const;
    const std::string& getRoot() const;
    const std::string& getCgiExtension() const;
    const std::string& getCgiPath() const;
    const std::vector<std::string>& getAllowedMethods() const;
    const std::string& getIndex() const;
};

#endif
