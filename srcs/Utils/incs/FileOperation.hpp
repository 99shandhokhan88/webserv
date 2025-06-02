#ifndef FILEOPERATION_HPP
#define FILEOPERATION_HPP

#include <string>
#include <functional>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

class FileOperation {
public:
    enum class Type {
        READ,
        WRITE,
        DELETE
    };

    enum class State {
        PENDING,
        IN_PROGRESS,
        COMPLETED,
        FAILED
    };

    FileOperation(Type type, const std::string& path, const std::string& content = "");
    ~FileOperation();

    // Getters
    int getFd() const { return fd; }
    Type getType() const { return type; }
    State getState() const { return state; }
    const std::string& getPath() const { return path; }
    const std::string& getContent() const { return content; }
    std::string& getResult() { return result; }

    // State management
    bool isCompleted() const { return state == State::COMPLETED; }
    bool hasFailed() const { return state == State::FAILED; }
    bool isPending() const { return state == State::PENDING; }

    // Operation execution
    bool start();
    bool handlePollEvent(short revents);
    void cleanup();

private:
    Type type;
    State state;
    std::string path;
    std::string content;
    std::string result;
    int fd;
    size_t bytesProcessed;
    size_t totalBytes;

    bool openFile();
    bool handleRead(short revents);
    bool handleWrite(short revents);
    bool handleDelete();
};

#endif // FILEOPERATION_HPP 