#include <pybind11/pybind11.h>

#include <cstdio>
#include <stdexcept>
#include <string>

/* Forward declarations to avoid type conflicts in the headers. */
extern "C" {
typedef struct __rc_data rc_data;
struct __rc_data {
    char * param;
    char * value;
    rc_data * next;
};
rc_data * rc_read(char * file_name, FILE * err_file);
void rc_clear(rc_data * data);
}

pybind11::dict parse_input_file(pybind11::object path) {
    pybind11::module_ os = pybind11::module_::import("os");
    pybind11::module_ sys = pybind11::module_::import("sys");

    std::string file = pybind11::str(path).cast<std::string>();
    if (!static_cast<bool>(os.attr("path").attr("exists")(path))) {
        throw std::runtime_error("Path '" + file + "' does not exist");
    }

    FILE * err = tmpfile();
    if (err == NULL) {
        throw std::runtime_error("Error creating error stream");
    }
    char * file_ = const_cast<char *>(file.c_str());
    rc_data * data = rc_read(file_, err);

    /* Report errors */
    static int SIZE = 80;
    char buffer[80];
    long int eof = ftell(err);
    fseek(err, 0, SEEK_SET);

    pybind11::list message;
    pybind11::str err_msg = "";
    while (feof(err) != 0) {
        int to_end = eof - ftell(err);
        int size = SIZE > to_end ? to_end : SIZE;
        if (fgets(buffer, size, err) == NULL) {
            message.append(pybind11::str(buffer));
        } else {
            err_msg = "Error occurred reading parse errors";
            break;
        }
    }
    fclose(err);
    if (pybind11::len(err_msg) != 0) {
        throw std::runtime_error(err_msg.cast<std::string>());
    }
    sys.attr("stderr").attr("write")(pybind11::str("").attr("join")(message));

    pybind11::dict result;
    rc_data * walker = data;
    while (walker != NULL && walker->param != NULL) {
        pybind11::str param = walker->param;
        pybind11::str value = walker->value == NULL ? "false" : walker->value;
        result[param] = value;
        walker = walker->next;
    }
    rc_clear(data);

    return result;
}


PYBIND11_MODULE(_cloudgen, m) {
    m.doc() = "Cloudgen C Bindings";

    m.def("parse_input_file",
          &parse_input_file,
          "Parse the configuration files"
          );
}
