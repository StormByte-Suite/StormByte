# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Summary]

StormByte Base is the C++26 foundation of the StormByte suite.

Every other module links this library.
This repository is not Buffer, Config, Crypto, Database, Logger, Multimedia, Network or System — those live in their own repos and depend on Base.

Public headers under `StormByte/` cover exceptions, `Expected`, little-endian `Serializable`, strings, paths, UUID v4, bitmasks, clonable types, `ThreadLock`, `Size`, and `StormByte::Type` concepts.
`StormByte::Safe` is the owned surface that crosses a DLL without the caller's CRT: collections, wait, and `Safe::Thread`.

If you landed here from a release link and have not read the tree:

- What this module is, how to build it, and short examples: [README.md](https://github.com/StormBytePP/StormByte/blob/master/README.md)
- Since 2.0.0 license changed: original source in this repository is dual-licensed, LGPL v3 or later **or** a commercial license from the copyright holder. The grant does not cover other StormByte modules or `thirdparty/`. [LICENSE](https://github.com/StormBytePP/StormByte/blob/master/LICENSE)

## [Unreleased]

[Unreleased]: https://github.com/StormBytePP/StormByte/compare/2.0.0...HEAD

## [2.0.0] - 2026-10-08

### Added

- `AllocationError`, `ExpiredWeakPointerError` and `OperationError`, anchored in Base for cross-DLL catching. The allocation exception uses a static message and does not allocate during construction. `Safe::Heap::RethrowException` preserves StormByte exception types and translates foreign failures. `BadOptionalAccess` and `BadVariantAccess` take their message from the constructor, like `OutOfBoundsError`.
- **`Safe::Optional` value API** — direct/converting construction and assignment from Safe values, enums and `std::optional`, `nullopt`, `value_or`, heterogeneous comparisons, in-place construction, `swap` and qualified monadic operations. Mutable `operator*` uses a callback-backed proxy with copy reads; `operator->` calls const members through a caller-owned snapshot that is valid for the full expression only. No pointer or reference to creator-owned storage crosses the ABI. Storage is on the Base heap.
- **Safe serialization** — `Serializable` now round-trips `Safe::Optional`, sequence/map collections, `Safe::Set`, `Safe::UnorderedSet`, `Safe::Queue` and `Safe::Binary`. Optional shares the `std::optional` wire format, and Queue preserves FIFO order. `Safe::Set` shares the `std::set` wire. A container with a `hasher` (`Safe::UnorderedSet`, `Safe::UnorderedMap`, and the `std` counterparts) writes the count and then the elements sorted by key, so the wire does not depend on the bucket order. Map elements are ordered through pointers: `Pair<const K, V>` is not assignable. Container iterators are serialized as `value_type` snapshots and decoded through Safe insertion operations. Queue decoding rejects counts above 1,048,576 to bound resource use. The corruption cases remain: truncation, a huge size field, bit-flip, byte-overwrite, random corruption and trailing garbage.
- **Safe STL-shaped APIs** — `Safe::Vector` adds insertion, emplacement, removal, resize and capacity requests, and borrows as `std::span`. `Safe::List` is a doubly linked list on the Base heap, with splice, merge, unique, sort and reverse. `Safe::Map` adds comparator-aware ordered bounds, insertion, range erase, stable key-identity iterators, `node_type`, extract and merge. `Safe::Set` is that map with `Monostate` as the mapped type: key-only iterators, `node_type`, extract and merge. The same comparator transfers the node; a different comparator copies the key. `Safe::UnorderedMap` is a hash table on the Base heap, keyed through `Safe::Hash`. `Safe::UnorderedSet` is that table with `Monostate` as the mapped type. The same hash transfers the node; a different hash copies the key. Iterator keys stay const. `Safe::Queue` adds `back`, `emplace`, `swap`, Safe random-access iterators, mutable callback-backed `front` / `back` proxies and `erase` for `<algorithm>` / `std::ranges`, without exposing creator-owned nodes across DLL boundaries. `Safe::String` / `Safe::WString` add common size-changing value modifiers while retaining Base-owned storage. Reusing a moved-from map with a stateful comparator is rejected rather than silently changing key order. Brace initialization works, including `Safe::List<int>{1, 6, 7}`, `Safe::Set<int>{1, 6, 7}`, `Safe::UnorderedSet<int>{1, 6, 7}` and `Safe::Map<Safe::String, int>{{"a", 1}}`.
- **Safe::String / Safe::WString** — UTF-8 and wide owned text, with range, lookup, case conversion and serialization support. Private SDS / wide-buffer storage and SSO keep allocation, mutation and destruction inside Base. Views are copied when retained and preserve their full length, including embedded NUL code units; `Bytes()` exposes a non-owning NUL-terminated pointer. `capacity()` / `reserve(Size)` exclude the trailing NUL, smaller requests never shrink, and value modifiers retain reserved capacity. Literal and `string_view` constructors are implicit. UTF-8 conversion between the two types preserves embedded NULs. `Split` fills a Safe vector and `Explode` fills a Safe queue. Covered by `StringTests` / `WStringTests`.
- **CRT-safe containers** — `Safe::Iterable<Container>` is the consumer cursor. It is not the iterator model of `Safe::Binary`, and it is not an alias of an STL container. `Safe::Optional<T>` is a zero-or-one-element range; `Safe::Queue<T>` is an iterable FIFO. `Safe::Pair<First,Second>` is a SafeValue entry pair, value-initializes both members, supports structured bindings, and copies to/from `std::pair`. Explicit STL imports and force-inline exports keep STL allocations in the caller CRT.
- **Safe::Binary** — owned contiguous sequence of `std::byte`, safe to use across a DLL boundary. Same kind of API as `std::vector<std::byte>` (iterators, `<algorithm>`, `std::ranges`, `std::span`, insert / erase / assign / `append` / `emplace_back` / `operator+=`). Lengths and indices use `StormByte::ByteSize`. Storage is `Safe::Vector<std::byte>` on Base's heap; the public type is not `std::vector<std::byte>`. `at()` throws `OutOfBoundsError`. Construct from `span`, pointer+`ByteSize`, range, initializer list, `string_view`, and a caller-owned `std::vector<std::byte>` (lvalue copies and leaves the vector; rvalue copies onto Base's heap then clears the vector — looks like a move, not a heap steal). Convert out with implicit `span` and `explicit operator std::vector<std::byte>` (`const&` copies and leaves `*this`; `&&` copies onto the caller CRT then clears `*this`). `append(Binary&&)` / `operator+=(Binary&&)` are a real same-heap move when `*this` is empty. Compare equal / unequal / three-way with another `Binary` and with `std::span<const std::byte>` (both operand orders). `HexDump()` and `HexDump(Size columns)` return a `Safe::String`: 8-digit offset, hex row, ASCII (non-printable as `.`). `columns` is a row width, not a byte length. `0` prints every byte on one line. Default `HexDump()` is 16 columns. `Type::Container` / `Type::Sized` / `Type::HasPushBack` / `Type::ByteInputRange` match; `Type::String` does not. `Serializable<Binary>` uses the container path (same wire as `std::vector<std::byte>`: `uint64` LE count + payload). Covered by `BinaryTests` and `SerializableTests`.
- **Safe::Variant and Safe::Monostate** — a variant stored on the Base heap, not a `std::variant` member. A move leaves the source valueless. `emplace` builds the replacement first, so a throw keeps the previous alternative. `get`, `get_if`, `visit` and `holds_alternative` are found by argument lookup; `std::get` and `std::visit` do not accept this type. Conversion to `std::variant` is explicit and does not steal. Covered by `VariantTests`.
- **Safe::Hash** — cross-module FNV-1a. Integral, enumeration, floating-point and pointer keys are closed in `hash.hxx`. `String`, `WString`, `Binary`, `Size`, `ByteSize`, `Pair`, `Optional`, `Variant` and `Monostate` are closed in their own headers. A type without a specialization is not a key of `UnorderedMap` or `UnorderedSet`. The call does not throw. `std::hash` specializations delegate to it. `long double` hashes its 80-bit payload, not the padding of the object representation. Covered by `HashTests`.
- **Safe::Heap** — public allocation surface (`Allocate` / `Free`) used by the Safe types. The object and the `shared_ptr` control block are allocated here. `AtomicShared` default-constructs its flag.
- **Safe wait** — `Safe::Mutex`, `Safe::UniqueLock`, `Safe::ConditionVariable` and `Safe::Atomic<T>`. Std-like to use, not an alias of the STL and not binary-compatible with it. The gate, the signal and the word live on Base's heap. Public headers do not include `<mutex>`, `<condition_variable>` or `<atomic>`. `Mutex` is not recursive: `lock`, `try_lock`, `unlock`. `unlock` without ownership is undefined. `UniqueLock` is header-only. It can drop and retake that mutex (`defer_lock`, `try_to_lock`, `adopt_lock`, `release`, `swap`). There is no `LockGuard`. `ConditionVariable` accepts only `UniqueLock`: `wait`, `wait_for`, `wait_until`, `notify_one`, `notify_all`. A spurious wake is valid. Predicate overloads stay in the header, so the callable never enters Base. Timed waits convert the clock in the caller and cross nanoseconds. The result is `Safe::CvStatus`, not `std::cv_status`. `Atomic<T>` is a one, two, four or eight byte word: integer, `bool`, enum or object pointer. `load`, `store`, `exchange`, `compare_exchange_weak`, `compare_exchange_strong`, `wait`, `notify_one`, `notify_all` and `is_lock_free`. Integral `T` also has `fetch_add`, `fetch_sub`, `fetch_and`, `fetch_or`, `fetch_xor` and the matching operators. An object pointer adds and subtracts elements. `void*` does not. Orders are `Safe::MemoryOrder`. A release or acq-rel failure order is promoted to acquire. `wait(captured)` has no predicate. `shared_mutex`, `once_flag` and `future` do not cross and are not added. `ThreadLock` stays the reentrant lock. Covered by `MutexTests`, `UniqueLockTests`, `ConditionVariableTests` and `AtomicTests`.
- **Safe::Thread** — an execution owned by Base, not an alias of `std::thread`. The native thread is started and joined in Base. Public headers do not include `<thread>`. The handle is an opaque pointer. `native_handle()` is `uintptr_t`, not a CRT thread type. `Id` is a `uint64_t` (zero is not an execution) with the six comparisons and `Value()`. Destroying a joinable thread terminates the process. Move leaves the source not joinable. `join` and `detach` on a thread that is not joinable throw `Safe::Exception`. The callable constructor is a header template: the decayed callable and its arguments are placed on Base's heap and the entry runs in the creator module. A void `Safe::Function` has its own constructor, stores the function by value and calls `Call`. `Status` stays in the execution. A Base exception does not escape it. `this_thread::get_id`, `yield`, `sleep_for` and `sleep_until` match the `std::this_thread` names. Timed sleep crosses nanoseconds. `Thread::Calling()` is the same identifier as `this_thread::get_id`. `shared_mutex`, `once_flag` and `future` still do not cross. Covered by `ThreadTests`.
- **Telemetry** and **Clock** — base session telemetry (`Telemetry`) with pure `operator Safe::String` and a protected named clock drawer (`Clock`) backed by a thread-safe PIMPL store (`Safe::Unique<Store>`). Leaves derive and add their own domain metrics and counters. `Clock::Measure()` and `Telemetry::MeasureClock(name)` return move-only sample tokens with independent start times; concurrent and nested samples aggregate safely without per-thread clock names or external Start/Stop locks. `Clock::GetValues()` returns a coherent Count/Time/MeanDuration snapshot. Covered by `TelemetryTests`.
- **`STORMBYTE_PUBLIC_TYPE`** — in `visibility.h`. Default visibility on ELF / Mach-O, empty on Windows. Put on header-only types (templates included) whose `typeinfo` and vtable every module emits, so `typeid` / `dynamic_cast` agree across DLLs (on ELF the loader also merges the copies) even when a consumer builds with `-fvisibility=hidden`. Windows needs nothing: MSVC compares RTTI by name and implicitly exports the base specializations of an exported class.
- **`STORMBYTE_FORCE_INLINE`** — in `platform.h`. `inline` plus `__forceinline` or `always_inline`, so the body is emitted in the caller. `inline` alone is only a hint.
- **Safe DLL-boundary contracts and typed callbacks** —
- `Type::IsSafe` is the Base-backed classification; `Type::MaybeSafe` is a recursive conditional classification. Consumer types opt in through `STORMBYTE_DECLARE_MAYBE_SAFE`; known incompatible STL types remain rejected.
- `Safe::Owner` publicly exposes creator-module clone/destruction for opaque state, with explicit MaybeSafe requirements.
- `Safe::Callback` and both `Safe::Function<Signature>` specializations are copyable as well as movable when supplied with a provider-module `noexcept Clone` callback. Copies own independent contexts; failed clones throw `StormByte::Exception`. Typed functions preserve that exception and report other callback exceptions as `Safe::Status::Failure`.
- **`Safe::Shared<T>` collection values** — admitted to Safe collections and wrappers. This does not certify the pointee's ABI or lifetime; `Safe::Unique<T>` remains excluded because collection reads require copyable values.
- **Error** — `Domain`, `Category`, `Code` (`Success`, `Unknown`) and `Fault`. Modules specialize `Domain` for their enums; `make_error_code` lives next to the enum so ADL feeds `std::error_code`. Category singletons stay in the module `.cxx`. `Fault` holds the code and a `Safe::String`. Not thrown. Covered by `ErrorTests`.
- **Shared** — `StormByte::Safe::Shared`, complements `std::shared_ptr` (`StormByte/safe/pointers.hxx`). It does not replace it. Exact type: `Safe::Heap::MakeShared`. Derived type: `Shared<Base>::MakePointer<Derived>` (the deleter still destroys the derived object). Daily operations match `std::shared_ptr` (copy, `reset`, `swap`, compare, `use_count`, `owner_before`). No constructor from a raw pointer or from `std::shared_ptr`, and no `release`. Converts implicitly to `std::shared_ptr<T>` (same control block, deleter stays Base). No conversion back. `StaticPointerCast`, `DynamicPointerCast`, `ConstPointerCast` and `ReinterpretPointerCast` keep that control block. Covered by `PointerTests`.
- **Unique** — `StormByte::Safe::Unique`, complements `std::unique_ptr` (`StormByte/safe/pointers.hxx`). It does not replace it. Exact type: `Safe::Heap::MakeUnique`. Derived type: `Unique<Base>::MakePointer<Derived>`, and `~Base` must be virtual or the call does not compile. No `release` and no constructor from a raw pointer. Converts on move to `std::unique_ptr<T, Safe::Heap::ObjectDeleter>` only. A `std::unique_ptr<T>` parameter has to change. Covered by `PointerTests`.
- **Weak** — `StormByte::Safe::Weak`, complements `std::weak_ptr`. Constructed only from `Shared`. `lock` returns `Shared`, or empty when expired. No constructor from `std::weak_ptr`. Covered by `PointerTests`.
- **ClonableTests** — clone / move / `MakePointer` coverage for `Shared` and `Unique`. `Derived::PointerType` used as storage. Implicit conversion to `std::shared_ptr` / `unique_ptr` with Base deleter. `ValidSmartPointer` rejects `std::shared_ptr` and default `std::unique_ptr`. A hidden-visibility shared library (`ClonablePlugin`) derives from `Clonable`; the tests check one shared `typeid`, `dynamic_cast` and `Clone` / `Move` across that boundary.
- **Size** — abstract unit count (`uint64_t` storage), same width on every host and safe across a DLL. Not an octet length. Implicit conversion only to `std::size_t` (clamp to `size_t::max`); every other integral destination is `explicit` and clamps to `T::max`. No `Value()`, no `operator bool`. Full arithmetic with any `Type::Integral` and with another `Size`; the result is always `Size`. Mixed `==` / `<=>` with any integral and with `ByteSize` are hidden friends so `n == 5` is not ambiguous. A negative integer operand is undefined and `assert`s when assertions are on. Overflow wraps in release. `operator Safe::String` / `operator Safe::WString` print the raw count (`atoi` style). Explicit instantiations of the templates live in the StormByte DLL. Covered by `SizeTests`.
- **ByteSize** — octet length (`uint64_t` storage), same width on every host and safe across a DLL. Implicit conversion only to `std::size_t` (clamp); every other integral destination is `explicit`. No `Value()`, no `operator bool`. Area products (`ByteSize * ByteSize`) are deleted. Scaling by `Size` or by an integer is allowed and yields `ByteSize`. IEC factories and free constants (`B`, `KiB`, `MiB`, `GiB`, `TiB`, `PiB`, `EiB`) so `1 * KiB` and `ByteSize::KiB(1)` are 1024 octets. SI (`KB`…`EB`) likewise. `operator Safe::String` / `operator Safe::WString` print IEC text (`0 B`, `1023 B`, `1.00 KiB`, `1.50 MiB`). Explicit instantiations live in the StormByte DLL. Covered by `ByteSizeTests`.
- **sds** — vendored under `thirdparty/sds` and compiled into StormByte. The fork supplies `ssize_t` for the Windows CRT. It is not a separate installed library.

### Changed

- **Breaking: Iterable and Safe collections** — removed reference-exposing `StormByte::Iterable`; all public collection ranges now use opaque creator-module storage and callback-backed iterators/proxies. Mutable sequence proxies support classic `<algorithm>` and `std::ranges` without exposing owner-module references. Rvalue imports move elements and leave their STL source valid and empty without adopting its allocator or allocation.
- **Breaking: BinaryData** — renamed to `Safe::Binary`. The private `std::vector` backend is gone; bytes live in `Safe::Vector<std::byte>`. `append_vector` is removed. A `using StormByte::Safe::Binary` covers the old name. The public contract is otherwise unchanged.
- **Mutable text ranges** — `Safe::String` / `Safe::WString` expose mutable `data`, iterators, reverse iterators, indexing, `front` and `back` for in-place algorithms over existing code units, including embedded NULs, while preserving the trailing NUL contract.
- **Text API migration** — Base text-producing APIs (`Size`, `ByteSize`, UUID, Base64, `Binary`, `Telemetry`) now expose `Safe::String` / `Safe::WString`. Error messages and exceptions store `Safe::String`; exception plain-text constructors accept `std::string_view` or `const Safe::String&` and copy the text. Text serialization retains the standard string wire format. `GenerateUUIDv4` and both `Base64Encode` overloads are `StormByte::` symbols. `Base64Decode` returns `Safe::Binary` and still takes `std::string_view`.
- Shared vs static follows CMake `BUILD_SHARED_LIBS`. There is no `STORMBYTE_SHARED` CMake option. When the library is shared, the compile definition `STORMBYTE_SHARED` is still set so `visibility.h` can distinguish `dllexport` / `dllimport` / static.
- **License change** — original source in this repository is dual-licensed: GNU LGPL v3 or later, or a commercial license from the copyright holder. The grant applies only to original StormByte source in this repository. It does not cover other StormByte modules or third-party material (including `thirdparty/`). No patent rights are granted.
- **Visibility** — `STORMBYTE_PUBLIC` comes first on a function declaration (`STORMBYTE_PUBLIC Safe::String Foo();`). clang-cl rejects `__declspec` after a reference return. `class STORMBYTE_PUBLIC` stays on the type. Exported templates are declared `extern template STORMBYTE_PUBLIC` / `extern template class STORMBYTE_PUBLIC` in the header and instantiated in the `.cxx` with `STORMBYTE_INSTANTIATE` (`dllexport` on Windows, empty on ELF so GCC does not warn `-Wattributes`). The `extern` line in the header is what ELF uses to export; do not put `STORMBYTE_INSTANTIATE` on that line.
- **Breaking**: **Exception.** `StormByte::Component` is gone. `what()` is `StormByte: <message>`, or `StormByte.<path>: <message>` when a parent passes the segments under `StormByte` (`Crypto.Crypter`), joined with `.`. A parent passes `Exception::Path` (a `std::string_view` of its segments) and forwards the format and the arguments. A bare string is not a path, because that is ambiguous with the format constructor. It does not format. `Exception` calls `std::format` in the caller's translation unit and copies a `const char*` into a `Safe::String`. The view is not stored and no `std::string` enters the DLL. A final leaf inherits the parent constructors and adds no segment. A literal with no arguments is the message as-is. `DeserializeError`, `OutOfBoundsError` and `Base64Error` are leaves of the root (`StormByte: …`). Each of those types, and the root, defines its destructor in Base's `.cxx` so the `typeinfo` is unique across a DLL. Named types in other modules do the same. Safe exceptions have their own path and `typeinfo`.
- **Expected** — the error is a `Safe::Shared<E>`, not a `std::shared_ptr` built with `std::make_shared`. `Unexpected` uses `Safe::Heap::MakeShared`, or `Safe::Shared::MakePointer` for a derived error. The call stays `Unexpected<E>("… {}", args…)`. Formatting runs in the caller; `E` is constructed from the resulting string. `Unexpected(result.error())` forwards that `Shared` and does not allocate. `Shared<E>` converts to `std::shared_ptr<E>`. A `std::shared_ptr` is not accepted. `Expected<T, E>` is still `std::expected`. Covered by `ExpectedTests`.
- **Breaking:** `Clonable` — moves to `StormByte::Safe::Clonable` (`StormByte/safe/clonable.hxx`; `StormByte/clonable.hxx` is gone). Polymorphic `Clone` / `Move`. It is not an owner: `MakePointer` forwards to `Shared::MakePointer` or `Unique::MakePointer`, which allocate the object and the `shared_ptr` control block on Base's heap. `Clonable<T>` is `Clonable<T, Shared<T>>`. Unique ownership is `Clonable<T, Unique<T>>`. `Clonable<T, std::shared_ptr<T>>` and `Clonable<T, std::unique_ptr<T>>` no longer compile. `PointerType` is `Shared<T>` / `Unique<T>`; overrides that already return `PointerType` from `MakePointer` do not change. After construction, `Shared` converts implicitly to `std::shared_ptr<T>` so existing `shared_ptr` call sites keep working. `Unique` does not convert to `std::unique_ptr<T>`. `ValidSmartPointer` uses `Type::SameAs`. A class in another DLL may derive from `Clonable`: `Clonable`, `Shared`, `Unique`, `Weak`, `Heap::ObjectDeleter` and `Heap::Allocator` carry `STORMBYTE_PUBLIC_TYPE`, so their per-module `typeinfo` and vtables merge instead of staying hidden in each DLL. Covered by `ClonableTests`.
- **Breaking:** **`StormByte::Safe`** — `Shared`, `Unique`, `Weak`, the pointer casts, `Heap` and `Clonable` / `ValidSmartPointer` live in `StormByte::Safe`, under `StormByte/safe/` (`pointers.hxx`, `clonable.hxx`, `heap.hxx`). `StormByte/safe_pointers.hxx` and `StormByte/clonable.hxx` are gone. No aliases are kept in `StormByte`. There is no `Safe` alias of the STL on non-Windows hosts. Passing and destroying a Safe value across libstdc++ and libc++ is supported. Catching a foreign exception is not.
- **Type::String** — also matches `StormByte::Safe::String` and `StormByte::Safe::WString`. They stay out of `Type::Container`.
- **Type::Sized** — `size()` may be implicitly convertible to `std::size_t` (STL containers) **or** be `StormByte::Size` / `StormByte::ByteSize`. Covered by `TypeTraitsTests`.
- **Type::Numeral** — also matches `StormByte::Size` and `StormByte::ByteSize`.
- **Type::Array** — StormByte flavor of a fixed-size sequence; `Serializable` uses it instead of stock `std::is_same` / `tuple_size` probes. `Binary` is not an array.
- **Size vs ByteSize everywhere** — public Base APIs no longer take or return a raw `std::size_t` / `std::uint64_t` when the value is a count. Character counts (`Safe::String::size`, `Safe::WString::size`, subscripts) are `Size`. Octet counts (`Binary::size` / `operator[]` / ctors / `Serializable<T>::Size`, wire lengths) are `ByteSize`. Mixed arithmetic and comparison stay typed: a `Size` result stays a `Size`; a `ByteSize` result stays a `ByteSize`.
- **Serializable** — container and string codecs use `ByteSize` for the on-wire length. `Safe::String` / `Safe::WString` codecs preserve embedded NULs; wide text is encoded as UTF-8 on the wire. Concepts used to branch (`Type::Array`, `Type::Container`, `Type::String`, `Type::Numeral`) are StormByte flavor, not stock `std::*` traits.
- **Tests** — one executable per public component, under `test/`, `test/safe/` and `test/type_traits/`. `ASSERT_*` no longer takes the function name. UUID, Base64, Bitmask, Clonable, Exception, Expected, ThreadLock, Iterable, Serializable, Pointers, TypeTraits, Size, ByteSize, Binary, List, Map, Set, UnorderedMap, UnorderedSet, Hash, Variant, Mutex, UniqueLock, ConditionVariable, Atomic and Thread cover the public surface, including `<algorithm>`.

### Removed

- **Old root String helpers** (`StormByte::String` in the former Base API) — replaced by the owned `StormByte::Safe::String` / `WString` types in Base.
- **System** (`StormByte::System` in Base: `TempFileName`, `CurrentPath`, `ExecutablePath`, `Sleep`) — leaves Base. Absorbed by the existing StormByte-System module.
- **UTF8Error** / **SystemError** — leave Base with String and System. Base retains `DeserializeError`, `OutOfBoundsError` and `Base64Error`, and adds `AllocationError`, `ExpiredWeakPointerError`, `OperationError`, `BadOptionalAccess` and `BadVariantAccess`.
- **CoreApiTests** — coverage lives in `ClonableTests` and `ErrorTests`.

### Fixed

- **`FindStormByte`.** `String` is no longer a package component; Base now provides the owned text types. `Buffer` pulls `Logger` and `System`; `Database` pulls `Logger`. `Crypto`, `Multimedia` and `Network` name only `Buffer`; the closure finds `Logger` and `System`. `Config` links only the core.
- **`ByteSize` / `Size`.** The integer constructors stay in the header. GCC does not emit a `constexpr` constructor that is both an `extern template` and an explicit instantiation, so `ByteSize(unsigned long long)` was missing from the shared library. The operators are still one copy in the DLL. `++` / `--` build the step with the private constructor, so they do not instantiate `unsigned int` early.
- **`Size` / `ByteSize` to `std::string`.** The conversion calls `operator Safe::String()` by name. On GCC, `static_cast<Safe::String>` picks `Safe::String(std::string)` and that calls the same operator again until the stack dies.
- **`ByteSize` wide text.** Its wide form uses the UTF-8 conversion between `Safe::String` and `Safe::WString`. `swprintf` and `%s` is a narrow string on glibc and a wide string on the Windows CRT, so `1.00 KiB` did not match.
- **Caller containers.** `Safe::String::operator std::string`, `Safe::WString::operator std::wstring`, `Size` / `ByteSize` `operator std::string`, their `operator<<`, and `Binary::operator std::vector` are `STORMBYTE_FORCE_INLINE`. On an exported class, `inline` can still be a call into the DLL, so the container was allocated here and freed by the caller. The copy runs through `span()` in the caller, then `clear()` on Base's heap.

[2.0.0]: https://github.com/StormBytePP/StormByte/compare/1.2.0...2.0.0

## [1.2.0] - 2026-09-17

### Added

- `String` helpers that only read the input now take `std::string_view` /
`std::wstring_view`: `IsNumeric`, `ToLower`, `ToUpper`, `Explode`, `Split`,
`UTF8Encode`, `UTF8Decode`, `SanitizeNewlines`, `ToByteVector`,
`RemoveWhitespace`, `IsInteger`. `std::string`, `std::wstring` and
literals convert to the views.
- `Base64Decode(std::string_view)` replaces `Base64Decode(const std::string&)`.

### Changed

- `SanitizeNewlines` no longer builds a throwaway copy and a `std::regex`;
it walks the view and maps `\r\n` to `\n`.

[1.2.0]: https://github.com/StormBytePP/StormByte/compare/1.1.1...1.2.0

## [1.1.1] - 2026-09-15

### Added

- **Type** category concepts — `LvalueReference` and `RvalueReference` (`std::is_lvalue_reference` / `std::is_rvalue_reference`), next to the existing `Reference`.
- **SystemError** — `Exception` specialization for `StormByte::System` helpers.

### Changed

- **System::ExecutablePath** — throws `SystemError` instead of returning `"NOPATH"`. Paths longer than 256 bytes are resolved.

### Fixed

- **Type::CopyConstructible / CopyAssignable** — no longer wrap `std::is_copy_*` alone. They require a real `T{src}` / `dest = src` and recurse into `value_type` only when `T` is a `Type::Container` and not a `Type::View`. `std::span<std::unique_ptr<T>>` stays copyable; `std::vector<std::unique_ptr<T>>` does not.
- **Type::HasPushBack / HasPushFront / HasInsert** — accept `value_type&&` as well as `const value_type&`. `std::vector<std::unique_ptr<T>>` is now `HasPushBack`, so `Iterable::add` takes the `push_back` path.
- **Iterable::add** — one forwarding `add(T&&)` plus a by-value sink for braced-init (`add({"k", v})`). The sink writes to the container; it does not call `add` again (that recursed until a stack overflow). clang-cl / MSVC no longer instantiate `push_back(const unique_ptr&)`.
- **Iterable** container constructor — single `Iterable(C&&)` constrained to `Container`. An lvalue copy requires `Type::CopyConstructible` on `value_type`; a move requires `Type::MoveConstructible`.
- **Iterable** copy / move — defaulted copy and copy-assign require `Type::CopyConstructible` / `Type::CopyAssignable` on `Container`; move and move-assign require `Type::MoveConstructible` / `Type::MoveAssignable`. `Iterable<std::vector<std::unique_ptr<T>>>` is move-only.
- **Iterable::Iterator / ConstIterator** — arithmetic (`+=`, `-=`, `+`, difference) uses `std::advance` / `std::distance` instead of `m_it += n`, so it compiles on bidirectional containers (`list`, `map`, `set`). `reference` / `pointer` come from `std::iterator_traits` of the wrapped iterator, not `Container::reference` (`*it` on `std::set` is `const T&`).
- **System::TempFileName** — throws `SystemError` instead of `std::runtime_error`. The file is created and left on disk; the caller unlinks it. Windows still only uses the first three characters of `prefix`.
- **System::ExecutablePath** — grows the platform buffer instead of truncating at 256 bytes (`GetModuleFileNameW` / `readlink`). Failure is `SystemError`, not a fake path.
- **System::CurrentPath** — wraps `std::filesystem::filesystem_error` in `SystemError` instead of leaking a standard exception.
- **Iterable copy / copy-assign** — user-provided members with `if constexpr` on `Type::CopyConstructible` / `CopyAssignable`. clang-cl / MSVC `dllexport` of a derived class no longer instantiates `vector<unique_ptr<T>>` copy via `requires = default`.

[1.1.1]: https://github.com/StormBytePP/StormByte/compare/1.1.0...1.1.1

## [1.1.0] - 2026-09-13

### Added

- **Component** — wrapper for the module name on the component-prefixed `Exception` constructor. Call sites must now write `Exception(Component("Base64"), "…", args...)`.
- **Type** concepts — `Sized`, `SmartPointer`, `Swappable`, `DerivedFrom`, `EqualityComparable`, `ThreeWayComparable`, `Hashable`.
- **Type** range and pointer concepts — range/iterator category wrappers, range value/reference/difference aliases, `ExplicitlyConvertibleTo`, `NullablePointer`, `ByteInputRange`, and `ByteInputIterator` for public generic APIs.
- **Type** object-semantics concepts — `TriviallyDefaultConstructible`, `TriviallyCopyConstructible`, `CopyAssignable`, `TriviallyCopyAssignable`, `TriviallyMoveConstructible`, `MoveAssignable`, `TriviallyMoveAssignable`, `Copyable`, `Movable`, rounding out the existing `CopyConstructible`/`MoveConstructible`/`TriviallyCopyable`/`TriviallyDestructible`/`Swappable` group.
- **Test handlers** — assertions evaluate operands once and report non-streamable values safely. New macros: `ASSERT_THROWS`, `ASSERT_NO_THROW`, `ASSERT_NEAR`, `ASSERT_CONTAINS`, `ASSERT_NOT_NULL`, covered by `TestHandlersTests`.
- **UTF8Error** — custom exception for invalid UTF-8 and wide-string Unicode input.

### Changed

- **Exception(component, fmt, args...)** — first argument is `Component`, not `std::string`. Source-breaking for every call that used the old two-string form.
- **Type** concepts — reorganized `type_traits.hxx` into focused container, wrapper, conversion, category, range, relation, and comparison groups without changing public contracts.
- **Template implementation split** — `Bitmask`, `Clonable`, `Iterable` (including its nested `Iterator`/`ConstIterator`), and `Serializable` now declare their members in the `.hxx` header and define them in a matching `.txx` file included at the bottom of the header. Public API and behavior are unchanged; `.txx` files are installed alongside the headers.
- **Serializable<T> explicit instantiation** — `bool`, all standard character/integer/floating-point types, and the four Codec-backed string types (`std::string`, `std::wstring`, `std::u16string`, `std::u32string`) are now explicitly instantiated inside the shared/dynamic library and declared `extern template` in the header, so consumers link against the library's code instead of re-instantiating it locally.

### Removed

- **`StormByte::swap_endian`** — the transitional alias of `Type::Detail::swap_endian` in the root `StormByte` namespace. Call `StormByte::Type::Detail::swap_endian` directly instead.

### Fixed

- **Exception(component, fmt, args...)** — `Exception(fmt, args...)` was chosen for every call with 2+ arguments, so the component string became the whole message and the format/args were discarded. `Component` makes the prefixed overload the only viable candidate.
- **Exception(format_string, Args...)** — the zero-argument path called `copy_str(fmt)`, which did not compile when that branch was selected (e.g. construction from `std::string_view`). It now copies `fmt.get()`, matching the documented “message as-is” behaviour.
- **Iterable::add** — `add(const value_type&)` requires `Type::CopyConstructible`; `add(value_type&&)` requires `Type::MoveConstructible`. clang-cl / MSVC no longer instantiate `push_back(const unique_ptr&)`.
- **Iterable::Iterator / ConstIterator** — `iterator_category` is taken from the underlying container iterator instead of being hardcoded as `random_access_iterator_tag`. `std::distance` / `std::advance` compile on `Iterable<std::list<…>>`, `map`, `set`, etc.
- **System::TempFileName** — builds the `mkstemp` template in a `std::string` sized to the prefix instead of a 256-byte buffer, so a long prefix no longer truncates the trailing `XXXXXX`.
- **Serializable<std::array>** — deserializes elements by index and rejects a serialized count that does not match the array extent.
- **Serializable constrained helpers** — container/pair/optional/trivial private helpers are member templates (`template<typename U = T> requires Type::* <U>`) so `template class Serializable<bool>` / `char` no longer instantiate bodies that call `.size()` on a scalar (clang-cl / clang++).
- **Unexpected<Base>(Derived)** — no longer conflicts with `Unexpected<E>(error)` when `Base` and `Derived` are the same type.
- **String** — `ToLower` / `ToUpper` no longer pass negative signed values to the C character functions; negative byte sizes keep their sign instead of wrapping.
- **String UTF-8 conversion** — wide-string conversion is now locale-independent and rejects malformed Unicode input with `UTF8Error`.

[1.1.0]: https://github.com/StormBytePP/StormByte/compare/1.0.0...1.1.0

## [1.0.0] - 2026-09-05

Initial public release of StormByte Base.

### Added

- **Exception** hierarchy with DLL-safe `const char*` storage and `std::format` support
- **Expected<T, E>** on `std::expected` with reference support and shared error ownership (`Unexpected` helpers)
- **Serializable** template
- Trivially copyable types, STL containers, `std::pair` and `std::optional`
- `Detail::Codec` for `std::string`, `std::wstring`, `std::u16string` and `std::u32string`
- Zero-copy input via `std::span<const std::byte>`
- Little-endian on the wire
- **Base64** encode / decode (standard alphabet, whitespace-tolerant decoder)
- **Bitmask** CRTP for unsigned flag enums (`|`, `&`, `^`, `~`, `Add`, `Remove`, `Has`, `HasAny`, `HasNone`)
- **Clonable** for smart-pointer clone / move (`std::shared_ptr` / `std::unique_ptr`)
- **ThreadLock** — owner-tracked lock; the owner may reenter; `Unlock` from a non-owner is a no-op
- **String** utilities
- Case conversion (`ToLower` / `ToUpper`)
- Splitting (`Explode`, `Split`)
- Human-readable number and byte-size formatting
- UTF-8 ↔ wide string conversion
- Byte vector ↔ string conversion
- Whitespace removal, newline sanitization, integer detection
- **System** utilities
- `TempFileName()`
- `CurrentPath()` (process cwd)
- `ExecutablePath()` (directory of the running executable)
- `Sleep()` for any `std::chrono::duration`
- **UUID** generation (`GenerateUUIDv4()` — RFC 4122 version 4)
- **Type** concepts under `StormByte::Type` (`String`, `Container`, `Optional`, `Pair`, enums, …)
- Platform macros (`WINDOWS`, `LINUX`, `MACOS`, `UNIX`, `BIT32` / `BIT64`)
- Compiler macros: `CLANG` (including clang-cl), `GCC`, `MSVC` (MSVC only; not clang-cl)
- Visibility macros for shared builds
- Unit tests, including serialization robustness

### Changed

- Serialization size fields on the wire are `std::uint64_t` (8 bytes). 32-bit and 64-bit hosts interchange.
- `System::CurrentPath()` is the process cwd. The old meaning is `ExecutablePath()`.

### Fixed

- `Base64Decode` documentation matches the lenient decoder (whitespace ignored, stops at first `=`, padding accepted but not strictly checked).
- `CurrentPath()` documentation matches the implementation.

### Notes

- First stable release of StormByte Base.
- The binary serialization layout is stable.
- Needs a C++26 compiler and CMake ≥ 3.28.
- Foundation for Buffer, Config, Crypto, Database, Logger, Multimedia, Network and System.

[1.0.0]: https://github.com/StormBytePP/StormByte/releases/tag/1.0.0
