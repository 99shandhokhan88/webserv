/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 22:48:11 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 23:48:08 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "FileHandler.hpp"          // Local header
#include <sys/stat.h>               // Standard library

// Read a file and return its contents
std::string FileHandler::readFile(const std::string& path)
{
    
    // Open the file
    std::ifstream file(path.c_str(), std::ios::binary);

    // Check if the file was opened successfully
    if (!file.is_open())
    {

        // Throw an exception if the file could not be opened
        throw   (std::runtime_error("Failed to open file: " + path));
    
    }

    // Read the file contents and store them in a string using an iterator and the file stream
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // Return the file contents
    return  (content);
}


// Write data to a file
void FileHandler::writeFile(const std::string& path, const std::string& data)
{

    // Open the file for writing
    std::ofstream file(path.c_str(), std::ios::binary);

    // Check if the file was opened successfully
    if (!file.is_open())
    {

        // Throw an exception if the file could not be opened
        throw   (std::runtime_error("Failed to open file for writing: " + path));
        
    }

    file << data;   // Write the data to the file
}


// Check if a file exists
bool FileHandler::fileExists(const std::string& path)
{
    struct stat buffer; // File status buffer
    
    return  (stat(path.c_str(), &buffer) == 0);  // Check if the file exists
}


// Check if a path is a directory
bool FileHandler::isDirectory(const std::string& path)
{
    struct stat buffer; // File status buffer
    
    if (stat(path.c_str(), &buffer) == 0)
    {
        return  (S_ISDIR(buffer.st_mode));   // Return false if the stat function fails
    }
    return  (false);  // Check if the path is a directory
}

// End of srcs/Utils/srcs/FileHandler.cpp
