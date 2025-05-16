#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/utsname.h>
#include <fstream>
#include <cstring>
#include <sys/sysinfo.h>
#include <limits>

// HTML special characters escaping
std::string htmlspecialchars(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}

// Get current time formatted
std::string getCurrentTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    std::stringstream ss;
    ss << (ltm->tm_year + 1900) << "-"
       << (ltm->tm_mon + 1) << "-"
       << ltm->tm_mday << " "
       << (ltm->tm_hour < 10 ? "0" : "") << ltm->tm_hour << ":"
       << (ltm->tm_min < 10 ? "0" : "") << ltm->tm_min << ":"
       << (ltm->tm_sec < 10 ? "0" : "") << ltm->tm_sec;
    return ss.str();
}

// Parse query string parameters
std::map<std::string, std::string> parseQueryString(const std::string& query) {
    std::map<std::string, std::string> data;
    std::string key, value;
    std::stringstream ss(query);
    
    while (std::getline(ss, key, '&')) {
        size_t pos = key.find('=');
        if (pos != std::string::npos) {
            value = key.substr(pos + 1);
            key = key.substr(0, pos);
            // URL decode the values
            std::string decoded_value;
            for (size_t i = 0; i < value.length(); ++i) {
                if (value[i] == '%' && i + 2 < value.length()) {
                    int hex_val;
                    std::stringstream hex_stream;
                    hex_stream << std::hex << value.substr(i + 1, 2);
                    hex_stream >> hex_val;
                    decoded_value += static_cast<char>(hex_val);
                    i += 2;
                } else if (value[i] == '+') {
                    decoded_value += ' ';
                } else {
                    decoded_value += value[i];
                }
            }
            data[key] = decoded_value;
        }
    }
    return data;
}

// Get system memory information
std::map<std::string, unsigned long> getSystemMemInfo() {
    struct sysinfo si;
    std::map<std::string, unsigned long> memInfo;
    
    if (sysinfo(&si) == 0) {
        memInfo["total_ram"] = si.totalram * si.mem_unit / (1024 * 1024);
        memInfo["free_ram"] = si.freeram * si.mem_unit / (1024 * 1024);
        memInfo["used_ram"] = (si.totalram - si.freeram) * si.mem_unit / (1024 * 1024);
    }
    
    return memInfo;
}

// Read POST data
std::string readPostData() {
    std::string data;
    if (const char* length_str = getenv("CONTENT_LENGTH")) {
        size_t length = std::stoul(length_str);
        if (length > 0 && length < 1000000) { // Limit to 1MB
            char* buffer = new char[length + 1];
            std::cin.read(buffer, length);
            buffer[length] = 0;
            data = buffer;
            delete[] buffer;
        }
    }
    return data;
}

int main() {
    std::cout << "Content-type: text/html\r\n\r\n";
    
    // Get environment variables
    std::map<std::string, std::string> env;
    const char* envVars[] = {
        "REQUEST_METHOD", "QUERY_STRING", "CONTENT_TYPE", "CONTENT_LENGTH",
        "SERVER_SOFTWARE", "SERVER_NAME", "SERVER_PROTOCOL", "SERVER_PORT",
        "HTTP_USER_AGENT", "REMOTE_ADDR", "PATH_INFO", "SCRIPT_NAME"
    };
    
    for (const char* var : envVars) {
        env[var] = getenv(var) ? getenv(var) : "";
    }
    
    // Parse query parameters
    std::map<std::string, std::string> queryParams;
    if (!env["QUERY_STRING"].empty()) {
        queryParams = parseQueryString(env["QUERY_STRING"]);
    }
    
    // Handle POST data
    std::string postData;
    if (env["REQUEST_METHOD"] == "POST") {
        postData = readPostData();
        if (env["CONTENT_TYPE"].find("application/x-www-form-urlencoded") != std::string::npos) {
            auto postParams = parseQueryString(postData);
            queryParams.insert(postParams.begin(), postParams.end());
        }
    }
    
    // Get system information
    struct utsname systemInfo;
    uname(&systemInfo);
    auto memInfo = getSystemMemInfo();
    
    // Start HTML output
    std::cout << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>C++ CGI Test Interface</title>
    <style>
        :root {
            --primary-color: #00BCD4;
            --secondary-color: #607D8B;
            --success-color: #4CAF50;
            --error-color: #f44336;
            --warning-color: #ff9800;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            margin: 0;
            padding: 20px;
            background: #f0f2f5;
            color: #333;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        header {
            background: var(--primary-color);
            color: white;
            padding: 20px;
            border-radius: 8px;
            margin-bottom: 20px;
            text-align: center;
            position: relative;
        }
        
        .home-button {
            position: absolute;
            top: 20px;
            right: 20px;
            background: white;
            color: var(--primary-color);
            padding: 8px 16px;
            border-radius: 4px;
            text-decoration: none;
            font-weight: bold;
            transition: opacity 0.2s;
        }
        
        .home-button:hover {
            opacity: 0.9;
        }
        
        .card {
            background: white;
            border-radius: 8px;
            padding: 20px;
            margin: 10px 0;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            border-left: 4px solid var(--primary-color);
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        
        .stat-box {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
            border: 1px solid #dee2e6;
        }
        
        .stat-value {
            font-size: 24px;
            font-weight: bold;
            color: var(--primary-color);
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 10px 0;
        }
        
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #dee2e6;
        }
        
        th {
            background: #f8f9fa;
            font-weight: 600;
        }
        
        pre {
            background: #2d2d2d;
            color: #fff;
            padding: 15px;
            border-radius: 4px;
            overflow-x: auto;
        }
        
        .badge {
            display: inline-block;
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 12px;
            font-weight: 600;
        }
        
        .badge-success { background: var(--success-color); color: white; }
        .badge-error { background: var(--error-color); color: white; }
        .badge-warning { background: var(--warning-color); color: white; }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>C++ CGI Test Interface</h1>
            <p>System Time: )" << getCurrentTime() << R"(</p>
            <a href="/cgi-bin/index.html" class="home-button">Return to Home</a>
        </header>

        <div class="card">
            <h2>System Information</h2>
            <div class="info-grid">
                <div class="stat-box">
                    <h3>Operating System</h3>
                    <div class="stat-value">)" << systemInfo.sysname << " " << systemInfo.release << R"(</div>
                </div>
                <div class="stat-box">
                    <h3>Machine Architecture</h3>
                    <div class="stat-value">)" << systemInfo.machine << R"(</div>
                </div>
                <div class="stat-box">
                    <h3>Memory Usage</h3>
                    <div class="stat-value">)" << memInfo["used_ram"] << "MB / " << memInfo["total_ram"] << R"(MB</div>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>Request Information</h2>
            <table>
                <tr>
                    <th>Parameter</th>
                    <th>Value</th>
                </tr>
                <tr>
                    <td>Request Method</td>
                    <td><span class="badge badge-success">)" << env["REQUEST_METHOD"] << R"(</span></td>
                </tr>
                <tr>
                    <td>Server Software</td>
                    <td>)" << env["SERVER_SOFTWARE"] << R"(</td>
                </tr>
                <tr>
                    <td>Server Protocol</td>
                    <td>)" << env["SERVER_PROTOCOL"] << R"(</td>
                </tr>
                <tr>
                    <td>Remote Address</td>
                    <td>)" << env["REMOTE_ADDR"] << R"(</td>
                </tr>
            </table>
        </div>

        <div class="card">
            <h2>Query Parameters</h2>)";
    
    if (!queryParams.empty()) {
        std::cout << R"(
            <table>
                <tr>
                    <th>Parameter</th>
                    <th>Value</th>
                </tr>)";
        
        for (const auto& param : queryParams) {
            std::cout << R"(
                <tr>
                    <td>)" << htmlspecialchars(param.first) << R"(</td>
                    <td>)" << htmlspecialchars(param.second) << R"(</td>
                </tr>)";
        }
        
        std::cout << R"(
            </table>)";
    } else {
        std::cout << R"(
            <p>No query parameters provided.</p>)";
    }
    
    std::cout << R"(
        </div>

        <div class="card">
            <h2>Test Form</h2>
            <form method="POST" action="/cgi-bin/test.cgi" enctype="multipart/form-data">
                <div>
                    <label for="name">Name:</label>
                    <input type="text" id="name" name="name" placeholder="Enter your name">
                </div>
                <div>
                    <label for="message">Message:</label>
                    <input type="text" id="message" name="message" placeholder="Enter a message">
                </div>
                <div>
                    <input type="submit" value="Submit">
                </div>
            </form>
        </div>

        <footer>
            <p>C++ CGI Test Interface - Compiled at )" << __DATE__ << " " << __TIME__ << R"(</p>
        </footer>
    </div>
</body>
</html>)";

    return 0;
} 