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
#include <compare>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
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

	struct BaseItem {
		int value;
		explicit BaseItem(int n): value(n) {}
		virtual ~BaseItem() = default;
	};

	struct ChildItem: BaseItem {
		int extra;
		explicit ChildItem(int n): BaseItem(n), extra(n + 1) {}
	};

	struct Node: Safe::EnableSharedFromThis<Node> {
		int value;
		explicit Node(int n): value(n) {}
		Safe::Shared<Node> Self() const { return SharedFromThis(); }
	};
}

// -------------------
// Atomic
// -------------------

int test_atomic_publish_exchange_and_compare() {
	Safe::AtomicShared<Item> empty;
	ASSERT_FALSE(static_cast<bool>(empty));
	ASSERT_NULL(empty.load().get());
	auto first = Safe::Shared<Item>::MakePointer<Item>(1);
	Safe::AtomicShared<Item> published(first);
	ASSERT_TRUE(static_cast<bool>(published));
	ASSERT_EQUAL(first.get(), published.load().get());
	ASSERT_EQUAL(2L, first.use_count());
	auto second = Safe::Shared<Item>::MakePointer<Item>(2);
	published.store(second);
	ASSERT_EQUAL(second.get(), published.load().get());
	auto third = Safe::Shared<Item>::MakePointer<Item>(3);
	Safe::Shared<Item> previous = published.exchange(third);
	ASSERT_EQUAL(second.get(), previous.get());
	ASSERT_EQUAL(third.get(), published.load().get());
	Safe::Shared<Item> expected = third;
	auto fourth = Safe::Shared<Item>::MakePointer<Item>(4);
	ASSERT_TRUE(published.compare_exchange_strong(expected, fourth));
	ASSERT_EQUAL(fourth.get(), published.load().get());
	Safe::Shared<Item> wrong = first;
	ASSERT_FALSE(published.compare_exchange_weak(wrong, first));
	ASSERT_EQUAL(fourth.get(), wrong.get());
	published = Safe::Shared<Item>{};
	ASSERT_FALSE(static_cast<bool>(published));
	RETURN_TEST(0);
}

// -------------------
// Cast
// -------------------

int test_pointer_casts_keep_control_block() {
	auto child = Safe::MakeShared<ChildItem>(5);
	Safe::Shared<BaseItem> as_base = Safe::StaticPointerCast<BaseItem>(child);
	ASSERT_EQUAL(child.get(), as_base.get());
	ASSERT_EQUAL(2L, child.use_count());
	Safe::Shared<ChildItem> back = Safe::DynamicPointerCast<ChildItem>(as_base);
	ASSERT_EQUAL(child.get(), back.get());
	Safe::Shared<Item> missing = Safe::DynamicPointerCast<Item>(as_base);
	ASSERT_NULL(missing.get());
	Safe::Shared<const ChildItem> constant = child;
	Safe::Shared<ChildItem> mutable_child = Safe::ConstPointerCast<ChildItem>(constant);
	ASSERT_EQUAL(child.get(), mutable_child.get());
	Safe::Shared<void> raw = Safe::ReinterpretPointerCast<void>(child);
	ASSERT_EQUAL(static_cast<void*>(child.get()), raw.get());
	Safe::Shared<void> erased(child);
	ASSERT_EQUAL(static_cast<void*>(child.get()), erased.get());
	ASSERT_TRUE(child.use_count() > 1L);
	RETURN_TEST(0);
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
	ASSERT_EQUAL(4, (*owned).value);
	ASSERT_EQUAL(1L, owned.use_count());
	Safe::Shared<Item> copy(owned);
	ASSERT_EQUAL(owned.get(), copy.get());
	ASSERT_EQUAL(2L, owned.use_count());
	Safe::Shared<Item> assigned;
	assigned = copy;
	ASSERT_EQUAL(3L, owned.use_count());
	Safe::Weak<Item> weak(owned);
	ASSERT_FALSE(weak.expired());
	Safe::Shared<Item> locked = weak.lock();
	ASSERT_EQUAL(owned.get(), locked.get());
	Safe::Shared<Item> moved(std::move(copy));
	ASSERT_NULL(copy.get());
	assigned = std::move(moved);
	ASSERT_NULL(moved.get());
	ASSERT_EQUAL(owned.get(), assigned.get());
	owned.reset();
	assigned.reset();
	locked.reset();
	ASSERT_TRUE(weak.expired());
	ASSERT_NULL(weak.lock().get());
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
	swap(left, right);
	ASSERT_EQUAL(left_pointer, left.get());
	ASSERT_TRUE(left.owner_before(right) || right.owner_before(left));
	ASSERT_FALSE(left.owner_before(left));
	Safe::Weak<Item> weak(left);
	ASSERT_FALSE(left.owner_before(weak));
	ASSERT_FALSE(weak.owner_before(left));
	ASSERT_TRUE(weak.owner_before(right) || right.owner_before(weak));
	std::shared_ptr<Item> exported(left);
	ASSERT_EQUAL(left.get(), exported.get());
	ASSERT_EQUAL(Safe::Hash<Safe::Shared<Item>>{}(left), std::hash<Safe::Shared<Item>>{}(left));
	auto node = Safe::MakeShared<Node>(6);
	Safe::Shared<Node> recovered = node->Self();
	ASSERT_EQUAL(node.get(), recovered.get());
	Node stack(1);
	ASSERT_THROWS(stack.Self(), Safe::ExpiredWeakPointerError);
	Node copied(stack);
	ASSERT_THROWS(copied.Self(), Safe::ExpiredWeakPointerError);
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

int test_shared_order_null_and_heterogeneous() {
	Safe::Shared<Item> empty;
	Safe::Shared<Item> also_empty(nullptr);
	auto owned = Safe::Shared<Item>::MakePointer<Item>(4);
	Safe::Shared<Item> copy(owned);
	auto other = Safe::Shared<Item>::MakePointer<Item>(4);
	ASSERT_TRUE(empty == also_empty);
	ASSERT_TRUE(empty == nullptr);
	ASSERT_TRUE(nullptr == empty);
	ASSERT_FALSE(empty != nullptr);
	ASSERT_TRUE((empty <=> nullptr) == 0);
	ASSERT_TRUE((nullptr <=> empty) == 0);
	ASSERT_FALSE(empty < nullptr);
	ASSERT_FALSE(nullptr < empty);
	ASSERT_TRUE(empty <= nullptr);
	ASSERT_TRUE(nullptr >= empty);
	ASSERT_TRUE(owned == copy);
	ASSERT_FALSE(owned != copy);
	ASSERT_TRUE((owned <=> copy) == 0);
	ASSERT_TRUE(owned <= copy);
	ASSERT_TRUE(owned >= copy);
	ASSERT_FALSE(owned == other);
	ASSERT_TRUE(owned != other);
	ASSERT_TRUE((owned <=> other) != 0);
	ASSERT_TRUE(owned < other || other < owned);
	ASSERT_TRUE(owned <= other || other <= owned);
	ASSERT_TRUE(owned > other || other > owned);
	ASSERT_TRUE(owned >= other || other >= owned);
	ASSERT_TRUE(std::less<Safe::Shared<Item>>{}(owned, other) || std::less<Safe::Shared<Item>>{}(other, owned));
	ASSERT_FALSE(std::less<Safe::Shared<Item>>{}(owned, copy));
	ASSERT_TRUE(owned != nullptr);
	ASSERT_TRUE(nullptr != owned);
	ASSERT_TRUE((owned <=> nullptr) != 0);
	ASSERT_TRUE(owned > nullptr || nullptr > owned);
	std::set<Safe::Shared<Item>> keys;
	keys.insert(owned);
	keys.insert(copy);
	keys.insert(other);
	ASSERT_EQUAL(std::size_t{2}, keys.size());
	ASSERT_TRUE(keys.contains(owned));
	Safe::Shared<ChildItem> child = Safe::Shared<ChildItem>::MakePointer<ChildItem>(8);
	Safe::Shared<BaseItem> base(child);
	ASSERT_TRUE(base == child);
	ASSERT_TRUE(child == base);
	ASSERT_FALSE(base != child);
	ASSERT_TRUE((base <=> child) == 0);
	ASSERT_TRUE((child <=> base) == 0);
	ASSERT_FALSE(base < child);
	ASSERT_FALSE(child < base);
	ASSERT_TRUE(base <= child);
	ASSERT_TRUE(child >= base);
	auto foreign = Safe::Shared<BaseItem>::MakePointer<BaseItem>(8);
	ASSERT_FALSE(base == foreign);
	ASSERT_TRUE(base < foreign || foreign < base);
	RETURN_TEST(0);
}

// -------------------
// Unique
// -------------------

int test_unique_move_only_and_export() {
	Safe::Unique<Item> empty(nullptr);
	ASSERT_FALSE(static_cast<bool>(empty));
	auto owned = Safe::Unique<Item>::MakePointer<Item>(9);
	ASSERT_EQUAL(9, (*owned).value);
	Safe::Unique<Item> moved(std::move(owned));
	ASSERT_NULL(owned.get());
	ASSERT_EQUAL(9, moved->value);
	Safe::Unique<Item> assigned;
	assigned = std::move(moved);
	ASSERT_NULL(moved.get());
	ASSERT_EQUAL(9, assigned->value);
	assigned.swap(moved);
	ASSERT_EQUAL(9, moved->value);
	swap(moved, assigned);
	ASSERT_EQUAL(9, assigned->value);
	std::unique_ptr<Item, Safe::Heap::ObjectDeleter> exported(std::move(assigned));
	ASSERT_NULL(assigned.get());
	ASSERT_EQUAL(9, exported->value);
	exported.reset();
	ASSERT_EQUAL(0, Item::live);
	RETURN_TEST(0);
}

int test_unique_order_null_and_heterogeneous() {
	Safe::Unique<Item> empty;
	Safe::Unique<Item> also_empty(nullptr);
	auto owned = Safe::Unique<Item>::MakePointer<Item>(9);
	auto other = Safe::Unique<Item>::MakePointer<Item>(9);
	ASSERT_TRUE(empty == also_empty);
	ASSERT_TRUE(empty == nullptr);
	ASSERT_TRUE(nullptr == empty);
	ASSERT_FALSE(empty != nullptr);
	ASSERT_TRUE((empty <=> nullptr) == 0);
	ASSERT_TRUE((nullptr <=> empty) == 0);
	ASSERT_FALSE(empty < nullptr);
	ASSERT_TRUE(empty <= nullptr);
	ASSERT_TRUE(nullptr >= empty);
	ASSERT_FALSE(owned == other);
	ASSERT_TRUE(owned != other);
	ASSERT_TRUE((owned <=> other) != 0);
	ASSERT_TRUE(owned < other || other < owned);
	ASSERT_TRUE(owned <= other || other <= owned);
	ASSERT_TRUE(owned > other || other > owned);
	ASSERT_TRUE(owned >= other || other >= owned);
	ASSERT_TRUE(std::less<Safe::Unique<Item>>{}(owned, other) || std::less<Safe::Unique<Item>>{}(other, owned));
	ASSERT_EQUAL(Safe::Hash<Safe::Unique<Item>>{}(owned), std::hash<Safe::Unique<Item>>{}(owned));
	ASSERT_TRUE(owned != nullptr);
	ASSERT_TRUE(nullptr != owned);
	ASSERT_TRUE((owned <=> nullptr) != 0);
	ASSERT_TRUE(owned > nullptr || nullptr > owned);
	Safe::Unique<Item> moved_from(std::move(owned));
	ASSERT_TRUE(owned == nullptr);
	ASSERT_TRUE(nullptr == owned);
	ASSERT_TRUE((owned <=> nullptr) == 0);
	ASSERT_TRUE(moved_from != owned);
	Safe::Unique<ChildItem> child = Safe::Unique<ChildItem>::MakePointer<ChildItem>(3);
	ChildItem* child_pointer = child.get();
	Safe::Unique<BaseItem> base(std::move(child));
	ASSERT_NULL(child.get());
	ASSERT_TRUE(child == nullptr);
	ASSERT_EQUAL(child_pointer, base.get());
	ASSERT_TRUE(base != nullptr);
	ASSERT_TRUE(nullptr != base);
	ASSERT_TRUE((base <=> nullptr) != 0);
	Safe::Unique<BaseItem> foreign = Safe::Unique<BaseItem>::MakePointer<BaseItem>(3);
	ASSERT_FALSE(base == foreign);
	ASSERT_TRUE(base < foreign || foreign < base);
	RETURN_TEST(0);
}

// -------------------
// Weak
// -------------------

int test_weak_copy_move_reset_and_hash() {
	Safe::Weak<Item> empty;
	Safe::Weak<Item> null(nullptr);
	ASSERT_TRUE(empty.expired());
	ASSERT_EQUAL(0L, empty.use_count());
	ASSERT_NULL(empty.lock().get());
	auto owned = Safe::Shared<Item>::MakePointer<Item>(7);
	Safe::Weak<Item> weak(owned);
	Safe::Weak<Item> copy(weak);
	ASSERT_FALSE(copy.expired());
	ASSERT_EQUAL(owned.get(), copy.lock().get());
	Safe::Weak<Item> moved(std::move(copy));
	ASSERT_NULL(copy.lock().get());
	Safe::Weak<Item> assigned;
	assigned = moved;
	ASSERT_EQUAL(owned.get(), assigned.lock().get());
	assigned = std::move(moved);
	ASSERT_EQUAL(owned.get(), assigned.lock().get());
	Safe::Weak<Item> other(owned);
	assigned.swap(other);
	swap(assigned, other);
	ASSERT_TRUE(assigned.owner_before(empty) || empty.owner_before(assigned));
	ASSERT_EQUAL(Safe::Hash<Safe::Weak<Item>>{}(assigned), std::hash<Safe::Weak<Item>>{}(assigned));
	assigned.reset();
	ASSERT_TRUE(assigned.expired());
	owned.reset();
	ASSERT_TRUE(other.expired());
	ASSERT_EQUAL(0, Item::live);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Atomic
	// -------------------
	result += test_atomic_publish_exchange_and_compare();

	// -------------------
	// Cast
	// -------------------
	result += test_pointer_casts_keep_control_block();

	// -------------------
	// Shared
	// -------------------
	result += test_shared_lifetime_copy_and_weak();
	result += test_shared_swap_export_and_from_this();
	result += test_shared_algorithm_sort_by_value();
	result += test_shared_order_null_and_heterogeneous();

	// -------------------
	// Unique
	// -------------------
	result += test_unique_move_only_and_export();
	result += test_unique_order_null_and_heterogeneous();

	// -------------------
	// Weak
	// -------------------
	result += test_weak_copy_move_reset_and_hash();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
