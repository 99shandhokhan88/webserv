// FileHandler.cpp
#include "FileHandler.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>  // For std::cerr
#include <string.h>  // For strerror

std::vector<std::string> FileHandler::listDirectory(const std::string& path) {
    std::vector<std::string> files;
    DIR* dir = opendir(path.c_str());
    
    if (dir) {
        struct dirent* ent;
        while ((ent = readdir(dir))) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                files.push_back(ent->d_name);
        }
        closedir(dir);
    }
    return files;
}

bool FileHandler::createDirectory(const std::string& path) {
    if (mkdir(path.c_str(), 0755) == 0) return true;
    return (errno == EEXIST) && isDirectory(path);
}

std::string FileHandler::readFile(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open file: " + path);
    return std::string(std::istreambuf_iterator<char>(file), 
                     std::istreambuf_iterator<char>());
}

bool FileHandler::writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file) return false;
    file << content;
    return file.good();
}

bool FileHandler::fileExists(const std::string& path) {
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0;
}

bool FileHandler::isDirectory(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0) && S_ISDIR(buffer.st_mode);
}

std::string FileHandler::getDirectory(const std::string& path) {
    size_t pos = path.find_last_of('/');
    return (pos != std::string::npos) ? path.substr(0, pos) : "";
}

bool FileHandler::deleteFile(const std::string& path) {
    if (!fileExists(path)) return false;
    return remove(path.c_str()) == 0;
}

bool FileHandler::deleteDirectory(const std::string& path) {
    if (!isDirectory(path)) return false;

    std::vector<std::string> entries = listDirectory(path);
    for (std::vector<std::string>::iterator it = entries.begin(); 
         it != entries.end(); ++it) {
        std::string fullPath = path + "/" + *it;
        if (isDirectory(fullPath)) {
            if (!deleteDirectory(fullPath)) return false;
        } else {
            if (!deleteFile(fullPath)) return false;
        }
    }

    return rmdir(path.c_str()) == 0;
}

std::string FileHandler::sanitizePath(const std::string& path) {
    std::string clean_path;
    
    // Remove invalid characters and normalize path
    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        
        // Skip semicolons and other invalid characters
        if (c == ';') continue;
        
        // Handle directory separators
        if (c == '\\') c = '/'; // Normalize Windows paths
        
        // Avoid duplicate slashes
        if (c == '/' && !clean_path.empty() && clean_path[clean_path.size()-1] == '/') continue;
        
        clean_path += c;
    }
    
    // Remove trailing slash unless it's root
    if (clean_path.size() > 1 && clean_path[clean_path.size()-1] == '/') {
        clean_path.erase(clean_path.size()-1);
    }
    
    return clean_path;
}