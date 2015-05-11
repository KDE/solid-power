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

#ifndef SOLID_POWER_H
#define SOLID_POWER_H

#include <QObject>
#include <QSet>

#include <solidpower_export.h>

namespace Solid
{
/**
 * This namespace allows to query the underlying system to obtain information
 * about the hardware available.
 *
 * It is the single entry point for power management. Applications should use
 * it to control or query the power management features of the system.
 *
 * Note that it's implemented as a singleton and encapsulates the backend logic.
 *
 * See the Notifier class documentation about what signals it provides and how
 * to connect to them.
 *
 * @author Kevin Ottens &lt;ervin@kde.org&gt;
 * @author Lukáš Tinkl &lt;ltinkl@redhat.com&gt;
 */
namespace PowerManagement
{

/**
 * This enum type defines the different suspend methods.
 */
enum SleepState {
    //! StandbyState: Processes are stopped, some hardware is deactivated (ACPI S1)
    StandbyState = 1,
    //! Processes are stopped, some hardware is deactivated (ACPI S1)
    SuspendState = 2,
    //! State of the machine is saved to disk, and the machine is powered down (ACPI S4)
    HibernateState = 4,
    /**
     * The contents of RAM are first copied to non-volatile storage like for regular hibernation,
     * but then, instead of powering down, the computer enters sleep mode
     * @since 4.11
     */
    HybridSuspendState = 8
};

/**
 * Retrieves a high level indication of how applications should behave according to the
 * power management subsystem. For example, when on battery power, this method will return
 * true.
 *
 * @return whether apps should conserve power
 * @see Notifier::appShouldConserveResourcesChanged(bool)
 */
SOLIDPOWER_EXPORT bool appShouldConserveResources();

/**
  * @return whether the system is able of suspending (putting the computer to sleep)
  * @see SuspendState
  * @since 5.x
  */
SOLIDPOWER_EXPORT bool canSuspend();

/**
  * @return whether the system is able of hibernating
  * @see HibernateState
  * @since 5.x
  */
SOLIDPOWER_EXPORT bool canHibernate();

/**
  * @return whether the system is able of hybrid sleep (hybrid suspend)
  * @see HybridSuspendState
  * @since 5.x
  */
SOLIDPOWER_EXPORT bool canHybridSleep();

/**
  * @return whether the system is able of rebooting (restarting the machine)
  * @since 5.x
  */
SOLIDPOWER_EXPORT bool canReboot();

/**
  * @return whether the system is able of shutting down (powering off the machine)
  * @since 5.x
  */
SOLIDPOWER_EXPORT bool canShutdown();

/**
 * Retrieves the set of suspend methods supported by the system.
 *
 * @return the sleep methods supported by this system
 * @see Solid::PowerManagement::SleepState
 */
SOLIDPOWER_EXPORT QSet<SleepState> supportedSleepStates();

/**
  * Tell the system to enter the suspend mode (aka sleep).
  *
  * For details see Solid::PowerManagement::SuspendState.
  *
  * Emits the signal Notifier::aboutToSuspend() before, and
  * Notifier::resumingFromSuspend() after.
  *
  * @since 5.x
  */
SOLIDPOWER_EXPORT void suspend();

/**
  * Tell the system to enter the hibernate mode
  *
  * For details see Solid::PowerManagement::HibernateState
  *
  * Emits the signal Notifier::aboutToSuspend() before, and
  * Notifier::resumingFromSuspend() after.
  *
  * @since 5.x
  */
SOLIDPOWER_EXPORT void hibernate();

/**
  * Tell the system to enter the hybrid sleep mode
  *
  * For details see Solid::PowerManagement::HybridSuspendState
  *
  * Emits the signal Notifier::aboutToSuspend() before, and
  * Notifier::resumingFromSuspend() after.
  *
  * @since 5.x
  */
SOLIDPOWER_EXPORT void hybridSleep();

/**
  * Tell the system to reboot the machine.
  *
  * Emits the signal Notifier::shuttingDown()
  *
  * @since 5.x
  */
SOLIDPOWER_EXPORT void reboot();

/**
  * Tell the system to shutdown (poweroff) the machine.
  *
  * Emits the signal Notifier::shuttingDown()
  *
  * @since 5.x
  */
SOLIDPOWER_EXPORT void shutdown();

/**
 * Requests that the system go to sleep
 *
 * @param state the sleep state use
 */
SOLIDPOWER_EXPORT void requestSleep(SleepState state);

/**
 * Tell the power management subsystem to suppress automatic system sleep until further
 * notice.
 *
 * @param reason Give a reason for not allowing sleep, to be used in giving user feedback
 * about why a sleep event was prevented
 * @return a 'cookie' value representing the suppression request. Used by the power manager to
 * track the application's outstanding suppression requests. Returns -1 if the request was
 * denied.
 */
SOLIDPOWER_EXPORT int beginSuppressingSleep(const QString &reason = QString());

/**
 * Tell the power management that a particular sleep suppression is no longer needed.  When
 * no more suppressions are active, the system will be free to sleep automatically
 * @param cookie The cookie acquired when requesting sleep suppression
 * @return true if the suppression was stopped, false if an invalid cookie was given
 */
SOLIDPOWER_EXPORT bool stopSuppressingSleep(int cookie);

/**
 * Tell the power management subsystem to suppress automatic screen power management until
 * further notice.
 *
 * @param reason Give a reason for not allowing screen power management, to be used in giving user feedback
 * about why a screen power management event was prevented
 * @return a 'cookie' value representing the suppression request. Used by the power manager to
 * track the application's outstanding suppression requests. Returns -1 if the request was
 * denied.
 *
 * @note Since 4.8, this function also inhibits screensaver
 *
 * @since 4.6
 */
SOLIDPOWER_EXPORT int beginSuppressingScreenPowerManagement(const QString &reason = QString());

/**
 * Tell the power management that a particular screen power management suppression is no longer needed.  When
 * no more suppressions are active, the system will be free to handle screen power management automatically
 * @param cookie The cookie acquired when requesting screen power management suppression
 * @return true if the suppression was stopped, false if an invalid cookie was given
 *
 * @since 4.6
 */
SOLIDPOWER_EXPORT bool stopSuppressingScreenPowerManagement(int cookie);

/**
  * @return true whether the system has a lid (typically found on laptops)
  *
  * @see isLidClosed()
  *
  * @since 5.x
  */
SOLIDPOWER_EXPORT bool hasLid();

/**
  * @return true whether the laptop's lid is closed
  * @see hasLid()
  * @see Notifier::isLidClosedChanged(bool)
  *
  * @since 5.x
  */
SOLIDPOWER_EXPORT bool isLidClosed();

/**
 * @brief The Notifier class
 *
 * The notifier's purpose is to connect to its signals and receive information about the various
 * events happening with powermanagement related features.
 *
 * Example:
 * @code
 *   QObject::connect(Solid::PowerManagement::notifier(), &Solid::PowerManagement::Notifier::appShouldConserveResourcesChanged,
 *                    [](bool onBattery) {
 *       qDebug() << "The system is now on battery:" << onBattery;
 *   });
 * @endcode
 */
class SOLIDPOWER_EXPORT Notifier : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    /**
     * This signal is emitted when the AC adapter is plugged or unplugged.
     * @param onBattery whether the system runs on battery
     * @see appShouldConserveResources()
     */
    void appShouldConserveResourcesChanged(bool onBattery);

    /**
     * This signal is emitted whenever the system is resuming from suspend. Applications should connect
     * to this signal to perform actions due after a wake up (such as updating clocks, etc.).
     *
     * @since 4.7
     */
    void resumingFromSuspend();

    /**
     * This signal is emitted whenever the system is going to suspend or hibernate. Applications should connect
     * to this signal to perform last second cleanups (not guaranteed to happen).
     *
     * @since 5.x
     */
    void aboutToSuspend();

    /**
     * This signal is emitted whenever the system is going to shutdown or reboot. Applications should connect
     * to this signal to perform last second cleanups (not guaranteed to happen).
     *
     * @since 5.x
     */
    void shuttingDown();

    /**
     * This signal is emitted when the status of the laptop's lid changes
     * @param closed whether the lid is currently closed or not
     * @see isLidClosed()
     *
     * @since 5.x
     */
    void isLidClosedChanged(bool closed);

protected:
    Notifier();
};

/**
  * Provides access to the Notifier class
  */
SOLIDPOWER_EXPORT Notifier *notifier();
}
}

#endif
