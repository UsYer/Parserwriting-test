// -*- c++ -*-
/* $Id$ */

/* Copyright (C) 2002 The gtkmm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "ustring.h"
#include "utf8.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <boost/locale.hpp>

const utf8::ustring::size_type utf8::ustring::npos = std::string::npos;

namespace
{

using utf8::ustring;

// Little helper to make the conversion from char32_t to UTF-8 a one-liner.
//
struct UnicharToUtf8
{
    char                buf[6];
    ustring::size_type  len;

    explicit UnicharToUtf8(char32_t uc)
        : len (0)
    {
        auto one_pass_end_it = utf8::utf32to8(&uc,(&uc)+1,&buf[0]);
        len = one_pass_end_it - &buf[0];
    }
};


// All utf8_*_offset() functions return npos if offset is out of range.
// The caller should decide if npos is a valid argument and just marks
// the whole string, or if it is not allowed (e.g. for start positions).
// In the latter case std::out_of_range should be thrown, but usually
// std::string will do that for us.

// First overload: stop on '\0' character.
static
ustring::size_type utf8_byte_offset(const char* str, ustring::size_type offset)
{
    if(offset == ustring::npos)
        return ustring::npos;

    const char *const utf8_skip = utf8::g_utf8_skip;
    const char* p = str;

    for(; offset != 0; --offset)
    {
        const unsigned int c = static_cast<unsigned char>(*p);

        if(c == 0)
            return ustring::npos;

        p += utf8_skip[c];
    }

    return (p - str);
}

// Second overload: stop when reaching maxlen.
static
ustring::size_type utf8_byte_offset(const char* str, ustring::size_type offset,
                                    ustring::size_type maxlen)
{
    if(offset == ustring::npos)
        return ustring::npos;

    const char *const utf8_skip = utf8::g_utf8_skip;
    const char *const pend = str + maxlen;
    const char* p = str;

    for(; offset != 0; --offset)
    {
        if(p >= pend)
            return ustring::npos;

        p += utf8_skip[static_cast<unsigned char>(*p)];
    }

    return (p - str);
}

// Third overload: stop when reaching str.size().
//
inline
ustring::size_type utf8_byte_offset(const std::string& str, ustring::size_type offset)
{
    return utf8_byte_offset(str.data(), offset, str.size());
}

// Takes UTF-8 character offset and count in ci and cn.
// Returns the byte offset and count in i and n.
//
struct Utf8SubstrBounds
{
    ustring::size_type i;
    ustring::size_type n;

    Utf8SubstrBounds(const std::string& str, ustring::size_type ci, ustring::size_type cn)
        :
        i (utf8_byte_offset(str, ci)),
        n (ustring::npos)
    {
        if(i != ustring::npos)
            n = utf8_byte_offset(str.data() + i, cn, str.size() - i);
    }
};

// Converts byte offset to UTF-8 character offset.
inline
ustring::size_type utf8_char_offset(const std::string& str, ustring::size_type offset)
{
    if(offset == ustring::npos)
        return ustring::npos;

    const char *const pdata = str.data();
    //return g_utf8_pointer_to_offset(pdata, pdata + offset);
    return utf8::distance(pdata, pdata + offset);
}


// Helper to implement ustring::find_first_of() and find_first_not_of().
// Returns the UTF-8 character offset, or ustring::npos if not found.
static
ustring::size_type utf8_find_first_of(const std::string& str, ustring::size_type offset,
                                      const char* utf8_match, long utf8_match_size,
                                      bool find_not_of)
{
    const ustring::size_type byte_offset = utf8_byte_offset(str, offset);
    if(byte_offset == ustring::npos)
        return ustring::npos;

    //long ucs4_match_size = 0;
    //const utf8::ScopedPtr<gunichar> ucs4_match(g_utf8_to_ucs4_fast(utf8_match, utf8_match_size, &ucs4_match_size));
    std::u32string match;
    utf8::utf8to32(utf8_match,utf8_match+utf8_match_size,std::back_inserter(match));

    const char32_t * match_begin = match.data();
    const char32_t * match_end   = match_begin + match.size();

    const char * str_begin = str.data();
    const char * str_end   = str_begin + str.size();

    /*  for(const char* pstr = str_begin + byte_offset;
          pstr < str_end;
          pstr = g_utf8_next_char(pstr))
    	  //utf8::advance(pstr, 1, str_end))
      {
        const char32_t *const pfound = std::find(match_begin, match_end, g_utf8_get_char(pstr));

        if((pfound != match_end) != find_not_of)
          return offset;

        ++offset;
      }*/
    utf8::iterator<const char *> endit(str_end, str_begin + byte_offset, str_end);
    for( utf8::iterator<const char *> pstr(str_begin, str_begin + byte_offset, str_end); pstr != endit ; ++pstr )
    {
        const char32_t *const pfound = std::find(match_begin, match_end, *pstr);

        if((pfound != match_end) != find_not_of)
            return offset;

        ++offset;
    }
    return ustring::npos;
}

// Helper to implement ustring::find_last_of() and find_last_not_of().
// Returns the UTF-8 character offset, or ustring::npos if not found.
static
ustring::size_type utf8_find_last_of(const std::string& str, ustring::size_type offset,
                                     const char* utf8_match, long utf8_match_size,
                                     bool find_not_of)
{
    //long ucs4_match_size = 0;
//const utf8::ScopedPtr<gunichar> ucs4_match(g_utf8_to_ucs4_fast(utf8_match, utf8_match_size, &ucs4_match_size));
    std::u32string match;
    auto one_pass_end_it = utf8::utf8to32(utf8_match,utf8_match+utf8_match_size,std::back_inserter(match));

    const char32_t *const match_begin =  match.data();
    const char32_t *const match_end   =  match_begin + match.size();
    //const char32_t *const match_begin = ucs4_match.get();
    //const char32_t *const match_end   = match_begin + ucs4_match_size;

    const char *const str_begin = str.data();
    const char* pstr = str_begin;

    // Set pstr one byte beyond the actual start position.
    const ustring::size_type byte_offset = utf8_byte_offset(str, offset);
    pstr += (byte_offset < str.size()) ? byte_offset + 1 : str.size();

    while(pstr > str_begin)
    {
        // Move to previous character.
        do
            --pstr;
        while((static_cast<unsigned char>(*pstr) & 0xC0u) == 0x80);

        //const char32_t *const pfound = std::find(match_begin, match_end, g_utf8_get_char(pstr));
        utf8::iterator<const char*> u32_it(pstr, str_begin, pstr);
        const char32_t *const pfound = std::find(match_begin, match_end, *u32_it);

        if((pfound != match_end) != find_not_of)
        {
            //return g_utf8_pointer_to_offset(str_begin, pstr);
            return utf8::distance(str_begin,pstr);
        }
    }

    return ustring::npos;
}

} // anonymous namespace


namespace utf8
{

/*#ifndef GLIBMM_HAVE_ALLOWS_STATIC_INLINE_NPOS
// Initialize static member here,
// because the compiler did not allow us do it inline.
const ustring::size_type ustring::npos = std::string::npos;
#endif*/

/*
 * We need our own version of g_utf8_get_char(), because the std::string
 * iterator is not necessarily a plain pointer (it's in fact not in GCC's
 * libstdc++-v3).  Copying the UTF-8 data into a temporary buffer isn't an
 * option since this operation is quite time critical.  The implementation
 * is quite different from g_utf8_get_char() -- both more generic and likely
 * faster.
 *
 * By looking at the first byte of a UTF-8 character one can determine the
 * number of bytes used.  GLib offers the g_utf8_skip[] array for this purpose,
 * but accessing this global variable would, on IA32 at least, introduce
 * a function call to fetch the Global Offset Table, plus two levels of
 * indirection in order to read the value.  Even worse, fetching the GOT is
 * always done right at the start of the function instead of the branch that
 * actually uses the variable.
 *
 * Fortunately, there's a better way to get the byte count.  As this table
 * shows, there's a nice regular pattern in the UTF-8 encoding scheme:
 *
 * 0x00000000 - 0x0000007F: 0xxxxxxx
 * 0x00000080 - 0x000007FF: 110xxxxx 10xxxxxx
 * 0x00000800 - 0x0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
 * 0x00010000 - 0x001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0x00200000 - 0x03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 * 0x04000000 - 0x7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * Except for the single byte case, the number of leading 1-bits equals the
 * byte count.  All that is needed is to shift the first byte to the left
 * until bit 7 becomes 0.  Naturally, doing so requires a loop -- but since
 * we already have one, no additional cost is introduced.  This shifting can
 * further be combined with the computation of the bitmask needed to eliminate
 * the leading length bits, thus saving yet another register.
 *
 * Note:  If you change this code, it is advisable to also review what the
 * compiler makes of it in the assembler output.  Except for some pointless
 * register moves, the generated code is sufficiently close to the optimum
 * with GCC 4.1.2 on x86_64.
 */
char32_t get_unichar_from_std_iterator(std::string::const_iterator pos)
{
    unsigned int result = static_cast<unsigned char>(*pos);

    if((result & 0x80) != 0)
    {
        unsigned int mask = 0x40;

        do
        {
            result <<= 6;
            const unsigned int c = static_cast<unsigned char>(*++pos);
            mask   <<= 5;
            result  += c - 0x80;
        }
        while((result & mask) != 0);

        result &= mask - 1;
    }

    return result;
}


/**** utf8::ustring ********************************************************/

ustring::ustring()
    :
    string_ ()
{}

ustring::ustring(const ustring& other)
    :
    string_ (other.string_)
{}

ustring::ustring(const ustring& src, ustring::size_type i, ustring::size_type n)
    :
    string_ ()
{
    const Utf8SubstrBounds bounds (src.string_, i, n);
    string_.assign(src.string_, bounds.i, bounds.n);
}

ustring::ustring(const char* src, ustring::size_type n)
    :
    string_ (src, utf8_byte_offset(src, n))
{}

ustring::ustring(const char* src)
    :
    string_ (src)
{}

ustring::ustring(ustring::size_type n, char32_t uc)
    :
    string_ ()
{
    if(uc < 0x80)
    {
        // Optimize the probably most common case.
        string_.assign(n, static_cast<char>(uc));
    }
    else
    {
        const UnicharToUtf8 conv (uc);
        string_.reserve(n * conv.len);

        for(; n > 0; --n)
            string_.append(conv.buf, conv.len);
    }
}

ustring::ustring(ustring::size_type n, char c)
    :
    string_ (n, c)
{}

ustring::ustring(const std::string& src)
    :
    string_ (src)
{}

ustring::~ustring()
{}

void ustring::swap(ustring& other)
{
    string_.swap(other.string_);
}


/**** utf8::ustring::operator=() *******************************************/

ustring& ustring::operator=(const ustring& other)
{
    string_ = other.string_;
    return *this;
}

ustring& ustring::operator=(const std::string& src)
{
    string_ = src;
    return *this;
}

ustring& ustring::operator=(const char* src)
{
    string_ = src;
    return *this;
}

ustring& ustring::operator=(char32_t uc)
{
    const UnicharToUtf8 conv (uc);
    string_.assign(conv.buf, conv.len);
    return *this;
}

ustring& ustring::operator=(char c)
{
    string_ = c;
    return *this;
}


/**** utf8::ustring::assign() **********************************************/

ustring& ustring::assign(const ustring& src)
{
    string_ = src.string_;
    return *this;
}

ustring& ustring::assign(const ustring& src, ustring::size_type i, ustring::size_type n)
{
    const Utf8SubstrBounds bounds (src.string_, i, n);
    string_.assign(src.string_, bounds.i, bounds.n);
    return *this;
}

ustring& ustring::assign(const char* src, ustring::size_type n)
{
    string_.assign(src, utf8_byte_offset(src, n));
    return *this;
}

ustring& ustring::assign(const char* src)
{
    string_ = src;
    return *this;
}

ustring& ustring::assign(ustring::size_type n, char32_t uc)
{
    ustring temp (n, uc);
    string_.swap(temp.string_);
    return *this;
}

ustring& ustring::assign(ustring::size_type n, char c)
{
    string_.assign(n, c);
    return *this;
}


/**** utf8::ustring::operator+=() ******************************************/

ustring& ustring::operator+=(const ustring& src)
{
    string_ += src.string_;
    return *this;
}

ustring& ustring::operator+=(const char* src)
{
    string_ += src;
    return *this;
}

ustring& ustring::operator+=(char32_t uc)
{
    const UnicharToUtf8 conv (uc);
    string_.append(conv.buf, conv.len);
    return *this;
}

ustring& ustring::operator+=(char c)
{
    string_ += c;
    return *this;
}


/**** utf8::ustring::push_back() *******************************************/

void ustring::push_back(char32_t uc)
{
    const UnicharToUtf8 conv (uc);
    string_.append(conv.buf, conv.len);
}

void ustring::push_back(char c)
{
    string_ += c;
}


/**** utf8::ustring::append() **********************************************/

ustring& ustring::append(const ustring& src)
{
    string_ += src.string_;
    return *this;
}

ustring& ustring::append(const ustring& src, ustring::size_type i, ustring::size_type n)
{
    const Utf8SubstrBounds bounds (src.string_, i, n);
    string_.append(src.string_, bounds.i, bounds.n);
    return *this;
}

ustring& ustring::append(const char* src, ustring::size_type n)
{
    string_.append(src, utf8_byte_offset(src, n));
    return *this;
}

ustring& ustring::append(const char* src)
{
    string_ += src;
    return *this;
}

ustring& ustring::append(ustring::size_type n, char32_t uc)
{
    string_.append(ustring(n, uc).string_);
    return *this;
}

ustring& ustring::append(ustring::size_type n, char c)
{
    string_.append(n, c);
    return *this;
}


/**** utf8::ustring::insert() **********************************************/

ustring& ustring::insert(ustring::size_type i, const ustring& src)
{
    string_.insert(utf8_byte_offset(string_, i), src.string_);
    return *this;
}

ustring& ustring::insert(ustring::size_type i, const ustring& src,
                         ustring::size_type i2, ustring::size_type n)
{
    const Utf8SubstrBounds bounds2 (src.string_, i2, n);
    string_.insert(utf8_byte_offset(string_, i), src.string_, bounds2.i, bounds2.n);
    return *this;
}

ustring& ustring::insert(ustring::size_type i, const char* src, ustring::size_type n)
{
    string_.insert(utf8_byte_offset(string_, i), src, utf8_byte_offset(src, n));
    return *this;
}

ustring& ustring::insert(ustring::size_type i, const char* src)
{
    string_.insert(utf8_byte_offset(string_, i), src);
    return *this;
}

ustring& ustring::insert(ustring::size_type i, ustring::size_type n, char32_t uc)
{
    string_.insert(utf8_byte_offset(string_, i), ustring(n, uc).string_);
    return *this;
}

ustring& ustring::insert(ustring::size_type i, ustring::size_type n, char c)
{
    string_.insert(utf8_byte_offset(string_, i), n, c);
    return *this;
}

ustring::iterator ustring::insert(ustring::iterator p, char32_t uc)
{
    const size_type offset = p.base() - string_.begin();
    const UnicharToUtf8 conv (uc);
    string_.insert(offset, conv.buf, conv.len);
    return iterator(string_.begin() + offset);
}

ustring::iterator ustring::insert(ustring::iterator p, char c)
{
    return iterator(string_.insert(p.base(), c));
}

void ustring::insert(ustring::iterator p, ustring::size_type n, char32_t uc)
{
    string_.insert(p.base() - string_.begin(), ustring(n, uc).string_);
}

void ustring::insert(ustring::iterator p, ustring::size_type n, char c)
{
    string_.insert(p.base(), n, c);
}


/**** utf8::ustring::replace() *********************************************/

ustring& ustring::replace(ustring::size_type i, ustring::size_type n, const ustring& src)
{
    const Utf8SubstrBounds bounds (string_, i, n);
    string_.replace(bounds.i, bounds.n, src.string_);
    return *this;
}

ustring& ustring::replace(ustring::size_type i, ustring::size_type n,
                          const ustring& src, ustring::size_type i2, ustring::size_type n2)
{
    const Utf8SubstrBounds bounds (string_, i, n);
    const Utf8SubstrBounds bounds2 (src.string_, i2, n2);
    string_.replace(bounds.i, bounds.n, src.string_, bounds2.i, bounds2.n);
    return *this;
}

ustring& ustring::replace(ustring::size_type i, ustring::size_type n,
                          const char* src, ustring::size_type n2)
{
    const Utf8SubstrBounds bounds (string_, i, n);
    string_.replace(bounds.i, bounds.n, src, utf8_byte_offset(src, n2));
    return *this;
}

ustring& ustring::replace(ustring::size_type i, ustring::size_type n, const char* src)
{
    const Utf8SubstrBounds bounds (string_, i, n);
    string_.replace(bounds.i, bounds.n, src);
    return *this;
}

ustring& ustring::replace(ustring::size_type i, ustring::size_type n,
                          ustring::size_type n2, char32_t uc)
{
    const Utf8SubstrBounds bounds (string_, i, n);
    string_.replace(bounds.i, bounds.n, ustring(n2, uc).string_);
    return *this;
}

ustring& ustring::replace(ustring::size_type i, ustring::size_type n,
                          ustring::size_type n2, char c)
{
    const Utf8SubstrBounds bounds (string_, i, n);
    string_.replace(bounds.i, bounds.n, n2, c);
    return *this;
}

ustring& ustring::replace(ustring::iterator pbegin, ustring::iterator pend, const ustring& src)
{
    string_.replace(pbegin.base(), pend.base(), src.string_);
    return *this;
}

ustring& ustring::replace(ustring::iterator pbegin, ustring::iterator pend,
                          const char* src, ustring::size_type n)
{
    string_.replace(pbegin.base(), pend.base(), src, utf8_byte_offset(src, n));
    return *this;
}

ustring& ustring::replace(ustring::iterator pbegin, ustring::iterator pend, const char* src)
{
    string_.replace(pbegin.base(), pend.base(), src);
    return *this;
}

ustring& ustring::replace(ustring::iterator pbegin, ustring::iterator pend,
                          ustring::size_type n, char32_t uc)
{
    string_.replace(pbegin.base(), pend.base(), ustring(n, uc).string_);
    return *this;
}

ustring& ustring::replace(ustring::iterator pbegin, ustring::iterator pend,
                          ustring::size_type n, char c)
{
    string_.replace(pbegin.base(), pend.base(), n, c);
    return *this;
}


/**** utf8::ustring::erase() ***********************************************/

void ustring::clear()
{
    string_.erase();
}

ustring& ustring::erase(ustring::size_type i, ustring::size_type n)
{
    const Utf8SubstrBounds bounds (string_, i, n);
    string_.erase(bounds.i, bounds.n);
    return *this;
}

ustring& ustring::erase()
{
    string_.erase();
    return *this;
}

ustring::iterator ustring::erase(ustring::iterator p)
{
    ustring::iterator iter_end = p;
    ++iter_end;

    return iterator(string_.erase(p.base(), iter_end.base()));
}

ustring::iterator ustring::erase(ustring::iterator pbegin, ustring::iterator pend)
{
    return iterator(string_.erase(pbegin.base(), pend.base()));
}


/**** utf8::ustring::compare() *********************************************/
//TODO: implement normalization, see http://git.gnome.org/browse/glib/tree/glib/gunidecomp.c#n359
//for implementation details on _g_utf8_normalize_wc
int ustring::compare(const ustring& rhs) const
{
    //return g_utf8_collate(string_.c_str(), rhs.string_.c_str());
    /*gunichar *str1_norm;
    gunichar *str2_norm;

    str1_norm = _g_utf8_normalize_wc (str1, -1, G_NORMALIZE_ALL_COMPOSE);
    str2_norm = _g_utf8_normalize_wc (str2, -1, G_NORMALIZE_ALL_COMPOSE);*/

    /*#ifdef PLAT_WIN //wchar_t is 16 bit
    std::u16string this_buffer, rhs_buffer;
    utf8::utf8to16(string_.begin(),string_.end(),std::back_inserter(this_buffer));
    utf8::utf8to16(rhs.string_.begin(),rhs.string_.end(),std::back_inserter(rhs_buffer));

    #else //wchar_t is 32 bit
    std::u32string this_buffer, rhs_buffer;
    utf8::utf8to32(string_.begin(),string_.end(),std::back_inserter(this_buffer));
    utf8::utf8to32(rhs.string_.begin(),rhs.string_.end(),std::back_inserter(rhs_buffer));
    #endif

    return wcscoll ((const wchar_t *)this_buffer.c_str(), (const wchar_t *)rhs_buffer.c_str());*/

    boost::locale::generator gen;
    return std::use_facet<boost::locale::collator<char>>(gen("")).compare(boost::locale::collator_base::quaternary,string_, rhs.string_);
    /*g_free (str1_norm);
    g_free (str2_norm);*/
}

int ustring::compare(const char* rhs) const
{
    //return g_utf8_collate(string_.c_str(), rhs);
    return compare(ustring(rhs));
}

int ustring::compare(ustring::size_type i, ustring::size_type n, const ustring& rhs) const
{
    return ustring(*this, i, n).compare(rhs);
}

int ustring::compare(ustring::size_type i, ustring::size_type n,
                     const ustring& rhs, ustring::size_type i2, ustring::size_type n2) const
{
    return ustring(*this, i, n).compare(ustring(rhs, i2, n2));
}

int ustring::compare(ustring::size_type i, ustring::size_type n,
                     const char* rhs, ustring::size_type n2) const
{
    return ustring(*this, i, n).compare(ustring(rhs, n2));
}

int ustring::compare(ustring::size_type i, ustring::size_type n, const char* rhs) const
{
    return ustring(*this, i, n).compare(rhs);
}


/**** utf8::ustring -- index access ****************************************/

ustring::value_type ustring::operator[](ustring::size_type i) const
{
    //return g_utf8_get_char(g_utf8_offset_to_pointer(string_.data(), i));
    auto begin_it = string_.cbegin(), end_it = string_.cend();
    auto octet_it = begin_it;
    utf8::advance (octet_it, i, end_it);
    utf8::iterator<std::string::const_iterator> it(octet_it,begin_it,end_it);
    return *it;
}

ustring::value_type ustring::at(ustring::size_type i) const
{
    //convert from number of codepoints to number of bytes
    const size_type byte_offset = utf8_byte_offset(string_, i);
    if( byte_offset >= string_.length() )
        throw std::out_of_range("error: index out of range");
    // Throws std::out_of_range if the index is invalid.
    //return g_utf8_get_char(&string_.at(byte_offset));
    return (*this)[i];
}


/**** utf8::ustring -- iterator access *************************************/

ustring::iterator ustring::begin()
{
    return iterator(string_.begin());
}

ustring::iterator ustring::end()
{
    return iterator(string_.end());
}

ustring::const_iterator ustring::begin() const
{
    return const_iterator(string_.begin());
}

ustring::const_iterator ustring::end() const
{
    return const_iterator(string_.end());
}

ustring::reverse_iterator ustring::rbegin()
{
    return reverse_iterator(iterator(string_.end()));
}

ustring::reverse_iterator ustring::rend()
{
    return reverse_iterator(iterator(string_.begin()));
}

ustring::const_reverse_iterator ustring::rbegin() const
{
    return const_reverse_iterator(const_iterator(string_.end()));
}

ustring::const_reverse_iterator ustring::rend() const
{
    return const_reverse_iterator(const_iterator(string_.begin()));
}


/**** utf8::ustring::find() ************************************************/

ustring::size_type ustring::find(const ustring& str, ustring::size_type i) const
{
    return utf8_char_offset(string_, string_.find(str.string_, utf8_byte_offset(string_, i)));
}

ustring::size_type ustring::find(const char* str, ustring::size_type i, ustring::size_type n) const
{
    return utf8_char_offset(string_, string_.find(str, utf8_byte_offset(string_, i),
                            utf8_byte_offset(str, n)));
}

ustring::size_type ustring::find(const char* str, ustring::size_type i) const
{
    return utf8_char_offset(string_, string_.find(str, utf8_byte_offset(string_, i)));
}

ustring::size_type ustring::find(char32_t uc, ustring::size_type i) const
{
    const UnicharToUtf8 conv (uc);
    return utf8_char_offset(string_, string_.find(conv.buf, utf8_byte_offset(string_, i), conv.len));
}

ustring::size_type ustring::find(char c, ustring::size_type i) const
{
    return utf8_char_offset(string_, string_.find(c, utf8_byte_offset(string_, i)));
}


/**** utf8::ustring::rfind() ***********************************************/

ustring::size_type ustring::rfind(const ustring& str, ustring::size_type i) const
{
    return utf8_char_offset(string_, string_.rfind(str.string_, utf8_byte_offset(string_, i)));
}

ustring::size_type ustring::rfind(const char* str, ustring::size_type i,
                                  ustring::size_type n) const
{
    return utf8_char_offset(string_, string_.rfind(str, utf8_byte_offset(string_, i),
                            utf8_byte_offset(str, n)));
}

ustring::size_type ustring::rfind(const char* str, ustring::size_type i) const
{
    return utf8_char_offset(string_, string_.rfind(str, utf8_byte_offset(string_, i)));
}

ustring::size_type ustring::rfind(char32_t uc, ustring::size_type i) const
{
    const UnicharToUtf8 conv (uc);
    return utf8_char_offset(string_, string_.rfind(conv.buf, utf8_byte_offset(string_, i), conv.len));
}

ustring::size_type ustring::rfind(char c, ustring::size_type i) const
{
    return utf8_char_offset(string_, string_.rfind(c, utf8_byte_offset(string_, i)));
}


/**** utf8::ustring::find_first_of() ***************************************/

ustring::size_type ustring::find_first_of(const ustring& match, ustring::size_type i) const
{
    return utf8_find_first_of(string_, i, match.string_.data(), match.string_.size(), false);
}

ustring::size_type ustring::find_first_of(const char* match,
        ustring::size_type i, ustring::size_type n) const
{
    return utf8_find_first_of(string_, i, match, n, false);
}

ustring::size_type ustring::find_first_of(const char* match, ustring::size_type i) const
{
    return utf8_find_first_of(string_, i, match, -1, false);
}

ustring::size_type ustring::find_first_of(char32_t uc, ustring::size_type i) const
{
    return find(uc, i);
}

ustring::size_type ustring::find_first_of(char c, ustring::size_type i) const
{
    return find(c, i);
}


/**** utf8::ustring::find_last_of() ****************************************/

ustring::size_type ustring::find_last_of(const ustring& match, ustring::size_type i) const
{
    return utf8_find_last_of(string_, i, match.string_.data(), match.string_.size(), false);
}

ustring::size_type ustring::find_last_of(const char* match,
        ustring::size_type i, ustring::size_type n) const
{
    return utf8_find_last_of(string_, i, match, n, false);
}

ustring::size_type ustring::find_last_of(const char* match, ustring::size_type i) const
{
    return utf8_find_last_of(string_, i, match, -1, false);
}

ustring::size_type ustring::find_last_of(char32_t uc, ustring::size_type i) const
{
    return rfind(uc, i);
}

ustring::size_type ustring::find_last_of(char c, ustring::size_type i) const
{
    return rfind(c, i);
}


/**** utf8::ustring::find_first_not_of() ***********************************/

ustring::size_type ustring::find_first_not_of(const ustring& match, ustring::size_type i) const
{
    return utf8_find_first_of(string_, i, match.string_.data(), match.string_.size(), true);
}

ustring::size_type ustring::find_first_not_of(const char* match,
        ustring::size_type i, ustring::size_type n) const
{
    return utf8_find_first_of(string_, i, match, n, true);
}

ustring::size_type ustring::find_first_not_of(const char* match, ustring::size_type i) const
{
    return utf8_find_first_of(string_, i, match, -1, true);
}

// Unfortunately, all of the find_*_not_of() methods for single
// characters need their own special implementation.
//
ustring::size_type ustring::find_first_not_of(char32_t uc, ustring::size_type i) const
{
    const size_type bi = utf8_byte_offset(string_, i);
    if(bi != npos)
    {
        const char * pbegin = string_.data();
        const char * pend   = pbegin + string_.size();

        /*for(const char* p = pbegin + bi;
            p < pend;
            p = g_utf8_next_char(p), ++i)
        {
          if(g_utf8_get_char(p) != uc)
            return i;
        }*/
        const char* p = pbegin + bi;
        utf8::iterator<const char *> endit(pend, p, pend);
        for( utf8::iterator<const char *> pstr(p, p, pend); pstr != endit ; ++pstr, ++i )
        {
            if(*pstr != uc)
                return i;
        }
    }
    return npos;
}

ustring::size_type ustring::find_first_not_of(char c, ustring::size_type i) const
{
    const size_type bi = utf8_byte_offset(string_, i);
    if(bi != npos)
    {
        const char * pbegin = string_.data();
        const char * pend   = pbegin + string_.size();

        /*for(const char* p = pbegin + bi;
            p < pend;
            p = g_utf8_next_char(p), ++i)
        {
          if(*p != c)
            return i;
        }*/
        const char* p = pbegin + bi;
        utf8::iterator<const char *> endit(pend, p, pend);
        for( utf8::iterator<const char *> pstr(p, p, pend); pstr != endit ; ++pstr, ++i )
        {
            if(*(pstr.base()) != c)
                return i;
        }
    }
    return npos;
}


/**** utf8::ustring::find_last_not_of() ************************************/

ustring::size_type ustring::find_last_not_of(const ustring& match, ustring::size_type i) const
{
    return utf8_find_last_of(string_, i, match.string_.data(), match.string_.size(), true);
}

ustring::size_type ustring::find_last_not_of(const char* match,
        ustring::size_type i, ustring::size_type n) const
{
    return utf8_find_last_of(string_, i, match, n, true);
}

ustring::size_type ustring::find_last_not_of(const char* match, ustring::size_type i) const
{
    return utf8_find_last_of(string_, i, match, -1, true);
}

// Unfortunately, all of the find_*_not_of() methods for single
// characters need their own special implementation.
//
ustring::size_type ustring::find_last_not_of(char32_t uc, ustring::size_type i) const
{
    const char * pbegin = string_.data();
    const char * pend   = pbegin + string_.size();
    size_type i_cur   = 0;
    size_type i_found = npos;

    /*for(const char* p = pbegin;
        p < pend && i_cur <= i;
        p = g_utf8_next_char(p), ++i_cur)
    {
      if(g_utf8_get_char(p) != uc)
        i_found = i_cur;
    }*/
    utf8::iterator<const char *> endit(pend, pbegin, pend);
    for( utf8::iterator<const char *> pstr(pbegin, pbegin, pend);
            pstr != endit && i_cur <= i;
            ++pstr, ++i_cur )
    {
        if(*pstr != uc)
            i_found = i_cur;
    }
    return i_found;
}

ustring::size_type ustring::find_last_not_of(char c, ustring::size_type i) const
{
    const char * pbegin = string_.data();
    const char * pend   = pbegin + string_.size();
    size_type i_cur   = 0;
    size_type i_found = npos;

    /*for(const char* p = pbegin;
        p < pend && i_cur <= i;
        p = g_utf8_next_char(p), ++i_cur)
    {
      if(*p != c)
        i_found = i_cur;
    }*/
    utf8::iterator<const char *> endit(pend, pbegin, pend);
    for( utf8::iterator<const char *> pstr(pbegin, pbegin, pend);
            pstr != endit && i_cur <= i;
            ++pstr, ++i_cur )
    {
        if(*(pstr.base()) != c)
            i_found = i_cur;
    }
    return i_found;
}


/**** utf8::ustring -- get size and resize *********************************/

bool ustring::empty() const
{
    return string_.empty();
}

ustring::size_type ustring::size() const
{
    //const char *const pdata = string_.data();
    //return g_utf8_pointer_to_offset(pdata, pdata + string_.size());
    return utf8::distance(string_.begin(), string_.end());
}

ustring::size_type ustring::length() const
{
    //const char *const pdata = string_.data();
    //return g_utf8_pointer_to_offset(pdata, pdata + string_.size());
    return utf8::distance(string_.begin(), string_.end());
}

ustring::size_type ustring::bytes() const
{
    return string_.size();
}

ustring::size_type ustring::capacity() const
{
    return string_.capacity();
}

ustring::size_type ustring::max_size() const
{
    return string_.max_size();
}

void ustring::resize(ustring::size_type n, char32_t uc)
{
    const size_type size_now = size();
    if(n < size_now)
        erase(n, npos);
    else if(n > size_now)
        append(n - size_now, uc);
}

void ustring::resize(ustring::size_type n, char c)
{
    const size_type size_now = size();
    if(n < size_now)
        erase(n, npos);
    else if(n > size_now)
        string_.append(n - size_now, c);
}

void ustring::reserve(ustring::size_type n)
{
    string_.reserve(n);
}


/**** utf8::ustring -- C string access *************************************/

const char* ustring::data() const
{
    return string_.data();
}

const char* ustring::c_str() const
{
    return string_.c_str();
}

// Note that copy() requests UTF-8 character offsets as
// parameters, but returns the number of copied bytes.
//
ustring::size_type ustring::copy(char* dest, ustring::size_type n, ustring::size_type i) const
{
    const Utf8SubstrBounds bounds (string_, i, n);
    return string_.copy(dest, bounds.n, bounds.i);
}


/**** utf8::ustring -- UTF-8 utilities *************************************/

bool ustring::validate() const
{
    //return (g_utf8_validate(string_.data(), string_.size(), 0) != 0);
    return utf8::is_valid(string_.begin(), string_.end());
}

bool ustring::validate(ustring::iterator& first_invalid)
{
    /*const char * pdata = string_.data();
    const char* valid_end = pdata;
    const int is_valid = g_utf8_validate(pdata, string_.size(), &valid_end);

    first_invalid = iterator(string_.begin() + (valid_end - pdata));*/
    auto invalid_it = utf8::find_invalid(string_.begin(), string_.end());
    bool valid = invalid_it == string_.end();
    first_invalid = ustring::iterator(invalid_it);
    return valid;
}

bool ustring::validate(ustring::const_iterator& first_invalid) const
{
    /*const char *const pdata = string_.data();
    const char* valid_end = pdata;
    const int is_valid = g_utf8_validate(pdata, string_.size(), &valid_end);

    first_invalid = const_iterator(string_.begin() + (valid_end - pdata));
    return (is_valid != 0);*/
    auto invalid_it = utf8::find_invalid(string_.begin(), string_.end());
    bool valid = invalid_it == string_.end();
    first_invalid = ustring::const_iterator(invalid_it);
    return valid;
}

bool ustring::is_ascii() const
{
    const char* p = string_.data();
    const char *const pend = p + string_.size();

    for(; p != pend; ++p)
    {
        if((static_cast<unsigned char>(*p) & 0x80u) != 0)
            return false;
    }

    return true;
}

ustring ustring::normalize(NormalizeMode mode) const
{
    //const ScopedPtr<char> buf (g_utf8_normalize(string_.data(), string_.size(),
    //                                            static_cast<GNormalizeMode>(int(mode))));

    boost::locale::norm_type boost_norm;
    switch(mode)
    {
    case NORMALIZE_NFD:
        //case NORMALIZE_DEFAULT:
        boost_norm = boost::locale::norm_type::norm_nfd;
        break;
    case NORMALIZE_NFC:
        //case NORMALIZE_DEFAULT_COMPOSE:
        boost_norm = boost::locale::norm_type::norm_nfc;
        break;
    case NORMALIZE_NFKD:
        //case NORMALIZE_ALL :
        boost_norm = boost::locale::norm_type::norm_nfkd;
        break;
    case NORMALIZE_NFKC:
        //case NORMALIZE_ALL_COMPOSE:
        boost_norm = boost::locale::norm_type::norm_nfkc;
        break;
    default:
        throw std::invalid_argument("unkown normalization mode.");
    }
    return ustring(boost::locale::normalize(string_,boost_norm));
}

ustring ustring::uppercase() const
{
    //const ScopedPtr<char> buf (g_utf8_strup(string_.data(), string_.size()));
    //return ustring(buf.get());
	boost::locale::generator gen;
    return ustring(boost::locale::to_upper(string_,gen("")));
}

ustring ustring::lowercase() const
{
    //const ScopedPtr<char> buf (g_utf8_strdown(string_.data(), string_.size()));
    //return ustring(buf.get());
	boost::locale::generator gen;
    return ustring(boost::locale::to_lower(string_,gen("")));
}

ustring ustring::casefold() const
{
    //const ScopedPtr<char> buf (g_utf8_casefold(string_.data(), string_.size()));
    //return ustring(buf.get());
    return ustring(boost::locale::fold_case(string_));
}

std::string ustring::collate_key() const
{
    //const ScopedPtr<char> buf (g_utf8_collate_key(string_.data(), string_.size()));
    //return std::string(buf.get());
    boost::locale::generator gen;
    return std::use_facet<boost::locale::collator<char>>(gen("")).transform(boost::locale::collator_base::quaternary,string_);
}

std::string ustring::casefold_collate_key() const
{
    //char *const casefold_buf = g_utf8_casefold(string_.data(), string_.size());
    //char *const key_buf      = g_utf8_collate_key(casefold_buf, -1);
    //g_free(casefold_buf);
    //return std::string(ScopedPtr<char>(key_buf).get());
    return casefold().collate_key();
}

/**** utf8::ustring -- Message formatting **********************************/
/*
// static
ustring ustring::compose_argv(const utf8::ustring& fmt, int argc, const ustring* const* argv)
{
  std::string::size_type result_size = fmt.raw().size();

  // Guesstimate the final string size.
  for (int i = 0; i < argc; ++i)
    result_size += argv[i]->raw().size();

  std::string result;
  result.reserve(result_size);

  const char* const pfmt = fmt.raw().c_str();
  const char* start = pfmt;

  while (const char* const stop = std::strchr(start, '%'))
  {
    if (stop[1] == '%')
    {
      result.append(start, stop - start + 1);
      start = stop + 2;
    }
    else
    {
      const int index = Ascii::digit_value(stop[1]) - 1;

      if (index >= 0 && index < argc)
      {
        result.append(start, stop - start);
        result += argv[index]->raw();
        start = stop + 2;
      }
      else
      {
        const char* const next = (stop[1] != '\0') ? g_utf8_next_char(stop + 1) : (stop + 1);

        // Copy invalid substitutions literally to the output.
        result.append(start, next - start);

        g_warning("invalid substitution \"%s\" in fmt string \"%s\"",
                  result.c_str() + result.size() - (next - stop), pfmt);
        start = next;
      }
    }
  }

  result.append(start, pfmt + fmt.raw().size() - start);

  return result;
}
*/
/**** utf8::ustring::SequenceToString **************************************/

ustring::SequenceToString<utf8::ustring::iterator,char32_t>
::SequenceToString(utf8::ustring::iterator pbegin, utf8::ustring::iterator pend)
    :
    std::string(pbegin.base(), pend.base())
{}

ustring::SequenceToString<utf8::ustring::const_iterator,char32_t>
::SequenceToString(utf8::ustring::const_iterator pbegin, utf8::ustring::const_iterator pend)
    :
    std::string(pbegin.base(), pend.base())
{}

/**** utf8::ustring::FormatStream ******************************************/
//
//ustring::FormatStream::FormatStream()
//:
//  stream_ ()
//{}
//
//ustring::FormatStream::~FormatStream()
//{}
//
//ustring ustring::FormatStream::to_string() const
//{
//  GError* error = 0;
//
//#ifdef GLIBMM_HAVE_WIDE_STREAM
//  const std::wstring str = stream_.str();
//
//# if defined(__STDC_ISO_10646__) && SIZEOF_WCHAR_T == 4
//  // Avoid going through iconv if wchar_t always contains UCS-4.
//  glong n_bytes = 0;
//  const ScopedPtr<char> buf (g_ucs4_to_utf8(reinterpret_cast<const gunichar*>(str.data()),
//                                            str.size(), 0, &n_bytes, &error));
//# elif defined(G_OS_WIN32) && SIZEOF_WCHAR_T == 2
//  // Avoid going through iconv if wchar_t always contains UTF-16.
//  glong n_bytes = 0;
//  const ScopedPtr<char> buf (g_utf16_to_utf8(reinterpret_cast<const gunichar2*>(str.data()),
//                                             str.size(), 0, &n_bytes, &error));
//# else
//  gsize n_bytes = 0;
//  const ScopedPtr<char> buf (g_convert(reinterpret_cast<const char*>(str.data()),
//                                       str.size() * sizeof(std::wstring::value_type),
//                                       "UTF-8", "WCHAR_T", 0, &n_bytes, &error));
//# endif /* !(__STDC_ISO_10646__ || G_OS_WIN32) */
//
//#else /* !GLIBMM_HAVE_WIDE_STREAM */
//  const std::string str = stream_.str();
//
//  gsize n_bytes = 0;
//  const ScopedPtr<char> buf (g_locale_to_utf8(str.data(), str.size(), 0, &n_bytes, &error));
//#endif /* !GLIBMM_HAVE_WIDE_STREAM */
//
//  if (error)
//  {
//    utf8::Error::throw_exception(error);
//  }
//
//  return ustring(buf.get(), buf.get() + n_bytes);
//}
/**** utf8::ustring -- stream I/O operators ********************************/

std::istream& operator>>(std::istream& is, utf8::ustring& utf8_string)
{
    std::string str;
    is >> str;

    /*GError* error = 0;
    gsize n_bytes = 0;
    const ScopedPtr<char> buf (g_locale_to_utf8(str.data(), str.size(), 0, &n_bytes, &error));

    if (error)
    {
      utf8::Error::throw_exception(error);
    }*/
    boost::locale::generator gen;
    utf8_string = boost::locale::conv::to_utf<char>(str,gen(""));

    //utf8_string.assign(buf.get(), buf.get() + n_bytes);

    return is;
}

std::ostream& operator<<(std::ostream& os, const utf8::ustring& utf8_string)
{
    /*GError* error = 0;
    const ScopedPtr<char> buf (g_locale_from_utf8(utf8_string.raw().data(),
                                                  utf8_string.raw().size(), 0, 0, &error));
    if (error)
    {
      utf8::Error::throw_exception(error);
    }*/

    boost::locale::generator gen;
	std::locale current = gen("");
    std::string str = boost::locale::conv::from_utf<char>(utf8_string.data(),current);

    // This won't work if the string contains NUL characters.  Unfortunately,
    // std::ostream::write() ignores format flags, so we cannot use that.
    // The only option would be to create a temporary std::string.  However,
    // even then GCC's libstdc++-v3 prints only the characters up to the first
    // NUL.  Given this, there doesn't seem much of a point in allowing NUL in
    // formatted output.  The semantics would be unclear anyway: what's the
    // screen width of a NUL?
    //os << buf.get();
    os << str;

    return os;
}

#ifdef GLIBMM_HAVE_WIDE_STREAM

std::wistream& operator>>(std::wistream& is, ustring& utf8_string)
{
    GError* error = 0;

    std::wstring wstr;
    is >> wstr;

#if defined(__STDC_ISO_10646__) && SIZEOF_WCHAR_T == 4
    // Avoid going through iconv if wchar_t always contains UCS-4.
    glong n_bytes = 0;
    const ScopedPtr<char> buf (g_ucs4_to_utf8(reinterpret_cast<const gunichar*>(wstr.data()),
                               wstr.size(), 0, &n_bytes, &error));
#elif defined(G_OS_WIN32) && SIZEOF_WCHAR_T == 2
    // Avoid going through iconv if wchar_t always contains UTF-16.
    glong n_bytes = 0;
    const ScopedPtr<char> buf (g_utf16_to_utf8(reinterpret_cast<const gunichar2*>(wstr.data()),
                               wstr.size(), 0, &n_bytes, &error));
#else
    gsize n_bytes = 0;
    const ScopedPtr<char> buf (g_convert(reinterpret_cast<const char*>(wstr.data()),
                                         wstr.size() * sizeof(std::wstring::value_type),
                                         "UTF-8", "WCHAR_T", 0, &n_bytes, &error));
#endif // !(__STDC_ISO_10646__ || G_OS_WIN32)

    if (error)
    {
        utf8::Error::throw_exception(error);
    }

    utf8_string.assign(buf.get(), buf.get() + n_bytes);

    return is;
}

std::wostream& operator<<(std::wostream& os, const ustring& utf8_string)
{
    GError* error = 0;

#if defined(__STDC_ISO_10646__) && SIZEOF_WCHAR_T == 4
    // Avoid going through iconv if wchar_t always contains UCS-4.
    const ScopedPtr<gunichar> buf (g_utf8_to_ucs4(utf8_string.raw().data(),
                                   utf8_string.raw().size(), 0, 0, &error));
#elif defined(G_OS_WIN32) && SIZEOF_WCHAR_T == 2
    // Avoid going through iconv if wchar_t always contains UTF-16.
    const ScopedPtr<gunichar2> buf (g_utf8_to_utf16(utf8_string.raw().data(),
                                    utf8_string.raw().size(), 0, 0, &error));
#else
    // TODO: For some reason the conversion from UTF-8 to WCHAR_T doesn't work
    // with g_convert(), while iconv on the command line handles it just fine.
    // Maybe a bug in GLib?
    const ScopedPtr<char> buf (g_convert(utf8_string.raw().data(), utf8_string.raw().size(),
                                         "WCHAR_T", "UTF-8", 0, 0, &error));
#endif // !(__STDC_ISO_10646__ || G_OS_WIN32)

    if (error)
    {
        utf8::Error::throw_exception(error);
    }

    // This won't work if the string contains NUL characters.  Unfortunately,
    // std::wostream::write() ignores format flags, so we cannot use that.
    // The only option would be to create a temporary std::wstring.  However,
    // even then GCC's libstdc++-v3 prints only the characters up to the first
    // NUL.  Given this, there doesn't seem much of a point in allowing NUL in
    // formatted output.  The semantics would be unclear anyway: what's the
    // screen width of a NUL?
    os << reinterpret_cast<wchar_t*>(buf.get());

    return os;
}

#endif /* GLIBMM_HAVE_WIDE_STREAM */

} // namespace Glib
