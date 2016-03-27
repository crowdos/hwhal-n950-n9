#include <hwhal/hwhal.h>
#include <hwhal/control.h>
#include "infohal.h"
#include "displayhal.h"
#include "lightshal.h"
#include <iostream>

class HAL : public HwHal {
  int version() {
    return HWHAL_VERSION_CURRENT;
  }

  bool init() {
    return true;
  }

  void destroy() {
    delete this;
  }

  Control *get(const std::string& name) {
    Control *ctl = nullptr;
    if (name == "info") {
      ctl = new InfoHal;
    } else if (name == "lights") {
      ctl = new LightsHal;
    } else if (name == "display") {
      ctl = new DisplayHal;
    } else {
      std::cerr << "Unknown hal ID " << name << std::endl;
    }

    return ctl;
  }

  void put(Control *control) {
    control->destroy();
  }
};

extern "C" {
  HwHal *__init() {
    return new HAL;
  }
}
