#include "Utils/Glob.hh"
#include <algorithm>
#include <cassert>

void Utils::glob(const std::string& pattern,std::vector<std::string>& res){
  glob_t glob_result;
  glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
  if (glob_result.gl_pathc>0) {
    for(unsigned int i=0;i<glob_result.gl_pathc;++i)
      res.push_back(std::string(glob_result.gl_pathv[i]));
    std::sort(res.end()-glob_result.gl_pathc,res.end());//for reproducibility we sort the newly added entries
  }
  globfree(&glob_result);
}
