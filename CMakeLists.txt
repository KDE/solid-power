cmake_minimum_required(VERSION 2.8.12)

project(SolidPower)

find_package(ECM 1.7.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

set(REQUIRED_QT_VERSION 5.3.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)

include(ECMPoQmTools)

set(KF5_VERSION "5.8.0") # handled by release scripts

ecm_setup_version(${KF5_VERSION} VARIABLE_PREFIX SOLIDPOWER
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/solid_power_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5SolidPowerConfigVersion.cmake"
    SOVERSION 1)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

add_subdirectory(src)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5SolidPower")

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5SolidPowerConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5SolidPowerConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5SolidPowerConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5SolidPowerConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KF5SolidPowerTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5SolidPowerTargets.cmake NAMESPACE KF5::)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/solid_power_version.h DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
