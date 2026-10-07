add_executable(BinaryTests binary_test.cxx)
target_link_libraries(BinaryTests StormByte)
add_test(NAME BinaryTests COMMAND BinaryTests)

add_executable(CallbackTests callback_test.cxx)
target_link_libraries(CallbackTests StormByte)
add_test(NAME CallbackTests COMMAND CallbackTests)

add_executable(ClonableTests clonable_test.cxx)
target_link_libraries(ClonableTests StormByte)
add_test(NAME ClonableTests COMMAND ClonableTests)

add_executable(SafeExceptionTests exception_test.cxx)
target_link_libraries(SafeExceptionTests StormByte)
add_test(NAME SafeExceptionTests COMMAND SafeExceptionTests)

add_executable(FunctionTests function_test.cxx)
target_link_libraries(FunctionTests StormByte)
add_test(NAME FunctionTests COMMAND FunctionTests)

add_executable(HeapTests heap_test.cxx)
target_link_libraries(HeapTests StormByte)
add_test(NAME HeapTests COMMAND HeapTests)

add_executable(IterableTests iterable_test.cxx)
target_link_libraries(IterableTests StormByte)
add_test(NAME IterableTests COMMAND IterableTests)

add_executable(MapTests map_test.cxx)
target_link_libraries(MapTests StormByte)
add_test(NAME MapTests COMMAND MapTests)

add_executable(OptionalTests optional_test.cxx)
target_link_libraries(OptionalTests StormByte)
add_test(NAME OptionalTests COMMAND OptionalTests)

add_executable(OwnerTests owner_test.cxx)
target_link_libraries(OwnerTests StormByte)
add_test(NAME OwnerTests COMMAND OwnerTests)

add_executable(PairTests pair_test.cxx)
target_link_libraries(PairTests StormByte)
add_test(NAME PairTests COMMAND PairTests)

add_executable(PointerTests pointers_test.cxx)
target_link_libraries(PointerTests StormByte)
add_test(NAME PointerTests COMMAND PointerTests)

add_executable(QueueTests queue_test.cxx)
target_link_libraries(QueueTests StormByte)
add_test(NAME QueueTests COMMAND QueueTests)

add_executable(StringTests string_test.cxx)
target_link_libraries(StringTests StormByte)
add_test(NAME StringTests COMMAND StringTests)

add_executable(VectorTests vector_test.cxx)
target_link_libraries(VectorTests StormByte)
add_test(NAME VectorTests COMMAND VectorTests)

add_executable(WStringTests wstring_test.cxx)
target_link_libraries(WStringTests StormByte)
add_test(NAME WStringTests COMMAND WStringTests)
