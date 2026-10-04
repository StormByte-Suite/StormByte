# StormByte

![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey)
![C++26](https://img.shields.io/badge/C%2B%2B-26-00599C?logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.28+-064F8C?logo=cmake&logoColor=white)
![License: LGPL v3 or commercial](https://img.shields.io/badge/License-LGPL_v3_or_commercial-blue.svg)
[![CI](https://github.com/StormBytePP/StormByte/actions/workflows/ci.yml/badge.svg)](https://github.com/StormBytePP/StormByte/actions/workflows/ci.yml)
[![Sponsor](https://img.shields.io/badge/Sponsor-StormBytePP-ea4aaa?logo=githubsponsors)](https://github.com/sponsors/StormBytePP)

This repository is **StormByte Base**: the C++26 foundation of the StormByte suite.

It is the module every other StormByte library links. Public headers live under `StormByte/` and cover exceptions, `Expected`, little-endian serialization, `Safe::String` / `Safe::WString`, `Safe::CString` / `Safe::WCString`, CRT-safe `Safe::Vector`, `Safe::Map`, `Safe::Optional` and `Safe::Queue` containers, `BinaryData`, `Size`, `ByteSize`, UUID v4, bitmasks, DLL-safe owners and clonable types (`StormByte::Safe`), a reentrant `ThreadLock`, and the `StormByte::Type` concepts.

The suite is split on purpose. Buffer, Config, Crypto, Database, Logger, Multimedia, Network and System are **other repositories**. They depend on this one; this one does not implement them.

## What this module does

- **Exceptions** — `StormByte::Exception`. `what()` is `StormByte: …`, or `StormByte.Crypto.Crypter: …` when a parent passes the segments under `StormByte`. The text is a `Safe::String`. A final leaf adds no segment.
- **Error** — `Domain`, `Category`, `Code` and `Fault` for `std::error_code`. `Fault` is not thrown; its text is a `Safe::String`.
- **Expected** — `Expected<T, E>` on top of `std::expected`. The error is a `Safe::Shared<E>` on Base's heap. It converts to `std::shared_ptr<E>`. `Unexpected<E>("… {}", arg)` stays as it is.
- **Serialization** — `Serializable<T>` to `BinaryData`, always little-endian, no BOM and no version tag. Optional / pair / container / trivial / `Detail::Codec<T>`. On-wire lengths are `ByteSize`.
- **Safe::String / Safe::WString** — owned UTF-8 and wide text on Base's heap, with range and text helpers. Construct from `string_view` / `wstring_view` (copied) or from `Safe::CString` / `Safe::WCString` (owned buffer). `Bytes()` returns a non-owning `const char*` / `const wchar_t*`.
- **Safe::CString / Safe::WCString** — public owned NUL-terminated buffers for explicit use; `Length()` is `Size`. Located under `StormByte/safe/`.
- **BinaryData** — owned contiguous `std::byte` sequence, safe to use across a DLL boundary. Same kind of API as `std::vector<std::byte>`. Lengths and indices use `ByteSize`. `HexDump` prints offset + hex + ASCII; column count is `std::size_t`.
- **Size** — abstract unit count (`uint64_t` storage), same width on every host and safe across a DLL. Implicit only to `std::size_t`. Character counts, iteration counts, “how many items”.
- **ByteSize** — octet length (`uint64_t` storage). Implicit only to `std::size_t`. IEC / SI units (`1 * KiB`), human-readable `Safe::String` (`1.00 KiB`). Area products are deleted.
- **UUID** — RFC 4122 version 4 (`GenerateUUIDv4`).
- **Bitmask** — CRTP flags over `Type::UnsignedEnum`.
- **Safe pointers** — `Safe::Shared<T>`, `Safe::Unique<T>` and `Safe::Weak<T>` (`StormByte/safe/pointers.hxx`) complement `std::shared_ptr`, `std::unique_ptr` and `std::weak_ptr`. They do not replace them: use the standard pointers unless the object must be freed on Base's heap. `Shared` converts implicitly to `std::shared_ptr<T>` (deleter stays Base). `Unique` converts on move only to `std::unique_ptr<T, Safe::Heap::ObjectDeleter>`. No `release`, and no constructor from a raw or standard pointer. The `Heap` implementation is not installed.
- **Clonable** — `Safe::Clonable` (`StormByte/safe/clonable.hxx`), polymorphic `Clone` / `Move`. Not an owner: the result is a `Shared` or a `Unique`. Safe to derive from in another DLL.
- **ThreadLock** — owner-thread reentry; `Unlock` from a non-owner is a no-op.
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
- [Safe text and buffers](#safe-text-and-buffers)
- [BinaryData](#binarydata)
- [Size](#size)
- [ByteSize](#bytesize)
- [Serialization](#serialization)
- [UUID](#uuid)
- [ThreadLock](#threadlock)
- [Clonable](#clonable)
- [Type concepts](#type-concepts)
- [Bitmask](#bitmask)
- [Telemetry](#telemetry)
- [Contributing](#contributing)
- [License](#license)

## Installation

Needs a C++26 compiler and CMake 3.28 or newer.

```sh
git clone https://github.com/StormBytePP/StormByte.git
cd StormByte
cmake -S . -B build
cmake --build build
```

Shared vs static follows CMake `BUILD_SHARED_LIBS` (declared in `lib/`, default ON). A plain configure builds the shared library. `-DBUILD_SHARED_LIBS=OFF` builds a static archive; on Windows the headers then do not use `dllimport`.

A shared build keeps this library as its own `.so` / `.dll`. Under the LGPL that is usually the simpler way to ship: the user can replace that file. A static archive is folded into your binary. The LGPL still applies to this code; you must give the recipient a way to relink your product with a different build of this library. If that does not fit how you distribute the final product, a commercial license is available from the copyright holder (see [License](#license)).

## Usage

Headers are `#include <StormByte/….hxx>`. Namespace root is `StormByte`.

### Exceptions

Base owns the exception system other modules inherit. A throw of `Exception` reads `StormByte: …`. A parent passes `Exception::Path` (a `string_view` of its segments) and forwards the format and the arguments. It does not format. A bare string is not a path: that would be ambiguous with the format constructor. Formatting happens in the caller's translation unit; the result is copied into a `Safe::String`. Plain text constructors accept `std::string_view` or `const Safe::String&`; neither view nor caller storage is retained.

A final leaf inherits the parent constructors and adds no segment, so `EncryptException("bad key {}", id)` reads `StormByte.Crypto.Crypter: bad key …`. `DeserializeError`, `OutOfBoundsError` and `Base64Error` are leaves of the root: `StormByte: …`.

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

### Safe Text and Buffers

`Safe::String` and `Safe::WString` are the owned text types used by Base APIs (`<StormByte/safe/string.hxx>` and `<StormByte/safe/wstring.hxx>`). They copy `std::string_view` / `std::wstring_view` inputs onto Base's heap and can take `Safe::CString` / `Safe::WCString` explicitly. `Bytes()` returns a non-owning `const char*` / `const wchar_t*`. Their `size()` / `length()` observers return `Size`; conversion to `std::string` / `std::wstring` is explicit and allocates in the caller.

`Safe::CString` and `Safe::WCString` are lower-level NUL-terminated buffers for explicit C-string use. They remain public, but Base text APIs use `Safe::String` / `Safe::WString` instead.

Owned buffers. `operator bool` is true when the pointer is not null: `""` / `L""` are valid empty text; a default-constructed object is null.

Construct from `const char*` / `const wchar_t*` (null stays null), from `std::string_view` / `std::wstring_view`, and from `const std::string&` / `const std::wstring&`. Those last two **copy** onto Base's heap. They are not a heap steal. An empty `string` / view yields `""` / `L""`, not a null buffer.

`CString::Length()` / `WCString::Length()` return `Size` (character count, not octets). `operator[]` takes `Size` on both the buffers and text wrappers.

`==` / `!=` / `<=>` compare text, not addresses. Two nulls are equal; null is not equal to `""` / `L""` and orders before any text. `swap` exchanges buffers. `std::hash` hashes the text (`0` when null), so the types work in `std::set` and `std::unordered_set`.

For the buffers, `explicit operator const char*` / `const wchar_t*` has the same lifetime as `std::string::c_str()` / `std::wstring::c_str()`. Their implicit `std::string` / `std::wstring` conversions and `operator<<` are inline (caller CRT).

```cpp
#include <StormByte/safe/cstring.hxx>
#include <StormByte/size.hxx>
#include <StormByte/safe/wcstring.hxx>
#include <iostream>
#include <set>
#include <string>

using namespace StormByte;
using namespace StormByte::Safe;

int main() {
	CString text("hello");
	if (text)
		std::cout << text << " " << static_cast<std::size_t>(text.Length()) << std::endl;

	CString from_std{std::string("hello")};
	if (text == from_std && text == "hello")
		std::cout << "same text" << std::endl;

	text.Reset();
	if (!text)
		std::cout << "null" << std::endl;

	CString empty("");
	if (empty && empty.Length() == Size{0} && text < empty)
		std::cout << "empty but valid" << std::endl;

	std::set<CString> ordered{CString("b"), CString("a")};

	WCString wide(L"wide");
	std::wcout << wide << std::endl;
}
```

### Safe Containers

`Safe::Vector<T>`, `Safe::Map<K, V>`, `Safe::Optional<T>` and `Safe::Queue<T>` keep their STL storage in the module that created each object. Their public operations copy values through caller-owned output parameters; they do not expose STL iterators or references.

Each container has an explicit constructor from its corresponding STL container and an explicit conversion back. Import copies elements and never adopts a foreign allocation. Export is `STORMBYTE_FORCE_INLINE`, so the returned `std::vector`, `std::map`, `std::optional` or `std::queue` is allocated and destroyed in the caller's CRT. Rvalue STL inputs are also copied; they are not heap steals.

`Safe::String::Split` and `Safe::WString::Split` can fill a `Safe::Vector`; their `Explode` counterparts can fill a `Safe::Queue`. These overloads return `Safe::Status` and replace the destination only on success. Existing caller-local `std::vector` / `std::queue` overloads remain available. Use the Safe overload when tokens need to cross a DLL boundary.

These types require a compatible C++ ABI, packing and calling convention, and their creator module plus Base must remain loaded until all instances are destroyed. They isolate container allocations across CRTs; they do not make C++ templates independent of toolchain ABI.

```cpp
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/string.hxx>
#include <vector>

using namespace StormByte;

std::vector<Safe::String> source{Safe::String("one"), Safe::String("two")};
Safe::Vector<Safe::String> safeValues(source);
std::vector<Safe::String> callerValues = static_cast<std::vector<Safe::String>>(safeValues);

Safe::Queue<Safe::String> tokens;
const auto status = Safe::String("a|b").Explode('|', tokens);
```

### BinaryData

`BinaryData` is the suite’s owned raw-byte container. Use it wherever a module would otherwise put `std::vector<std::byte>` in a public signature.

`std::vector` is not a safe ABI type between two copies of a C++ runtime. A vector allocated in the application and grown, returned or destroyed inside a StormByte shared library (or the other way around) uses two heaps. On Windows that is a hard crash when CRTs differ; on Unix it fails when libc++ and libstdc++ mix.

`BinaryData` owns its storage on StormByte Base’s heap. Construction, growth and destruction always run in this library. Other suite modules can carry payloads, encoded blobs, file images or wire fragments without exporting `std::vector<std::byte>`.

It is not text (`CString`) and not a structured document. Lengths and indices are `StormByte::ByteSize`. Member names stay lowercase to match the STL.

For `<algorithm>` and `std::ranges` it supports everything `std::vector<std::byte>` supports on a contiguous sequence of bytes: copy / transform / sort / reverse / rotate / unique / remove / replace / partition / heap / set operations / binary search / permutations, plus iterators, `std::span` and insert / erase / assign / append / `operator+=` / emplace. `std::iota` is the exception that is *also* true of `std::vector<std::byte>`: `std::byte` is an enum class and has no `operator++`.

`at()` throws `OutOfBoundsError`. `operator[]` is unchecked, like `std::vector`, and takes `ByteSize`.

Compare with another `BinaryData` or with `std::span<const std::byte>` (`==`, `!=`, `<=>`, both operand orders).

**Hex dump.** `HexDump()` and `HexDump(Size columns)` return a `Safe::String`. Each line is an 8-digit offset, a row of hex bytes, and the same bytes as ASCII (non-printable as `.`). `columns` is a **row width**, not a byte length. `0` prints every byte on one line. The default is 16 columns.

**`std::vector` and `std::span`.** You can build a `BinaryData` from a `span` or from a caller-owned `vector`. You can view the bytes as a `span` (implicit). You can copy them out to a `vector` (`explicit operator std::vector<std::byte>`). The rvalue overloads *look* like a move: the source is emptied after the copy. They are not a heap steal. Base cannot donate its pointer to a foreign `vector`, and it cannot adopt a caller `vector` pointer. Peak usage is two copies during the transfer.

`append(BinaryData&&)` / `operator+=(BinaryData&&)` is different: both sides live on Base’s heap, so that move is real when `*this` is empty.

`Serializable<BinaryData>` uses the container path. The wire is the same as `std::vector<std::byte>`: `uint64` little-endian count, then the payload.

```cpp
#include <StormByte/binary_data.hxx>
#include <StormByte/byte_size.hxx>
#include <StormByte/serializable.hxx>
#include <algorithm>
#include <iostream>
#include <ranges>
#include <span>
#include <vector>

using namespace StormByte;

int main() {
	BinaryData payload{std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}};
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
	BinaryData back{std::move(caller)};

	BinaryData extra{std::byte{0xFF}};
	back += std::move(extra);

	auto blob = Serializable<BinaryData>(back).Serialize();
	auto loaded = Serializable<BinaryData>::Deserialize(blob);
	if (loaded)
		std::cout << (loaded.value() == back) << std::endl;
}
```

```cpp
#include <StormByte/binary_data.hxx>
#include <algorithm>
#include <array>

using namespace StormByte;

BinaryData from_range() {
	const std::array<unsigned char, 4> raw{1, 2, 3, 4};
	BinaryData data(raw);
	data.insert(data.begin() + 1, std::byte{9});
	data.erase(data.begin() + 2);
	return data;
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

Wire is little-endian. `Serialize()` returns `BinaryData`. `Deserialize` reads a prefix; leftover bytes stay with the caller. Custom types specialize `StormByte::Detail::Codec<T>` (`Size` returns `ByteSize` / `Write` / `Read`), not `Serializable<T>`.

`BinaryData` is a `Type::Container` of `std::byte`. No `Codec` specialization is required; the container path writes the same layout as `std::vector<std::byte>`.

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

`wstring` / `u16string` / `u32string` travel as `uint64` UTF-8 length + UTF-8 bytes. Host `wchar_t` width never appears on the wire.

### UUID

```cpp
#include <StormByte/uuid.hxx>
#include <iostream>

int main() {
	std::cout << StormByte::GenerateUUIDv4() << std::endl;
}
```

### ThreadLock

The owner may `Lock()` again. Another thread blocks. `Unlock()` from a non-owner does nothing.

### Safe pointers

`Shared<T>`, `Unique<T>` and `Weak<T>` live in `StormByte::Safe` (`StormByte/safe/pointers.hxx`) and complement the standard smart pointers. They do not replace them. Use `std::shared_ptr`, `std::unique_ptr` and `std::weak_ptr` when the object does not cross a DLL. Use these when the object must be freed on Base's heap. The heap implementation is private and is not installed.

`Safe::Heap::MakeShared<T>(args…)` / `Safe::Heap::MakeUnique<T>(args…)` construct `T`. `MakePointer<Derived>` constructs a derived object and owns it as the base. For `Unique`, `~Base` must be virtual in that case. There is no constructor from a raw pointer or from a standard smart pointer, and `Unique` has no `release`.

The daily operations match the standard ones, so a port is a signature change. `Shared` also converts implicitly to `std::shared_ptr<T>` and keeps Base's deleter, so a parameter that is already `std::shared_ptr<T>` does not have to change. There is no conversion back. `Unique` converts on move only to `std::unique_ptr<T, Safe::Heap::ObjectDeleter>`. A `std::unique_ptr<T>` parameter has to change. `Weak` is built from a `Shared`, and `lock` returns a `Shared`.

### Clonable

`Safe::Clonable` (`StormByte/safe/clonable.hxx`) is not an owner and it is not a smart pointer. `Shared` and `Unique` own the object. `Clonable` is the polymorphic interface: from a base you can `Clone` or `Move` and get the dynamic type back, without naming the derived class. `MakePointer` forwards to `Shared::MakePointer` or `Unique::MakePointer`, so the allocation is written once.

`Clonable<T>` stores a `Shared<T>`. `Clonable<T, Unique<T>>` stores a `Unique<T>`. `std::shared_ptr` and `std::unique_ptr` are not accepted as that parameter. `~T` is virtual because `Clone` and `Move` are.

A class in another DLL may derive from `Clonable`. Storage always goes through Base's exported heap, and the `Safe` types carry `STORMBYTE_PUBLIC_TYPE` so their `typeinfo` and vtables have default visibility on Linux and macOS: every module agrees on `typeid` and `dynamic_cast`, even when the deriving module builds with `-fvisibility=hidden`. Export `T` from its own module (`class MYLIB_PUBLIC Shape : public Safe::Clonable<Shape>`). The module that defines the dynamic type must stay loaded while any of its objects exist.

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

class Token : public Clonable<Token, Unique<Token>> {
public:
	PointerType Clone() const override {
		return MakePointer<Token>(*this);
	}

	PointerType Move() override {
		return MakePointer<Token>(std::move(*this));
	}
};

void use(const Shape& shape) {
	Shape::PointerType copy = shape.Clone();
	std::shared_ptr<Shape> as_std = copy;
	(void)as_std;
}
```

### Type concepts

```cpp
#include <StormByte/binary_data.hxx>
#include <StormByte/byte_size.hxx>
#include <StormByte/size.hxx>
#include <StormByte/type_traits.hxx>
#include <string>
#include <vector>
#include <optional>

using namespace StormByte;

static_assert(Type::String<std::string>);
static_assert(Type::Container<std::vector<int>>);
static_assert(Type::Container<BinaryData>);
static_assert(Type::Sized<BinaryData>);
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

`Telemetry` is the derive-and-extend session object for operations across the StormByte suite. Clocks are protected named stopwatches (`Clock`) managed in a thread-safe drawer, while modules add their own domain counters and metrics.

```cpp
#include <StormByte/telemetry.hxx>

using namespace StormByte;

class MyTelemetry final : public Telemetry {
public:
	void TrackJob() {
		Clock("job").Start();
		// ... perform work ...
		Clock("job").Stop();
	}

	operator Safe::String() const override {
		return Safe::String(std::string_view(std::string("job_count=") + std::to_string(Clock("job").Count())));
	}
};
```

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
