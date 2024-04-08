#include "utility.h"
#include <cctype>
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

std::ostream &util::operator<<(std::ostream &os, const util::PdfObj &obj) {
  obj.write(os);
  return os;
}

std::ostream &util::operator<<(std::ostream &os,
                               const util::PdfObj *const obj) {
  obj->write(os);
  return os;
}

util::PdfTopLevel util::parse_top_level_obj(std::istream &is) {
  return PdfTopLevel(0, 0, new PdfNull());
}

// For now I am going to put all logic for parsing pdf objects into this
// function in a proper parser some of the decisions about what what objects to
// parse from the stream might be context dependent. E.g. if we are parsing the
// document at the top level a digit should represent the start of a top level
// object and we could simply look for the numbers and obj keyword. If we didn't
// find them then it would be an error. However, in other contexts we need to
// parse any object, like in an array, so we need to be able to call a single
// function and get an object back. The only really difficult things to parse
// will be numbers, object references, and top level style objects insized
// decoded streams as they all start with the same tokens but can contain
// multiple tokens. So we could parse an int and then have to decide if the next
// 3 objects could make up a coposite object probably move the stream back
// before returning if the first object is all we need.

// NOTE while parsing we do not need a function to skip whitespace, unless we
// want special behaviour. is >> in will skip whitespace and is.get(in) will
// grab whitespace. So anywhere where whitespace ends a token or can be
// included in a token, like strings and names, you have to use get to ensure
// you can accurately test for whitespace.

util::PdfObj *util::parse_pdf_obj(std::istream &is) {
  // what are the starting chars for many types
  // < starts << or is followed by a hex digit for <F4...>
  // ( starts a string
  // / starts a name
  // t and f start bools
  // [ starts an array
  // a digit starts a number, a reference, or a top level object, this could
  // be dependent on context.
  // - starts a negative number
  char in;
  std::string token;
  size_t len;

  is >> in;
  token.push_back(in);

  switch (in) {
  case 'n': // should start a null object
    for (size_t i = 0; i < 3; ++i) {
      is >> in;
      token += in;
    }
    if (token == "null") {
      return new PdfNull();
    } else {
      throw std::runtime_error(std::string("Parse Error: Invalid PDF null: ") +
                               token);
    }
  case 't': // should start bool
  case 'f':
    len = in == 't' ? 3 : 4;
    for (size_t i = 0; i < len; ++i) {
      is >> in;
      token += in;
    }
    if (token == "true") {
      return new PdfBool(true);
    } else if (token == "false") {
      return new PdfBool(false);
    } else {
      throw std::runtime_error(std::string("Parse Error: Invalid PDF bool: ") +
                               token);
    }
  case '(': // should start a string
    // TODO handle escape characters \n, \t, \r, \b, \f, \ddd (octal digits?)
    // TODO paraenthesis in a string do not need to be escaped if they
    //      are balanced, which we can do with a stack. But of course if one
    //      is escaped it does not go on the stack.
    // TODO a \ needs to be escaped also
    token.pop_back(); // remove (
    while (true) {
      is.get(in);
      if (in == ')' && !token.empty() && token.back() != '\\')
        break;
      token += in;
    }
    return new PdfString(token);
  case '/': // should start a name
    return new PdfName(get_name_token(is));
  case '<': // should start a dict (we will add hex strings later)
    is.get(in);
    if (in == '<') {
      is.unget();
      is.unget();
      PdfDict *dict = parse_pdf_dict(is);
      is >> in;
      if (in == 's') {
        is.unget();
        std::vector<char> stream = parse_pdf_content_stream(is);
        PdfStream *obj = new PdfStream(dict);
        obj->stream = stream;
        return obj;
      }
      return dict;
    } else if (isxdigit(in)) {
      throw std::runtime_error(
          std::string("Parse Error: No support for hex strings yet."));
    } else {
      throw std::runtime_error(
          std::string("Parse Error: Expected < or hex digit after <, Got ") +
          in);
    }
  case '[': // should start an array
    is.unget();
    return parse_pdf_array(is);
    break;
  case '-': // should start a number -/+123 -/+123.456 .90 etc.
  case '+':
  case '.':
    break;
  case 0: // Digits start numbers, refs, or obj/endobj
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 9:
    break;
  default: // if we see anything else it is an error in the doc or the parser
    throw std::runtime_error(
        std::string("Parse Error: Unexpected char to start a PDF object: ") +
        in);
  }
  throw std::runtime_error(
      std::string("Parse Error: Unexpected char to start a PDF object: ") + in);
}

util::PdfArray *util::parse_pdf_array(std::istream &is) {
  PdfArray *arr = new PdfArray();
  char in;
  is >> in;

  // TODO handle EOF
  if (in == '[') {
    while (true) {
      is >> in;
      if (in == ']')
        break;
      is.unget();
      PdfObj *obj = util::parse_pdf_obj(is);
      arr->objects.push_back(obj);
    }
    return arr;
  } else {
    delete arr;
    throw std::runtime_error(
        std::string("Parse Error: Arrays must start with a [, Got ") + in);
  }
}

util::PdfName util::parse_pdf_name_obj(std::istream &is) {
  std::string name("/");
  char in;
  is >> in;
  if (in != '/')
    throw std::runtime_error(
        std::string("Parse Error: Pdf names must start with /, Got ") + in);

  return PdfName(get_name_token(is));
}

std::string util::get_name_token(std::istream &is) {
  std::string token("/");
  char in;
  while (true) {
    is.get(in);
    if (is.eof()) {
      break;
    } else if (ends_name(in)) {
      is.unget();
      break;
    } else if (!valid_name_char(in)) {
      throw std::runtime_error(
          std::string("Parse Error: Invalid char in PDF name: ") + in);
    }
    token += in;
  }
  return token;
}

util::PdfDict *util::parse_pdf_dict(std::istream &is) {
  char in, inB;
  is >> in;
  is.get(inB);
  if (in == '<' && inB == '<') {
    PdfDict *dict = new PdfDict();
    while (true) {
      is >> in;
      if (in == '>') {
        is >> in;
        if (in == '>' || is.eof()) {
          break;
        } else {
          throw std::runtime_error(
              std::string("Parse Error: Dicts must end with >>, Got >") + in);
        }
      }
      is.unget();
      PdfName name = parse_pdf_name_obj(is);
      PdfObj *obj = parse_pdf_obj(is);
      dict->pairs[name] = obj;
    }
    return dict;
  } else {
    throw std::runtime_error(
        std::string("Parse Error: Dicts must start with <<, Got ") + in + inB);
  }
}

std::vector<char> util::parse_pdf_content_stream(std::istream &is) {
  std::vector<char> contents;
  std::string token;
  char in;
  for (size_t i = 0; i < 6; i++) {
    is >> in;
    token += in;
  }

  if (token != "stream") {
    throw std::runtime_error(
        std::string(
            "Parse Error: Content stream must start with stream, Got ") +
        token);
  }

  std::string line;
  while (!is.eof()) {
    getline(is, line);
    auto pos = line.find("endstream");
    if (pos == std::string::npos) {
      contents.insert(contents.end(), line.begin(), line.end());
    } else {
      contents.insert(contents.end(), contents.begin(), contents.begin() + pos);
      break;
    }
  }
  return contents;
}

bool util::ends_name(char ch) {
  // Is it only space???
  return isspace(ch);
}

bool util::valid_name_char(char ch) {
  // I need to consult the specification to be sure of all chars not allowed
  // in a pdf name object
  return !isspace(ch) && !iscntrl(ch) && ch != '$' && ch != '*' && ch != '%' &&
         ch != '^' && ch != '&' && ch != '"' && ch != '\'';
}
