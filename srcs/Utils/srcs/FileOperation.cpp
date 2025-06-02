#include "FileOperation.hpp"
#include <iostream>
#include <cstring>

FileOperation::FileOperation(Type type, const std::string& path, const std::string& content)
    : type(type), state(State::PENDING), path(path), content(content), fd(-1), bytesProcessed(0), totalBytes(0) {}

FileOperation::~FileOperation() {
    cleanup();
}

bool FileOperation::start() {
    if (state != State::PENDING) {
        return false;
    }

    if (!openFile()) {
        state = State::FAILED;
        return false;
    }

    state = State::IN_PROGRESS;
    return true;
}

bool FileOperation::openFile() {
    int flags;
    mode_t mode = 0644; // Default file permissions

    switch (type) {
        case Type::READ:
            flags = O_RDONLY;
            break;
        case Type::WRITE:
            flags = O_WRONLY | O_CREAT | O_TRUNC;
            break;
        case Type::DELETE:
            return true; // No need to open file for delete
        default:
            return false;
    }

    if (type != Type::DELETE) {
        fd = open(path.c_str(), flags, mode);
        if (fd == -1) {
            std::cerr << "Failed to open file " << path << ": " << strerror(errno) << std::endl;
            return false;
        }

        // Set non-blocking mode
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            close(fd);
            fd = -1;
            return false;
        }

        // For write operations, set total bytes
        if (type == Type::WRITE) {
            totalBytes = content.size();
        }
    }

    return true;
}

bool FileOperation::handlePollEvent(short revents) {
    if (state != State::IN_PROGRESS) {
        return false;
    }

    if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
        state = State::FAILED;
        return false;
    }

    switch (type) {
        case Type::READ:
            return handleRead(revents);
        case Type::WRITE:
            return handleWrite(revents);
        case Type::DELETE:
            return handleDelete();
        default:
            state = State::FAILED;
            return false;
    }
}

bool FileOperation::handleRead(short revents) {
    if (!(revents & POLLIN)) {
        return true; // Not ready to read yet
    }

    char buffer[4096];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

    if (bytesRead < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true; // Not ready yet, try again later
        }
        state = State::FAILED;
        return false;
    }

    if (bytesRead == 0) {
        // EOF reached
        state = State::COMPLETED;
        return false;
    }

    result.append(buffer, bytesRead);
    return true;
}

bool FileOperation::handleWrite(short revents) {
    if (!(revents & POLLOUT)) {
        return true; // Not ready to write yet
    }

    if (bytesProcessed >= totalBytes) {
        state = State::COMPLETED;
        return false;
    }

    ssize_t bytesToWrite = std::min(static_cast<size_t>(4096), totalBytes - bytesProcessed);
    ssize_t bytesWritten = write(fd, content.c_str() + bytesProcessed, bytesToWrite);

    if (bytesWritten < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return true; // Not ready yet, try again later
        }
        state = State::FAILED;
        return false;
    }

    bytesProcessed += bytesWritten;
    if (bytesProcessed >= totalBytes) {
        state = State::COMPLETED;
        return false;
    }

    return true;
}

bool FileOperation::handleDelete() {
    if (unlink(path.c_str()) == -1) {
        state = State::FAILED;
        return false;
    }
    state = State::COMPLETED;
    return false;
}

void FileOperation::cleanup() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
} 