#ifndef SENSORS_HAL_H
#define SENSORS_HAL_H

#include <hwhal/sensors.h>
#include <map>

class LoopIntegration;

class SensorsHal : public Sensors {
public:
  class SensorHal;

  SensorsHal(LoopIntegration *loop);
  ~SensorsHal();

  bool hasSensor(const Sensor& sensor);
  bool monitor(const Sensor& sensor, const std::function<void(const Sensors::Reading&)>& listener);

private:
  LoopIntegration *m_loop;
  std::map<Sensors::Sensor, SensorsHal::SensorHal *> m_sensors;
};

#endif /* SENSORS_HAL_H */
