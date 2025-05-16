#include "CGIExecutor.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <iostream>
#include "../../HTTP/incs/Request.hpp"
#include "../../Config/incs/LocationConfig.hpp"
#include "../../Utils/incs/FileHandler.hpp"

// Utility function to convert values to string
template <typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Utility function to extract the directory from a file path
std::string extractDirectory(const std::string& path) {
    size_t found = path.find_last_of("/\\");
    return (found != std::string::npos) ? path.substr(0, found) : "";
}

void free_env(char** env) {
    if (env) {
        for (char** e = env; *e; e++) free(*e);
        delete[] env;
    }
}

CGIExecutor::CGIExecutor(const Request& request, const LocationConfig& location) : 
    _request(request), 
    _location(location),
    _env(NULL) {}

CGIExecutor::~CGIExecutor() {
    free_env(_env);
}

void CGIExecutor::setupEnvironment() {
    std::map<std::string, std::string> env_map;
    
    std::string script_path = getScriptPath();

    env_map["REQUEST_METHOD"] = _request.getMethod();
    env_map["SCRIPT_NAME"] = _request.getPath();  
    env_map["SCRIPT_FILENAME"] = script_path;
    env_map["PATH_TRANSLATED"] = script_path;
    env_map["PATH_INFO"] = "";  
    env_map["QUERY_STRING"] = _request.getQueryString();
    env_map["CONTENT_TYPE"] = _request.getHeader("Content-Type");
    env_map["CONTENT_LENGTH"] = _request.getHeader("Content-Length");
    env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    env_map["SERVER_NAME"] = "localhost";  
    env_map["SERVER_PORT"] = "8080";       
    env_map["REMOTE_ADDR"] = "127.0.0.1";
    env_map["DOCUMENT_ROOT"] = _location.getRoot();

    // Allocate space for entries + NULL terminator
    _env = new char*[env_map.size() + 1];
    int i = 0;

    for (std::map<std::string, std::string>::iterator it = env_map.begin(); 
         it != env_map.end(); ++it) {
        std::string entry = it->first + "=" + it->second;
        _env[i++] = strdup(entry.c_str());
    }
    _env[i] = NULL; // Proper NULL termination
}

// In CGIExecutor::createExecArgs()
char** CGIExecutor::createExecArgs() const {
    char** args = new char*[3];
    const std::string script_path = getScriptPath(); // Use validated path
    
    args[0] = strdup(_location.getCgiPath().c_str());
    args[1] = strdup(script_path.c_str());
    args[2] = NULL;
    
    return args;
}

// In CGIExecutor::getScriptPath()
std::string CGIExecutor::getScriptPath() const {
    // Costruisci il percorso completo mantenendo la struttura delle directory
    std::string base_path = _location.getRoot();
    if (base_path[base_path.length() - 1] != '/') {
        base_path += '/';
    }
    
    // Mantieni il percorso completo includendo cgi-bin
    std::string relative_path = _request.getPath();
    if (relative_path[0] == '/') {
        relative_path = relative_path.substr(1);
    }
    
    std::string script_path = FileHandler::sanitizePath(base_path + relative_path);
    
    std::cerr << "Base path: " << base_path << std::endl;
    std::cerr << "Relative path: " << relative_path << std::endl;
    std::cerr << "Full script path: " << script_path << std::endl;
    
    if (!FileHandler::fileExists(script_path)) {
        throw std::runtime_error("CGI script not found: " + script_path);
    }
    return script_path;
}

std::string CGIExecutor::execute() {
    try {
        setupEnvironment();
        
        // Debug: Print all environment variables
        std::cerr << "\n=== CGI Environment Variables ===\n";
        for (char** env = _env; *env != NULL; env++) {
            std::cerr << *env << std::endl;
        }
        std::cerr << "==============================\n";
        
        const std::string script_path = getScriptPath();

        // Detailed error checking
        std::cerr << "Checking script: " << script_path << std::endl;
        
        if (!FileHandler::fileExists(script_path)) {
            std::cerr << "Script not found: " << script_path << std::endl;
            throw std::runtime_error("CGI script not found: " + script_path);
        }
        
        if (!FileHandler::isExecutable(script_path)) {
            std::cerr << "Script not executable: " << script_path << std::endl;
            // Try to make it executable
            if (chmod(script_path.c_str(), 0755) != 0) {
                std::cerr << "Failed to make script executable: " << strerror(errno) << std::endl;
                throw std::runtime_error("CGI script not executable and couldn't set permissions: " + script_path);
            }
            std::cerr << "Made script executable: " << script_path << std::endl;
        }

        // Debug output
        std::cerr << "=== CGI Execution Details ===\n"
                  << "Script path: " << script_path << "\n"
                  << "CGI interpreter: " << _location.getCgiPath() << "\n"
                  << "Request method: " << _request.getMethod() << "\n"
                  << "Query string: " << _request.getQueryString() << "\n"
                  << "Content type: " << _request.getContentType() << "\n"
                  << "Content length: " << _request.getBodySize() << "\n"
                  << "Working directory: " << extractDirectory(script_path) << "\n"
                  << "==========================\n";

        char** args = createExecArgs();
        if (!args || !args[0] || !args[1]) {
            throw std::runtime_error("Failed to create CGI arguments");
        }

        std::cerr << "Created CGI arguments:\n"
                  << "args[0]: " << (args[0] ? args[0] : "NULL") << "\n"
                  << "args[1]: " << (args[1] ? args[1] : "NULL") << "\n";

        int pipe_in[2], pipe_out[2];
        if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
            free_env(args);
            throw std::runtime_error("Pipe creation failed: " + std::string(strerror(errno)));
        }

        pid_t pid = fork();
        if (pid == -1) {
            close(pipe_in[0]); close(pipe_in[1]);
            close(pipe_out[0]); close(pipe_out[1]);
            free_env(args);
            throw std::runtime_error("Fork failed: " + std::string(strerror(errno)));
        }

        if (pid == 0) {  // Child process
            try {
                close(pipe_in[1]); 
                close(pipe_out[0]);

                if (dup2(pipe_in[0], STDIN_FILENO) == -1) {
                    throw std::runtime_error("dup2 failed for stdin");
                }
                if (dup2(pipe_out[1], STDOUT_FILENO) == -1) {
                    throw std::runtime_error("dup2 failed for stdout");
                }

                // Change to script directory
                std::string script_dir = extractDirectory(script_path);
                if (chdir(script_dir.c_str()) == -1) {
                    throw std::runtime_error("Failed to change directory to: " + script_dir);
                }
                
                execve(args[0], args, _env);
                throw std::runtime_error("Execve failed: " + std::string(strerror(errno)));
            } catch (const std::exception& e) {
                std::cerr << "CGI child process error: " << e.what() << std::endl;
                exit(EXIT_FAILURE);
            }
        } 

        // Parent process
        close(pipe_in[0]); 
        close(pipe_out[1]);

        // Write POST data if needed
        if (_request.getMethod() == "POST") {
            const std::string& body = _request.getBody();
            const char* body_data = body.c_str();
            size_t remaining = body.size();
            ssize_t written;

            while (remaining > 0) {
                written = write(pipe_in[1], body_data, remaining);
                if (written <= 0) {
                    close(pipe_in[1]);
                    close(pipe_out[0]);
                    free_env(args);
                    throw std::runtime_error("Failed to write POST data to CGI: " + std::string(strerror(errno)));
                }
                body_data += written;
                remaining -= written;
            }
        }
        close(pipe_in[1]);

        // Read output with timeout
        std::string output;
        char buffer[4096];
        ssize_t bytes_read;
        int timeout = 30; // 30 seconds timeout
        fd_set read_fds;
        struct timeval tv;

        while (true) {
            FD_ZERO(&read_fds);
            FD_SET(pipe_out[0], &read_fds);
            tv.tv_sec = timeout;
            tv.tv_usec = 0;

            int ready = select(pipe_out[0] + 1, &read_fds, NULL, NULL, &tv);
            if (ready == -1) {
                close(pipe_out[0]);
                free_env(args);
                throw std::runtime_error("Select failed: " + std::string(strerror(errno)));
            }
            if (ready == 0) {
                close(pipe_out[0]);
                free_env(args);
                throw std::runtime_error("CGI script timed out after " + toString(timeout) + " seconds");
            }

            bytes_read = read(pipe_out[0], buffer, sizeof(buffer));
            if (bytes_read <= 0) break;
            output.append(buffer, bytes_read);
        }
        close(pipe_out[0]);

        // Cleanup arguments
        free_env(args);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                throw std::runtime_error("CGI script exited with status " + toString(exit_status));
            }
        } else if (WIFSIGNALED(status)) {
            throw std::runtime_error("CGI script terminated by signal " + toString(WTERMSIG(status)));
        }

        // Process CGI output
        size_t header_end = output.find("\r\n\r\n");
        if (header_end == std::string::npos) {
            // No headers found, assume it's all body
            return "Content-Type: text/html\r\n\r\n" + output;
        }

        // Extract headers and body
        std::string headers = output.substr(0, header_end);
        std::string body = output.substr(header_end + 4);

        // Check if Content-Type is already present in headers
        bool hasContentType = (headers.find("Content-Type:") != std::string::npos);
        
        // Build final response
        std::string response = "";
        if (!hasContentType) {
            response += "Content-Type: text/html\r\n";
        }
        response += headers + "\r\n\r\n" + body;

        return response;

    } catch (const std::exception& e) {
        std::cerr << "CGI Execution Error: " << e.what() << std::endl;
        throw; // Re-throw to let the server handle it
    }
}