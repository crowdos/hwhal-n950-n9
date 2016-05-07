cmake_minimum_required(VERSION 2.6)
project(HwHal-n950-n9)
set(CMAKE_CXX_FLAGS "-std=c++0x")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -W")

find_package(PkgConfig REQUIRED)
pkg_check_modules(UDEV REQUIRED libudev)

include_directories(${UDEV_INCLUDE_DIRS})
add_definitions(${UDEV_CFLAGS} ${UDEV_CFLAGS_OTHER})

add_library(hwhal MODULE
  displayhal.cpp
  infohal.cpp
  lightshal.cpp
  usbhal.cpp
  sensorshal.cpp
  sysfs.cpp
  plugin.cpp
  )

target_link_libraries(hwhal hwhal-keys-evdev ${UDEV_LIBRARIES})
install(TARGETS hwhal LIBRARY DESTINATION /usr/lib/hwhal/)
