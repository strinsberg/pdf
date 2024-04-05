#include "pdf_parser.h"
#include <zlib.h>
#include <string>

PdfParser::PdfParser(const std::string &d) : data(d) {}

std::string PdfParser::extract_text() {
  return "not implemented";
}
