#ifndef USB_HAL_H
#define USB_HAL_H

#include <hwhal/usb.h>
#include <hwhal/loopintegration.h>

class UDev;

class UsbHal : public Usb {
public:
  UsbHal(LoopIntegration *loop);
  ~UsbHal();

  void addListener(std::function<void(bool)>& listener);
  bool isCableConnected();
  bool setMode(const Mode& mode);

private:
  void setup();
  void setCableConnected(bool connected);

  UDev *m_udev;
  LoopIntegration *m_loop;
  bool m_connected;
  std::function<void(bool)> m_listener;
  std::unique_ptr<LoopIntegration::FdWatcher> m_fd;
  std::unique_ptr<LoopIntegration::Timer> m_timer;
};

#endif /* USB_HAL_H */
