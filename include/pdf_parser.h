#ifndef PDF_PARSER_H
#define PDF_PARSER_H

#include <string>
#include <vector>

/* I want to experiment with PDFs, they seem pretty cool underneath. This class
 * will probably change a lot as I experiement, but it can hold data and some
 * methods. For now it just has a naive function for printing out a pdf with
 * all of the streams decompressed. Currently it just reads uses one big
 * buffer to hopefully hold all of the text of the full pdf from the string.
 * However, I wrote a function that can decompress from a stream and that uses
 * proper buffers to read and decompress in chunks. It should be more robuts,
 * though it needs some error checking on the inflate calls. It could also be
 * used to decompress a stream on the fly from a file stream without having to
 * read the whole PDF into memory. I think I will try to setup the parser to
 * work with an input stream instead of a string.
 */
class PdfParser {
public:
  // Initialize the parser with a reference to the string of bytes.
  PdfParser(const std::string &data);

  // A simple way to decode the streams in a pdf that uses deflate compression.
  // For now it is a way to allow me to better inspect an actual pdf in full
  // without the compression, but before I build the full parser.
  std::string naive_inflate();

protected:
  const std::string &data;
};

#endif
