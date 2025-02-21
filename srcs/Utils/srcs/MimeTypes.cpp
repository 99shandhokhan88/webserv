#include "../incs/MimeTypes.hpp"

// Inizializzazione membro statico
std::map<std::string, std::string> MimeTypes::_types;

void MimeTypes::initTypes() {
    _types[".html"] = "text/html";
    _types[".css"] = "text/css";
    _types[".js"] = "application/javascript";
    _types[".png"] = "image/png";
    _types[".jpg"] = "image/jpeg";
    _types[".jpeg"] = "image/jpeg";
    _types[".gif"] = "image/gif";
}

std::string MimeTypes::getType(const std::string& filename) {
    static bool initialized = false;
    if (!initialized) {
        initTypes();
        initialized = true;
    }
    
    size_t dot_pos = filename.find_last_of(".");
    if (dot_pos != std::string::npos) {
        std::string extension = filename.substr(dot_pos);
        if (_types.find(extension) != _types.end()) {
            return _types[extension];
        }
    }
    return "text/plain";
}