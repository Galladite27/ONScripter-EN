#include "BaseReader.h"

#include "gtest/gtest.h"

namespace {

TEST (BaseReaderTest, FileInfoConstructor) {
  BaseReader::FileInfo fi;
  EXPECT_EQ(0u, fi.offset);
  EXPECT_EQ(0u, fi.length);
  EXPECT_EQ(0u, fi.original_length);
}

TEST (BaseReaderTest, ArchiveInfoConstructor) {
  BaseReader::ArchiveInfo ai;
  EXPECT_EQ(NULL, ai.next);
  EXPECT_EQ(NULL, ai.file_handle);
  EXPECT_EQ(NULL, ai.file_name);
  EXPECT_EQ(NULL, ai.fi_list);
  EXPECT_EQ(0u, ai.num_of_files);
  EXPECT_EQ(0u, ai.base_offset);
}

} // namespace
