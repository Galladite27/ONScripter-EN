#include <cstring>

#include "Encoding.h"

#include "gtest/gtest.h"

namespace {

TEST (TestEncoding, DefaultConstructor) {
  Encoding enc;
  EXPECT_EQ(Encoding::CODE_CP932, enc.getEncoding());
}

TEST (TestEncoding, getEncoding) {
  Encoding enc;
  int code = enc.getEncoding();
  EXPECT_EQ(Encoding::CODE_CP932, code);
}

TEST (TestEncoding, setEncoding) {
  Encoding enc;
  enc.setEncoding(Encoding::CODE_UTF8);
  EXPECT_EQ(Encoding::CODE_UTF8, enc.getEncoding());
}

TEST (TestEncoding, getTextMarker) {
  Encoding enc;
  char marker;

  enc.setEncoding(Encoding::CODE_UTF8);
  marker = enc.getTextMarker();
  EXPECT_TRUE(marker);
  EXPECT_EQ('^', marker);
}

TEST (TestEncoding, getTextMarkerWithCP932) {
  Encoding enc;
  char marker;

  enc.setEncoding(Encoding::CODE_CP932);
  marker = enc.getTextMarker();
  EXPECT_TRUE(marker);
  EXPECT_EQ('`', marker);
}

TEST (TestEncoding, getBytes) {
  Encoding enc;
  int bytes = -1;

  bytes = enc.getBytes('a', Encoding::CODE_UTF8);
  EXPECT_EQ(1, bytes);
  bytes = enc.getBytes(0xC0, Encoding::CODE_UTF8);
  EXPECT_EQ(2, bytes);
  bytes = enc.getBytes(0xE0, Encoding::CODE_UTF8);
  EXPECT_EQ(3, bytes);
  bytes = enc.getBytes(0xF0, Encoding::CODE_UTF8);
  EXPECT_EQ(4, bytes);
}

TEST (TestEncoding, getNum) {
  const char* buf = "abc123";
  Encoding enc;
  int num = enc.getNum((const unsigned char*)buf);
  EXPECT_EQ(6, num);
}

TEST (TestEncoding, getUTF16) {
  Encoding enc;
  std::string text("abc123");
  unsigned short unicode = enc.getUTF16(text.c_str(), Encoding::CODE_UTF8);
  EXPECT_EQ(97, unicode);
}

TEST (TestEncoding, getUTF16WithCP932) {
  Encoding enc;
  char* str = new char[8];
  ::strncpy(str, "abc123", 6);
  unsigned short unicode = enc.getUTF16(str, Encoding::CODE_CP932);
  EXPECT_EQ(97, unicode);
  delete[] str;
  str = new char[2];
  str[0] = 0xA0;
  str[1] = 0x0;
  unicode = enc.getUTF16(str, Encoding::CODE_CP932);
  EXPECT_EQ(65376, unicode);
  delete[] str;
}

} // namespace
