#include "usbhal.h"
#include <iostream>
#include <libudev.h>

#define SYSFS_FILE "/sys/class/power_supply/usb/uevent"

class UDev {
public:
  UDev() :
    m_fd(-1),
    m_udev(nullptr),
    m_mon(nullptr) {

    m_udev = udev_new();
    if (!m_udev) {
      err("Failed to initialize udev");
      return;
    }

    m_mon = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_mon) {
      err("Failed to initialize udev monitor");
      return;
    }

    if (udev_monitor_filter_add_match_subsystem_devtype(m_mon, "power_supply", NULL) < 0) {
      err("Failed to add udev monitor match");
      return;
    }

    if (udev_monitor_enable_receiving(m_mon) < 0) {
      err("Failed to enable udev monitor");
      return;
    }

    int fd = udev_monitor_get_fd(m_mon);
    if (fd < 0) {
      err("Failed to get udev monitor fd");
      return;
    }

    m_fd = fd;
  }

  ~UDev() {
    if (m_mon) {
      udev_monitor_unref(m_mon);
      m_mon = nullptr;
    }

    if (m_udev) {
      udev_unref(m_udev);
      m_udev = nullptr;
    }
  }

  void enumerate(const std::function<void(bool)>& cb) {
    if (!m_udev) {
      return;
    }

    udev_enumerate *enumerate = udev_enumerate_new(m_udev);
    udev_enumerate_add_match_subsystem(enumerate, "power_supply");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    if (devices) {
      struct udev_list_entry *dev_list_entry;
      udev_list_entry_foreach(dev_list_entry, devices) {
	const char *path = udev_list_entry_get_name(dev_list_entry);
	struct udev_device *dev = udev_device_new_from_syspath(m_udev, path);
	const char *sysname = udev_device_get_sysname(dev);
	const char *action = udev_device_get_action(dev);

	if (sysname && sysname == std::string("usb") &&
	    action && action == std::string("change")) {
	  check(dev, cb);
	}

	udev_device_unref(dev);
      }
    }

    udev_enumerate_unref(enumerate);
  }

  void tick(const std::function<void(bool)>& cb) {
    struct udev_device *dev = udev_monitor_receive_device(m_mon);
    if (!dev) {
      return;
    }

    if (udev_device_get_sysname(dev) != std::string("usb")) {
      udev_device_unref(dev);
      return;
    }

    if (udev_device_get_action(dev) != std::string("change")) {
      udev_device_unref(dev);
      return;
    }

    check(dev, cb);
    udev_device_unref(dev);
  }

  int fd() {
    return m_fd;
  }

private:
  void err(const std::string& msg) {
    std::cerr << msg << std::endl;
  }

  void check(struct udev_device *dev, const std::function<void(bool)>& cb) {
    const char *power = udev_device_get_property_value(dev, "POWER_SUPPLY_PRESENT");
    if (!power) {
      power = udev_device_get_property_value(dev, "POWER_SUPPLY_ONLINE");
    }

    if (!power) {
      std::cerr << "Cannot find power supply indicator" << std::endl;
      cb(false);
      return;
    }

    if (power != std::string("1")) {
      // Disconnected.
      cb(false);
      return;
    }

    // We might have a charger or a cable:
    power = udev_device_get_property_value(dev, "POWER_SUPPLY_TYPE");
    if (!power) {
      // OK. We cannot tell. Just assume we have nothing connected :/
      cb(false);
      return;
    }

    if (power == std::string("USB") || power == std::string("USB_CDP")) {
      // We have a cable :-)
      cb(true);
    } else if (power == std::string("USB_DCP")) {
      // This is a charger:
      cb(false);
    } else {
      // We have no idea what that is
      cb(false);
      std::cerr << "I don't know what is " << power << std::endl;
    }
  }

  int m_fd;
  struct udev *m_udev;
  struct udev_monitor *m_mon;
};

UsbHal::UsbHal(LoopIntegration *loop) :
  m_udev(nullptr),
  m_loop(loop),
  m_connected(false) {

  setup();
}

UsbHal::~UsbHal() {
  if (m_fd) {
    m_fd->stop();
  }

  if (m_timer) {
    m_timer->stop();
  }

  delete m_udev;
  m_udev = nullptr;
}

void UsbHal::addListener(const std::function<void(bool)>& listener) {
  m_listener = listener;
}

bool UsbHal::isCableConnected() {
  return m_connected;
}

bool UsbHal::setMode(const Mode& mode) {
  // TODO: we ignore mode for now
#if 0
  int ret = system("/sbin/modprobe -q g_ether");
  if (ret == -1) {
    return false;
  }

  if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
    return true;
  }

  return false;
#endif

  return false;
}

void UsbHal::setCableConnected(bool connected) {
  if (m_connected != connected) {
    m_connected = connected;

    if (m_listener) {
      m_listener(m_connected);
    }
  }
}

void UsbHal::setup() {
  m_timer.reset();

  m_udev = new UDev;

  int fd = m_udev->fd();
  if (fd < 0) {
    throw std::runtime_error("Failed to initialize udev");
  }

  m_udev->enumerate([this](bool connected) {setCableConnected(connected); });

  m_fd = m_loop->addFileDescriptor([this](bool ok) {
      if (!ok) {
	delete m_udev;
	m_udev = nullptr;
	m_fd.reset(nullptr);
	m_timer = m_loop->post([this](){setup();}, 1000 /* ms */);
	return;
      }

      m_udev->tick([this](bool connected) {setCableConnected(connected); });
    }, fd);
}
