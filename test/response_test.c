#include <CUnit/CUnit.h>
#include <stdlib.h>

#include "../src/response.h"



void test_resp_new() {
    response_t* resp = resp_new(200);

    CU_ASSERT_PTR_NOT_NULL_FATAL(resp);
    CU_ASSERT_STRING_EQUAL(resp->protocol, "HTTP/1.1");
    CU_ASSERT_EQUAL(resp->status, 200);
    CU_ASSERT_STRING_EQUAL(resp->reason, "OK");
    resp_free(resp);
}

void test_resp_to_str() {
    response_t* resp = resp_new(200);
    char* str = NULL;
    char* expected_status_line = "HTTP/1.1 200 OK\r\n";
    char* expected_header1 = "Server: Served\r\n";

    CU_ASSERT_PTR_NOT_NULL_FATAL(resp);
    resp_to_str(resp, &str);

    CU_ASSERT_NSTRING_EQUAL(
        str, expected_status_line, strlen(expected_status_line));
    CU_ASSERT_NSTRING_EQUAL(
        str + strlen(expected_status_line), expected_header1, strlen(expected_header1));

    resp_free(resp);
    free(str);
}