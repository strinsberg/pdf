#ifndef PDF_PARSER_H
#define PDF_PARSER_H

#include <string>
#include <vector>

/* Ok I decided to start in a more simple way. I am going to just build a parser
* that can traverse and extract information from a simple pdf. It will just read
* from a whole string at a time and have a bunch of methods for accessing
* different information. For now some things will be niave and I won't try to
* build a complex object structure. I just want to see if I can properly read
* and decode a pdf document. Then depending on what my goals might be I can
* separate different processes and consider things like building a full pdf
* object graph and structure in memory. For just parsing and understanding what
* is in a pdf I do not need to do all that. It seems that a pdf reader might
* need to incrementaly read a document or seek to different parts to be efficient.
* Holding everything in memory would be inefficient, however, I suspect that it
* is necessary to cache certain things in some way where possible. I don't know
* what the best ways to do that, but it has to be important. I suspect for a
* pdf editor there will need to be an in memory object structure created for
* a page that is being edited and and used to generate alterations to the document
* on a save. But I have no idea.
 */

class PdfParser {
public:
  // Initialize the parser with a reference to the string of bytes.
  PdfParser(const std::string &data);

  // Pull all the textual data from the pdf and return it as a single string.
  // This will be messy for now, but perhaps we can add some formatting to the
  // returned text to make it match the actual document structure eventually.
  // This is the first step discovering what is needed to parse each object
  // type and work with the cross reference table etc.
  std::string extract_text();

protected:
  const std::string &data;
};

#endif
