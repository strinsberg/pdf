#ifndef PDF_UTILITY_H
#define PDF_UTILITY_H

#include <ios>
#include <istream>
#include <vector>

namespace util {

// return the size in bytes of the stream from the current position.
// restores the current position in the stream
size_t bytes_till_end(std::istream &is);

// read a file as bytes into a character vector
std::vector<char> slurp_bytes(const std::string &filename);

// decompress a given number of bytes in an input stream
// restores the current position in the stream
std::vector<char> inflate_stream(std::istream &is, std::streamoff);

} // namespace util

#endif
