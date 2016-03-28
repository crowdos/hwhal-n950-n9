#include "displayhal.h"
#include <linux/fb.h>
#include <iostream>

#define FB_BLANK_PATH  "/sys/class/graphics/fb0/blank"

DisplayHal::DisplayHal() :
  m_blank(true) {

  m_fd.open(FB_BLANK_PATH);
}

DisplayHal::~DisplayHal() {
  m_fd.close();
}

void DisplayHal::blank(bool blank) {
  std::string val = blank ? std::to_string(FB_BLANK_POWERDOWN) : std::to_string(FB_BLANK_UNBLANK);

  if (m_fd.write(val)) {
    m_blank = blank;
  }
}

bool DisplayHal::isBlank() {
  return m_blank;
}
