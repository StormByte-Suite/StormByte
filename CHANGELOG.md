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

## [2.0.0] - 2026-09-27

### Added

- **`STORMBYTE_FORCE_INLINE`** — in `platform.h`. `inline` plus `__forceinline` or `always_inline`, so the body is emitted in the caller. `inline` alone is only a hint.
- **BinaryData** — owned contiguous sequence of `std::byte`, safe to use across a DLL boundary. Same kind of API as `std::vector<std::byte>` (iterators, `<algorithm>`, `std::ranges`, `std::span`, insert / erase / assign / `append` / `emplace_back` / `operator+=`). Lengths and indices use `StormByte::ByteSize`. Storage lives on Base's heap; the public type is not `std::vector<std::byte>`. `at()` throws `OutOfBoundsError`. Construct from `span`, pointer+`ByteSize`, range, initializer list, `string_view`, and a caller-owned `std::vector<std::byte>` (lvalue copies and leaves the vector; rvalue copies onto Base's heap then clears the vector — looks like a move, not a heap steal). Convert out with implicit `span` and `explicit operator std::vector<std::byte>` (`const&` copies and leaves `*this`; `&&` copies onto the caller CRT then clears `*this`). `append(BinaryData&&)` / `operator+=(BinaryData&&)` are a real same-heap move when `*this` is empty. Compare equal / unequal / three-way with another `BinaryData` and with `std::span<const std::byte>` (both operand orders). `HexDump()` and `HexDump(std::size_t columns)` return a `CString`: 8-digit offset, hex row, ASCII (non-printable as `.`). `columns` is a row width, not a byte length — it is `std::size_t`, not `ByteSize`. `0` prints every byte on one line. Default `HexDump()` is 16 columns. `Type::Container` / `Type::Sized` / `Type::HasPushBack` / `Type::ByteInputRange` match; `Type::String` does not. `Serializable<BinaryData>` uses the container path (same wire as `std::vector<std::byte>`: `uint64` LE count + payload). Covered by `BinaryDataTests` and `SerializationTests`.
- **CString** — owned NUL-terminated buffer, safe to use across a DLL boundary. Not a `std::string`. Copy, move, `Reset`, `swap`, `Length` (`StormByte::Size`), observer `operator[]` (indices `[0, Length()]`; `Length()` is the trailing NUL; null or past `Length()` is undefined and `assert`s when assertions are on), explicit `operator bool` (non-null pointer; `""` is valid empty text), explicit `const char*` (same lifetime as `std::string::c_str()`), explicit `std::string_view` (empty view when null; `[0, Length())`; same lifetime), implicit `std::string`, `operator<<`, content `==` / `!=` / `<=>`, and `std::hash`. Construct from `const char*` (null stays null), `std::string_view` and `const std::string&` (copy onto Base's heap, not a steal; empty yields `""`, not null). Covered by `CStringTests`.
- **WCString** — wide counterpart of `CString` (`wchar_t` / `std::wstring` / `std::wstring_view` / `std::wostream`). Same comparison, hash, subscript and lifetime rules as `std::wstring::c_str()`. `Length()` is `StormByte::Size`. Construct from `const wchar_t*` (null stays null), `std::wstring_view` and `const std::wstring&` (copy onto Base's heap; empty yields `L""`). Covered by `WCStringTests`.
- **Error** — `Domain`, `Category`, `Code` (`Success`, `Unknown`) and `Fault`. Modules specialize `Domain` for their enums; `make_error_code` lives next to the enum so ADL feeds `std::error_code`. Category singletons stay in the module `.cxx`. `Fault` holds the code and a `CString`. Not thrown. Covered by `ErrorTests`.
- **Shared** — complements `std::shared_ptr` (`safe_pointers.hxx`). It does not replace it. Exact type: `Heap::MakeShared`. Derived type: `Shared<Base>::MakePointer<Derived>` (the deleter still destroys the derived object). Daily operations match `std::shared_ptr` (copy, `reset`, `swap`, compare, `use_count`, `owner_before`). No constructor from a raw pointer or from `std::shared_ptr`, and no `release`. Converts implicitly to `std::shared_ptr<T>` (same control block, deleter stays Base). No conversion back. `StaticPointerCast`, `DynamicPointerCast`, `ConstPointerCast` and `ReinterpretPointerCast` keep that control block. Covered by `SafePointersTests`.
- **Unique** — complements `std::unique_ptr` (`safe_pointers.hxx`). It does not replace it. Exact type: `Heap::MakeUnique`. Derived type: `Unique<Base>::MakePointer<Derived>`, and `~Base` must be virtual or the call does not compile. No `release` and no constructor from a raw pointer. Converts on move to `std::unique_ptr<T, Heap::ObjectDeleter>` only. A `std::unique_ptr<T>` parameter has to change. `Heap` stays private and is not installed. Covered by `SafePointersTests`.
- **Weak** — complements `std::weak_ptr`. Constructed only from `Shared`. `lock` returns `Shared`, or empty when expired. No constructor from `std::weak_ptr`. Covered by `SafePointersTests`.
- **ClonableTests** — clone / move / `MakePointer` coverage for `Shared` and `Unique`. `Derived::PointerType` used as storage. Implicit conversion to `std::shared_ptr` / `unique_ptr` with Base deleter. `ValidSmartPointer` rejects `std::shared_ptr` and default `std::unique_ptr`.
- **Size** — abstract unit count (`uint64_t` storage), same width on every host and safe across a DLL. Not an octet length. Implicit conversion only to `std::size_t` (clamp to `size_t::max`); every other integral destination is `explicit` and clamps to `T::max`. No `Value()`, no `operator bool`. Full arithmetic with any `Type::Integral` and with another `Size`; the result is always `Size`. Mixed `==` / `<=>` with any integral and with `ByteSize` are hidden friends so `n == 5` is not ambiguous. A negative integer operand is undefined and `assert`s when assertions are on. Overflow wraps in release. `operator CString` / `operator WCString` print the raw count (`atoi` style). Explicit instantiations of the templates live in the StormByte DLL. Covered by `SizeTests`.
- **ByteSize** — octet length (`uint64_t` storage), same width on every host and safe across a DLL. Implicit conversion only to `std::size_t` (clamp); every other integral destination is `explicit`. No `Value()`, no `operator bool`. Area products (`ByteSize * ByteSize`) are deleted. Scaling by `Size` or by an integer is allowed and yields `ByteSize`. Mixed `==` / `<=>` with any integral and with `Size`. IEC factories and free constants (`B`, `KiB`, `MiB`, `GiB`, `TiB`, `PiB`, `EiB`) so `1 * KiB` and `ByteSize::KiB(1)` are 1024 octets. SI (`KB`…`EB`) likewise. `operator CString` / `operator WCString` print IEC text (`0 B`, `1023 B`, `1.00 KiB`, `1.50 MiB`). Explicit instantiations live in the StormByte DLL. Covered by `ByteSizeTests`.

### Changed

- Shared vs static follows CMake `BUILD_SHARED_LIBS`. There is no `STORMBYTE_SHARED` CMake option. When the library is shared, the compile definition `STORMBYTE_SHARED` is still set so `visibility.h` can distinguish `dllexport` / `dllimport` / static.
- **License change** — original source in this repository is dual-licensed: GNU LGPL v3 or later, or a commercial license from the copyright holder. The grant applies only to original StormByte source in this repository. It does not cover other StormByte modules or third-party material (including `thirdparty/`). No patent rights are granted.
- **Visibility** — `STORMBYTE_PUBLIC` comes first on a function declaration (`STORMBYTE_PUBLIC CString Foo();`). clang-cl rejects `__declspec` after a reference return. `class STORMBYTE_PUBLIC` stays on the type. Exported templates are declared `extern template STORMBYTE_PUBLIC` / `extern template class STORMBYTE_PUBLIC` in the header and instantiated in the `.cxx` with `STORMBYTE_INSTANTIATE` (`dllexport` on Windows, empty on ELF so GCC does not warn `-Wattributes`). The `extern` line in the header is what ELF uses to export; do not put `STORMBYTE_INSTANTIATE` on that line.
- **Breaking**: **Exception.** `StormByte::Component` is gone. `what()` is `StormByte: <message>`, or `StormByte.<path>: <message>` when a parent passes the segments under `StormByte` (`Crypto.Crypter`), joined with `.`. A parent passes `Exception::Path` (a `std::string_view` of its segments) and forwards the format and the arguments. A bare string is not a path, because that is ambiguous with the format constructor. It does not format. `Exception` calls `std::format` in the caller's translation unit and copies a `const char*` into a `CString`. The view is not stored and no `std::string` enters the DLL. A final leaf inherits the parent constructors and adds no segment. A literal with no arguments is the message as-is. `DeserializeError`, `OutOfBoundsError` and `Base64Error` are leaves of the root (`StormByte: …`). Each of those types, and the root, defines its destructor in Base's `.cxx` so the `typeinfo` is unique across a DLL. Named types in other modules do the same.
- **Expected** — the error is a `Shared<E>`, not a `std::shared_ptr` built with `std::make_shared`. `Unexpected` uses `Heap::MakeShared`, or `Shared::MakePointer` for a derived error. The call stays `Unexpected<E>("… {}", args…)`. Formatting runs in the caller; `E` is constructed from the resulting string. `Unexpected(result.error())` forwards that `Shared` and does not allocate. `Shared<E>` converts to `std::shared_ptr<E>`. A `std::shared_ptr` is not accepted. `Expected<T, E>` is still `std::expected`. Covered by `ExpectedTests`.
- **Breaking:** `Clonable` — polymorphic `Clone` / `Move`. It is not an owner: `MakePointer` forwards to `Shared::MakePointer` or `Unique::MakePointer`, which allocate the object and the `shared_ptr` control block on Base's heap (`Heap::Allocate` / `Heap::Free` in a private TU that is not installed). `Clonable<T>` is `Clonable<T, Shared<T>>`. Unique ownership is `Clonable<T, Unique<T>>`. `Clonable<T, std::shared_ptr<T>>` and `Clonable<T, std::unique_ptr<T>>` no longer compile. `PointerType` is `Shared<T>` / `Unique<T>`; overrides that already return `PointerType` from `MakePointer` do not change. After construction, `Shared` converts implicitly to `std::shared_ptr<T>` so existing `shared_ptr` call sites keep working. `Unique` does not convert to `std::unique_ptr<T>`. `ValidSmartPointer` uses `Type::SameAs`. Covered by `ClonableTests`.
- **Type::String** — also matches `StormByte::String::String` and `StormByte::String::WString` (forward-declared in Base). They stay out of `Type::Container`.
- **Type::Sized** — `size()` may be implicitly convertible to `std::size_t` (STL containers) **or** be `StormByte::Size` / `StormByte::ByteSize`. Covered by `TypeTraitsTests`.
- **Type::Numeral** — also matches `StormByte::Size` and `StormByte::ByteSize`.
- **Type::Array** — StormByte flavor of a fixed-size sequence; `Serializable` uses it instead of stock `std::is_same` / `tuple_size` probes. `BinaryData` is not an array.
- **GenerateUUIDv4** — returns `CString` instead of `std::string`.
- **Base64Encode** — both overloads return `CString` instead of `std::string`. **Base64Decode** returns `BinaryData` and still takes `std::string_view`.
- **Size vs ByteSize everywhere** — public Base APIs no longer take or return a raw `std::size_t` / `std::uint64_t` when the value is a count. Character counts (`CString::Length`, `WCString::Length`, subscripts) are `Size`. Octet counts (`BinaryData::size` / `operator[]` / ctors / `Serializable<T>::Size`, wire lengths) are `ByteSize`. Mixed arithmetic and comparison stay typed: a `Size` result stays a `Size`; a `ByteSize` result stays a `ByteSize`.
- **Serializable** — container and string codecs use `ByteSize` for the on-wire length. Concepts used to branch (`Type::Array`, `Type::Container`, `Type::String`, `Type::Numeral`) are StormByte flavor, not stock `std::*` traits.
- **Tests** — suite section headers (`// -------------------`) match in the body and in `main`. UUID, Base64, Bitmask, Clonable, Exception, Expected, ThreadLock, TestHandlers, Iterable, Serialization, SafePointers, TypeTraits, Size, ByteSize and BinaryData cover the public surface (format, padding, invalid input, clone independence, non-owner unlock, bounds, wire, corruption, units, `*` / `/` / `%`, `<algorithm>` / ranges / iterators, `HexDump` columns, span comparison, `operator+=`, IEC text). `Type::Sized` covers a container whose `size()` returns `StormByte::Size` or `StormByte::ByteSize`. `Base64Decode` of an encode result uses an explicit `std::string_view`. `SerializationTests` cover `BinaryData` roundtrip, the same wire as `std::vector<std::byte>`, truncation, a huge size field, bit-flip / byte-overwrite / random corruption, trailing garbage and `std::optional<BinaryData>`.

### Removed

- **String** (`StormByte::String` helpers, `string.hxx` / `string.cxx`) — leaves Base. Helpers move to a new StormByte-String library.
- **System** (`StormByte::System` in Base: `TempFileName`, `CurrentPath`, `ExecutablePath`, `Sleep`) — leaves Base. Absorbed by the existing StormByte-System module.
- **UTF8Error** / **SystemError** — leave Base with String and System. Derived exceptions that remain are `DeserializeError`, `OutOfBoundsError` and `Base64Error`.
- **CoreApiTests** — coverage lives in `ClonableTests` and `ErrorTests`.

### Fixed

- **`FindStormByte`.** `Buffer` pulls `Logger`, `String` and `System`. `Logger` and `System` pull `String`. `Database` pulls `Logger` and `String`. `Crypto`, `Multimedia` and `Network` name only `Buffer`; the closure still finds `Logger`, `String` and `System`. `Config` and `String` still link only the core.
- **`ByteSize` / `Size`.** The integer constructors stay in the header. GCC does not emit a `constexpr` constructor that is both an `extern template` and an explicit instantiation, so `SizeTests` crashed and `ByteSize(unsigned long long)` was missing from the shared library. The operators are still one copy in the DLL. `++` / `--` build the step with the private constructor, so they do not instantiate `unsigned int` early.
- **`Size` / `ByteSize` to `std::string`.** The conversion calls `operator CString()` by name. On GCC, `static_cast<CString>` picks `CString(std::string)` and that calls the same operator again until the stack dies.
- **`CString` / `WCString`.** Each constructs from the other. Narrow text is UTF-8. `ByteSize`'s wide form uses that conversion. `swprintf` and `%s` is a narrow string on glibc and a wide string on the Windows CRT, so `1.00 KiB` did not match.
- **Caller containers.** `CString::operator std::string`, `WCString::operator std::wstring`, `Size` / `ByteSize` `operator std::string`, their `operator<<`, and `BinaryData::operator std::vector` are `STORMBYTE_FORCE_INLINE`. On an exported class, `inline` can still be a call into the DLL, so the container was allocated here and freed by the caller. The vector operators were out of line; they now copy through `span()` in the caller, then `clear()` on Base's heap.

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