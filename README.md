# Solid-Power
Solid Power API

## Introduction

This framework is the single entry point for power management. KDE applications should use it to control or query the power management features of the system.

Note that it's implemented as a singleton and encapsulates the backend logic.

## Usage

If you are using CMake, you need to have

    find_package(KF5SolidPower NO_MODULE)

(or similar) in your CMakeLists.txt file, and you need to link to KF5::SolidPower.

See the documentation for the Solid::PowerManagement namespace, and the [tutorials on
TechBase][tutorials].

## Dependencies

The 2 currently implemented backends (login1/upower and HAL) require the respective interfaces to be present
on DBUS at runtime.
