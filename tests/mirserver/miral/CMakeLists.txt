set(
  MIRAL_TEST_SOURCES
  edid_test.cpp
)

include_directories(
  ${MIROIL_INCLUDE_DIRS}
)

add_executable(MirALTests ${MIRAL_TEST_SOURCES})

target_link_libraries(MirALTests
  qpa-mirserver
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
  ${MIROIL_LDFLAGS}
)

add_test(MirAL, MirALTests)
