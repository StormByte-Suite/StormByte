# StormByte

![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)
![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.28+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3 or commercial](https://img.shields.io/badge/License-LGPL_v3_or_commercial-blue.svg)
[![CI](https://github.com/StormBytePP/StormByte/actions/workflows/ci.yml/badge.svg)](https://github.com/StormBytePP/StormByte/actions/workflows/ci.yml)
[![Sponsor](https://img.shields.io/badge/Sponsor-StormBytePP-ea4aaa?logo=githubsponsors)](https://github.com/sponsors/StormBytePP)

This repository is **StormByte Base**: the C++26 foundation of the StormByte suite.

It is the module every other StormByte library links. Public headers live under `StormByte/` and cover exceptions, `Expected`, little-endian serialization, the `StormByte::Safe` value types, `Size`, `ByteSize`, UUID v4, bitmasks, a reentrant `ThreadLock`, and the `StormByte::Type` concepts.

The suite is split on purpose. Buffer, Config, Crypto, Database, Logger, Multimedia, Network and System are **other repositories**. They depend on this one; this one does not implement them.

## What this module does

- **Exceptions** — `StormByte::Exception`. `what()` is `StormByte: …`, or `StormByte.Crypto.Crypter: …` when a parent passes the segments under `StormByte`. The text is a `Safe::String`. A final leaf adds no segment.
- **Error** — `Domain`, `Category`, `Code` and `Fault` for `std::error_code`. `Fault` is not thrown; its text is a `Safe::String`.
- **Expected** — `Expected<T, E>` on top of `std::expected`. The error is a `Safe::Shared<E>` on Base's heap. It converts to `std::shared_ptr<E>`. `Unexpected<E>("… {}", arg)` stays as it is.
- **Serialization** — `Serializable<T>` to `Safe::Binary`, always little-endian, no BOM and no version tag. Optional / pair / container / trivial / `Detail::Codec<T>`. On-wire lengths are `ByteSize`.
- **Safe** — text, bytes, collections, optional, pair, variant, hash, owners, callbacks and the wait primitives whose storage lives on Base's heap. A value can be created in one module and destroyed in another when those modules do not share a C++ runtime. See [Safe](#safe).
- **Size** — abstract unit count (`uint64_t` storage), same width on every host and safe across a DLL. Implicit only to `std::size_t`. Character counts, iteration counts, “how many items”.
- **ByteSize** — octet length (`uint64_t` storage). Implicit only to `std::size_t`. IEC / SI units (`1 * KiB`), human-readable `Safe::String` (`1.00 KiB`). Area products are deleted.
- **UUID** — RFC 4122 version 4 (`GenerateUUIDv4`), returned as `Safe::String`.
- **Bitmask** — CRTP flags over `Type::UnsignedEnum`.
- **ThreadLock** — owner-thread reentry; `Unlock` from a non-owner is a no-op. Not the cross-module wait. That is [Wait](#wait), inside Safe.
- **Type concepts** — `StormByte::Type::*` (`String`, `Container`, `Optional`, `Pair`, `Numeral`, `Array`, …). `Numeral` includes `Size` and `ByteSize`. No `enable_if` / `void_t` next to them.
- **Platform / visibility** — `WINDOWS` / `LINUX` / `MACOS`, `BIT32` / `BIT64`, `CLANG` / `GCC` / `MSVC` (clang-cl is `CLANG`, not `MSVC`).

Public Base APIs do not take or return a raw `std::size_t` / `std::uint64_t` when the value is a count. Characters and units are `Size`. Octets are `ByteSize`.

## The rest of the suite

| Module | Role | API |
| --- | --- | --- |
| **Base** | This repository | [/StormByte](https://suite.stormbyte.org/StormByte) |
| [Buffer](https://github.com/StormBytePP/StormByte-Buffer) | FIFO, SharedFIFO, Ring, Producer/Consumer and multi-stage pipelines | [/StormByte-Buffer](https://suite.stormbyte.org/StormByte-Buffer) |
| [Config](https://github.com/StormBytePP/StormByte-Config) | Human-readable text and versioned binary documents (groups, lists, raw bytes) | [/StormByte-Config](https://suite.stormbyte.org/StormByte-Config) |
| [Crypto](https://github.com/StormBytePP/StormByte-Crypto) | Hash, compress, encrypt, sign and key agreement — Crypto++ never leaves the private tree | [/StormByte-Crypto](https://suite.stormbyte.org/StormByte-Crypto) |
| [Database](https://github.com/StormBytePP/StormByte-Database) | One API over SQLite, PostgreSQL and MariaDB | [/StormByte-Database](https://suite.stormbyte.org/StormByte-Database) |
| [Logger](https://github.com/StormBytePP/StormByte-Logger) | Stream logger with levels, headers, human-readable sizes and redaction (`ThreadedLog`) | [/StormByte-Logger](https://suite.stormbyte.org/StormByte-Logger) |
| [Multimedia](https://github.com/StormBytePP/StormByte-Multimedia) | Decode, encode and containers without raw FFmpeg types; codecs enabled only if present | [/StormByte-Multimedia](https://suite.stormbyte.org/StormByte-Multimedia) |
| [Network](https://github.com/StormBytePP/StormByte-Network) | Framed packets, Client/Server, IPv4/IPv6 TCP and Buffer pipelines (compress/encrypt) | [/StormByte-Network](https://suite.stormbyte.org/StormByte-Network) |
| [System](https://github.com/StormBytePP/StormByte-System) | Processes, pipes and environment variables across Linux, Windows and macOS | [/StormByte-System](https://suite.stormbyte.org/StormByte-System) |

## Table of Contents

- [What this module does](#what-this-module-does)
- [The rest of the suite](#the-rest-of-the-suite)
- [Installation](#installation)
- [Usage](#usage)
  - [Exceptions](#exceptions)
  - [Expected](#expected)
  - [Error](#error)
  - [Safe](#safe)
    - [Contract](#contract)
    - [STORMBYTE_DECLARE_MAYBE_SAFE](#stormbyte_declare_maybe_safe)
    - [Text](#text)
    - [Binary](#binary)
    - [Collections](#collections)
      - [Vector](#vector)
      - [List](#list)
      - [Queue](#queue)
      - [Map](#map)
      - [Set](#set)
      - [UnorderedMap](#unorderedmap)
      - [UnorderedSet](#unorderedset)
      - [Iterable](#iterable)
    - [Optional, Pair, Variant](#optional-pair-variant)
    - [Hash](#hash)
    - [Pointers and Clonable](#pointers-and-clonable)
    - [Callbacks and owners](#callbacks-and-owners)
    - [Wait](#wait)
      - [Mutex](#mutex)
      - [SharedMutex](#sharedmutex)
      - [UniqueLock](#uniquelock)
      - [SharedLock](#sharedlock)
      - [ConditionVariable](#conditionvariable)
      - [ConditionVariableAny](#conditionvariableany)
      - [Atomic](#atomic)
  - [Size](#size)
  - [ByteSize](#bytesize)
  - [Serialization](#serialization)
  - [UUID](#uuid)
  - [ThreadLock](#threadlock)
  - [Type concepts](#type-concepts)
  - [Bitmask](#bitmask)
  - [Telemetry](#telemetry)
- [Contributing](#contributing)
- [License](#license)
- [Support](#support)

## Installation

Needs a C++26 compiler and CMake 3.28 or newer.

```sh
git clone --recurse-submodules https://github.com/StormBytePP/StormByte.git
cd StormByte
cmake -S . -B build
cmake --build build
```

Shared vs static follows CMake `BUILD_SHARED_LIBS` (declared in `lib/`, default ON). A plain configure builds the shared library. `-DBUILD_SHARED_LIBS=OFF` builds a static archive; on Windows the headers then do not use `dllimport`.

A shared build keeps this library as its own `.so` / `.dll`. Under the LGPL that is usually the simpler way to ship: the user can replace that file. A static archive is folded into your binary. The LGPL still applies to this code; you must give the recipient a way to relink your product with a different build of this library. If that does not fit how you distribute the final product, a commercial license is available from the copyright holder (see [License](#license)).

`thirdparty/sds` is a private submodule compiled into StormByte. It is not installed.

## Usage

Headers are `#include <StormByte/….hxx>`. Namespace root is `StormByte`.

### Exceptions

Base owns the exception system other modules inherit. A throw of `Exception` reads `StormByte: …`. A parent passes `Exception::Path` (a `string_view` of its segments) and forwards the format and the arguments. It does not format. A bare string is not a path: that would be ambiguous with the format constructor. Formatting happens in the caller's translation unit; the result is copied into a `Safe::String`. Plain text constructors accept `std::string_view` or `const Safe::String&`; neither view nor caller storage is retained.

A final leaf inherits the parent constructors and adds no segment, so `EncryptException("bad key {}", id)` reads `StormByte.Crypto.Crypter: bad key …`. `DeserializeError`, `OutOfBoundsError` and `Base64Error` are leaves of the root: `StormByte: …`.

`AllocationError` reports allocation failure with a static message and a non-allocating default constructor. `ExpiredWeakPointerError` reports promotion of an empty or expired `Safe::Weak`. `OperationError` wraps other foreign failures. `BadOptionalAccess` and `BadVariantAccess` take their message from the constructor. Each named exception has its destructor defined in Base. `Safe::Heap::Allocate`, Safe pointer factories, text buffer allocation and named clock creation translate the identified standard failures to StormByte exceptions. Factories preserve an existing StormByte exception's dynamic type. Inside an active exception handler, modules can call `Safe::Heap::RethrowException()` to preserve a StormByte exception, translate `std::bad_alloc` to `AllocationError`, or translate another foreign exception to `OperationError`.

This translation is a boundary policy, not a guarantee about arbitrary STL operations or caller-owned conversions. Modules must translate foreign exceptions at their own throwing API boundaries and contain them in `noexcept` paths. Throwing from a `noexcept` API still terminates; changing the exception type does not change that contract. A Safe value may cross libstdc++ and libc++. An exception thrown by the other runtime may not.

Each named type defines its destructor in that module's `.cxx`. That keeps one `typeinfo`, so `catch` matches across a DLL.

```cpp
#include <StormByte/exception.hxx>
#include <iostream>

using namespace StormByte;

class CryptoError: public Exception {
	public:
		template <typename... Args>
		explicit CryptoError(std::format_string<Args...> fmt, Args&&... args)
			: Exception(Path{"Crypto"}, fmt, std::forward<Args>(args)...) {}

		~CryptoError() override;

	protected:
		template <typename... Args>
		explicit CryptoError(Path child, std::format_string<Args...> fmt, Args&&... args)
			: Exception(Path{std::string("Crypto.") + std::string(child.text)}, fmt, std::forward<Args>(args)...) {}
};

class CrypterError: public CryptoError {
	public:
		template <typename... Args>
		explicit CrypterError(std::format_string<Args...> fmt, Args&&... args)
			: CryptoError(Path{"Crypter"}, fmt, std::forward<Args>(args)...) {}

		~CrypterError() override;
};

class EncryptError: public CrypterError {
	public:
		using CrypterError::CrypterError;
		~EncryptError() override;
};

void process_data(int value) {
	if (value < 0)
		throw Exception("Invalid value: {}", value);
}

int main() {
	try {
		process_data(-5);
	} catch (const Exception& e) {
		std::cerr << e.what() << std::endl; // StormByte: Invalid value: -5
	}
}
```

`~CryptoError`, `~CrypterError` and `~EncryptError` are defined in the module `.cxx` (`= default` is enough).

### Expected

The error is a `Safe::Shared<E>` on Base's heap. Read it with `result.error()->what()`. It converts to `std::shared_ptr<E>` when a signature already asks for one. The call does not change: `Unexpected<E>("Password '{}' not found", name)` formats in the caller and constructs `E` from that string. `Unexpected(result.error())` forwards the same `Shared` and does not allocate. A `std::shared_ptr` is not accepted.

```cpp
#include <StormByte/expected.hxx>
#include <StormByte/exception.hxx>
#include <iostream>

using namespace StormByte;

Expected<int, Exception> divide(int a, int b) {
	if (b == 0)
		return Unexpected<Exception>("Division by zero");
	return a / b;
}
```

### Error

`Fault` wraps a `std::error_code`. Across a DLL use `Fault::what()` (backed by `Safe::String`), not `error_code::message()`.

```cpp
#include <StormByte/error.hxx>
#include <iostream>
#include <system_error>

using namespace StormByte;

int main() {
	const std::error_code code = Error::Code::Unknown;
	const Error::Fault fault{code};
	if (fault)
		std::cerr << fault.what() << std::endl;
}
```

A module adds its own enum, specializes `Error::Domain`, and puts `make_error_code` next to the enum so ADL fills `std::error_code`. The category singleton lives in that module’s `.cxx`.

### Safe

`StormByte::Safe` is the set of values a public signature can hand across a module boundary.

Inside one module, `std::string`, `std::vector` and the rest of the standard library are the right tools. They are faster, they are more complete, and a function that never leaves the translation unit should keep them. Safe exists for the other case: a string, a buffer, a container or a wait created in a plugin and destroyed in the host, or the other way around.

#### Contract

A standard container allocates with the C++ runtime that compiled the caller. On Windows the debug CRT and the release CRT are different heaps: a `std::string` built in a release DLL and destroyed in a debug EXE is a heap mismatch, and the other direction is the same failure. On Linux and macOS the libc is usually shared, but libstdc++ and libc++ do not share allocators, and an address-comparing runtime does not share `typeinfo`. Handing the object across that boundary is enough to crash. A `std::mutex`, a `std::shared_mutex` or a `std::condition_variable` is the same kind of object: the wait runs in the runtime that constructed it.

Safe keeps the heap in Base. `Safe::Heap::Allocate` and `Safe::Heap::Free` own every block. Construction, growth and destruction of a Safe value run there. A move from `std::vector` or `std::string` does not steal the pointer: the elements are copied onto the Base heap and the source is then cleared. An explicit conversion back is `STORMBYTE_FORCE_INLINE`, so the caller runtime allocates that copy and later frees it. Peak use during the transfer is two copies.

The surface is std-like (`begin`, `size`, `push_back`, `lock`, `wait`, brace initialization) so a call site changes the type and little else. It is not binary-compatible with the STL. There is no `Safe` alias of `std::vector` on non-Windows hosts. `<algorithm>` and `std::ranges` work through the public iterators. They do not see a node owned by the other module.

What the contract covers:

- A Safe value may be created with libstdc++ and destroyed with libc++, or the other way around.
- On Windows it may cross a debug CRT and a release CRT.
- Both sides use the same C++ ABI, packing and calling convention.
- Base, and every module that owns a callback or a `MaybeSafe` value, stay loaded until that value is gone.
- `Size` and `ByteSize` are the crossable counts. Conversion to `std::size_t` is explicit in the sense that only that destination is implicit; every other integral destination is `explicit`.
- A `Safe::Mutex`, a `Safe::SharedMutex`, a `Safe::ConditionVariable`, a `Safe::ConditionVariableAny` and a `Safe::Atomic<T>` may be waited on in one module and notified in another. The gate, the shared gate, the signal and the word live on Base's heap.

What it does not cover:

- Catching an exception thrown by the other runtime. StormByte exceptions are anchored in Base. A foreign exception is not.
- Matching `std::hash` of another STL. `Safe::Hash` is the cross-module hash. `std::hash` specializations delegate to it inside one module.
- A faster or more complete container than the STL. Inside one module, keep the STL.
- `std::thread`, `std::once_flag` and `std::future`. A thread created and joined inside one `.cxx` does not cross. Those types are not added.

#### STORMBYTE_DECLARE_MAYBE_SAFE

`Type::IsSafe<T>` means Base already knows the type and backs this contract. `Safe::String`, `Safe::Vector<int>`, `Size` and `Exception` are in that set. A consumer does not specialize `IsSafe` or `IsMaybeSafe`.

`Type::MaybeSafe<T>` is the opt-in for a type Base cannot see into. After the type is complete, at global namespace scope:

```cpp
STORMBYTE_DECLARE_MAYBE_SAFE(my::plugin::Record);
```

The macro is a registration, not a proof. C++ cannot inspect private members, and it cannot tell whether a destructor is defined out of line. The provider asserts that the type is safe to copy, move, assign and destroy across the boundary. Base then checks the operations the chosen wrapper needs and propagates the level: a composition of `IsSafe` types stays `IsSafe`; one `MaybeSafe` member makes the composition `MaybeSafe`.

The type must meet all of this:

- Copy, move, assignment and destruction free every resource with the allocator and the module that allocated it. A `std::string` or `std::vector` member does not meet that. Store a `Safe::String` or a `Safe::Vector`, or define the special members out of line in the provider so the STL operation runs in the module that owns the CRT.
- Resource-bearing classes define the relevant constructors, assignments and the destructor in the provider `.cxx`. The header-only special member would run in the consumer.
- The provider module stays loaded for every live value.
- The type is not a raw pointer, a reference, a standard container or string, a standard smart pointer, `std::function`, or a standard tuple / optional / variant. Base rejects those even if someone registers them. The veto applies through `Safe::Shared`, `Safe::Unique`, `Safe::Weak` and Safe collections. Use the Safe counterpart.
- A `MaybeSafe` element also meets the construction, copy, assignment and movement requirements of the wrapper that stores it. `Safe::Unique<T>` and `Safe::Weak<T>` are not collection values. `Safe::Shared<T>` is, and that does not certify the pointee.

Base cannot find a banned member hidden inside a user class. That stays on the provider who wrote the macro. A wrong registration compiles and fails at the boundary.

Derived exceptions are `MaybeSafe` for the same reason: define each named destructor out of line in its module so the `typeinfo` has one anchor. `Exception` itself is `IsSafe`.

#### Text

`Safe::String` and `Safe::WString` (`StormByte/safe/string.hxx`, `StormByte/safe/wstring.hxx`) are owned UTF-8 and wide text. Storage is a private SDS / wide buffer with SSO, not a `std::string` member. Allocation, mutation and destruction run in Base.

Construct from a literal, `const char*` / `const wchar_t*` (null stays null), or `std::string_view` / `std::wstring_view`. Those constructors are implicit, so `Safe::Map<Safe::String, int>{{"a", 1}}` works. Views copy every code unit, including embedded NULs. `size()` is a `Size` and counts the stored sequence, not the first C-string prefix. `Bytes()` is a borrowed NUL-terminated pointer; a C API that ignores length stops at the first embedded NUL. `capacity()` / `reserve(Size)` exclude the trailing NUL and never shrink. Explicit conversion to `std::string` / `std::wstring` allocates in the caller.

```cpp
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <iostream>
#include <string>
#include <string_view>

using namespace StormByte;
using namespace StormByte::Safe;

int main() {
	String text("hello");
	if (text)
		std::cout << text << " " << static_cast<std::size_t>(text.size()) << std::endl;

	String full{std::string_view{"a\0b", 3}};
	std::string caller_copy = static_cast<std::string>(full);
	std::cout << caller_copy.size() << std::endl;

	WString wide{std::wstring_view{L"wide"}};
	std::wcout << wide << std::endl;
}
```

#### Binary

`Safe::Binary` (`StormByte/safe/binary.hxx`) is the owned raw-byte container. Use it wherever a module would otherwise put `std::vector<std::byte>` in a public signature. `BinaryData` is the old name.

Bytes live in `Safe::Vector<std::byte>` on Base's heap. Lengths and indices are `ByteSize`. `at()` throws `OutOfBoundsError`. `operator[]` is unchecked. Iterators are `std::byte*`, so `<algorithm>`, `std::ranges` and `std::span` see a contiguous range. `std::iota` does not apply: `std::byte` has no `operator++`, the same limit as `std::vector<std::byte>`.

Build from a `span`, a pointer plus `ByteSize`, a range, an initializer list, a `string_view`, or a caller-owned `std::vector<std::byte>`. An lvalue copies and leaves the vector. An rvalue copies onto Base's heap and then clears the vector: it looks like a move, it is not a heap steal. Implicit `span` views the bytes. `explicit operator std::vector<std::byte>` copies into the caller runtime. `append(Binary&&)` is a real same-heap move when `*this` is empty.

`HexDump()` and `HexDump(Size columns)` return a `Safe::String`: 8-digit offset, hex row, ASCII (non-printable as `.`). `columns` is a row width. `0` prints every byte on one line. The default is 16.

```cpp
#include <StormByte/safe/binary.hxx>
#include <StormByte/byte_size.hxx>
#include <StormByte/serializable.hxx>
#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

using namespace StormByte;

int main() {
	Safe::Binary payload{std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}};
	payload.push_back(std::byte{0x00});
	payload += payload.span().first(2);

	std::ranges::reverse(payload);
	std::sort(payload.begin(), payload.end());

	if (!payload.empty())
		payload.front() = std::byte{0x01};

	const ByteSize n = payload.size();
	const std::size_t host = n;
	std::cout << host << std::endl;
	std::cout << payload.HexDump(8) << std::endl;

	std::vector<std::byte> caller = static_cast<std::vector<std::byte>>(payload);
	Safe::Binary back{std::move(caller)};

	Safe::Binary extra{std::byte{0xFF}};
	back += std::move(extra);

	auto blob = Serializable<Safe::Binary>(back).Serialize();
	auto loaded = Serializable<Safe::Binary>::Deserialize(blob);
	if (loaded)
		std::cout << (loaded.value() == back) << std::endl;
}
```

#### Collections

These types own their nodes on the Base heap. They are not aliases of the STL containers. Iterators and mutable proxies are callback-backed. `<algorithm>` and `std::ranges` do not see a creator-owned node. A move leaves the source valid and empty and keeps the creator-module callbacks. Lvalue import copies. Rvalue import moves elements and leaves the STL source valid and empty; it does not adopt the allocator. Export is `STORMBYTE_FORCE_INLINE` and `explicit`.

##### Vector

`Safe::Vector` is the contiguous sequence. It borrows as `std::span`. `Split` fills one.

##### List

`Safe::List` is a doubly linked list. It has splice, merge, unique, sort and reverse.

##### Queue

`Safe::Queue` keeps FIFO `push` / `pop` and still exposes random-access iterators. `Explode` fills one.

##### Map

`Safe::Map` is ordered, with bounds, `node_type`, extract and merge. A moved-from map with a stateful comparator cannot be reused.

##### Set

`Safe::Set` is that map with `Monostate` as the mapped type. Iterators expose the key. It has the same `node_type`, extract and merge. The same comparator transfers the node. A different comparator copies the key.

##### UnorderedMap

`Safe::UnorderedMap` is a hash table keyed through `Safe::Hash`. A type without a `Safe::Hash` specialization is not a key.

##### UnorderedSet

`Safe::UnorderedSet` is that table with `Monostate` as the mapped type. Iterators expose the key, and that key stays const. The same hash transfers the node. A different hash copies the key.

##### Iterable

`Safe::Iterable` is a cursor for a consumer that does not want to write one (`Tracks : Iterable<Vector<Track>>`). It is not the storage of the collections above. Binary does not use it: Binary needs `std::byte*` and a `ByteSize` size.

```cpp
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/set.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/unordered_set.hxx>
#include <StormByte/safe/vector.hxx>

using namespace StormByte;

Safe::Map<Safe::String, int> scores{{"a", 1}, {"b", 2}};
Safe::Set<int> unique{1, 6, 7, 6};
Safe::UnorderedSet<int> hashed{1, 6, 7, 6};
Safe::Vector<Safe::String> names{"one", "two"};
```

#### Optional, Pair, Variant

`Safe::Optional` is zero or one value on the Base heap. It accepts Safe values, enums and `std::optional`, plus `value_or`, comparisons, `swap` and the monadic operations. Mutable `operator*` is a callback proxy. `operator->` uses a caller-owned snapshot valid for the full expression only. A wrong read throws `BadOptionalAccess`.

`Safe::Pair` value-initializes both members, supports structured bindings, and copies to and from `std::pair`.

`Safe::Variant` stores the active alternative on the Base heap. It is not a `std::variant` member. `Safe::Monostate` is the empty alternative. A move leaves the source valueless. `emplace` builds the replacement first. `get`, `get_if`, `visit` and `holds_alternative` are found by argument lookup. `std::get` and `std::visit` do not accept this type. Conversion to `std::variant` is explicit and does not steal. A wrong alternative throws `BadVariantAccess`.

#### Hash

`Safe::Hash` is a cross-module FNV-1a. Integral, enumeration, floating-point and pointer keys are closed in `hash.hxx`. `String`, `WString`, `Binary`, `Size`, `ByteSize`, `Pair`, `Optional`, `Variant` and `Monostate` are closed next to the type. A type without a specialization is not a key of `UnorderedMap` or `UnorderedSet`. The call does not throw. `long double` hashes its payload, not the padding. `std::hash` specializations delegate to it. It is not the standard library hash, and it is not required to match `std::hash<int>` in another STL.

#### Pointers and Clonable

`Shared<T>`, `Unique<T>` and `Weak<T>` (`StormByte/safe/pointers.hxx`) complement the standard smart pointers. They do not replace them. Use the standard pointers when the object does not cross a module. Use these when it must be freed on Base's heap.

`Safe::Heap::MakeShared<T>(args…)` / `Safe::Heap::MakeUnique<T>(args…)` construct `T`. `MakePointer<Derived>` constructs a derived object and owns it as the base. For `Unique`, `~Base` must be virtual in that case. There is no constructor from a raw pointer or from a standard smart pointer, and `Unique` has no `release`. `AtomicShared` default-constructs its flag.

`Shared` converts implicitly to `std::shared_ptr<T>` and keeps Base's deleter. There is no conversion back. `Unique` converts on move only to `std::unique_ptr<T, Safe::Heap::ObjectDeleter>`. `Weak` is built from a `Shared`, and `lock` returns a `Shared`.

`Safe::Clonable` (`StormByte/safe/clonable.hxx`) is not an owner. `Clonable<T>` stores a `Shared<T>`. `Clonable<T, Unique<T>>` stores a `Unique<T>`. `std::shared_ptr` and `std::unique_ptr` are not accepted as that parameter. `MakePointer` forwards to the pointer factory, so the allocation is written once. A class in another DLL may derive from `Clonable`. The Safe types carry `STORMBYTE_PUBLIC_TYPE`, so `typeid` and `dynamic_cast` agree even when the deriving module builds with `-fvisibility=hidden`. The module that defines the dynamic type must stay loaded.

```cpp
#include <StormByte/safe/clonable.hxx>
#include <memory>

using namespace StormByte::Safe;

class Shape : public Clonable<Shape> {
public:
	PointerType Clone() const override {
		return MakePointer<Shape>(*this);
	}

	PointerType Move() override {
		return MakePointer<Shape>(std::move(*this));
	}
};

void use(const Shape& shape) {
	Shape::PointerType copy = shape.Clone();
	std::shared_ptr<Shape> as_std = copy;
	(void)as_std;
}
```

#### Callbacks and owners

`Safe::Callback` and `Safe::Function<Signature>` own their context in the provider module and are copyable when the provider supplies a `noexcept Clone`. A failed clone throws `StormByte::Exception`. A thrown StormByte exception crosses unchanged; any other exception becomes `Status::Failure`. Arguments are by value or `const` lvalue reference for the duration of the call. Raw pointers and mutable references are rejected.

`Safe::Owner` is a copyable opaque state owner. Copy invokes the provider `Clone`. Destroy invokes `Destroy` in the provider. `Get()` is a borrowed pointer, invalid after move or destroy. `Owner` is `MaybeSafe`: the registration is an assertion, not a proof of the private members. An `Owner` that points at a registry object owns the callback state, not the referent. The provider keeps that referent alive, or documents the exact invalidation.

#### Wait

The wait primitives are Safe values. The call looks like the standard one. The object is not a `std::mutex`, a `std::shared_mutex`, a `std::condition_variable` or a `std::atomic`, and it is not an alias of any of them. The gate, the shared gate, the signal and the word are allocated on Base's heap, so one module can wait and another can notify without sharing a CRT. Public headers do not include `<mutex>`, `<shared_mutex>`, `<condition_variable>` or `<atomic>`.

`Thread`, `once_flag` and `future` do not cross that boundary and are not added. A `std::thread` created and joined inside one `.cxx` does not cross. `ThreadLock` stays the reentrant lock of one owner thread. It is not this wait.

##### Mutex

`Safe::Mutex` (`StormByte/safe/mutex.hxx`) is not recursive. `lock`, `try_lock` and `unlock` are the surface. `unlock` without ownership is undefined, as it is for `std::mutex`. The gate is constructed and destroyed in Base. The type is not copyable or movable.

##### SharedMutex

`Safe::SharedMutex` (`StormByte/safe/shared_mutex.hxx`) is not recursive. One exclusive owner, or several shared owners. `lock`, `try_lock` and `unlock` are the exclusive surface. `lock_shared`, `try_lock_shared` and `unlock_shared` are the shared surface. `unlock` or `unlock_shared` without that ownership is undefined. The gate is constructed and destroyed in Base. The type is not copyable or movable.

##### UniqueLock

`Safe::UniqueLock` (`StormByte/safe/unique_lock.hxx`) owns at most one `Mutex`. It is header-only: a pointer and an ownership flag, not a standard lock. The wait drops that ownership and takes it back. It does not lock a `SharedMutex`.

`defer_lock` associates the mutex and does not lock it. `try_to_lock` tries once. `adopt_lock` assumes the calling thread already owns it. `release` drops the association without unlocking. `swap` exchanges two locks. There is no `LockGuard`.

##### SharedLock

`Safe::SharedLock` (`StormByte/safe/shared_lock.hxx`) owns at most one shared hold of a `SharedMutex`. It is header-only: a pointer and an ownership flag, not a standard lock. `lock` and `unlock` take and drop that shared hold, so `ConditionVariableAny` can wait on it. The exclusive hold stays on `SharedMutex::lock`.

`defer_lock`, `try_to_lock`, `adopt_lock`, `release` and `swap` match `UniqueLock`. The destructor drops the shared hold when this lock owns it.

##### ConditionVariable

`Safe::ConditionVariable` (`StormByte/safe/condition_variable.hxx`) wakes threads waiting on a `Mutex`. It accepts only `UniqueLock`. `wait`, `wait_for`, `wait_until`, `notify_one` and `notify_all` are the surface. A spurious wake is valid.

The predicate overloads stay in the header. The callable is evaluated in the caller and never enters Base. A timed wait converts the caller's clock to nanoseconds in the caller. The result is `Safe::CvStatus` (`NoTimeout` or `Timeout`), not `std::cv_status`.

##### ConditionVariableAny

`Safe::ConditionVariableAny` (`StormByte/safe/condition_variable_any.hxx`) is not a `ConditionVariable`. It accepts any lock with `lock()` and `unlock()`, including `UniqueLock` and `SharedLock`. The surface is the same: `wait`, `wait_for`, `wait_until`, `notify_one` and `notify_all`. A spurious wake is valid. The predicate stays in the caller.

The signal keeps its own gate on Base's heap. A notify takes that gate before signalling, so the wake is not lost between dropping the caller's lock and parking. The caller's lock is dropped and retaken in the header. No standard lock type is exported.

##### Atomic

`Safe::Atomic<T>` (`StormByte/safe/atomic.hxx`) is a word of one, two, four or eight bytes. `T` is a trivially copyable integer, `bool`, enum or object pointer. The template copies bytes in the caller. The word lives on Base's heap.

`load`, `store`, `exchange`, `compare_exchange_weak`, `compare_exchange_strong`, `wait`, `notify_one`, `notify_all` and `is_lock_free` are the common surface. An omitted order is sequential consistency. Integral `T` also has `fetch_add`, `fetch_sub`, `fetch_and`, `fetch_or`, `fetch_xor` and the matching operators. An object pointer adds and subtracts elements. `void*` does not. Orders are `Safe::MemoryOrder`. A release or acq-rel failure order is promoted to acquire. `wait(captured)` has no predicate: the caller loops. `is_lock_free` reports the Base word, not a `std::atomic` in the caller.

```cpp
#include <StormByte/safe/atomic.hxx>
#include <StormByte/safe/condition_variable.hxx>
#include <StormByte/safe/condition_variable_any.hxx>
#include <StormByte/safe/mutex.hxx>
#include <StormByte/safe/shared_lock.hxx>
#include <StormByte/safe/shared_mutex.hxx>
#include <StormByte/safe/unique_lock.hxx>
#include <chrono>

using namespace StormByte;

void wait_for_generation(Safe::Atomic<std::size_t>& generation) {
	const auto captured = generation.load();
	generation.wait(captured);
}

void publish(Safe::Atomic<std::size_t>& generation) {
	generation.fetch_add(std::size_t{1}, Safe::MemoryOrder::Release);
	generation.notify_one();
}

void wait_until_ready(Safe::Mutex& mutex, Safe::ConditionVariable& condition, bool& ready) {
	Safe::UniqueLock lock(mutex);
	condition.wait(lock, [&]() { return ready; });
}

bool wait_a_while(Safe::Mutex& mutex, Safe::ConditionVariable& condition, bool& ready) {
	Safe::UniqueLock lock(mutex);
	return condition.wait_for(lock, std::chrono::milliseconds(40), [&]() { return ready; });
}

void wait_shared(Safe::SharedMutex& mutex, Safe::ConditionVariableAny& condition, bool& ready) {
	Safe::SharedLock lock(mutex);
	condition.wait(lock, [&]() { return ready; });
}
```

### Size

`Size` is an **abstract unit count**, not an octet length. Storage is `uint64_t`, the same width on 32-bit and 64-bit hosts, and safe to return across a DLL.

Use it for “how many characters”, “how many items”, “how many steps”. Octet lengths belong to `ByteSize`.

Implicit conversion exists only to `std::size_t` (clamped to `size_t::max`). Every other integral destination is `explicit` and clamps to `T::max`. There is no `Value()` and no `operator bool`.

`Size{100}` is valid. A negative integer is undefined and `assert`s when assertions are on.

All arithmetic with another `Size` or with any `Type::Integral` yields `Size`. Mixed `==` / `<=>` with integers and with `ByteSize` compare the numeric counts. `std::size_t n = size_a + 3 * size_b;` works because the sum is a `Size` and that converts implicitly.

`operator Safe::String` / `operator Safe::WString` print the raw count.

```cpp
#include <StormByte/safe/string.hxx>
#include <StormByte/size.hxx>
#include <iostream>

using namespace StormByte;

int main() {
	const Size chars{5};
	const Size more = chars + 3;
	const std::size_t host = more * 2;
	if (chars == 5 && 5 == chars)
		std::cout << static_cast<Safe::String>(more) << " " << host << std::endl;
}
```

### ByteSize

`ByteSize` is an **octet length**. Storage is `uint64_t`, the same width on every host, and safe to return across a DLL.

Implicit conversion exists only to `std::size_t` (clamped). Every other integral destination is `explicit`. There is no `Value()` and no `operator bool`.

Area products (`ByteSize * ByteSize`) are deleted: two lengths do not make a length. Scaling by a `Size` or by an integer is allowed and yields `ByteSize`.

IEC factories live on the type (`ByteSize::KiB(1)`). Free constants live in `StormByte` so `1 * KiB` and `2 * MiB` work after `using namespace StormByte`. SI constants (`KB`…`EB`) are the same pattern.

`operator Safe::String` / `operator Safe::WString` print IEC text: `0 B`, `1023 B`, `1.00 KiB`, `1.50 MiB`. Only the `B` unit stays without decimals.

```cpp
#include <StormByte/byte_size.hxx>
#include <StormByte/safe/string.hxx>
#include <iostream>

using namespace StormByte;

int main() {
	const ByteSize chunk = 4 * MiB + 512 * KiB;
	const ByteSize twice = chunk * 2;
	const ByteSize pieces = twice / 1024;
	const ByteSize leftover = twice % 1024;
	const std::size_t host = chunk;

	std::cout << chunk << std::endl;
	std::cout << static_cast<Safe::String>(twice) << " " << pieces << " " << leftover << std::endl;
	std::cout << host << std::endl;

	if ((1 * KiB) == ByteSize{1024} && chunk > 1 * MiB)
		std::cout << "units" << std::endl;
}
```

### Serialization

Wire is little-endian. `Serialize()` returns `Safe::Binary`. `Deserialize` reads a prefix; leftover bytes stay with the caller. Custom types specialize `StormByte::Detail::Codec<T>` (`Size` returns `ByteSize` / `Write` / `Read`), not `Serializable<T>`.

Built-in generic serialization supports `std::optional` and `Safe::Optional` with identical presence/value framing, pair-like values including `Safe::Pair`, iterable containers including `Safe::Vector`, `Safe::Map` and `Safe::Set`, and FIFO queues including `Safe::Queue` (count followed by values in pop order). `Safe::Set` shares the `std::set` wire. A container with a `hasher` (`Safe::UnorderedSet`, `Safe::UnorderedMap`, and the `std` counterparts) writes the count and then the elements sorted by key, so the wire does not follow the bucket order. Safe collection iterators are snapshotted into their declared `value_type`; decoding uses each type's public insertion API. `Safe::Shared`, `Safe::Unique`, `Safe::Weak`, `Safe::Callback` and `Safe::Clonable` are not generically serializable: pointer identity, callback context and dynamic ownership have no portable value encoding.

`Safe::Binary` is a `Type::Container` of `std::byte`. No `Codec` specialization is required; the container path writes the same layout as `std::vector<std::byte>`.

```cpp
#include <StormByte/serializable.hxx>
#include <iostream>
#include <string>
#include <vector>

using namespace StormByte;

int main() {
	int number = 42;
	auto blob = Serializable<int>(number).Serialize();
	auto back = Serializable<int>::Deserialize(blob);
	if (back)
		std::cout << back.value() << std::endl;

	std::string text = "Hello, World!";
	auto sblob = Serializable<std::string>(text).Serialize();
	auto sback = Serializable<std::string>::Deserialize(sblob);

	std::vector<int> numbers{1, 2, 3};
	auto vblob = Serializable<std::vector<int>>(numbers).Serialize();
	auto vback = Serializable<std::vector<int>>::Deserialize(vblob.span());
}
```

`wstring` / `u16string` / `u32string` travel as `uint64` UTF-8 length + UTF-8 bytes. Host `wchar_t` width never appears on the wire. `Safe::String` and `Safe::WString` keep embedded NULs and use the same string wire.

### UUID

```cpp
#include <StormByte/uuid.hxx>
#include <iostream>

int main() {
	std::cout << StormByte::GenerateUUIDv4() << std::endl;
}
```

### ThreadLock

The owner may `Lock()` again. Another thread blocks. `Unlock()` from a non-owner does nothing. This is not the cross-module wait. That lives under [Wait](#wait).

### Type concepts

```cpp
#include <StormByte/safe/binary.hxx>
#include <StormByte/byte_size.hxx>
#include <StormByte/size.hxx>
#include <StormByte/type_traits.hxx>
#include <string>
#include <vector>
#include <optional>

using namespace StormByte;

static_assert(Type::String<std::string>);
static_assert(Type::String<Safe::String>);
static_assert(Type::Container<std::vector<int>>);
static_assert(Type::Container<Safe::Binary>);
static_assert(Type::Sized<Safe::Binary>);
static_assert(Type::Numeral<Size>);
static_assert(Type::Numeral<ByteSize>);
static_assert(Type::Optional<std::optional<int>>);
```

`Type::Detail::swap_endian` always reverses bytes. Serializable decides when to call it (host not little-endian).

### Bitmask

Needs an unsigned scoped enum. Operators return the derived CRTP type. Helpers are `Add`, `Remove`, `Has`, `HasAny`, `HasNone`, `Value` (not `Any` / `None`).

```cpp
#include <StormByte/bitmask.hxx>

using namespace StormByte;

enum class MyFlags : uint8_t { FlagA = 0x01, FlagB = 0x02 };

class MyBitmask : public Bitmask<MyBitmask, MyFlags> {
public:
	using Bitmask<MyBitmask, MyFlags>::Bitmask;
};
```

### Telemetry

`Telemetry` is the derive-and-extend session object for operations across the StormByte suite. Named clocks aggregate independent samples; a thread-safe drawer finds the aggregate, and each sample owns its own start time. Concurrent and nested samples with the same name cannot replace or stop one another. Modules add their own domain counters and metrics.

```cpp
#include <StormByte/telemetry.hxx>

using namespace StormByte;

class MyTelemetry final : public Telemetry {
public:
	void TrackJob() {
		auto sample = MeasureClock("job");
		// ... perform work ...
		(void)sample.Stop();
	}

	operator Safe::String() const override {
		return Safe::String(std::string_view(std::string("job_count=") + std::to_string(Clock("job").Count())));
	}
};
```

`Clock::Sample` is move-only and records exactly once, on explicit `Stop()` or destruction. Its token may move to the thread that completes it; do not concurrently access one token from multiple threads. Independent tokens from the same named clock may overlap freely. `Clock::GetValues()` returns Count, cumulative Time and MeanDuration from one coherent snapshot. There is no shared-clock `Start()` / `Stop()` pair: use `Clock::Measure()` or `Telemetry::MeasureClock(name)` so every stop belongs to its own start.

## Contributing

Issues and pull requests belong on this repository. Fork and open a PR against `master`.

Read [CONTRIBUTING.md](CONTRIBUTING.md) before you send a patch (copyright assignment and review rules). Coding rules are in [CODING_STYLE.md](CODING_STYLE.md).

## License

Since 2.0.0, original source in this repository is dual-licensed: GNU Lesser General Public License v3 or later, or a commercial license from the copyright holder (David C. Manuelda <StormByte@gmail.com>).

The grant applies only to original StormByte source in this repository. It does not cover other StormByte modules or third-party material shipped here (including everything under `thirdparty/`), which remains under its own license. Neither license grants patent rights.

See [LICENSE](LICENSE) for the dual-license notice and [COPYING.LGPLv3](COPYING.LGPLv3) for the full GNU LGPL version 3 text. Also <https://www.gnu.org/licenses/lgpl-3.0.html>.

Static linking under the LGPL is described under [Installation](#installation).

## Support

StormByte is developed in spare time. Sponsorship is optional and does not buy features, priority or support.

- [GitHub Sponsors](https://github.com/sponsors/StormBytePP)
- [PayPal](https://paypal.me/StormBytePP)
