if(BUILD_TESTING)
    jmmt_simple_test(fourcc_policy_tests)
    target_link_libraries(fourcc_policy_tests PRIVATE
        mco::nounit
        jmmt::libjmmt
    )
endif()
