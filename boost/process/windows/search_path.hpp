// Copyright (c) 2006, 2007 Julio M. Merino Vidal
// Copyright (c) 2008 Ilya Sokolov, Boris Schaeling
// Copyright (c) 2009 Boris Schaeling
// Copyright (c) 2010 Felipe Tanus, Boris Schaeling
// Copyright (c) 2011, 2012 Jeff Flinn, Boris Schaeling
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PROCESS_WINDOWS_SEARCH_PATH_HPP
#define BOOST_PROCESS_WINDOWS_SEARCH_PATH_HPP

#include <boost/process/config.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/array.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <boost/detail/winapi/shell.hpp>



namespace boost { namespace process { namespace windows {

inline std::wstring search_path(
        const std::wstring &filename,
        std::wstring path = L"")
{
    if (path.empty())
    {
        path = ::_wgetenv(L"PATH");
        if (path.empty())
            BOOST_PROCESS_THROW(std::runtime_error(
                "Environment variable PATH not found"));
    }

    typedef boost::tokenizer<boost::char_separator<wchar_t>,
        std::wstring::const_iterator, std::wstring> tokenizer;
    boost::char_separator<wchar_t> sep(L";");
    tokenizer tok(path, sep);
    for (tokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
    {
        boost::filesystem::path p = *it;
        p /= filename;
        boost::array<std::wstring, 4> extensions =
            { L"", L".exe", L".com", L".bat" };
        for (boost::array<std::wstring, 4>::iterator it2 = extensions.begin();
            it2 != extensions.end(); ++it2)
        {
            boost::filesystem::path p2 = p;
            p2 += *it2;
            boost::system::error_code ec;
            bool file = boost::filesystem::is_regular_file(p2, ec);
            if (!ec && file &&
            		::boost::detail::winapi::sh_get_file_info(p2.c_str(), 0, 0, 0, ::boost::detail::winapi::SHGFI_EXETYPE_))
            {
                return p2.wstring();
            }
        }
    }
    return L"";
}

#if !defined( BOOST_NO_ANSI_APIS )
inline std::string search_path(
        const std::string &filename,
        std::string path = "")
{
    if (path.empty())
    {
        path = ::getenv("PATH");
        if (path.empty())
            BOOST_PROCESS_THROW(std::runtime_error(
                "Environment variable PATH not found"));
    }

    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(";");
    tokenizer tok(path, sep);
    for (tokenizer::iterator it = tok.begin(); it != tok.end(); ++it)
    {
        boost::filesystem::path p = *it;
        p /= filename;
        boost::array<std::string, 4> extensions =
            { "", ".exe", ".com", ".bat" };
        for (boost::array<std::string, 4>::iterator it2 = extensions.begin();
            it2 != extensions.end(); ++it2)
        {
            boost::filesystem::path p2 = p;
            p2 += *it2;
            boost::system::error_code ec;
            bool file = boost::filesystem::is_regular_file(p2, ec);
            if (!ec && file &&
            	::boost::detail::winapi::sh_get_file_info(p2.string().c_str(), 0, 0, 0, ::boost::detail::winapi::SHGFI_EXETYPE_))
            {
                return p2.string();
            }
        }
    }
    return "";
}
#endif //BOOST_NO_ANSI_APIS

}}}

#endif
