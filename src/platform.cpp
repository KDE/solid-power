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

#include <QString>
#include <QGlobalStatic>
#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>

#include <future>

#include "platform.h"
#include "platform_p.h"

Q_LOGGING_CATEGORY(SOLID_PLATFORM, "solid.platform")

#define HOSTNAME1_SERVICE QStringLiteral("org.freedesktop.hostname1")
#define HOSTNAME1_PATH QStringLiteral("/org/freedesktop/hostname1")
#define HOSTNAME1_IFACE QStringLiteral("org.freedesktop.hostname1")

#define DBUS_PROPS_IFACE QStringLiteral("org.freedesktop.DBus.Properties")

Q_GLOBAL_STATIC(PlatformPrivate, globalPlatform)

QString getHostname1Property(const QString &name)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(HOSTNAME1_SERVICE, HOSTNAME1_PATH, DBUS_PROPS_IFACE, QStringLiteral("Get"));
    msg << HOSTNAME1_IFACE;
    msg << name;
    QDBusReply<QVariant> reply = QDBusConnection::systemBus().asyncCall(msg);
    if (reply.isValid()) {
        return reply.value().toString();
    } else {
        qCWarning(SOLID_PLATFORM) << name << reply.error().name() << reply.error().message();
    }
    return QString();
}

PlatformPrivate::PlatformPrivate()
{
    QMetaObject::invokeMethod(this, "init");
}

PlatformPrivate::~PlatformPrivate()
{
}

void PlatformPrivate::init()
{
    auto sfut = std::async(std::launch::async, getHostname1Property, QStringLiteral("Chassis"));
    auto hfut = std::async(std::launch::async, getHostname1Property, QStringLiteral("Hostname"));
    auto ifut = std::async(std::launch::async, getHostname1Property, QStringLiteral("IconName"));
    auto ponfut = std::async(std::launch::async, getHostname1Property, QStringLiteral("OperatingSystemPrettyName"));

    const QString chs = sfut.get();
    if (chs == QStringLiteral("desktop") || chs.isEmpty()) {
        chassis = Solid::Platform::Chassis::Desktop;
    } else if (chs == QStringLiteral("laptop")) {
        chassis = Solid::Platform::Chassis::Laptop;
    } else if (chs == QStringLiteral("server")) {
        chassis = Solid::Platform::Chassis::Server;
    } else if (chs == QStringLiteral("tablet")) {
        chassis = Solid::Platform::Chassis::Tablet;
    } else if (chs == QStringLiteral("handset")) {
        chassis = Solid::Platform::Chassis::Phone;
    } else if (chs == QStringLiteral("vm")) {
        chassis = Solid::Platform::Chassis::VM;
    } else if (chs == QStringLiteral("container")) {
        chassis = Solid::Platform::Chassis::Container;
    }

    hostname = hfut.get();
    if (hostname.isEmpty()) {
        hostname = QStringLiteral("localhost");
    }
    iconName = ifut.get();
    if (iconName.isEmpty()) {
        iconName = QStringLiteral("computer");
    }
    prettyOSName = ponfut.get();
}

Solid::Platform::Chassis Solid::Platform::chassis()
{
    return globalPlatform->chassis;
}

QString Solid::Platform::hostname()
{
    return globalPlatform->hostname;
}

QString Solid::Platform::iconName()
{
    return globalPlatform->iconName;
}

QString Solid::Platform::prettyOSName()
{
    return globalPlatform->prettyOSName;
}
