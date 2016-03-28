cmake_minimum_required(VERSION 2.6)
project(HwHal-n950-n9)
set(CMAKE_CXX_FLAGS "-std=c++0x")

add_library(hwhal MODULE
  displayhal.cpp
  infohal.cpp
  lightshal.cpp
  sysfs.cpp
  plugin.cpp
  )

install(TARGETS hwhal LIBRARY DESTINATION /usr/lib/hwhal/)
