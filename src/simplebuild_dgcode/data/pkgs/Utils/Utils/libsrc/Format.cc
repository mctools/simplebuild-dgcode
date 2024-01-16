#include "Utils/Format.hh"

//based on http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
void Utils::string_format(std::string& output, const std::string fmt, ...) {
  //NB: fmt is not passed by reference since 'va_start' has undefined behavior with reference types

  std::string tmpoutput;//in case output is used somewhere in the input as well (to reformat a string)
  //output.clear();
  int size=fmt.size()+128;
  va_list ap;
  while (true) {
    tmpoutput.resize(size);
    va_start(ap, fmt);
    int n = vsnprintf((char *)tmpoutput.c_str(), size, fmt.c_str(), ap);
    va_end(ap);
    if (n > -1 && n < size) {
      //success, used n<size chars
      output = tmpoutput;
      output.resize(n);
      return;
    }
    if (n > -1)
      size=n+1;//failure, but told us number of chars needed (+ null char)
    else
      size*=2;//failure, try size doubling.
  }
}

//NB, comment on web: va_start with a reference argument has problems on
//MSVC. It fails silently and returns pointers to random memory. As a
//workaround, use std::string fmt instead of std::string &fmt, or write a
//wrapper object. – Steve Hanov Aug 27 at 17:53
