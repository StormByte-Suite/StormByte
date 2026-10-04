/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte source in this repository.
 * They do not cover other StormByte modules or any third-party material
 * shipped with this repository (including everything under thirdparty/),
 * which remains under its own license.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#include <StormByte/size.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/optional.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits.hxx>

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

using namespace StormByte;

struct TraitBase {};
struct TraitDerived: TraitBase {};
struct TraitUnrelated {};

struct MoveOnlyValue {
	MoveOnlyValue() = default;
	MoveOnlyValue(const MoveOnlyValue&) = delete;
	MoveOnlyValue(MoveOnlyValue&&) = default;
};

struct SizeCounted {
	using value_type = std::byte;
	std::byte* begin();
	std::byte* end();
	const std::byte* begin() const;
	const std::byte* end() const;
	Size size() const;
	std::byte& operator[](const Size& index);
};

template<typename T>
constexpr bool is_string_v = Type::String<T>;
template<typename T>
constexpr bool is_container_v = Type::Container<T>;
template<typename T>
constexpr bool has_push_back_v = Type::HasPushBack<T>;
template<typename T>
constexpr bool has_push_front_v = Type::HasPushFront<T>;
template<typename T>
constexpr bool has_insert_v = Type::HasInsert<T>;
template<typename T, typename U>
constexpr bool has_subscript_v = Type::HasSubscript<T, U>;
template<typename T>
constexpr bool has_key_type_v = Type::HasKeyType<T>;
template<typename T>
constexpr bool has_mapped_type_v = Type::HasMappedType<T>;
template<typename T>
constexpr bool is_optional_v = Type::Optional<T>;
template<typename T>
constexpr bool is_pair_v = Type::Pair<T>;
template<typename T>
constexpr bool is_variant_v = Type::Variant<T>;
template<typename T, typename U>
constexpr bool variant_has_type_v = Type::VariantHasType<T, U>;

// -------------------
// Containers
// -------------------

int test_string_concept() {
	int result = 0;
	ASSERT_TRUE("test_string_concept", is_string_v<std::string>);
	ASSERT_TRUE("test_string_concept", is_string_v<std::wstring>);
	ASSERT_TRUE("test_string_concept", is_string_v<Safe::String>);
	ASSERT_TRUE("test_string_concept", is_string_v<Safe::WString>);
	ASSERT_FALSE("test_string_concept", is_string_v<const char*>);
	ASSERT_FALSE("test_string_concept", (is_string_v<std::vector<char>>));
	ASSERT_FALSE("test_string_concept", is_string_v<int>);
	RETURN_TEST("test_string_concept", result);
}

int test_container_excludes_string() {
	int result = 0;
	ASSERT_TRUE("test_container_excludes_string", (is_container_v<std::vector<int>>));
	ASSERT_TRUE("test_container_excludes_string", (is_container_v<std::deque<int>>));
	ASSERT_TRUE("test_container_excludes_string", (is_container_v<std::list<int>>));
	ASSERT_TRUE("test_container_excludes_string", (is_container_v<std::map<int, int>>));
	ASSERT_TRUE("test_container_excludes_string", (is_container_v<std::set<int>>));
	ASSERT_FALSE("test_container_excludes_string", is_container_v<std::string>);
	ASSERT_FALSE("test_container_excludes_string", is_container_v<std::wstring>);
	ASSERT_FALSE("test_container_excludes_string", is_container_v<Safe::String>);
	ASSERT_FALSE("test_container_excludes_string", is_container_v<Safe::WString>);
	ASSERT_FALSE("test_container_excludes_string", is_container_v<int>);
	ASSERT_FALSE("test_container_excludes_string", is_container_v<void*>);
	RETURN_TEST("test_container_excludes_string", result);
}

int test_array_container_behaviour() {
	int result = 0;
	ASSERT_TRUE("test_array_container_behaviour", (is_container_v<std::array<int, 3>>));
	ASSERT_FALSE("test_array_container_behaviour", (has_push_back_v<std::array<int, 3>>));
	ASSERT_FALSE("test_array_container_behaviour", (has_push_front_v<std::array<int, 3>>));
	ASSERT_FALSE("test_array_container_behaviour", (has_insert_v<std::array<int, 3>>));
	ASSERT_TRUE("test_array_container_behaviour", (has_subscript_v<std::array<int, 3>, std::size_t>));
	RETURN_TEST("test_array_container_behaviour", result);
}

int test_has_push_back_sequences() {
	int result = 0;
	ASSERT_TRUE("test_has_push_back_sequences", (has_push_back_v<std::vector<int>>));
	ASSERT_TRUE("test_has_push_back_sequences", (has_push_back_v<std::deque<int>>));
	ASSERT_TRUE("test_has_push_back_sequences", (has_push_back_v<std::list<int>>));
	ASSERT_FALSE("test_has_push_back_sequences", (has_push_back_v<std::map<int, int>>));
	ASSERT_FALSE("test_has_push_back_sequences", (has_push_back_v<std::set<int>>));
	ASSERT_FALSE("test_has_push_back_sequences", has_push_back_v<std::string>);
	RETURN_TEST("test_has_push_back_sequences", result);
}

int test_has_push_front_sequences() {
	int result = 0;
	ASSERT_FALSE("test_has_push_front_sequences", (has_push_front_v<std::vector<int>>));
	ASSERT_TRUE("test_has_push_front_sequences", (has_push_front_v<std::deque<int>>));
	ASSERT_TRUE("test_has_push_front_sequences", (has_push_front_v<std::list<int>>));
	ASSERT_FALSE("test_has_push_front_sequences", (has_push_front_v<std::map<int, int>>));
	ASSERT_FALSE("test_has_push_front_sequences", (has_push_front_v<std::set<int>>));
	RETURN_TEST("test_has_push_front_sequences", result);
}

int test_has_insert_associative_only() {
	int result = 0;
	ASSERT_TRUE("test_has_insert_associative_only", (has_insert_v<std::map<int, int>>));
	ASSERT_TRUE("test_has_insert_associative_only", (has_insert_v<std::unordered_map<int, int>>));
	ASSERT_TRUE("test_has_insert_associative_only", (has_insert_v<std::set<int>>));
	ASSERT_TRUE("test_has_insert_associative_only", (has_insert_v<std::unordered_set<int>>));
	ASSERT_FALSE("test_has_insert_associative_only", (has_insert_v<std::vector<int>>));
	ASSERT_FALSE("test_has_insert_associative_only", (has_insert_v<std::deque<int>>));
	ASSERT_FALSE("test_has_insert_associative_only", (has_insert_v<std::list<int>>));
	ASSERT_FALSE("test_has_insert_associative_only", has_insert_v<std::string>);
	RETURN_TEST("test_has_insert_associative_only", result);
}

int test_mutation_concepts_accept_move_only() {
	int result = 0;
	ASSERT_TRUE("test_mutation_concepts_accept_move_only", (Type::HasPushBack<std::vector<int>>));
	ASSERT_TRUE("test_mutation_concepts_accept_move_only", (Type::HasPushBack<std::vector<std::unique_ptr<int>>>));
	ASSERT_TRUE("test_mutation_concepts_accept_move_only", (Type::HasPushFront<std::deque<std::unique_ptr<int>>>));
	ASSERT_FALSE("test_mutation_concepts_accept_move_only", (Type::HasPushFront<std::vector<std::unique_ptr<int>>>));
	ASSERT_TRUE("test_mutation_concepts_accept_move_only", (Type::HasInsert<std::set<int>>));
	ASSERT_FALSE("test_mutation_concepts_accept_move_only", (Type::HasInsert<std::vector<std::unique_ptr<int>>>));
	RETURN_TEST("test_mutation_concepts_accept_move_only", result);
}

int test_cvref_decay_on_container_concepts() {
	int result = 0;
	ASSERT_TRUE("test_cvref_decay_on_container_concepts", (has_push_back_v<std::vector<int>&>));
	ASSERT_TRUE("test_cvref_decay_on_container_concepts", (has_push_back_v<const std::vector<int>>));
	ASSERT_TRUE("test_cvref_decay_on_container_concepts", (has_push_back_v<const std::vector<int>&>));
	ASSERT_FALSE("test_cvref_decay_on_container_concepts", (has_push_front_v<std::vector<int>&>));
	ASSERT_FALSE("test_cvref_decay_on_container_concepts", (has_push_front_v<const std::vector<int>&>));
	ASSERT_TRUE("test_cvref_decay_on_container_concepts", (has_insert_v<std::map<int, int>&>));
	ASSERT_TRUE("test_cvref_decay_on_container_concepts", (has_insert_v<const std::map<int, int>&>));
	ASSERT_FALSE("test_cvref_decay_on_container_concepts", (has_insert_v<std::vector<int>&>));
	RETURN_TEST("test_cvref_decay_on_container_concepts", result);
}

int test_vector_add_path_is_push_back_only() {
	int result = 0;
	constexpr bool pb = Type::HasPushBack<std::vector<int>>;
	constexpr bool pf = Type::HasPushFront<std::vector<int>>;
	constexpr bool ins = Type::HasInsert<std::vector<int>>;
	ASSERT_TRUE("test_vector_add_path_is_push_back_only", pb);
	ASSERT_FALSE("test_vector_add_path_is_push_back_only", pf);
	ASSERT_FALSE("test_vector_add_path_is_push_back_only", ins);
	ASSERT_EQUAL("test_vector_add_path_is_push_back_only", 1, (pb ? 1 : 0) + (pf ? 1 : 0) + (ins ? 1 : 0));
	RETURN_TEST("test_vector_add_path_is_push_back_only", result);
}

int test_map_add_path_is_insert_only() {
	int result = 0;
	constexpr bool pb = Type::HasPushBack<std::map<std::string, int>>;
	constexpr bool pf = Type::HasPushFront<std::map<std::string, int>>;
	constexpr bool ins = Type::HasInsert<std::map<std::string, int>>;
	ASSERT_FALSE("test_map_add_path_is_insert_only", pb);
	ASSERT_FALSE("test_map_add_path_is_insert_only", pf);
	ASSERT_TRUE("test_map_add_path_is_insert_only", ins);
	ASSERT_EQUAL("test_map_add_path_is_insert_only", 1, (pb ? 1 : 0) + (pf ? 1 : 0) + (ins ? 1 : 0));
	RETURN_TEST("test_map_add_path_is_insert_only", result);
}

int test_deque_may_have_both_push_apis() {
	int result = 0;
	ASSERT_TRUE("test_deque_may_have_both_push_apis", (has_push_back_v<std::deque<int>>));
	ASSERT_TRUE("test_deque_may_have_both_push_apis", (has_push_front_v<std::deque<int>>));
	ASSERT_FALSE("test_deque_may_have_both_push_apis", (has_insert_v<std::deque<int>>));
	RETURN_TEST("test_deque_may_have_both_push_apis", result);
}

int test_has_key_and_mapped_type() {
	int result = 0;
	ASSERT_TRUE("test_has_key_and_mapped_type", (has_key_type_v<std::map<int, int>>));
	ASSERT_TRUE("test_has_key_and_mapped_type", (has_mapped_type_v<std::map<int, int>>));
	ASSERT_TRUE("test_has_key_and_mapped_type", (has_key_type_v<std::set<int>>));
	ASSERT_FALSE("test_has_key_and_mapped_type", (has_mapped_type_v<std::set<int>>));
	ASSERT_FALSE("test_has_key_and_mapped_type", (has_key_type_v<std::vector<int>>));
	ASSERT_FALSE("test_has_key_and_mapped_type", (has_mapped_type_v<std::vector<int>>));
	RETURN_TEST("test_has_key_and_mapped_type", result);
}

int test_has_subscript() {
	int result = 0;
	ASSERT_TRUE("test_has_subscript", (has_subscript_v<std::vector<int>, std::size_t>));
	ASSERT_TRUE("test_has_subscript", (has_subscript_v<std::deque<int>, std::size_t>));
	ASSERT_TRUE("test_has_subscript", (has_subscript_v<std::map<int, int>, int>));
	ASSERT_TRUE("test_has_subscript", (has_subscript_v<SizeCounted, Size>));
	ASSERT_FALSE("test_has_subscript", (has_subscript_v<std::list<int>, std::size_t>));
	ASSERT_FALSE("test_has_subscript", (has_subscript_v<std::set<int>, int>));
	RETURN_TEST("test_has_subscript", result);
}

int test_sized_accepts_size_and_size_t() {
	int result = 0;
	ASSERT_TRUE("test_sized_accepts_size_and_size_t", Type::Sized<std::vector<int>>);
	ASSERT_TRUE("test_sized_accepts_size_and_size_t", Type::Sized<SizeCounted>);
	ASSERT_TRUE("test_sized_accepts_size_and_size_t", Type::Container<SizeCounted>);
	ASSERT_FALSE("test_sized_accepts_size_and_size_t", Type::Sized<int>);
	RETURN_TEST("test_sized_accepts_size_and_size_t", result);
}

// -------------------
// Wrappers
// -------------------

int test_optional_concept() {
	int result = 0;
	ASSERT_TRUE("test_optional_concept", (is_optional_v<std::optional<int>>));
	ASSERT_TRUE("test_optional_concept", (is_optional_v<std::optional<std::string>>));
	ASSERT_TRUE("test_optional_concept", Type::Optional<Safe::Optional<Safe::String>>);
	ASSERT_FALSE("test_optional_concept", is_optional_v<int>);
	ASSERT_FALSE("test_optional_concept", (is_optional_v<std::vector<int>>));
	ASSERT_TRUE("test_optional_concept", Type::Queue<Safe::Queue<Safe::String>>);
	ASSERT_TRUE("test_optional_concept", Type::Queue<std::queue<int>>);
	RETURN_TEST("test_optional_concept", result);
}

int test_pair_concept() {
	int result = 0;
	ASSERT_TRUE("test_pair_concept", (is_pair_v<std::pair<int, int>>));
	ASSERT_TRUE("test_pair_concept", (is_pair_v<std::pair<std::string, double>>));
	ASSERT_FALSE("test_pair_concept", is_pair_v<int>);
	ASSERT_FALSE("test_pair_concept", (is_pair_v<std::vector<int>>));
	RETURN_TEST("test_pair_concept", result);
}

int test_variant_concepts() {
	int result = 0;
	using V = std::variant<int, std::string, double>;
	ASSERT_TRUE("test_variant_concepts", is_variant_v<V>);
	ASSERT_FALSE("test_variant_concepts", is_variant_v<int>);
	ASSERT_FALSE("test_variant_concepts", (is_variant_v<std::optional<int>>));
	ASSERT_TRUE("test_variant_concepts", (variant_has_type_v<V, int>));
	ASSERT_TRUE("test_variant_concepts", (variant_has_type_v<V, std::string>));
	ASSERT_TRUE("test_variant_concepts", (variant_has_type_v<V, double>));
	ASSERT_FALSE("test_variant_concepts", (variant_has_type_v<V, float>));
	ASSERT_FALSE("test_variant_concepts", (variant_has_type_v<V, char*>));
	ASSERT_TRUE("test_variant_concepts", (variant_has_type_v<V, const int>));
	ASSERT_TRUE("test_variant_concepts", (variant_has_type_v<V, int&>));
	RETURN_TEST("test_variant_concepts", result);
}

// -------------------
// Enums
// -------------------

enum UnscopedEnum { UE_A = 1 };
enum class ScopedEnum : std::uint16_t { A = 2 };
enum class SignedScoped : int { B = -1 };

int test_enum_concepts() {
	int result = 0;
	ASSERT_TRUE("test_enum_concepts", Type::Enum<UnscopedEnum>);
	ASSERT_TRUE("test_enum_concepts", Type::Enum<ScopedEnum>);
	ASSERT_TRUE("test_enum_concepts", Type::ScopedEnum<ScopedEnum>);
	ASSERT_FALSE("test_enum_concepts", Type::ScopedEnum<UnscopedEnum>);
	ASSERT_TRUE("test_enum_concepts", Type::UnsignedEnum<ScopedEnum>);
	ASSERT_FALSE("test_enum_concepts", Type::UnsignedEnum<SignedScoped>);
	ASSERT_EQUAL("test_enum_concepts", 2, static_cast<int>(Type::ToUnderlying(ScopedEnum::A)));
	ASSERT_EQUAL("test_enum_concepts", -1, Type::ToUnderlying(SignedScoped::B));
	ASSERT_TRUE("test_enum_concepts", (Type::SameAs<Type::UnderlyingType<ScopedEnum>, std::uint16_t>));
	RETURN_TEST("test_enum_concepts", result);
}

// -------------------
// Categories
// -------------------

int test_arithmetic_and_cv_concepts() {
	int result = 0;
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Integral<int>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::FloatingPoint<double>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Arithmetic<float>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Signed<int>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Unsigned<unsigned>);
	ASSERT_FALSE("test_arithmetic_and_cv_concepts", Type::Signed<unsigned>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Const<const int>);
	ASSERT_FALSE("test_arithmetic_and_cv_concepts", Type::Const<int>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Pointer<int*>);
	ASSERT_FALSE("test_arithmetic_and_cv_concepts", Type::Pointer<int>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Reference<int&>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Reference<int&&>);
	ASSERT_FALSE("test_arithmetic_and_cv_concepts", Type::Reference<int>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::LvalueReference<int&>);
	ASSERT_FALSE("test_arithmetic_and_cv_concepts", Type::LvalueReference<int&&>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::RvalueReference<int&&>);
	ASSERT_FALSE("test_arithmetic_and_cv_concepts", Type::RvalueReference<int&>);
	ASSERT_TRUE("test_arithmetic_and_cv_concepts", Type::Class<std::string>);
	ASSERT_FALSE("test_arithmetic_and_cv_concepts", Type::Class<int>);
	RETURN_TEST("test_arithmetic_and_cv_concepts", result);
}

// -------------------
// Conversions
// -------------------

int test_same_as_and_convertible() {
	int result = 0;
	ASSERT_TRUE("test_same_as_and_convertible", (Type::SameAs<int, int>));
	ASSERT_TRUE("test_same_as_and_convertible", (Type::SameAs<int, const int&>));
	ASSERT_FALSE("test_same_as_and_convertible", (Type::SameAs<int, long>));
	ASSERT_TRUE("test_same_as_and_convertible", (Type::ConvertibleTo<int, long>));
	ASSERT_FALSE("test_same_as_and_convertible", (Type::ConvertibleTo<std::string, int>));
	RETURN_TEST("test_same_as_and_convertible", result);
}

// -------------------
// Ranges
// -------------------

int test_range_and_byte_concepts() {
	int result = 0;
	using Values = std::vector<int>;
	using Iterator = Values::iterator;
	using ListIterator = std::list<int>::iterator;
	using Output = std::back_insert_iterator<Values>;
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::Range<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::Range<Values&>));
	ASSERT_FALSE("test_range_and_byte_concepts", (Type::Range<int>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::InputRange<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::OutputRange<Values, int>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::ForwardRange<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::BidirectionalRange<std::list<int>>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::RandomAccessRange<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::ContiguousRange<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::SizedRange<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::CommonRange<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::View<std::ranges::ref_view<Values>>));
	ASSERT_FALSE("test_range_and_byte_concepts", (Type::View<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::BorrowedRange<Values&>));
	ASSERT_FALSE("test_range_and_byte_concepts", (Type::BorrowedRange<Values>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::InputIterator<Iterator>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::OutputIterator<Output, int>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::ForwardIterator<Iterator>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::BidirectionalIterator<ListIterator>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::RandomAccessIterator<Iterator>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::ContiguousIterator<Iterator>));
	ASSERT_FALSE("test_range_and_byte_concepts", (Type::InputIterator<int>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::SentinelFor<Iterator, Iterator>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::SizedSentinelFor<Iterator, Iterator>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::SameAs<Type::RangeValue<Values>, int>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::SameAs<Type::RangeReference<Values>, int&>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::SameAs<Type::RangeDifference<Values>, std::ptrdiff_t>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::SameAs<Type::IteratorValue<Iterator>, int>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::ExplicitlyConvertibleTo<int, std::byte>));
	ASSERT_FALSE("test_range_and_byte_concepts", (Type::ConvertibleTo<int, std::byte>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::ByteInputRange<std::vector<int>>));
	ASSERT_TRUE("test_range_and_byte_concepts", (Type::ByteInputIterator<std::vector<int>::iterator>));
	ASSERT_FALSE("test_range_and_byte_concepts", (Type::ByteInputRange<int>));
	ASSERT_FALSE("test_range_and_byte_concepts", (Type::ByteInputRange<std::vector<std::string>>));
	ASSERT_FALSE("test_range_and_byte_concepts", (Type::ByteInputIterator<std::vector<std::string>::iterator>));
	RETURN_TEST("test_range_and_byte_concepts", result);
}

// -------------------
// Object semantics
// -------------------

int test_constructible_and_callable() {
	int result = 0;
	ASSERT_TRUE("test_constructible_and_callable", Type::DefaultConstructible<int>);
	ASSERT_TRUE("test_constructible_and_callable", Type::CopyConstructible<std::string>);
	ASSERT_TRUE("test_constructible_and_callable", (Type::MoveConstructible<std::vector<int>>));
	ASSERT_TRUE("test_constructible_and_callable", (Type::Callable<int(*)(int), int>));
	auto lambda = [](int x) { return x + 1; };
	ASSERT_TRUE("test_constructible_and_callable", (Type::Callable<decltype(lambda), int>));
	ASSERT_FALSE("test_constructible_and_callable", (Type::Callable<int, int>));
	RETURN_TEST("test_constructible_and_callable", result);
}

int test_trivially_copyable() {
	int result = 0;
	ASSERT_TRUE("test_trivially_copyable", Type::TriviallyCopyable<int>);
	ASSERT_TRUE("test_trivially_copyable", Type::TriviallyDestructible<int>);
	ASSERT_FALSE("test_trivially_copyable", Type::TriviallyCopyable<std::string>);
	RETURN_TEST("test_trivially_copyable", result);
}

int test_extended_type_concepts() {
	int result = 0;
	ASSERT_TRUE("test_extended_type_concepts", Type::Sized<std::vector<int>>);
	ASSERT_TRUE("test_extended_type_concepts", Type::Sized<SizeCounted>);
	ASSERT_FALSE("test_extended_type_concepts", Type::Sized<int>);
	ASSERT_TRUE("test_extended_type_concepts", Type::SmartPointer<std::unique_ptr<int>>);
	ASSERT_TRUE("test_extended_type_concepts", Type::SmartPointer<std::shared_ptr<int>>);
	ASSERT_FALSE("test_extended_type_concepts", Type::SmartPointer<int*>);
	ASSERT_TRUE("test_extended_type_concepts", Type::NullablePointer<std::unique_ptr<int>>);
	ASSERT_TRUE("test_extended_type_concepts", Type::NullablePointer<std::shared_ptr<int>>);
	ASSERT_FALSE("test_extended_type_concepts", Type::NullablePointer<MoveOnlyValue>);
	ASSERT_TRUE("test_extended_type_concepts", Type::MoveConstructible<MoveOnlyValue>);
	ASSERT_TRUE("test_extended_type_concepts", Type::Swappable<std::string>);
	ASSERT_TRUE("test_extended_type_concepts", (Type::DerivedFrom<TraitDerived, TraitBase>));
	ASSERT_TRUE("test_extended_type_concepts", (Type::DerivedFrom<const TraitDerived, const TraitBase>));
	ASSERT_TRUE("test_extended_type_concepts", (Type::DerivedFrom<TraitBase, TraitBase>));
	ASSERT_FALSE("test_extended_type_concepts", (Type::DerivedFrom<TraitUnrelated, TraitBase>));
	ASSERT_TRUE("test_extended_type_concepts", Type::EqualityComparable<int>);
	ASSERT_TRUE("test_extended_type_concepts", Type::ThreeWayComparable<int>);
	ASSERT_TRUE("test_extended_type_concepts", Type::Hashable<std::string>);
	ASSERT_FALSE("test_extended_type_concepts", Type::Hashable<std::vector<int>>);
	RETURN_TEST("test_extended_type_concepts", result);
}

int main() {
	int result = 0;

	// -------------------
	// Containers
	// -------------------
	result += test_string_concept();
	result += test_container_excludes_string();
	result += test_array_container_behaviour();
	result += test_has_push_back_sequences();
	result += test_has_push_front_sequences();
	result += test_has_insert_associative_only();
	result += test_mutation_concepts_accept_move_only();
	result += test_cvref_decay_on_container_concepts();
	result += test_vector_add_path_is_push_back_only();
	result += test_map_add_path_is_insert_only();
	result += test_deque_may_have_both_push_apis();
	result += test_has_key_and_mapped_type();
	result += test_has_subscript();
	result += test_sized_accepts_size_and_size_t();

	// -------------------
	// Wrappers
	// -------------------
	result += test_optional_concept();
	result += test_pair_concept();
	result += test_variant_concepts();

	// -------------------
	// Enums
	// -------------------
	result += test_enum_concepts();

	// -------------------
	// Categories
	// -------------------
	result += test_arithmetic_and_cv_concepts();

	// -------------------
	// Conversions
	// -------------------
	result += test_same_as_and_convertible();

	// -------------------
	// Ranges
	// -------------------
	result += test_range_and_byte_concepts();

	// -------------------
	// Object semantics
	// -------------------
	result += test_constructible_and_callable();
	result += test_trivially_copyable();
	result += test_extended_type_concepts();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
