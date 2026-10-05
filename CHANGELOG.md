# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Summary]

StormByte Base is the C++26 foundation of the StormByte suite.

Every other module links this library.
This repository is not Buffer, Config, Crypto, Database, Logger, Multimedia, Network or System — those live in their own repos and depend on Base.

Public headers under `StormByte/` cover exceptions, `Expected`, little-endian `Serializable`, strings, paths, UUID v4, bitmasks, clonable types, `ThreadLock`, `Size`, and `StormByte::Type` concepts.

If you landed here from a release link and have not read the tree:

- What this module is, how to build it, and short examples: [README.md](https://github.com/StormBytePP/StormByte/blob/master/README.md)
- Since 2.0.0 license changed: original source in this repository is dual-licensed, LGPL v3 or later **or** a commercial license from the copyright holder. The grant does not cover other StormByte modules or `thirdparty/`. [LICENSE](https://github.com/StormBytePP/StormByte/blob/master/LICENSE)

## [Unreleased]

[Unreleased]: https://github.com/StormBytePP/StormByte/compare/2.0.0...HEAD

## [2.0.0] - 2026-10-05

### Added

- `AllocationError`, `ExpiredWeakPointerError` and `OperationError`, anchored in Base for cross-DLL catching. The allocation exception uses a static message and does not allocate during construction. `Safe::Heap::RethrowException` preserves StormByte exception types and translates foreign failures.

- **`Safe::Optional` value API** — direct/converting construction and assignment from Safe values, enums and `std::optional`, `nullopt`, `value_or`, heterogeneous comparisons, in-place construction, `swap` and qualified monadic operations. Mutable `operator*` uses a callback-backed proxy with copy reads; `operator->` calls const members through a caller-owned snapshot that is valid for the full expression only. No pointer or reference to creator-owned storage crosses the ABI.
- **Safe serialization** — `Serializable` now round-trips `Safe::Optional`, sequence/map collections and `Safe::Queue`; Optional shares the `std::optional` wire format, and Queue preserves FIFO order. Container iterators are serialized as `value_type` snapshots and decoded through Safe insertion operations. Queue decoding rejects counts above 1,048,576 to bound resource use.
- **Safe STL-shaped APIs** — `Safe::Vector` adds insertion, emplacement, removal, resize and capacity requests; `Safe::Map` adds comparator-aware ordered bounds, insertion, range erase, stable key-identity iterators and arrow proxies; `Safe::Queue` adds `back`, `emplace`, `swap`, Safe random-access iterators, mutable callback-backed `front` / `back` proxies and `erase` for `<algorithm>` / `std::ranges`, without exposing creator-owned nodes across DLL boundaries. `Safe::String` / `Safe::WString` add common size-changing value modifiers while retaining Base-owned storage. Reusing a moved-from map with a stateful comparator is rejected rather than silently changing key order.
- **Safe::String / Safe::WString** — UTF-8 and wide owned text integrated from StormByte-String into Base, with range, lookup, case conversion and serialization support. Private PIMPL storage keeps `std::string` / `std::wstring` allocation, mutation and destruction inside Base. Views are copied when retained and preserve their full length, including embedded NUL code units; `Bytes()` exposes a non-owning NUL-terminated pointer. `capacity()` / `reserve(Size)` exclude the trailing NUL, smaller requests never shrink, and value modifiers retain reserved capacity. UTF-8 conversion between the two types preserves embedded NULs. Covered by `StringTests` / `WStringTests`.
- **CRT-safe containers** — `Safe::Iterable<Container>` owns sequence and ordered-map storage behind creator-module callbacks. `Safe::Vector<T>` and `Safe::Map<K,V>` are aliases of its sequence and map specializations. `Safe::Optional<T>` is a zero-or-one-element range; `Safe::Queue<T>` is an iterable FIFO backed by callback-owned deque storage. `Safe::Pair<First,Second>` is a SafeValue entry pair, supports structured bindings, and copies to/from `std::pair`. Explicit STL imports and force-inline exports keep STL allocations in the caller CRT. `Safe::String::Split` / `Safe::WString::Split` fill a Safe vector, and `Explode` fills a Safe queue.
- **Telemetry** and **Clock** — base session telemetry (`Telemetry`) with pure `operator Safe::String` and a protected named clock drawer (`Clock`) backed by a thread-safe PIMPL store (`Safe::Unique<Store>`). Leaves derive and add their own domain metrics and counters. `Clock::Measure()` and `Telemetry::MeasureClock(name)` return move-only sample tokens with independent start times; concurrent and nested samples aggregate safely without per-thread clock names or external Start/Stop locks. `Clock::GetValues()` returns a coherent Count/Time/MeanDuration snapshot. Covered by `TelemetryTests`.
- **`STORMBYTE_PUBLIC_TYPE`** — in `visibility.h`. Default visibility on ELF / Mach-O, empty on Windows. Put on header-only types (templates included) whose `typeinfo` and vtable every module emits, so `typeid` / `dynamic_cast` agree across DLLs (on ELF the loader also merges the copies) even when a consumer builds with `-fvisibility=hidden`. Windows needs nothing: MSVC compares RTTI by name and implicitly exports the base specializations of an exported class.
- **`STORMBYTE_FORCE_INLINE`** — in `platform.h`. `inline` plus `__forceinline` or `always_inline`, so the body is emitted in the caller. `inline` alone is only a hint.
- **Safe DLL-boundary contracts and typed callbacks** —
  - `Type::IsSafe` is the Base-backed classification; `Type::MaybeSafe` is a recursive conditional classification. Consumer types opt in through `STORMBYTE_DECLARE_MAYBE_SAFE`; known incompatible STL types remain rejected.
  - `Safe::Owner` publicly exposes creator-module clone/destruction for opaque state, with explicit MaybeSafe requirements.
  - `Safe::Callback` and both `Safe::Function<Signature>` specializations are copyable as well as movable when supplied with a provider-module `noexcept Clone` callback. Copies own independent contexts; failed clones throw `StormByte::Exception`. Typed functions preserve that exception and report other callback exceptions as `Safe::Status::Failure`.
- **`Safe::Shared<T>` collection values** — admitted to Safe collections and wrappers. This does not certify the pointee's ABI or lifetime; `Safe::Unique<T>` remains excluded because collection reads require copyable values.
- **BinaryData** — owned contiguous sequence of `std::byte`, safe to use across a DLL boundary. Same kind of API as `std::vector<std::byte>` (iterators, `<algorithm>`, `std::ranges`, `std::span`, insert / erase / assign / `append` / `emplace_back` / `operator+=`). Lengths and indices use `StormByte::ByteSize`. Storage lives on Base's heap; the public type is not `std::vector<std::byte>`. `at()` throws `OutOfBoundsError`. Construct from `span`, pointer+`ByteSize`, range, initializer list, `string_view`, and a caller-owned `std::vector<std::byte>` (lvalue copies and leaves the vector; rvalue copies onto Base's heap then clears the vector — looks like a move, not a heap steal). Convert out with implicit `span` and `explicit operator std::vector<std::byte>` (`const&` copies and leaves `*this`; `&&` copies onto the caller CRT then clears `*this`). `append(BinaryData&&)` / `operator+=(BinaryData&&)` are a real same-heap move when `*this` is empty. Compare equal / unequal / three-way with another `BinaryData` and with `std::span<const std::byte>` (both operand orders). `HexDump()` and `HexDump(Size columns)` return a `Safe::String`: 8-digit offset, hex row, ASCII (non-printable as `.`). `columns` is a row width, not a byte length. `0` prints every byte on one line. Default `HexDump()` is 16 columns. `Type::Container` / `Type::Sized` / `Type::HasPushBack` / `Type::ByteInputRange` match; `Type::String` does not. `Serializable<BinaryData>` uses the container path (same wire as `std::vector<std::byte>`: `uint64` LE count + payload). Covered by `BinaryDataTests` and `SerializationTests`.
- **Error** — `Domain`, `Category`, `Code` (`Success`, `Unknown`) and `Fault`. Modules specialize `Domain` for their enums; `make_error_code` lives next to the enum so ADL feeds `std::error_code`. Category singletons stay in the module `.cxx`. `Fault` holds the code and a `Safe::String`. Not thrown. Covered by `ErrorTests`.
- **Shared** — `StormByte::Safe::Shared`, complements `std::shared_ptr` (`StormByte/safe/pointers.hxx`). It does not replace it. Exact type: `Safe::Heap::MakeShared`. Derived type: `Shared<Base>::MakePointer<Derived>` (the deleter still destroys the derived object). Daily operations match `std::shared_ptr` (copy, `reset`, `swap`, compare, `use_count`, `owner_before`). No constructor from a raw pointer or from `std::shared_ptr`, and no `release`. Converts implicitly to `std::shared_ptr<T>` (same control block, deleter stays Base). No conversion back. `StaticPointerCast`, `DynamicPointerCast`, `ConstPointerCast` and `ReinterpretPointerCast` keep that control block. Covered by `SafePointersTests`.
- **Unique** — `StormByte::Safe::Unique`, complements `std::unique_ptr` (`StormByte/safe/pointers.hxx`). It does not replace it. Exact type: `Safe::Heap::MakeUnique`. Derived type: `Unique<Base>::MakePointer<Derived>`, and `~Base` must be virtual or the call does not compile. No `release` and no constructor from a raw pointer. Converts on move to `std::unique_ptr<T, Safe::Heap::ObjectDeleter>` only. A `std::unique_ptr<T>` parameter has to change. `Safe::Heap` stays private and is not installed. Covered by `SafePointersTests`.
- **Weak** — `StormByte::Safe::Weak`, complements `std::weak_ptr`. Constructed only from `Shared`. `lock` returns `Shared`, or empty when expired. No constructor from `std::weak_ptr`. Covered by `SafePointersTests`.
- **ClonableTests** — clone / move / `MakePointer` coverage for `Shared` and `Unique`. `Derived::PointerType` used as storage. Implicit conversion to `std::shared_ptr` / `unique_ptr` with Base deleter. `ValidSmartPointer` rejects `std::shared_ptr` and default `std::unique_ptr`. A hidden-visibility shared library (`ClonablePlugin`) derives from `Clonable`; the tests check one shared `typeid`, `dynamic_cast` and `Clone` / `Move` across that boundary.
- **Size** — abstract unit count (`uint64_t` storage), same width on every host and safe across a DLL. Not an octet length. Implicit conversion only to `std::size_t` (clamp to `size_t::max`); every other integral destination is `explicit` and clamps to `T::max`. No `Value()`, no `operator bool`. Full arithmetic with any `Type::Integral` and with another `Size`; the result is always `Size`. Mixed `==` / `<=>` with any integral and with `ByteSize` are hidden friends so `n == 5` is not ambiguous. A negative integer operand is undefined and `assert`s when assertions are on. Overflow wraps in release. `operator Safe::String` / `operator Safe::WString` print the raw count (`atoi` style). Explicit instantiations of the templates live in the StormByte DLL. Covered by `SizeTests`.
- **ByteSize** — octet length (`uint64_t` storage), same width on every host and safe across a DLL. Implicit conversion only to `std::size_t` (clamp); every other integral destination is `explicit`. No `Value()`, no `operator bool`. Area products (`ByteSize * ByteSize`) are deleted. Scaling by `Size` or by an integer is allowed and yields `ByteSize`. IEC factories and free constants (`B`, `KiB`, `MiB`, `GiB`, `TiB`, `PiB`, `EiB`) so `1 * KiB` and `ByteSize::KiB(1)` are 1024 octets. SI (`KB`…`EB`) likewise. `operator Safe::String` / `operator Safe::WString` print IEC text (`0 B`, `1023 B`, `1.00 KiB`, `1.50 MiB`). Explicit instantiations live in the StormByte DLL. Covered by `ByteSizeTests`.

### Changed

- **Breaking: Iterable and Safe collections** — removed reference-exposing `StormByte::Iterable`; all public collection ranges now use opaque creator-module storage and callback-backed iterators/proxies. Mutable sequence proxies support classic `<algorithm>` and `std::ranges` without exposing owner-module references. Rvalue imports move elements and leave their STL source valid and empty without adopting its allocator or allocation.
- **Mutable text ranges** — `Safe::String` / `Safe::WString` expose mutable `data`, iterators, reverse iterators, indexing, `front` and `back` for in-place algorithms over existing code units, including embedded NULs, while preserving the trailing NUL contract.
- **Text API migration** — Base text-producing APIs (`Size`, `ByteSize`, UUID, Base64, `BinaryData`, `Telemetry`) now expose `Safe::String` / `Safe::WString`. Error messages and exceptions store `Safe::String`; exception plain-text constructors accept `std::string_view` or `const Safe::String&` and copy the text. Text serialization retains the standard string wire format.
- Shared vs static follows CMake `BUILD_SHARED_LIBS`. There is no `STORMBYTE_SHARED` CMake option. When the library is shared, the compile definition `STORMBYTE_SHARED` is still set so `visibility.h` can distinguish `dllexport` / `dllimport` / static.
- **License change** — original source in this repository is dual-licensed: GNU LGPL v3 or later, or a commercial license from the copyright holder. The grant applies only to original StormByte source in this repository. It does not cover other StormByte modules or third-party material (including `thirdparty/`). No patent rights are granted.
- **Visibility** — `STORMBYTE_PUBLIC` comes first on a function declaration (`STORMBYTE_PUBLIC Safe::String Foo();`). clang-cl rejects `__declspec` after a reference return. `class STORMBYTE_PUBLIC` stays on the type. Exported templates are declared `extern template STORMBYTE_PUBLIC` / `extern template class STORMBYTE_PUBLIC` in the header and instantiated in the `.cxx` with `STORMBYTE_INSTANTIATE` (`dllexport` on Windows, empty on ELF so GCC does not warn `-Wattributes`). The `extern` line in the header is what ELF uses to export; do not put `STORMBYTE_INSTANTIATE` on that line.
- **Breaking**: **Exception.** `StormByte::Component` is gone. `what()` is `StormByte: <message>`, or `StormByte.<path>: <message>` when a parent passes the segments under `StormByte` (`Crypto.Crypter`), joined with `.`. A parent passes `Exception::Path` (a `std::string_view` of its segments) and forwards the format and the arguments. A bare string is not a path, because that is ambiguous with the format constructor. It does not format. `Exception` calls `std::format` in the caller's translation unit and copies a `const char*` into a `Safe::String`. The view is not stored and no `std::string` enters the DLL. A final leaf inherits the parent constructors and adds no segment. A literal with no arguments is the message as-is. `DeserializeError`, `OutOfBoundsError` and `Base64Error` are leaves of the root (`StormByte: …`). Each of those types, and the root, defines its destructor in Base's `.cxx` so the `typeinfo` is unique across a DLL. Named types in other modules do the same.
- **Expected** — the error is a `Safe::Shared<E>`, not a `std::shared_ptr` built with `std::make_shared`. `Unexpected` uses `Safe::Heap::MakeShared`, or `Safe::Shared::MakePointer` for a derived error. The call stays `Unexpected<E>("… {}", args…)`. Formatting runs in the caller; `E` is constructed from the resulting string. `Unexpected(result.error())` forwards that `Shared` and does not allocate. `Shared<E>` converts to `std::shared_ptr<E>`. A `std::shared_ptr` is not accepted. `Expected<T, E>` is still `std::expected`. Covered by `ExpectedTests`.
- **Breaking:** `Clonable` — moves to `StormByte::Safe::Clonable` (`StormByte/safe/clonable.hxx`; `StormByte/clonable.hxx` is gone). Polymorphic `Clone` / `Move`. It is not an owner: `MakePointer` forwards to `Shared::MakePointer` or `Unique::MakePointer`, which allocate the object and the `shared_ptr` control block on Base's heap (`Safe::Heap::Allocate` / `Safe::Heap::Free` in a private TU that is not installed). `Clonable<T>` is `Clonable<T, Shared<T>>`. Unique ownership is `Clonable<T, Unique<T>>`. `Clonable<T, std::shared_ptr<T>>` and `Clonable<T, std::unique_ptr<T>>` no longer compile. `PointerType` is `Shared<T>` / `Unique<T>`; overrides that already return `PointerType` from `MakePointer` do not change. After construction, `Shared` converts implicitly to `std::shared_ptr<T>` so existing `shared_ptr` call sites keep working. `Unique` does not convert to `std::unique_ptr<T>`. `ValidSmartPointer` uses `Type::SameAs`. A class in another DLL may derive from `Clonable`: `Clonable`, `Shared`, `Unique`, `Weak`, `Heap::ObjectDeleter` and `Heap::Allocator` carry `STORMBYTE_PUBLIC_TYPE`, so their per-module `typeinfo` and vtables merge instead of staying hidden in each DLL (before, `Clonable<T>` was hidden in a `-fvisibility=hidden` consumer, and address-comparing runtimes such as libc++ disagreed on `typeid` / `dynamic_cast`). Covered by `ClonableTests`.
- **Breaking:** **`StormByte::Safe`** — `Shared`, `Unique`, `Weak`, the pointer casts, `Heap` and `Clonable` / `ValidSmartPointer` live in `StormByte::Safe`, under `StormByte/safe/` (`pointers.hxx`, `clonable.hxx`). `StormByte/safe_pointers.hxx` and `StormByte/clonable.hxx` are gone. No aliases are kept in `StormByte`.
- **Type::String** — also matches `StormByte::Safe::String` and `StormByte::Safe::WString`. They stay out of `Type::Container`.
- **Type::Sized** — `size()` may be implicitly convertible to `std::size_t` (STL containers) **or** be `StormByte::Size` / `StormByte::ByteSize`. Covered by `TypeTraitsTests`.
- **Type::Numeral** — also matches `StormByte::Size` and `StormByte::ByteSize`.
- **Type::Array** — StormByte flavor of a fixed-size sequence; `Serializable` uses it instead of stock `std::is_same` / `tuple_size` probes. `BinaryData` is not an array.
- **GenerateUUIDv4** — returns `Safe::String` instead of `std::string`.
- **Base64Encode** — both overloads return `Safe::String` instead of `std::string`. **Base64Decode** returns `BinaryData` and still takes `std::string_view`.
- **Size vs ByteSize everywhere** — public Base APIs no longer take or return a raw `std::size_t` / `std::uint64_t` when the value is a count. Character counts (`Safe::String::size`, `Safe::WString::size`, subscripts) are `Size`. Octet counts (`BinaryData::size` / `operator[]` / ctors / `Serializable<T>::Size`, wire lengths) are `ByteSize`. Mixed arithmetic and comparison stay typed: a `Size` result stays a `Size`; a `ByteSize` result stays a `ByteSize`.
- **Serializable** — container and string codecs use `ByteSize` for the on-wire length. `Safe::String` / `Safe::WString` codecs preserve embedded NULs; wide text is encoded as UTF-8 on the wire. Concepts used to branch (`Type::Array`, `Type::Container`, `Type::String`, `Type::Numeral`) are StormByte flavor, not stock `std::*` traits.
- **Tests** — suite section headers (`// -------------------`) match in the body and in `main`. UUID, Base64, Bitmask, Clonable, Exception, Expected, ThreadLock, TestHandlers, Iterable, Serialization, SafePointers, TypeTraits, Size, ByteSize and BinaryData cover the public surface (format, padding, invalid input, clone independence, non-owner unlock, bounds, wire, corruption, units, `*` / `/` / `%`, `<algorithm>` / ranges / iterators, `HexDump` columns, span comparison, `operator+=`, IEC text). `Type::Sized` covers a container whose `size()` returns `StormByte::Size` or `StormByte::ByteSize`. `Base64Decode` of an encode result uses an explicit `std::string_view`. `SerializationTests` cover `BinaryData` roundtrip, the same wire as `std::vector<std::byte>`, truncation, a huge size field, bit-flip / byte-overwrite / random corruption, trailing garbage and `std::optional<BinaryData>`.

### Removed

- **Old root String helpers** (`StormByte::String` in the former Base API) — replaced by the owned `StormByte::Safe::String` / `WString` types in Base.
- **System** (`StormByte::System` in Base: `TempFileName`, `CurrentPath`, `ExecutablePath`, `Sleep`) — leaves Base. Absorbed by the existing StormByte-System module.
- **UTF8Error** / **SystemError** — leave Base with String and System. Base retains `DeserializeError`, `OutOfBoundsError` and `Base64Error`, and adds `AllocationError`, `ExpiredWeakPointerError` and `OperationError`.
- **CoreApiTests** — coverage lives in `ClonableTests` and `ErrorTests`.

### Fixed
- Safe heap and text buffer allocation, pointer factories and named clock creation translate the identified foreign failures into StormByte exceptions. Expired weak-owner promotion throws `ExpiredWeakPointerError`; overflowing text capacities throw `OutOfBoundsError`. Wide buffer size validation includes the code-unit width. Shared control-block allocation failure no longer destroys the object twice.
- `Safe::Owner` rejects non-null state without a destruction callback and reports attempts to copy non-clonable state. `Telemetry::MeasureClock` is best-effort and `noexcept`, with `Sample::Active` exposing inactive tokens; named clock insertion releases its lock on failure and read-only lookup does not allocate a temporary key.
- **Safe collection values.** Admit arithmetic scalar types such as integers and floating-point values, alongside enums, for use in Safe collections.
- **Safe collection reuse across modules.** Preserve each sequence, map and queue's creator-module storage callbacks after moving from it, so reusing a moved-from collection recreates its storage with the originating module's STL ABI.

- **`FindStormByte`.** `String` is no longer a package component; Base now provides the owned text types. `Buffer` pulls `Logger` and `System`; `Database` pulls `Logger`. `Crypto`, `Multimedia` and `Network` name only `Buffer`; the closure finds `Logger` and `System`. `Config` links only the core.
- **`ByteSize` / `Size`.** The integer constructors stay in the header. GCC does not emit a `constexpr` constructor that is both an `extern template` and an explicit instantiation, so `SizeTests` crashed and `ByteSize(unsigned long long)` was missing from the shared library. The operators are still one copy in the DLL. `++` / `--` build the step with the private constructor, so they do not instantiate `unsigned int` early.
- **`Size` / `ByteSize` to `std::string`.** The conversion calls `operator Safe::String()` by name. On GCC, `static_cast<Safe::String>` picks `Safe::String(std::string)` and that calls the same operator again until the stack dies.
- **`ByteSize` wide text.** Its wide form uses the UTF-8 conversion between `Safe::String` and `Safe::WString`. `swprintf` and `%s` is a narrow string on glibc and a wide string on the Windows CRT, so `1.00 KiB` did not match.
- **Caller containers.** `Safe::String::operator std::string`, `Safe::WString::operator std::wstring`, `Size` / `ByteSize` `operator std::string`, their `operator<<`, and `BinaryData::operator std::vector` are `STORMBYTE_FORCE_INLINE`. On an exported class, `inline` can still be a call into the DLL, so the container was allocated here and freed by the caller. The vector operators were out of line; they now copy through `span()` in the caller, then `clear()` on Base's heap.

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
