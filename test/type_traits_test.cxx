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

#include <StormByte/byte_size.hxx>
#include <StormByte/size.hxx>
#include <StormByte/safe/optional.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/string.hxx>
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
#include <queue>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
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
	MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;
	MoveOnlyValue(MoveOnlyValue&&) = default;
	MoveOnlyValue& operator=(MoveOnlyValue&&) = default;
};

struct UnorderedValue {};

struct SizeCounted {
	using value_type = std::byte;
	std::byte* begin();
	std::byte* end();
	const std::byte* begin() const;
	const std::byte* end() const;
	Size size() const;
	std::byte& operator[](const Size& index);
};

bool returns_bool() { return true; }
int returns_int() { return 1; }
void returns_void() {}

enum UnscopedEnum { UE_A = 1 };
enum class ScopedEnum : std::uint16_t { A = 2 };
enum class SignedScoped : int { B = -1 };

// -------------------
// Containers
// -------------------

int test_string_concept() {
	constexpr std::string_view test = "test_string_concept";
	int result = 0;
	ASSERT_TRUE(test, Type::String<std::string>);
	ASSERT_TRUE(test, Type::String<std::wstring>);
	ASSERT_TRUE(test, Type::String<Safe::String>);
	ASSERT_TRUE(test, Type::String<Safe::WString>);
	ASSERT_FALSE(test, Type::String<const char*>);
	ASSERT_FALSE(test, (Type::String<std::vector<char>>));
	ASSERT_FALSE(test, Type::String<int>);
	RETURN_TEST(test, result);
}

int test_container_excludes_string() {
	constexpr std::string_view test = "test_container_excludes_string";
	int result = 0;
	ASSERT_TRUE(test, (Type::Container<std::vector<int>>));
	ASSERT_TRUE(test, (Type::Container<std::deque<int>>));
	ASSERT_TRUE(test, (Type::Container<std::list<int>>));
	ASSERT_TRUE(test, (Type::Container<std::map<int, int>>));
	ASSERT_TRUE(test, (Type::Container<std::set<int>>));
	ASSERT_FALSE(test, Type::Container<std::string>);
	ASSERT_FALSE(test, Type::Container<std::wstring>);
	ASSERT_FALSE(test, Type::Container<Safe::String>);
	ASSERT_FALSE(test, Type::Container<Safe::WString>);
	ASSERT_FALSE(test, Type::Container<int>);
	ASSERT_FALSE(test, Type::Container<void*>);
	RETURN_TEST(test, result);
}

int test_array_container_behaviour() {
	constexpr std::string_view test = "test_array_container_behaviour";
	int result = 0;
	ASSERT_TRUE(test, (Type::Container<std::array<int, 3>>));
	ASSERT_FALSE(test, (Type::HasPushBack<std::array<int, 3>>));
	ASSERT_FALSE(test, (Type::HasPushFront<std::array<int, 3>>));
	ASSERT_FALSE(test, (Type::HasInsert<std::array<int, 3>>));
	ASSERT_TRUE(test, (Type::HasSubscript<std::array<int, 3>, std::size_t>));
	RETURN_TEST(test, result);
}

int test_has_push_back_sequences() {
	constexpr std::string_view test = "test_has_push_back_sequences";
	int result = 0;
	ASSERT_TRUE(test, (Type::HasPushBack<std::vector<int>>));
	ASSERT_TRUE(test, (Type::HasPushBack<std::deque<int>>));
	ASSERT_TRUE(test, (Type::HasPushBack<std::list<int>>));
	ASSERT_FALSE(test, (Type::HasPushBack<std::map<int, int>>));
	ASSERT_FALSE(test, (Type::HasPushBack<std::set<int>>));
	ASSERT_FALSE(test, Type::HasPushBack<std::string>);
	RETURN_TEST(test, result);
}

int test_has_push_front_sequences() {
	constexpr std::string_view test = "test_has_push_front_sequences";
	int result = 0;
	ASSERT_FALSE(test, (Type::HasPushFront<std::vector<int>>));
	ASSERT_TRUE(test, (Type::HasPushFront<std::deque<int>>));
	ASSERT_TRUE(test, (Type::HasPushFront<std::list<int>>));
	ASSERT_FALSE(test, (Type::HasPushFront<std::map<int, int>>));
	ASSERT_FALSE(test, (Type::HasPushFront<std::set<int>>));
	RETURN_TEST(test, result);
}

int test_has_insert_associative_only() {
	constexpr std::string_view test = "test_has_insert_associative_only";
	int result = 0;
	ASSERT_TRUE(test, (Type::HasInsert<std::map<int, int>>));
	ASSERT_TRUE(test, (Type::HasInsert<std::unordered_map<int, int>>));
	ASSERT_TRUE(test, (Type::HasInsert<std::set<int>>));
	ASSERT_TRUE(test, (Type::HasInsert<std::unordered_set<int>>));
	ASSERT_FALSE(test, (Type::HasInsert<std::vector<int>>));
	ASSERT_FALSE(test, (Type::HasInsert<std::deque<int>>));
	ASSERT_FALSE(test, (Type::HasInsert<std::list<int>>));
	ASSERT_FALSE(test, Type::HasInsert<std::string>);
	RETURN_TEST(test, result);
}

int test_mutation_concepts_accept_move_only() {
	constexpr std::string_view test = "test_mutation_concepts_accept_move_only";
	int result = 0;
	ASSERT_TRUE(test, (Type::HasPushBack<std::vector<int>>));
	ASSERT_TRUE(test, (Type::HasPushBack<std::vector<std::unique_ptr<int>>>));
	ASSERT_TRUE(test, (Type::HasPushFront<std::deque<std::unique_ptr<int>>>));
	ASSERT_FALSE(test, (Type::HasPushFront<std::vector<std::unique_ptr<int>>>));
	ASSERT_TRUE(test, (Type::HasInsert<std::set<int>>));
	ASSERT_FALSE(test, (Type::HasInsert<std::vector<std::unique_ptr<int>>>));
	RETURN_TEST(test, result);
}

int test_cvref_decay_on_container_concepts() {
	constexpr std::string_view test = "test_cvref_decay_on_container_concepts";
	int result = 0;
	ASSERT_TRUE(test, (Type::HasPushBack<std::vector<int>&>));
	ASSERT_TRUE(test, (Type::HasPushBack<const std::vector<int>>));
	ASSERT_TRUE(test, (Type::HasPushBack<const std::vector<int>&>));
	ASSERT_FALSE(test, (Type::HasPushFront<std::vector<int>&>));
	ASSERT_FALSE(test, (Type::HasPushFront<const std::vector<int>&>));
	ASSERT_TRUE(test, (Type::HasInsert<std::map<int, int>&>));
	ASSERT_TRUE(test, (Type::HasInsert<const std::map<int, int>&>));
	ASSERT_FALSE(test, (Type::HasInsert<std::vector<int>&>));
	RETURN_TEST(test, result);
}

int test_add_path_is_exclusive() {
	constexpr std::string_view test = "test_add_path_is_exclusive";
	int result = 0;
	ASSERT_TRUE(test, (Type::HasPushBack<std::vector<int>>));
	ASSERT_FALSE(test, (Type::HasPushFront<std::vector<int>>));
	ASSERT_FALSE(test, (Type::HasInsert<std::vector<int>>));
	ASSERT_FALSE(test, (Type::HasPushBack<std::map<std::string, int>>));
	ASSERT_FALSE(test, (Type::HasPushFront<std::map<std::string, int>>));
	ASSERT_TRUE(test, (Type::HasInsert<std::map<std::string, int>>));
	ASSERT_TRUE(test, (Type::HasPushBack<std::deque<int>>));
	ASSERT_TRUE(test, (Type::HasPushFront<std::deque<int>>));
	ASSERT_FALSE(test, (Type::HasInsert<std::deque<int>>));
	RETURN_TEST(test, result);
}

int test_has_subscript_keys_and_indexes() {
	constexpr std::string_view test = "test_has_subscript_keys_and_indexes";
	int result = 0;
	ASSERT_TRUE(test, (Type::HasSubscript<std::vector<int>, std::size_t>));
	ASSERT_TRUE(test, (Type::HasSubscript<std::deque<int>, std::size_t>));
	ASSERT_FALSE(test, (Type::HasSubscript<std::list<int>, std::size_t>));
	ASSERT_TRUE(test, (Type::HasSubscript<std::map<int, int>, int>));
	ASSERT_TRUE(test, (Type::HasSubscript<std::unordered_map<int, int>, int>));
	ASSERT_FALSE(test, (Type::HasSubscript<std::set<int>, int>));
	ASSERT_TRUE(test, (Type::HasSubscript<SizeCounted, Size>));
	RETURN_TEST(test, result);
}

int test_has_key_and_mapped_type() {
	constexpr std::string_view test = "test_has_key_and_mapped_type";
	int result = 0;
	ASSERT_TRUE(test, (Type::HasKeyType<std::map<int, int>>));
	ASSERT_TRUE(test, (Type::HasKeyType<std::unordered_map<int, int>>));
	ASSERT_TRUE(test, (Type::HasKeyType<std::set<int>>));
	ASSERT_FALSE(test, (Type::HasKeyType<std::vector<int>>));
	ASSERT_TRUE(test, (Type::HasMappedType<std::map<int, int>>));
	ASSERT_TRUE(test, (Type::HasMappedType<std::unordered_map<int, int>>));
	ASSERT_FALSE(test, (Type::HasMappedType<std::set<int>>));
	ASSERT_FALSE(test, (Type::HasMappedType<std::vector<int>>));
	RETURN_TEST(test, result);
}

int test_sized_accepts_size_and_size_t() {
	constexpr std::string_view test = "test_sized_accepts_size_and_size_t";
	int result = 0;
	ASSERT_TRUE(test, Type::Sized<std::vector<int>>);
	ASSERT_TRUE(test, Type::Sized<SizeCounted>);
	ASSERT_TRUE(test, Type::Container<SizeCounted>);
	ASSERT_FALSE(test, Type::Sized<int>);
	RETURN_TEST(test, result);
}

int test_optional_pair_and_variant() {
	constexpr std::string_view test = "test_optional_pair_and_variant";
	int result = 0;
	using V = std::variant<int, std::string, double>;
	ASSERT_TRUE(test, Type::Optional<std::optional<int>>);
	ASSERT_TRUE(test, (Type::Optional<std::optional<std::string>>));
	ASSERT_TRUE(test, Type::Optional<Safe::Optional<Safe::String>>);
	ASSERT_FALSE(test, Type::Optional<int>);
	ASSERT_FALSE(test, (Type::Optional<std::vector<int>>));
	ASSERT_TRUE(test, Type::Queue<Safe::Queue<Safe::String>>);
	ASSERT_TRUE(test, Type::Queue<std::queue<int>>);
	ASSERT_FALSE(test, Type::Queue<std::vector<int>>);
	ASSERT_TRUE(test, (Type::Pair<std::pair<int, int>>));
	ASSERT_TRUE(test, (Type::Pair<std::pair<std::string, double>>));
	ASSERT_FALSE(test, Type::Pair<int>);
	ASSERT_FALSE(test, (Type::Pair<std::vector<int>>));
	ASSERT_TRUE(test, Type::Variant<V>);
	ASSERT_FALSE(test, Type::Variant<int>);
	ASSERT_FALSE(test, (Type::Variant<std::optional<int>>));
	ASSERT_TRUE(test, (Type::VariantHasType<V, int>));
	ASSERT_TRUE(test, (Type::VariantHasType<V, std::string>));
	ASSERT_TRUE(test, (Type::VariantHasType<V, double>));
	ASSERT_TRUE(test, (Type::VariantHasType<V, const int>));
	ASSERT_TRUE(test, (Type::VariantHasType<V, int&>));
	ASSERT_FALSE(test, (Type::VariantHasType<V, float>));
	ASSERT_FALSE(test, (Type::VariantHasType<V, char*>));
	RETURN_TEST(test, result);
}

// -------------------
// Enums
// -------------------

int test_enum_concepts() {
	constexpr std::string_view test = "test_enum_concepts";
	int result = 0;
	ASSERT_TRUE(test, Type::Enum<UnscopedEnum>);
	ASSERT_TRUE(test, Type::Enum<ScopedEnum>);
	ASSERT_TRUE(test, Type::ScopedEnum<ScopedEnum>);
	ASSERT_FALSE(test, Type::ScopedEnum<UnscopedEnum>);
	ASSERT_TRUE(test, Type::UnsignedEnum<ScopedEnum>);
	ASSERT_FALSE(test, Type::UnsignedEnum<SignedScoped>);
	ASSERT_EQUAL(test, 2, static_cast<int>(Type::ToUnderlying(ScopedEnum::A)));
	ASSERT_EQUAL(test, -1, Type::ToUnderlying(SignedScoped::B));
	ASSERT_TRUE(test, (Type::SameAs<Type::UnderlyingType<ScopedEnum>, std::uint16_t>));
	RETURN_TEST(test, result);
}

// -------------------
// Categories
// -------------------

int test_categories() {
	constexpr std::string_view test = "test_categories";
	int result = 0;
	ASSERT_TRUE(test, Type::Reference<int&>);
	ASSERT_TRUE(test, Type::Reference<int&&>);
	ASSERT_FALSE(test, Type::Reference<int>);
	ASSERT_TRUE(test, Type::LvalueReference<const int&>);
	ASSERT_FALSE(test, Type::LvalueReference<int&&>);
	ASSERT_TRUE(test, Type::RvalueReference<int&&>);
	ASSERT_FALSE(test, Type::RvalueReference<int&>);
	ASSERT_TRUE(test, Type::Pointer<int*>);
	ASSERT_FALSE(test, Type::Pointer<int>);
	ASSERT_TRUE(test, Type::SmartPointer<std::shared_ptr<int>>);
	ASSERT_TRUE(test, Type::SmartPointer<std::unique_ptr<int>>);
	ASSERT_FALSE(test, Type::SmartPointer<int*>);
	ASSERT_TRUE(test, Type::NullablePointer<std::shared_ptr<int>>);
	ASSERT_TRUE(test, Type::NullablePointer<std::unique_ptr<int>>);
	ASSERT_FALSE(test, Type::NullablePointer<MoveOnlyValue>);
	ASSERT_TRUE(test, Type::Integral<int>);
	ASSERT_TRUE(test, Type::Integral<bool>);
	ASSERT_FALSE(test, Type::Integral<double>);
	ASSERT_TRUE(test, Type::FloatingPoint<double>);
	ASSERT_FALSE(test, Type::FloatingPoint<int>);
	ASSERT_TRUE(test, Type::Arithmetic<float>);
	ASSERT_TRUE(test, Type::Signed<int>);
	ASSERT_FALSE(test, Type::Signed<bool>);
	ASSERT_TRUE(test, Type::Unsigned<unsigned>);
	ASSERT_FALSE(test, Type::Unsigned<int>);
	ASSERT_TRUE(test, Type::Numeral<int>);
	ASSERT_TRUE(test, Type::Numeral<std::size_t>);
	ASSERT_TRUE(test, Type::Numeral<Size>);
	ASSERT_TRUE(test, Type::Numeral<ByteSize>);
	ASSERT_FALSE(test, Type::Numeral<double>);
	ASSERT_TRUE(test, Type::Const<const int>);
	ASSERT_FALSE(test, Type::Const<int>);
	ASSERT_FALSE(test, Type::Const<const int&>);
	ASSERT_TRUE(test, Type::Class<Safe::String>);
	ASSERT_FALSE(test, Type::Class<int>);
	RETURN_TEST(test, result);
}

// -------------------
// Object semantics
// -------------------

int test_object_semantics() {
	constexpr std::string_view test = "test_object_semantics";
	int result = 0;
	ASSERT_TRUE(test, Type::TriviallyCopyable<int>);
	ASSERT_FALSE(test, Type::TriviallyCopyable<Safe::String>);
	ASSERT_TRUE(test, Type::TriviallyDestructible<int>);
	ASSERT_FALSE(test, Type::TriviallyDestructible<Safe::String>);
	ASSERT_TRUE(test, Type::Destructible<Safe::String>);
	ASSERT_TRUE(test, Type::Destructible<int>);
	ASSERT_TRUE(test, Type::DefaultConstructible<int>);
	ASSERT_TRUE(test, Type::TriviallyDefaultConstructible<int>);
	ASSERT_FALSE(test, Type::TriviallyDefaultConstructible<Safe::String>);
	ASSERT_TRUE(test, (Type::CopyConstructible<std::vector<int>>));
	ASSERT_FALSE(test, (Type::CopyConstructible<std::vector<std::unique_ptr<int>>>));
	ASSERT_TRUE(test, Type::TriviallyCopyConstructible<int>);
	ASSERT_TRUE(test, Type::MoveConstructible<MoveOnlyValue>);
	ASSERT_FALSE(test, Type::CopyConstructible<MoveOnlyValue>);
	ASSERT_TRUE(test, Type::TriviallyMoveConstructible<int>);
	ASSERT_TRUE(test, (Type::CopyAssignable<std::vector<int>>));
	ASSERT_FALSE(test, (Type::CopyAssignable<std::vector<std::unique_ptr<int>>>));
	ASSERT_TRUE(test, Type::TriviallyCopyAssignable<int>);
	ASSERT_TRUE(test, Type::MoveAssignable<MoveOnlyValue>);
	ASSERT_TRUE(test, Type::TriviallyMoveAssignable<int>);
	ASSERT_TRUE(test, Type::Copyable<int>);
	ASSERT_FALSE(test, Type::Copyable<MoveOnlyValue>);
	ASSERT_TRUE(test, Type::Movable<MoveOnlyValue>);
	ASSERT_TRUE(test, Type::Swappable<int>);
	ASSERT_TRUE(test, Type::Swappable<std::string>);
	ASSERT_TRUE(test, (Type::ConstructibleFrom<Safe::String, const char*>));
	ASSERT_FALSE(test, (Type::ConstructibleFrom<int, Safe::String>));
	ASSERT_TRUE(test, (Type::AssignableFrom<int&, int>));
	ASSERT_FALSE(test, (Type::AssignableFrom<const int&, int>));
	ASSERT_TRUE(test, Type::TotallyOrdered<int>);
	ASSERT_TRUE(test, Type::TotallyOrdered<std::string>);
	ASSERT_FALSE(test, Type::TotallyOrdered<UnorderedValue>);
	ASSERT_TRUE(test, Type::Semiregular<int>);
	ASSERT_FALSE(test, Type::Semiregular<MoveOnlyValue>);
	ASSERT_TRUE(test, Type::Regular<int>);
	ASSERT_FALSE(test, Type::Regular<MoveOnlyValue>);
	ASSERT_FALSE(test, Type::Regular<UnorderedValue>);
	RETURN_TEST(test, result);
}

// -------------------
// Relations
// -------------------

int test_relations() {
	constexpr std::string_view test = "test_relations";
	int result = 0;
	ASSERT_TRUE(test, (Type::Callable<decltype(returns_int)>));
	ASSERT_TRUE(test, (Type::Callable<int(*)(int), int>));
	ASSERT_FALSE(test, (Type::Callable<decltype(returns_int), int>));
	ASSERT_FALSE(test, (Type::Callable<int, int>));
	ASSERT_TRUE(test, (Type::Invocable<decltype(returns_bool)>));
	ASSERT_FALSE(test, (Type::Invocable<decltype(returns_bool), int>));
	ASSERT_FALSE(test, (Type::Invocable<int, int>));
	ASSERT_TRUE(test, (Type::InvocableR<int, decltype(returns_int)>));
	ASSERT_FALSE(test, (Type::InvocableR<Safe::String, decltype(returns_int)>));
	ASSERT_TRUE(test, (Type::Predicate<decltype(returns_bool)>));
	ASSERT_TRUE(test, (Type::Predicate<decltype(returns_int)>));
	ASSERT_FALSE(test, (Type::Predicate<decltype(returns_void)>));
	ASSERT_TRUE(test, (Type::SameAs<int, int>));
	ASSERT_TRUE(test, (Type::SameAs<int, const int&>));
	ASSERT_FALSE(test, (Type::SameAs<int, long>));
	ASSERT_TRUE(test, (Type::DerivedFrom<TraitDerived, TraitBase>));
	ASSERT_TRUE(test, (Type::DerivedFrom<const TraitDerived, const TraitBase>));
	ASSERT_TRUE(test, (Type::DerivedFrom<TraitBase, TraitBase>));
	ASSERT_FALSE(test, (Type::DerivedFrom<TraitUnrelated, TraitBase>));
	ASSERT_TRUE(test, (Type::ConvertibleTo<int, long>));
	ASSERT_FALSE(test, (Type::ConvertibleTo<Safe::String, int>));
	ASSERT_FALSE(test, (Type::ConvertibleTo<int, std::byte>));
	ASSERT_TRUE(test, (Type::ExplicitlyConvertibleTo<int, std::byte>));
	ASSERT_TRUE(test, (Type::ExplicitlyConvertibleTo<Size, std::size_t>));
	RETURN_TEST(test, result);
}

// -------------------
// Comparison
// -------------------

int test_comparison() {
	constexpr std::string_view test = "test_comparison";
	int result = 0;
	ASSERT_TRUE(test, Type::EqualityComparable<int>);
	ASSERT_TRUE(test, Type::EqualityComparable<Safe::String>);
	ASSERT_FALSE(test, Type::EqualityComparable<UnorderedValue>);
	ASSERT_TRUE(test, Type::ThreeWayComparable<int>);
	ASSERT_FALSE(test, Type::ThreeWayComparable<UnorderedValue>);
	ASSERT_TRUE(test, Type::Hashable<int>);
	ASSERT_TRUE(test, Type::Hashable<std::string>);
	ASSERT_FALSE(test, Type::Hashable<std::vector<int>>);
	ASSERT_FALSE(test, Type::Hashable<UnorderedValue>);
	RETURN_TEST(test, result);
}

// -------------------
// Ranges
// -------------------

int test_ranges() {
	constexpr std::string_view test = "test_ranges";
	int result = 0;
	using Values = std::vector<int>;
	using Iterator = Values::iterator;
	using ListIterator = std::list<int>::iterator;
	using Output = std::back_insert_iterator<Values>;
	ASSERT_TRUE(test, (Type::Range<Values>));
	ASSERT_TRUE(test, (Type::Range<Values&>));
	ASSERT_FALSE(test, (Type::Range<int>));
	ASSERT_TRUE(test, (Type::InputRange<Values>));
	ASSERT_TRUE(test, (Type::OutputRange<Values, int>));
	ASSERT_TRUE(test, (Type::ForwardRange<Values>));
	ASSERT_TRUE(test, (Type::BidirectionalRange<std::list<int>>));
	ASSERT_TRUE(test, (Type::RandomAccessRange<Values>));
	ASSERT_FALSE(test, (Type::RandomAccessRange<std::list<int>>));
	ASSERT_TRUE(test, (Type::ContiguousRange<Values>));
	ASSERT_TRUE(test, (Type::SizedRange<Values>));
	ASSERT_TRUE(test, (Type::CommonRange<Values>));
	ASSERT_TRUE(test, (Type::View<std::ranges::ref_view<Values>>));
	ASSERT_FALSE(test, (Type::View<Values>));
	ASSERT_TRUE(test, (Type::BorrowedRange<Values&>));
	ASSERT_FALSE(test, (Type::BorrowedRange<Values>));
	ASSERT_TRUE(test, (Type::InputIterator<Iterator>));
	ASSERT_FALSE(test, (Type::InputIterator<int>));
	ASSERT_TRUE(test, (Type::OutputIterator<Output, int>));
	ASSERT_TRUE(test, (Type::ForwardIterator<Iterator>));
	ASSERT_TRUE(test, (Type::BidirectionalIterator<ListIterator>));
	ASSERT_TRUE(test, (Type::RandomAccessIterator<Iterator>));
	ASSERT_TRUE(test, (Type::ContiguousIterator<Iterator>));
	ASSERT_FALSE(test, (Type::ContiguousIterator<ListIterator>));
	ASSERT_TRUE(test, (Type::SentinelFor<Iterator, Iterator>));
	ASSERT_TRUE(test, (Type::SizedSentinelFor<Iterator, Iterator>));
	ASSERT_TRUE(test, (Type::SameAs<Type::RangeValue<Values>, int>));
	ASSERT_TRUE(test, (Type::SameAs<Type::RangeReference<Values>, int&>));
	ASSERT_TRUE(test, (Type::SameAs<Type::RangeDifference<Values>, std::ptrdiff_t>));
	ASSERT_TRUE(test, (Type::SameAs<Type::IteratorValue<Iterator>, int>));
	ASSERT_TRUE(test, (Type::ByteInputRange<std::vector<int>>));
	ASSERT_TRUE(test, (Type::ByteInputRange<std::vector<std::byte>>));
	ASSERT_TRUE(test, (Type::ByteInputIterator<Iterator>));
	ASSERT_FALSE(test, (Type::ByteInputRange<int>));
	ASSERT_FALSE(test, (Type::ByteInputRange<std::vector<std::string>>));
	ASSERT_FALSE(test, (Type::ByteInputIterator<std::vector<std::string>::iterator>));
	RETURN_TEST(test, result);
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
	result += test_add_path_is_exclusive();
	result += test_has_subscript_keys_and_indexes();
	result += test_has_key_and_mapped_type();
	result += test_sized_accepts_size_and_size_t();
	result += test_optional_pair_and_variant();

	// -------------------
	// Enums
	// -------------------

	result += test_enum_concepts();

	// -------------------
	// Categories
	// -------------------

	result += test_categories();

	// -------------------
	// Object semantics
	// -------------------

	result += test_object_semantics();

	// -------------------
	// Relations
	// -------------------

	result += test_relations();

	// -------------------
	// Comparison
	// -------------------

	result += test_comparison();

	// -------------------
	// Ranges
	// -------------------

	result += test_ranges();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
