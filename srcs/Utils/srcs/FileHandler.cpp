
#include "../../../incs/webserv.hpp"


#include "FileHandler.hpp"


#include "../../Utils/incs/StringUtils.hpp"

// Initialize static members
std::queue<FileOperation*> FileHandler::pendingOperations;
std::map<int, FileOperation*> FileHandler::activeOperations;

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
    return files;  // Added missing return statement
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
    FileOperation* op = new FileOperation(FILE_OP_READ, path);
    addFileOperation(op);
    
    // Wait for operation to complete
    while (!op->isCompleted() && !op->hasFailed()) {
        handleFileOperations();
    }
    
    std::string result = op->getResult();
    delete op;
    return result;
}

bool FileHandler::writeFile(const std::string& path, const std::string& content) {
    // Extract content after 'textcontent='
    size_t content_start = content.find("textcontent=");
    std::string real_content = (content_start != std::string::npos) ? 
        content.substr(content_start + 12) : content;
    
    // URL decode
    real_content = StringUtils::urlDecode(real_content);
    
    FileOperation* op = new FileOperation(FILE_OP_WRITE, path, real_content);
    addFileOperation(op);
    
    // Wait for operation to complete
    while (!op->isCompleted() && !op->hasFailed()) {
        handleFileOperations();
    }
    
    bool success = op->isCompleted();
    delete op;
    return success;
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
    
    FileOperation* op = new FileOperation(FILE_OP_DELETE, path);
    addFileOperation(op);
    
    // Wait for operation to complete
    while (!op->isCompleted() && !op->hasFailed()) {
        handleFileOperations();
    }
    
    bool success = op->isCompleted();
    delete op;
    return success;
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
        char c = path[i];
        
        // Skip control characters and special chars
        if (c <= 32 || c == ';' || c == '|' || c == '&') continue;
        
        // Normalize slashes
        if (c == '\\') c = '/';
        
        // Skip duplicate slashes
        if (c == '/' && last_was_slash) continue;
        
        last_was_slash = (c == '/');
        clean_path += c;
    }
    
    return clean_path;
}

bool FileHandler::writeBinaryFile(const std::string& path, const std::string& content) {
    // Ensure directory exists
    std::string dir = path.substr(0, path.find_last_of('/'));
    if (!FileHandler::createDirectory(dir)) {
        std::cerr << "Failed to create directory: " << dir << std::endl;
        return false;
    }
    
    FileOperation* op = new FileOperation(FILE_OP_WRITE, path, content);
    addFileOperation(op);
    
    // Wait for operation to complete
    while (!op->isCompleted() && !op->hasFailed()) {
        handleFileOperations();
    }
    
    bool success = op->isCompleted();
    delete op;
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
        return normalizedPath;
    }
    
    // Build absolute path
    std::string absolutePath = std::string(cwd) + "/" + normalizedPath;
    
    // Resolve the absolute path
    if (realpath(absolutePath.c_str(), resolvedPath) == NULL) {
        std::cerr << "Failed to resolve path: " << absolutePath << std::endl;
        return absolutePath;
    }
    
    return std::string(resolvedPath);
}

bool FileHandler::isPathWithinRoot(const std::string& path, const std::string& root) {
    std::string absPath = FileHandler::getAbsolutePath(path);
    std::string absRoot = FileHandler::getAbsolutePath(root);
    
    return absPath.find(absRoot) == 0;
}

void FileHandler::addFileOperation(FileOperation* op) {
    if (op->start()) {
        activeOperations[op->getFd()] = op;
    } else {
        pendingOperations.push(op);
    }
}

void FileHandler::handleFileOperations() {
    if (activeOperations.empty() && pendingOperations.empty()) {
        return;
    }

    // Create poll array for active operations
    std::vector<struct pollfd> pollfds;
    for (std::map<int, FileOperation*>::const_iterator it = activeOperations.begin();
         it != activeOperations.end(); ++it) {
        struct pollfd pfd;
        pfd.fd = it->first;
        pfd.events = (it->second->getType() == FILE_OP_READ) ? POLLIN : POLLOUT;
        pfd.revents = 0;
        pollfds.push_back(pfd);
    }

    // Poll for events
    if (!pollfds.empty()) {
        int poll_result = poll(&pollfds[0], pollfds.size(), 0);
        if (poll_result > 0) {
            for (size_t i = 0; i < pollfds.size(); ++i) {
                FileOperation* op = activeOperations[pollfds[i].fd];
                if (!op->handlePollEvent(pollfds[i].revents)) {
                    // Operation completed or failed
                    op->cleanup();
                    activeOperations.erase(pollfds[i].fd);
                }
            }
        }
    }

    // Process pending operations
    while (!pendingOperations.empty()) {
        FileOperation* op = pendingOperations.front();
        pendingOperations.pop();
        
        if (op->start()) {
            activeOperations[op->getFd()] = op;
        } else {
            delete op;
        }
    }
}

void FileHandler::cleanup() {
    // Clean up active operations
    for (std::map<int, FileOperation*>::iterator it = activeOperations.begin();
         it != activeOperations.end(); ++it) {
        it->second->cleanup();
        delete it->second;
    }
    activeOperations.clear();

    // Clean up pending operations
    while (!pendingOperations.empty()) {
        delete pendingOperations.front();
        pendingOperations.pop();
    }
}