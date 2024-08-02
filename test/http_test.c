#include <string.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include "../src/http.h"

#define RUN_MODE CU_BRM_VERBOSE

void http_request();
void http_response();

int main() {
    CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();
    
    pSuite = CU_add_suite("http_request_suite", 0, 0);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if(NULL == CU_add_test(pSuite, "http_request", http_request) ||
        NULL == CU_add_test(pSuite, "http_response", http_response)) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    CU_basic_set_mode(RUN_MODE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}



void http_request() {
    char* msg = 
        "GET /static/img/background.png HTTP/1.1\r\n"
        "Host: localhost:8000\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*\r\n"
        "\r\n";

    struct request req;
    int status = req_parse(msg, &req, strlen(msg));

    CU_ASSERT_EQUAL(status, 200);
    CU_ASSERT_EQUAL(req.method, GET);
    CU_ASSERT_STRING_EQUAL(req.target, "/static/img/background.png");
    CU_ASSERT_STRING_EQUAL(req.protocol, "HTTP/1.1");

    req_free(&req);
}

void http_response() {

    struct response resp;
    resp_new(&resp, 400, NULL);

    resp_add_header(&resp, "Header", "value");

    CU_ASSERT_EQUAL(resp.status, 400);
    CU_ASSERT_EQUAL(resp.body, NULL);
    CU_ASSERT_STRING_EQUAL(resp_get_header(&resp, "Server"), "Served");

    char* str = malloc(8192);
    resp_to_str(&resp, str);

    printf("\n%s\n", str);

    resp_free(&resp);
}