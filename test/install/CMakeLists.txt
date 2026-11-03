cmake_minimum_required(VERSION 3.28)
project(StormByteInstalledConsumer LANGUAGES CXX)

find_package(StormByte REQUIRED COMPONENTS Base)

add_executable(InstalledConsumer installed_consumer_test.cxx)
target_compile_features(InstalledConsumer PRIVATE cxx_std_26)
target_link_libraries(InstalledConsumer PRIVATE StormByte::Base)

enable_testing()
add_test(NAME InstalledConsumer COMMAND InstalledConsumer)
