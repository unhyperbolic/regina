
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

/*! \file python/helpers/listview.h
 *  \brief Assists with wrapping instances of Regina's lightweight ListView
 *  template class.
 */

#include "utilities/listview.h"
#include "../docstrings/utilities/listview.h"
#include <cxxabi.h>

namespace regina::python {

namespace detail {
    /**
     * Casts the given C++ object to Python and writes its __repr__ to
     * the given C++ output stream.
     *
     * For the moment this should only be used with objects where we
     * know in advance that this process will succeed.
     */
    template <typename T>
    void writeRepr(std::ostream& out, const T& obj) {
        out << static_cast<std::string>(pybind11::str(
            pybind11::cast(obj).attr("__repr__")()));
    }
}

/**
 * Adds Python bindings for one of Regina's ListView classes, given the
 * class type.
 *
 * The type \a T must be of the form regina::ListView<...>, and must be
 * explicitly specified in the template arguments for addListView().
 *
 * Typically you would not write out the full ListView type explicitly (since
 * this is an implementation detail that may change in future versions of
 * Regina); instead you would access the type using an appropriate decltype()
 * statement.
 *
 * The Python class corresponding to \a T will not be given a unique name;
 * instead all such types will be called \c ListView, and will be put into
 * their own unique namespaces to avoid clashes.
 *
 * Return value policies work as follows:
 *
 * - When accessing a list element (e.g., through front() or back(), or
 *   via iteration or indexing), the relationship between the list element
 *   and the ListView will be defined by the template argument \a Policy.
 *   By default, list elements that are pointers (e.g., when calling
 *   Triangulation::vertices()) will use a \c reference_internal policy,
 *   and thus keep the ListView alive; list elements that are values
 *   (e.g., when calling Vertex::embeddings()) will use a \c copy policy,
 *   and thus will have independent lifespans.
 *
 * - If you need the list elements to keep the underlying container alive,
 *   you will need to indicate this when wrapping whatever function returns a
 *   ListView.  Since ListView objects are passed by value, you cannot
 *   use \c reference_internal; instead you should pass an appropriate
 *   \c keep_alive argument.  For example, Triangulation::vertices()
 *   would use pybind11::keep_alive<0,1>().  Note that this assumes that the
 *   list elements are pointers (or if not, the \a Policy argument has been set
 *   appropriately): thus the list elements keep the ListView alive, and
 *   in turn the ListView keeps the container alive.
 */
template <class T, pybind11::return_value_policy Policy =
    (std::is_pointer<typename T::value_type>::value ?
        pybind11::return_value_policy::reference_internal :
        pybind11::return_value_policy::copy)>
void addListView(pybind11::module_& m) {
    // Instead of naming these classes uniquely, just call them all ListView
    // and make them all local to their own unique Python namespaces.
    // End users should not be constructing them anyway.
    auto c = pybind11::class_<T>(pybind11::handle(), "ListView",
            pybind11::module_local(), doc::ListView)
        .def(pybind11::init<const T&>(), doc::ListView_::__init)
        .def("__iter__", [](const T& view) {
            return pybind11::make_iterator<Policy>(view.begin(), view.end());
        }, pybind11::keep_alive<0, 1>(), // iterator keeps ListView alive
            doc::ListView_::__iter__)
        .def("__getitem__", [](const T& view, size_t index) {
            if (index >= view.size())
                throw pybind11::index_error("ListView index out of range");
            return view[index];
        }, Policy, doc::ListView_::__array)
        .def("empty", &T::empty, doc::ListView_::empty)
        .def("size", &T::size, doc::ListView_::size)
        .def("__len__", &T::size, doc::ListView_::size)
        .def("front", [](const T& view) {
            if (view.empty())
                throw pybind11::index_error("List is empty");
            return view.front();
        }, Policy, doc::ListView_::front)
        .def("back", [](const T& view) {
            if (view.empty())
                throw pybind11::index_error("List is empty");
            return view.back();
        }, Policy, doc::ListView_::back)
        ;
    regina::python::add_output_custom(c, [](const T& view, std::ostream& out) {
        out << "[ ";
        // For very small lists, output the entire list.
        // For larger lists, do not output everything.
        if (view.empty()) {
            out << "[ ]";
        } else if (view.size() <= 5) {
            bool started = false;
            for (const auto& i : view) {
                if (started)
                    out << ", ";
                else
                    started = true;
                detail::writeRepr(out, i);
            }
            out << ' ';
        } else {
            for (int i = 0; i < 3; ++i) {
                detail::writeRepr(out, view[i]);
                out << ", ";
            }
            out << "..., ";
            detail::writeRepr(out, view.back());
            out << ' ';
        }
        out << ']';
    });
    regina::python::add_eq_operators(c,
        doc::ListView_::__eq, doc::ListView_::__ne);
}

/**
 * Adds Python bindings for one of Regina's ListView classes, given an
 * object of that type.
 *
 * The type \a T must be of the form regina::ListView<T>.  Typically
 * this type would be deduced automatically, and you would not need to
 * supply any template arguments with this function call.
 *
 * This routine has the effect of (i) calling addListView<T>(m) to wrap the
 * C++ class corresponding to \a view, and then (ii) returning \a view itself.
 * This is simply a convenience that allows you to wrap a ListView class
 * and set a class attribute or global constant, all in a single function call.
 *
 * The default return value policies supplied by addListView() will be used,
 * and it is not possible to override them here.  See addListView() for
 * further details.
 */
template <typename T>
regina::ListView<T> wrapListView(pybind11::module_& m,
        regina::ListView<T> view) {
    addListView<regina::ListView<T>>(m);
    // Remember: ListView is lightweight and cheap to pass by value.
    return view;
}

/**
 * Adds Python bindings for one of Regina's ListView classes, and wraps the
 * given fixed-size C-style array in such a ListView.
 *
 * The given array should be a C-style array whose elements are of type \a T,
 * and whose size \a n is known at compile-time.  Typically both \a T and \a n
 * would be deduced automatically, and you would not need to supply any
 * template arguments with this function call.
 *
 * This routine has the effect of (i) creating a ListView of the appropriate
 * type to wrap \a array; (ii) calling addListView<T>(m) to wrap this
 * ListView class in Python; and (iii) returning this ListView object
 * so that it can be set as a class attribute or global constant.
 *
 * The default return value policies supplied by addListView() will be used,
 * and it is not possible to override them here.  See addListView() for
 * further details.
 */
template <typename T, int n>
regina::ListView<T[n]> wrapListView(pybind11::module_& m, const T (&array)[n]) {
    addListView<regina::ListView<T[n]>>(m);
    // Remember: ListView is lightweight and cheap to pass by value.
    return regina::ListView(array);
}

} // namespace regina::python

