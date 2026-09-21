if(NOT COMMAND catch_discover_tests)
  find_package(grevir-test-support CONFIG REQUIRED)
endif()
add_executable(grevir_encoder_runtime encoder_test.cpp)
target_link_libraries(grevir_encoder_runtime PRIVATE grevir::encoder Catch2::Catch2WithMain)
set_target_properties(grevir_encoder_runtime PROPERTIES CXX_EXTENSIONS OFF)
catch_discover_tests(grevir_encoder_runtime TEST_PREFIX "encoder."
  PROPERTIES LABELS "encoder" TIMEOUT 10)
