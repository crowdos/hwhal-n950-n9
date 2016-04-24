#include "usbhal.h"
#include <iostream>
#include <libudev.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define SYSFS_FILE "/sys/class/power_supply/usb/uevent"

UsbHal::UsbHal() {
  if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, m_sv) == -1) {
    throw std::runtime_error("Failed to create sockets");
  }

  try {
    m_thread = std::thread(&UsbHal::run, this);
  } catch (std::exception& ex) {
    std::cerr << "Failed to start USB monitoring thread: " << ex.what() << std::endl;
    throw;
  }
}

UsbHal::~UsbHal() {
  char c = 'c';
  write(m_sv[0], &c, 1);

  try {
    m_thread.join();
  } catch (...) {
    // Nothing
  }

  close(m_sv[0]);
  close(m_sv[1]);
}

void UsbHal::addListener(std::function<void(bool)>& listener) {
  std::lock_guard<std::mutex> lock(m_lock);
  m_listener = listener;
}

bool UsbHal::isCableConnected() {
  std::lock_guard<std::mutex> lock(m_lock);
  return m_connected;
}

bool UsbHal::setMode(const Mode& mode) {
  // TODO: we ignore mode for now
  int ret = system("/sbin/modprobe -q g_ether");
  if (ret == -1) {
    return false;
  }

  if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
    return true;
  }

  return false;
}

void UsbHal::run() {
  struct udev *udev = udev_new();
  if(!udev) {
    throw std::runtime_error("Failed to initialize udev");
    return;
  }

  struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");
  if (!mon) {
    udev_unref(udev);
    throw std::runtime_error("Failed to initialize udev monitor");
    return;
  }

  if (udev_monitor_filter_add_match_subsystem_devtype(mon, "power_supply", NULL) < 0) {
    udev_monitor_unref(mon);
    udev_unref(udev);
    throw std::runtime_error("Failed to add udev monitor match");
    return;
  }

  if (udev_monitor_enable_receiving(mon) < 0) {
    udev_monitor_unref(mon);
    udev_unref(udev);
    throw std::runtime_error("Failed to enable udev monitor");
    return;
  }

  int fd = udev_monitor_get_fd(mon);
  if (fd < 0) {
    udev_monitor_unref(mon);
    udev_unref(udev);
    throw std::runtime_error("Failed to get udev monitor fd");
    return;
  }

  // Enumerate:
  udev_enumerate *enumerate = udev_enumerate_new(udev);
  udev_enumerate_add_match_subsystem(enumerate, "power_supply");
  udev_enumerate_scan_devices(enumerate);
  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
  if (devices) {
    struct udev_list_entry *dev_list_entry;
    udev_list_entry_foreach(dev_list_entry, devices) {
      const char *path = udev_list_entry_get_name(dev_list_entry);
      struct udev_device *dev = udev_device_new_from_syspath(udev, path);

      if (udev_device_get_sysname(dev) == std::string("usb") &&
	  udev_device_get_action(dev) == std::string("change")) {
	update(dev);
      }

      udev_device_unref(dev);
    }
  }

  udev_enumerate_unref(enumerate);

  fd_set fds;
  struct timeval tv;

  while (true) {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    FD_SET(m_sv[1], &fds);

    if (select((fd > m_sv[1] ? fd : m_sv[1]) + 1, &fds, NULL, NULL, NULL) > 0) {
      if (FD_ISSET(fd, &fds)) {
	struct udev_device *dev = udev_monitor_receive_device(mon);
	if (!dev) {
	  continue;
	}

	if (udev_device_get_sysname(dev) != std::string("usb")) {
	  udev_device_unref(dev);
	  continue;
	}

	if (udev_device_get_action(dev) != std::string("change")) {
	  udev_device_unref(dev);
	  continue;
	}

	update(dev);
	udev_device_unref(dev);
      } else {
	goto out;
      }
    }
  }


out:
  close(fd); // TODO: is this correct?
  udev_monitor_unref(mon);
  udev_unref(udev);
}

void UsbHal::update(struct udev_device *dev) {
  const char *power = udev_device_get_property_value(dev, "POWER_SUPPLY_PRESENT");
  if (!power) {
    power = udev_device_get_property_value(dev, "POWER_SUPPLY_ONLINE");
  }

  if (!power) {
    std::cerr << "Cannot find power supply indicator" << std::endl;
    setCableConnected(false);
    return;
  }

  if (power != std::string("1")) {
    // Disconnected.
    setCableConnected(false);
  } else {
    setCableConnected(true);
  }
}

void UsbHal::setCableConnected(bool connected) {
  std::lock_guard<std::mutex> lock(m_lock);

  if (m_connected != connected) {
    m_connected = connected;

    if (m_listener) {
      m_listener(m_connected);
    }
  }
}
