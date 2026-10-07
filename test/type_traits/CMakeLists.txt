add_executable(CategoriesTests categories_test.cxx)
target_link_libraries(CategoriesTests StormByte)
add_test(NAME CategoriesTests COMMAND CategoriesTests)

add_executable(ComparisonTests comparison_test.cxx)
target_link_libraries(ComparisonTests StormByte)
add_test(NAME ComparisonTests COMMAND ComparisonTests)

add_executable(ContainersTests containers_test.cxx)
target_link_libraries(ContainersTests StormByte)
add_test(NAME ContainersTests COMMAND ContainersTests)

add_executable(ConversionsTests conversions_test.cxx)
target_link_libraries(ConversionsTests StormByte)
add_test(NAME ConversionsTests COMMAND ConversionsTests)

add_executable(DetailTests detail_test.cxx)
target_link_libraries(DetailTests StormByte)
add_test(NAME DetailTests COMMAND DetailTests)

add_executable(EnumsTests enums_test.cxx)
target_link_libraries(EnumsTests StormByte)
add_test(NAME EnumsTests COMMAND EnumsTests)

add_executable(ObjectSemanticsTests object_semantics_test.cxx)
target_link_libraries(ObjectSemanticsTests StormByte)
add_test(NAME ObjectSemanticsTests COMMAND ObjectSemanticsTests)

add_executable(RangesTests ranges_test.cxx)
target_link_libraries(RangesTests StormByte)
add_test(NAME RangesTests COMMAND RangesTests)

add_executable(RelationsTests relations_test.cxx)
target_link_libraries(RelationsTests StormByte)
add_test(NAME RelationsTests COMMAND RelationsTests)

add_executable(SafeTraitsTests safe_test.cxx)
target_link_libraries(SafeTraitsTests StormByte)
add_test(NAME SafeTraitsTests COMMAND SafeTraitsTests)

add_executable(WrappersTests wrappers_test.cxx)
target_link_libraries(WrappersTests StormByte)
add_test(NAME WrappersTests COMMAND WrappersTests)
