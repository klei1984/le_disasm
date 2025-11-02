/* Copyright (C) 2010  Unavowed <unavowed@vexillium.org>
 * Copyright (C) 2017  samunders-core <samunders-core@users.noreply.github.com>
 * Copyright (C) 2019-2025  klei1984 <53688147+klei1984@users.noreply.github.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LE_DISASM_ERROR_HPP_
#define LE_DISASM_ERROR_HPP_

#include <sstream>
#include <stdexcept>
#include <string>

#include "stacktrace.hpp"

class Error : public std::exception {
public:
    Error();
    Error(const Error& that);
    virtual ~Error() throw();

    virtual const char* what() const throw();

    template <typename T>
    Error& operator<<(const T& t) {
        mStream << t;
        return *this;
    }

private:
    mutable std::stringstream mStream;
    mutable std::string mWhat;
};

#endif
