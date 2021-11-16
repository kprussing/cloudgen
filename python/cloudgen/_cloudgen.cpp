#include <pybind11/pybind11.h>

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(_cloudgen, m) {
    m.doc() = "Cloudgen C Bindings";

    m.def("add", &add);
}
