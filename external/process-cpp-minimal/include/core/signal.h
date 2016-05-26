/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#ifndef COM_UBUNTU_SIGNAL_H_
#define COM_UBUNTU_SIGNAL_H_

#include <core/connection.h>

#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <set>

namespace core
{
/**
 * @brief A signal class that observers can subscribe to.
 * @tparam Arguments List of argument types passed on to observers when the signal is emitted.
 */
template<typename ...Arguments>
class Signal
{
public:
    /**
     * @brief Slot is the function type that observers have to provide to connect to this signal.
     */
    typedef std::function<void(Arguments...)> Slot;

private:
    struct SlotWrapper
    {
        void operator()(Arguments... args)
        {
            dispatcher(std::bind(slot, args...));
        }

        Slot slot;
        Connection::Dispatcher dispatcher;
        Connection connection;
    };

public:
    /**
     * @brief Signal constructs a new instance. Never throws.
     */
    inline Signal() noexcept(true) : d(new Private())
    {
    }

    inline ~Signal()
    {
        std::lock_guard<std::mutex> lg(d->guard);
        for (auto slot : d->slot_list)
            slot.connection.reset();
    }

    // Copy construction, assignment and equality comparison are disabled.
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;
    bool operator==(const Signal&) const = delete;

    /**
     * @brief Connects the provided slot to this signal instance.
     *
     * Calling this method is thread-safe and synchronized with any
     * other connect, signal emission or disconnect calls.
     *
     * @param slot The function to be called when the signal is emitted.
     * @return A connection object corresponding to the signal-slot connection.
     */
    inline Connection connect(const Slot& slot) const
    {
        // Helpers to initialize an invalid connection.
        static const Connection::Disconnector empty_disconnector{};
        static const Connection::DispatcherInstaller empty_dispatcher_installer{};

        // The default dispatcher immediately executes the function object
        // provided as argument on whatever thread is currently running.
        static const Connection::Dispatcher default_dispatcher
                = [](const std::function<void()>& handler) { handler(); };

        Connection conn{empty_disconnector, empty_dispatcher_installer};

        std::lock_guard<std::mutex> lg(d->guard);

        auto result = d->slot_list.insert(
                    d->slot_list.end(),
                    SlotWrapper{slot, default_dispatcher, conn});

        // We implicitly share our internal state with the connection here
        // by passing in our private bits contained in 'd' to the std::bind call.
        // This admittedly uncommon approach allows us to cleanly manage connection
        // and signal lifetimes without the need to mark everything as mutable.
        conn.d->disconnector = std::bind(
                    &Private::disconnect_slot_for_iterator,
                    d,
                    result);
        conn.d->dispatcher_installer = std::bind(
                    &Private::install_dispatcher_for_iterator,
                    d,
                    std::placeholders::_1,
                    result);

        return conn;
    }

    /**
     * @brief operator () emits the signal with the provided parameters.
     *
     * Please note that signal emissions might not be delivered immediately to
     * registered slots, depending on whether the respective connection is dispatched
     * via a queueing dispatcher. For that reason, the lifetime of the arguments has to
     * exceed the scope of the call to this operator and its surrounding scope.
     *
     * @param args The arguments to be passed on to registered slots.
     */
    inline void operator()(Arguments... args)
    {
        std::lock_guard<std::mutex> lg(d->guard);
        for(auto slot : d->slot_list)
        {
            slot(args...);
        }
    }

private:
    struct Private
    {
        typedef std::list<SlotWrapper> SlotList;

        inline void disconnect_slot_for_iterator(typename SlotList::iterator it)
        {
            std::lock_guard<std::mutex> lg(guard);
            slot_list.erase(it);
        }

        inline void install_dispatcher_for_iterator(const Connection::Dispatcher& dispatcher,
                                                    typename SlotList::iterator it)
        {
            std::lock_guard<std::mutex> lg(guard);
            it->dispatcher = dispatcher;
        }

        std::mutex guard;
        SlotList slot_list;
    };
    std::shared_ptr<Private> d;
};

/**
 * @brief A signal class that observers can subscribe to,
 * template specialization for signals without arguments.
 */
template<>
class Signal<void>
{
public:
    /**
     * @brief Slot is the function type that observers have to provide to connect to this signal.
     */
    typedef std::function<void()> Slot;

private:
    struct SlotWrapper
    {
        void operator()()
        {
            dispatcher(slot);
        }

        Slot slot;
        Connection::Dispatcher dispatcher;
        Connection connection;
    };

public:
    /**
     * @brief Signal constructs a new instance. Never throws.
     */
    inline Signal() noexcept(true) : d(new Private())
    {
    }

    inline ~Signal()
    {
        std::lock_guard<std::mutex> lg(d->guard);
        for (auto slot : d->slot_list)
            slot.connection.reset();
    }

    // Copy construction, assignment and equality comparison are disabled.
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;
    bool operator==(const Signal&) const = delete;

    /**
     * @brief Connects the provided slot to this signal instance.
     *
     * Calling this method is thread-safe and synchronized with any
     * other connect, signal emission or disconnect calls.
     *
     * @param slot The function to be called when the signal is emitted.
     * @return A connection object corresponding to the signal-slot connection.
     */
    inline Connection connect(const Slot& slot) const
    {
        // Helpers to initialize an invalid connection.
        static const Connection::Disconnector empty_disconnector{};
        static const Connection::DispatcherInstaller empty_dispatcher_installer{};

        // The default dispatcher immediately executes the function object
        // provided as argument on whatever thread is currently running.
        static const Connection::Dispatcher default_dispatcher
                = [](const std::function<void()>& handler) { handler(); };

        Connection conn{empty_disconnector, empty_dispatcher_installer};

        std::lock_guard<std::mutex> lg(d->guard);

        auto result = d->slot_list.insert(
                    d->slot_list.end(),
                    SlotWrapper{slot, default_dispatcher, conn});

        // We implicitly share our internal state with the connection here
        // by passing in our private bits contained in 'd' to the std::bind call.
        // This admittedly uncommon approach allows us to cleanly manage connection
        // and signal lifetimes without the need to mark everything as mutable.
        conn.d->disconnector = std::bind(
                    &Private::disconnect_slot_for_iterator,
                    d,
                    result);
        conn.d->dispatcher_installer = std::bind(
                    &Private::install_dispatcher_for_iterator,
                    d,
                    std::placeholders::_1,
                    result);

        return conn;
    }

    /**
     * @brief operator () emits the signal.
     *
     * Please note that signal emissions might not be delivered immediately to
     * registered slots, depending on whether the respective connection is dispatched
     * via a queueing dispatcher.
     */
    inline void operator()()
    {
        std::lock_guard<std::mutex> lg(d->guard);
        for(auto slot : d->slot_list)
        {
            slot();
        }
    }

private:
    struct Private
    {
        typedef std::list<SlotWrapper> SlotList;

        inline void disconnect_slot_for_iterator(typename SlotList::iterator it)
        {
            std::lock_guard<std::mutex> lg(guard);
            slot_list.erase(it);
        }

        inline void install_dispatcher_for_iterator(const Connection::Dispatcher& dispatcher,
                                                    typename SlotList::iterator it)
        {
            std::lock_guard<std::mutex> lg(guard);
            it->dispatcher = dispatcher;
        }

        std::mutex guard;
        SlotList slot_list;
    };
    std::shared_ptr<Private> d;
};
}

#endif // COM_UBUNTU_SIGNAL_H_
