// FileHandler.cpp
#include "FileHandler.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>  // For std::cerr
#include <string.h>  // For strerror
#include "../../Utils/incs/StringUtils.hpp"

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
    // Try to create directory first
    if (mkdir(path.c_str(), 0755) == 0) return true;
    
    // If failed because already exists
    if (errno == EEXIST) return isDirectory(path);
    
    // If failed because parent doesn't exist, try creating parents
    if (errno == ENOENT) {
        size_t pos = path.find_last_of('/');
        if (pos == std::string::npos) return false;
        
        std::string parent = path.substr(0, pos);
        if (!createDirectory(parent)) return false;
        return mkdir(path.c_str(), 0755) == 0;
    }
    
    std::cerr << "Error creating directory " << path 
              << ": " << strerror(errno) << std::endl;
    return false;
}

std::string FileHandler::readFile(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open file: " + path);
    return std::string(std::istreambuf_iterator<char>(file), 
                     std::istreambuf_iterator<char>());
}

bool FileHandler::writeFile(const std::string& path, const std::string& content) {
    // Estrai solo il contenuto dopo 'textcontent='
    size_t content_start = content.find("textcontent=");
    std::string real_content = (content_start != std::string::npos) ? 
        content.substr(content_start + 12) : content;
    
    // Decodifica URL
    real_content = StringUtils::urlDecode(real_content);
    
    std::ofstream file(path.c_str());
    if (!file) return false;
    
    file << real_content;
    return file.good();
}

bool FileHandler::fileExists(const std::string& path) {
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0;
}

bool FileHandler::isDirectory(const std::string& path) {
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        std::cerr << "Error checking directory: " << path 
                  << " - " << strerror(errno) << std::endl;
        return false;
    }
    return S_ISDIR(buffer.st_mode);
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
    bool last_was_slash = false;
    const size_t len = path.length();
    
    for (size_t i = 0; i < len; ++i) {
        char c = path[i];  // Removed const to allow modification
        
        // Skip control characters and special chars
        if (c <= 32 || c == ';' || c == '|' || c == '&') continue;
        
        // Normalize slashes
        if (c == '\\') c = '/';
        
        // Skip duplicate slashes
        if (c == '/' && last_was_slash) continue;
        
        last_was_slash = (c == '/');
        clean_path += c;
    }
    
    // DON'T remove trailing slash for directory paths - this is needed for autoindex
    // Only remove trailing slash for non-root paths if it's not intended to be a directory
    // The server logic will handle directory detection and redirects properly
    
    return clean_path;
}

bool FileHandler::writeBinaryFile(const std::string& path, const std::string& content) {
    // Ensure directory exists
    std::string dir = path.substr(0, path.find_last_of('/'));
    if (!FileHandler::createDirectory(dir)) {
        std::cerr << "Failed to create directory: " << dir << std::endl;
        return false;
    }
    
    // Open file in binary mode
    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << path << std::endl;
        return false;
    }
    
    // Write content
    file.write(content.c_str(), content.size());
    bool success = file.good();
    file.close();
    
    std::cout << "File written: " << path << " (size: " << content.size() << " bytes)" << std::endl;
    return success;
}

std::string FileHandler::getAbsolutePath(const std::string& relativePath) {
    char resolvedPath[PATH_MAX];
    
    // First normalize the path by removing . and .. components
    std::string normalizedPath = FileHandler::sanitizePath(relativePath);
    
    // If it's already an absolute path, return it
    if (!normalizedPath.empty() && normalizedPath[0] == '/') {
        return normalizedPath;
    }
    
    // Get the current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        std::cerr << "Failed to get current working directory" << std::endl;
        return normalizedPath; // Return as is if we can't get CWD
    }
    
    // Build absolute path
    std::string absolutePath = std::string(cwd) + "/" + normalizedPath;
    
    // Resolve the absolute path
    if (realpath(absolutePath.c_str(), resolvedPath) == NULL) {
        std::cerr << "Failed to resolve path: " << absolutePath << std::endl;
        return absolutePath; // Return unresolved absolute path
    }
    
    return std::string(resolvedPath);
}

bool FileHandler::isPathWithinRoot(const std::string& path, const std::string& root) {
    std::string absPath = FileHandler::getAbsolutePath(path);
    std::string absRoot = FileHandler::getAbsolutePath(root);
    
    return absPath.find(absRoot) == 0;
}