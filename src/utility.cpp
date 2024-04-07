#include "utility.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <zlib.h>

const size_t BUFF_SIZE = 1024;

size_t util::bytes_till_end(std::istream &is) {
  // Save current position
  std::streamsize pos = is.tellg();

  // Seek to end and get size
  is.seekg(0, std::ios_base::end);
  std::streamsize size = is.tellg() - pos;

  // Return to original position
  is.seekg(pos, std::ios_base::beg);

  return size;
}

// TODO make sure this is as robust as it can be and write tests to check on
// any errors, where possible.
std::vector<char> util::slurp_bytes(const std::string &filename) {
  // Open the file
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open())
    throw std::ios_base::failure("Failed to open file: " + filename);

  // Get the size and initialize the vector
  size_t size = util::bytes_till_end(file);
  std::vector<char> contents(size);

  // Read in the contents
  file.read(reinterpret_cast<char *>(contents.data()), size);
  if (file.gcount() != size)
    throw std::ios_base::failure("Error reading file: " + filename);

  // Close file and return contents
  file.close();
  return contents;
}

// TODO try to make this as robust as possible. It passes a test on a file
// bigger than the buffer size, but I could add tests for bad data etc. and
// ensure that all of my bounds etc. cannot fail.
std::vector<char> util::inflate_stream(std::istream &is, std::streamoff size) {
  std::vector<char> contents;

  // save the position in the stream
  std::streampos initial = is.tellg();
  std::streampos end = initial + size;
  std::streampos current = initial;

  // Create buffers and how much it holds
  char in_buffer[BUFF_SIZE];
  char out_buffer[BUFF_SIZE];

  // Initialize the decompression stream
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  // TODO check return for errors
  inflateInit(&strm);

  // Process the file stream in chunks until we reach the desired offset
  while (current < end) {
    // Find the new position in the stream and read chars into the input buffer
    current = is.tellg();
    size_t chars_to_read =
        end - current < BUFF_SIZE ? end - current : BUFF_SIZE;
    is.read(in_buffer, chars_to_read);
    size_t chars_read = is.tellg() - current;
    if (chars_read == 0) {
      break;
    }

    strm.next_in = reinterpret_cast<Bytef *>(in_buffer);
    strm.avail_in = chars_read;

    // Loop to decode the contents, we need to loop again because the
    // decompressed contents will be larger than the input.
    do {
      strm.next_out = reinterpret_cast<Bytef *>(out_buffer);
      strm.avail_out = BUFF_SIZE;

      // TODO Check return for errors
      int ret = inflate(&strm, Z_NO_FLUSH);

      size_t chars_decoded = BUFF_SIZE - strm.avail_out;
      contents.insert(contents.end(), out_buffer, out_buffer + chars_decoded);
    } while (strm.avail_out == 0);
  }

  // Clean up
  inflateEnd(&strm);
  is.seekg(initial);

  return contents;
}

// Prototypes /////////////////////////////////////////////////////////////////

/* There are several types of pdfobjects to parse
 *
 * 1. a top level object that has a object number, generation number, and is
 * wrapped in obj endobj. This may not really count as an object, but since it
 * has some data associated with it other than the object inside it seems to be
 * best represented on it's own, or many objects would have empty values for
 * that data.
 *
 * 2. Strings wrapped in () or hex strings wrapped in <>
 * 3. Numbers as integer or real
 * 4. Boolean true or false
 * 5. Indirect references <num> <gen> R
 * 6. Arrays wrapped in [] that can contain any of 2-7
 * 7. Dictionaries wrapped in << >> with name keys and object values
 * 8. Names which are /SomeName, / by itself can be a valid name
 * 9. Streams. These start with a dictionary and are preceded by strea/endstream
 *
 * To tell the difference between a dict and a stream you have to parse the
 * dict first and then see if it has a stream keyword after it.
 *
 */

util::PdfTopLevel util::parse_top_level_obj(std::istream &is) {
  return PdfTopLevel(0, 0, new PdfNull());
}

util::PdfObj* util::parse_pdf_obj(std::istream &is) {
  return new PdfNull();
}
