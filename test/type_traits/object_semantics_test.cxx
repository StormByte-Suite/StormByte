/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This is the base for StormByte Suite libraries.
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
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/object_semantics.hxx>

#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

using namespace StormByte;

namespace {
	struct Pod {
		int value;
	};

	struct Counted {
		int value = 0;
		Counted() = default;
		Counted(const Counted&) = default;
		Counted& operator=(const Counted&) = default;
		bool operator==(const Counted&) const = default;
	};

	struct MoveOnly {
		MoveOnly() = default;
		MoveOnly(const MoveOnly&) = delete;
		MoveOnly& operator=(const MoveOnly&) = delete;
		MoveOnly(MoveOnly&&) = default;
		MoveOnly& operator=(MoveOnly&&) = default;
	};

	struct NoDefault {
		explicit NoDefault(int) {}
	};

	struct UserDestroyed {
		~UserDestroyed() {}
	};
}

// -------------------
// Assign
// -------------------

int test_assign_copy_and_move() {
	static_assert(Type::CopyAssignable<int>);
	static_assert(Type::CopyAssignable<std::vector<int>>);
	static_assert(Type::CopyAssignable<Safe::Vector<int>>);
	static_assert(Type::CopyAssignable<Safe::String>);
	static_assert(Type::CopyAssignable<Size>);
	static_assert(Type::CopyAssignable<ByteSize>);
	static_assert(!Type::CopyAssignable<std::vector<std::unique_ptr<int>>>);
	static_assert(Type::CopyAssignable<std::span<std::unique_ptr<int>>>);
	static_assert(Type::TriviallyCopyAssignable<int>);
	static_assert(Type::TriviallyCopyAssignable<Pod>);
	static_assert(!Type::TriviallyCopyAssignable<std::string>);
	static_assert(!Type::TriviallyCopyAssignable<Safe::String>);
	static_assert(Type::MoveAssignable<MoveOnly>);
	static_assert(Type::MoveAssignable<std::unique_ptr<int>>);
	static_assert(Type::MoveAssignable<std::vector<std::unique_ptr<int>>>);
	static_assert(Type::TriviallyMoveAssignable<int>);
	static_assert(!Type::TriviallyMoveAssignable<std::string>);
	static_assert(Type::AssignableFrom<int&, int>);
	static_assert(Type::AssignableFrom<std::string&, const char*>);
	static_assert(!Type::AssignableFrom<const int&, int>);
	ASSERT_TRUE(Type::CopyAssignable<Safe::Vector<int>>);
	ASSERT_FALSE(Type::CopyAssignable<std::vector<std::unique_ptr<int>>>);
	ASSERT_TRUE(Type::MoveAssignable<MoveOnly>);
	RETURN_TEST(0);
}

// -------------------
// Combined
// -------------------

int test_combined_regular_and_order() {
	static_assert(Type::Copyable<int>);
	static_assert(Type::Copyable<Safe::String>);
	static_assert(Type::Copyable<Safe::Vector<int>>);
	static_assert(!Type::Copyable<MoveOnly>);
	static_assert(!Type::Copyable<std::unique_ptr<int>>);
	static_assert(Type::Movable<MoveOnly>);
	static_assert(Type::Movable<std::unique_ptr<int>>);
	static_assert(Type::Movable<Safe::Vector<int>>);
	static_assert(Type::Swappable<int>);
	static_assert(Type::Swappable<std::string>);
	static_assert(Type::Swappable<Safe::String>);
	static_assert(Type::Swappable<Size>);
	static_assert(Type::Destructible<Counted>);
	static_assert(Type::Destructible<int>);
	static_assert(!Type::Destructible<void>);
	static_assert(Type::ConstructibleFrom<std::string, const char*>);
	static_assert(Type::ConstructibleFrom<Size, int>);
	static_assert(!Type::ConstructibleFrom<NoDefault>);
	static_assert(Type::TotallyOrdered<int>);
	static_assert(Type::TotallyOrdered<std::string>);
	static_assert(Type::TotallyOrdered<Size>);
	static_assert(Type::TotallyOrdered<ByteSize>);
	static_assert(Type::TotallyOrdered<Safe::String>);
	static_assert(Type::TotallyOrdered<std::unique_ptr<int>>);
	static_assert(Type::Semiregular<int>);
	static_assert(Type::Semiregular<Safe::String>);
	static_assert(Type::Semiregular<Size>);
	static_assert(!Type::Semiregular<MoveOnly>);
	static_assert(!Type::Semiregular<NoDefault>);
	static_assert(Type::Regular<int>);
	static_assert(Type::Regular<std::string>);
	static_assert(Type::Regular<Safe::String>);
	static_assert(Type::Regular<Size>);
	static_assert(Type::Regular<ByteSize>);
	static_assert(!Type::Regular<MoveOnly>);
	ASSERT_TRUE(Type::Regular<Safe::String>);
	ASSERT_TRUE(Type::TotallyOrdered<ByteSize>);
	ASSERT_FALSE(Type::Copyable<MoveOnly>);
	RETURN_TEST(0);
}

// -------------------
// Construct
// -------------------

int test_construct_copy_move_default() {
	static_assert(Type::DefaultConstructible<int>);
	static_assert(Type::DefaultConstructible<std::string>);
	static_assert(Type::DefaultConstructible<Safe::String>);
	static_assert(Type::DefaultConstructible<Safe::Vector<int>>);
	static_assert(Type::DefaultConstructible<Size>);
	static_assert(Type::DefaultConstructible<ByteSize>);
	static_assert(!Type::DefaultConstructible<NoDefault>);
	static_assert(!Type::DefaultConstructible<void>);
	static_assert(Type::TriviallyDefaultConstructible<int>);
	static_assert(Type::TriviallyDefaultConstructible<Pod>);
	static_assert(!Type::TriviallyDefaultConstructible<std::string>);
	static_assert(!Type::TriviallyDefaultConstructible<Safe::String>);
	static_assert(Type::CopyConstructible<int>);
	static_assert(Type::CopyConstructible<std::vector<int>>);
	static_assert(Type::CopyConstructible<Safe::Vector<int>>);
	static_assert(Type::CopyConstructible<Safe::String>);
	static_assert(Type::CopyConstructible<std::span<std::unique_ptr<int>>>);
	static_assert(!Type::CopyConstructible<std::vector<std::unique_ptr<int>>>);
	static_assert(!Type::CopyConstructible<std::unique_ptr<int>>);
	static_assert(Type::TriviallyCopyConstructible<int>);
	static_assert(Type::TriviallyCopyConstructible<Pod>);
	static_assert(!Type::TriviallyCopyConstructible<std::string>);
	static_assert(Type::MoveConstructible<MoveOnly>);
	static_assert(Type::MoveConstructible<std::unique_ptr<int>>);
	static_assert(Type::MoveConstructible<std::vector<std::unique_ptr<int>>>);
	static_assert(Type::MoveConstructible<Safe::String>);
	static_assert(Type::TriviallyMoveConstructible<int>);
	static_assert(!Type::TriviallyMoveConstructible<std::string>);
	ASSERT_TRUE(Type::CopyConstructible<Safe::Vector<int>>);
	ASSERT_FALSE(Type::CopyConstructible<std::vector<std::unique_ptr<int>>>);
	ASSERT_TRUE(Type::CopyConstructible<std::span<std::unique_ptr<int>>>);
	ASSERT_TRUE(Type::MoveConstructible<MoveOnly>);
	RETURN_TEST(0);
}

// -------------------
// Trivial
// -------------------

int test_trivial_copy_and_destroy() {
	static_assert(Type::TriviallyCopyable<int>);
	static_assert(Type::TriviallyCopyable<Pod>);
	static_assert(Type::TriviallyCopyable<int*>);
	static_assert(Type::TriviallyCopyable<Size>);
	static_assert(Type::TriviallyCopyable<ByteSize>);
	static_assert(!Type::TriviallyCopyable<std::string>);
	static_assert(!Type::TriviallyCopyable<Safe::String>);
	static_assert(!Type::TriviallyCopyable<Safe::Vector<int>>);
	static_assert(!Type::TriviallyCopyable<std::unique_ptr<int>>);
	static_assert(Type::TriviallyDestructible<int>);
	static_assert(Type::TriviallyDestructible<Pod>);
	static_assert(Type::TriviallyDestructible<Size>);
	static_assert(Type::TriviallyDestructible<Counted>);
	static_assert(!Type::TriviallyDestructible<std::string>);
	static_assert(!Type::TriviallyDestructible<Safe::String>);
	static_assert(!Type::TriviallyDestructible<UserDestroyed>);
	ASSERT_TRUE(Type::TriviallyCopyable<ByteSize>);
	ASSERT_FALSE(Type::TriviallyCopyable<Safe::String>);
	ASSERT_FALSE(Type::TriviallyDestructible<Safe::Vector<int>>);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Assign
	// -------------------
	result += test_assign_copy_and_move();

	// -------------------
	// Combined
	// -------------------
	result += test_combined_regular_and_order();

	// -------------------
	// Construct
	// -------------------
	result += test_construct_copy_move_default();

	// -------------------
	// Trivial
	// -------------------
	result += test_trivial_copy_and_destroy();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
