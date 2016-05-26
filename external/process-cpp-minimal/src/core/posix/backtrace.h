/*
 * Copyright © 2014 Canonical Ltd.
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

#ifndef CORE_POSIX_BACKTRACE_H_
#define CORE_POSIX_BACKTRACE_H_

#include <core/posix/visibility.h>

#include <functional>
#include <memory>
#include <string>

namespace core
{
namespace posix
{
namespace backtrace
{
/**
 * @brief The Frame class models an individual frame of a backtrace.
 */
class Frame
{
public:
    /**
     * @brief The Symbol class models the symbolic representation of a frame pointer.
     */
    class Symbol
    {
    public:

        static std::shared_ptr<Symbol> for_testing_from_raw_symbol(const char* symbol);

        Symbol(const Symbol&) = delete;
        virtual ~Symbol() = default;

        Symbol& operator=(const Symbol&) = delete;

        /**
         * @brief is_cxx checks whether the symbol refers to a mangled C++ symbol.
         * @return true iff the symbol refers to a mangled C++ symbol.
         */
        virtual bool is_cxx() const = 0;

        /**
         * @brief demangled returns the demangled C++ symbol name or raw.
         */
        virtual std::string demangled() const = 0;

        /**
         * @brief raw The raw symbolic representation of a frame pointer.
         * @return
         */
        virtual std::string raw() const = 0;

    protected:
        Symbol() = default;
    };

    Frame(const Frame&) = delete;
    virtual ~Frame() = default;

    Frame& operator=(const Frame&) = delete;

    /**
     * @brief depth returns the depth of this frame in the overall backtrace.
     */
    virtual std::size_t depth() const = 0;

    /**
     * @brief frame_pointer returns the the raw frame pointer of this frame.
     * @return
     */
    virtual void* frame_pointer() const = 0;

    /**
     * @brief symbol returns the symbolic representation of this frame.
     */
    virtual const Symbol& symbol() const = 0;

protected:
    Frame() = default;
};

/**
 * @brief FrameHandler is the functor invoked for every frame of a backtrace.
 *
 * A FrameHandler should return true if it wants to continue walking the stack
 * or false otherwise.
 */
typedef std::function<bool(const Frame& frame)> FrameHandler;

/**
 *@brief visit_with_handler iterates the backtrace of the calling program,
 *invoking the handler for every frame.
 *
 * A FrameHandler should return true if it wants to continue walking the stack
 * or false otherwise
 *
 * @param handler The handler invoked for every frame.
 */
void CORE_POSIX_DLL_PUBLIC visit_with_handler(const FrameHandler& handler);
}
}
}

#endif // CORE_POSIX_BACKTRACE_H_
