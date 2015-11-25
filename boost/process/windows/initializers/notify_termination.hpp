// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROCESS_WINDOWS_INITIALIZERS_SEQUENCE_HPP
#define BOOST_PROCESS_WINDOWS_INITIALIZERS_SEQUENCE_HPP

#include <boost/asio/windows/object_handle.hpp>
#include <boost/process/windows/initializers/initializer_base.hpp>
#include <boost/function.hpp>

namespace boost { namespace process { namespace windows { namespace initializers {

struct notify_termination: public initializer_base
{
    typedef boost::function<void(int, int)> callback_t;

    notify_termination(asio::io_service& ios, const callback_t& callback):
        io_service_(ios),
        callback_(callback)
    {}

    template <class WindowsExecutor>
    void on_CreateProcess_success(WindowsExecutor& e) const
    {
        // On Windows, arrange to wait for child termination *after* execute()
        // returns, since we need the hProcess from the PROCESS_INFORMATION
        // block set by CreateProcessA().
        asio::windows::object_handle handle(io_service_, e.proc_info.hProcess);
        // extract native_handle() here to bypass odd object_handle copy behavior
        asio::windows::object_handle::native_handle_type
            native_handle(handle.native_handle());
        // copy callback_ to avoid
        // error C3480: a lambda capture variable must be from an enclosing function scope
        callback_t callback(callback_);
        handle.async_wait(
            [native_handle, callback](const boost::system::error_code&)
            {
                DWORD exit_code;
                int rc = -1, signal = 0;
                ::GetExitCodeProcess(native_handle, &exit_code);
                // High-order bit 0 means voluntary termination. Windows
                // produces large negative numbers (0xCnnnnnnn) when a process
                // terminates badly. Pass the same exit_code either way, but
                // map it to the same (return code, signal) convention we use
                // for Posix.
                if (! (exit_code & 0x80000000))
                    rc = int(exit_code);
                else
                    signal = int(exit_code);
                callback(rc, signal);
            });
    }

    asio::io_service& io_service_;
    callback_t callback_;
};

}}}}

#endif
