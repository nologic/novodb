//
//  validation_tests.cpp
//  Novodb
//
//  Created by mike on 10/13/14.
//  Copyright (c) 2014 Mikhail Sosonkin. All rights reserved.
//

#include "gtest/gtest.h"
#include "../Independent/request_router.h"

#include "include/cef_request.h"

TEST(RouterConstraints, IntKey) {
    CefRefPtr<CefRequest> request1 = CefRequest::Create();
    request1->SetMethod(CefString("GET"));
    request1->SetURL(CefString("dbg-llvm://create?key=3"));
    
    EXPECT_TRUE(RequestConstraint::has_int("key")(request1).is_valid());
    
    CefRefPtr<CefRequest> request2 = CefRequest::Create();
    request2->SetMethod(CefString("GET"));
    request2->SetURL(CefString("dbg-llvm://create?key=a"));

    EXPECT_FALSE(RequestConstraint::has_int("key")(request2).is_valid());
    
    CefRefPtr<CefRequest> request3 = CefRequest::Create();
    request3->SetMethod(CefString("GET"));
    request3->SetURL(CefString("dbg-llvm://create"));
    
    EXPECT_FALSE(RequestConstraint::has_int("key")(request3).is_valid());

}

TEST(RouterConstraints, ExistKey) {
    CefRefPtr<CefRequest> request1 = CefRequest::Create();
    request1->SetMethod(CefString("GET"));
    request1->SetURL(CefString("dbg-llvm://create?key=3"));
    
    EXPECT_TRUE(RequestConstraint::exists({"key"})(request1).is_valid());

    CefRefPtr<CefRequest> request2 = CefRequest::Create();
    request2->SetMethod(CefString("GET"));
    request2->SetURL(CefString("dbg-llvm://create?key=3&key2=2"));
    
    EXPECT_TRUE(RequestConstraint::exists({"key2", "key"})(request2).is_valid());

    CefRefPtr<CefRequest> request3 = CefRequest::Create();
    request3->SetMethod(CefString("GET"));
    request3->SetURL(CefString("dbg-llvm://create?key=3&key2=2"));
    
    EXPECT_TRUE(RequestConstraint::exists({"key2"})(request3).is_valid());

    CefRefPtr<CefRequest> request4 = CefRequest::Create();
    request4->SetMethod(CefString("GET"));
    request4->SetURL(CefString("dbg-llvm://create?key=3"));
    
    EXPECT_FALSE(RequestConstraint::exists({"key2"})(request4).is_valid());
}