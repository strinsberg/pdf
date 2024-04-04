#include <string>

class PdfObject {
public:
  PdfObject();
  virtual std::string write() = 0;
};
