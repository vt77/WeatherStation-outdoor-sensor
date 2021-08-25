

#include <unity.h>
#include<iostream>
#include <stdio.h>
#include "JsonStaticString.h"

vt77::JsonStaticString<64> jsonString;

void test_json_string(void) {
    jsonString.start();
    TEST_ASSERT_MESSAGE(jsonString.insert("str1","test1"),"Inser string 1");
    TEST_ASSERT_MESSAGE(jsonString.insert("str2","test2"),"Inser string 2");
    TEST_ASSERT_MESSAGE(jsonString.insert("float",(double)32.2),"Insert float");
    TEST_ASSERT_MESSAGE(jsonString.insert("float",(int)28),"Insert int");
    jsonString.close();
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_json_string);
    UNITY_END();
    std::cout << jsonString << std::endl;
}

