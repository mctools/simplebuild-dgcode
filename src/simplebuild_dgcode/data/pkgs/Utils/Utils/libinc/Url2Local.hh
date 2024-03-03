#ifndef Utils_Url2Local_hh
#define Utils_Url2Local_hh

#include <string>

namespace Utils {

  //If input_filename is a local path (or an invalid filename), it will be
  //returned unaltered. If it is a url, the file at the url will be downloaded
  //to a local cache and the path to the local file returned. On next invocation
  //the previously downloaded file will be reused, saving a reload:

  std::string url2local(std::string input_filename, std::string cachedir="");
}

#endif
