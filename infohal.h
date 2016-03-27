#ifndef INFO_HAL_H
#define INFO_HAL_H

#include <hwhal/info.h>

class InfoHal : public Info {
public:
  std::string maker();
  std::string model();
  std::string codeName();

private:
  void readCodeName();

  std::string m_code;
};

#endif /* INFO_HAL_H */
