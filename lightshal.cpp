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
  m_fd.open(BRIGHTNESS_FILE);
}

LightsHal::~LightsHal() {

}

int LightsHal::minBacklightBrightness() {
  return 0;
}

int LightsHal::maxBacklightBrightness() {
  if (m_maxBrightness == -1) {
    Sysfs fd;
    if (fd.open(MAX_BRIGHTNESS_FILE, false)) {
      std::string val;
      if (fd.read(val)) {
	try {
	  m_maxBrightness = std::stoi(val);
	} catch (...) {
	  // Nothing :/
	}
      }
    }
  }

  return m_maxBrightness;
}

int LightsHal::backlightBrightness() {
  std::string val;
  if (!m_fd.read(val)) {
    return -1;
  }

  try {
    return std::stoi(val);
  } catch (...) {
    // Nothing :/
    return -1;
  }
}

void LightsHal::setBacklightBrightness(int brightness) {
  m_fd.write(std::to_string(brightness));
}
