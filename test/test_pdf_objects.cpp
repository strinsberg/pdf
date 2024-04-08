#include "utility.h"
#include <gtest/gtest.h>
#include <sstream>

// This way we can remove this when these objects are moved, rather than
// refactoring all of the names.
using namespace util;

TEST(PdfNull, DoesItWriteItsValueToTheStream) {
  PdfNull obj;
  std::stringstream os;
  obj.write(os);
  EXPECT_EQ(os.str(), "null");
}

TEST(PdfInt, DoesItWriteItsValueToTheStream) {
  PdfInt obj(1024);
  std::stringstream os;
  obj.write(os);
  EXPECT_EQ(os.str(), "1024");
}

TEST(PdfReal, DoesItWriteItsValueToTheStream) {
  PdfReal obj(1024.09876);
  std::stringstream os;
  obj.write(os);
  EXPECT_EQ(os.str(), "1024.09876");
}

TEST(PdfString, DoesItWriteItsValueToTheStream) {
  PdfString obj("Hello, World!");
  std::stringstream os;
  obj.write(os);
  EXPECT_EQ(os.str(), "(Hello, World!)");
}

TEST(PdfName, DoesItWriteItsValueToTheStream) {
  PdfName obj("/SomeName");
  std::stringstream os;
  obj.write(os);
  EXPECT_EQ(os.str(), "/SomeName");
}

TEST(PdfRef, DoesItWriteItsValueToTheStream) {
  PdfRef obj(23, 0);
  std::stringstream os;
  obj.write(os);
  EXPECT_EQ(os.str(), "23 0 R");
}

TEST(PdfBool, DoesItWriteItsValueToTheStream) {
  PdfBool obj_t(true);
  PdfBool obj_f(false);

  std::stringstream os;
  obj_t.write(os);
  EXPECT_EQ(os.str(), "true");

  os.str("");
  os.clear();
  obj_f.write(os);
  EXPECT_EQ(os.str(), "false");
}

TEST(PdfArray, DoesItWriteItsValueToTheStream) {
  PdfArray obj;
  obj.objects.push_back(new PdfNull());
  obj.objects.push_back(new PdfBool(true));
  obj.objects.push_back(new PdfName("/Strinsberg"));
  obj.objects.push_back(new PdfInt(42));

  std::stringstream os;
  obj.write(os);
  EXPECT_EQ(os.str(), "[ null true /Strinsberg 42 ]");
}

TEST(PdfDict, DoesItWriteItsValueToTheStream) {
  PdfDict obj;
  obj.pairs[PdfName("/First")] = new PdfNull();
  obj.pairs[PdfName("/Second")] = new PdfBool(false);
  obj.pairs[PdfName("/Third")] = new PdfString("Hello");
  obj.pairs[PdfName("/Fourth")] = new PdfReal(42.05);

  std::stringstream os;
  obj.write(os);
  // Note we are using an ordered map so the order is string < on the keys
  // This is somewhat unreliable, but if it breaks here the above insertions
  // will also have to change and it should be clear.
  EXPECT_EQ(os.str(),
            "<< /First null /Fourth 42.05 /Second false /Third (Hello) >>");
}

TEST(PdfStream, DoesItWriteItsValueToTheStream) {
  PdfStream obj;
  obj.dict->pairs[PdfName("/First")] = new PdfNull();
  obj.dict->pairs[PdfName("/Second")] = new PdfBool(false);
  obj.dict->pairs[PdfName("/Third")] = new PdfString("Hello");
  obj.dict->pairs[PdfName("/Fourth")] = new PdfReal(42.05);
  obj.stream = {'H', 'e', 'l', 'l', 'o', ',', ' ',
                'W', 'o', 'r', 'l', 'd', '!'};

  std::stringstream os;
  obj.write(os);
  std::string expected(
      "<< /First null /Fourth 42.05 /Second false /Third (Hello) >>\n"
      "stream\nHello, World!\nendstream\n");
  EXPECT_EQ(os.str(), expected);
}

TEST(PdfTopLevel, DoesItWriteItsValueToTheStream) {
  PdfArray *obj = new PdfArray();
  obj->objects.push_back(new PdfNull());
  obj->objects.push_back(new PdfBool(true));
  obj->objects.push_back(new PdfName("/Strinsberg"));
  obj->objects.push_back(new PdfInt(42));

  PdfTopLevel top(23, 0, obj);

  std::stringstream os;
  top.write(os);
  std::string expected = "23 0 obj\n"
                         "[ null true /Strinsberg 42 ]\n"
                         "endobj\n";
  EXPECT_EQ(os.str(), expected);
}
