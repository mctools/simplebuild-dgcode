#include "ZLibUtils/Compress.hh"
#include <string>
#include <iostream>
#include "Core/Types.hh"

// Note: H.C.Andersen is no longer copyright!! :-)


static const std::string data_orig = "Det sydede og bruste, mens Ilden flammede under Gryden, det var Taellelysets Vugge – og ud af den lune Vugge gled Lyset for[m]fuldendt, helstoebt, skinnende hvidt og slankt det var dannet paa en Maade, som fik Alle, der saae det til at troe at det maatte give Loevte om en lys og straalende Fremtid – og Loevterne, som Alle saae, skulde det virkelig holde og opfylde."
"Faaret – et nydeligt lille Faar – var Lysets Moder og Smeltegryden var dets Fader. Fra dets Moder havde det arvet sin blendende hvide Krop og en Ahnelse om Livet; men fra / dets Fader havde det faaet Lysten til den flammende Ild, der engang skulde gaae det igjennem Marv og Been – og ”lyse” for det i Livet."
""
"Ja saadan var det skabt og udviklet, da det med de bedste, de lyseste Forhaabninger kastede sig ud i Livet. Der traf det saa underlig mange Medskabninger som det indlod sig med; thi det vilde laere Livet at kjende – og maaskee derved finde den Plads, hvor det selv passede bedst."
""
"Men det troede altfor godt"
"om Verden; den broed sig kun om sig selv og slet ikke om Taellelyset; thi den kunde ikke forstaae, til hvad Gavn det kunde vaere, og derfor soegte den saa at bruge det til Fordeel for sig selv og toge forkeert fat paa Lyset, de sorte Fingre satte stoerre og stoerre Pletter paa den reene Uskyldsfarve; denne svandt efterhaanden ganske bort og blev heelt tildaekket af Smuds / fra Omverd[e]nen, der var kommet i altfor"
"svaer Beroering med det, meget naermere end Lyset kunde taale, da det ikke havde kundet skjelne Reent fra Ureent, – men endnu var det i sit Inderste uskyldig og ufordaervet."
""
"Da saae de falske Venner, at de ikke kunde naae det Indre – og vrede kastede de Lyset bort som en unyttig Tingest."
""
"Men de[n] ydre sorte Skal holdt alle de Gode borte, – de vare bange for at smittes af den sorte Farve, for at faae Pletter paa sig, – og saa holdt de sig borte."
""
"Nu stod det stakkels Taellelys saa ene og forladt, det vidste hverken ud eller ind. Det saae sig forstoedt af det Gode og det opdagede nu, at det kun havde vaeret et Redskab til at fremme det slette, det foelte sig da saa uendelig ulyksalig, fordi det havde tilbragt dets Liv til ingen Nytte, ja det havde maaskee endogsaa svaertet det Bedre i sin Omgang –, det kunde ikke fatte, hvorfor eller hvortil det egentlig / var skabt, hvorfor det skulde leve paa Jorden – og maaskee oedelaegge sig selv og andre."
""
"Meer og meer, dybere og dybere grublede det, men jo meere det taenkte, desto stoerre blev dets Mismod, da det slet ikke kunde finde noget Godt, noget virkeligt Indhold for sig selv – eller see det Maal, som det havde faaet ved dets Foedsel. – Det var ligesom det sorte Daekke ogsaa havde tilsloeret dets OEine."
""
"Men da traf det en lille Flamme, et Fyrtoei; det kjendte Lyset bedre, end Taellelyset kjendte sig selv; thi Fyrtoeiet saae saa klart – tvaers igjennem den ydre Skal – og der inden for fandt det saa meget Godt; derfor naermede det sig til det, og lyse Formodninger vaktes hos Lyset; det antaendtes og Hjertet smaeltede i det."
""
"Flammen straalede ud – som Formaelingens Glaedesfakkel, Alt blev lyst og klart rundt omkring, og det oplyste Veien for dets Omgivelser, dets sande Venner – og med Held soegte de nu Sandheden under Lysets Skue."
""
"Men ogsaa Legemet var kraftigt nok / til at naere og baere den flammende Ild."
""
"– Draabe paa Draabe som Spirer til nyt Liv trillede runde og buttede ned ad stammen og daekkede med deres Legemer – Fortidens Smuds."
""
"De vare ikke blot Formaelingens legemlige men ogsaa deres [a]andelige Udbytte. Og Taellelyset havde fundet dets rette Plads i Livet – og viist, at det var et rigtigt Lys, som lyste laenge til Glaede for sig selv og dets Medskabninger –"
""
"H.C. Andersen.";

int main(int,char**) {
  Utils::DynBuffer<char> data_zipped;
  unsigned zippeddataLength;
  ZLibUtils::compressToBuffer(data_orig.c_str(), data_orig.size()+1, data_zipped,zippeddataLength);
  std::cout<<"Compressed data from "<<data_orig.size()+1<<" B to "<<zippeddataLength<<" B ("<<zippeddataLength*100.0/(1+data_orig.size())<<" %%)\n"<<std::endl;

  //Unzip:
  Utils::DynBuffer<char> data_unzipped;
  ZLibUtils::decompressToBufferNew(data_zipped.data(), zippeddataLength, data_unzipped);
  unsigned unzippeddataLength = static_cast<unsigned>(data_unzipped.size());
  if (data_orig.size()+1!=unzippeddataLength) {
    printf("ERROR: uncompressed data has different size than original data\n");
    return 1;
  }
  std::string data_uncomp_str;
  data_uncomp_str.assign(&(data_unzipped[0]),unzippeddataLength-1);
  if (data_uncomp_str!=data_orig) {
    printf("ERROR: uncompressed data different than original data\n");
    return 1;
  }

  ZLibUtils::compressToBuffer("", 0, data_zipped,zippeddataLength);
  if (zippeddataLength!=sizeof(std::uint32_t)||*(reinterpret_cast<std::uint32_t*>(&data_zipped[0]))!=0) {
    printf("ERROR: compressing empty buffer yields unexpected result %i %i\n",zippeddataLength,*(reinterpret_cast<std::uint32_t*>(&data_zipped[0])));
    return 1;
  }

  const std::uint32_t nullbuffer = 0;
  ZLibUtils::decompressToBufferNew(reinterpret_cast<const char*>(&nullbuffer),sizeof(nullbuffer),data_unzipped);
  if (data_unzipped.size()!=0) {
    printf("ERROR: uncompressing null buffer yields unexpected result\n");
    return 1;
  }
}
