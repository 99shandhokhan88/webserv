// FileHandler.hpp
#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>  // Add this for directory operations
#include <cstring>   // For strcmp

class FileHandler {
public:
    // Directory operations
    static std::vector<std::string> listDirectory(const std::string& path);
    static bool createDirectory(const std::string& path);
    
    // File operations
    static std::string readFile(const std::string& path);
    static bool writeFile(const std::string& path, const std::string& content);
    static bool fileExists(const std::string& path);
    static bool isDirectory(const std::string& path);
};

#endif