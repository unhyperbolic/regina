
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Python Interface                                                      *
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

#include "../pybind11/pybind11.h"
#include "packet/attachment.h"
#include "../helpers.h"

using pybind11::overload_cast;
using regina::Attachment;

void addAttachment(pybind11::module_& m) {
    pybind11::class_<Attachment, regina::Packet, std::shared_ptr<Attachment>>(
            m, "Attachment")
        .def(pybind11::init<>())
        .def(pybind11::init<const char*>())
        .def(pybind11::init<const Attachment&>())
        .def("swap", &Attachment::swap)
        .def("isNull", &Attachment::isNull)
        .def("size", &Attachment::size)
        .def("filename", &Attachment::filename)
        .def("extension", &Attachment::extension)
        .def("reset", overload_cast<>(&Attachment::reset))
        .def("save", &Attachment::save)
        .def("savePDF", &Attachment::save) // deprecated
        .def_property_readonly_static("typeID", [](pybind11::object) {
            // We cannot take the address of typeID, so use a getter function.
            return Attachment::typeID;
        })
    ;

    m.def("swap", (void(*)(Attachment&, Attachment&))(regina::swap));

    m.attr("PDF") = m.attr("Attachment"); // deprecated typedef
}
