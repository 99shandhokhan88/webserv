// FileHandler.hpp
#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <string>
#include <vector>
#include <unistd.h>
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
    static std::string getDirectory(const std::string& path);

    static std::string readFile(const std::string& path);
    static bool writeFile(const std::string& path, const std::string& content);
    static bool fileExists(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isExecutable(const std::string& path) {
        return access(path.c_str(), X_OK) == 0;
    }
    static bool deleteFile(const std::string& path);
    static bool deleteDirectory(const std::string& path);
    static std::string sanitizePath(const std::string& path);

    // In FileHandler.hpp
static std::string normalizePath(const std::string& path) {
    std::string result;
    std::vector<std::string> parts;
    
    size_t start = 0;
    while (start < path.size()) {
        size_t end = path.find('/', start);
        std::string part = path.substr(start, end - start);
        
        if (part == "..") {
            if (!parts.empty()) parts.pop_back();
        } 
        else if (!part.empty() && part != ".") {
            parts.push_back(part);
        }
        
        start = (end == std::string::npos) ? end : end + 1;
    }
    
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) result += "/";
        result += parts[i];
    }
    
    return "/" + result;
}
};

#endif