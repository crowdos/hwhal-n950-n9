#ifndef LIGHTS_HAL_H
#define LIGHTS_HAL_H

#include <hwhal/lights.h>
#include "sysfs.h"

class LightsHal : public Lights {
public:
  LightsHal();
  ~LightsHal();

  int minBacklightBrightness();
  int maxBacklightBrightness();
  int backlightBrightness();
  void setBacklightBrightness(int brightness);

private:
  Sysfs m_fd;
  int m_maxBrightness;
};

#endif /* LIGHTS_HAL_H */
