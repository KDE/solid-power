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

#ifndef SOLID_POWER_LOGIN1_P_H
#define SOLID_POWER_LOGIN1_P_H

#include <QDBusInterface>
#include <QLoggingCategory>

#include "powermanagement.h"

Q_DECLARE_LOGGING_CATEGORY(SOLID_POWER)

namespace Solid
{
class PowerManagementPrivate : public PowerManagement::Notifier
{
    Q_OBJECT
public:
    PowerManagementPrivate();
    ~PowerManagementPrivate();

    void makeLogin1Call(const QString &method);

public Q_SLOTS:
    void init();
    void upowerPropertiesChanged(const QString& interface, const QVariantMap& changedProperties, const QStringList& invalidated);
    void login1Resuming(bool active);
    void login1ShuttingDown(bool active);

public:
    bool powerSaveStatus = false;
    bool hasLid = false;
    bool isLidClosed = false;
    QSet<Solid::PowerManagement::SleepState> supportedSleepStates;
};
}

#endif
