add_subdirectory(process-cpp-minimal)
add_subdirectory(android-emugl)
add_subdirectory(xdg)

# use supplied libraries if not on Ubuntu
IF(NOT DBUS_CPP_FOUND)
  add_subdirectory(properties-cpp)
  add_subdirectory(dbus-cpp)

  # provide the target name globally for linking agains dbus-cpp
  SET(DBUS_CPP_LIBRARIES "dbus-cpp" CACHE STRING "DBus-cpp library" FORCE)
ENDIF()
