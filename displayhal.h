#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include <hwhal/display.h>
#include "sysfs.h"

class DisplayHal : public Display {
public:
  DisplayHal();
  ~DisplayHal();

  void blank(bool blank);
  bool isBlank();

private:
  Sysfs m_fd;
  bool m_blank;
};

#endif /* DISPLAY_HAL_H */
