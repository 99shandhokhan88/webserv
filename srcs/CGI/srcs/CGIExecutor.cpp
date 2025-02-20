/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIExecutor.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/13 11:42:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/19 17:36:18 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "CGIExecutor.hpp"
#include "../../Config/incs/ServerConfig.hpp"
#include "../../HTTP/incs/Request.hpp"

// Constructor
CGIExecutor::CGIExecutor() {}

// Destructor
CGIExecutor::~CGIExecutor() {}

// Execute a CGI script
std::string CGIExecutor::execute(
    const std::string& script_path,
    const std::map<std::string, std::string>& env_vars,
    const std::string& request_body
) {
    setupEnvironment(env_vars);
    return executeScript(script_path, request_body);
}

// Set up environment variables for the CGI script
void CGIExecutor::setupEnvironment(const std::map<std::string, std::string>& env_vars) {
    clearenv();
    for (std::map<std::string, std::string>::const_iterator it = env_vars.begin(); it != env_vars.end(); ++it) {
        setenv(it->first.c_str(), it->second.c_str(), 1);
    }
}

// Execute the CGI script and capture its output
std::string CGIExecutor::executeScript(const std::string& script_path, const std::string& request_body) {
    int pipe_in[2];  // Pipe for sending data to the CGI script
    int pipe_out[2]; // Pipe for receiving data from the CGI script

    // Create pipes
    if (pipe(pipe_in) || pipe(pipe_out)) {
        throw std::runtime_error("Error: failed to create pipes for the CGI!");
    }

    // Fork the process
    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Error: failed to fork the process for the CGI!");
    }

    if (pid == 0) {
        // Child process (CGI script)

        // Redirect stdin and stdout
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);

        // Close unused pipe ends
        close(pipe_in[1]);
        close(pipe_out[0]);

        // Execute the CGI script
        execl(script_path.c_str(), script_path.c_str(), NULL);

        // If execl fails
        std::cerr << "Failed to execute CGI script: " << script_path << std::endl;
        exit(1);
    } else {
        // Parent process (server)

        // Close unused pipe ends
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Write request body to CGI script
        writeToPipe(pipe_in, request_body);

        // Close write end of pipe_in
        close(pipe_in[1]);

        // Read output from CGI script
        std::string output = readFromFd(pipe_out[0]);

        // Close read end of pipe_out
        close(pipe_out[0]);

        // Wait for child process to finish
        int status;
        waitpid(pid, &status, 0);

        // Check if CGI script exited successfully
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            throw std::runtime_error("Error: the CGI script exited with an error!");
        }

        return output;
    }
}

// Helper function to read from a file descriptor
std::string CGIExecutor::readFromFd(int fd) {
    char buffer[4096];
    std::string output;

    while (true) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) break;
        output.append(buffer, bytes_read);
    }

    return output;
}

// Write request body to pipe
void CGIExecutor::writeToPipe(int pipe_in[2], const std::string& request_body) {
    ssize_t bytes_written = write(pipe_in[1], request_body.c_str(), request_body.size());

    if (bytes_written == -1) {
        std::cerr << "Error writing to pipe: " << strerror(errno) << std::endl;
        close(pipe_in[1]);
        throw std::runtime_error("Failed to write to CGI input pipe");
    }

    if (bytes_written < static_cast<ssize_t>(request_body.size())) {
        std::cerr << "Warning: Partial write to pipe" << std::endl;
    }
}
