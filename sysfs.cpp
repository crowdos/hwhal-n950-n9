#include "sysfs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

Sysfs::Sysfs() :
  m_fd(-1) {

}

Sysfs::~Sysfs() {
  close();
}

bool Sysfs::open(const char *path, bool rw) {
  if (m_fd != -1) {
    std::cerr << "An existing file has been opened" << std::endl;
    return false;
  }

  m_fd = ::open(path, rw ? O_RDWR : O_RDONLY);
  if (m_fd == -1) {
    std::cerr << "Failed to open " << path << ": " << std::strerror(errno) << std::endl;
  }

  return m_fd != -1;
}

void Sysfs::close() {
  if (m_fd != -1) {
    ::close(m_fd);
    m_fd = -1;
  }
}

bool Sysfs::read(std::string& str) {
  if (m_fd == -1) {
    return false;
  }

  if (lseek(m_fd, 0, SEEK_SET) != 0) {
    std::cerr << "Failed to seek: " << std::strerror(errno) << std::endl;
    return false;
  }

  char c;

  ssize_t rc = 0;
  do {
    rc = ::read(m_fd, &c, sizeof(c));

    if (rc == -1) {
      std::cerr << "Failed to read: " << std::strerror(errno) << std::endl;
      return false;
    }

    if (rc == 0) {
      break;
    }

    str += c;
  } while (rc > -1);

  return true;
}

bool Sysfs::write(const std::string& str) {
  if (m_fd == -1) {
    return false;
  }

  size_t s = ::write(m_fd, str.c_str(), str.size());
  if (s != str.size()) {
    std::cerr << "Failed to write: " << std::strerror(errno) << std::endl;
    return false;
  }

  /*
  if (fsync(m_fd) != 0) {
    std::cerr << "Failed to sync: " << std::strerror(errno) << std::endl;
  }
  */

  return true;
}

