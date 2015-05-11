/*
    Copyright (C) 2014 Lukáš Tinkl <lukas@kde.org>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QDebug>

#include "powermanagement.h"
#include "platform.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("KDE"));
    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setApplicationName(QStringLiteral("SolidPower"));
    app.setApplicationVersion(QStringLiteral("0.1"));

    QObject::connect(Solid::PowerManagement::notifier(), &Solid::PowerManagement::Notifier::appShouldConserveResourcesChanged,
                     [](bool onBattery) {
        qDebug() << "The system is now on battery:" << onBattery;
    });

    if (Solid::PowerManagement::hasLid()) {
        QObject::connect(Solid::PowerManagement::notifier(), &Solid::PowerManagement::Notifier::isLidClosedChanged,
                         [](bool closed) {
            qDebug() << "The laptop lid is now closed:" << closed;
        });
    }

    QObject::connect(Solid::PowerManagement::notifier(), &Solid::PowerManagement::Notifier::aboutToSuspend,
                     []() {
        qDebug() << "The system is going to suspend...";
    });

    QObject::connect(Solid::PowerManagement::notifier(), &Solid::PowerManagement::Notifier::resumingFromSuspend,
                     []() {
        qDebug() << "The system is resuming...";
    });

    qDebug() << "Power saving mode:";
    qDebug() << "Is on battery:" << Solid::PowerManagement::appShouldConserveResources();
    qDebug() << "Has lid:" << Solid::PowerManagement::hasLid();
    qDebug() << "Is lid closed:" << Solid::PowerManagement::isLidClosed();
    qDebug() << "\nSleep methods:";
    qDebug() << "Can suspend:" << Solid::PowerManagement::canSuspend();
    qDebug() << "Can hibernate:" << Solid::PowerManagement::canHibernate();
    qDebug() << "Can hybrid sleep:" << Solid::PowerManagement::canHybridSleep();
    qDebug() << "Supported sleep methods (as above):" << Solid::PowerManagement::supportedSleepStates();
    qDebug() << "\nReboot/shutdown:";
    qDebug() << "Can reboot:" << Solid::PowerManagement::canReboot();
    qDebug() << "Can shutdown (poweroff):" << Solid::PowerManagement::canShutdown();
    qDebug() << "\nPlatform:";
    qDebug() << "Chassis:" << (int)Solid::Platform::chassis();
    qDebug() << "Hostname:" << Solid::Platform::hostname();
    qDebug() << "Icon name:" << Solid::Platform::iconName();
    qDebug() << "Pretty OS name:" << Solid::Platform::prettyOSName();

    return app.exec();
}
