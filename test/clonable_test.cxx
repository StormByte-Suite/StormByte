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

#include "clonable_plugin.hxx"

#include <StormByte/safe/clonable.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits.hxx>

#include <memory>
#include <typeinfo>
#include <utility>

using namespace StormByte;
using namespace StormByte::Safe;

namespace {
	class SharedItem: public Clonable<SharedItem, Shared<SharedItem>> {
		public:
			explicit SharedItem(int value): value(value) {}

			PointerType Clone() const override {
				return MakePointer<SharedItem>(value);
			}

			PointerType Move() override {
				return MakePointer<SharedItem>(std::move(*this));
			}

			int value;
	};

	class UniqueItem: public Clonable<UniqueItem, Unique<UniqueItem>> {
		public:
			explicit UniqueItem(int value): value(value) {}

			PointerType Clone() const override {
				return MakePointer<UniqueItem>(value);
			}

			PointerType Move() override {
				return MakePointer<UniqueItem>(std::move(*this));
			}

			int value;
	};

	int consume_shared_ptr(const std::shared_ptr<SharedItem>& item) {
		return item->value;
	}

	int consume_unique_ptr(const std::unique_ptr<UniqueItem, Heap::ObjectDeleter>& item) {
		return item->value;
	}
}

// -------------------
// Shared
// -------------------

int test_shared_clone_copies() {
	int result = 0;
	SharedItem original(42);
	SharedItem::PointerType clone = original.Clone();
	ASSERT_TRUE("test_shared_clone_copies", clone != nullptr);
	ASSERT_EQUAL("test_shared_clone_copies", 42, clone->value);
	ASSERT_TRUE("test_shared_clone_copies", clone.get() != &original);
	RETURN_TEST("test_shared_clone_copies", result);
}

int test_shared_clone_is_independent() {
	int result = 0;
	SharedItem original(1);
	SharedItem::PointerType clone = original.Clone();
	clone->value = 99;
	ASSERT_EQUAL("test_shared_clone_is_independent", 1, original.value);
	ASSERT_EQUAL("test_shared_clone_is_independent", 99, clone->value);
	RETURN_TEST("test_shared_clone_is_independent", result);
}

int test_shared_converts_to_std_shared_ptr() {
	int result = 0;
	SharedItem original(11);
	SharedItem::PointerType item = original.Clone();
	std::shared_ptr<SharedItem> as_std = item;
	ASSERT_TRUE("test_shared_converts_to_std_shared_ptr", as_std != nullptr);
	ASSERT_EQUAL("test_shared_converts_to_std_shared_ptr", 11, as_std->value);
	ASSERT_TRUE("test_shared_converts_to_std_shared_ptr", as_std.get() == item.get());
	ASSERT_EQUAL("test_shared_converts_to_std_shared_ptr", 11, consume_shared_ptr(item));
	RETURN_TEST("test_shared_converts_to_std_shared_ptr", result);
}

int test_shared_make_pointer() {
	int result = 0;
	SharedItem::PointerType pointer = SharedItem::MakePointer<SharedItem>(8);
	ASSERT_TRUE("test_shared_make_pointer", pointer != nullptr);
	ASSERT_EQUAL("test_shared_make_pointer", 8, pointer->value);
	ASSERT_TRUE("test_shared_make_pointer", pointer.use_count() == 1);
	RETURN_TEST("test_shared_make_pointer", result);
}

int test_shared_move() {
	int result = 0;
	SharedItem original(7);
	SharedItem::PointerType moved = original.Move();
	ASSERT_TRUE("test_shared_move", moved != nullptr);
	ASSERT_EQUAL("test_shared_move", 7, moved->value);
	ASSERT_TRUE("test_shared_move", moved.get() != &original);
	RETURN_TEST("test_shared_move", result);
}

int test_shared_pointer_storage() {
	int result = 0;
	SharedItem original(4);
	SharedItem::PointerType first = original.Clone();
	SharedItem::PointerType second = first;
	ASSERT_EQUAL("test_shared_pointer_storage", 4, first->value);
	ASSERT_EQUAL("test_shared_pointer_storage", 4, second->value);
	ASSERT_TRUE("test_shared_pointer_storage", first.get() == second.get());
	second->value = 20;
	ASSERT_EQUAL("test_shared_pointer_storage", 20, first->value);
	RETURN_TEST("test_shared_pointer_storage", result);
}

int test_shared_pointer_type() {
	int result = 0;
	static_assert(Type::SameAs<SharedItem::PointerType, Shared<SharedItem>>);
	static_assert(ValidSmartPointer<SharedItem::PointerType, SharedItem>);
	static_assert(!ValidSmartPointer<std::shared_ptr<SharedItem>, SharedItem>);
	SharedItem::PointerType clone = SharedItem(3).Clone();
	ASSERT_TRUE("test_shared_pointer_type", clone != nullptr);
	RETURN_TEST("test_shared_pointer_type", result);
}

// -------------------
// Unique
// -------------------

int test_unique_clone_copies() {
	int result = 0;
	UniqueItem original(42);
	UniqueItem::PointerType clone = original.Clone();
	ASSERT_TRUE("test_unique_clone_copies", clone != nullptr);
	ASSERT_EQUAL("test_unique_clone_copies", 42, clone->value);
	ASSERT_TRUE("test_unique_clone_copies", clone.get() != &original);
	RETURN_TEST("test_unique_clone_copies", result);
}

int test_unique_clone_is_independent() {
	int result = 0;
	UniqueItem original(1);
	UniqueItem::PointerType clone = original.Clone();
	clone->value = 99;
	ASSERT_EQUAL("test_unique_clone_is_independent", 1, original.value);
	ASSERT_EQUAL("test_unique_clone_is_independent", 99, clone->value);
	RETURN_TEST("test_unique_clone_is_independent", result);
}

int test_unique_converts_to_std_unique_ptr() {
	int result = 0;
	UniqueItem original(11);
	UniqueItem::PointerType item = original.Clone();
	std::unique_ptr<UniqueItem, Heap::ObjectDeleter> as_std = std::move(item);
	ASSERT_TRUE("test_unique_converts_to_std_unique_ptr", as_std != nullptr);
	ASSERT_EQUAL("test_unique_converts_to_std_unique_ptr", 11, as_std->value);
	ASSERT_EQUAL("test_unique_converts_to_std_unique_ptr", 11, consume_unique_ptr(as_std));
	RETURN_TEST("test_unique_converts_to_std_unique_ptr", result);
}

int test_unique_make_pointer() {
	int result = 0;
	UniqueItem::PointerType pointer = UniqueItem::MakePointer<UniqueItem>(8);
	ASSERT_TRUE("test_unique_make_pointer", pointer != nullptr);
	ASSERT_EQUAL("test_unique_make_pointer", 8, pointer->value);
	RETURN_TEST("test_unique_make_pointer", result);
}

int test_unique_move() {
	int result = 0;
	UniqueItem original(7);
	UniqueItem::PointerType moved = original.Move();
	ASSERT_TRUE("test_unique_move", moved != nullptr);
	ASSERT_EQUAL("test_unique_move", 7, moved->value);
	ASSERT_TRUE("test_unique_move", moved.get() != &original);
	RETURN_TEST("test_unique_move", result);
}

int test_unique_pointer_storage() {
	int result = 0;
	UniqueItem original(4);
	UniqueItem::PointerType item = original.Clone();
	ASSERT_EQUAL("test_unique_pointer_storage", 4, item->value);
	item->value = 20;
	ASSERT_EQUAL("test_unique_pointer_storage", 20, item->value);
	RETURN_TEST("test_unique_pointer_storage", result);
}

int test_unique_pointer_type() {
	int result = 0;
	static_assert(Type::SameAs<UniqueItem::PointerType, Unique<UniqueItem>>);
	static_assert(ValidSmartPointer<UniqueItem::PointerType, UniqueItem>);
	static_assert(!ValidSmartPointer<std::unique_ptr<UniqueItem>, UniqueItem>);
	UniqueItem::PointerType clone = UniqueItem(3).Clone();
	ASSERT_TRUE("test_unique_pointer_type", clone != nullptr);
	RETURN_TEST("test_unique_pointer_type", result);
}

int test_unique_reset_releases() {
	int result = 0;
	UniqueItem::PointerType pointer = UniqueItem::MakePointer<UniqueItem>(5);
	ASSERT_TRUE("test_unique_reset_releases", pointer != nullptr);
	pointer.reset();
	ASSERT_TRUE("test_unique_reset_releases", pointer == nullptr);
	RETURN_TEST("test_unique_reset_releases", result);
}

// -------------------
// Module boundary
// -------------------

int test_module_typeinfo_is_shared() {
	int result = 0;
	ASSERT_TRUE("test_module_typeinfo_is_shared", typeid(Clonable<PluginItem>) == PluginClonableType());
#ifndef WINDOWS
	// ELF / Mach-O: one merged type_info, so address-comparing runtimes (libc++) agree too.
	ASSERT_TRUE("test_module_typeinfo_is_shared", &typeid(Clonable<PluginItem>) == &PluginClonableType());
#endif
	RETURN_TEST("test_module_typeinfo_is_shared", result);
}

int test_module_dynamic_cast() {
	int result = 0;
	PluginItem::PointerType item = MakePluginItem(7);
	Clonable<PluginItem>* base = PluginAsClonable(*item);
	ASSERT_TRUE("test_module_dynamic_cast", dynamic_cast<PluginItem*>(base) == item.get());
	RETURN_TEST("test_module_dynamic_cast", result);
}

int test_module_clone_and_move() {
	int result = 0;
	PluginItem::PointerType item = MakePluginItem(12);
	PluginItem::PointerType clone = item->Clone();
	ASSERT_TRUE("test_module_clone_and_move", clone != nullptr);
	ASSERT_TRUE("test_module_clone_and_move", clone.get() != item.get());
	ASSERT_EQUAL("test_module_clone_and_move", 12, clone->value);
	PluginItem::PointerType moved = clone->Move();
	ASSERT_EQUAL("test_module_clone_and_move", 12, moved->value);
	std::shared_ptr<PluginItem> as_std = moved;
	item.reset();
	clone.reset();
	moved.reset();
	ASSERT_EQUAL("test_module_clone_and_move", 12, as_std->value);
	RETURN_TEST("test_module_clone_and_move", result);
}

int main() {
	int result = 0;

	// -------------------
	// Shared
	// -------------------
	result += test_shared_clone_copies();
	result += test_shared_clone_is_independent();
	result += test_shared_converts_to_std_shared_ptr();
	result += test_shared_make_pointer();
	result += test_shared_move();
	result += test_shared_pointer_storage();
	result += test_shared_pointer_type();

	// -------------------
	// Unique
	// -------------------
	result += test_unique_clone_copies();
	result += test_unique_clone_is_independent();
	result += test_unique_converts_to_std_unique_ptr();
	result += test_unique_make_pointer();
	result += test_unique_move();
	result += test_unique_pointer_storage();
	result += test_unique_pointer_type();
	result += test_unique_reset_releases();

	// -------------------
	// Module boundary
	// -------------------
	result += test_module_typeinfo_is_shared();
	result += test_module_dynamic_cast();
	result += test_module_clone_and_move();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
