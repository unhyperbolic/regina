
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2021, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  As an exception, when this program is distributed through (i) the     *
 *  App Store by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or     *
 *  (iii) Google Play by Google Inc., then that store may impose any      *
 *  digital rights management, device limits and/or redistribution        *
 *  restrictions that are required by its terms of service.               *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,       *
 *  MA 02110-1301, USA.                                                   *
 *                                                                        *
 **************************************************************************/

#include <mutex>
#include <sstream>
#include "maths/perm.h"

namespace regina {

namespace {
    std::mutex precomputeMutex;
}

void Perm<6>::precompute() {
    std::lock_guard<std::mutex> lock(precomputeMutex);
    if (! products_) {
        products_ = new Code[Perm<n>::nPerms][Perm<n>::nPerms];
        for (Code i = 0; i < Perm<n>::nPerms; ++i)
            for (Code j = 0; j < Perm<n>::nPerms; ++j)
                products_[i][j] =
                    (Perm<n>::fromPermCode2(i) *
                     Perm<n>::fromPermCode2(j)).permCode2();
    }
}

std::string Perm<6>::str() const {
    char ans[7];
    for (int i = 0; i < 6; i++)
        ans[i] = static_cast<char>('0' + (*this)[i]);
    ans[6] = 0;

    return ans;
}

std::string Perm<6>::trunc(unsigned len) const {
    char ans[7];
    for (unsigned i = 0; i < len; ++i)
        ans[i] = static_cast<char>('0' + (*this)[i]);
    ans[len] = 0;
    return ans;
}

} // namespace regina

