#include "CGIExecutor.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <poll.h>
#include "../../HTTP/incs/Request.hpp"
#include "../../Config/incs/LocationConfig.hpp"
#include "../../Utils/incs/FileHandler.hpp"
#include "../../Utils/incs/StringUtils.hpp"



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
    const std::string script_path = getScriptPath();
    size_t dot_pos = script_path.find_last_of('.');
    std::string ext = (dot_pos != std::string::npos) ? script_path.substr(dot_pos) : "";
    
    std::string interpreter = _location.getCgiInterpreter(ext);
    if (interpreter.empty()) {
        throw std::runtime_error("No CGI interpreter configured for extension: " + ext);
    }
    
    std::cerr << "DEBUG: Setting up CGI for extension: " << ext << " with interpreter: " << interpreter << std::endl;
    
    // Configurazione per diversi tipi di script
    char** args = new char*[3];
    args[0] = strdup(interpreter.c_str());
    // Estrai solo il nome del file (basename)
size_t last_slash = script_path.find_last_of('/');
std::string script_name = (last_slash != std::string::npos) ? script_path.substr(last_slash + 1) : script_path;
args[1] = strdup(script_name.c_str());

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

        std::cerr << "Created CGI arguments:\n";
        for (int i = 0; args[i] != NULL; i++) {
            std::cerr << "args[" << i << "]: " << (args[i] ? args[i] : "NULL") << "\n";
        }

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
                    std::cerr << "Child process: dup2 failed for stdin: " << strerror(errno) << std::endl;
                    throw std::runtime_error("dup2 failed for stdin");
                }
                if (dup2(pipe_out[1], STDOUT_FILENO) == -1) {
                    std::cerr << "Child process: dup2 failed for stdout: " << strerror(errno) << std::endl;
                    throw std::runtime_error("dup2 failed for stdout");
                }

                // Change to script directory
                std::string script_dir = extractDirectory(script_path);
                if (chdir(script_dir.c_str()) == -1) {
                    std::cerr << "Child process: Failed to change directory to: " << script_dir << " - " << strerror(errno) << std::endl;
                    throw std::runtime_error("Failed to change directory to: " + script_dir);
                }
                
                std::cerr << "Child process: About to execute CGI with:\n";
                std::cerr << "  Interpreter: " << args[0] << std::endl;
                std::cerr << "  Script: " << args[1] << std::endl;
                
                execve(args[0], args, _env);
                
                // If we get here, execve has failed
                std::cerr << "Child process: execve failed: " << strerror(errno) << std::endl;
                _exit(1);
            } catch (const std::exception& e) {
                std::cerr << "Child process exception: " << e.what() << std::endl;
                _exit(1);
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

        // Read output with timeout using poll() instead of select()
        std::string output;
        char buffer[4096];
        ssize_t bytes_read;
        int timeout_seconds = 30; // 30 seconds timeout
        bool timeout_occurred = false;

        // Setup non-blocking I/O
        fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);

        time_t start_time = time(NULL);
        while (true) {
            time_t current_time = time(NULL);
            int remaining_time = timeout_seconds - (current_time - start_time);
            
            if (remaining_time <= 0) {
                std::cerr << "CGI TIMEOUT: Script exceeded " << timeout_seconds << " seconds, terminating process " << pid << std::endl;
                timeout_occurred = true;
                break;
            }

            // Use poll() instead of select() - consistent with project choice
            struct pollfd pfd;
            pfd.fd = pipe_out[0];
            pfd.events = POLLIN;
            pfd.revents = 0;

            int poll_timeout_ms = remaining_time * 1000; // Convert to milliseconds
            int poll_result = poll(&pfd, 1, poll_timeout_ms);
            
            if (poll_result == -1) {
                if (errno == EINTR) continue; // Interrupted by signal, retry
                close(pipe_out[0]);
                free_env(args);
                kill(pid, SIGKILL); // Terminate child process
                waitpid(pid, NULL, 0); // Clean up zombie
                throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));
            }
            if (poll_result == 0) {
                std::cerr << "CGI TIMEOUT: No data available within timeout" << std::endl;
                timeout_occurred = true;
                break;
            }

            if (pfd.revents & POLLIN) {
                bytes_read = read(pipe_out[0], buffer, sizeof(buffer));
                if (bytes_read < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue; // No data available, continue waiting
                    }
                    std::cerr << "Read error: " << strerror(errno) << std::endl;
                    break;
                } else if (bytes_read == 0) {
                    std::cerr << "CGI process closed output pipe" << std::endl;
                    break; // EOF
                } else {
                    output.append(buffer, bytes_read);
                }
            }
            
            if (pfd.revents & (POLLHUP | POLLERR)) {
                std::cerr << "CGI process closed connection or error occurred" << std::endl;
                break;
            }
        }
        close(pipe_out[0]);

        // Handle timeout
        if (timeout_occurred) {
            std::cerr << "CGI TIMEOUT: Terminating child process " << pid << std::endl;
            
            // First try SIGTERM (graceful termination)
            if (kill(pid, SIGTERM) == 0) {
                sleep(2); // Give process 2 seconds to terminate gracefully
                
                // Check if process is still alive
                int status;
                pid_t result = waitpid(pid, &status, WNOHANG);
                if (result == 0) {
                    // Process still alive, force kill
                    std::cerr << "CGI TIMEOUT: Process " << pid << " didn't respond to SIGTERM, sending SIGKILL" << std::endl;
                    kill(pid, SIGKILL);
                    waitpid(pid, &status, 0); // Wait for forced termination
                }
            }
            
            free_env(args);
            throw std::runtime_error("CGI_TIMEOUT:CGI script timed out after " + StringUtils::toString(timeout_seconds) + " seconds");
        }

        // Cleanup arguments
        free_env(args);

        int status;
        pid_t wait_result = waitpid(pid, &status, 0);
        if (wait_result == -1) {
            throw std::runtime_error("Failed to wait for CGI process: " + std::string(strerror(errno)));
        }

        // Check if child process exited normally
        if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            throw std::runtime_error("CGI process terminated by signal " + StringUtils::toString(sig));
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw std::runtime_error("CGI process exited with status " + StringUtils::toString(WEXITSTATUS(status)));
        }

        // --- PARSING CGI OUTPUT ---
        std::cout << "DEBUG: CGI raw output size: " << output.size() << " bytes." << std::endl;
        if (output.size() > 0) {
            std::cout << "DEBUG: First 100 chars: " << output.substr(0, std::min(output.size(), size_t(100))) << std::endl;
        }
        
        // Split headers and body
        size_t header_end = output.find("\r\n\r\n");
        if (header_end == std::string::npos)
            header_end = output.find("\n\n");
            
        std::cout << "DEBUG: Header end position: " << header_end << std::endl;
        
        std::string headers, body;
        if (header_end != std::string::npos) {
            headers = output.substr(0, header_end);
            body = output.substr(header_end + ((output[header_end] == '\r') ? 4 : 2));
            std::cout << "DEBUG: Headers size: " << headers.size() << ", Body size: " << body.size() << std::endl;
        } else {
            // No headers, treat all as body
            body = output;
            std::cout << "DEBUG: No headers found, all content is body. Size: " << body.size() << std::endl;
        }
        // Compose HTTP response
        std::string response = "HTTP/1.1 200 OK\r\n";
        if (!headers.empty()) {
            response += headers + "\r\n\r\n";
        } else {
            response += "Content-Type: text/html\r\n\r\n";
        }
        response += body;
        
        std::cout << "DEBUG: Final response size: " << response.size() << " bytes." << std::endl;
        std::cout << "DEBUG: First 100 chars: " << response.substr(0, std::min(response.size(), size_t(100))) << std::endl;
        
        return response;

    } catch (const std::exception& e) {
        std::cerr << "CGI Execution Error: " << e.what() << std::endl;
        throw; // Re-throw to let the server handle it
    }
}