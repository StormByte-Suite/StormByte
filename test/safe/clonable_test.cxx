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

#include <StormByte/safe/clonable.hxx>
#include <StormByte/safe/pointers.hxx>
#include <StormByte/test_handlers.h>

#include <iostream>
#include <utility>

using namespace StormByte;

namespace {
	struct Item: Safe::Clonable<Item> {
		int value;
		inline static int live = 0;
		inline static int destroyed = 0;
		inline static int clones = 0;
		inline static int moves = 0;
		explicit Item(int n): value(n) { ++live; }
		~Item() override { --live; ++destroyed; }
		PointerType Clone() const override { ++clones; return MakePointer<Item>(value); }
		PointerType Move() override { ++moves; return MakePointer<Item>(std::exchange(value, 0)); }
	};

	struct UniqueItem: Safe::Clonable<UniqueItem, Safe::Unique<UniqueItem>> {
		int value;
		inline static int live = 0;
		inline static int destroyed = 0;
		inline static int clones = 0;
		inline static int moves = 0;
		explicit UniqueItem(int n): value(n) { ++live; }
		~UniqueItem() override { --live; ++destroyed; }
		PointerType Clone() const override { ++clones; return MakePointer<UniqueItem>(value); }
		PointerType Move() override { ++moves; return MakePointer<UniqueItem>(std::exchange(value, 0)); }
	};
}

// -------------------
// Life
// -------------------

int test_clone_shared_and_unique() {
	auto shared = Item::MakePointer<Item>(3);
	auto cloned = shared->Clone();
	ASSERT_TRUE(shared.get() != cloned.get());
	ASSERT_EQUAL(3, cloned->value);
	ASSERT_EQUAL(3, shared->value);
	auto moved = shared->Move();
	ASSERT_EQUAL(3, moved->value);
	ASSERT_EQUAL(0, shared->value);
	auto unique = Item::MakePointer<Item>(5);
	ASSERT_EQUAL(5, unique->value);
	RETURN_TEST(0);
}

int test_shared_clone_replacement_and_lifetime() {
	const int destroyed = Item::destroyed;
	const int clones = Item::clones;
	const int moves = Item::moves;
	ASSERT_EQUAL(0, Item::live);
	auto source = Item::MakePointer<Item>(21);
	Safe::Clonable<Item>& interface = *source;
	auto clone = interface.Clone();
	ASSERT_TRUE(source.get() != clone.get());
	ASSERT_EQUAL(21, source->value);
	ASSERT_EQUAL(21, clone->value);
	ASSERT_EQUAL(clones + 1, Item::clones);
	ASSERT_EQUAL(2, Item::live);
	Safe::Weak<Item> old_clone(clone);
	clone = interface.Clone();
	ASSERT_TRUE(old_clone.expired());
	ASSERT_EQUAL(clones + 2, Item::clones);
	ASSERT_EQUAL(destroyed + 1, Item::destroyed);
	ASSERT_EQUAL(2, Item::live);
	auto moved = interface.Move();
	ASSERT_TRUE(source.get() != moved.get());
	ASSERT_EQUAL(0, source->value);
	ASSERT_EQUAL(21, moved->value);
	ASSERT_EQUAL(21, clone->value);
	ASSERT_EQUAL(moves + 1, Item::moves);
	ASSERT_EQUAL(3, Item::live);
	auto alias = clone;
	clone = Safe::Shared<Item>(nullptr);
	ASSERT_NULL(clone.get());
	ASSERT_EQUAL(1L, alias.use_count());
	ASSERT_EQUAL(destroyed + 1, Item::destroyed);
	source.reset();
	ASSERT_EQUAL(destroyed + 2, Item::destroyed);
	ASSERT_EQUAL(21, alias->value);
	alias.reset();
	moved.reset();
	ASSERT_EQUAL(destroyed + 4, Item::destroyed);
	ASSERT_EQUAL(0, Item::live);
	RETURN_TEST(0);
}

int test_unique_clone_move_replacement_and_lifetime() {
	const int destroyed = UniqueItem::destroyed;
	const int clones = UniqueItem::clones;
	const int moves = UniqueItem::moves;
	ASSERT_EQUAL(0, UniqueItem::live);
	auto source = UniqueItem::MakePointer<UniqueItem>(22);
	Safe::Clonable<UniqueItem, Safe::Unique<UniqueItem>>& interface = *source;
	auto clone = interface.Clone();
	ASSERT_TRUE(source.get() != clone.get());
	ASSERT_EQUAL(22, source->value);
	ASSERT_EQUAL(22, clone->value);
	ASSERT_EQUAL(clones + 1, UniqueItem::clones);
	ASSERT_EQUAL(2, UniqueItem::live);
	clone = interface.Clone();
	ASSERT_EQUAL(clones + 2, UniqueItem::clones);
	ASSERT_EQUAL(destroyed + 1, UniqueItem::destroyed);
	ASSERT_EQUAL(2, UniqueItem::live);
	auto moved = interface.Move();
	ASSERT_TRUE(source.get() != moved.get());
	ASSERT_EQUAL(0, source->value);
	ASSERT_EQUAL(22, moved->value);
	ASSERT_EQUAL(22, clone->value);
	ASSERT_EQUAL(moves + 1, UniqueItem::moves);
	ASSERT_EQUAL(3, UniqueItem::live);
	UniqueItem* pointer = moved.get();
	moved = std::move(moved);
	ASSERT_EQUAL(pointer, moved.get());
	ASSERT_EQUAL(destroyed + 1, UniqueItem::destroyed);
	auto taken = std::move(moved);
	ASSERT_NULL(moved.get());
	ASSERT_EQUAL(pointer, taken.get());
	clone = Safe::Unique<UniqueItem>(nullptr);
	ASSERT_NULL(clone.get());
	ASSERT_EQUAL(destroyed + 2, UniqueItem::destroyed);
	source.reset();
	taken.reset();
	moved.reset();
	ASSERT_EQUAL(destroyed + 4, UniqueItem::destroyed);
	ASSERT_EQUAL(0, UniqueItem::live);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Life
	// -------------------
	result += test_clone_shared_and_unique();
	result += test_shared_clone_replacement_and_lifetime();
	result += test_unique_clone_move_replacement_and_lifetime();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
