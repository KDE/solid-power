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

#include <future>

#include "powermanagement.h"
#include "power_login1_p.h"

Q_LOGGING_CATEGORY(SOLID_POWER, "solid.power.login1")

#define UPOWER_SERVICE QStringLiteral("org.freedesktop.UPower")
#define UPOWER_PATH QStringLiteral("/org/freedesktop/UPower")
#define UPOWER_IFACE QStringLiteral("org.freedesktop.UPower")

#define LOGIN1_SERVICE QStringLiteral("org.freedesktop.login1")
#define LOGIN1_PATH QStringLiteral("/org/freedesktop/login1")
#define LOGIN1_IFACE QStringLiteral("org.freedesktop.login1.Manager")

#define PROP_ON_BATTERY QStringLiteral("OnBattery")
#define PROP_HAS_LID QStringLiteral("LidIsPresent")
#define PROP_LID_CLOSED QStringLiteral("LidIsClosed")

#define DBUS_PROPS_IFACE QStringLiteral("org.freedesktop.DBus.Properties")

Q_GLOBAL_STATIC(Solid::PowerManagementPrivate, globalPowerManager)

bool checkLogin1Call(const QString &method)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(LOGIN1_SERVICE, LOGIN1_PATH, LOGIN1_IFACE, method);
    QDBusReply<QString> reply = QDBusConnection::systemBus().asyncCall(msg);
    if (reply.isValid()) {
        //qCDebug(SOLID_POWER) << method << reply.value();
        return (reply == QStringLiteral("yes") || reply == QStringLiteral("challenge"));
    } else {
        qCWarning(SOLID_POWER) << method << reply.error().name() << reply.error().message();
    }
    return false;
}

bool checkUPowerProperty(const QString &name)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(UPOWER_SERVICE, UPOWER_PATH, DBUS_PROPS_IFACE, QStringLiteral("Get"));
    msg << UPOWER_IFACE;
    msg << name;
    QDBusReply<QVariant> reply = QDBusConnection::systemBus().asyncCall(msg);
    if (reply.isValid()) {
        return reply.value().toBool();
    } else {
        qCWarning(SOLID_POWER) << name << reply.error().name() << reply.error().message();
    }
    return false;
}

// private
Solid::PowerManagementPrivate::PowerManagementPrivate()
{
    QMetaObject::invokeMethod(this, "init");
}

Solid::PowerManagementPrivate::~PowerManagementPrivate()
{
}

void Solid::PowerManagementPrivate::makeLogin1Call(const QString &method)
{
    qCDebug(SOLID_POWER) << "Making Login1 call:" << method;
    QDBusMessage msg = QDBusMessage::createMethodCall(LOGIN1_SERVICE, LOGIN1_PATH, LOGIN1_IFACE, method);
    msg << true; // interactive
    QDBusConnection::systemBus().asyncCall(msg);
}

void Solid::PowerManagementPrivate::init()
{
    // setup notifier signals
    auto conn = QDBusConnection::systemBus();
    conn.connect(UPOWER_SERVICE, UPOWER_PATH, DBUS_PROPS_IFACE,
                 QStringLiteral("PropertiesChanged"),
                 this, SLOT(upowerPropertiesChanged(QString, QVariantMap, QStringList))
                );

    conn.connect(LOGIN1_SERVICE, LOGIN1_PATH, LOGIN1_IFACE,
                 QStringLiteral("PrepareForSleep"),
                 this, SLOT(login1Resuming(bool))
                );

    conn.connect(LOGIN1_SERVICE, LOGIN1_PATH, LOGIN1_IFACE,
                 QStringLiteral("PrepareForShutdown"),
                 this, SLOT(login1ShuttingDown(bool))
                );

    // fill properties
    auto ps = std::async(std::launch::async, checkUPowerProperty, PROP_ON_BATTERY);
    auto hl = std::async(std::launch::async, checkUPowerProperty, PROP_HAS_LID);
    auto lc = std::async(std::launch::async, checkUPowerProperty, PROP_LID_CLOSED);

    powerSaveStatus = ps.get();
    hasLid = hl.get();
    isLidClosed = lc.get();
}

Solid::PowerManagement::Notifier::Notifier()
{
}

Solid::PowerManagement::Notifier *Solid::PowerManagement::notifier()
{
    return globalPowerManager;
}

void Solid::PowerManagementPrivate::upowerPropertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidated)
{
    Q_UNUSED(invalidated)
    if (interface != UPOWER_IFACE) {
        return;
    }

    if (changedProperties.contains(PROP_ON_BATTERY)) {
        powerSaveStatus = changedProperties.value(PROP_ON_BATTERY).toBool();
        Q_EMIT appShouldConserveResourcesChanged(powerSaveStatus);
    }
    if (changedProperties.contains(PROP_LID_CLOSED)) {
        isLidClosed = changedProperties.value(PROP_LID_CLOSED).toBool();
        Q_EMIT isLidClosedChanged(isLidClosed);
    }
}

void Solid::PowerManagementPrivate::login1Resuming(bool active)
{
    if (active) {
        Q_EMIT aboutToSuspend();
    } else {
        Q_EMIT resumingFromSuspend();
    }
}

void Solid::PowerManagementPrivate::login1ShuttingDown(bool active)
{
    if (active) {
        Q_EMIT shuttingDown();
    }
}

// public
bool Solid::PowerManagement::appShouldConserveResources()
{
    return globalPowerManager->powerSaveStatus;
}

bool Solid::PowerManagement::canSuspend()
{
    return std::async(std::launch::async, checkLogin1Call, QStringLiteral("CanSuspend")).get();
}

bool Solid::PowerManagement::canHibernate()
{
    return std::async(std::launch::async, checkLogin1Call, QStringLiteral("CanHibernate")).get();
}

bool Solid::PowerManagement::canHybridSleep()
{
    return std::async(std::launch::async, checkLogin1Call, QStringLiteral("CanHybridSleep")).get();
}

bool Solid::PowerManagement::canReboot()
{
    return std::async(std::launch::async, checkLogin1Call, QStringLiteral("CanReboot")).get();
}

bool Solid::PowerManagement::canShutdown()
{
    return std::async(std::launch::async, checkLogin1Call, QStringLiteral("CanPowerOff")).get();
}

QSet<Solid::PowerManagement::SleepState> Solid::PowerManagement::supportedSleepStates()
{
    QSet<Solid::PowerManagement::SleepState> result;
    if (canSuspend()) {
        result += Solid::PowerManagement::SuspendState;
    }
    if (canHibernate()) {
        result += Solid::PowerManagement::HibernateState;
    }
    if (canHybridSleep()) {
        result += Solid::PowerManagement::HybridSuspendState;
    }
    return result;
}

void Solid::PowerManagement::suspend()
{
    globalPowerManager->makeLogin1Call(QStringLiteral("Suspend"));
}

void Solid::PowerManagement::hibernate()
{
    globalPowerManager->makeLogin1Call(QStringLiteral("Hibernate"));
}

void Solid::PowerManagement::hybridSleep()
{
    globalPowerManager->makeLogin1Call(QStringLiteral("HybridSleep"));
}

void Solid::PowerManagement::reboot()
{
    globalPowerManager->makeLogin1Call(QStringLiteral("Reboot"));
}

void Solid::PowerManagement::shutdown()
{
    globalPowerManager->makeLogin1Call(QStringLiteral("PowerOff"));
}

void Solid::PowerManagement::requestSleep(Solid::PowerManagement::SleepState state)
{
    switch (state) {
    case Solid::PowerManagement::SuspendState:
    case Solid::PowerManagement::StandbyState:
        suspend();
        break;
    case Solid::PowerManagement::HibernateState:
        hibernate();
        break;
    case Solid::PowerManagement::HybridSuspendState:
        hybridSleep();
        break;
    default:
        qCWarning(SOLID_POWER) << Q_FUNC_INFO << "Unsupported sleep state requested" << state;
    }
}

bool Solid::PowerManagement::hasLid()
{
    return globalPowerManager->hasLid;
}

bool Solid::PowerManagement::isLidClosed()
{
    return globalPowerManager->isLidClosed;
}
