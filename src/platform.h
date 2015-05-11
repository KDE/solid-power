/*
    Copyright 2015 Lukáš Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include <solidpower_export.h>

#ifndef SOLID_PLATFORM_H
#define SOLID_PLATFORM_H

#include <QString>

namespace Solid {

/**
 * This namespace allows to query the underlying system to obtain information
 * about the platform.
 *
 * Note that it's implemented as a singleton and encapsulates the backend logic.
 *
 * @author Lukáš Tinkl &lt;ltinkl@redhat.com&gt;
 */
namespace Platform {

/**
 * Defines the possible chassis types of the underlying system
 */
enum class Chassis {
    //! Unknown chassis type
    Unknown = 0,
    //! Desktop computer
    Desktop,
    //! Portable (laptop/notebook)
    Laptop,
    //! Server
    Server,
    //! Tablet
    Tablet,
    //! Phone
    Phone,
    //! Virtual machine
    VM,
    //! Virtual container
    Container
};

/**
  * @return the system's chassis type (formfactor)
  */
SOLIDPOWER_EXPORT Chassis chassis();

/**
  * @return the system's hostname
  */
SOLIDPOWER_EXPORT QString hostname();

/**
  * @return the name of the icon representing the system's form factor (e.g. "computer-laptop")
  */
SOLIDPOWER_EXPORT QString iconName();

/**
  * @return user-friendly name of the operating system
  */
SOLIDPOWER_EXPORT QString prettyOSName();

} // namespace Platform

} // namespace Solid

#endif
