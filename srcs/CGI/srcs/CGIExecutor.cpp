#include "CGIExecutor.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sstream>  // For std::ostringstream
#include <string>   // For std::string
#include <cstring>
#include "../../Utils/incs/FileHandler.hpp"

static void free_env(char** env) {
    for (char** e = env; *e; e++) free(*e);
    delete[] env;
}

CGIExecutor::CGIExecutor(const Request& request, const LocationConfig& location) : 
    _request(request), 
    _location(location),
    _env(NULL) {}

CGIExecutor::~CGIExecutor() {
    if (_env) free_env(_env);
}

void CGIExecutor::setupEnvironment() {
    std::map<std::string, std::string> env_map;
    
    env_map["REQUEST_METHOD"] = _request.getMethod();
    env_map["SCRIPT_FILENAME"] = _location.getRoot() + _request.getPath();
    env_map["PATH_INFO"] = _request.getPath();
    env_map["QUERY_STRING"] = _request.getQueryString();
    env_map["CONTENT_TYPE"] = _request.getHeader("Content-Type");
    env_map["CONTENT_LENGTH"] = _request.getHeader("Content-Length");
    env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    env_map["SERVER_SOFTWARE"] = "webserv/1.0";
    env_map["REMOTE_ADDR"] = "127.0.0.1";

    _env = new char*[env_map.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env_map.begin(); 
         it != env_map.end(); ++it) {
        std::string entry = it->first + "=" + it->second;
        _env[i] = strdup(entry.c_str());
        i++;
    }
    _env[i] = NULL;
}

char** CGIExecutor::createExecArgs() const {
    char** args = new char*[3];
    args[0] = strdup(_location.getCgiPath().c_str());
    args[1] = strdup((_location.getRoot() + _request.getPath()).c_str());
    args[2] = NULL;
    return args;
}

std::string CGIExecutor::execute() {




    const std::string script_path = _location.getRoot() + _request.getPath();
    if (!FileHandler::fileExists(script_path)) {
        throw std::runtime_error("CGI script not found: " + script_path);
    }
    if (!FileHandler::isExecutable(script_path)) {
        throw std::runtime_error("CGI script not executable: " + script_path);
    }

    setupEnvironment();
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

    if (pid == 0) {
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);

        execve(args[0], args, _env);
        exit(EXIT_FAILURE);
    }
    else
    {
        close(pipe_in[0]);
        close(pipe_out[1]);

        if (_request.getMethod() == "POST")
        {
            const std::string& body = _request.getBody();
            ssize_t bytes_written = write(pipe_in[1], body.c_str(), body.size());
            
            if (bytes_written == -1)
            {
                close(pipe_in[1]);
                close(pipe_out[0]);
                free_env(args);
                throw std::runtime_error("CGI write error: " + std::string(strerror(errno)));
            }
            else if (bytes_written < static_cast<ssize_t>(body.size()))
            {
                close(pipe_in[1]);
                close(pipe_out[0]);
                free_env(args);
                    std::ostringstream oss;
                        oss << "Partial CGI write: " << bytes_written 
                            << "/" << body.size() << " bytes";
                        throw std::runtime_error(oss.str());
            }
        }
        close(pipe_in[1]);
        

        char buffer[4096];
        std::string output;
        ssize_t bytes_read;
        
        while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))))
        {
            if (bytes_read == -1)
                break;
            output.append(buffer, bytes_read);
        }
        close(pipe_out[0]);

        int status;
        waitpid(pid, &status, 0);
        free_env(args);

        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            throw std::runtime_error("CGI script exited with error status");
        }

        return output;
    }
}