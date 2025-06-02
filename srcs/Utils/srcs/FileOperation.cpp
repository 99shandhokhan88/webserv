
#include "../../../incs/webserv.hpp"


#include "FileOperation.hpp"


FileOperation::FileOperation(FileOperationType type, const std::string& path, const std::string& content)
    : type(type), state(FILE_OP_PENDING), path(path), content(content), fd(-1), bytesProcessed(0), totalBytes(0) {}

FileOperation::~FileOperation() {
    cleanup();
}

bool FileOperation::isCompleted() const { return state == FILE_OP_COMPLETED; }
bool FileOperation::hasFailed() const { return state == FILE_OP_FAILED; }
bool FileOperation::isPending() const { return state == FILE_OP_PENDING; }
const std::string& FileOperation::getResult() const { return result; }

bool FileOperation::start() {
    if (state != FILE_OP_PENDING) {
        return false;
    }

    if (!openFile()) {
        state = FILE_OP_FAILED;
        return false;
    }

    state = FILE_OP_IN_PROGRESS;
    return true;
}

bool FileOperation::openFile() {
    int flags;
    mode_t mode = 0644;

    switch (type) {
        case FILE_OP_READ:
            flags = O_RDONLY;
            break;
        case FILE_OP_WRITE:
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            break;
        case FILE_OP_DELETE:
            return true;
        default:
            return false;
    }

    if (type != FILE_OP_DELETE) {
        fd = open(path.c_str(), flags, mode);
        if (fd == -1) {
            std::cerr << "Failed to open file " << path << ": " << strerror(errno) << std::endl;
            return false;
        }

        int current_flags = fcntl(fd, F_GETFL, 0);
        if (current_flags == -1 || fcntl(fd, F_SETFL, current_flags | O_NONBLOCK) == -1) {
            close(fd);
            fd = -1;
            return false;
        }

        if (type == FILE_OP_WRITE) {
            totalBytes = content.size();
        }
    }

    return true;
}

bool FileOperation::handlePollEvent(short revents) {
    if (state != FILE_OP_IN_PROGRESS) {
        return false;
    }

    if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
        state = FILE_OP_FAILED;
        return false;
    }

    switch (type) {
        case FILE_OP_READ:
            return handleRead(revents);
        case FILE_OP_WRITE:
            return handleWrite(revents);
        case FILE_OP_DELETE:
            return handleDelete();
        default:
            state = FILE_OP_FAILED;
            return false;
    }
}

bool FileOperation::handleRead(short revents) {
    if (!(revents & POLLIN)) {
        return true;
    }

    char buffer[4096];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

    if (bytesRead < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true;
        }
        state = FILE_OP_FAILED;
        return false;
    }

    if (bytesRead == 0) {
        state = FILE_OP_COMPLETED;
        return false;
    }

    result.append(buffer, bytesRead);
    return true;
}

bool FileOperation::handleWrite(short revents) {
    if (!(revents & POLLOUT)) {
        return true;
    }

    if (bytesProcessed >= totalBytes) {
        state = FILE_OP_COMPLETED;
        return false;
    }

    ssize_t bytesToWrite = std::min(static_cast<size_t>(4096), totalBytes - bytesProcessed);
    ssize_t bytesWritten = write(fd, content.c_str() + bytesProcessed, bytesToWrite);

    if (bytesWritten < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true;
        }
        state = FILE_OP_FAILED;
        return false;
    }

    bytesProcessed += bytesWritten;
    if (bytesProcessed >= totalBytes) {
        state = FILE_OP_COMPLETED;
        return false;
    }

    return true;
}

bool FileOperation::handleDelete() {
    if (unlink(path.c_str()) == -1) {
        state = FILE_OP_FAILED;
        return false;
    }
    state = FILE_OP_COMPLETED;
    return false;
}

void FileOperation::cleanup() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}