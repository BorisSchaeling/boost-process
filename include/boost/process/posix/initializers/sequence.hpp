// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROCESS_POSIX_INITIALIZERS_SEQUENCE_HPP
#define BOOST_PROCESS_POSIX_INITIALIZERS_SEQUENCE_HPP

#include <boost/process/posix/initializers/initializer_base.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/member.hpp>
#include <boost/type_erasure/relaxed.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/range/algorithm/for_each.hpp>

// Define initializer_base methods to Boost.TypeErasure. The convenience macro
// must be used in the global namespace.
BOOST_TYPE_ERASURE_MEMBER((boost)(process)(posix)(initializers)(has_on_fork_setup),
                                                                    on_fork_setup, 1)
BOOST_TYPE_ERASURE_MEMBER((boost)(process)(posix)(initializers)(has_on_fork_error),
                                                                    on_fork_error, 1)
BOOST_TYPE_ERASURE_MEMBER((boost)(process)(posix)(initializers)(has_on_fork_success),
                                                                    on_fork_success, 1)
BOOST_TYPE_ERASURE_MEMBER((boost)(process)(posix)(initializers)(has_on_exec_setup),
                                                                    on_exec_setup, 1)
BOOST_TYPE_ERASURE_MEMBER((boost)(process)(posix)(initializers)(has_on_exec_error),
                                                                    on_exec_error, 1)

namespace boost { namespace process { namespace posix { namespace initializers {

// any_initializer should be able to store any subclass of initializer_base.
// We use Boost.TypeErasure because initializer_base is not itself
// polymorphic. This constrains the methods to accept specifically executor&
// rather than a template argument, but that should only be a problem for a
// user-coded initializer that expects an executor subclass. If such an
// initializer is stored in any_initializer, you might have to static_cast the
// argument to your executor type.
typedef
    boost::type_erasure::any<
        boost::mpl::vector<
            boost::type_erasure::copy_constructible<>,
            has_on_fork_setup  <void(executor&), const boost::type_erasure::_self>,
            has_on_fork_error  <void(executor&), const boost::type_erasure::_self>,
            has_on_fork_success<void(executor&), const boost::type_erasure::_self>,
            has_on_exec_setup  <void(executor&), const boost::type_erasure::_self>,
            has_on_exec_error  <void(executor&), const boost::type_erasure::_self>,
            boost::type_erasure::relaxed> >
    any_initializer;

// sequence_wrapper_ adapts any copyable runtime container acceptable to
// boost::range::for_each() -- e.g. std::vector<any_initializer> -- for use as
// an execute() initializer.
template <typename Sequence>
struct sequence_wrapper_: public initializer_base
{
    sequence_wrapper_(const Sequence& c): sequence_(c) {}

    Sequence sequence_;

    template <class PosixExecutor>
    void on_fork_setup(PosixExecutor& e) const
    {
        boost::range::for_each(sequence_, typename PosixExecutor::call_on_fork_setup(e));
    }

    template <class PosixExecutor>
    void on_fork_error(PosixExecutor& e) const
    {
        boost::range::for_each(sequence_, typename PosixExecutor::call_on_fork_error(e));
    }

    template <class PosixExecutor>
    void on_fork_success(PosixExecutor& e) const
    {
        boost::range::for_each(sequence_, typename PosixExecutor::call_on_fork_success(e));
    }

    template <class PosixExecutor>
    void on_exec_setup(PosixExecutor& e) const
    {
        boost::range::for_each(sequence_, typename PosixExecutor::call_on_exec_setup(e));
    }

    template <class PosixExecutor>
    void on_exec_error(PosixExecutor& e) const
    {
        boost::range::for_each(sequence_, typename PosixExecutor::call_on_exec_error(e));
    }
};

// sequence() is a convenience function to construct a sequence_wrapper_<T>
// without having to state T.
template <typename Sequence>
sequence_wrapper_<Sequence>
sequence(const Sequence& c)
{
    return sequence_wrapper_<Sequence>(c);
}

}}}}

#endif
