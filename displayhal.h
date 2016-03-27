#ifndef DISPLAY_HAL_H
#define DISPLAY_HAL_H

#include <hwhal/display.h>

class DisplayHal : public Display {
public:
  DisplayHal();
  ~DisplayHal();

  void blank(bool blank);
  bool isBlank();

private:
  int m_fd;
};

#endif /* DISPLAY_HAL_H */
