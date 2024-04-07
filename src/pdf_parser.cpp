#include "pdf_parser.h"
#include <iostream>
#include <string>
#include <zlib.h>

PdfParser::PdfParser(const std::string &d) : data(d) {}

// This appears to work on my test pdf that was built using lualatex
// I have written a utility function to do the decoding that I will make more
// robust and use to rewrite this function. I might even move this to the
// utility section as it is not really a parsing function. When parsing the
// more generic inflate function can be used to decompress a stream as needed
// from an input stream without needing to read the whole file into memory.
std::string PdfParser::naive_inflate() {
  size_t start = 0;
  size_t end = 0;
  std::string new_data = "";

  // Initialize a big buffer for holding output, really I should use a small
  // buffer and loop while decompressing and push the decompressed bytes into
  // the string in chunks.
  uint out_buf_size = 65536;
  auto out_buffer = (Bytef *)malloc(sizeof(Bytef *) * out_buf_size);

  while (start != std::string::npos && end != std::string::npos) {
    // find stream and copy the data between streams into the new string
    start = data.find("stream", end + 10);
    if (start == std::string::npos)
      break;

    new_data += data.substr(end, (start + 7) - end);

    // Find the end of the stream and cut out the data
    end = data.find("endstream", start);
    if (end == std::string::npos)
      break;
    std::string stream = data.substr(start + 7, end - (start + 7));

    // Initialize the decompression stream
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = stream.size();
    strm.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(stream.data()));
    strm.avail_out = (uInt)out_buf_size;
    strm.next_out = out_buffer;

    // Decompress
    inflateInit(&strm);
    inflate(&strm, Z_NO_FLUSH);
    inflateEnd(&strm);

    // Move the decompressed data into the new data
    // This is not the most efficient way to do this
    size_t out_size = out_buf_size - strm.avail_out;
    for (size_t i = 0; i < out_size; ++i) {
      new_data += out_buffer[i];
    }
  }

  new_data += data.substr(end, data.length() - end);

  free(out_buffer);
  return new_data;
}
