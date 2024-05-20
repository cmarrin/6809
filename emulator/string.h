/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#pragma once

#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>
#include <functional>
#include <stdarg.h>
#include "containers.h"

namespace m8r {

//
//  Class: String
//
//  String class that works on both Mac and ESP
//

class string {
public:
    static constexpr uint32_t DefaultFloatDigits = 6;

    string() : _size(1), _capacity(0) { }
    string(const uint8_t* s, int32_t len = -1) { *this = string(reinterpret_cast<const char*>(s), len); }
    string(const char* s, int32_t len = -1) : _size(0), _capacity(0)
    {
        if (!s) {
            return;
        }
        if (len == -1) {
            len = static_cast<int32_t>(strlen(s));
        }
        ensureCapacity(len + 1);
        if (len) {
            memcpy(_data, s, len);
        }
        _size = len + 1;
        _data[_size - 1] = '\0';
    }
    
    string(const string& other)
    {
        *this = other;
    };
    
    string(string&& other)
    {
        delete [ ] _data;
        _data = other._data;
        other._data = nullptr;
        _size = other._size;
        other._size = 0;
        _capacity = other._capacity;
        other._capacity = 0;
    }
    
    string(char c)
    {
        ensureCapacity(2);
        char* s = _data;
        s[0] = c;
        s[1] = '\0';
        _size = 2;
    }
    
    string(double, uint8_t decimalDigits = DefaultFloatDigits);
    string(int32_t);
    string(uint32_t);
    string(void*);

    ~string()
    {
        delete [ ] _data;
        _data = nullptr;
        _destroyed = true;
    };
    
    string& operator=(const string& other)
    {
    assert(!_destroyed && !other._destroyed);
        delete [ ] _data;
        _size = other._size;
        _capacity = other._capacity;
        if (!other._data) {
            _data = nullptr;
            return *this;
        }
        
        _data = new char[_capacity];
        assert(_data);
        if (_data) {
            memcpy(_data, other._data, _size);
        } else {
            _capacity = 0;
            _size = 1;
        }
        return *this;
    }
    
    string& operator=(string&& other)
    {
    assert(!_destroyed && !other._destroyed);
        if (this == &other) {
            return *this;
        }

        delete [ ] _data;

        _data = other._data;
        _size = other._size;
        _capacity = other._capacity;

        other._data = nullptr;
        other._size = 0;
        other._capacity = 0;

        return *this;
    }
    
    string& operator=(char c)
    {
        ensureCapacity(2);
        char* s = _data;
        s[0] = c;
        s[1] = '\0';
        _size = 2;
        return *this;
    }

    operator bool () { return !empty(); }
    
    const char& operator[](uint16_t i) const { assert(i < _size - 1); return _data[i]; };
    char& operator[](uint16_t i) { assert(i < _size - 1); return _data[i]; };
    const char& at(uint16_t i) const { assert(i < _size - 1); return _data[i]; };
    char& at(uint16_t i) { assert(i < _size - 1); return _data[i]; };
    
    char& back() { return at(size() - 1); }
    const char& back() const { return at(size() - 1); }

    char& front() { return at(0); }
    const char& front() const { return at(0); }

    uint16_t size() const { return _size ? (_size - 1) : 0; }
    bool empty() const { return _size <= 1; }
    void clear() { _size = 1; if (_data) _data[0] = '\0'; }
    string& operator+=(uint8_t c)
    {
        ensureCapacity(_size + 1);
        char* s = _data;
        s[_size - 1] = c;
        s[_size++] = '\0';
        return *this;
    }
    
    string& operator+=(char c)
    {
        ensureCapacity(_size + 1);
        char* s = _data;
        s[_size - 1] = c;
        s[_size] = '\0';
        _size += 1;
        return *this;
    }
    
    string& operator+=(const char* s)
    {
    assert(!_destroyed);
        uint16_t len = strlen(s);
        ensureCapacity(_size + len);
        memcpy(_data + _size - 1, s, len + 1);
        _size += len;
        return *this;
    }
    
    string& operator+=(const string& s) { assert(!_destroyed && !s._destroyed); return *this += s.c_str(); }
    
    friend string operator +(const string& s1 , const string& s2) { string s = s1; s += s2; return s; }
    friend string operator +(const string& s1 , const char* s2) { string s = s1; s += s2; return s; }
    friend string operator +(const string& s1 , char c) { string s = s1; s += c; return s; }
    friend string operator +(const char* s1 , const string& s2) { string s = s1; s += s2; return s; }
    
    bool operator<(const string& other) const { return compare(*this, other) < 0; }
    bool operator<=(const string& other) const { return compare(*this, other) <= 0; }
    bool operator>(const string& other) const { return compare(*this, other) > 0; }
    bool operator>=(const string& other) const { return compare(*this, other) >= 0; }
    bool operator==(const string& other) const { return compare(*this, other) == 0; }
    bool operator!=(const string& other) const { return compare(*this, other) != 0; }
    
    friend int compare(const string& a, const string& b)
    {
        return strcmp(a.c_str(), b.c_str());
    }

    const char* c_str() const { return _data ? _data : ""; }
    string& erase(uint16_t pos, uint16_t len);

    string& erase(uint16_t pos = 0)
    {
        return erase(pos, _size - pos);
    }
    
    string slice(int32_t start, int32_t end) const;
    
    string slice(int32_t start) const
    {
        return slice(start, static_cast<int32_t>(size()));
    }
    
    string trim() const;
    
    // If skipEmpty is true, substrings of zero length are not added to the array
    vector<string> split(const string& separator, bool skipEmpty = false) const;
    
    static string join(const vector<string>& array, const string& separator);
    
    static string join(const vector<char>& array);
    
    bool isMarked() const { return _marked; }
    void setMarked(bool b) { _marked = b; }
    
    void reserve(uint16_t size) { ensureCapacity(size); }
    
    // Print passed size with K, M, or G suffix as needed
    // There is a space between the number and the suffix. If no suffix there is just an ending space
    // decimalDigits specifies the number of digits to the right of the decimal point. Value is
    // rounded to this many digits. Trailing zeros are omitted. If there are no digits to the
    // right of the dp, either because of rounding or decimalDigits = 0, the dp is omitted
    static string prettySize(uint32_t size, uint8_t decimalDigits = DefaultFloatDigits, bool binary = false);
    
    static string vformat(const char* format, va_list args);
    static string format(const char* format, ...);

private:
    void doEnsureCapacity(uint16_t size);
    
    void ensureCapacity(uint16_t size)
    {
        if (_capacity >= size) {
            return;
        }
        doEnsureCapacity(size);
    }
    
    uint16_t _size = 0;
    uint16_t _capacity = 0;
    char* _data = nullptr;
    bool _marked = true;
    bool _destroyed = false;
};

}
