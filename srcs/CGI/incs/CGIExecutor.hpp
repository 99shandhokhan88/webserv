
#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP




#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>


#include "../../HTTP/incs/Request.hpp"
#include "../../Config/incs/LocationConfig.hpp"

void free_env(char** env);

class CGIExecutor {
public:
    CGIExecutor(const Request& request, const LocationConfig& location);
    ~CGIExecutor();
    void setupEnvironment();
    char** createExecArgs() const;
    std::string execute();
    std::string getScriptPath() const;

private:
    Request _request;
    LocationConfig _location;
    char** _env;
};

#endif // CGIEXECUTOR_HPP