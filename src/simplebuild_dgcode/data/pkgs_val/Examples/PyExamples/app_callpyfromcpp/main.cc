#include "Core/Python.hh"
#include <cstdio>
#include <pybind11/embed.h> //for scoped_interpreter

int main(int,char**) {

  //Task: Call the python function "some_function" which is defined in the
  //PyExamples.Example python module.

  //First we need to initialise python, since we are not inside a python module
  //here:

  py::scoped_interpreter guard{};

  //Alternatively, we could have done this interpreter initialisation with a
  //call to "pyextra::ensurePyInit();", but that is mostly intended for library
  //code which does not know if the interpreter is already initialised

  //Then import the desired module:
  py::object mod = pyextra::pyimport("PyExamples.Example");

  //And grab the desired function:
  py::object some_function = mod.attr("some_function");

  //And call it (converting the returned py::object to a C++ double):
  double res = some_function(2.2,-0.4).cast<double>();

  //Print the result:
  printf("Result of calling PyExamples.Example.some_function(2.2,-0.4): %g\n",res);

  return 0;
}
