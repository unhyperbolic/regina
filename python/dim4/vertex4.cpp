
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Python Interface                                                      *
 *                                                                        *
 *  Copyright (c) 1999-2022, Ben Burton                                   *
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

#include "../pybind11/pybind11.h"
#include "triangulation/dim3.h"
#include "triangulation/dim4.h"
#include "../helpers.h"
#include "../docstrings/triangulation/alias/face.h"
#include "../docstrings/triangulation/alias/facenumber.h"
#include "../docstrings/triangulation/dim4/vertex4.h"
#include "../docstrings/triangulation/detail/face.h"
#include "../docstrings/triangulation/detail/facenumbering.h"
#include "../docstrings/triangulation/generic/faceembedding.h"

using regina::Vertex;
using regina::VertexEmbedding;
using regina::Face;
using regina::FaceEmbedding;

void addVertex4(pybind11::module_& m) {
    RDOC_SCOPE_BEGIN(FaceEmbedding)
    RDOC_SCOPE_BASE_3(detail::FaceEmbeddingBase, alias::FaceNumber,
        alias::SimplexVoid)

    auto e = pybind11::class_<FaceEmbedding<4, 0>>(m, "FaceEmbedding4_0",
            rdoc_scope)
        .def(pybind11::init<regina::Pentachoron<4>*, regina::Perm<5>>(),
            rdoc::__init)
        .def(pybind11::init<const VertexEmbedding<4>&>(), rdoc::__copy)
        .def("simplex", &VertexEmbedding<4>::simplex,
            pybind11::return_value_policy::reference, rbase::simplex)
        .def("pentachoron", &VertexEmbedding<4>::pentachoron,
            pybind11::return_value_policy::reference, rbase3::pentachoron)
        .def("face", &VertexEmbedding<4>::face, rbase::face)
        .def("vertex", &VertexEmbedding<4>::vertex, rbase2::vertex)
        .def("vertices", &VertexEmbedding<4>::vertices, rbase::vertices)
    ;
    regina::python::add_output(e);
    regina::python::add_eq_operators(e, rbase::__eq, rbase::__ne);

    RDOC_SCOPE_SWITCH(Face)
    RDOC_SCOPE_BASE_2(detail::FaceBase, detail::FaceNumberingAPI)

    auto c = pybind11::class_<Face<4, 0>>(m, "Face4_0")
        .def("index", &Vertex<4>::index)
        .def("embedding", &Vertex<4>::embedding)
        .def("embeddings", &Vertex<4>::embeddings)
        .def("__iter__", [](const Vertex<4>& f) {
            // By default, make_iterator uses reference_internal.
            return pybind11::make_iterator<pybind11::return_value_policy::copy>(
                f.begin(), f.end());
        }, pybind11::keep_alive<0, 1>()) // iterator keeps Face alive
        .def("front", &Vertex<4>::front)
        .def("back", &Vertex<4>::back)
        .def("triangulation", &Vertex<4>::triangulation)
        .def("component", &Vertex<4>::component,
            pybind11::return_value_policy::reference)
        .def("boundaryComponent", &Vertex<4>::boundaryComponent,
            pybind11::return_value_policy::reference)
        .def("degree", &Vertex<4>::degree)
        .def("buildLink", [](const Vertex<4>& v) {
            // Return a clone of the resulting triangulation.
            // This is because Python cannot enforce the constness of
            // the reference that would normally be returned.
            return new regina::Triangulation<3>(v.buildLink());
        })
        .def("buildLinkInclusion", &Vertex<4>::buildLinkInclusion)
        .def("isLinkOrientable", &Vertex<4>::isLinkOrientable)
        .def("isValid", &Vertex<4>::isValid)
        .def("hasBadIdentification", &Vertex<4>::hasBadIdentification)
        .def("hasBadLink", &Vertex<4>::hasBadLink)
        .def("isIdeal", &Vertex<4>::isIdeal)
        .def("isBoundary", &Vertex<4>::isBoundary)
        .def("linkingSurface", &Vertex<4>::linkingSurface)
        .def_static("ordering", &Vertex<4>::ordering)
        .def_static("faceNumber", &Vertex<4>::faceNumber)
        .def_static("containsVertex", &Vertex<4>::containsVertex)
        .def_readonly_static("nFaces", &Vertex<4>::nFaces)
        .def_readonly_static("lexNumbering", &Vertex<4>::lexNumbering)
        .def_readonly_static("oppositeDim", &Vertex<4>::oppositeDim)
        .def_readonly_static("dimension", &Vertex<4>::dimension)
        .def_readonly_static("subdimension", &Vertex<4>::subdimension)
    ;
    regina::python::add_output(c);
    regina::python::add_eq_operators(c);

    RDOC_SCOPE_END

    regina::python::addListView<
        decltype(std::declval<Vertex<4>>().embeddings())>(m);

    m.attr("VertexEmbedding4") = m.attr("FaceEmbedding4_0");
    m.attr("Vertex4") = m.attr("Face4_0");
}

