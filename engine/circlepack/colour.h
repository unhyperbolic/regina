
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2011, Ben Burton                                   *
 *  For further details contact Ben Burton (bab@debian.org).              *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
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

/*! \file circlepack/colour.h
 *  \brief Used by circlepack to store colours. 
 */

#ifndef __COLOUR_H
#ifndef __DOXYGEN
#define __COLOUR_H
#endif

namespace regina{

class colour {
private:
	float r;
	float b;
	float g;
public:
	colour();
	colour(float setR, float setG, float setB);
	float getR();
	float getB();
	float getG();
	void printForPS(std::ostream& out);
	void changeColour(float setR, float setG, float setB);
	float modOne(float num);
};

}//namespace regina
#endif

