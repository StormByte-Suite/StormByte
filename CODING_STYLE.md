# StormByte coding style

This is the flavor used in Base. Other suite modules follow it unless their own file says otherwise. Match the files already in the tree when something here is silent.

## Files

Headers are `.hxx`, sources `.cxx`, template bodies `.txx` included at the bottom of the header. Start every C or C++ file with `#pragma once` in the header and with the license banner used in this repository, unchanged. CMake and Markdown do not take that banner.

Include `StormByte/…` first, then a blank line, then the standard library. Do not `using namespace` in a header. `using namespace StormByte;` in a `.cxx` after the includes is fine.

Indent with tabs. Spaces for indentation are wrong. Do not mix them to line up code; Doxygen `///<` on members may share a column by using tabs.

## Shape

Braces are K&R: the `{` sits on the same line as `class`, `struct`, `enum`, `namespace`, `if`, `for`, `while` or the function signature. `public:` / `private:` are one tab in; members one more.

A single-statement `if` / `else` / `else if` has no braces. Put `else` and `else if` on their own line, not on the same line as a closing `}`.

```
if (unit == 0 || remainder == 0)
	std::snprintf(...);
else {
	...
}
```

Pointers and references bind to the type: `const char* str`, `Safe::String& other`, `operator const char*()`. Not `char *str`.

Types, enumerations and functions are PascalCase (`Base64Encode`, `Length`, `Fault`). Macros are `SCREAMING_SNAKE` (`STORMBYTE_PUBLIC`, `WINDOWS`). One statement per line.

## Language

C++26. RAII: no bare `new` / `delete` in new code. Use Safe owners for module-owned resources.

Public templates use `StormByte::Type` concepts. Do not put `std::enable_if`, `void_t` or a raw `std::is_*` next to those concepts.

`enum class` only. Converting constructors are `explicit` unless the type already documents an implicit conversion (`Safe::String` to `std::string_view` is that case). STL-string exports are explicit and allocate in the caller's CRT. Mark `noexcept` only when it is true. Prefer `constexpr` when there is no heap and no I/O; a conversion that builds a `Safe::String` inside the DLL is not `constexpr`.

Precondition failures (`operator[]` out of range, a negative `Size`) are undefined and `assert` when assertions are on. Do not `throw` for those.

Platform tests are `#ifdef WINDOWS`, `#elifdef MACOS`, `#else`. Not `#if defined(WINDOWS)`.

No anonymous namespace in a public header.

## DLL boundary

`STORMBYTE_PUBLIC` comes **first** on a function declaration. clang-cl rejects `__declspec` after a reference return type.

```
STORMBYTE_PUBLIC Safe::String GenerateUUIDv4() noexcept;
STORMBYTE_PUBLIC const Category<Code>& category() noexcept;
static STORMBYTE_PUBLIC std::size_t Size(const std::string& data) noexcept;
```

Do not write `Safe::String STORMBYTE_PUBLIC Foo();`.

A class keeps the attribute on the type: `class STORMBYTE_PUBLIC Fault`.

Exported templates are split:

```
// header — ELF export + MSVC import/export of the declaration
extern template class STORMBYTE_PUBLIC Serializable<int>;
extern template STORMBYTE_PUBLIC Size operator*<int>(int, Unit) noexcept;
extern template STORMBYTE_PUBLIC Size::Size(int) noexcept;

// .cxx — the body lives only here; consumers must not instantiate
template class STORMBYTE_INSTANTIATE Serializable<int>;
template STORMBYTE_INSTANTIATE Size operator*<int>(int, Unit) noexcept;
template STORMBYTE_INSTANTIATE Size::Size(int) noexcept;
```

`STORMBYTE_INSTANTIATE` is `dllexport` on Windows and empty on ELF (so GCC does not warn `-Wattributes`). Never put it on the `extern` line. Never write `STORMBYTE_PUBLIC extern template`.

Do not repeat `STORMBYTE_PUBLIC` on an ordinary `.cxx` definition.

Values that leave the shared library are `Safe::String`, `Safe::WString`, `Size`, `Fault`, or a borrowed `const char*` owned by this library. Do not return `std::string` or `std::size_t` as the object that crosses the boundary. Safe text keeps STL storage private behind a PIMPL, allocated and destroyed in Base's CRT. Length-bearing views preserve embedded NUL code units; pointer-only C-string inputs end at the first NUL.

## Doxygen

Document every public declaration except `= delete`. Large classes use `@name` groups. `@ref` uses the qualified name (`StormByte::Type::Container`, `StormByte::Error::Domain`). Align member `///<` comments to the same column when they fit.

Wrap `extern template` noise in `/// @cond` / `/// @endcond` so it does not show up as a page of instantiations.

## Commits and tests

Conventional Commits in English (`feat:`, `fix:`, `docs:`, `test:`, `refactor:`). One topic per commit.

Test section banners are identical in the test body and in `main`:

```
// -------------------
// Construct
// -------------------
```

Do not invent `=== Construct ===` or print section titles with `cout`.
