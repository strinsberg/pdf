#include "pdf_parser.h"
#include <fstream>
#include <iostream>
#include <vector>

// This is an informal test to be able to read a test pdf document and check
// what it looks like with the streams decompressed. The file is not included
// in the project and it is not a test to confirm the correctness of a function.

int main() {
  // Read from a test pdf file
  std::ifstream file(
      "/home/strinsberg/Documents/references-steven-deutekom.pdf",
      std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file");
  }

  file.seekg(0, std::ios::end);
  std::string contents(static_cast<std::string::size_type>(file.tellg()), ' ');
  file.seekg(0, std::ios::beg);
  file.read(&contents[0], contents.size());

  file.close();

  // Create a parser and printout the decompressed pdf
  PdfParser parser(contents);
  std::cout << parser.naive_inflate() << std::endl;

  return 0;
}
