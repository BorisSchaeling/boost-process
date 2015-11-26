// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROCESS_POSIX_INITIALIZERS_NOTIFY_TERMINATION_HPP
#define BOOST_PROCESS_POSIX_INITIALIZERS_NOTIFY_TERMINATION_HPP

#include <boost/process/posix/initializers/initializer_base.hpp>
#include <boost/process/posix/initializers/notify_io_service.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <map>

namespace boost { namespace process { namespace posix { namespace initializers {

// notify_termination implies notify_io_service.
// notify_io_service should get all method calls we don't explicitly intercept.
struct notify_termination: public notify_io_service_<asio::io_service>
{
    typedef notify_io_service_<asio::io_service> notify_io_service;
    typedef boost::function<void(int, int)> callback_t;

    notify_termination(asio::io_service& ios, const callback_t& callback):
        notify_io_service(ios),
        io_service_(ios),
        callback_(callback)
    {}

    // TODO: perhaps manage_SIGCHLD should become an asio service, its
    // instance associated with the passed io_service.
    struct manage_signal
    {
        manage_signal(int signal_number):
            signal_number_(signal_number)
        {
            // It's important to capture the previous
            // ::sigaction(signal_number_) handler BEFORE initializing
            // asio::signal_set(signal_number_), which is why this code is in
            // a base-class constructor.
            std::memset(&prev_, 0, sizeof(prev_));
            // query old action, do NOT set new action here
            if (::sigaction(signal_number_, 0, &prev_) == -1)
            {
                // If for any reason we can't query the previous handler, set
                // dummy_handler so we can always forward to previous.
                prev_.sa_handler = &dummy_handler;
            }
        }

        void forward(siginfo_t* siginfo, void* context)
        {
            // SA_SIGINFO distinguishes between sa_handler and sa_sigaction
            if (prev_.sa_flags & SA_SIGINFO)
            {
                // previous handler used sa_sigaction, pass all params
                prev_.sa_sigaction(signal_number_, siginfo, context);
            }
            else
            {
                // previous handler used sa_handler
                // It would be clever if the special values SIG_DFL and
                // SIG_IGN were actually pointers to no-op functions -- but
                // I've never seen documentation guaranteeing that.
                if (! (prev_.sa_handler == SIG_DFL || prev_.sa_handler == SIG_IGN))
                {
                    // sa_handler only takes signal_number, discard siginfo and context
                    prev_.sa_handler(signal_number_);
                }
            }
        }

        static void dummy_handler(int) {}

        int signal_number_;
        struct sigaction prev_;
    };

    struct manage_SIGCHLD: public manage_signal
    {
        manage_SIGCHLD(asio::io_service& ios):
            manage_signal(SIGCHLD),
            // On construction, arrange to catch SIGCHLD. The intent is that
            // there should be only one instance of this struct process-wide.
            set_(ios, SIGCHLD)
        {
            // Immediately route every SIGCHLD signal to our notify() method.
            set_.async_wait(boost::bind(&manage_SIGCHLD::notify, this, _1, _2));
        }

        void add(pid_t pid, callback_t callback)
        {
            {
                // TODO: lock map here
                // We really do not expect to find a pre-existing entry with
                // the same pid value.
                callback_map_[pid] = callback;
            } // unlock callback_map_
        }

        void notify(const boost::system::error_code&, int)
        {
            // Note that although SIGCHLD triggers this method, in fact the
            // call is mediated via the asio::io_service. We can perform
            // normal operations, beyond the restricted set permitted for an
            // actual signal handler.
            // We should only reach this code on receipt of a SIGCHLD signal,
            // which should mean that some child process has definitely
            // terminated. Even so, pass WNOHANG to waitpid() since we cannot
            // afford to wait for anything!
            int status;
            pid_t pid = ::waitpid(-1, &status, WNOHANG);
            if (pid < 0)
            {
                // TODO: how to properly indicate error?
                return;
            }

            // Unpack the status int to our conventional (rc, signal) pair.
            // Default rc to -1 so callback function won't be fooled into
            // thinking child terminated normally if it was killed by a
            // signal. code will be passed as the siginfo_t::si_code if we
            // decide to forward this SIGCHLD to the previous handler.
            int code = 0, rc = -1, signal = 0;
            if (WIFEXITED(status))
            {
                code = CLD_EXITED;
                rc = WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status))
            {
#if defined(WCOREDUMP) && defined(CLD_DUMPED)
                if (WCOREDUMP(status))
                {
                    code = CLD_DUMPED;
                }
                else
#endif // WCOREDUMP && CLD_DUMPED
                {
                    code = CLD_KILLED;
                }
                signal = WTERMSIG(status);
            }
            else if (WIFSTOPPED(status))
            {
                code = CLD_STOPPED;
                signal = WSTOPSIG(status);
            }
            else if (WIFCONTINUED(status))
            {
                code = CLD_CONTINUED;
            }
            // not sure when we would decide to pass CLD_TRAPPED

            callback_t callback;
            {
                // TODO: lock map here
                callback_map_t::iterator found = callback_map_.find(pid);
                if (found == callback_map_.end())
                {
                    // This is not a child we forked, or at least not a child
                    // forked with the notify_termination initializer. Forward
                    // to previous SIGCHLD handler.
                    return forward_SIGCHLD(code, pid, status);
                }

                // Here we're sure the child that terminated is really ours.
                // Do we care about this particular SIGCHLD?
                if (! (code == CLD_EXITED || code == CLD_KILLED || code == CLD_DUMPED))
                {
                    // The notify_termination initializer is specifically
                    // about termination. Don't call our callback for anything
                    // less.
                    return;
                }

                // found indicates our callback entry. Copy the callback
                // function because we're just about to erase the map entry.
                callback = found->second;
                callback_map_.erase(found);
            } // unlock callback_map_

            // boost::function can be "empty". If so, don't try to call it.
            if (callback)
            {
                // call caller's callback
                callback(rc, signal);
            }
        }

        void forward_SIGCHLD(int code, pid_t pid, int status)
        {
            // Unfortunately asio::signal_set::async_wait() doesn't pass us
            // the siginfo_t received by the actual signal handler. Fake one
            // up, based on the fields that should be defined for SIGCHLD.
            siginfo_t fake;
            std::memset(&fake, 0, sizeof(fake));
            fake.si_signo = SIGCHLD;
            fake.si_errno = 0;      // defined for all signals, but what?
            fake.si_code = code;
            fake.si_pid = pid;
            fake.si_uid = getuid(); // we don't know this for pid
            fake.si_status = status;
            // We don't know either of these fields, and anyway not all
            // siginfo_t definitions even have them.
            //fake.si_utime = 0;
            //fake.si_stime = 0;
            // okay, call the previous handler
            // we have no idea of the context pointer
            forward(&fake, 0);
        }

        asio::signal_set set_;
        typedef std::map<pid_t, callback_t> callback_map_t;
        callback_map_t callback_map_;
    };

    template <class PosixExecutor>
    void on_fork_setup(PosixExecutor& e) const
    {
        // forward this call
        notify_io_service::on_fork_setup(e);
        // On Posix, we must arrange to catch SIGCHLD *before* executing the
        // child program, since it might terminate before execute() returns.
        // Instantiate manage_SIGCHLD; that's what catches SIGCHLD signals.
        get_manage_SIGCHLD(io_service_);
    }

    template <class PosixExecutor>
    void on_fork_success(PosixExecutor& e) const
    {
        // forward this call
        notify_io_service::on_fork_success(e);
        // Add this (pid, callback) pair to our SIGCHLD map right away,
        // trusting that we won't actually get SIGCHLD notification until the
        // caller pumps this io_service_.
        get_manage_SIGCHLD(io_service_).add(e.pid, callback_);
    }

    asio::io_service& io_service_;
    callback_t callback_;

    // TODO: perhaps we can improve on a function-static instance
    static manage_SIGCHLD& get_manage_SIGCHLD(asio::io_service& ios)
    {
        // who manages a child?
        static manage_SIGCHLD nanny(ios);
        return nanny;
    }
};

}}}}

#endif
