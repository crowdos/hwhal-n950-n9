#ifndef USB_HAL_H
#define USB_HAL_H

#include <hwhal/usb.h>
#include <thread>
#include <mutex>

class UsbHal : public Usb {
public:
  UsbHal();
  ~UsbHal();

  void addListener(std::function<void(bool)>& listener);
  bool isCableConnected();
  bool setMode(const Mode& mode);

private:
  void run();
  void update(struct udev_device *dev);
  void setCableConnected(bool connected);

  int m_sv[2];
  std::thread m_thread;
  std::mutex m_lock;
  bool m_connected = false;
  std::function<void(bool)> m_listener;
};

#endif /* USB_HAL_H */
