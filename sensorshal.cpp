#include "sensorshal.h"
#include "sysfs.h"
#include <hwhal/loopintegration.h>
#include <sstream>

#define ACCELEROMETER_PATH_POLL "/sys/devices/platform/lis3lv02d/position"
#define ACCELEROMETER_PATH_RATE "/sys/devices/platform/lis3lv02d/rate"
#define ACCELEROMETER_RATE      100
#define ACCELEROMETER_MS        1000/ACCELEROMETER_RATE

class SensorsHal::SensorHal {
public:
  SensorHal() {}
  virtual ~SensorHal() {}
  virtual bool start() = 0;
  virtual void stop() = 0;
};

class Accelerometer : public SensorsHal::SensorHal {
public:
  Accelerometer(LoopIntegration *loop, const std::function<void(const std::vector<int>&)>& cb) :
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

    if (!m_file.open(ACCELEROMETER_PATH_POLL, true)) {
      return false;
    }

    m_id = m_loop->post(ACCELEROMETER_MS, [this]() {
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

	  std::vector<int> data;
	  std::istringstream ss(s);
	  for (int x = 0; x < 3; x++) {
	    std::string n;
	    std::getline(ss, n, ',');
	    if (n.empty()) {
	      return;
	    }

	    data.push_back(std::stoi(n));
	  }

	  m_cb(data);
	} catch (...) {
	  // Nothing:
	}
      });

    return true;
  }

  void stop() {
    m_loop->cancel(m_id);
    m_id = 0;
    m_file.close();
  }

private:
  Sysfs m_file;
  uint64_t m_id;
  LoopIntegration *m_loop;
  const std::function<void(const std::vector<int>&)> m_cb;
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
    return true;

    // TODO: support these
  case Proximity:
  case Magnetometer:
  case AmbientLight:
    return false;
  }

  return false;
}

bool SensorsHal::monitor(const Sensor& sensor,
			 const std::function<void(const std::vector<int>&)>& listener) {
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
