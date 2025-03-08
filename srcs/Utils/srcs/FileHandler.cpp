// FileHandler.cpp
#include "FileHandler.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

// List directory contents (filter out . and ..)
std::vector<std::string> FileHandler::listDirectory(const std::string& path) {
    std::vector<std::string> files;
    DIR* dir = opendir(path.c_str());
    
    if (dir) {
        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                files.push_back(ent->d_name);
        }
        closedir(dir);
    }
    return files;
}

// Create directory with error checking
bool FileHandler::createDirectory(const std::string& path) {
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

// Read file contents
std::string FileHandler::readFile(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path);
    return std::string((std::istreambuf_iterator<char>(file)), 
                      std::istreambuf_iterator<char>());
}

// Write file with success/failure return
bool FileHandler::writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path.c_str());
    if (!file) return false;
    file << content;
    return file.good();
}

// Check file existence
bool FileHandler::fileExists(const std::string& path) {
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0;
}

// Check if path is directory
bool FileHandler::isDirectory(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
    return S_ISDIR(buffer.st_mode);
}