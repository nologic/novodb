//
//  action_tests.cpp
//  Novodb
//
//  Created by mike on 12/14/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "gtest/gtest.h"

#include "../Independent/request_router.h"

using namespace novo;

TEST(RouterOperations, ActionResponseCopy) {
    ActionResponse ar1 = ActionResponse::no_error();
    ActionResponse ar2 = ar1;
    
    EXPECT_EQ(ar1.get_path().toString(), ar2.get_path().toString());
}