#include "nlohmann/json.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using json = nlohmann::json;

TEST (JsonTest, Default) {
	json example;
	example["test"] = true;
	ASSERT_TRUE(example["test"]);
}
