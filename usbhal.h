#ifndef USB_HAL_H
#define USB_HAL_H

#include <hwhal/usb.h>

class UDev;
class LoopIntegration;

class UsbHal : public Usb {
public:
  UsbHal(LoopIntegration *loop);
  ~UsbHal();

  void addListener(const std::function<void(bool)>& listener);
  bool isCableConnected();
  bool setMode(const Mode& mode);

private:
  void setup();
  void setCableConnected(bool connected);

  UDev *m_udev;
  LoopIntegration *m_loop;
  bool m_connected;
  std::function<void(bool)> m_listener;
  uint64_t m_fd;
  uint64_t m_timer;
};

#endif /* USB_HAL_H */
