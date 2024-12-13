#include <string.h>
#include <stdlib.h>
#include <CUnit/CUnit.h>
#include "../src/request.h"
#include "../src/util.h"

// Test a simple message
void test_req_parse_simple() {
    char* msg = strdup(
        "GET /static/img/background.png HTTP/1.1\r\n"
        "Host: localhost:8000\r\n"
        "\r\n");
    
    request_t* req = NULL;
    int status;
    req = req_parse(msg, req, strlen(msg), &status);

    CU_ASSERT_PTR_NOT_NULL_FATAL(req);
    CU_ASSERT_EQUAL(status, 200);
    CU_ASSERT_EQUAL(req->method, GET);
    CU_ASSERT_STRING_EQUAL(req->target, "/static/img/background.png");
    CU_ASSERT_STRING_EQUAL(req->protocol, "HTTP/1.1");
    CU_ASSERT_STRING_EQUAL(req->headers->name, "host");
    CU_ASSERT_STRING_EQUAL(req->headers->value, "localhost:8000");

    free(msg);
}
