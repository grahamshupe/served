#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "request_test.h"
#include "response_test.h"

#define RUN_MODE CU_BRM_VERBOSE

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();
    
    /* Test arrays: */
    CU_TestInfo req_tests[] = {
        {"test_req_parse_simple", test_req_parse_simple},
        CU_TEST_INFO_NULL
    };
    CU_TestInfo resp_tests[] = {
        {"test_resp_new", test_resp_new},
        {"test_resp_to_str", test_resp_to_str},
        CU_TEST_INFO_NULL
    };

    /* Suites: */
    CU_SuiteInfo suites[] = {
        {"request_test", NULL, NULL, req_tests},
        {"response_test", NULL, NULL, resp_tests},
        CU_SUITE_INFO_NULL
    };

    if (CU_register_suites(suites) != CUE_SUCCESS) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    CU_basic_set_mode(RUN_MODE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}