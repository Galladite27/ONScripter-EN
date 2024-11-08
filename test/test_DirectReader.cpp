#include <iostream>

#include "DirectReader.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class MockPathProvider : public PathProvider
{
public:
  MOCK_CONST_METHOD1(get_path, const char *(int));
  MOCK_CONST_METHOD0(get_all_paths, const char *());
  MOCK_CONST_METHOD0(get_num_paths, int());
  MOCK_CONST_METHOD0(max_path_len, size_t());
};

extern bool hasTwoByteChar(const unsigned char *);

namespace {

TEST (DirectReaderTest, OpenAndClose) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);
  ASSERT_EQ(0u, dr.open());
  ASSERT_EQ(0u, dr.close());
}

TEST (DirectReaderTest, getArchiveName) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);
  ASSERT_STREQ("direct", dr.getArchiveName());
}

TEST (DirectReaderTest, getNumFiles) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);
  ASSERT_EQ(0u, dr.getNumFiles());
}

TEST (DirectReaderTest, getFileByIndex) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);
  struct DirectReader::FileInfo fInfo = dr.getFileByIndex(0);
  ASSERT_EQ(0u, fInfo.compression_type);
  ASSERT_EQ(0u, fInfo.length);
  ASSERT_EQ(0u, fInfo.offset);
  ASSERT_EQ(0u, fInfo.original_length);
}

#define MAX_PATH_LEN (int)128
#define PATH_COUNT (int)1

TEST (DirectReaderTest, getFileLengthWithNonexistentFile) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);

  size_t length = dr.getFileLength("");

  ASSERT_EQ(0u, length);
}

TEST (DirectReaderTest, getFileLengthWithNull) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);

  size_t length = dr.getFileLength(NULL);

  ASSERT_EQ(0u, length);
}

TEST (DirectReaderTest, getFileLengthWithTempFile_POSIX) {
#if !defined(WIN32) && !defined(MACOSX)
  int result = 0;
  char *tmp = ::getenv("TMPDIR");
  const unsigned char *str = new unsigned char[256];
  char *tmpl = new char[256];
  tmpl[0] = '\0'; // ensure 'tmpl' is an empty string
  if (tmp != NULL) {
    ::strcpy(tmpl, tmp);
    ::strncat(tmpl, "/", 1);
  } else {
    ::strcpy(tmpl, "/var/tmp/");
    tmp = new char[strlen(tmpl) + 1];
    tmp[0] = '\0'; // ensure 'tmp' is an empty string
    ::strncpy(tmp, tmpl, strlen(tmpl));
  }
  const int FNAME_MAX = 256;
  char *fName = new char[FNAME_MAX];
  ::memset(fName, 0, FNAME_MAX);
  ::strncpy(fName, "directreader_test_tmpXXXXXX", 28);
  delete[] tmpl;
  MockPathProvider provider;
  EXPECT_CALL(provider, max_path_len()).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(MAX_PATH_LEN));
  EXPECT_CALL(provider, get_num_paths()).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(PATH_COUNT));
  EXPECT_CALL(provider, get_path(testing::Le(PATH_COUNT))).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(tmp));
  DirectReader dr(provider, str);
  FILE *file = NULL;
  ::chdir(tmp);
  int fd = ::mkstemp(fName);
  if (fd) {
    file = ::fdopen(fd, "w");
    if (file) {
      ::fprintf(file, "%s\n", "abc123");
    } else {
      std::cerr << "File descriptor" << fd << " could not be converted to a FILE* : errno=" << errno << std::endl;
      ::close(fd);
    }
  } else {
    std::cerr << "File " << tmp << "/" << fName << " was not created : errno=" << errno << std::endl;
  }

  if (file) {
    result = ::fclose(file);
    if (result != 0) {
      std::cerr << "File " << tmp << "/" << fName << " was not closed : errno=" << errno << std::endl;
    }
  }

  delete[] str;

  const size_t length = dr.getFileLength(fName);

  result = ::unlink(fName);
  if (result != 0) {
    std::cerr << "File " << tmp << "/" << fName << " was not removed : errno=" << errno << std::endl;
  }

  delete[] fName;

  ASSERT_EQ(7u, length);
#else
  SUCCEED();  //NOTE the version of GTest we use does not support skipping
#endif
}

TEST (DirectReaderTest, getFile) {
  char *tmp = ::getenv("TMPDIR");
  const unsigned char *str = new unsigned char[256];
  char *tmpl = new char[256];
  tmpl[0] = '\0'; // ensure 'tmpl' is an empty string
  if (tmp != NULL) {
    ::strcpy(tmpl, tmp);
    ::strncat(tmpl, "/", 1);
  } else {
    ::strcpy(tmpl, "/var/tmp/");
    tmp = new char[strlen(tmpl) + 1];
    tmp[0] = '\0'; // ensure 'tmp' is an empty string
    ::strncpy(tmp, tmpl, strlen(tmpl));
  }
  const int FNAME_MAX = 256;
  char *fName = new char[FNAME_MAX];
  ::memset(fName, 0, FNAME_MAX);
  ::strncpy(fName, "directreader_test_tmpXXXXXX", 28);
  delete[] tmpl;
  MockPathProvider provider;
  EXPECT_CALL(provider, max_path_len()).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(MAX_PATH_LEN));
  EXPECT_CALL(provider, get_num_paths()).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(PATH_COUNT));
  EXPECT_CALL(provider, get_path(testing::Le(PATH_COUNT))).Times(testing::AtLeast(1)).WillRepeatedly(testing::Return(tmp));
  DirectReader dr(provider, str);
  unsigned char* buf = new unsigned char[256];
  int loc = 0;
  ::chdir(tmp);
  int fd = ::mkstemp(fName);
  if (!fd) {
    std::cerr << "File " << fName << " was not created : errno=" << errno << std::endl;
  }

  int result = ::close(fd);
  if (result != 0) {
    std::cerr << "File " << fName << " was not closed : errno=" << errno << std::endl;
  }

  const size_t length = dr.getFile(fName, buf, &loc);

  result = ::unlink(fName);
  if (result != 0) {
    std::cerr << "File " << fName << " was not removed : errno=" << errno << std::endl;
  }

  delete[] buf;
  delete[] fName;
  delete[] str;

  ASSERT_EQ(0u, length);
  ASSERT_EQ(0u, loc);
}

TEST (DirectReaderTest, getFileWithEmptyFileName) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);
  unsigned char* buf = new unsigned char[256];
  int loc;

  size_t size = dr.getFile("", buf, &loc);

  delete[] buf;

  ASSERT_EQ(0u, size);
  ASSERT_EQ(BaseReader::ARCHIVE_TYPE_NONE, loc);
}

TEST (DirectReaderTest, getFileWithNull) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);
  ASSERT_EQ(0u, dr.getFile(NULL, NULL, NULL));
}

TEST (DirectReaderTest, convertFromSJISToEUC) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);
  char *sjis_no = new char[3];
  sjis_no[0] = 0x82;
  sjis_no[1] = 0xCC;
  sjis_no[2] = 0x0;
  dr.convertFromSJISToEUC(sjis_no);
  ASSERT_STREQ("\xA4\xCE", sjis_no);
  delete[] sjis_no;
}

TEST (DirectReaderTest, convertFromSJISToUTF8) {
  const unsigned char* str = NULL;
  MockPathProvider provider;
  DirectReader dr(provider, str);
  char *dst = new char[2];
  dst[0] = '\0';
  char *sjis_no = new char[3];
  sjis_no[0] = 0x82;
  sjis_no[1] = 0xCC;
  sjis_no[2] = '\0';
  dr.convertFromSJISToUTF8(dst, sjis_no);
// NOTE this method does not funcion correctly on anything other than Mac and Windows
#if defined(RECODING_FILENAMES) || defined(UTF8_FILESYSTEM) || defined(WIN32)
  ASSERT_STREQ("\xE3\x81\xAE", dst);
#endif
  delete[] sjis_no;
  delete[] dst;
}

TEST (DirectReaderTest, hasTwoByteChar) {
  unsigned char *str = new unsigned char[3];
  str[0] = 0x79;
  str[1] = 0x0;
  str[2] = 0x0; // NOTE 3rd byte MUST be a NULL!
  ASSERT_FALSE(::hasTwoByteChar(str));
  delete[] str;
  str = new unsigned char[3];
  str[0] = 0x80;
  str[1] = 0x0;
  str[2] = 0x0;
  ASSERT_TRUE(::hasTwoByteChar(str));
  delete[] str;
  str = new unsigned char[3];
  str[0] = 0xE0;
  str[1] = 0x0;
  str[2] = 0x0;
  ASSERT_TRUE(::hasTwoByteChar(str));
  delete[] str;
}

TEST (DirectReaderTest, hasTwoByteCharWithEmpty) {
  unsigned char* str = new unsigned char[1];
  str[0] = 0x0;
  ASSERT_FALSE(::hasTwoByteChar(str));
  delete[] str;
}

TEST (DirectReaderTest, hasTwoByteCharWithNull) {
  const unsigned char* str = NULL;
  ASSERT_FALSE(::hasTwoByteChar(str));
}

} // namespace
