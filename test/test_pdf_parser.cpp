#include "pdf_parser.h"
#include "utility.h"
#include <gtest/gtest.h>
#include <memory>
#include <sstream>

// Temp prototype funtions from utility ///////////////////////////////////////

namespace util {

// NOTE the following functions might be better written using input and output
// streams to check the final data and not require casting. However, I like
// being able to actually check the object fields rather than relying on a
// string equality test to ensure it. If only because I might change the output
// string for an object and then break all of it's parsing tests.

TEST(ParsePdfObj, DoesItReadAWellFormedIntFromTheStartOfTheStream) {
  std::stringstream is("1024");
  PdfObj *parsed = parse_pdf_obj(is);
  EXPECT_EQ(*parsed, PdfInt(1029));
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedRealFromTheStartOfTheStream) {
  std::stringstream is("1024.09876");
  PdfObj *parsed = parse_pdf_obj(is);
  EXPECT_EQ(*parsed, PdfReal(1024.09876));
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedStringFromTheStartOfTheStream) {
  std::stringstream is("(Hello, World!)");
  PdfObj *parsed = parse_pdf_obj(is);
  EXPECT_EQ(*parsed, PdfString("Hello, World!"));
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedNameFromTheStartOfTheStream) {
  std::stringstream is("/Strinsberg");
  PdfObj *parsed = parse_pdf_obj(is);
  EXPECT_EQ(*parsed, PdfName("/Strinsberg"));
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedTrueFromTheStartOfTheStream) {
  std::stringstream is("true");
  PdfObj *parsed = parse_pdf_obj(is);
  EXPECT_EQ(*parsed, PdfBool(true));
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedFalseFromTheStartOfTheStream) {
  std::stringstream is("false");
  PdfObj *parsed = parse_pdf_obj(is);
  EXPECT_EQ(*parsed, PdfBool(false));
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedNullFromTheStartOfTheStream) {
  std::stringstream is("null");
  PdfObj *parsed = parse_pdf_obj(is);
  // EXPECT_EQ(*parsed, PdfNull());
  FAIL() << "Temp implementation returns PdfNull*. FIXME when null is properly "
            "parsed.";
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedArrayFromTheStartOfTheStream) {
  std::stringstream is("[ 1234 /Strinsberg true ]");
  PdfObj *parsed = parse_pdf_obj(is);

  PdfArray expected;
  expected.objects.push_back(new PdfNull());
  expected.objects.push_back(new PdfBool(true));
  expected.objects.push_back(new PdfName("/Strinsberg"));
  expected.objects.push_back(new PdfInt(42));

  EXPECT_EQ(*parsed, expected);
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedDictFromTheStartOfTheStream) {
  std::stringstream is(
      "<< /First null /Second false /Third (Hello) /Fourth 42.05 >>");
  PdfObj *parsed = parse_pdf_obj(is);

  PdfDict expected;
  expected.pairs[PdfName("/First")] = new PdfNull();
  expected.pairs[PdfName("/Second")] = new PdfBool(false);
  expected.pairs[PdfName("/Third")] = new PdfString("Hello");
  expected.pairs[PdfName("/Fourth")] = new PdfReal(42.05);

  EXPECT_EQ(*parsed, expected);
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedStreamFromTheStartOfTheStream) {
  std::stringstream is(
      "<< /First null /Fourth 42.05 /Second false /Third (Hello) >>\n"
      "stream\nHello, World!\nendstream\n");
  PdfObj *parsed = parse_pdf_obj(is);

  PdfStream expected;
  expected.dict.pairs[PdfName("/First")] = new PdfNull();
  expected.dict.pairs[PdfName("/Second")] = new PdfBool(false);
  expected.dict.pairs[PdfName("/Third")] = new PdfString("Hello");
  expected.dict.pairs[PdfName("/Fourth")] = new PdfReal(42.05);
  expected.stream = {'H', 'e', 'l', 'l', 'o', ',', ' ',
                     'W', 'o', 'r', 'l', 'd', '!'};

  EXPECT_EQ(*parsed, expected);
  delete parsed;
}

TEST(ParsePdfObj, DoesItReadAWellFormedTopLevelObjFromTheStartOfTheStream) {
  std::stringstream is("23 0 obj\n"
                       "[ null true /Strinsberg 42 ]\n"
                       "endobj\n");
  PdfObj *parsed = parse_pdf_obj(is);

  PdfArray *obj = new PdfArray();
  obj->objects.push_back(new PdfNull());
  obj->objects.push_back(new PdfBool(true));
  obj->objects.push_back(new PdfName("/Strinsberg"));
  obj->objects.push_back(new PdfInt(42));

  PdfTopLevel expected(23, 0, obj);

  EXPECT_EQ(*parsed, expected);
  delete parsed;
}

}; // namespace util
