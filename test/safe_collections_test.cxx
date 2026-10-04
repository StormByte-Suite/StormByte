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
#include <StormByte/safe/pair.hxx>
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
static_assert(Type::SafeComponent<Safe::Callback>);
static_assert(Type::SafeValue<Safe::String>);
static_assert(Type::SafeValue<Safe::Vector<Safe::String>>);
static_assert(Type::SafeValue<Safe::Map<Safe::String, Safe::String>>);
static_assert(Type::SafeValue<Safe::Optional<Safe::String>>);
static_assert(Type::SafeValue<Safe::Queue<Safe::String>>);
static_assert(Type::SafeValue<Safe::Pair<Safe::String, Safe::String>>);
static_assert(Type::SafeValue<BinaryData>);
static_assert(Type::SafeValue<Size>);
static_assert(Type::SafeValue<ByteSize>);
static_assert(!Type::SafeValue<const Safe::String>);
static_assert(!Type::SafeValue<Safe::String&>);
static_assert(!Type::SafeValue<Safe::Shared<Safe::String>>);
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
	Safe::Vector<Safe::String> values(std::vector<Safe::String>{
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
	const auto found = std::ranges::find_if(dictionary, [](const auto& entry) { return entry.first == "beta"; });
	ASSERT_TRUE("TestSafeVectorAlgorithms", found != dictionary.end());
	std::ranges::for_each(dictionary, [](auto entry) { entry.second = Safe::String("updated"); });
	Safe::String mapped;
	ASSERT_TRUE("TestSafeMapAlgorithms", dictionary.at(Safe::String("alpha")) == "updated");
	RETURN_TEST("TestSafeMapAlgorithms", 0);
}

int TestSafeOptionalAlgorithms() {
	Safe::Optional<Safe::String> maybe(std::optional<Safe::String>{Safe::String("before")});
	ASSERT_TRUE("TestSafeOptionalAlgorithms", std::ranges::find(maybe, Safe::String("before")) != maybe.end());
	*maybe.begin() = Safe::String("after");
	ASSERT_TRUE("TestSafeOptionalAlgorithms", maybe.value() == "after");
	std::optional<Safe::String> source{Safe::String("moved")};
	Safe::Optional<Safe::String> moved(std::move(source));
	ASSERT_TRUE("TestSafeOptionalAlgorithms", !source.has_value() && moved.value() == "moved");
	RETURN_TEST("TestSafeOptionalAlgorithms", 0);
}

int TestSafePair() {
	Safe::Pair<Safe::String, Safe::String> pair(Safe::String("key"), Safe::String("value"));
	auto [key, value] = pair;
	ASSERT_TRUE("TestSafePair", key == "key" && value == "value");
	pair.second = Safe::String("updated");
	ASSERT_TRUE("TestSafePair", pair.first == "key" && pair.second == "updated");
	auto copy = pair;
	copy.first = Safe::String("copy");
	ASSERT_TRUE("TestSafePair", pair.first == "key" && copy.first == "copy");
	RETURN_TEST("TestSafePair", 0);
}

int TestSafeQueueSTLAPI() {
	Safe::Queue<Safe::String> queue;
	queue.push(Safe::String("queued"));
	ASSERT_TRUE("TestSafeQueueSTLAPI", queue.size() == Size(1) && queue.front() == "queued");
	queue.pop();
	ASSERT_TRUE("TestSafeQueueSTLAPI", queue.empty());
	std::queue<Safe::String> source;
	source.push(Safe::String("moved"));
	Safe::Queue<Safe::String> moved(std::move(source));
	ASSERT_TRUE("TestSafeQueueSTLAPI", source.empty() && moved.front() == "moved");
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