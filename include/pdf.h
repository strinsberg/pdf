#include "pdf_object.h"
#include <vector>

class Pdf {
public:
  Pdf() : version("2.0"), objects(std::vector<PdfObject*>()) {}
  // add a new object, and ensure it ends up in the right place in the obj list
  //void add_object(PdfObject* obj) {}
  // generate the pdf file code
  std::string write() { return ""; }
  // build a pdf from a string/bytes
  //Pdf read(std::string data) { return Pdf(); }

protected:
  std::string version;
  // traversing the graph would be easy if objects are at the index of their
  // number, but will require some work to ensure this happens when adding
  // obects that may not be in order.
  std::vector<PdfObject*> objects;
};

