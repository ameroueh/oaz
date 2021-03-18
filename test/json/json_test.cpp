#include "nlohmann/json.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using json = nlohmann::json;

TEST(JsonTest, Default) {
  json example;
  example["test"] = true;
  ASSERT_TRUE(example["test"]);
}
