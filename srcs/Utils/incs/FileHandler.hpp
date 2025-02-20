/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/09 23:47:20 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 19:48:41 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP


#include <string>
#include <fstream>
#include <sys/stat.h>


class   FileHandler
{

public:

    // Read a file and return its contents
    static std::string readFile(const std::string& path);

    // Write data to a file
    static void writeFile(const std::string& path, const std::string& data);

    // Check if a file exists
    static bool fileExists(const std::string& path);

    // Check if a path is a directory
    static bool isDirectory(const std::string& path);
    
};

#endif // FILEHANDLER_HPP
