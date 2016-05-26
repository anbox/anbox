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

#include "backtrace.h"

#include <cxxabi.h>

#include <execinfo.h>

namespace bt = core::posix::backtrace;

namespace impl
{
std::tuple<std::string, bool> demangle(const std::string& symbol)
{
    int status = 1;
    auto result = abi::__cxa_demangle(symbol.c_str(),
                                      nullptr,
                                      nullptr,
                                      &status);

    if (!result || status != 0)
    {
        return std::make_tuple(std::string(), false);
    }

    std::string s{result};
    ::free(result);

    return std::make_tuple(s, true);
}

struct Frame : public bt::Frame
{
    struct Symbol : public bt::Frame::Symbol
    {
        Symbol(const char* symbol) : raw_(symbol)
        {
            auto first = raw_.find_first_of("(");
            auto last = raw_.find_last_of(")");

            if (first != std::string::npos && last != std::string::npos)
            {
                auto mangled_symbol = raw_.substr(first+1,
                                                  (last-1) - (first+1));

                auto plus = mangled_symbol.find_first_of("+");
                if (plus != std::string::npos)
                    mangled_symbol.erase(plus);

                std::tie(demangled_, is_cxx_) = demangle(mangled_symbol);
                if (!is_cxx_)
                    demangled_ = raw_;
            }
        }

        bool is_cxx() const
        {
            return is_cxx_;
        }

        std::string demangled() const
        {
            return demangled_;
        }

        std::string raw() const
        {
            return raw_;
        }

        std::string raw_;
        std::string demangled_;
        bool is_cxx_ = false;
    };

    std::size_t depth_;
    void* frame_pointer_;
    Symbol symbol_;

    Frame(std::size_t depth, void* frame_pointer, const char* symbol)
        : depth_(depth),
          frame_pointer_(frame_pointer),
          symbol_(symbol)
    {
    }

    std::size_t depth() const
    {
        return depth_;
    }

    virtual void* frame_pointer() const
    {
        return frame_pointer_;
    }

    const Symbol& symbol() const
    {
        return symbol_;
    }
};
}

std::shared_ptr<bt::Frame::Symbol> bt::Frame::Symbol::for_testing_from_raw_symbol(const char* symbol)
{
    return std::shared_ptr<bt::Frame::Symbol>(new impl::Frame::Symbol(symbol));
}

void bt::visit_with_handler(const bt::FrameHandler& handler)
{
    static const unsigned int max_frames=64;
    void *frames[max_frames];

    auto frame_count = ::backtrace(frames, max_frames);
    auto symbols = ::backtrace_symbols(frames, frame_count);

    struct Scope
    {
        Scope(char** symbols) : symbols(symbols)
        {
        }

        ~Scope()
        {
            ::free(symbols);
        }

        char** symbols = nullptr;
    } scope{symbols};

    for (int i = 0; i < frame_count; i++)
    {
        impl::Frame frame(i, frames[i], symbols[i]);
        if (!handler(frame))
            return;
    }
}
