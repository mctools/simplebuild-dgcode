#include "Utils/ProgressiveHash.hh"
#include <string>
#include <cstdio>
#include <cassert>

int main(int,char**)
{
  //Some data (note: Edgar Allan Poe is no longer under copyright).
  std::string poem =
    "Once upon a midnight dreary, while I pondered weak and weary,"
    "Over many a quaint and curious volume of forgotten lore,"
    "While I nodded, nearly napping, suddenly there came a tapping,"
    "As of some one gently rapping, rapping at my chamber door."
    "`'Tis some visitor,' I muttered, `tapping at my chamber door -"
    "Only this, and nothing more.'";
  const char * data = &(poem[0]);
  const unsigned datalength = poem.size()+1;//include null char
  assert(datalength>200);

  ProgressiveHash ph;
  ph.addData(data,datalength);

  const ProgressiveHash::hashtype hash = ph.getHash();

  printf("Calculated hash of test string: %x\n",hash);

  if (hash!=0x8b18353) {
    //this would make it hard to validate old data files:
    printf("error: hash function changed! This is not a good idea!\n");
    return 1;
  }

  //Try again after a reset:
  ph.reset();
  ph.addData(data,datalength);
  if (hash!=ph.getHash()) {
    printf("error: different hash after a reset()!\n");
    return 1;
  }

  //Finally double-check that when adding the data in multiple steps, it doesn't
  //matter what the steps are as long as all the data is eventually fed through:
  for (unsigned n1=1;n1<100;++n1) {
    for (unsigned n2=1;n2<100;++n2) {
      ph.reset();
      ph.addData(&(data[0]),n1);
      ph.addData(&(data[n1]),n2);
      ph.addData(&(data[n1+n2]),datalength-n1-n2);
      if (hash!=ph.getHash()) {
        printf("error: different hash when calculating in multiple steps!\n");
        return 1;
      }
    }
  }
  printf("All tests succeeded!\n");
  return 0;
}
