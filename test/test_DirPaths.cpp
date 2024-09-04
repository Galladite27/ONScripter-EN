#include "DirPaths.h"

#include "gtest/gtest.h"

namespace {

TEST (DirPathsTest, ConstructorWithPath) {
  std::string path = ".";
  const DirPaths dp(path.c_str());
  EXPECT_EQ(1, dp.get_num_paths());
}

TEST (DirPathsTest, CopyConstructor) {
  std::string path = ".";
  const DirPaths dp(path.c_str());
  const DirPaths d2 = dp;
  EXPECT_EQ(1, dp.get_num_paths());
  EXPECT_EQ(1, d2.get_num_paths());
}

TEST (DirPathsTest, OperatorEquals) {
  DirPaths dp;
  DirPaths const d2;
  dp = d2;
  EXPECT_EQ(0, dp.get_num_paths());
  EXPECT_EQ(0, d2.get_num_paths());
}

TEST (DirPathsTest, addWithDirPaths) {
  std::string path = ".";
  DirPaths dp(path.c_str());
  DirPaths d2(path.c_str());
  dp.add(d2);
  EXPECT_EQ(2, dp.get_num_paths());
}

TEST (DirPathsTest, get_path) {
  std::string path = ".";
  DirPaths dp(path.c_str());
  EXPECT_TRUE(dp.get_path(0));
}

TEST (DirPathsTest, get_all_paths) {
  std::string path = ".";
  DirPaths dp(path.c_str());
  EXPECT_TRUE(dp.get_all_paths());
}

TEST (DirPathsTest, max_path_len) {
  std::string path = ".";
  DirPaths dp(path.c_str());
  EXPECT_EQ(2, dp.max_path_len());
}

} // namespace
