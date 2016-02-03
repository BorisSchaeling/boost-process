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

#include <boost/process/windows/initializers/initializer_base.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/member.hpp>
#include <boost/type_erasure/relaxed.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/range/algorithm/for_each.hpp>

// Define initializer_base methods to Boost.TypeErasure. The convenience macro
// must be used in the global namespace.
BOOST_TYPE_ERASURE_MEMBER((boost)(process)(windows)(initializers)(has_on_CreateProcess_setup),
                                                                      on_CreateProcess_setup, 1)
BOOST_TYPE_ERASURE_MEMBER((boost)(process)(windows)(initializers)(has_on_CreateProcess_error),
                                                                      on_CreateProcess_error, 1)
BOOST_TYPE_ERASURE_MEMBER((boost)(process)(windows)(initializers)(has_on_CreateProcess_success),
                                                                      on_CreateProcess_success, 1)

namespace boost { namespace process { namespace windows { namespace initializers {

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
            has_on_CreateProcess_setup  <void(executor&), const boost::type_erasure::_self>,
            has_on_CreateProcess_error  <void(executor&), const boost::type_erasure::_self>,
            has_on_CreateProcess_success<void(executor&), const boost::type_erasure::_self>,
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

    template <class WindowsExecutor>
    void on_CreateProcess_setup(WindowsExecutor& e) const
    {
        boost::range::for_each(sequence_, typename WindowsExecutor::call_on_CreateProcess_setup(e));
    }

    template <class WindowsExecutor>
    void on_CreateProcess_error(WindowsExecutor& e) const
    {
        boost::range::for_each(sequence_, typename WindowsExecutor::call_on_CreateProcess_error(e));
    }

    template <class WindowsExecutor>
    void on_CreateProcess_success(WindowsExecutor& e) const
    {
        boost::range::for_each(sequence_, typename WindowsExecutor::call_on_CreateProcess_success(e));
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
