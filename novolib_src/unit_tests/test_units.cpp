//
//  test_units.cpp
//  Novodb
//
//  Created by mike on 10/12/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "gtest/gtest.h"
#include "../Independent/json_serialize.h"

TEST(ToString, ZeroZero) {
    EXPECT_EQ("0", addr_to_string(0x0));
}