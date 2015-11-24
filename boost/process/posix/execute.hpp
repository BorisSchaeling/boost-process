// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROCESS_POSIX_EXECUTE_HPP
#define BOOST_PROCESS_POSIX_EXECUTE_HPP

#include <boost/process/posix/executor.hpp>
#include <boost/process/posix/child.hpp>
#include <boost/fusion/tuple/make_tuple.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/ref.hpp>

namespace boost { namespace process { namespace posix {

// The following helpers are used to generate C++03 execute() overloads from
// execute(i0) through execute(i0, ..., i(BOOST_PROCESS_EXECUTE_INITIALIZERS-1)).

// template <class I0, class I1, ...> are handled by BOOST_PP_ENUM_PARAMS.

// execute() params 'const I0 &i0, const I1 &i1, ...'
#define execute_param(z, n, data) const I##n &i##n

// make_tuple() params 'boost::cref(i0), boost::cref(i1), ...'
#define tuple_param(z, n, data) boost::cref(i##n)

#define BOOST_PP_LOCAL_MACRO(n)                                         \
/*       <class I0, class I1, ...> */                                   \
template <BOOST_PP_ENUM_PARAMS(n, class I)>                             \
/*    execute(const I0 &i0, const I1 &i1, ...) */                       \
child execute(BOOST_PP_ENUM(n, execute_param, xx))                      \
{                                                                       \
    /*     executor()(boost::fusion::make_tuple(boost::cref(i0), boost::cref(i1), ...); */ \
    return executor()(boost::fusion::make_tuple(BOOST_PP_ENUM(n, tuple_param, xx))); \
}

#define BOOST_PP_LOCAL_LIMITS (1, BOOST_PROCESS_EXECUTE_INITIALIZERS)
#include BOOST_PP_LOCAL_ITERATE()
#undef execute_param
#undef tuple_param

}}}

#endif
