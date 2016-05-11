#include "sensorshal.h"
#include "sysfs.h"
#include <hwhal/loopintegration.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/types.h>

#define ACCELEROMETER_PATH_POLL "/sys/devices/platform/lis3lv02d/position"
#define ACCELEROMETER_PATH_RATE "/sys/devices/platform/lis3lv02d/rate"
#define ACCELEROMETER_RATE      100
#define ACCELEROMETER_MS        1000/ACCELEROMETER_RATE
#define MAGNETOMETER_PATH       "/dev/ak89750"

struct ak8975_data {
  __s16 x;
  __s16 y;
  __s16 z;
  __u16 valid;
} __attribute__((packed));

class SensorsHal::SensorHal {
public:
  SensorHal() {}
  virtual ~SensorHal() {}
  virtual bool start() = 0;
  virtual void stop() = 0;
};

class Accelerometer : public SensorsHal::SensorHal {
public:
  Accelerometer(LoopIntegration *loop, const std::function<void(const Sensors::Reading&)>& cb) :
    m_id(0),
    m_loop(loop),
    m_cb(cb) {

  }

  ~Accelerometer() {
    stop();
    m_loop = nullptr;
  }

  bool start() {
    Sysfs rate;
    if (!rate.open(ACCELEROMETER_PATH_RATE, true)) {
      return false;
    }

    std::stringstream s;
    s << ACCELEROMETER_RATE;
    if (!rate.write(s.str())) {
      return false;
    }

    rate.close();

    if (!m_file.open(ACCELEROMETER_PATH_POLL, false)) {
      return false;
    }

    read();

    m_id = m_loop->post(ACCELEROMETER_MS, [this]() {
	read();
      });

    return true;
  }

  void stop() {
    m_loop->cancel(m_id);
    m_id = 0;
    m_file.close();
  }

private:
  void read() {
    try {
      std::string s;
      if (!m_file.read(s)) {
	return;
      }

      while (s.front() == '(') {
	s.erase(0, 1);
      }

      while (s.back() == ')') {
	s.erase(s.size() -1 , 1);
      }

      Sensors::Reading r;
      std::istringstream ss(s);
      for (int x = 0; x < 3; x++) {
	std::string n;
	std::getline(ss, n, ',');
	if (n.empty()) {
	  return;
	}

	r.data[x] = std::stof(n);
      }

      r.valid = Sensors::Valid;
      m_cb(r);
    } catch (...) {
      // Nothing:
    }
  }

  Sysfs m_file;
  uint64_t m_id;
  LoopIntegration *m_loop;
  const std::function<void(const Sensors::Reading&)> m_cb;
};

class Magnetometer : public SensorsHal::SensorHal {
public:
  Magnetometer(LoopIntegration *loop, const std::function<void(const Sensors::Reading&)>& cb) :
    m_fd(-1),
    m_id(0),
    m_loop(loop),
    m_cb(cb) {

  }

  ~Magnetometer() {
    stop();
    m_loop = nullptr;
  }

  bool start() {
    m_fd = open(MAGNETOMETER_PATH, O_RDONLY);
    if (m_fd == -1) {
      return false;
    }

    read();

    m_id = m_loop->addFileDescriptor(m_fd, [this](bool ok) {
	if (ok) {
	  read();
	} else {
	  stop();
	  start();
	}
      });

    return m_id > 0;
  }

  void stop() {
    m_loop->cancel(m_id);
    m_id = 0;
    close(m_fd);
    m_fd = -1;
  }

private:
  void read() {
    struct ak8975_data data;
    if (::read(m_fd, &data, sizeof(data)) == sizeof(data)) {
      Sensors::Reading r;
      r.data[0] = data.x;
      r.data[1] = data.y;
      r.data[2] = data.z;
      r.valid = data.valid == 1 ? Sensors::Valid : Sensors::Invalid;
      m_cb(r);
    }
  }

  int m_fd;
  uint64_t m_id;
  LoopIntegration *m_loop;
  const std::function<void(const Sensors::Reading&)> m_cb;
};

SensorsHal::SensorsHal(LoopIntegration *loop) :
  m_loop(loop) {

}

SensorsHal::~SensorsHal() {
  while (!m_sensors.empty()) {
    auto iter = m_sensors.begin();
    auto s = iter->second;
    m_sensors.erase(iter);
    s->stop();
    delete s;
  }
}

bool SensorsHal::hasSensor(const Sensor& sensor) {
  switch (sensor) {
  case Accelerometer:
  case Magnetometer:
    return true;

    // TODO: support these
  case Proximity:

  case AmbientLight:
    return false;
  }

  return false;
}

bool SensorsHal::monitor(const Sensor& sensor,
			 const std::function<void(const Sensors::Reading&)>& listener) {
  if (!hasSensor(sensor)) {
    return false;
  }

  auto iter = m_sensors.find(sensor);

  if (!listener) {
    if (iter != m_sensors.end()) {
      auto s = iter->second;
      m_sensors.erase(iter);
      s->stop();
      delete s;
    }

    return true;
  }

  if (iter != m_sensors.end()) {
    return false;
  }

  SensorsHal::SensorHal *s = nullptr;
  switch (sensor) {
  case Proximity:

    break;
  case Magnetometer:
    s = new ::Magnetometer(m_loop, listener);
    break;

  case Accelerometer:
    s = new ::Accelerometer(m_loop, listener);
    break;

  case AmbientLight:
    break;
  }

  if (!s) {
    return false;
  }

  if (!s->start()) {
    delete s;
    return false;
  }

  m_sensors.insert(std::make_pair(sensor, s));

  return true;
}
