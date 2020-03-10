macro(run_conan)
set(CONAN_FILE "${CMAKE_SOURCE_DIR}/thirdparty/conan.cmake")
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS ${CONAN_FILE})
  message(STATUS
          "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" ${CONAN_FILE})
endif()

include(${CONAN_FILE})

conan_add_remote(NAME bincrafters URL
                 https://api.bintray.com/conan/bincrafters/public-conan)

conan_cmake_run(
  REQUIRES ${CONAN_EXTRA_REQUIRES}
  OPTIONS  ${CONAN_EXTRA_OPTIONS}
  BASIC_SETUP
  CMAKE_TARGETS # individual targets to link to
  BUILD
  missing
)
endmacro()
