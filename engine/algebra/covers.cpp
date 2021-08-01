
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 2021, Ben Burton                                        *
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

#include "algebra/grouppresentation.h"
#include "maths/perm.h"
#include "maths/matrix.h"

namespace regina {

namespace {
    // The S_n indices representing conjugacy minimal permutations,
    // currently computed for n <= 6.
    constexpr int allMinimalPerms[] = {
        0, 1, 2, 6, 9, 27, 32, 127, 128, 146, 153
    };
    constexpr int nMinimalPerms[] = { 1, 1, 2, 3, 5, 7, 11 };

    // The maximum size of an automorphism group for a conjugacy minimal
    // permutation, excluding the case where the automorphism group is
    // all of S_n.
    constexpr int maxMinimalAutGroup[] = { 0, 0, 0, 3, 8, 12, 48 };

    // The (-1)-terminated automorphism group corresponding to each
    // conjugacy minimal permutation, or an empty list if the automorphism
    // group is all of S_n.
    // The code that generated these arrays can be found in aut.py, in the same
    // directory as this source file.
    template <int n> constexpr int
        minimalAutGroup[nMinimalPerms[n]][maxMinimalAutGroup[n] + 1];
    template <> constexpr int minimalAutGroup<2>[][1] = {
        /* 0 */ { -1 },
        /* 1 */ { -1 }
    };
    template <> constexpr int minimalAutGroup<3>[][4] = {
        /* 0 */ { -1, 0, 0, 0 },
        /* 1 */ { 0, 1, -1, 0 },
        /* 2 */ { 0, 2, 4, -1 }
    };
    template <> constexpr int minimalAutGroup<4>[][9] = {
        /* 0 */ { -1, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 1 */ { 0, 1, 6, 7, -1, 0, 0, 0, 0 },
        /* 2 */ { 0, 2, 4, -1, 0, 0, 0, 0, 0 },
        /* 6 */ { 0, 1, 6, 7, 16, 17, 22, 23, -1 },
        /* 9 */ { 0, 9, 16, 19, -1, 0, 0, 0, 0 }
    };
    template <> constexpr int minimalAutGroup<5>[][13] = {
        /* 0 */ { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 1 */ { 0, 1, 6, 7, 24, 25, 30, 31, 48, 49, 54, 55, -1 },
        /* 2 */ { 0, 2, 4, 25, 27, 29, -1, 0, 0, 0, 0, 0, 0 },
        /* 6 */ { 0, 1, 6, 7, 16, 17, 22, 23, -1, 0, 0, 0, 0 },
        /* 9 */ { 0, 9, 16, 19, -1, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 27 */ { 0, 2, 4, 25, 27, 29, -1, 0, 0, 0, 0, 0, 0 },
        /* 32 */ { 0, 32, 64, 90, 96, -1, 0, 0, 0, 0, 0, 0, 0 }
    };
    template <> constexpr int minimalAutGroup<6>[][49] = {
        /* 0 */ { -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 1 */ { 0, 1, 6, 7, 24, 25, 30, 31, 48, 49, 54, 55, 120, 121, 126,
                  127, 144, 145, 150, 151, 168, 169, 174, 175, 240, 241, 246,
                  247, 264, 265, 270, 271, 288, 289, 294, 295, 360, 361, 366,
                  367, 384, 385, 390, 391, 408, 409, 414, 415, -1 },
        /* 2 */ { 0, 2, 4, 25, 27, 29, 121, 123, 125, 144, 146, 148, 240, 242,
                  244, 265, 267, 269, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 6 */ { 0, 1, 6, 7, 16, 17, 22, 23, 120, 121, 126, 127, 136, 137,
                  142, 143, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 9 */ { 0, 9, 16, 19, 121, 128, 137, 138, -1, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 27 */ { 0, 2, 4, 25, 27, 29, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 32 */ { 0, 32, 64, 90, 96, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 127 */ { 0, 1, 6, 7, 16, 17, 22, 23, 120, 121, 126, 127, 136, 137,
                    142, 143, 288, 289, 294, 295, 304, 305, 310, 311, 408, 409,
                    414, 415, 424, 425, 430, 431, 576, 577, 582, 583, 592, 593,
                    598, 599, 696, 697, 702, 703, 712, 713, 718, 719, -1 },
        /* 128 */ { 0, 9, 16, 19, 121, 128, 137, 138, -1, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 146 */ { 0, 2, 4, 144, 146, 148, 240, 242, 244, 451, 453, 455, 595,
                    597, 599, 691, 693, 695, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        /* 153 */ { 0, 153, 304, 451, 576, 601, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    };

    /**
     * A class similar in nature to GroupExpression, which is used by
     * RelationScheme to represent both group relations and also contiguous
     * subexpressions within relations.
     *
     * The differences between Formula and GroupExpression include:
     *
     * - Formula uses a vector, because using a contiguous block of memory is
     *   more important here than the ability to spice formulae together.
     *
     * - Formula uses not only the group generators with indices 0 <= i < nGen,
     *   but also additional subexpressions that can be computed separately
     *   and cached.  These subexpressions (which are represented by their
     *   own Formula objects) are indicated by terms whose "generators" have
     *   indices i >= nGen.
     */
    struct Formula {
        std::vector<GroupExpressionTerm> terms;
        bool isRelation;

        Formula() = default;
        Formula(const Formula&) = default;
        Formula(Formula&&) = default;
        Formula(bool isReln) : isRelation(isReln) {}
        Formula& operator = (const Formula&) = default;
        Formula& operator = (Formula&&) = default;

        /**
         * Looks for occurrences of the formula \a inner as a contiguous
         * subexpression of this formula.  If it finds any such occurrences,
         * it replaces each with a single term of the form index^1.
         *
         * This routine will happily replace multiple occurrences of \a inner,
         * but only when these occurrences are non-overlapping.
         *
         * As an exception, if \a inner is empty, this routine will *not*
         * make any replacements.
         *
         * This routine runs in quadratic time (since it processes each
         * replacement separately, and each such replacement involves
         * repacking the vector of terms).  We do not worry too much
         * about this, because the time spent doing this replacements is
         * insignificant compared to the "real" work of enumerateCovers().
         */
        void tryReplace(const Formula& inner, unsigned long index) {
            if (inner.terms.size() == 0)
                return;

            for (size_t from = 0; from + inner.terms.size() <= terms.size();
                    ++from) {
                if (std::equal(inner.terms.begin(), inner.terms.end(),
                        terms.begin() + from)) {
                    // We have found a replacement.
                    // Move everything *after* the occurrence of inner
                    // forward, leaving a gap of just one term which we
                    // then set to index^1.
                    if (inner.terms.size() > 1) {
                        std::move(terms.begin() + from + inner.terms.size(),
                            terms.end(), terms.begin() + from + 1);
                        terms.resize(terms.size() + 1 - inner.terms.size());
                    }
                    terms[from] = { index, 1 };
                }
            }
        }

        /**
         * An ordering on formulae, which RelationScheme uses to determine in
         * which order we should compute subexpressions at the same depth.
         *
         * Here we prioritise relations above all (since proving that a
         * relation does not hold allows us to backtrack immediately
         * when enumerating covers).  After this, we prioritise shorter
         * expressions (since later we will try to detect occurrences of
         * shorter expressions within longer ones).
         *
         * Note that "depth" here refers to the largest index generator
         * that appears in the formula, once all cached subexpressions
         * are expanded in terms of the original generators of the group
         * presentation.
         */
        struct Compare {
            bool operator ()(const Formula& a, const Formula& b) const {
                if (a.isRelation && ! b.isRelation)
                    return true;
                if (b.isRelation && ! a.isRelation)
                    return false;
                return a.terms.size() < b.terms.size() ||
                    (a.terms.size() == b.terms.size() && a.terms < b.terms);
            }
        };
    };

    /**
     * This is a helper class for enumerateCovers(), whose purpose is to
     * speed up the tests for whether a candidate representation of the
     * generators in the symmetric group S_n respects the group relations.
     *
     * The idea is the following:
     *
     * - The members rep[0..(nGen-1)] are the representatives of the
     *   group generators in S_n.
     *
     * - The members computed[nGen...] are additional elements of S_n
     *   that correpsond to formulae (i.e., group expressions) involving
     *   the generators.  These formulae typically appear as contiguous
     *   subexpressions of the group relations.
     *
     * - In particular, for compCount[d] <= i < compCount[d+1], the expressions
     *   formulae[i] can all be written in terms of the generators 0..d only.
     *   We refer to these as the formulae "at depth d".  We compute the
     *   corresponding permutations as soon as we have chosen representatives
     *   for generators 0..d, and cache them in computed[i].
     *
     * - We allow ourselves to write formulae[j] in terms of formulae[i]
     *   for i < j (as well as the original group generators appropriate
     *   for the depth).  This means that we can reuse the computations
     *   for these subexpressions, which in practice saves significant
     *   time over the alternative strategy of testing all group relations
     *   from scratch for every choice of representatives for all generators.
     *
     * - In particular, every group relation appears as one of our formulae.
     *   This means that we can effectively check the group relations as
     *   we perform the various computations for formulae[i].
     *
     * The RelationScheme() constructor is responsible for taking the
     * group relations and deciding what additional formulae to use, and
     * in what order.  It assumes that minimaxGenerators() has already
     * been called on the group presentation.
     *
     * Once you have chosen generator d, the function computeFor(d) will
     * compute the formulae at depth d.  It is assumed that the formulae
     * at depths < d have already been computed, and that the representatives
     * for all generators 0..d have already been chosen.
     */
    template <int index>
    struct RelationScheme {
        size_t nGen;
        std::vector<Formula> formulae;
        size_t* compCount; // length nGenerators + 1
        Perm<index>* rep;
        Perm<index>* computed;

        // Do we want to compose permutations using precomputed tables that are
        // generated at runtime?
        //
        // Note that for index <= 5 the Perm<index> class already uses lookup
        // tables out-of-the-box and so there is no need for us to manage this
        // ourselves here.  For index >= 7 the Perm<index> class does not (yet)
        // have a runtime precomputation facility built in.  So this leaves
        // index == 6 as the only case where this is relevant.
        static constexpr bool cacheProducts = (index == 6);

        RelationScheme(const GroupPresentation& g) {
            if constexpr (cacheProducts) {
                Perm<index>::precompute();
            }

            nGen = g.countGenerators();
            compCount = new size_t[nGen + 1];

            // nSeen will be the total number of formulae that we have
            // available to work with, including the group generators as
            // well as all additional subexpressions that are stored in
            // formulae[].  In particular, we should always have
            // nSeen == formulae.size() + nGen.
            long nSeen = nGen;

            // Work out all the additonal formulae we will want to compute.
            // Initially we will give these temporary indices, which we
            // store as the values in the maps foundExp[depth].
            // We will reindex all our formulae later, once we have a
            // complete set.

            // As we walk through each relation, currExp[i] will hold the
            // maximum length sub-expression ending at the current
            // position, using only generators of index <= i, and
            // *excluding* all trailing terms with generators of index < i.
            std::vector<GroupExpressionTerm>* currExp =
                new std::vector<GroupExpressionTerm>[nGen];

            // The formulae that we will compute at depth d are stored
            // as keys in the map foundExp[d].  The corresponding values
            // (as noted earlier) are the temporary indices for each formulae.
            typedef std::map<Formula, long, Formula::Compare> ExpressionMap;
            ExpressionMap* foundExp = new ExpressionMap[nGen];

            unsigned long depth;
            for (const auto& r : g.relations()) {
                depth = nGen; // the last generator seen
                unsigned long prev;

                for (const auto& t : r.terms()) {
                    if (t.generator < depth) {
                        // Start a new subexpression at a smaller depth.
                        depth = t.generator;
                        currExp[depth].push_back({ depth, t.exponent });
                    } else {
                        // Finish off all subexpressions at depths
                        // below the newly-seen generator.
                        while (depth < t.generator) {
                            if (currExp[depth].size() == 1 &&
                                    currExp[depth].front().exponent == 1) {
                                // This expression is just a single symbol.
                                // Reuse that symbol instead of creating
                                // a new one.
                                prev = currExp[depth].front().generator;
                                currExp[depth].clear();
                            } else {
                                // We use a swap and a move to avoid a
                                // deep copy of currExp[depth].
                                // A side-effect is that this clears out
                                // currExp[depth] (which we want to do).
                                Formula tmp(false);
                                tmp.terms.swap(currExp[depth]);
                                auto result = foundExp[depth].emplace(
                                    std::move(tmp), nSeen);
                                if (result.second) {
                                    // This was a new term that we
                                    // hadn't seen before.
                                    prev = nSeen++;
                                } else {
                                    // We already have this same expression
                                    // stashed away as a formula from earlier.
                                    // Reuse its index.
                                    prev = result.first->second;
                                }
                            }
                            // Append the term prev^1 to the expression at
                            // the next higher depth, where prev is the
                            // index of the formula that we just closed off.
                            ++depth;
                            currExp[depth].push_back({ prev, 1 });
                        }
                        // Finally, actually append the newly-seen term
                        // that we are looking at now.
                        // Note that depth == t.generator at this point.
                        currExp[depth].push_back({ depth, t.exponent });
                    }
                }

                // We are guaranteed that the last term in the relation uses
                // the highest generator index that appears in the relation.
                //
                // This means that currExp[depth] is the entire relation,
                // and the relation does not use any generators with index
                // greater than depth.
                //
                // Again we use a swap and a move to avoid a deep copy
                // of currExp[depth], and this also clears currExp[depth].
                Formula tmp(true);
                tmp.terms.swap(currExp[depth]);
                auto result = foundExp[depth].emplace(std::move(tmp), nSeen);
                if (result.second) {
                    // We have not seen this formula before.
                    ++nSeen;
                }

                // It's conceivable that this same expression also appears in
                // non-relation form.  Currently this would mean we are
                // computing it twice, once with isRelation == true, and once
                // with isRelation == false (which will be treated as different
                // keys in the foundExp[...] maps).  This is inefficient, but
                // otherwise harmless.
                //
                // However: if this *does* happen that it means that one group
                // relation is a strict subexpression of another.  Assuming
                // the group presentation has been simplified, this should not
                // happen.  So just leave the inefficieny here, under the
                // assumption that it will never be triggered (but it's
                // harmless if it is).
            }

            // Now we have a full set of formulae.
            // Reindex them, using the order induced by the foundExp[...] maps,
            // in order of increasing depth.
            // This ordering will put all relations first at each depth level
            // (so we can backtrack sooner if the relation does not hold).
            //
            // Note that, by construction, each expression only uses other
            // expressions at a lower depth, which means that an
            // expression with final index i will only every use terms
            // with indices j < i.  So it will be safe to compute them
            // in the order formulae[0], formulae[1], ....
            //
            // The reindexing is a two-stage process: (1) work out how
            // the original indices map to the final indices; and then
            // (2) fix all the terms in all the formulae that *use* these
            // indices.
            unsigned long* reindex = new unsigned long[nSeen];
            long newIndex = nGen;
            for (depth = 0; depth < nGen; ++depth) {
                for (const auto& exp : foundExp[depth])
                    reindex[exp.second] = newIndex++;
            }
            for (depth = 0; depth < nGen; ++depth) {
                for (auto& exp : foundExp[depth]) {
                    Formula f(exp.first.isRelation);
                    f.terms.reserve(exp.first.terms.size());
                    for (auto& t : exp.first.terms)
                        if (t.generator < nGen)
                            f.terms.push_back(t);
                        else
                            f.terms.push_back(
                                { reindex[t.generator], t.exponent });
                    formulae.push_back(std::move(f));
                }
            }

            // Finally, record out how many formulae we actually have at
            // each depth.
            compCount[0] = 0;
            for (depth = 0; depth < nGen; ++depth)
                compCount[depth + 1] = compCount[depth] +
                    foundExp[depth].size();

            // At this point we are done, and we could happily finish.
            // However, we make one more pass in an attempt to simplify
            // our formulae a little more.
            //
            // We see now if it is possible to use the results from earlier
            // formulae in the computations of later ones.  We work backwards
            // from the longer relations to the shorter ones, since we want to
            // prioritise large substitutions if any are possible.
            long outer, inner;
            for (outer = static_cast<long>(formulae.size()) - 1; outer >= 0;
                    --outer) {
                for (inner = outer - 1; inner >= 0; --inner) {
                    formulae[outer].tryReplace(formulae[inner], inner + nGen);
                }
            }

            delete[] reindex;
            delete[] foundExp;
            delete[] currExp;

            // Now everything else is done: prepare for the big search
            // for representatives, which is where the *real* work happens.
            rep = new Perm<index>[nGen];
            computed = new Perm<index>[compCount[nGen]];
        }

        ~RelationScheme() {
            delete[] compCount;
            delete[] rep;
            delete[] computed;
        }

        /**
         * Compute the representative in S_n for formulae[piece].
         *
         * Returns false if this formulae is one of the group relations
         * and the resulting computation is not the identity (i.e., the
         * group relation is not being respected by our current choice
         * of rep[...]).  In this case we do *not* store the result of
         * the computation, since we will be backtracking immediately.
         */
        bool computePiece(size_t piece) {
            Perm<index> comb;
            for (const auto& t : formulae[piece].terms) {
                Perm<index> gen = (t.generator < nGen ?
                    rep[t.generator] : computed[t.generator - nGen]);
                // Pull out exponents +/-1, since in practice these are
                // common and we can avoid the (small) overhead of pow().
                if constexpr (cacheProducts) {
                    switch (t.exponent) {
                        case 1:
                            comb = gen.cachedComp(comb);
                            break;
                        case -1:
                            comb = gen.inverse().cachedComp(comb);
                            break;
                        default:
                            comb = gen.cachedPow(t.exponent).cachedComp(comb);
                            break;
                    }
                } else {
                    switch (t.exponent) {
                        case 1:
                            comb = gen * comb;
                            break;
                        case -1:
                            comb = gen.inverse() * comb;
                            break;
                        default:
                            comb = gen.pow(t.exponent) * comb;
                            break;
                    }
                }
            }
            if (formulae[piece].isRelation && ! comb.isIdentity())
                return false;
            else {
                computed[piece] = comb;
                return true;
            }
        }

        /**
         * Compute the representative in S_n for all formulae at the
         * given depth (where 0 <= depth < nGen).
         *
         * Returns false if *any* of the corresponding formulae is one of the
         * group relations and the resulting computation is not the identity
         * (i.e., the group relations are not being respected).
         */
        bool computeFor(size_t depth) {
            for (size_t i = compCount[depth]; i < compCount[depth + 1]; ++i)
                if (! computePiece(i))
                    return false;
            return true;
        }

        /**
         * Dumps the details of this data structure to the given output stream.
         *
         * If the total number of generators and formulae exceeds 26,
         * this routine will start to output junk (since it uses
         * lower-case letters to denote generators and formulae).
         *
         * Since this is a private routine for diagnostic purposes that is
         * never actually called, we will leave it like this for now.
         */
        void dump(std::ostream& out) {
            out << "#gen: " << nGen << std::endl;
            out << "compCount:";
            for (int i = 0; i <= nGen; ++i)
                out << ' ' << compCount[i];
            out << std::endl;

            out << "Formulae:" << std::endl;
            for (int i = 0; i < compCount[nGen]; ++i) {
                out << char('a' + nGen + i);
                if (formulae[i].isRelation)
                    out << "[*]";
                out << " :=";
                for (const auto& t : formulae[i].terms) {
                    out << ' ';
                    if (t.exponent == 1)
                        out << char('a' + t.generator);
                    else
                        out << char('a' + t.generator) << '^' << t.exponent;
                }
                out << std::endl;
            }
        }
    };
}

void GroupPresentation::minimaxGenerators() {
    if (relations_.size() == 0 || nGenerators_ == 0) {
        // Nothing to relabel.
        return;
    }

    // Build a table of which relations contain which generators.
    // Rows = relations; columns = generators.
    Matrix<bool> inc = incidence();

    // Note how we plan to relabel the generators.
    unsigned long* relabel = new unsigned long[nGenerators_];
    unsigned long* relabelInv = new unsigned long[nGenerators_];
    for (unsigned long i = 0; i < nGenerators_; ++i)
        relabel[i] = relabelInv[i] = i;

    size_t gensUsed = 0;
    for (size_t rowsUsed = 0; rowsUsed < inc.rows(); ++rowsUsed) {
        // Find the row in [r, #relns) that uses the fewest generators
        // not yet seen (i.e., from [gensUsed, #gens) after relabelling).
        size_t useRow = rowsUsed;

        size_t best = 0;
        for (size_t g = gensUsed; g < nGenerators_; ++g)
            if (inc.entry(rowsUsed, relabelInv[g]))
                ++best;

        for (size_t r = rowsUsed + 1; r < inc.rows(); ++r) {
            size_t curr = 0;
            for (size_t g = gensUsed; g < nGenerators_; ++g)
                if (inc.entry(r, relabelInv[g]))
                    ++curr;
            // TOOD: Make this test quicker by precomputing word lengths.
            if (curr < best ||
                    (curr == best &&
                     relations_[r].wordLength() <
                     relations_[useRow].wordLength())) {
                best = curr;
                useRow = r;
            }
        }

        if (useRow != rowsUsed) {
            inc.swapRows(useRow, rowsUsed);
            relations_[useRow].swap(relations_[rowsUsed]);
        }

        if (gensUsed == 0 && best == 0) {
            // This relation is empty (as are all those above it).
            continue;
        }

        // This relation is non-empty (as are all those below it).
        if (best > 0) {
            // This relation brings in new, previously unseen generator(s).
            // Make plans to relabel those new generators to use the
            // next available generator labels.
            for (size_t g = gensUsed; g < nGenerators_; ++g)
                if (inc.entry(rowsUsed, relabelInv[g])) {
                    // Whatever was being relabelled to g should now be
                    // relabelled to gensUsed.
                    if (g != gensUsed) {
                        std::swap(relabelInv[g], relabelInv[gensUsed]);
                        std::swap(relabel[relabelInv[g]],
                            relabel[relabelInv[gensUsed]]);
                    }
                    ++gensUsed;
                }
        }

        // The highest numbered generator that relation #rowsUsed uses is
        // now precisely (gensUsed-1).
        // Cycle the relation around so that its last term uses its
        // highest numbered generator.
        while (relations_[rowsUsed].terms().back().generator !=
                relabelInv[gensUsed - 1])
            relations_[rowsUsed].cycleLeft();
    }

    // Now do the actual relabelling.
    for (auto& r : relations_)
        for (auto& t : r.terms())
            t.generator = relabel[t.generator];

    delete[] relabelInv;
    delete[] relabel;
}

template <int index>
size_t GroupPresentation::enumerateCoversInternal(
        std::function<void(GroupPresentation&)>&& action) {
    if (nGenerators_ == 0) {
        // We have the trivial group.
        // There is only one trivial representation, and it is not transitive.
        return 0;
    }

    if (nGenerators_ == 1) {
        // To be transitive, the representation of the unique generator must
        // be a cycle, and all such representations are conjugate (so there
        // is at most one cover to generate).
        //
        // To satisfy the relations:
        //
        // - If we are Z, then there are no non-trivial relations and so
        //   they are vacuously satisfied.  The resulting subgroup is also Z.
        //
        // - If we are Z_n, then n must be a multiple of index.  The resulting
        //   subgroup is Z_(n/index).
        //
        // TODO: Hard-code this result and return.
    }

    // Relabel and reorder generators and relations so that we can check
    // relations as early as possible and backtrack if they break.
    minimaxGenerators();

    // Make a plan for how we will incrementally test consistency with
    // the group relations.
    RelationScheme<index> scheme(*this);

    // Prepare to choose an S(index) representative for each generator.
    // The representative for generator i will be scheme.rep[i].
    // All representatives will be initialised to the identity.
    size_t nReps = 0;

    size_t* nAut = new size_t[nGenerators_];
    Perm<index> (*aut)[maxMinimalAutGroup[index] + 1] =
        new Perm<index>[nGenerators_][maxMinimalAutGroup[index] + 1];

    size_t pos = 0; // The generator whose current rep we are about to try.
    while (true) {
        bool backtrack = false;

        // Check consistency with the group relations that we haven't
        // yet checked, and that containly only generators whose reps
        // have been chosen so far.
        if (! backtrack) {
            if (! scheme.computeFor(pos))
                backtrack = true;
        }

        // Check that the reps are conjugacy minimal, so far.
        // Note: for index 2, *everything* is conjugacy minimal.
        if constexpr (index > 2) {
            if (! backtrack) {
                if (pos == 0 || nAut[pos - 1] == 0) {
                    // Currently the automorphism group for the entire
                    // set of reps chosen before now is all of S_index.
                    // This means that rep[pos] needs to be conjugacy minimal.
                    if (scheme.rep[pos].isConjugacyMinimal()) {
                        if (scheme.rep[pos].isIdentity()) {
                            // The automorphism group remains all of S_index.
                            nAut[pos] = 0;
                        } else {
                            // Set up the automorphism group for this rep
                            // by explicitly listing the automorphisms.
                            int idx = 0;
                            while (allMinimalPerms[idx] !=
                                    scheme.rep[pos].SnIndex())
                                ++idx;

                            nAut[pos] = 0;
                            while (minimalAutGroup<index>[idx][nAut[pos]]
                                    >= 0) {
                                aut[pos][nAut[pos]] = Perm<index>::Sn[
                                    minimalAutGroup<index>[idx][nAut[pos]]];
                                ++nAut[pos];
                            }
                        }
                    } else {
                        backtrack = true;
                    }
                } else {
                    // The previous reps are together conjugacy minimal,
                    // and we have their automorphism group stored.
                    nAut[pos] = 0;
                    Perm<index> conj;
                    for (int a = 0; a < nAut[pos - 1]; ++a) {
                        Perm<index> p = aut[pos - 1][a];
                        if constexpr (RelationScheme<index>::cacheProducts) {
                            conj = p.cachedComp(scheme.rep[pos], p.inverse());
                        } else {
                            conj = p * scheme.rep[pos] * p.inverse();
                        }
                        if (conj < scheme.rep[pos]) {
                            // Not conjugacy minimal.
                            backtrack = true;
                            break;
                        } else if (conj == scheme.rep[pos]) {
                            // This remains part of our automorphism
                            // group going forwards.
                            aut[pos][nAut[pos]++] = p;
                        }
                    }
                }
            }
        }

        // Move on to the next generator.
        if (! backtrack) {
            ++pos;
            if (pos == nGenerators_) {
                // We have a candidate representation.

                // Is it transitive?
                //
                // Use a depth-first search to see if we can reach
                // every sheet using the chosen reps.
                //
                // We also record *which* routes we found to reach
                // all of the sheets, since together these give us a
                // "spanning tree" of subgroup generators that should all
                // be replaced with the identity in the subgroup.

                // Note: we are guaranteed nGenerators_ >= 1.

                bool seen[index];
                std::fill(seen, seen + index, false);
                seen[0] = true;

                int nFound = 1;

                int stack[index];
                int stackSize = 1;
                stack[0] = 0;

                unsigned long spanningTree[index - 1];

                while (nFound < index && stackSize > 0) {
                    int from = stack[--stackSize];
                    for (unsigned long i = 0; i < nGenerators_; ++i) {
                        int to = scheme.rep[i][from];
                        if (! seen[to]) {
                            seen[to] = true;
                            stack[stackSize++] = to;

                            // Add (generator i, sheet from) to the
                            // spanning tree.
                            spanningTree[nFound - 1] = i * index + from;

                            ++nFound;
                        }
                    }
                }

                if (nFound == index) {
                    // The representation is transitive!
                    // Build the subgroup representation and act on it.

                    GroupPresentation sub;
                    sub.nGenerators_ = index * nGenerators_;
                    sub.relations_.reserve(index * relations_.size());

                    std::sort(spanningTree, spanningTree + index - 1);

                    unsigned long* rewrite =
                        new unsigned long[sub.nGenerators_];

                    // Work out how the subgroup generators will be relabelled
                    // once the spanning tree is removed.
                    unsigned long treeIdx = 0;
                    for (unsigned long i = 0; i < sub.nGenerators_; ++i) {
                        if (treeIdx < index - 1 && spanningTree[treeIdx] == i) {
                            // This generator will be removed from the subgroup.
                            rewrite[i] = sub.nGenerators_;
                            ++treeIdx;
                        } else
                            rewrite[i] = i - treeIdx;
                    }
                    sub.nGenerators_ -= (index - 1);

                    for (const auto& r : relations_) {
                        for (int start = 0; start < index; ++start) {
                            GroupExpression e;
                            int sheet = start;
                            Perm<index> p;
                            unsigned long gen;
                            for (const auto& t : r.terms()) {
                                if (t.exponent > 0) {
                                    for (long i = 0; i < t.exponent; ++i) {
                                        gen = rewrite[
                                            t.generator * index + sheet];
                                        if (gen < sub.nGenerators_)
                                            e.addTermLast(gen, 1);
                                        sheet = scheme.rep[t.generator][sheet];
                                    }
                                } else if (t.exponent < 0) {
                                    for (long i = 0; i > t.exponent; --i) {
                                        sheet = scheme.rep[t.generator]
                                            .preImageOf(sheet);
                                        gen = rewrite[
                                            t.generator * index + sheet];
                                        if (gen < sub.nGenerators_)
                                            e.addTermLast(gen, -1);
                                    }
                                }
                            }
                            if (! e.terms().empty())
                                sub.relations_.push_back(std::move(e));
                        }
                    }

                    delete[] rewrite;

                    ++nReps;
                    action(sub);
                }

                --pos;
                backtrack = true;
            } else {
                continue;
            }
        }

        if (backtrack) {
            while (true) {
                ++scheme.rep[pos];
                if (! scheme.rep[pos].isIdentity())
                    break;
                if (pos == 0)
                    goto finished;
                --pos;
            }
        }
    }

finished:

    delete[] aut;
    delete[] nAut;
    return nReps;
}

// Instantiate templates for all valid indices.
template size_t GroupPresentation::enumerateCoversInternal<2>(
        std::function<void(GroupPresentation&)>&& action);
template size_t GroupPresentation::enumerateCoversInternal<3>(
        std::function<void(GroupPresentation&)>&& action);
template size_t GroupPresentation::enumerateCoversInternal<4>(
        std::function<void(GroupPresentation&)>&& action);
template size_t GroupPresentation::enumerateCoversInternal<5>(
        std::function<void(GroupPresentation&)>&& action);
template size_t GroupPresentation::enumerateCoversInternal<6>(
        std::function<void(GroupPresentation&)>&& action);

} // namespace regina
