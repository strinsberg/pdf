#include "utility.h"
#include <fstream>
#include <gtest/gtest.h>

TEST(BytesTillEnd, ReadsTheNumOfBytesTillEndOfStreamAndRestoresStreamPos) {
  std::ifstream input("test/assets/hello_world.txt", std::ios::binary);
  if (!input.is_open())
    FAIL() << "Test file did not open";

  EXPECT_EQ(util::bytes_till_end(input), 14);
  EXPECT_EQ(input.tellg(), 0);

  input.seekg(5, std::ios_base::beg);
  EXPECT_EQ(util::bytes_till_end(input), 9);
  EXPECT_EQ(input.tellg(), 5);
}

TEST(SlurpFileBytes, ReadsTheBytesFromTheFileIntoCharVector) {
  auto input = util::slurp_bytes("test/assets/hello_world.txt");
  std::vector<char> expected = {'H', 'e', 'l', 'l', 'o', ',', ' ',
                                'W', 'o', 'r', 'l', 'd', '!', '\n'};
  EXPECT_EQ(input, expected);
}

TEST(Inflate, does_it_decompress_deflate_text) {
  std::ifstream input("test/assets/the_raven.gz", std::ios::binary);
  if (!input.is_open())
    FAIL() << "Compressed data file did not open";

  size_t input_size = util::bytes_till_end(input);

  auto expected = util::slurp_bytes("test/assets/the_raven.txt");

  auto decompressed = util::inflate_stream(input, input_size);
  EXPECT_EQ(decompressed.size(), expected.size());
  EXPECT_EQ(decompressed, expected);

  input.close();
}
