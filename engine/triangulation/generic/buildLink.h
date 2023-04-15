#ifndef __REGINA_BUILD_LINK_H
#define __REGINA_BUILD_LINK_H

#include "triangulation/dim2.h"
#include "triangulation/dim3.h"
#include "triangulation/dim4.h"
#include "triangulation/generic/triangulation.h"
//#include "triangulation/generic/face.h"

namespace regina {

template<int dim, int subdim = 0>
Triangulation<dim - subdim - 1> buildLink(const Face<dim, subdim> &face)
{
    Triangulation<dim - subdim - 1> ans;

    typename Triangulation<dim - subdim - 1>::ChangeEventSpan span(ans);

    ans.newSimplices(face.degree());

    size_t linkSimplexIndex = 0;

    for (const FaceEmbedding<dim, subdim> &embedding : face) {
        Simplex<dim>* const simplex = embedding.simplex();
        Simplex<dim - subdim - 1> * const linkSimplex =
            ans.simplex(linkSimplexIndex);
        const Perm<dim + 1> &vertices = embedding.vertices();

        for (size_t j = 0; j < dim - subdim; ++j) {

            if (linkSimplex->adjacentSimplex(j))
                continue;

            const int facet = vertices[subdim + 1 + j];

            Simplex<dim>* const adjacentSimplex =
                simplex->adjacentSimplex(facet);
            if (! adjacentSimplex)
                continue;

            const Perm<dim + 1> &adjacentGluing =
                simplex->adjacentGluing(facet);

            const int adjacentFace =
                subdim == 0
                    ? adjacentGluing[vertices[0]]
                    : FaceNumbering<dim, subdim>::faceNumber(
                        adjacentGluing * vertices);

            size_t adjacentLinkSimplexIndex = 0;
            
            for (const FaceEmbedding<dim, subdim> &adjacentEmbedding : face) {
                if (adjacentEmbedding.simplex() == adjacentSimplex &&
                    adjacentEmbedding.face() == adjacentFace) {

                    const Perm<dim + 1> adjacentVertices =
                        adjacentEmbedding.vertices();

                    Simplex<dim - subdim - 1> * const adjacentLinkSimplex =
                        ans.simplex(adjacentLinkSimplexIndex);
                    
                    linkSimplex->join(
                        j,
                        adjacentLinkSimplex,
                        Perm<dim - subdim>::contractFront(
                            adjacentVertices.inverse() *
                            adjacentGluing *
                            vertices));
                    break;
                }
                ++adjacentLinkSimplexIndex;
            }
                    
        }
        ++linkSimplexIndex;
    }
    
    return ans;
}
    
}

#endif
