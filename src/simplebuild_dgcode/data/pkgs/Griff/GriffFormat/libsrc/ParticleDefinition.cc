#include <cassert>
#include "GriffFormat/ParticleDefinition.hh"
#include "EvtFile/FileWriter.hh"
#include "Utils/ByteStream.hh"

GriffFormat::ParticleDefinition::ParticleDefinition(const char*& data)
{
#ifndef NDEBUG
  const char* datain=data;
#endif
  ByteStream::read<ParticleDefinition>(data,*this);//we checked no padding!
  assert(data-datain == sizeof(*this));
}

void GriffFormat::ParticleDefinition::write(EvtFile::FileWriter&fw)
{
  fw.writeDataDBSection((char*)this,sizeof(*this));//we checked no padding!
}

void GriffFormat::ParticleDefinition::writeRaw(char* data)
{
  std::memcpy(data,(void*)this,sizeof(*this));
}

