
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2014, Ben Burton                                   *
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

/* end stub */

#include <sstream>
#include "maths/nperm2.h"

namespace regina {

const int NPerm2::nPerms;
const int NPerm2::nPerms_1;

const NPerm3 NPerm3::S3[6] = {
    NPerm3((NPerm3::Code)0), NPerm3(1), NPerm3(2),
    NPerm3(3), NPerm3(4), NPerm3(5)
};

const NPerm3* NPerm3::Sn = NPerm3::S3;

const unsigned NPerm3::invS3[6] = {
    0, 1, 4, 3, 2, 5
};

const unsigned* NPerm3::invSn = NPerm3::invS3;

const NPerm3 NPerm3::orderedS3[6] = {
    NPerm3(code012), NPerm3(code021),
    NPerm3(code102), NPerm3(code120),
    NPerm3(code201), NPerm3(code210)
};

const NPerm3* NPerm3::orderedSn = NPerm3::orderedS3;

const NPerm3 NPerm3::S2[2] = {
    NPerm3(code012), NPerm3(code102)
};

const NPerm3* NPerm3::Sn_1 = NPerm3::S2;

const NPerm3::Code NPerm3::imageTable[6][3] = {
    { 0, 1, 2 },
    { 0, 2, 1 },
    { 1, 2, 0 },
    { 1, 0, 2 },
    { 2, 0, 1 },
    { 2, 1, 0 }
};

const NPerm3::Code NPerm3::productTable[6][6] = {
    { 0, 1, 2, 3, 4, 5 },
    { 1, 0, 5, 4, 3, 2 },
    { 2, 3, 4, 5, 0, 1 },
    { 3, 2, 1, 0, 5, 4 },
    { 4, 5, 0, 1, 2, 3 },
    { 5, 4, 3, 2, 1, 0 }
};

NPerm3::NPerm3(const int* a, const int* b) {
    int image[3];
    image[a[0]] = b[0];
    image[a[1]] = b[1];
    image[a[2]] = b[2];

    switch (image[0]) {
        case 0:
            code_ = static_cast<Code>(image[1] == 1 ? 0 : 1); break;
        case 1:
            code_ = static_cast<Code>(image[1] == 2 ? 2 : 3); break;
        case 2:
            code_ = static_cast<Code>(image[1] == 0 ? 4 : 5); break;
    }
}

std::string NPerm3::str() const {
    char ans[4];
    ans[0] = static_cast<char>('0' + imageTable[code_][0]);
    ans[1] = static_cast<char>('0' + imageTable[code_][1]);
    ans[2] = static_cast<char>('0' + imageTable[code_][2]);
    ans[3] = 0;
    return ans;
}

std::string NPerm3::trunc(unsigned len) const {
    char ans[4];
    for (unsigned i = 0; i < len; ++i)
        ans[i] = static_cast<char>('0' + imageTable[code_][i]);
    ans[len] = 0;
    return ans;
}

} // namespace regina

