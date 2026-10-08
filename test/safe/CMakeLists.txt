add_executable(AtomicTests atomic_test.cxx)
target_link_libraries(AtomicTests StormByte)
add_test(NAME AtomicTests COMMAND AtomicTests)

add_executable(BinaryTests binary_test.cxx)
target_link_libraries(BinaryTests StormByte)
add_test(NAME BinaryTests COMMAND BinaryTests)

add_executable(CallbackTests callback_test.cxx)
target_link_libraries(CallbackTests StormByte)
add_test(NAME CallbackTests COMMAND CallbackTests)

add_executable(ClonableTests clonable_test.cxx)
target_link_libraries(ClonableTests StormByte)
add_test(NAME ClonableTests COMMAND ClonableTests)

add_executable(ConditionVariableAnyTests condition_variable_any_test.cxx)
target_link_libraries(ConditionVariableAnyTests StormByte)
add_test(NAME ConditionVariableAnyTests COMMAND ConditionVariableAnyTests)

add_executable(ConditionVariableTests condition_variable_test.cxx)
target_link_libraries(ConditionVariableTests StormByte)
add_test(NAME ConditionVariableTests COMMAND ConditionVariableTests)

add_executable(DequeTests deque_test.cxx)
target_link_libraries(DequeTests StormByte)
add_test(NAME DequeTests COMMAND DequeTests)

add_executable(ExclusiveLockTests exclusive_lock_test.cxx)
target_link_libraries(ExclusiveLockTests StormByte)
add_test(NAME ExclusiveLockTests COMMAND ExclusiveLockTests)

add_executable(SafeExceptionTests exception_test.cxx)
target_link_libraries(SafeExceptionTests StormByte)
add_test(NAME SafeExceptionTests COMMAND SafeExceptionTests)

add_executable(FunctionTests function_test.cxx)
target_link_libraries(FunctionTests StormByte)
add_test(NAME FunctionTests COMMAND FunctionTests)

add_executable(HashTests hash_test.cxx)
target_link_libraries(HashTests StormByte)
add_test(NAME HashTests COMMAND HashTests)

add_executable(HeapTests heap_test.cxx)
target_link_libraries(HeapTests StormByte)
add_test(NAME HeapTests COMMAND HeapTests)

add_executable(IterableTests iterable_test.cxx)
target_link_libraries(IterableTests StormByte)
add_test(NAME IterableTests COMMAND IterableTests)

add_executable(ListTests list_test.cxx)
target_link_libraries(ListTests StormByte)
add_test(NAME ListTests COMMAND ListTests)

add_executable(MapTests map_test.cxx)
target_link_libraries(MapTests StormByte)
add_test(NAME MapTests COMMAND MapTests)

add_executable(MutexTests mutex_test.cxx)
target_link_libraries(MutexTests StormByte)
add_test(NAME MutexTests COMMAND MutexTests)

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

add_executable(SetTests set_test.cxx)
target_link_libraries(SetTests StormByte)
add_test(NAME SetTests COMMAND SetTests)

add_executable(SharedLockTests shared_lock_test.cxx)
target_link_libraries(SharedLockTests StormByte)
add_test(NAME SharedLockTests COMMAND SharedLockTests)

add_executable(SharedMutexTests shared_mutex_test.cxx)
target_link_libraries(SharedMutexTests StormByte)
add_test(NAME SharedMutexTests COMMAND SharedMutexTests)

add_executable(StringTests string_test.cxx)
target_link_libraries(StringTests StormByte)
add_test(NAME StringTests COMMAND StringTests)

add_executable(UniqueLockTests unique_lock_test.cxx)
target_link_libraries(UniqueLockTests StormByte)
add_test(NAME UniqueLockTests COMMAND UniqueLockTests)

add_executable(UnorderedMapTests unordered_map_test.cxx)
target_link_libraries(UnorderedMapTests StormByte)
add_test(NAME UnorderedMapTests COMMAND UnorderedMapTests)

add_executable(UnorderedSetTests unordered_set_test.cxx)
target_link_libraries(UnorderedSetTests StormByte)
add_test(NAME UnorderedSetTests COMMAND UnorderedSetTests)

add_executable(VariantTests variant_test.cxx)
target_link_libraries(VariantTests StormByte)
add_test(NAME VariantTests COMMAND VariantTests)

add_executable(VectorTests vector_test.cxx)
target_link_libraries(VectorTests StormByte)
add_test(NAME VectorTests COMMAND VectorTests)

add_executable(WStringTests wstring_test.cxx)
target_link_libraries(WStringTests StormByte)
add_test(NAME WStringTests COMMAND WStringTests)
