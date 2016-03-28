#ifndef SYSFS_H
#define SYSFS_H

#include <string>

class Sysfs {
public:
  Sysfs();
  ~Sysfs();

  bool open(const char *path, bool rw = true);

  void close();

  bool read(std::string& str);
  bool write(const std::string& str);

private:
  int m_fd;
};

#endif /* SYSFS_H */
