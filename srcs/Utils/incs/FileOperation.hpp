#ifndef FILEOPERATION_HPP
#define FILEOPERATION_HPP

#include "../../../incs/webserv.hpp"



enum FileOperationType {
    FILE_OP_READ,
    FILE_OP_WRITE,
    FILE_OP_DELETE
};

enum FileOperationState {
    FILE_OP_PENDING,
    FILE_OP_IN_PROGRESS,
    FILE_OP_COMPLETED,
    FILE_OP_FAILED
};

class FileOperation {
public:
    FileOperation(FileOperationType type, const std::string& path, const std::string& content = "");
    ~FileOperation();

    bool start();
    bool handlePollEvent(short revents);
    bool isCompleted() const;
    bool hasFailed() const;
    bool isPending() const;
    const std::string& getResult() const;
    int getFd() const { return fd; }
    FileOperationType getType() const { return type; }
    void cleanup();

private:
    FileOperationType type;
    FileOperationState state;
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

#endif