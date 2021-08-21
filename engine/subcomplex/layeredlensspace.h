
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

/*! \file subcomplex/layeredlensspace.h
 *  \brief Deals with layered lens space components of a triangulation.
 */

#ifndef __REGINA_LAYEREDLENSSPACE_H
#ifndef __DOXYGEN
#define __REGINA_LAYEREDLENSSPACE_H
#endif

#include <optional>
#include "regina-core.h"
#include "subcomplex/layeredsolidtorus.h"
#include "subcomplex/standardtri.h"

namespace regina {

/**
 * \weakgroup subcomplex
 * @{
 */

/**
 * Represents a layered lens space component of a triangulation.
 * A layered lens space is considered to be any layered solid torus glued
 * to a degenerate (2,1,1) layered solid torus (i.e., a one-triangle mobius
 * strip).  Note that the three possible gluing options represent the
 * three possible ways of closing the initial torus - either twisting it
 * shut (in one of two possible ways) or snapping it shut without any twist.
 * 
 * A layered lens space must contain at least one tetrahedron.
 *
 * All optional StandardTriangulation routines are implemented for this class.
 *
 * This class supports copying but does not implement separate move operations,
 * since its internal data is so small that copying is just as efficient.
 * It implements the C++ Swappable requirement via its own member and global
 * swap() functions, for consistency with the other StandardTriangulation
 * subclasses.  Note that the only way to create these objects (aside from
 * copying or moving) is via the static member function recognise().
 */
class LayeredLensSpace : public StandardTriangulation {
    private:
        LayeredSolidTorus torus_;
            /**< The layered solid torus that forms the basis of this
                 layered lens space. */
        int mobiusBoundaryGroup_;
            /**< The edge group of the top level tetrahedron in the
                 layered solid torus to which the boundary of the mobius
                 strip is glued. */
        unsigned long p_,q_;
            /**< The lens space parameters for L(p,q). */

    public:
        /**
         * Creates a new copy of this structure.
         */
        LayeredLensSpace(const LayeredLensSpace&) = default;

        /**
         * Sets this to be a copy of the given structure.
         *
         * @return a reference to this structure.
         */
        LayeredLensSpace& operator = (const LayeredLensSpace&) = default;

        /**
         * Deprecated routine that returns a new copy of this structure.
         *
         * \deprecated Just use the copy constructor instead.
         *
         * @return a newly created clone.
         */
        [[deprecated]] LayeredLensSpace* clone() const;

        /**
         * Swaps the contents of this and the given structure.
         *
         * @param other the structure whose contents should be swapped
         * with this.
         */
        void swap(LayeredLensSpace& other) noexcept;

        /**
         * Returns the first parameter \a p of this lens space L(p,q).
         *
         * @return the first parameter \a p.
         */
        unsigned long p() const;
        /**
         * Returns the second parameter \a q of this lens space L(p,q).
         *
         * @return the second parameter \a q.
         */
        unsigned long q() const;

        /**
         * Returns the layered solid torus to which the mobius strip is
         * glued.
         *
         * @return the layered solid torus.
         */
        const LayeredSolidTorus& torus() const;
        /**
         * Determines which edge of the layered solid torus is glued to
         * the boundary of the mobius strip (i.e., the weight 2 edge
         * of the degenerate (2,1,1) layered solid torus).  The return
         * value will be one of the three top level tetrahedron edge
         * groups in the layered solid torus; see
         * LayeredSolidTorus::topEdge() for further details about
         * edge groups.
         *
         * @return the top level edge group of the layered solid torus to
         * which the mobius strip boundary is glued.
         */
        int mobiusBoundaryGroup() const;
        /**
         * Determines if the layered solid torus that forms the basis for
         * this lens space is snapped shut (folded closed without a twist).
         *
         * @return \c true if and only if the torus is snapped shut.
         */
        bool isSnapped() const;
        /**
         * Determines if the layered solid torus that forms the basis for
         * this lens space is twisted shut (folded closed with a twist).
         *
         * @return \c true if and only if the torus is twisted shut.
         */
        bool isTwisted() const;

        /**
         * Determines if the given triangulation component is a layered
         * lens space.
         *
         * @param comp the triangulation component to examine.
         * @return a structure containing details of the layered lens space,
         * or no value if the given component is not a layered lens space.
         */
        static std::optional<LayeredLensSpace> recognise(
            const Component<3>* comp);
        /**
         * A deprecated alias to recognise if a component forms a
         * layered lens space.
         *
         * \deprecated This function has been renamed to recognise().
         * See recognise() for details on the parameters and return value.
         */
        [[deprecated]] static std::optional<LayeredLensSpace>
            isLayeredLensSpace(const Component<3>* comp);

        std::unique_ptr<Manifold> manifold() const override;
        std::optional<AbelianGroup> homology() const override;
        std::ostream& writeName(std::ostream& out) const override;
        std::ostream& writeTeXName(std::ostream& out) const override;
        void writeTextLong(std::ostream& out) const override;

    private:
        /**
         * Creates a new structure initialised with the given layered
         * solid torus.  All other data members will be left uninitialised.
         */
        LayeredLensSpace(const LayeredSolidTorus& torus);
};

/**
 * Swaps the contents of the two given structures.
 *
 * This global routine simply calls LayeredLensSpace::swap(); it is provided
 * so that LayeredLensSpace meets the C++ Swappable requirements.
 *
 * @param a the first alternative whose contents should be swapped.
 * @param b the second alternative whose contents should be swapped.
 */
void swap(LayeredLensSpace& a, LayeredLensSpace& b) noexcept;

/*@}*/

// Inline functions for LayeredLensSpace

inline LayeredLensSpace::LayeredLensSpace(const LayeredSolidTorus& torus) :
        torus_(torus) {
}

inline LayeredLensSpace* LayeredLensSpace::clone() const {
    return new LayeredLensSpace(*this);
}

inline unsigned long LayeredLensSpace::p() const {
    return p_;
}
inline unsigned long LayeredLensSpace::q() const {
    return q_;
}
inline const LayeredSolidTorus& LayeredLensSpace::torus() const {
    return torus_;
}
inline int LayeredLensSpace::mobiusBoundaryGroup() const {
    return mobiusBoundaryGroup_;
}
inline bool LayeredLensSpace::isSnapped() const {
    return (torus_.topEdge(mobiusBoundaryGroup_, 1) == -1);
}
inline bool LayeredLensSpace::isTwisted() const {
    return (torus_.topEdge(mobiusBoundaryGroup_, 1) != -1);
}
inline void LayeredLensSpace::writeTextLong(std::ostream& out) const {
    out << "Layered lens space ";
    writeName(out);
}

inline std::optional<LayeredLensSpace> LayeredLensSpace::isLayeredLensSpace(
        const Component<3>* comp) {
    return recognise(comp);
}

inline void swap(LayeredLensSpace& a, LayeredLensSpace& b) noexcept {
    a.swap(b);
}

} // namespace regina

#endif

