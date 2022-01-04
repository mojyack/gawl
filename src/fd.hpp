#pragma once
#include <cstdint>
#include <optional>
#include <vector>

#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

class FileDescriptor {
  private:
    int fd = -1;

    auto close_fd() -> void {
        if(fd != -1) {
            close(fd);
        }
    }

  public:
    auto read(void* const data, const size_t size) const -> bool {
        size_t len = 0;
        while(len < size) {
            const auto n = ::read(fd, (uint8_t*)data + len, size - len);
            if(n == -1) {
                return false;
            }
            len += n;
        }
        return true;
    }
    auto read(const size_t size) const -> std::optional<std::vector<uint8_t>> {
        auto r = std::vector<uint8_t>(size);
        return read(r.data(), size) ? std::make_optional(r) : std::nullopt;
    }
    template <class T>
    auto read() const -> std::optional<T> {
        auto r = T();
        return read(&r, sizeof(T)) ? std::make_optional(r) : std::nullopt;
    }
    auto read_sized() const -> std::optional<std::vector<uint8_t>> {
        const auto size = read<size_t>();
        if(!size.has_value()) {
            return std::nullopt;
        }
        auto r = read(*size);
        if(!r.has_value()) {
            return std::nullopt;
        }
        return r;
    }
    template <class T>
    auto write(const T data) const -> bool {
        const size_t size = sizeof(T);
        return write(&data, size);
    }
    auto write(const void* data, const size_t size) const -> bool {
        size_t wrote = 0;
        while(wrote != size) {
            const auto r = ::write(fd, data, size);
            if(r == -1) {
                return false;
            }
            wrote += r;
        }
        return true;
    }
    auto forget() -> void {
        fd = -1;
    }
    operator int() const {
        return fd;
    }
    auto operator=(FileDescriptor& o) -> FileDescriptor& {
        return *this = std::move(o);
    }
    auto operator=(FileDescriptor&& o) -> FileDescriptor& {
        close_fd();
        fd = o.fd;
        o.forget();
        return *this;
    }
    FileDescriptor() : fd(-1) {}
    FileDescriptor(const int fd) : fd(fd) {}
    FileDescriptor(FileDescriptor& o) {
        *this = o;
    }
    FileDescriptor(FileDescriptor&& o) {
        *this = o;
    }
    ~FileDescriptor() {
        close_fd();
    }
};

class EventFileDescriptor {
  private:
    const FileDescriptor fd;

  public:
    auto notify() const -> void {
        fd.write((uint64_t)1);
    }
    auto consume() const -> uint64_t {
        const auto v = fd.read<uint64_t>();
        return v.has_value() ? *v : 0;
    }
    auto wait() const -> void {
        auto pfd = pollfd{fd, POLLIN, 0};
        while(true) {
            poll(&pfd, 1, -1);
            if(pfd.revents & POLLIN) {
                consume();
                return;
            }
        }
        return;
    }
    operator int() const {
        return fd;
    }
    EventFileDescriptor() : fd(eventfd(0, 0)) {}
};
