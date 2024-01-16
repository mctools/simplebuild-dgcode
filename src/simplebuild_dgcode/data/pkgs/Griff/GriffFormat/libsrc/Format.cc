#include "GriffFormat/Format.hh"

const GriffFormat::Format * GriffFormat::Format::getFormat()
{
  static const Format theFormat;
  return &theFormat;
}
