/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/16 12:17:23 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file webserv.hpp
 * @brief Header principale del progetto webserv
 * 
 * Questo file contiene:
 * - Tutte le librerie standard utilizzate nel progetto
 * - Forward declarations di tutte le classi
 * - Definizioni di costanti globali
 * - Include guards per evitare inclusioni multiple
 * 
 * Tutti gli altri file del progetto dovranno includere solo questo header.
 */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// ==================== LIBRERIE STANDARD C++ ====================

// I/O Stream
#include <iostream>
#include <fstream>
#include <sstream>

// Containers
#include <string>
#include <vector>
#include <map>
#include <set>

// Algorithms and utilities
#include <algorithm>
#include <stdexcept>
#include <limits>

// C-style libraries
#include <cstring>
#include <cstdlib>
#include <cctype>

// ==================== LIBRERIE SISTEMA UNIX/LINUX ====================

// Socket programming
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

// File operations
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

// Process management
#include <sys/wait.h>

// I/O Multiplexing
#include <poll.h>

// Error handling
#include <errno.h>

// Additional C libraries
#include <limits.h>
#include <stdlib.h>

// ==================== FORWARD DECLARATIONS ====================

// Core classes
class Server;
class Client;

// HTTP classes
class Request;
class Response;

// Configuration classes
class ConfigParser;
class ServerConfig;
class LocationConfig;

// CGI classes
class CGIExecutor;

// Utility classes
class FileHandler;
class StringUtils;
class MimeTypes;

// ==================== COSTANTI GLOBALI ====================

// HTTP Status Codes
#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_NO_CONTENT 204
#define HTTP_BAD_REQUEST 400
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_METHOD_NOT_ALLOWED 405
#define HTTP_REQUEST_ENTITY_TOO_LARGE 413
#define HTTP_INTERNAL_SERVER_ERROR 500
#define HTTP_NOT_IMPLEMENTED 501

// Buffer sizes
#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 1000

// Default values
#define DEFAULT_PORT 8080
#define DEFAULT_MAX_BODY_SIZE 1048576  // 1MB
#define DEFAULT_ROOT "./www"
#define DEFAULT_INDEX "index.html"

// HTTP constants
#define HTTP_VERSION "HTTP/1.1"
#define HTTP_HEADER_SEPARATOR "\r\n\r\n"
#define HTTP_LINE_SEPARATOR "\r\n"

// ==================== UTILITY FUNCTIONS ====================

// Free environment variables (for CGI)
void free_env(char** env);

// String utility functions
std::string extractBoundary(const std::string& contentType);

#endif // WEBSERV_HPP
