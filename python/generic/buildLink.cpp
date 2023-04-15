#include "../pybind11/pybind11.h"

#include "triangulation/generic/buildLink.h"

void addBuildLink(pybind11::module_&m) {
    m.def("buildLink", regina::buildLink<3, 0>);
    m.def("buildLink", regina::buildLink<4, 0>);
    m.def("buildLink", regina::buildLink<5, 0>);
    m.def("buildLink", regina::buildLink<6, 0>);
    m.def("buildLink", regina::buildLink<7, 0>);

    m.def("buildLink", regina::buildLink<4, 1>);
    m.def("buildLink", regina::buildLink<5, 1>);
    m.def("buildLink", regina::buildLink<6, 1>);
    m.def("buildLink", regina::buildLink<7, 1>);

    m.def("buildLink", regina::buildLink<5, 2>);
    m.def("buildLink", regina::buildLink<6, 2>);
    m.def("buildLink", regina::buildLink<7, 2>);

    m.def("buildLink", regina::buildLink<6, 3>);
    m.def("buildLink", regina::buildLink<7, 3>);

    m.def("buildLink", regina::buildLink<7, 4>);
}
