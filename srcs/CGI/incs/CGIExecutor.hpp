#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include "../../../incs/webserv.hpp"
#include "../../HTTP/incs/Request.hpp"
#include "../../Config/incs/LocationConfig.hpp"

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