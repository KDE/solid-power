/*
    Copyright 2005-2007 Kevin Ottens <ervin@kde.org>
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
#include <QDBusMetaType>

#include "powermanagement.h"
#include "power_hal_p.h"

Q_LOGGING_CATEGORY(SOLID_POWER, "solid.power.hal")

#define HAL_SERVICE QStringLiteral("org.freedesktop.Hal")
#define HAL_PATH QStringLiteral("/org/freedesktop/Hal/devices/computer")
#define HAL_PATH_MANAGER QStringLiteral("/org/freedesktop/Hal/Manager")

#define HAL_IFACE_DEVICE QStringLiteral("org.freedesktop.Hal.Device")
#define HAL_IFACE_POWER QStringLiteral("org.freedesktop.Hal.Device.SystemPowerManagement")
#define HAL_IFACE_MANAGER QStringLiteral("org.freedesktop.Hal.Manager")

Q_GLOBAL_STATIC(Solid::PowerManagementPrivate, globalPowerManager)

Q_DECLARE_METATYPE(ChangeDescription)
Q_DECLARE_METATYPE(QList<ChangeDescription>)

const QDBusArgument &operator<<(QDBusArgument &arg, const ChangeDescription &change)
{
    arg.beginStructure();
    arg << change.key << change.added << change.removed;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ChangeDescription &change)
{
    arg.beginStructure();
    arg >> change.key >> change.added >> change.removed;
    arg.endStructure();
    return arg;
}

// private
Solid::PowerManagementPrivate::PowerManagementPrivate():
    halComputer(HAL_SERVICE, HAL_PATH, HAL_IFACE_DEVICE,
                QDBusConnection::systemBus()),
    halPowerManagement(HAL_SERVICE, HAL_PATH, HAL_IFACE_POWER,
                       QDBusConnection::systemBus()),
    halManager(HAL_SERVICE, HAL_PATH_MANAGER, HAL_IFACE_MANAGER,
               QDBusConnection::systemBus())
{
    qDBusRegisterMetaType<ChangeDescription>();
    qDBusRegisterMetaType<QList<ChangeDescription> >();
    QMetaObject::invokeMethod(this, "init");
}

Solid::PowerManagementPrivate::~PowerManagementPrivate()
{
}

bool Solid::PowerManagementPrivate::checkHalProperty(const QString &prop)
{
    QDBusReply<bool> reply = halComputer.asyncCall(QStringLiteral("GetPropertyBoolean"), prop);
    if (reply.isValid()) {
        //qCDebug(SOLID_POWER) << prop << reply.value();
        return reply;
    } else {
        qCWarning(SOLID_POWER) << prop << reply.error().name() << reply.error().message();
    }
    return false;
}

void Solid::PowerManagementPrivate::makeHalCall(const QString &method, int param)
{
    qCDebug(SOLID_POWER) << "Making HAL call:" << method;
    halPowerManagement.asyncCall(method, param);
}

void Solid::PowerManagementPrivate::init()
{
    // init the power save mode
    powerSaveMode = checkHalProperty(QStringLiteral("power_management.is_powersave_set"));

    // get the supported sleep methods
    if (checkHalProperty(QStringLiteral("power_management.can_suspend"))) {
        supportedSleepStates += Solid::PowerManagement::SuspendState;
    }
    if (checkHalProperty(QStringLiteral("power_management.can_hibernate"))) {
        supportedSleepStates += Solid::PowerManagement::HibernateState;
    }
    if (checkHalProperty(QStringLiteral("power_management.can_suspend_hybrid"))) {
        supportedSleepStates += Solid::PowerManagement::HybridSuspendState;
    }

    // subscribe to the power save mode property updates
    QDBusConnection::systemBus().connect(HAL_SERVICE, HAL_PATH, HAL_IFACE_DEVICE,
                                         QStringLiteral("PropertyModified"), this,
                                         SLOT(slotPropertyModified(int,QList<ChangeDescription>)));

    // find the lid, if any
    QDBusReply<QStringList> lidCall = halManager.asyncCall(QStringLiteral("FindDeviceStringMatch"), QStringLiteral("button.type"), QStringLiteral("lid"));
    if (lidCall.isValid() && !lidCall.value().isEmpty()) {
        const QString path = lidCall.value().first();
        if (!path.isEmpty() && path != QStringLiteral("/")) {
            lidIface = new QDBusInterface(HAL_SERVICE, path, HAL_IFACE_DEVICE, QDBusConnection::systemBus(), this);
            if (lidIface->isValid()) {
                hasLid = true;
                slotLidButtonPressed();
                // setup notifier signals
                QDBusConnection::systemBus().connect(HAL_SERVICE, path, HAL_IFACE_DEVICE,
                                                     QStringLiteral("Condition"), this,
                                                     SLOT(slotLidButtonPressed(QString, QString)));
            }
        }
    } else {
        qCWarning(SOLID_POWER) << lidCall.error().name() << lidCall.error().message();
    }
}

void Solid::PowerManagementPrivate::slotLidButtonPressed(const QString &type, const QString &reason)
{
    Q_UNUSED(reason)
    const bool wasClosed = isLidClosed;
    if (type == QStringLiteral("ButtonPressed")) {
        QDBusReply<bool> lidReply = lidIface->asyncCall(QStringLiteral("GetPropertyBoolean"), QStringLiteral("button.state.value"));
        if (lidReply.isValid()) {
            if (lidReply != wasClosed) {
                isLidClosed = lidReply;
                Q_EMIT isLidClosedChanged(isLidClosed);
            }
        }
    }
}

void Solid::PowerManagementPrivate::slotPropertyModified(int count, const QList<ChangeDescription> &changes)
{
    Q_UNUSED(count)
    // Int num_changes, Array of struct {String property_name, Bool added, Bool removed}
    Q_FOREACH(const ChangeDescription &change, changes) {
        if (change.key == QStringLiteral("power_management.is_powersave_set") && !change.added && !change.removed) {
            powerSaveMode = checkHalProperty(QStringLiteral("power_management.is_powersave_set"));
            Q_EMIT appShouldConserveResourcesChanged(powerSaveMode);
        }
    }
}

Solid::PowerManagement::Notifier::Notifier()
{
}

Solid::PowerManagement::Notifier *Solid::PowerManagement::notifier()
{
    return globalPowerManager;
}

// public
bool Solid::PowerManagement::appShouldConserveResources()
{
    return globalPowerManager->powerSaveMode;
}

bool Solid::PowerManagement::canSuspend()
{
    return globalPowerManager->supportedSleepStates.contains(SuspendState);
}

bool Solid::PowerManagement::canHibernate()
{
    return globalPowerManager->supportedSleepStates.contains(HibernateState);
}

bool Solid::PowerManagement::canHybridSleep()
{
    return globalPowerManager->supportedSleepStates.contains(HybridSuspendState);
}

bool Solid::PowerManagement::canReboot()
{
    return true; // TODO check
}

bool Solid::PowerManagement::canShutdown()
{
    return true; // TODO check
}

QSet<Solid::PowerManagement::SleepState> Solid::PowerManagement::supportedSleepStates()
{
    return globalPowerManager->supportedSleepStates;
}

void Solid::PowerManagement::suspend()
{
    Q_EMIT globalPowerManager->aboutToSuspend(); // yea :)
    globalPowerManager->makeHalCall(QStringLiteral("Suspend"));
}

void Solid::PowerManagement::hibernate()
{
    Q_EMIT globalPowerManager->aboutToSuspend(); // yea :)
    globalPowerManager->makeHalCall(QStringLiteral("Hibernate"), -1);
}

void Solid::PowerManagement::hybridSleep()
{
    Q_EMIT globalPowerManager->aboutToSuspend(); // yea :)
    globalPowerManager->makeHalCall(QStringLiteral("SuspendHybrid"));
}

void Solid::PowerManagement::reboot()
{
    Q_EMIT globalPowerManager->shuttingDown(); // yea :)
    globalPowerManager->makeHalCall(QStringLiteral("Reboot"));
}

void Solid::PowerManagement::shutdown()
{
    Q_EMIT globalPowerManager->shuttingDown(); // yea :)
    globalPowerManager->makeHalCall(QStringLiteral("Shutdown"));
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
