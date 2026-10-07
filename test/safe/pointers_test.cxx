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

#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/pointers.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

using namespace StormByte;

namespace {
	struct Item {
		int value;
		static int live;
		explicit Item(int n): value(n) { ++live; }
		~Item() { --live; }
	};
	int Item::live = 0;

	struct Node: Safe::EnableSharedFromThis<Node> {
		int value;
		explicit Node(int n): value(n) {}
		Safe::Shared<Node> Self() const { return SharedFromThis(); }
	};
}

// -------------------
// Shared
// -------------------

int test_shared_lifetime_copy_and_weak() {
	Safe::Shared<Item> empty;
	Safe::Shared<Item> null(nullptr);
	ASSERT_FALSE(static_cast<bool>(empty));
	ASSERT_NULL(empty.get());
	ASSERT_EQUAL(0L, empty.use_count());
	auto owned = Safe::Shared<Item>::MakePointer<Item>(4);
	ASSERT_EQUAL(4, owned->value);
	ASSERT_EQUAL(1L, owned.use_count());
	Safe::Shared<Item> copy(owned);
	ASSERT_EQUAL(owned.get(), copy.get());
	ASSERT_EQUAL(2L, owned.use_count());
	Safe::Weak<Item> weak(owned);
	ASSERT_FALSE(weak.expired());
	Safe::Shared<Item> locked = weak.lock();
	ASSERT_EQUAL(owned.get(), locked.get());
	Safe::Shared<Item> moved(std::move(copy));
	ASSERT_NULL(copy.get());
	owned.reset();
	moved.reset();
	locked.reset();
	ASSERT_TRUE(weak.expired());
	ASSERT_THROWS(Safe::Shared<Item>(weak), Safe::ExpiredWeakPointerError);
	ASSERT_EQUAL(0, Item::live);
	RETURN_TEST(0);
}

int test_shared_swap_export_and_from_this() {
	auto left = Safe::Shared<Item>::MakePointer<Item>(1);
	auto right = Safe::Shared<Item>::MakePointer<Item>(2);
	Item* left_pointer = left.get();
	left.swap(right);
	ASSERT_EQUAL(left_pointer, right.get());
	ASSERT_TRUE(left.owner_before(right) || right.owner_before(left));
	std::shared_ptr<Item> exported(left);
	ASSERT_EQUAL(left.get(), exported.get());
	auto node = Safe::MakeShared<Node>(6);
	Safe::Shared<Node> recovered = node->Self();
	ASSERT_EQUAL(node.get(), recovered.get());
	Node stack(1);
	ASSERT_THROWS(stack.Self(), Safe::ExpiredWeakPointerError);
	RETURN_TEST(0);
}

int test_shared_algorithm_sort_by_value() {
	std::vector<Safe::Shared<Item>> values;
	values.push_back(Safe::Shared<Item>::MakePointer<Item>(3));
	values.push_back(Safe::Shared<Item>::MakePointer<Item>(1));
	values.push_back(Safe::Shared<Item>::MakePointer<Item>(2));
	std::sort(values.begin(), values.end(), [](const Safe::Shared<Item>& left, const Safe::Shared<Item>& right) {
		return left->value < right->value;
	});
	ASSERT_EQUAL(1, values[0]->value);
	ASSERT_EQUAL(3, values[2]->value);
	auto found = std::find_if(values.begin(), values.end(), [](const Safe::Shared<Item>& item) {
		return item->value == 2;
	});
	ASSERT_TRUE(found != values.end());
	RETURN_TEST(0);
}

// -------------------
// Unique
// -------------------

int test_unique_move_only_and_export() {
	Safe::Unique<Item> empty(nullptr);
	ASSERT_FALSE(static_cast<bool>(empty));
	auto owned = Safe::Unique<Item>::MakePointer<Item>(9);
	Safe::Unique<Item> moved(std::move(owned));
	ASSERT_NULL(owned.get());
	ASSERT_EQUAL(9, moved->value);
	std::unique_ptr<Item, Safe::Heap::ObjectDeleter> exported(std::move(moved));
	ASSERT_NULL(moved.get());
	ASSERT_EQUAL(9, exported->value);
	exported.reset();
	ASSERT_EQUAL(0, Item::live);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Shared
	// -------------------
	result += test_shared_lifetime_copy_and_weak();
	result += test_shared_swap_export_and_from_this();
	result += test_shared_algorithm_sort_by_value();

	// -------------------
	// Unique
	// -------------------
	result += test_unique_move_only_and_export();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
