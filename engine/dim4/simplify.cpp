
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

#include "dim4/dim4triangulation.h"
#include "maths/nperm3.h"
#include "triangulation/nedge.h"
#include <sstream>

namespace regina {

namespace {
    // A helper routine that describes the mapping between subcomplexes
    // in a 2-4 / 4-2 move.
    //
    // For the two-pentachoron subcomplex S2, the common facet is 0123.
    // The pentachora are joined as follows:
    //   P0 : 0123 <-> P1 : 0123 (identity)
    //
    // For the four-pentachoron subcomplex S4, the common edge is 01.
    // The pentachora are joined as follows:
    //   P0 : 0134 <-> P1 : 0143
    //   P0 : 0124 <-> P2 : 0142
    //   P0 : 0123 <-> P3 : 0132
    //   P1 : 0123 <-> P2 : 0132
    //   P1 : 0124 <-> P3 : 0142
    //   P2 : 0134 <-> P3 : 0143
    //
    // For i in 0,1 and j in 0,1,2,3:
    // S2 pentachoron i, facet j <-> S4 pentachoron j, facet i.
    // The gluing permutation is as follows:
    //   S4,i   <-> S2,j
    //   S4,1-i <-> S2,4
    //   S4,2   <-> S2,(1-j) % 4
    //   S4,3   <-> S2,(2+j) % 4
    //   S4,4   <-> S2,(3-j)
    //
    // This routine merely constructs this gluing permutation.
    // The permutation returned runs from S4 to S2 (so i -> j).
    NPerm5 fourTwoPerm(int i /* 0 or 1 */, int j /* 0, 1, 2 or 3 */) {
        // Use & 3 instead of % 4 for non-negative integers.
        if (i == 0)
            return NPerm5(j, 4, (5 - j) & 3, (2 + j) & 3, (3 - j) & 3);
        else
            return NPerm5(4, j, (5 - j) & 3, (2 + j) & 3, (3 - j) & 3);
    }

    // A helper routine that describes the mapping between subcomplexes
    // in a 3-3 move.
    //
    // For each three-pentachoron subcomplex, the common triangle is 012.
    // The pentachora are joined as follows:
    //   P0 : 0124 <-> P1 : 0123
    //   P1 : 0124 <-> P2 : 0123
    //   P2 : 0124 <-> P0 : 0123
    //
    // For i in 0,1,2 and j in 0,1,2:
    // S3a pentachoron i, facet j <-> S3b pentachoron j, facet i.
    // The gluing permutation is as follows:
    //   S3a,j         <-> S3b,i
    //   S3a,(j+1) % 3 <-> S3b,3
    //   S3a,(j+2) % 3 <-> S3b,4
    //   S3a,3         <-> S3b,(i+1) % 3
    //   S3a,4         <-> S3b,(i+2) % 3
    //
    // This routine merely constructs this gluing permutation.
    // The permutation returned runs from S3b to S3a (so i -> j).
    // Note that threeThreePerm(i,j).inverse() == threeThreePerm(j,i).
    NPerm5 threeThreePerm(int i /* 0, 1 or 2 */, int j /* 0, 1 or 2 */) {
        if (i == 0)
            return NPerm5(j, 3, 4, (j + 1) % 3, (j + 2) % 3);
        else if (i == 1)
            return NPerm5(4, j, 3, (j + 1) % 3, (j + 2) % 3);
        else
            return NPerm5(3, 4, j, (j + 1) % 3, (j + 2) % 3);
    }

    // A helper routine that uses union-find to test whether a graph
    // contains cycles.  This is used by Dim4Triangulation::collapseEdge().
    //
    // This routine returns true if the given edge connects two distinct
    // components of the graph, or false if both endpoints of the edge
    // are already in the same component (i.e., a cycle has been created).
    //
    // This routine is a direct clone of the corresponding routine for
    // NTriangulation::collapseEdge().  It would be nice to reuse this
    // routine instead of duplicate it, but that is for another day.
    bool unionFindInsert(long* parent, long* depth, long vtx1, long vtx2) {
        // Find the root of the tree containing vtx1 and vtx2.
        long top1, top2;

        for (top1 = vtx1; parent[top1] >= 0; top1 = parent[top1])
            ;
        for (top2 = vtx2; parent[top2] >= 0; top2 = parent[top2])
            ;

        // Are both vertices in the same component?
        if (top1 == top2)
            return false;

        // Join the two components.
        // Insert the shallower tree beneath the deeper tree.
        if (depth[top1] < depth[top2]) {
            parent[top1] = top2;
        } else {
            parent[top2] = top1;
            if (depth[top1] == depth[top2])
                ++depth[top1];
        }
        return true;
    }
}

/**
 *  Not yet implemented. 
 */
bool Dim4Triangulation::fiveOneMove(Dim4Vertex* vrt, bool check, 
            bool perform) {
    // step 1: check if this is a valid move.  Is the link of this vertex
    //         the cone on a standard 5-tetrahedron sphere? 
    if (vrt->getNumberOfEmbeddings()!=5) return false;
    // TODO 


    // step 2: if so, let's remove 4 of those pentachora, and re-glue...


    return false;
}

bool Dim4Triangulation::fourTwoMove(Dim4Edge* e, bool check, bool perform) {
    const std::vector<Dim4EdgeEmbedding>& embs = e->getEmbeddings();
    if (check) {
        // e must be valid and non-boundary.
        if (e->isBoundary() || ! e->isValid())
            return false;
        // e must have degree four.
        if (embs.size() != 4)
            return false;
    }

    // e must meet four distinct pentachora, which must be glued around
    // the edge in a way that gives a 3-simplex link.  Find these pentachora.
    Dim4Pentachoron* oldPent[4];
    NPerm5 oldVertices[4]; // 01 -> edge, 234 -> link

    // We will permute oldVertices so that:
    // oldPent[0] / 34 -> oldPent[1] / 43
    // oldPent[0] / 24 -> oldPent[2] / 42
    // oldPent[0] / 23 -> oldPent[3] / 32
    // oldPent[1] / 23 -> oldPent[2] / 32
    // oldPent[1] / 24 -> oldPent[3] / 42
    // oldPent[2] / 34 -> oldPent[3] / 43
    // This is possible iff we have a 3-simplex link.

    oldPent[0] = embs[0].getPentachoron();
    oldVertices[0] = embs[0].getVertices();

    int i,j;
    for (i = 1; i < 4; ++i) {
        oldPent[i] = oldPent[0]->adjacentPentachoron(oldVertices[0][i + 1]);
        if (check)
            for (j = 0; j < i; ++j)
                if (oldPent[i] == oldPent[j])
                    return false;
        oldVertices[i] = oldPent[0]->adjacentGluing(oldVertices[0][i + 1]) *
            oldVertices[0] * NPerm5((i % 3) + 2, ((i + 1) % 3) + 2);
    }

    if (check) {
        if (oldPent[2] != oldPent[1]->adjacentPentachoron(oldVertices[1][4]))
            return false;
        if (oldPent[3] != oldPent[1]->adjacentPentachoron(oldVertices[1][3]))
            return false;
        if (oldPent[3] != oldPent[2]->adjacentPentachoron(oldVertices[2][2]))
            return false;

        if (oldVertices[2] != oldPent[1]->adjacentGluing(oldVertices[1][4])
                * oldVertices[1] * NPerm5(2, 3))
            return false;
        if (oldVertices[3] != oldPent[1]->adjacentGluing(oldVertices[1][3])
                * oldVertices[1] * NPerm5(2, 4))
            return false;
        if (oldVertices[3] != oldPent[2]->adjacentGluing(oldVertices[2][2])
                * oldVertices[2] * NPerm5(3, 4))
            return false;
    }

    if (! perform)
        return true;

    // Perform the move.
    bool rememberSimpleLinks = knownSimpleLinks_;

    ChangeEventSpan span(this);

    // Create two new pentachora.
    Dim4Pentachoron* newPent[2];
    for (i = 0; i < 2; ++i)
        newPent[i] = newPentachoron();

    // Find where their facets need to be glued.
    // Old pentachoron j, facet i <-> New pentachoron i, facet j.
    Dim4Pentachoron* adjPent[2][4];
    NPerm5 adjGluing[2][4];
    int k,l;
    for (i = 0; i < 2; ++i) { // new pentachora ; old facets
        for (j = 0; j < 4; ++j) { // new facets ; old pentachora
            adjPent[i][j] = oldPent[j]->adjacentPentachoron(oldVertices[j][i]);
            adjGluing[i][j] = oldPent[j]->adjacentGluing(oldVertices[j][i]) *
                oldVertices[j] * fourTwoPerm(i, j).inverse();

            // Are we are gluing a new pentachoron to itself?
            for (k = 0; k < 4; ++k) {
                if (adjPent[i][j] == oldPent[k]) {
                    for (l = 0; l < 2; ++l) {
                        if (adjGluing[i][j][j] == oldVertices[k][l]) {
                            // This glues to old pentachoron k,
                            // facet oldVertices[k][l].
                            // This means we glue new(i:j) to new(l:k).
                            if (i > l || (i == l && j > k)) {
                                // Ensure we make the gluing in just one
                                // direction, not both directions.
                                adjPent[i][j] = 0;
                            } else {
                                // Adjust the gluing to point to the new
                                // pentachoron.
                                adjPent[i][j] = newPent[l];
                                adjGluing[i][j] =
                                    fourTwoPerm(l, k) *
                                    oldVertices[k].inverse() *
                                    adjGluing[i][j];
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    // Now go ahead and make the gluings.
    for (j = 0; j < 4; ++j)
        oldPent[j]->isolate();
    for (i = 0; i < 2; ++i)
        for (j = 0; j < 4; ++j)
            if (adjPent[i][j])
                newPent[i]->joinTo(j, adjPent[i][j], adjGluing[i][j]);
    newPent[0]->joinTo(4, newPent[1], NPerm5());

    // Delete the old pentachora and insert the new.
    for (i = 0; i < 4; ++i)
        removePentachoron(oldPent[i]);

    // All done!
    knownSimpleLinks_ = rememberSimpleLinks;

    return true;
}

bool Dim4Triangulation::threeThreeMove(Dim4Triangle* f, bool check,
        bool perform) {
    const std::deque<Dim4TriangleEmbedding>& embs = f->getEmbeddings();
    if (check) {
        // f must be valid and non-boundary.
        if (f->isBoundary() || ! f->isValid())
            return false;
        // f must have degree three.
        if (embs.size() != 3)
            return false;
    }

    // f must meet three distinct pentachora.  Find these pentachora.
    Dim4Pentachoron* oldPent[3];
    NPerm5 oldVertices[3]; // 012 -> triangle, 34 -> link
    int i,j;
    for (i = 0; i < 3; ++i) {
        oldPent[i] = embs[i].getPentachoron();
        if (check)
            for (j = 0; j < i; ++j)
                if (oldPent[i] == oldPent[j])
                    return false;
        oldVertices[i] = embs[i].getVertices();
    }

    if (! perform)
        return true;

    // Perform the move.
    bool rememberSimpleLinks = knownSimpleLinks_;

    ChangeEventSpan span(this);

    // Create three new pentachora.
    Dim4Pentachoron* newPent[3];
    for (i = 0; i < 3; ++i)
        newPent[i] = newPentachoron();

    // Find where their facets need to be glued.
    // Old pentachoron j, facet i <-> New pentachoron i, facet j.
    Dim4Pentachoron* adjPent[3][3];
    NPerm5 adjGluing[3][3];
    int k,l;
    for (i = 0; i < 3; ++i) { // new pentachora ; old facets
        for (j = 0; j < 3; ++j) { // new facets ; old pentachora
            adjPent[i][j] = oldPent[j]->adjacentPentachoron(oldVertices[j][i]);
            adjGluing[i][j] = oldPent[j]->adjacentGluing(oldVertices[j][i]) *
                oldVertices[j] * threeThreePerm(j, i);

            // Are we are gluing a new pentachoron to itself?
            for (k = 0; k < 3; ++k) {
                if (adjPent[i][j] == oldPent[k]) {
                    for (l = 0; l < 3; ++l) {
                        if (adjGluing[i][j][j] == oldVertices[k][l]) {
                            // This glues to old pentachoron k,
                            // facet oldVertices[k][l].
                            // This means we glue new(i:j) to new(l:k).
                            if (i > l || (i == l && j > k)) {
                                // Ensure we make the gluing in just one
                                // direction, not both directions.
                                adjPent[i][j] = 0;
                            } else {
                                // Adjust the gluing to point to the new
                                // pentachoron.
                                adjPent[i][j] = newPent[l];
                                adjGluing[i][j] =
                                    threeThreePerm(l, k) *
                                    oldVertices[k].inverse() *
                                    adjGluing[i][j];
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    // Now go ahead and make the gluings.
    for (j = 0; j < 3; ++j)
        oldPent[j]->isolate();
    for (i = 0; i < 3; ++i)
        for (j = 0; j < 3; ++j)
            if (adjPent[i][j])
                newPent[i]->joinTo(j, adjPent[i][j], adjGluing[i][j]);
    newPent[0]->joinTo(3, newPent[1], NPerm5(3, 4));
    newPent[1]->joinTo(3, newPent[2], NPerm5(3, 4));
    newPent[2]->joinTo(3, newPent[0], NPerm5(3, 4));

    // Delete the old pentachora and insert the new.
    for (i = 0; i < 3; ++i)
        removePentachoron(oldPent[i]);

    // All done!
    knownSimpleLinks_ = rememberSimpleLinks;
    return true;
}

bool Dim4Triangulation::twoFourMove(Dim4Tetrahedron* f, bool check,
        bool perform) {
    if (check) {
        if (f->getNumberOfEmbeddings() != 2)
            return false;
        // We now know that the given facet is not on the boundary.
    }

    // f must meet two distinct pentachora.  Find these pentachora.
    Dim4Pentachoron* oldPent[2];
    NPerm5 oldVertices[2]; // 0123 -> facet.
    int i;
    for (i = 0; i < 2; ++i) {
        oldPent[i] = f->getEmbedding(i).getPentachoron();
        oldVertices[i] = f->getEmbedding(i).getVertices();
    }

    if (check)
        if (oldPent[0] == oldPent[1])
            return false;

    if (! perform)
        return true;

    // Perform the move.
    bool rememberSimpleLinks = knownSimpleLinks_;

    ChangeEventSpan span(this);

    // Create four new pentachora.
    Dim4Pentachoron* newPent[4];
    for (i = 0; i < 4; ++i)
        newPent[i] = newPentachoron();

    // Find where their facets need to be glued.
    // Old pentachoron j, facet i <-> New pentachoron i, facet j.
    Dim4Pentachoron* adjPent[4][2];
    NPerm5 adjGluing[4][2];
    int j,k,l;
    for (i = 0; i < 4; ++i) { // new pentachora ; old facets
        for (j = 0; j < 2; ++j) { // new facets ; old pentachora
            adjPent[i][j] = oldPent[j]->adjacentPentachoron(oldVertices[j][i]);
            adjGluing[i][j] = oldPent[j]->adjacentGluing(oldVertices[j][i]) *
                oldVertices[j] * fourTwoPerm(j, i);

            // Are we are gluing a new pentachoron to itself?
            for (k = 0; k < 2; ++k) {
                if (adjPent[i][j] == oldPent[k]) {
                    for (l = 0; l < 4; ++l) {
                        if (adjGluing[i][j][j] == oldVertices[k][l]) {
                            // This glues to old pentachoron k,
                            // facet oldVertices[k][l].
                            // This means we glue new(i:j) to new(l:k).
                            if (i > l || (i == l && j > k)) {
                                // Ensure we make the gluing in just one
                                // direction, not both directions.
                                adjPent[i][j] = 0;
                            } else {
                                // Adjust the gluing to point to the new
                                // pentachoron.
                                adjPent[i][j] = newPent[l];
                                adjGluing[i][j] =
                                    fourTwoPerm(k, l).inverse() *
                                    oldVertices[k].inverse() *
                                    adjGluing[i][j];
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    // Now go ahead and make the gluings.
    for (j = 0; j < 2; ++j)
        oldPent[j]->isolate();
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 2; ++j)
            if (adjPent[i][j])
                newPent[i]->joinTo(j, adjPent[i][j], adjGluing[i][j]);
    newPent[0]->joinTo(2, newPent[1], NPerm5(3, 4));
    newPent[0]->joinTo(3, newPent[2], NPerm5(2, 4));
    newPent[0]->joinTo(4, newPent[3], NPerm5(2, 3));
    newPent[1]->joinTo(4, newPent[2], NPerm5(2, 3));
    newPent[1]->joinTo(3, newPent[3], NPerm5(2, 4));
    newPent[2]->joinTo(2, newPent[3], NPerm5(3, 4));

    // Delete the old pentachora and insert the new.
    for (i = 0; i < 2; ++i)
        removePentachoron(oldPent[i]);

    // All done!
    knownSimpleLinks_ = rememberSimpleLinks;
    return true;
}


// subordinate to oneFiveMove.  Make static (i.e. nonexported?)
NPerm5 fourShift(const unsigned long &i)
{ // 4 maps to index of facet of old pen, which is 4-i, 
  // the pentachora: pen, newPen[i] are across from all other labels increasing
 if (i==4) return NPerm5();
 else return NPerm5(i==3 ? 1 : 0, i>=2 ? 2 : 1, i>=1 ? 3 : 2, 4, 3-i);
}

bool Dim4Triangulation::oneFiveMove(Dim4Pentachoron* pen, bool check, 
    bool perform) {
    if ( !perform ) return true; // you can always do this move. 
    // not yet okay-ed by bab
    #ifdef DEBUG
    std::cerr << "Performing 1->5 move\n";
    #endif

    // before we unglue, record how the adjacent pentachora are glued to pen. 
    Dim4Pentachoron* oldPens[5];        
    NPerm5 oldGlue[5];
    for (unsigned long i=0; i<5; i++) { 
        oldPens[i] = pen->adjacentPentachoron(i); 
        if (oldPens[i]!=NULL) oldGlue[i] = pen->adjacentGluing(i); 
        }

    // unglue facets 0,1,2,3 of pen, keep gluing of 4th facet. 
    for (unsigned long i=0; i<4; i++) if (pen->adjacentPentachoron(i)!=NULL)
         pen->unjoin(i); // joined to self? 

    // the new pentachora
    Dim4Pentachoron* newPens[4];
    for (unsigned long i=0; i<4; i++) newPens[i] = newPentachoron();

    // pen -> newPens STEP 1
    pen->joinTo(3, newPens[0], NPerm5());           // pen 3 -> nP[0] 3
    pen->joinTo(2, newPens[1], NPerm5(0,1,3,2,4));  // pen 2 -> nP[1] 3
    pen->joinTo(1, newPens[2], NPerm5(0,3,1,2,4));  // pen 1 -> nP[2] 3
    pen->joinTo(0, newPens[3], NPerm5(3,0,1,2,4));  // pen 0 -> nP[3] 3

    // attach the newPens to the old triangulation. STEP 2
    for (unsigned long i=0; i<4; i++)
     { // if pen was glued to itself across facet i, be careful about how newPen
       // is glued... 
       if (oldPens[i] == pen) { // pen self-gluing
        if (newPens[3-i]->adjacentPentachoron(4) == NULL) // 1st time through
         {
          if (oldGlue[i][i]==4) // facet 4 was unglued, special case
            newPens[3-i]->joinTo(4, pen, 
            fourShift(4).inverse()*oldGlue[i]*fourShift(3-i));
          else // pair matching of two of facets 0,1,2,3
            newPens[3-i]->joinTo(4, newPens[3-oldGlue[i][i]], 
                fourShift(3-oldGlue[i][i]).inverse()*
                oldGlue[i]*fourShift(3-i) );
         }
        } // end self-gluings
       else if (oldPens[i]!=NULL) newPens[3-i]->joinTo(4, oldPens[i], 
                oldGlue[i]*fourShift(3-i) );
     } 

    // glue the newPens to each other, where appropriate
    newPens[0]->joinTo(2, newPens[1], NPerm5() );         // nP[0] 2 to nP[1] 2
    newPens[1]->joinTo(1, newPens[2], NPerm5() );         // nP[1] 1 to nP[2] 1
    newPens[2]->joinTo(0, newPens[3], NPerm5() );         // nP[2] 0 to nP[3] 0
    newPens[0]->joinTo(1, newPens[2], NPerm5(0,2,1,3,4) );// nP[0] 1 to nP[2] 2
    newPens[1]->joinTo(0, newPens[3], NPerm5(1,0,2,3,4) );// nP[1] 0 to nP[3] 1 
    newPens[0]->joinTo(0, newPens[3], NPerm5(2,0,1,3,4) );// nP[0] 0 to nP[3] 2

    return true;
}

bool Dim4Triangulation::twoZeroMove(Dim4Triangle* t, bool check, bool perform) {
    if (check) {
        if (t->isBoundary() || ! t->isValid())
            return false;
        if (t->getNumberOfEmbeddings() != 2)
            return false;
    }

    Dim4Pentachoron* pent[2];
    NPerm5 perm[2];

    int i;
    for (i = 0; i < 2; ++i) {
        pent[i] = t->getEmbedding(i).getPentachoron();
        perm[i] = t->getEmbedding(i).getVertices();
    }

    // Lots of checks required...
    if (check) {
        // The two pentachora must be distinct.
        if (pent[0] == pent[1])
            return false;

        Dim4Edge* edge[2];
        Dim4Triangle* tri[2][3];
        Dim4Tetrahedron* tet[2][3];

        for (i = 0; i < 2; ++i) {
            edge[i] = pent[i]->getEdge(
                Dim4Edge::edgeNumber[perm[i][3]][perm[i][4]]);
            tri[i][0] = pent[i]->getTriangle(Dim4Triangle::triangleNumber
                [perm[i][0]][perm[i][3]][perm[i][4]]);
            tri[i][1] = pent[i]->getTriangle(Dim4Triangle::triangleNumber
                [perm[i][1]][perm[i][3]][perm[i][4]]);
            tri[i][2] = pent[i]->getTriangle(Dim4Triangle::triangleNumber
                [perm[i][2]][perm[i][3]][perm[i][4]]);
            tet[i][0] = pent[i]->getTetrahedron(perm[i][0]);
            tet[i][1] = pent[i]->getTetrahedron(perm[i][1]);
            tet[i][2] = pent[i]->getTetrahedron(perm[i][2]);
        }

        // No bad loops of edges.
        // Closed loops:
        if (edge[0] == edge[1])
            return false;
        // Bounded loops:
        if (edge[0]->isBoundary() && edge[1]->isBoundary())
            return false;

        // No bad loops of triangles.
        // Closed loops of length 1:
        for (i = 0; i < 3; ++i)
            if (tri[0][i] == tri[1][i])
                return false;
        // Closed loops of length 2:
        for (i = 0; i < 3; ++i) {
            if (tri[0][i] == tri[0][(i + 1) % 3] &&
                    tri[1][i] == tri[1][(i + 1) % 3])
                return false;
            if (tri[0][i] == tri[1][(i + 1) % 3] &&
                    tri[1][i] == tri[0][(i + 1) % 3])
                return false;
        }
        // Closed loops of length 3:
        if (tri[0][0] == tri[1][1] && tri[0][1] == tri[1][2] &&
                tri[0][2] == tri[1][0])
            return false;
        if (tri[1][0] == tri[0][1] && tri[1][1] == tri[0][2] &&
                tri[1][2] == tri[0][0])
            return false;
        for (i = 0; i < 6; ++i)
            if (tri[0][NPerm3::S3[i][0]] == tri[0][NPerm3::S3[i][1]] &&
                    tri[1][NPerm3::S3[i][1]] == tri[1][NPerm3::S3[i][2]] &&
                    tri[0][NPerm3::S3[i][2]] == tri[1][NPerm3::S3[i][0]])
                return false;
        // Bounded loops not already covered by the earlier edge-based test:
        for (i = 0; i < 3; ++i) {
            if (tri[0][i]->isBoundary() &&
                    tri[1][i] == tri[1][(i + 1) % 3] &&
                    tri[0][(i + 1) % 3]->isBoundary())
                return false;
            if (tri[1][i]->isBoundary() &&
                    tri[0][i] == tri[0][(i + 1) % 3] &&
                    tri[1][(i + 1) % 3]->isBoundary())
                return false;
        }
        for (i = 0; i < 6; ++i) {
            if (tri[0][NPerm3::S3[i][0]]->isBoundary() &&
                    tri[1][NPerm3::S3[i][0]] == tri[1][NPerm3::S3[i][1]] &&
                    tri[0][NPerm3::S3[i][1]] == tri[1][NPerm3::S3[i][2]] &&
                    tri[0][NPerm3::S3[i][2]]->isBoundary())
                return false;
            if (tri[1][NPerm3::S3[i][0]]->isBoundary() &&
                    tri[0][NPerm3::S3[i][0]] == tri[0][NPerm3::S3[i][1]] &&
                    tri[1][NPerm3::S3[i][1]] == tri[0][NPerm3::S3[i][2]] &&
                    tri[1][NPerm3::S3[i][2]]->isBoundary())
                return false;
        }

        // No bad loops of tetrahedra.
        // Closed loops of length 1:
        for (i = 0; i < 3; ++i)
            if (tet[0][i] == tet[1][i])
                return false;
        // Closed loops of length 2:
        for (i = 0; i < 3; ++i) {
            if (tet[0][i] == tet[0][(i + 1) % 3] &&
                    tet[1][i] == tet[1][(i + 1) % 3])
                return false;
            if (tet[0][i] == tet[1][(i + 1) % 3] &&
                    tet[1][i] == tet[0][(i + 1) % 3])
                return false;
        }
        // Bounded loops of length 2 not already covered by the earlier
        // edge-based test:
        for (i = 0; i < 3; ++i) {
            if (tet[0][i]->isBoundary() &&
                    tet[1][i] == tet[1][(i + 1) % 3] &&
                    tet[0][(i + 1) % 3]->isBoundary())
                return false;
            if (tet[1][i]->isBoundary() &&
                    tet[0][i] == tet[0][(i + 1) % 3] &&
                    tet[1][(i + 1) % 3]->isBoundary())
                return false;
        }
        // Closed and bounded loops of length 3 are all covered by the
        // following check:
        if (pent[0]->getComponent()->getNumberOfPentachora() == 2)
            return false;
    }

    if (! perform)
        return true;

    // Perform the move.
    bool rememberSimpleLinks = knownSimpleLinks_;

    ChangeEventSpan span(this);

    // Unglue facets from the doomed pentachora and glue them to each other.
    NPerm5 crossover = pent[0]->adjacentGluing(perm[0][3]);
    NPerm5 gluing;
    Dim4Pentachoron *top, *bottom;
    int topFacet;
    for (i = 0; i < 3; ++i) {
        top = pent[0]->adjacentPentachoron(perm[0][i]);
        bottom = pent[1]->adjacentPentachoron(perm[1][i]);

        if (! top) {
            // Bottom facet becomes boundary.
            pent[1]->unjoin(perm[1][i]);
        } else if (! bottom) {
            // Top facet becomes boundary.
            pent[0]->unjoin(perm[0][i]);
        } else {
            // Bottom and top facets join.
            topFacet = pent[0]->adjacentFacet(perm[0][i]);
            gluing = pent[1]->adjacentGluing(perm[1][i]) *
                crossover * top->adjacentGluing(topFacet);
            pent[0]->unjoin(perm[0][i]);
            pent[1]->unjoin(perm[1][i]);
            top->joinTo(topFacet, bottom, gluing);
        }
    }

    // Finally remove and dispose of the pentachora.
    removePentachoron(pent[0]);
    removePentachoron(pent[1]);

    knownSimpleLinks_ = rememberSimpleLinks;
    return true;
}

bool Dim4Triangulation::twoZeroMove(Dim4Edge* e, bool check, bool perform) {
    if (check) {
        // The follow test also implicitly ensures that the edge link is
        // a 2-sphere.  See Dim4Edge::isValid() for details.
        if (e->isBoundary() || ! e->isValid())
            return false;
        if (e->getNumberOfEmbeddings() != 2)
            return false;
    }

    Dim4Pentachoron* pent[2];
    NPerm5 perm[2];

    int i;
    for (i = 0; i < 2; ++i) {
        pent[i] = e->getEmbedding(i).getPentachoron();
        perm[i] = e->getEmbedding(i).getVertices();
    }

    if (check) {
        if (pent[0] == pent[1])
            return false;

        // No bad loops of triangles.
        Dim4Triangle* tri[2];
        for (i = 0; i < 2; ++i)
            tri[i] = pent[i]->getTriangle(Dim4Triangle::triangleNumber
                [perm[i][2]][perm[i][3]][perm[i][4]]);

        if (tri[0] == tri[1])
            return false;
        if (tri[0]->isBoundary() && tri[1]->isBoundary())
            return false;

        // No bad loops of tetrahedra.
        Dim4Tetrahedron* tet[2][2];
        for (i = 0; i < 2; ++i) {
            tet[i][0] = pent[i]->getTetrahedron(perm[i][0]);
            tet[i][1] = pent[i]->getTetrahedron(perm[i][1]);
        }

        if (tet[0][0] == tet[1][0] || tet[0][1] == tet[1][1])
            return false;

        // The cases with two pairs of identified facets and with one
        // pair of identified facets plus one pair of boundary facets
        // are all covered by the following check.
        if (pent[0]->getComponent()->getNumberOfPentachora() == 2)
            return false;

        // Check that the pentachora are joined along all three facets.
        if (pent[0]->adjacentPentachoron(perm[0][2]) != pent[1])
            return false;
        if (pent[0]->adjacentPentachoron(perm[0][3]) != pent[1])
            return false;
        if (pent[0]->adjacentPentachoron(perm[0][4]) != pent[1])
            return false;
    }

    if (! perform)
        return true;

    // Perform the move.
    bool rememberSimpleLinks = knownSimpleLinks_;

    ChangeEventSpan span(this);

    // Unglue facets from the doomed pentachora and glue them to each other.
    NPerm5 crossover = pent[0]->adjacentGluing(perm[0][2]);
    NPerm5 gluing;
    Dim4Pentachoron *top, *bottom;
    int topFacet;
    for (i = 0; i < 2; ++i) {
        top = pent[0]->adjacentPentachoron(perm[0][i]);
        bottom = pent[1]->adjacentPentachoron(perm[1][i]);

        if (! top) {
            // Bottom facet becomes boundary.
            pent[1]->unjoin(perm[1][i]);
        } else if (! bottom) {
            // Top facet becomes boundary.
            pent[0]->unjoin(perm[0][i]);
        } else {
            // Bottom and top facets join.
            topFacet = pent[0]->adjacentFacet(perm[0][i]);
            gluing = pent[1]->adjacentGluing(perm[1][i]) *
                crossover * top->adjacentGluing(topFacet);
            pent[0]->unjoin(perm[0][i]);
            pent[1]->unjoin(perm[1][i]);
            top->joinTo(topFacet, bottom, gluing);
        }
    }

    // Finally remove and dispose of the pentachora.
    removePentachoron(pent[0]);
    removePentachoron(pent[1]);

    knownSimpleLinks_ = rememberSimpleLinks;
    return true;
}

bool Dim4Triangulation::openBook(Dim4Tetrahedron* t, bool check, bool perform) {
    const Dim4TetrahedronEmbedding& emb = t->getEmbedding(0);
    Dim4Pentachoron* pent = emb.getPentachoron();

    // Check that the triangle has exactly two boundary edges.
    // Note that this will imply that the triangle joins two tetrahedra.
    if (check) {
        int i;
        for (i = 0; i < 4; ++i)
            if (! t->getVertex(i)->isValid())
                return false;
        for (i = 0; i < 6; ++i)
            if (! t->getEdge(i)->isValid())
                return false;
        for (i = 0; i < 4; ++i)
            if (! t->getTriangle(i)->isValid())
                return false;

        NPerm5 vertices = emb.getVertices();

        int nBdry = 0;
        int bdryTriangle[4];

        for (i = 0; i < 4; ++i)
            if (t->getTriangle(i)->isBoundary())
                bdryTriangle[nBdry++] = i;

        if (nBdry < 1 || nBdry > 3)
            return false;

        if (nBdry == 2) {
            // Remaining edge is non-boundary.
            int edge = NEdge::edgeNumber[bdryTriangle[0]][bdryTriangle[1]];
            if (t->getEdge(edge)->isBoundary())
                return false;

            // Remaining two triangles are not identified.
            if (t->getTriangle(NEdge::edgeVertex[5 - edge][0]) ==
                    t->getTriangle(NEdge::edgeVertex[5 - edge][1]))
                return false;
        } else if (nBdry == 1) {
            // Remaining vertex is non-boundary.
            if (t->getVertex(bdryTriangle[0])->isBoundary())
                return false;

            // No two of the remaining three edges are identified.
            Dim4Edge* internal[3];
            internal[0] = t->getEdge(
                NEdge::edgeNumber[bdryTriangle[0]][(bdryTriangle[0] + 1) % 4]);
            internal[1] = t->getEdge(
                NEdge::edgeNumber[bdryTriangle[0]][(bdryTriangle[0] + 2) % 4]);
            internal[2] = t->getEdge(
                NEdge::edgeNumber[bdryTriangle[0]][(bdryTriangle[0] + 3) % 4]);

            if (internal[0] == internal[1] || internal[1] == internal[2] ||
                    internal[2] == internal[0])
                return false;
        }
    }

    if (! perform)
        return true;

    // Actually perform the move.
    // Don't bother with a block since this is so simple.
    bool rememberSimpleLinks = knownSimpleLinks_;

    pent->unjoin(emb.getTetrahedron());

    knownSimpleLinks_ = rememberSimpleLinks;
    return true;
}

bool Dim4Triangulation::shellBoundary(Dim4Pentachoron* p,
        bool check, bool perform) {
    // To perform the move we don't even need a skeleton.
    if (check) {
        if (! calculatedSkeleton_)
            calculateSkeleton();

        // All edges and triangles must be valid.
        int i;
        for (i = 0; i < 10; ++i)
            if (! p->getEdge(i)->isValid())
                return false;
        for (i = 0; i < 10; ++i)
            if (! p->getTriangle(i)->isValid())
                return false;

        // Precisely 1, 2, 3 or 4 boundary facets.
        int nBdry = 0;
        int bdry[5];
        for (i = 0; i < 5; ++i)
            if (p->getTetrahedron(i)->isBoundary())
                bdry[nBdry++] = i;
        if (nBdry < 1 || nBdry > 4)
            return false;

        if (nBdry == 1) {
            // Opposite vertex not in boundary.
            if (p->getVertex(bdry[0])->isBoundary())
                return false;

            // No two of the remaining four edges identified.
            Dim4Edge* internal[4];
            int j = 0;
            for (i = 0; i < 5; ++i)
                if (i != bdry[0])
                    internal[j++] = p->getEdge(
                        Dim4Edge::edgeNumber[bdry[0]][i]);

            for (i = 0; i < 4; ++i)
                for (j = i + 1; j < 4; ++j)
                    if (internal[i] == internal[j])
                        return false;
        } else if (nBdry == 2) {
            // Opposite edge not in boundary.
            i = Dim4Edge::edgeNumber[bdry[0]][bdry[1]];
            if (p->getEdge(i)->isBoundary())
                return false;

            // No two of the remaining three triangles identified.
            Dim4Triangle* internal[3];
            int j = 0;
            for (i = 0; i < 5; ++i)
                if (i != bdry[0] && i != bdry[1])
                    internal[j++] = p->getTriangle(
                        Dim4Triangle::triangleNumber[bdry[0]][bdry[1]][i]);

            if (internal[0] == internal[1] ||
                    internal[1] == internal[2] ||
                    internal[2] == internal[0])
                return false;
        } else if (nBdry == 3) {
            // Opposite triangle not in boundary.
            i = Dim4Triangle::triangleNumber[bdry[0]][bdry[1]][bdry[2]];
            if (p->getTriangle(i)->isBoundary())
                return false;

            // Remaining two facets not identified.
            if (p->adjacentPentachoron(Dim4Edge::edgeVertex[i][0]) == p)
                return false;
        }
    }

    if (! perform)
        return true;

    // Actually perform the move.
    // Don't bother with a block since this is so simple.
    bool rememberSimpleLinks = knownSimpleLinks_;

    removePentachoron(p);

    knownSimpleLinks_ = rememberSimpleLinks;
    return true;
}

bool Dim4Triangulation::collapseEdge(Dim4Edge* e, bool check, bool perform) {
    // Find the pentachora to remove.
    const std::vector<Dim4EdgeEmbedding>& embs = e->getEmbeddings();

    std::vector<Dim4EdgeEmbedding>::const_iterator it;
    Dim4Pentachoron* pent = 0;
    NPerm5 p;

    if (check) {
        // We need a valid edge before we test anything else.
        // From this we know that the edge link is a disc or sphere.
        if (! e->isValid())
            return false;

        // CHECK 0: The pentachora around the edge must be distinct.
        // We check this as follows:
        //
        // - None of the triangles containing edge e must contain e twice.
        //   We throw this into check 2 below (see points [0a] and [0b]).
        //
        // - The only remaining bad cases involve some tetrahedron with
        //   e as two opposite edges.  In this case one can prove that
        //   we have a bad chain of bigons, which will be picked up in
        //   check 2 below.  This works both for internal tetrahedra
        //   (which give a bad chain of internal bigons) and boundary
        //   tetrahedra (which give a bad chain of boundary bigons).

        // CHECK 1: Can we collapse the edge to a point (which in turn
        // collapses triangles to bigons and so on up the dimensions)?

        // The vertices must be distinct.
        if (e->getVertex(0) == e->getVertex(1))
            return false;

        // If both vertices are in the boundary then we must be collapsing a
        // boundary edge, and both vertices must have plain old ball links.
        // Recall that ideal vertices return isBoundary() == true.
        if (e->getVertex(0)->isBoundary() && e->getVertex(1)->isBoundary()) {
            if (! e->isBoundary())
                return false;

            // Since e is a boundary edge, both vertex links are bounded
            // 3-manifolds.  This means that the vertex links are balls
            // if and only if the vertices are valid.
            if (! e->getVertex(0)->isValid())
                return false;
            if (! e->getVertex(1)->isValid())
                return false;
        }

        // CHECK 2: Triangles containing the edge have now become bigons.
        // Can we flatten each bigon to an edge (leaving behind
        // triangular pillows behind and so on up the dimensions)?
        //
        // This is trickier.  Even if every individual bigon is okay, we
        // don't want a _chain_ of bigons together to crush a sphere or
        // projective plane.
        //
        // The way we do this is as follows.  Consider each Dim4Edge* to be
        // a vertex of some graph G, and consider each bigon to be an edge
        // in this graph G.  The vertices at either end of the edge in G
        // are the (Dim4Edge*)s that bound the bigon.
        //
        // We can happily flatten each bigon if and only if the graph G
        // contains no cycles.  We shall test this using union-find,
        // which should have log-linear complexity.
        //
        // We deal with boundary edges and invalid edges as follows.
        // All boundary and/or invalid edges become the *same* vertex in
        // the graph G.  This means, for instance, that a bigon joining two
        // distinct boundary edges is not allowed.
        //
        // If edge e is itself a boundary edge, things become more
        // interesting again.  In this case, the *boundary* bigons
        // are not subject to the same restrictions -- crushing bigons
        // along the boundary does no harm, *unless* the boundary bigon
        // edges themselves form a cycle.  This is essentially the same
        // dilemma as before but one dimension down, and we must test this
        // separately.
        {
            long nEdges = edges_.size();

            // The parent of each edge in the union-find tree, or -1 if
            // an edge is at the root of a tree.
            //
            // This array is indexed by edge number in the triangulation.
            // Although we might not use many of these edges, it's fast
            // and simple.  The "unified boundary" is assigned the edge
            // number nEdges.
            long* parent = new long[nEdges + 1];

            // The depth of each subtree in the union-find tree.
            long* depth = new long[nEdges + 1];

            Dim4Edge *upper, *lower;
            Dim4Triangle* triangle;
            TriangleIterator fit;
            long id1, id2;
            int i;

            if (e->isBoundary()) {
                // Search for cycles in boundary bigons.

                std::fill(parent, parent + nEdges + 1, -1);
                std::fill(depth, depth + nEdges + 1, 0);

                // Run through all boundary triangles containing e.
                for (fit = triangles_.begin(); fit != triangles_.end(); ++fit) {
                    triangle = *fit;
                    if (! triangle->isBoundary())
                        continue;

                    for (i = 0; i < 3; ++i)
                        if (triangle->getEdge(i) == e)
                            break;
                    if (i == 3)
                        continue;

                    // This triangle contains edge e (specifically, as edge i
                    // of this triangle).

                    upper = triangle->getEdge((i + 1) % 3);
                    lower = triangle->getEdge((i + 2) % 3);

                    if (upper == e || lower == e) {
                        // [0a]: Check 0 fails; this triangle contains edge e
                        // more than once.
                        delete[] depth;
                        delete[] parent;
                        return false;
                    }

                    // This bigon joins nodes id1 and id2 in the graph G.
                    if (! unionFindInsert(parent, depth,
                            upper->markedIndex(), lower->markedIndex())) {
                        delete[] depth;
                        delete[] parent;
                        return false;
                    }
                }

                // All looks good on the boundary.
            }

            // Search for cycles in internal bigons.

            std::fill(parent, parent + nEdges + 1, -1);
            std::fill(depth, depth + nEdges + 1, 0);

            // Run through all internal triangles containing e.
            for (fit = triangles_.begin(); fit != triangles_.end(); ++fit) {
                triangle = *fit;
                if (triangle->isBoundary())
                    continue;

                for (i = 0; i < 3; ++i)
                    if (triangle->getEdge(i) == e)
                        break;
                if (i == 3)
                    continue;

                // This triangle contains edge e (specifically, as edge i
                // of this triangle).

                upper = triangle->getEdge((i + 1) % 3);
                lower = triangle->getEdge((i + 2) % 3);

                if (upper == e || lower == e) {
                    // [0b]: Check 0 fails; this triangle contains edge e
                    // more than once.
                    delete[] depth;
                    delete[] parent;
                    return false;
                }

                id1 = ((upper->isBoundary() || ! upper->isValid()) ?
                    nEdges : upper->markedIndex());
                id2 = ((lower->isBoundary() || ! lower->isValid()) ?
                    nEdges : lower->markedIndex());

                // This bigon joins nodes id1 and id2 in the graph G.
                if (! unionFindInsert(parent, depth, id1, id2)) {
                    delete[] depth;
                    delete[] parent;
                    return false;
                }
            }

            // All looks good internally also.

            // No bad chains of bigons!
            delete[] depth;
            delete[] parent;
        }

        // CHECK 3: Tetrahedra containing the edge have now become
        // triangular pillows.  Can we flatten each pillow to a triangle
        // (leaving behind "tetrahedral 4-pillows" in higher dimensions)?
        //
        // We deal with this the same way we deal with flattening bigons
        // to edges.  Again, we must treat internal pillows and
        // boundary pillows separately.
        {
            long nTriangles = triangles_.size();

            // The parent of each triangle in the union-find tree, or -1 if
            // a triangle is at the root of a tree.
            //
            // This array is indexed by triangle number in the triangulation.
            // Although we might not use many of these triangles, it's fast
            // and simple.  The "unified boundary" is assigned the triangle
            // number nTriangles.
            long* parent = new long[nTriangles + 1];

            // The depth of each subtree in the union-find tree.
            long* depth = new long[nTriangles + 1];

            Dim4Triangle *upper, *lower;
            Dim4Tetrahedron* tet;
            TetrahedronIterator tit;
            long id1, id2;
            int i;

            if (e->isBoundary()) {
                // Search for cycles in boundary pillows.

                std::fill(parent, parent + nTriangles + 1, -1);
                std::fill(depth, depth + nTriangles + 1, 0);

                // Run through all boundary tetrahedra containing e.
                for (tit = tetrahedra_.begin(); tit != tetrahedra_.end();
                        ++tit) {
                    tet = *tit;
                    if (! tet->isBoundary())
                        continue;

                    for (i = 0; i < 6; ++i)
                        if (tet->getEdge(i) == e)
                            break;
                    if (i == 6)
                        continue;

                    // This tetrahedron contains edge e (specifically, as
                    // edge i of this tetrahedron).

                    upper = tet->getTriangle(NEdge::edgeVertex[i][0]);
                    lower = tet->getTriangle(NEdge::edgeVertex[i][1]);

                    if (! unionFindInsert(parent, depth,
                            upper->markedIndex(), lower->markedIndex())) {
                        delete[] depth;
                        delete[] parent;
                        return false;
                    }
                }

                // All looks good on the boundary.
            }

            // Search for cycles in internal pillows.

            std::fill(parent, parent + nTriangles + 1, -1);
            std::fill(depth, depth + nTriangles + 1, 0);

            // Run through all internal tetrahedra containing e.
            for (tit = tetrahedra_.begin(); tit != tetrahedra_.end(); ++tit) {
                tet = *tit;
                if (tet->isBoundary())
                    continue;

                for (i = 0; i < 6; ++i)
                    if (tet->getEdge(i) == e)
                        break;
                if (i == 6)
                    continue;

                // This tetrahedron contains edge e (specifically, as edge i
                // of this tetrahedron).

                upper = tet->getTriangle(NEdge::edgeVertex[i][0]);
                lower = tet->getTriangle(NEdge::edgeVertex[i][1]);

                id1 = ((upper->isBoundary() || ! upper->isValid()) ?
                    nTriangles : upper->markedIndex());
                id2 = ((lower->isBoundary() || ! lower->isValid()) ?
                    nTriangles : lower->markedIndex());

                // This pillow joins nodes id1 and id2 in the graph G.
                if (! unionFindInsert(parent, depth, id1, id2)) {
                    delete[] depth;
                    delete[] parent;
                    return false;
                }
            }

            // All looks good internally also.

            // No bad chains of bigons!
            delete[] depth;
            delete[] parent;
        }

        // CHECK 4: Can we flatten each tetrahedral 4-pillow to a tetrahedron?
        //
        // Again, even if each individual pillow is okay, we don't want
        // a chain of pillows together to completely crush away a
        // 4-manifold component.
        //
        // This means no cycles of pillows, and no chains of pillows
        // that run from boundary to boundary.
        //
        // Test this in the same way that we tested edges.  It's kind of
        // overkill, since each vertex in the corresponding graph G will
        // have degree <= 2, but it's fast so we'll do it.
        {
            long nTets = tetrahedra_.size();

            // The parent of each tetrahedron in the union-find tree,
            // or -1 if a tetrahedron is at the root of a tree.
            //
            // This array is indexed by tetrahedron number in the triangulation.
            // The "unified boundary" is assigned the triangle number nTets.
            long* parent = new long[nTets + 1];
            std::fill(parent, parent + nTets + 1, -1);

            // The depth of each subtree in the union-find tree.
            long* depth = new long[nTets + 1];
            std::fill(depth, depth + nTets + 1, 0);

            Dim4Tetrahedron *upper, *lower;
            long id1, id2;

            for (it = embs.begin(); it != embs.end(); ++it) {
                pent = it->getPentachoron();
                p = it->getVertices();

                upper = pent->getTetrahedron(p[0]);
                lower = pent->getTetrahedron(p[1]);

                id1 = (upper->isBoundary() ? nTets : upper->markedIndex());
                id2 = (lower->isBoundary() ? nTets : lower->markedIndex());

                // This 4-pillow joins nodes id1 and id2 in the graph G.
                if (! unionFindInsert(parent, depth, id1, id2)) {
                    delete[] depth;
                    delete[] parent;
                    return false;
                }
            }

            // No bad chains of 4-pillows!
            delete[] depth;
            delete[] parent;
        }
    }

    if (! perform)
        return true;

    // Perform the move.
    bool rememberSimpleLinks = knownSimpleLinks_;

    ChangeEventSpan span(this);
    NPerm5 topPerm, botPerm;
    Dim4Pentachoron *top, *bot;

    // Clone the edge embeddings because we cannot rely on skeletal
    // objects once we start changing the triangulation.
    unsigned nEmbs = embs.size();
    Dim4Pentachoron** embPent = new Dim4Pentachoron*[nEmbs];
    NPerm5* embVert = new NPerm5[nEmbs];
    unsigned i;
    for (i = 0; i < embs.size(); ++i) {
        embPent[i] = embs[i].getPentachoron();
        embVert[i] = embs[i].getVertices();
    }

    for (i = 0; i < nEmbs; ++i) {
        top = embPent[i]->adjacentPentachoron(embVert[i][0]);
        topPerm = embPent[i]->adjacentGluing(embVert[i][0]);
        bot = embPent[i]->adjacentPentachoron(embVert[i][1]);
        botPerm = embPent[i]->adjacentGluing(embVert[i][1]);

        embPent[i]->isolate();
        if (top && bot)
            top->joinTo(topPerm[embVert[i][0]], bot,
                botPerm * NPerm5(embVert[i][0], embVert[i][1]) *
                topPerm.inverse());

        removePentachoron(embPent[i]);
    }

    delete[] embPent;
    delete[] embVert;

    knownSimpleLinks_ = rememberSimpleLinks;
    return true;
}

} // namespace regina
