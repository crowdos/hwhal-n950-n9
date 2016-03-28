#include "lightshal.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#define MAX_BRIGHTNESS_FILE         "/sys/class/backlight/display0/max_brightness"
#define BRIGHTNESS_FILE             "/sys/class/backlight/display0/brightness"

LightsHal::LightsHal() :
  m_maxBrightness(-1) {

}

LightsHal::~LightsHal() {

}

int LightsHal::minBacklightBrightness() {
  return 0;
}

int LightsHal::maxBacklightBrightness() {
  if (m_maxBrightness == -1) {
    m_maxBrightness = readFile(MAX_BRIGHTNESS_FILE);
  }

  return m_maxBrightness;
}

int LightsHal::backlightBrightness() {
  return readFile(BRIGHTNESS_FILE);
}

void LightsHal::setBacklightBrightness(int brightness) {
  int fd = open(BRIGHTNESS_FILE, O_WRONLY);

  if (fd == -1) {
    std::cerr << "Failed to open " << BRIGHTNESS_FILE << ": " << std::strerror(errno) << std::endl;
    return;
  }

  if (write(fd, &brightness, sizeof(brightness)) <= 0) {
    std::cerr << "Failed to write to " << BRIGHTNESS_FILE << ": " << std::strerror(errno) << std::endl;
  }

  close(fd);
}

int LightsHal::readFile(const char *file) {
  int fd = open(file, O_RDONLY);

  if (fd == -1) {
    std::cerr << "Failed to open " << file << ": " << std::strerror(errno) << std::endl;
    return -1;
  }

  int val = -1;
  if (read(fd, &val, sizeof(val)) == -1) {
    std::cerr << "Failed to read from " << file << ": " << std::strerror(errno) << std::endl;
  }

  close(fd);

  return val;
}
