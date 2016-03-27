#ifndef LIGHTS_HAL_H
#define LIGHTS_HAL_H

#include <hwhal/lights.h>

class LightsHal : public Lights {
public:
  LightsHal();
  ~LightsHal();

  int minBacklightBrightness();
  int maxBacklightBrightness();
  int backlightBrightness();
  void setBacklightBrightness(int brightness);

private:
  int readFile(const char *file);

  int m_maxBrightness;
};

#endif /* LIGHTS_HAL_H */
