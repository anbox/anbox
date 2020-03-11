pkg_check_modules(GTEST      REQUIRED gtest     )
pkg_check_modules(GTEST_MAIN REQUIRED gtest_main)
pkg_check_modules(GMOCK      REQUIRED gmock     )
pkg_check_modules(GMOCK_MAIN REQUIRED gmock_main)

set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
