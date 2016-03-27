#include "infohal.h"
#include <fstream>

std::string InfoHal::maker() {
  return "Nokia";
}

std::string InfoHal::model() {
  readCodeName();

  if (m_code == "RM-696") {
    return "N9";
  } else if (m_code == "RM-680") {
    return "N950";
  } else {
    return m_code;
  }
}

std::string InfoHal::codeName() {
  readCodeName();

  return m_code;
}

void InfoHal::readCodeName() {
  if (!m_code.empty()) {
    return;
  }

  std::ifstream in;
  in.open("/proc/cmdline");

  if (!in.is_open()) {
    return;
  }

  std::string data;
  while (in >> data) {
    std::string::size_type pos = data.find("product_name=");
    if (pos != std::string::npos) {
      m_code = data.substr(pos + 13);
      return;
    }

    data.clear();
  }

  in.close();
}
