/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIExecutor.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 11:42:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/27 19:48:47 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/CGIExecutor.hpp"

// Constructor with request and location
CGIExecutor::CGIExecutor(const Request& req, const LocationConfig& loc) : 
    _request(req), _location(loc) {}

CGIExecutor::~CGIExecutor() {}

std::string CGIExecutor::execute() {
    int pipe_in[2];
    int pipe_out[2];
    pid_t pid;

    // Create pipes with proper error checking
    if (pipe(pipe_in) == -1) {
        throw CGIException("Pipe creation failed");
    }
    if (pipe(pipe_out) == -1) {
        close(pipe_in[0]);
        close(pipe_in[1]);
        throw CGIException("Pipe creation failed");
    }

    pid = fork();
    if (pid == -1) {
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        throw CGIException("Fork failed");
    }

    if (pid == 0) { // Child process
        close(pipe_in[1]);
        close(pipe_out[0]);

        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);

        // Set environment variables
        setenv("REQUEST_METHOD", _request.getMethod().c_str(), 1);
        setenv("QUERY_STRING", _request.getQueryString().c_str(), 1);
        
        
        
        // C++98-compatible content length conversion
        std::ostringstream oss;
        oss << _request.getBody().size();
        setenv("CONTENT_LENGTH", oss.str().c_str(), 1);

        // Execute script
        execl(_location.getCgiPath().c_str(),
              _location.getCgiPath().c_str(),
              (char*)NULL);

        std::cerr << "execl failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Write request body
        ssize_t bytes_written = write(pipe_in[1], 
                                    _request.getBody().c_str(),
                                    _request.getBody().size());
        if (bytes_written == -1) {
            close(pipe_in[1]);
            close(pipe_out[0]);
            throw CGIException("Write to pipe failed");
        }
        close(pipe_in[1]);

        // Read output
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
            _output.append(buffer, bytes_read);
        }
        close(pipe_out[0]);

        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw CGIException("CGI script exited with error");
        }
    }
    return _output;
}
