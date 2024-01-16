#include "SimpleHists/Hist1D.hh"
#include "SimpleHists/Hist2D.hh"
#include "SimpleHists/HistCollection.hh"
#include <cstdio>

int main(int,char**) {

  SimpleHists::Hist1D h("A title (0)",5,0.0,10.0);
  h.dump(true);//test empty dump

  h.setXLabel("some xlabel");
  h.setTitle("A title");
  h.setYLabel("some ylabel");
  h.setComment("some comment");
  h.fill(0.001);
  h.fill(0.001,0.5);
  h.fill(5.001,1.5);
  h.fill(15.001);//overflow
  h.fill(-15.001,0.333);//underflow

  h.dump(true);

  std::string buf;
  h.serialise(buf);

  printf("\nSerialised to buffer of length %i\n\n",(int)buf.size());

  printf("Deserialisation 1:\n\n");

  SimpleHists::Hist1D h2(buf);
  h2.dump(true);

  printf("\n");
  printf("Deserialisation 2:\n\n");

  SimpleHists::HistBase * h3 = SimpleHists::deserialise(buf);
  h3->dump(true);
  delete h3;

  //Test merging:

  printf("\nMerge test premerge:\n\n");

  SimpleHists::Hist1D hmerge1(buf);
  SimpleHists::Hist1D hmerge2(buf);

  hmerge1.fill(2.5,0.4);
  hmerge2.fill(8.8,4.3);
  hmerge1.dump(true);
  hmerge2.dump(true);

  printf("\nMerge test postmerge:\n\n");

  hmerge1.merge(&hmerge2);
  hmerge1.dump(true);

  // printf("\nMerge test reference:\n\n");

  // SimpleHists::Hist1D href("Reference",5,0.0,10.0);
  // for (unsigned i=0;i<2;++i) {
  //   href.fill(0.001);
  //   href.fill(0.001,0.5);
  //   href.fill(5.001,1.5);
  //   href.fill(15.001);//overflow
  //   href.fill(-15.001,0.333);//underflow
  //   if (i%2)
  //     href.fill(2.5,0.4);
  //   else
  //     href.fill(8.8,4.3);
  // }
  // href.dump(true);


  //Test hist collections:
  printf("\nTest HistCollection:\n\n");

  SimpleHists::HistCollection hc;
  auto h1d = hc.book1D(7,-5.0,5.0,"a_test_hist");
  h1d->setXLabel("A fine xlabel");
  h1d->fill(1.2);
  h1d->fill(-2.4,0.01);
  printf("\nSome histogram before persistification:\n\n");
  h1d->dump(true);
  hc.saveToFile("testhc",true);

  SimpleHists::HistCollection hc2("testhc");
  printf("\nSame histogram after persistification+loading:\n\n");
  hc2.hist("a_test_hist")->dump(true);

  printf("Test normalisation - original:\n");
  SimpleHists::Hist1D hnorm(buf);
  hnorm.dump(true,"  ");
  printf("Test normalisation:\n");
  hnorm.norm();
  hnorm.dump(true,"  ");

  //2D hists:

  SimpleHists::Hist2D h2d("A 2d hist",5,0.0,5.0,3,-2.0,2.0);
  h2d.dump(true);//test empty dump
  h2d.setXLabel("some xlabel");
  h2d.setYLabel("some ylabel");
  h2d.setComment("some comment");
  h2d.fill(0.5,0.2);
  h2d.fill(0.0,17.5,12.2);//overflow y
  h2d.dump(true);

  //HistCounts:

  SimpleHists::HistCounts * stats2(0);
  {
    SimpleHists::HistCounts stats("Stats");
    auto c1 = stats.addCounter("Counter 1");
    auto c2 = stats.addCounter("counter_2");
    c2.setComment("This is a comment about the second counter");
    auto c3 = stats.addCounter("Counter 3","<Counter 3>");
    stats.addCounter("An unused counter");
    auto c4 = stats.addCounter("A counter summing c1 and c2");
    c1 += 1;
    ++c1;
    c2 += 12.2;
    for (int i=0;i<100;++i)
      c3 += 1.0;

    c4 += c1;
    c4 += c2;

    stats.dump(true,"s1: ");
    stats2 = dynamic_cast<SimpleHists::HistCounts*>(stats.clone());
    ++c1;++c2;++c3;++c4;

  }
  stats2->dump(true,"s2: ");
  std::string statbuf;
  stats2->serialise(statbuf);
  printf("Serialised to %i bytes\n",(int)statbuf.size());

  auto hdeser = SimpleHists::deserialise(statbuf);
  hdeser->dump(true,"s3: ");
  printf("getMaxContent() = %g\n",stats2->getMaxContent());
  printf("Dump after setErrorsByContent:\n");
  stats2->setErrorsByContent();
  stats2->dump(true,"s4: ");
  delete stats2;


  return 0;
}
