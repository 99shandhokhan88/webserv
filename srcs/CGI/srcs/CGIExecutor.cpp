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
    
std::string script_path = _location.getRoot();

if (script_path[script_path.size()-1] != '/') script_path += "/";
script_path += _request.getPath().substr(_location.getPath().length());


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
    std::string script_path = FileHandler::sanitizePath(
        _location.getRoot() + "/" + 
        _request.getPath().substr(_location.getPath().length())
    );
    
    if (!FileHandler::fileExists(script_path)) {
        throw std::runtime_error("CGI script not found: " + script_path);
    }
    return script_path;
}



std::string CGIExecutor::execute() {
    // Must call setupEnvironment FIRST!
    setupEnvironment();
    
const std::string script_path = FileHandler::normalizePath(
    _location.getRoot() + "/" + 
    _request.getPath().substr(_location.getPath().length())
);
if (!FileHandler::fileExists(script_path)) {
    throw std::runtime_error("CGI script not found: " + script_path);
}
std::string output;

    std::cout << "=== CGI DEBUG ===" << std::endl;
    std::cout << "Script path: " << script_path << std::endl;
    std::cout << "POST Body size: " << _request.getBody().size() << std::endl;
    std::cout << "Environment variables:" << std::endl;
    
    if (_env) {
        for (char** e = _env; *e != NULL; e++) {
            std::cout << "  " << *e << std::endl;
        }
    }

    if (!FileHandler::fileExists(script_path)) {
        throw std::runtime_error("CGI script not found: " + script_path);
    }
    if (!FileHandler::isExecutable(script_path)) {
        throw std::runtime_error("CGI script not executable: " + script_path);
    }

    char** args = createExecArgs();

    int pipe_in[2], pipe_out[2];
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        free_env(args);
        throw std::runtime_error("Pipe creation failed");
    }

    pid_t pid = fork();
    if (pid == -1) {
        free_env(args);
        throw std::runtime_error("Fork failed");
    }

    if (pid == 0) {  // Child process
        close(pipe_in[1]); 
        close(pipe_out[0]);

        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        
        execve(args[0], args, _env);
        std::cerr << "Execve failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    } 
    else {  // Parent process
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
            throw std::runtime_error("Failed to write POST data to CGI");
        }
        body_data += written;
        remaining -= written;
    }
}
        close(pipe_in[1]);

        // Read output
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytes_read);
        }
        close(pipe_out[0]);

        // Cleanup arguments
        free_env(args);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw std::runtime_error("CGI script exited with error status");
        }

        // Extract response body
        size_t header_end = output.find("\r\n\r\n");
        return (header_end != std::string::npos) ? 
               output.substr(header_end + 4) : 
               output;
    }
}