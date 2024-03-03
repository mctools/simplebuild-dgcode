#include "Utils/Url2Local.hh"
#include "Utils/Cmd.hh"
#include "Core/String.hh"
#include <stdexcept>
#include <vector>
#include <iostream>

std::string Utils::url2local(std::string input_filename, std::string cachedir)
{
  //Wrap sb_utils_url2local command, except for local files:
  if ( input_filename.find("://") == std::string::npos )
    return input_filename;//not an url
  std::string cmd("sb_utils_url2local ");
  cmd += '"';
  cmd += input_filename;
  cmd += "\" \"";
  cmd += cachedir;
  cmd += '"';
  auto res = Utils::launch_cmd( cmd.c_str() );
  if ( res.first != 0 )
    throw std::runtime_error(std::string("cmd failed: ")+cmd);
  //Only return the last line of the output, and re-print the rest:
  std::vector<std::string> parts;
  Core::split_noempty(parts,res.second,"\n\r");
  if ( parts.empty() )
    throw std::runtime_error(std::string("cmd gave unexpected output: ")+cmd);
  auto itBack = std::prev(parts.end());
  for ( auto it = parts.begin(); it != itBack; ++it )
    std::cout<<*it<<std::endl;
  return std::move(*itBack);
}
