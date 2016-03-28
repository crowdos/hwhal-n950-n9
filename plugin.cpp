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

  Control *get(const ControlId& id) {
    Control *ctl = nullptr;

    switch (id) {
    case ControlId::Info:
      ctl = new InfoHal;
      break;

    case ControlId::Lights:
      ctl = new LightsHal;
      break;

    case ControlId::Display:
      ctl = new DisplayHal;
      break;

    default:
      std::cerr << "Unknown hal ID "
		<< static_cast<std::underlying_type<ControlId>::type>(id)
		<< std::endl;
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
