/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>
    Copyright 2013-2015 Lukáš Tinkl <ltinkl@redhat.com>

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

#include <QCoreApplication>
#include <QDBusMessage>
#include <QDBusReply>

#include "powermanagement.h"
#include "inhibitions_p.h"

Q_GLOBAL_STATIC(InhibitionsPrivate, globalInhibitions)

InhibitionsPrivate::InhibitionsPrivate():
    policyAgentIface(QStringLiteral("org.kde.Solid.PowerManagement.PolicyAgent"),
                     QStringLiteral("/org/kde/Solid/PowerManagement/PolicyAgent"),
                     QDBusConnection::sessionBus()),
    inhibitIface(QStringLiteral("org.freedesktop.PowerManagement.Inhibit"),
                 QStringLiteral("/org/freedesktop/PowerManagement/Inhibit"),
                 QDBusConnection::sessionBus())
{
}

InhibitionsPrivate::~InhibitionsPrivate()
{
}

int Solid::PowerManagement::beginSuppressingSleep(const QString &reason)
{
    QDBusReply<uint> reply;
    if (globalInhibitions->policyAgentIface.isValid()) {
        reply = globalInhibitions->policyAgentIface.AddInhibition(
                    (uint)InhibitionsPrivate::InterruptSession,
                    QCoreApplication::applicationName(), reason);
    } else {
        // Fallback to the fd.o Inhibit interface
        reply = globalInhibitions->inhibitIface.Inhibit(QCoreApplication::applicationName(), reason);
    }

    if (reply.isValid()) {
        return reply;
    } else {
        return -1;
    }
}

bool Solid::PowerManagement::stopSuppressingSleep(int cookie)
{
    if (globalInhibitions->policyAgentIface.isValid()) {
        return globalInhibitions->policyAgentIface.ReleaseInhibition(cookie).isValid();
    } else {
        // Fallback to the fd.o Inhibit interface
        return globalInhibitions->inhibitIface.UnInhibit(cookie).isValid();
    }
}

int Solid::PowerManagement::beginSuppressingScreenPowerManagement(const QString &reason)
{
    if (globalInhibitions->policyAgentIface.isValid()) {
        QDBusReply<uint> reply = globalInhibitions->policyAgentIface.AddInhibition(
                    (uint)InhibitionsPrivate::ChangeScreenSettings,
                    QCoreApplication::applicationName(), reason);

        if (reply.isValid()) {
            QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"),
                                                                  QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("Inhibit"));
            message << QCoreApplication::applicationName();
            message << reason;

            QDBusReply<uint> ssReply = QDBusConnection::sessionBus().asyncCall(message);
            if (ssReply.isValid()) {
                globalInhibitions->screensaverCookiesForPowerDevilCookies.insert(reply, ssReply.value());
            }

            return reply;
        } else {
            return -1;
        }
    } else {
        // No way to fallback on something, hence return failure
        return -1;
    }
}

bool Solid::PowerManagement::stopSuppressingScreenPowerManagement(int cookie)
{
    if (globalInhibitions->policyAgentIface.isValid()) {
        bool result = globalInhibitions->policyAgentIface.ReleaseInhibition(cookie).isValid();

        if (globalInhibitions->screensaverCookiesForPowerDevilCookies.contains(cookie)) {
            QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"),
                                                                  QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("UnInhibit"));
            message << globalInhibitions->screensaverCookiesForPowerDevilCookies.take(cookie);
            QDBusConnection::sessionBus().asyncCall(message);
        }

        return result;
    } else {
        // No way to fallback on something, hence return failure
        return false;
    }
}

