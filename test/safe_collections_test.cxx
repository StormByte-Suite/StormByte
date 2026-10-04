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

#include "safe_collections_plugin.hxx"

#include <StormByte/binary_data.hxx>
#include <StormByte/exception.hxx>
#include <StormByte/safe/clonable.hxx>
#include <StormByte/safe/cstring.hxx>
#include <StormByte/safe/function.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/pointers.hxx>
#include <StormByte/safe/wcstring.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <optional>
#include <map>
#include <memory>
#include <queue>
#include <ranges>
#include <string>
#include <vector>

using namespace StormByte;

enum class OptionalLevel {
	Info,
	Warning
};

struct OptionalAggregateFixture {
	Safe::Optional<OptionalLevel> Level;
};

struct ConsumerMaybeSafeFixture {
	int Value = 0;
};

STORMBYTE_DECLARE_MAYBE_SAFE(ConsumerMaybeSafeFixture);

struct DirectionalStringCompare {
	bool descending = false;

	bool operator()(const Safe::String& left, const Safe::String& right) const {
		return descending ? right < left : left < right;
	}
};

static_assert(Type::SafeComponent<const Safe::String&>);
static_assert(Type::SafeComponent<Safe::CString>);
static_assert(Type::SafeComponent<Safe::WString>);
static_assert(Type::SafeComponent<Safe::WCString>);
static_assert(Type::SafeComponent<Safe::Shared<int>>);
static_assert(Type::SafeComponent<Safe::Unique<int>>);
static_assert(Type::SafeComponent<Safe::Weak<int>>);
static_assert(Type::SafeComponent<Safe::Clonable<int>>);
static_assert(Type::SafeComponent<Safe::Vector<Safe::String>>);
static_assert(Type::SafeComponent<Safe::Map<Safe::String, Safe::String>>);
static_assert(Type::SafeComponent<Safe::Optional<Safe::String>>);
static_assert(Type::SafeComponent<Safe::Queue<Safe::String>>);
static_assert(Type::SafeComponent<SafeCollectionsFixture::Nested>);
static_assert(Type::IsSafe<Exception>::value);
static_assert(Type::MaybeSafe<SafeCollectionsFixture::ProviderException>);
static_assert(!Type::IsSafe<SafeCollectionsFixture::ProviderException>::value);
static_assert(Type::MaybeSafe<std::expected<int, Exception>>);
static_assert(Type::MaybeSafe<Safe::Callback>);
static_assert(!Type::IsSafe<Safe::Callback>::value);
static_assert(Type::MaybeSafe<ConsumerMaybeSafeFixture>);
static_assert(Type::SafeValue<ConsumerMaybeSafeFixture>);
static_assert(Type::MaybeSafe<SafeCollectionsFixture::MaybeValue>);
static_assert(Type::SafeValue<SafeCollectionsFixture::MaybeValue>);
static_assert(Type::MaybeSafe<Safe::Owner>);
static_assert(Type::SafeValue<Safe::Owner>);
static_assert(Type::MaybeSafe<SafeCollectionsFixture::MaybeOwners>);
static_assert(Type::MaybeSafe<Safe::Vector<SafeCollectionsFixture::MaybeValue>>);
static_assert(Type::MaybeSafe<Safe::Shared<Safe::Vector<Safe::Shared<SafeCollectionsFixture::MaybeValue>>>>);
static_assert(Type::MaybeSafe<Safe::Map<Safe::String, SafeCollectionsFixture::MaybeValue>>);
static_assert(Type::MaybeSafe<Safe::Optional<SafeCollectionsFixture::MaybeValue>>);
static_assert(Type::MaybeSafe<Safe::Queue<SafeCollectionsFixture::MaybeValue>>);
static_assert(Type::MaybeSafe<Safe::Pair<SafeCollectionsFixture::MaybeValue, Safe::String>>);
static_assert(!Type::MaybeSafe<std::vector<int>>);
static_assert(!Type::SafeComponent<std::vector<int>>);
static_assert(Type::IsSafe<Safe::Shared<Safe::Vector<Safe::Shared<int>>>>::value);
template<typename T>
concept CanMakeSafeVector = requires { typename Safe::Vector<T>; };
static_assert(!CanMakeSafeVector<Safe::Shared<std::vector<int>>>);
using IntegralCallback = Safe::Function<void(bool, char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long)>;
static_assert(std::is_move_constructible_v<IntegralCallback>);
static_assert(!std::is_copy_constructible_v<IntegralCallback>);
static_assert(Type::MaybeSafe<Safe::Function<Size(Size)>>);
static_assert(!Type::IsSafe<Safe::Function<Size(Size)>>::value);
static_assert(Type::SafeValue<Safe::String>);
static_assert(Type::SafeValue<Safe::Shared<Safe::String>>);
static_assert(Type::SafeValue<int>);
static_assert(Type::SafeValue<bool>);
static_assert(Type::SafeValue<double>);
static_assert(Type::SafeValue<Safe::Vector<Safe::String>>);
static_assert(Type::SafeValue<Safe::Map<Safe::String, Safe::String>>);
static_assert(Type::SafeValue<Safe::Optional<Safe::String>>);
static_assert(Type::SafeValue<OptionalLevel>);
static_assert(Type::SafeValue<Safe::Optional<OptionalLevel>>);
static_assert(Type::SafeValue<Safe::Queue<Safe::String>>);
static_assert(Type::SafeValue<Safe::Pair<Safe::String, Safe::String>>);
static_assert(Type::SafeValue<BinaryData>);
static_assert(Type::SafeValue<Size>);
static_assert(Type::SafeValue<ByteSize>);
static_assert(!Type::SafeValue<const Safe::String>);
static_assert(!Type::SafeValue<Safe::String&>);
static_assert(!Type::SafeValue<Safe::Unique<Safe::String>>);
static_assert(!Type::SafeValue<Safe::Weak<Safe::String>>);
static_assert(!Type::SafeValue<Safe::Clonable<int>>);
static_assert(!Type::SafeValue<std::string>);
static_assert(!Type::SafeValue<std::vector<Safe::String>>);
static_assert(!Type::SafeValue<std::optional<Safe::String>>);
static_assert(!Type::SafeValue<std::queue<Safe::String>>);
static_assert(!Type::SafeValue<Safe::Callback>);
static_assert(!Type::SafeValue<int*>);
	static_assert(std::random_access_iterator<Safe::Vector<Safe::String>::iterator>);
	static_assert(std::sortable<Safe::Vector<Safe::String>::iterator>);
	static_assert(std::bidirectional_iterator<Safe::Map<Safe::String, Safe::String>::iterator>);
	static_assert(std::ranges::input_range<Safe::Map<Safe::String, Safe::String>>);
	static_assert(std::ranges::input_range<Safe::Optional<Safe::String>>);
	static_assert(!std::ranges::range<Safe::Queue<Safe::String>>);

namespace {
	struct OwnerFixtureState {
		int* destroyed;
	};

	void* FailOwnerClone(const void*) noexcept {
		return nullptr;
	}

	void DestroyOwnerFixtureState(void* state) noexcept {
		auto owner = std::unique_ptr<OwnerFixtureState>(static_cast<OwnerFixtureState*>(state));
		++*owner->destroyed;
	}

	StormByte::Safe::Status IgnoreCallback(void*, const StormByte::Safe::String&) noexcept {
		return StormByte::Safe::Status::Success;
	}

	void RecordCallbackRelease(void* context) noexcept {
		++*static_cast<int*>(context);
	}
}

// -------------------
// Owner copy failure
// -------------------

int TestOwnerCopyFailure() {
	int destroyed = 0;
	{
		auto sourceState = std::make_unique<OwnerFixtureState>(&destroyed);
		Safe::Detail::Owner source(sourceState.get(), &FailOwnerClone, &DestroyOwnerFixtureState);
		sourceState.release();
		auto targetState = std::make_unique<OwnerFixtureState>(&destroyed);
		Safe::Detail::Owner target(targetState.get(), &FailOwnerClone, &DestroyOwnerFixtureState);
		targetState.release();
		void* originalTarget = target.Get();
		ASSERT_THROWS("TestOwnerCopyFailure", Safe::Detail::Owner(source), Exception);
		ASSERT_THROWS("TestOwnerCopyFailure", target = source, Exception);
		ASSERT_TRUE("TestOwnerCopyFailure", source.Get() != nullptr && target.Get() == originalTarget);
		ASSERT_EQUAL("TestOwnerCopyFailure", 0, destroyed);
	}
	ASSERT_EQUAL("TestOwnerCopyFailure", 2, destroyed);
	RETURN_TEST("TestOwnerCopyFailure", 0);
}

int TestOwnerRejectsInvalidCallbacks() {
	int destroyed = 0;
	auto rejectedState = std::make_unique<OwnerFixtureState>(&destroyed);
	ASSERT_THROWS("TestOwnerRejectsInvalidCallbacks", Safe::Owner(rejectedState.get(), nullptr, nullptr), Exception);
	ASSERT_TRUE("TestOwnerRejectsInvalidCallbacks", rejectedState != nullptr);
	{
		auto ownedState = std::make_unique<OwnerFixtureState>(&destroyed);
		void* statePointer = ownedState.get();
		Safe::Owner moveOnly(statePointer, nullptr, &DestroyOwnerFixtureState);
		ownedState.release();
		ASSERT_THROWS("TestOwnerRejectsInvalidCallbacks", Safe::Owner(moveOnly), Exception);
		ASSERT_TRUE("TestOwnerRejectsInvalidCallbacks", moveOnly.Get() == statePointer);
		ASSERT_EQUAL("TestOwnerRejectsInvalidCallbacks", 0, destroyed);
	}
	ASSERT_EQUAL("TestOwnerRejectsInvalidCallbacks", 1, destroyed);
	RETURN_TEST("TestOwnerRejectsInvalidCallbacks", 0);
}

// -------------------
// Callback validation
// -------------------

int TestCallbackValidation() {
	int releaseCount = 0;
	ASSERT_THROWS("TestCallbackValidation", Safe::Callback(&releaseCount, nullptr, &RecordCallbackRelease), Exception);
	ASSERT_THROWS("TestCallbackValidation", Safe::Callback(&releaseCount, &IgnoreCallback, nullptr), Exception);
	ASSERT_THROWS("TestCallbackValidation", Safe::Callback(nullptr, &IgnoreCallback, &RecordCallbackRelease), Exception);
	ASSERT_EQUAL("TestCallbackValidation", 0, releaseCount);
	RETURN_TEST("TestCallbackValidation", 0);
}

int TestSafeVectorAlgorithms() {
	Safe::Vector<int> scalarValues{1, 2, 3};
	scalarValues[1] = 4;
	ASSERT_TRUE("TestSafeVectorAlgorithms", static_cast<int>(scalarValues[1]) == 4);

	Safe::Shared<int> sharedValue = Safe::Heap::MakeShared<int>(42);
	Safe::Vector<Safe::Shared<int>> sharedValues{sharedValue};
	sharedValue.reset();
	const auto sharedSnapshot = static_cast<Safe::Shared<int>>(sharedValues.at(0));
	ASSERT_TRUE("TestSafeVectorAlgorithms", sharedSnapshot && *sharedSnapshot == 42);

	Safe::Vector<Safe::String> listValues{Safe::String("one"), Safe::String("two")};
	Safe::Vector<Safe::String> countValues(2, Safe::String("fill"));
	countValues.assign({Safe::String("one"), Safe::String("two")});
	ASSERT_TRUE("TestSafeVectorAlgorithms", listValues == countValues && !(listValues < countValues));
	countValues.insert(countValues.cbegin() + 1, 2, Safe::String("middle"));
	ASSERT_TRUE("TestSafeVectorAlgorithms", countValues.size() == 4 && countValues[1] == "middle" && countValues[2] == "middle");
	countValues.assign({Safe::String("one"), Safe::String("two")});
	ASSERT_TRUE("TestSafeVectorAlgorithms", listValues == countValues);

	Safe::Vector<Safe::String> values(std::vector<Safe::String>{
		Safe::String("gamma"), Safe::String("alpha"), Safe::String("beta"), Safe::String("alpha")
	});
	std::vector<Safe::String> imported{Safe::String("from STL")};
	Safe::Vector<Safe::String> assigned;
	assigned = std::move(imported);
	ASSERT_TRUE("TestSafeVectorAlgorithms", imported.empty() && assigned.front() == "from STL");
	values.reserve(12);
	ASSERT_TRUE("TestSafeVectorAlgorithms", values.capacity() >= 12);
	values.emplace(values.cbegin() + 1, "inserted");
	values.insert(values.cend(), Safe::String("tail"));
	ASSERT_TRUE("TestSafeVectorAlgorithms", values.size() == 6 && values[1] == "inserted" && values.back() == "tail");
	values.pop_back();
	values.resize(2);
	values.resize(3, Safe::String("fill"));
	ASSERT_TRUE("TestSafeVectorAlgorithms", values.size() == 3 && values[2] == "fill");
	values.resize(5);
	ASSERT_TRUE("TestSafeVectorAlgorithms", values.size() == 5 && static_cast<Safe::String>(values[4]).empty());
	values.resize(4);
	values.shrink_to_fit();
	ASSERT_TRUE("TestSafeVectorAlgorithms", values.capacity() >= values.size());
	values = Safe::Vector<Safe::String>(std::vector<Safe::String>{
		Safe::String("gamma"), Safe::String("alpha"), Safe::String("beta"), Safe::String("alpha")
	});
	std::sort(values.begin(), values.end());
	ASSERT_TRUE("TestSafeVectorAlgorithms", values.size() == Size(4));
	ASSERT_TRUE("TestSafeVectorAlgorithms", std::find(values.cbegin(), values.cend(), Safe::String("beta")) != values.cend());
	auto newEnd = std::remove(values.begin(), values.end(), Safe::String("alpha"));
	values.erase(newEnd, values.end());
	const auto output = static_cast<std::vector<Safe::String>>(values);
	ASSERT_TRUE("TestSafeVectorAlgorithms", output.size() == 2);
	ASSERT_TRUE("TestSafeVectorAlgorithms", output[0] == "beta" && output[1] == "gamma");
	std::ranges::sort(values);
	RETURN_TEST("TestSafeVectorAlgorithms", 0);
}

int TestSafeMapAlgorithms() {
	Safe::Map<Safe::String, Safe::String> dictionary(std::map<Safe::String, Safe::String>{
		{Safe::String("alpha"), Safe::String("first")}, {Safe::String("beta"), Safe::String("second")}
	});
	std::map<Safe::String, Safe::String> importedMap{{Safe::String("imported"), Safe::String("value")}};
	Safe::Map<Safe::String, Safe::String> assigned;
	assigned = std::move(importedMap);
	ASSERT_TRUE("TestSafeMapAlgorithms", importedMap.empty() && assigned.at(Safe::String("imported")) == "value");
	Safe::Map<Safe::String, Safe::String> movedMap(std::move(assigned));
	ASSERT_TRUE("TestSafeMapAlgorithms", assigned.empty() && movedMap.contains(Safe::String("imported")));
	const auto exportedMovedFrom = static_cast<std::map<Safe::String, Safe::String>>(assigned);
	ASSERT_TRUE("TestSafeMapAlgorithms", exportedMovedFrom.empty());
	ASSERT_TRUE("TestSafeMapAlgorithms", dictionary.count(Safe::String("beta")) == 1 && dictionary.count(Safe::String("missing")) == 0);
	ASSERT_TRUE("TestSafeMapAlgorithms", dictionary.lower_bound(Safe::String("al")) == dictionary.begin());
	ASSERT_TRUE("TestSafeMapAlgorithms", dictionary.upper_bound(Safe::String("beta")) == dictionary.end());
	auto [equalFirst, equalLast] = dictionary.equal_range(Safe::String("beta"));
	ASSERT_TRUE("TestSafeMapAlgorithms", equalFirst != equalLast && equalFirst->first == "beta");
	equalFirst->second = Safe::String("through arrow proxy");
	ASSERT_TRUE("TestSafeMapAlgorithms", dictionary.at(Safe::String("beta")) == "through arrow proxy");
	auto [inserted, wasInserted] = dictionary.try_emplace(Safe::String("gamma"), Safe::String("third"));
	ASSERT_TRUE("TestSafeMapAlgorithms", wasInserted && inserted->second == "third");
	auto [existing, wasInsertedAgain] = dictionary.try_emplace(Safe::String("gamma"), Safe::String("ignored"));
	ASSERT_TRUE("TestSafeMapAlgorithms", !wasInsertedAgain && existing->second == "third");
	auto stable = dictionary.find(Safe::String("beta"));
	dictionary.try_emplace(Safe::String("aardvark"), Safe::String("earlier"));
	ASSERT_TRUE("TestSafeMapAlgorithms", stable->first == "beta");
	++stable;
	ASSERT_TRUE("TestSafeMapAlgorithms", stable->first == "gamma");
	const auto pairInsert = dictionary.insert(std::pair{Safe::String("delta"), Safe::String("fourth")});
	ASSERT_TRUE("TestSafeMapAlgorithms", pairInsert.second && dictionary.contains(Safe::String("delta")));
	const auto found = std::ranges::find_if(dictionary, [](const auto& entry) { return entry.first == "beta"; });
	ASSERT_TRUE("TestSafeVectorAlgorithms", found != dictionary.end());
	std::ranges::for_each(dictionary, [](auto entry) { entry.second = Safe::String("updated"); });
	Safe::String mapped;
	ASSERT_TRUE("TestSafeMapAlgorithms", dictionary.at(Safe::String("alpha")) == "updated");
	auto rangeFirst = dictionary.find(Safe::String("beta"));
	auto rangeLast = dictionary.find(Safe::String("delta"));
	dictionary.erase(rangeFirst, rangeLast);
	ASSERT_TRUE("TestSafeMapAlgorithms", !dictionary.contains(Safe::String("beta")) && dictionary.contains(Safe::String("delta")));
	Safe::Map<Safe::String, Safe::String> other;
	other.try_emplace(Safe::String("other"), Safe::String("entry"));
	dictionary.swap(other);
	ASSERT_TRUE("TestSafeMapAlgorithms", dictionary.contains(Safe::String("other")) && other.contains(Safe::String("delta")));
	using DescendingContainer = std::map<Safe::String, Safe::String, std::greater<Safe::String>>;
	Safe::Iterable<DescendingContainer> descending;
	descending.try_emplace(Safe::String("alpha"), Safe::String("first"));
	descending.try_emplace(Safe::String("beta"), Safe::String("second"));
	ASSERT_TRUE("TestSafeMapAlgorithms", descending.begin()->first == "beta" &&
		descending.find(Safe::String("alpha")) != descending.end() &&
		descending.lower_bound(Safe::String("beta"))->first == "beta");
	using StatefulContainer = std::map<Safe::String, Safe::String, DirectionalStringCompare>;
	StatefulContainer statefulSource(DirectionalStringCompare{true});
	statefulSource.emplace(Safe::String("alpha"), Safe::String("first"));
	statefulSource.emplace(Safe::String("beta"), Safe::String("second"));
	Safe::Iterable<StatefulContainer> stateful(statefulSource);
	const auto exportedStateful = static_cast<StatefulContainer>(stateful);
	ASSERT_TRUE("TestSafeMapAlgorithms", exportedStateful.key_comp().descending &&
		exportedStateful.begin()->first == "beta" && stateful.begin()->first == "beta");
	Safe::Iterable<StatefulContainer> movedStateful(std::move(stateful));
	ASSERT_THROWS("TestSafeMapAlgorithms", stateful.try_emplace(Safe::String("gamma"), Safe::String("third")), Exception);
	ASSERT_THROWS("TestSafeMapAlgorithms", static_cast<StatefulContainer>(stateful), Exception);
	ASSERT_TRUE("TestSafeMapAlgorithms", movedStateful.begin()->first == "beta");
	RETURN_TEST("TestSafeMapAlgorithms", 0);
}

int TestSafeOptionalAlgorithms() {
	Safe::Optional<Safe::String> direct = Safe::String("direct");
	ASSERT_TRUE("TestSafeOptionalAlgorithms", direct.has_value() && direct.value() == "direct");
	Safe::Optional<Safe::String> fromLiteral("literal");
	ASSERT_TRUE("TestSafeOptionalAlgorithms", fromLiteral == Safe::String("literal"));
	std::optional<Safe::CString> narrowBuffer{Safe::CString("converted optional")};
	Safe::Optional<Safe::String> convertedFromSTL(narrowBuffer);
	Safe::Optional<Safe::CString> safeBuffer(Safe::CString("converted safe"));
	Safe::Optional<Safe::String> convertedFromSafe(safeBuffer);
	convertedFromSafe = safeBuffer;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", convertedFromSTL == Safe::String("converted optional") && convertedFromSafe == Safe::String("converted safe"));
	std::optional<Safe::String> standardDirect{Safe::String("implicit import")};
	Safe::Optional<Safe::String> implicitImport = standardDirect;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", implicitImport == Safe::String("implicit import"));
	const Safe::String copiedValue("copied assignment");
	direct = copiedValue;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", direct.value() == "copied assignment");
	direct = Safe::String("assigned");
	ASSERT_TRUE("TestSafeOptionalAlgorithms", direct.value() == "assigned");
	Safe::Optional<Safe::String> empty(std::nullopt);
	ASSERT_TRUE("TestSafeOptionalAlgorithms", !empty.has_value() && empty == std::nullopt && empty.value_or(Safe::String("fallback")) == "fallback");
	ASSERT_TRUE("TestSafeOptionalAlgorithms", std::move(empty).value_or(Safe::String("rvalue fallback")) == "rvalue fallback");
	empty = Safe::String("filled");
	ASSERT_TRUE("TestSafeOptionalAlgorithms", empty.value_or(Safe::String("fallback")) == "filled");
	empty = std::nullopt;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", !empty.has_value());
	std::optional<Safe::String> importedValue{Safe::String("std optional")};
	empty = importedValue;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", empty == Safe::String("std optional"));
	std::optional<Safe::String> movedImportedValue{Safe::String("moved std optional")};
	empty = std::move(movedImportedValue);
	ASSERT_TRUE("TestSafeOptionalAlgorithms", !movedImportedValue.has_value() && empty == Safe::String("moved std optional"));
	empty = std::optional<Safe::String>{};
	ASSERT_TRUE("TestSafeOptionalAlgorithms", empty == std::nullopt);
	const auto emptyTransform = empty.transform([](const Safe::String& value) { return value; });
	const auto emptyChain = empty.and_then([](const Safe::String& value) {
		return Safe::Optional<Safe::String>(value);
	});
	ASSERT_TRUE("TestSafeOptionalAlgorithms", emptyTransform == std::nullopt && emptyChain == std::nullopt);

	OptionalAggregateFixture aggregate{.Level = OptionalLevel::Info};
	Safe::Optional<OptionalLevel> enumValue = OptionalLevel::Info;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", aggregate.Level == enumValue && enumValue == OptionalLevel::Info);
	enumValue = OptionalLevel::Warning;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", enumValue.value() == OptionalLevel::Warning);
	ASSERT_TRUE("TestSafeOptionalAlgorithms", enumValue > OptionalLevel::Info && std::nullopt < enumValue);
	const Safe::Optional<OptionalLevel> constEnum = enumValue;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", *constEnum == OptionalLevel::Warning);
	Safe::Optional<OptionalLevel> inPlace(std::in_place, OptionalLevel::Info);
	ASSERT_TRUE("TestSafeOptionalAlgorithms", inPlace == OptionalLevel::Info && inPlace != std::nullopt);
	auto transformed = enumValue.transform([](const OptionalLevel& level) {
		return level == OptionalLevel::Warning ? OptionalLevel::Info : level;
	});
	auto chained = enumValue.and_then([](const OptionalLevel& level) {
		return Safe::Optional<OptionalLevel>(level);
	});
	auto recovered = Safe::Optional<OptionalLevel>{}.or_else([] {
		return Safe::Optional<OptionalLevel>(OptionalLevel::Warning);
	});
	ASSERT_TRUE("TestSafeOptionalAlgorithms", transformed == OptionalLevel::Info && chained == enumValue && recovered == enumValue);
	auto consumedTransform = std::move(enumValue).transform([](OptionalLevel&& level) { return level; });
	ASSERT_TRUE("TestSafeOptionalAlgorithms", consumedTransform == OptionalLevel::Warning);
	auto consumedChain = std::move(consumedTransform).and_then([](OptionalLevel&& level) {
		return Safe::Optional<OptionalLevel>(level);
	});
	ASSERT_TRUE("TestSafeOptionalAlgorithms", consumedChain == OptionalLevel::Warning);
	inPlace.swap(enumValue);
	ASSERT_TRUE("TestSafeOptionalAlgorithms", inPlace == OptionalLevel::Warning && enumValue == OptionalLevel::Info);
	auto recoveredRvalue = std::move(enumValue).or_else([] { return Safe::Optional<OptionalLevel>(OptionalLevel::Info); });
	ASSERT_TRUE("TestSafeOptionalAlgorithms", recoveredRvalue == OptionalLevel::Info);

	Safe::Optional<Safe::String> maybe(std::optional<Safe::String>{Safe::String("before")});
	const std::optional<std::string_view> standardView{std::string_view("before")};
	ASSERT_TRUE("TestSafeOptionalStandardComparison", maybe == standardView && standardView == maybe);
	ASSERT_TRUE("TestSafeOptionalValueComparison", maybe == std::string_view("before") && std::string_view("before") == maybe);
	ASSERT_TRUE("TestSafeOptionalNulloptOrdering", maybe <=> std::nullopt > 0);
	ASSERT_TRUE("TestSafeOptionalAlgorithms", std::ranges::find(maybe, Safe::String("before")) != maybe.end());
	*maybe.begin() = Safe::String("after");
	ASSERT_TRUE("TestSafeOptionalAlgorithms", maybe.value() == "after");
	*maybe = Safe::String("through dereference proxy");
	const Safe::String dereferenced = *maybe;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", maybe.value() == "through dereference proxy" && dereferenced == maybe.value());
	ASSERT_TRUE("TestSafeOptionalAlgorithms", *maybe == Safe::String("through dereference proxy") && maybe->starts_with("through"));
	const auto mutatedMonadic = maybe.transform([](Safe::String& value) {
		value = Safe::String("mutated monadic snapshot");
		return value;
	});
	ASSERT_TRUE("TestSafeOptionalAlgorithms", mutatedMonadic == maybe && maybe == Safe::String("mutated monadic snapshot"));
	const auto mutatedChain = maybe.and_then([](Safe::String& value) {
		value = Safe::String("mutated and_then snapshot");
		return Safe::Optional<Safe::String>(value);
	});
	ASSERT_TRUE("TestSafeOptionalAlgorithms", mutatedChain == maybe && maybe == Safe::String("mutated and_then snapshot"));
	ASSERT_THROWS("TestSafeOptionalAlgorithms", *empty, Exception);
	ASSERT_THROWS("TestSafeOptionalAlgorithms", empty->size(), Exception);
	const auto copied = maybe;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", copied == maybe && copied == Safe::String("mutated and_then snapshot"));
	Safe::Optional<Safe::String> copyAssigned;
	copyAssigned = copied;
	ASSERT_TRUE("TestSafeOptionalAlgorithms", copyAssigned == maybe);
	Safe::Optional<Safe::String> movedCopy;
	movedCopy = std::move(copyAssigned);
	ASSERT_TRUE("TestSafeOptionalAlgorithms", movedCopy == maybe && !copyAssigned.has_value());
	std::optional<Safe::String> source{Safe::String("moved")};
	Safe::Optional<Safe::String> moved(std::move(source));
	ASSERT_TRUE("TestSafeOptionalAlgorithms", !source.has_value() && moved.value() == "moved");
	RETURN_TEST("TestSafeOptionalAlgorithms", 0);
}

int TestSafePair() {
	Safe::Pair<Safe::String, Safe::String> pair(Safe::String("key"), Safe::String("value"));
	pair = std::pair<Safe::String, Safe::String>{Safe::String("imported"), Safe::String("pair")};
	ASSERT_TRUE("TestSafePair", pair.first == "imported" && pair.second == "pair");
	auto [key, value] = pair;
	ASSERT_TRUE("TestSafePair", key == "imported" && value == "pair");
	pair.second = Safe::String("updated");
	ASSERT_TRUE("TestSafePair", pair.first == "imported" && pair.second == "updated");
	auto copy = pair;
	copy.first = Safe::String("copy");
	ASSERT_TRUE("TestSafePair", pair.first == "imported" && copy.first == "copy");
	Safe::Pair<Safe::String, Safe::String> other(Safe::String("other"), Safe::String("value"));
	pair.swap(other);
	ASSERT_TRUE("TestSafePair", pair.first == "other" && other.first == "imported");
	RETURN_TEST("TestSafePair", 0);
}

int TestSafeQueueSTLAPI() {
	Safe::Queue<Safe::String> queue;
	const Safe::String inserted = queue.emplace("queued");
	queue.push(Safe::String("tail"));
	ASSERT_TRUE("TestSafeQueueSTLAPI", inserted == "queued" && queue.size() == Size(2) && queue.front() == "queued" && queue.back() == "tail");
	queue.pop();
	ASSERT_TRUE("TestSafeQueueSTLAPI", queue.size() == Size(1) && queue.front() == "tail");
	Safe::Queue<Safe::String> other;
	other.emplace("other");
	queue.swap(other);
	ASSERT_TRUE("TestSafeQueueSTLAPI", queue.front() == "other" && other.front() == "tail");
	Safe::Queue<Safe::String> same(queue);
	ASSERT_TRUE("TestSafeQueueSTLAPI", same == queue && same <= queue);
	std::queue<Safe::String> source;
	source.push(Safe::String("moved"));
	Safe::Queue<Safe::String> moved(std::move(source));
	ASSERT_TRUE("TestSafeQueueSTLAPI", source.empty() && moved.front() == "moved");
	std::queue<Safe::String> assigned;
	assigned.push(Safe::String("assigned"));
	moved = std::move(assigned);
	ASSERT_TRUE("TestSafeQueueSTLAPI", assigned.empty() && moved.front() == "assigned");
	RETURN_TEST("TestSafeQueueSTLAPI", 0);
}

// -------------------
// DLL ownership
// -------------------

int TestDLLOwnership() {
	int result = 0;
	ASSERT_TRUE("TestDLLOwnership", SafeCollectionsFixture::ExerciseCollections());
	ASSERT_TRUE("TestDLLOwnership", SafeCollectionsFixture::LiveContexts() == 0);
	RETURN_TEST("TestDLLOwnership", result);
}

int main() {
	int result = 0;

	// -------------------
	// Callback validation
	// -------------------

	result += TestCallbackValidation();

	// -------------------
	// Owner copy failure
	// -------------------

	result += TestOwnerCopyFailure();
	result += TestOwnerRejectsInvalidCallbacks();
	result += TestSafeVectorAlgorithms();
	result += TestSafeMapAlgorithms();
	result += TestSafeOptionalAlgorithms();
	result += TestSafePair();
	result += TestSafeQueueSTLAPI();

	// -------------------
	// DLL ownership
	// -------------------

	result += TestDLLOwnership();
		return result;
}
