#include "Utils/DummyParamHolder.hh"
#include "Utils/ByteStream.hh"

Utils::DummyParamHolder::DummyParamHolder()
 : ParametersBase()
{
}

Utils::DummyParamHolder::DummyParamHolder(const char*data)
  : ParametersBase()
{
  std::uint32_t npars;
  ByteStream::read(data,npars);
  std::string tmpstr;
  std::uint8_t tmptype;
  for (unsigned ipar=0;ipar<npars;++ipar) {
    ByteStream::read(data,tmpstr);
    ByteStream::read(data,tmptype);
    if (tmptype==0) {
      double val;
      ByteStream::read(data,val);
      addParameterDouble(tmpstr.c_str(),val);
    } else if (tmptype==1) {
      int32_t val;
      ByteStream::read(data,val);
      addParameterInt(tmpstr.c_str(),val);
    } else if (tmptype==2) {
      std::uint8_t val;
      ByteStream::read(data,val);
      addParameterBoolean(tmpstr.c_str(),val);
    } else if (tmptype==3) {
      std::string val;
      ByteStream::read(data,val);
      addParameterString(tmpstr.c_str(),val.c_str());
    } else {
      assert(false);
    }
  }
  lock();
}

Utils::DummyParamHolder::~DummyParamHolder()
{
}
