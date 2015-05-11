# compile in the loader (solidpower_qt defined in Messages.sh)
ecm_create_qm_loader(solidpower_QM_LOADER solidpower_qt)

# make moc happy
set(moc_HDRS powermanagement.h)

# backends
if(CMAKE_SYSTEM_NAME MATCHES Linux)
    message(STATUS "Building Solid Login1/UPower backend.")
    set(solidpower_LIB_SRCS power_login1.cpp)
else()
    message(STATUS "Building Solid HAL backend.")
    set(solidpower_LIB_SRCS power_hal.cpp)
endif()

set(solidpower_LIB_SRCS inhibitions.cpp platform.cpp ${solidpower_LIB_SRCS} ${solidpower_QM_LOADER})

set_source_files_properties(org.freedesktop.PowerManagement.Inhibit.xml
                            org.kde.Solid.PowerManagement.PolicyAgent.xml
                            PROPERTIES NO_NAMESPACE TRUE)

qt5_add_dbus_interfaces(solidpower_LIB_SRCS
                        org.freedesktop.PowerManagement.Inhibit.xml
                        org.kde.Solid.PowerManagement.PolicyAgent.xml)

# library
qt5_wrap_cpp(solidpower_LIB_SRCS ${moc_HDRS})
add_library(KF5SolidPower ${solidpower_LIB_SRCS})

target_include_directories(KF5SolidPower PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")
generate_export_header(KF5SolidPower BASE_NAME SolidPower)
add_library(KF5::SolidPower ALIAS KF5SolidPower)

# Apps must include <Solid/File> or <solid/file.h>
target_include_directories(KF5SolidPower INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/Solid/Power>")

target_link_libraries(KF5SolidPower PUBLIC Qt5::Core
                                    PRIVATE Qt5::DBus
)

set_target_properties(KF5SolidPower PROPERTIES VERSION ${SOLIDPOWER_VERSION_STRING}
                                    SOVERSION ${SOLIDPOWER_SOVERSION}
                                    EXPORT_NAME SolidPower
)

ecm_generate_headers(SolidPower_CamelCase_HEADERS
  HEADER_NAMES
  PowerManagement
  Platform

  REQUIRED_HEADERS SolidPower_HEADERS
  #PREFIX SolidPower
)

install(FILES ${SolidPower_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/Solid/Power COMPONENT Devel)

install(TARGETS KF5SolidPower EXPORT KF5SolidPowerTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/solidpower_export.h
  ${SolidPower_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/Solid/Power COMPONENT Devel
)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME SolidPower LIB_NAME KF5SolidPower DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/Solid/Power)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

add_subdirectory(test)
