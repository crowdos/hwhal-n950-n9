#include "displayhal.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <iostream>
#include <cstring>

#define FB_BLANK_PATH  "/sys/class/graphics/fb0/blank"

DisplayHal::DisplayHal() {
  m_fd = open(FB_BLANK_PATH, O_RDWR);
  if (m_fd == -1) {
    std::cerr << "Failed to open " << FB_BLANK_PATH << ": " << std::strerror(errno) << std::endl;
  }
}

DisplayHal::~DisplayHal() {
  if (m_fd != -1) {
    close(m_fd);
  }
}

void DisplayHal::blank(bool blank) {
  int val = blank ? FB_BLANK_POWERDOWN : FB_BLANK_UNBLANK;

  if (m_fd == -1) {
    return;
  }

  if (write(m_fd, &val, sizeof(val)) != sizeof(val)) {
    std::cerr << "Failed to write to " << FB_BLANK_PATH << ": " << std::strerror(errno) << std::endl;
  }
}

bool DisplayHal::isBlank() {
  if (m_fd == -1) {
    return true;
  }

  int val;
  if (read(m_fd, &val, sizeof(val)) != sizeof(val)) {
    std::cerr << "Failed to read from " << FB_BLANK_PATH << ": " << std::strerror(errno) << std::endl;
    return true;
  }

  return val != FB_BLANK_UNBLANK;
}
