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
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/test_handlers.h>

#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

using namespace StormByte;
using namespace StormByte::Safe;

namespace {
	class Base {
		public:
			explicit Base(int value): value(value) {}
			virtual ~Base() = default;
			virtual int id() const { return 1; }
			int value;
	};

	class Derived: public Base {
		public:
			static int destroyed;

			explicit Derived(int value): Base(value) {}
			~Derived() override { ++destroyed; }
			int id() const override { return 2; }
	};

	class Plain {
		public:
			~Plain() = default;
	};

	class PlainChild: public Plain {};

	class FailingConstructor {
		public:
			explicit FailingConstructor(int failure) {
				if (failure == 0)
					throw std::bad_alloc();
				if (failure == 1)
					throw std::runtime_error("Foreign constructor failure");
				if (failure == 2)
					throw OutOfBoundsError("StormByte constructor failure");
				throw 42;
			}
	};

	int Derived::destroyed = 0;
}

// -------------------
// Shared
// -------------------

int test_shared_make_shared_constructs() {
	int result = 0;
	Shared<Base> item = Heap::MakeShared<Base>(9);
	ASSERT_TRUE("test_shared_make_shared_constructs", item != nullptr);
	ASSERT_EQUAL("test_shared_make_shared_constructs", 9, item->value);
	ASSERT_EQUAL("test_shared_make_shared_constructs", 1, item->id());
	ASSERT_TRUE("test_shared_make_shared_constructs", item.use_count() == 1);
	RETURN_TEST("test_shared_make_shared_constructs", result);
}

int test_shared_converts_to_std_shared_ptr() {
	int result = 0;
	Shared<Base> item = Heap::MakeShared<Base>(11);
	std::shared_ptr<Base> as_std = item;
	ASSERT_TRUE("test_shared_converts_to_std_shared_ptr", as_std != nullptr);
	ASSERT_TRUE("test_shared_converts_to_std_shared_ptr", as_std.get() == item.get());
	ASSERT_EQUAL("test_shared_converts_to_std_shared_ptr", 11, as_std->value);
	ASSERT_TRUE("test_shared_converts_to_std_shared_ptr", item.use_count() == 2);
	RETURN_TEST("test_shared_converts_to_std_shared_ptr", result);
}

int test_shared_make_pointer_keeps_derived() {
	int result = 0;
	Derived::destroyed = 0;
	Shared<Base> item = Shared<Base>::MakePointer<Derived>(4);
	ASSERT_EQUAL("test_shared_make_pointer_keeps_derived", 2, item->id());
	ASSERT_EQUAL("test_shared_make_pointer_keeps_derived", 4, item->value);
	item.reset();
	ASSERT_EQUAL("test_shared_make_pointer_keeps_derived", 1, Derived::destroyed);
	ASSERT_TRUE("test_shared_make_pointer_keeps_derived", item == nullptr);
	RETURN_TEST("test_shared_make_pointer_keeps_derived", result);
}

int test_shared_swap_and_compare() {
	int result = 0;
	Shared<Base> first = Heap::MakeShared<Base>(1);
	Shared<Base> second = Heap::MakeShared<Base>(2);
	ASSERT_TRUE("test_shared_swap_and_compare", first != second);
	first.swap(second);
	ASSERT_EQUAL("test_shared_swap_and_compare", 2, first->value);
	ASSERT_EQUAL("test_shared_swap_and_compare", 1, second->value);
	ASSERT_TRUE("test_shared_swap_and_compare", (first <=> second) != 0);
	RETURN_TEST("test_shared_swap_and_compare", result);
}

int test_shared_rejects_std_shared_ptr() {
	int result = 0;
	static_assert(!std::is_constructible_v<Shared<Base>, std::shared_ptr<Base>>);
	static_assert(!std::is_constructible_v<Shared<Base>, std::shared_ptr<Derived>>);
	ASSERT_TRUE("test_shared_rejects_std_shared_ptr", true);
	RETURN_TEST("test_shared_rejects_std_shared_ptr", result);
}

// -------------------
// Unique
// -------------------

int test_unique_make_unique_constructs() {
	int result = 0;
	Unique<Base> item = Heap::MakeUnique<Base>(9);
	ASSERT_TRUE("test_unique_make_unique_constructs", item != nullptr);
	ASSERT_EQUAL("test_unique_make_unique_constructs", 9, item->value);
	ASSERT_EQUAL("test_unique_make_unique_constructs", 1, item->id());
	RETURN_TEST("test_unique_make_unique_constructs", result);
}

int test_unique_converts_to_std_unique_ptr() {
	int result = 0;
	Unique<Base> item = Heap::MakeUnique<Base>(11);
	std::unique_ptr<Base, Heap::ObjectDeleter> as_std = std::move(item);
	ASSERT_TRUE("test_unique_converts_to_std_unique_ptr", item == nullptr);
	ASSERT_TRUE("test_unique_converts_to_std_unique_ptr", as_std != nullptr);
	ASSERT_EQUAL("test_unique_converts_to_std_unique_ptr", 11, as_std->value);
	RETURN_TEST("test_unique_converts_to_std_unique_ptr", result);
}

int test_unique_make_pointer_keeps_derived() {
	int result = 0;
	Derived::destroyed = 0;
	Unique<Base> item = Unique<Base>::MakePointer<Derived>(4);
	ASSERT_EQUAL("test_unique_make_pointer_keeps_derived", 2, item->id());
	std::unique_ptr<Base, Heap::ObjectDeleter> as_std = std::move(item);
	as_std.reset();
	ASSERT_EQUAL("test_unique_make_pointer_keeps_derived", 1, Derived::destroyed);
	RETURN_TEST("test_unique_make_pointer_keeps_derived", result);
}

int test_unique_rejects_std_unique_ptr() {
	int result = 0;
	static_assert(!std::is_constructible_v<Unique<Base>, std::unique_ptr<Base>>);
	static_assert(!std::is_constructible_v<Unique<Base>, std::unique_ptr<Base, Heap::ObjectDeleter>>);
	ASSERT_TRUE("test_unique_rejects_std_unique_ptr", true);
	RETURN_TEST("test_unique_rejects_std_unique_ptr", result);
}

// -------------------
// Weak
// -------------------

int test_weak_locks_and_expires() {
	int result = 0;
	Shared<Base> item = Heap::MakeShared<Base>(3);
	Weak<Base> watch(item);
	ASSERT_TRUE("test_weak_locks_and_expires", !watch.expired());
	ASSERT_TRUE("test_weak_locks_and_expires", watch.use_count() == 1);
	Shared<Base> locked = watch.lock();
	ASSERT_EQUAL("test_weak_locks_and_expires", 3, locked->value);
	ASSERT_TRUE("test_weak_locks_and_expires", locked.get() == item.get());
	item.reset();
	locked.reset();
	ASSERT_TRUE("test_weak_locks_and_expires", watch.expired());
	ASSERT_TRUE("test_weak_locks_and_expires", watch.lock() == nullptr);
	RETURN_TEST("test_weak_locks_and_expires", result);
}

int test_weak_rejects_std_weak_ptr() {
	int result = 0;
	static_assert(!std::is_constructible_v<Weak<Base>, std::weak_ptr<Base>>);
	static_assert(!std::is_constructible_v<Weak<Base>, std::shared_ptr<Base>>);
	ASSERT_TRUE("test_weak_rejects_std_weak_ptr", true);
	RETURN_TEST("test_weak_rejects_std_weak_ptr", result);
}

int test_stormbyte_pointer_failures() {
	int result = 0;
	Weak<Base> empty;
	ASSERT_THROWS("test_stormbyte_pointer_failures", Shared<Base>(empty), ExpiredWeakPointerError);
	auto owner = Heap::MakeShared<Base>(7);
	Weak<Base> observer(owner);
	ASSERT_EQUAL("test_stormbyte_pointer_failures", Shared<Base>(observer)->value, 7);
	owner.reset();
	ASSERT_THROWS("test_stormbyte_pointer_failures", Shared<Base>(observer), ExpiredWeakPointerError);
	ASSERT_THROWS("test_stormbyte_pointer_failures", Heap::Allocate(std::numeric_limits<std::size_t>::max()), AllocationError);
	RETURN_TEST("test_stormbyte_pointer_failures", result);
}

int test_factory_exception_translation() {
	int result = 0;
	ASSERT_THROWS("test_factory_exception_translation", Heap::MakeShared<FailingConstructor>(0), AllocationError);
	ASSERT_THROWS("test_factory_exception_translation", Heap::MakeUnique<FailingConstructor>(0), AllocationError);
	ASSERT_THROWS("test_factory_exception_translation", Heap::MakeShared<FailingConstructor>(1), OperationError);
	ASSERT_THROWS("test_factory_exception_translation", Heap::MakeUnique<FailingConstructor>(1), OperationError);
	ASSERT_THROWS("test_factory_exception_translation", Heap::MakeShared<FailingConstructor>(2), OutOfBoundsError);
	ASSERT_THROWS("test_factory_exception_translation", Heap::MakeUnique<FailingConstructor>(2), OutOfBoundsError);
	ASSERT_THROWS("test_factory_exception_translation", Heap::MakeShared<FailingConstructor>(3), OperationError);
	ASSERT_THROWS("test_factory_exception_translation", Heap::MakeUnique<FailingConstructor>(3), OperationError);
	Safe::String text("unchanged");
	ASSERT_THROWS("test_factory_exception_translation", text.reserve(std::numeric_limits<std::size_t>::max()), OutOfBoundsError);
	ASSERT_EQUAL("test_factory_exception_translation", std::string(text), std::string("unchanged"));
	Safe::WString wide(L"unchanged");
	ASSERT_THROWS("test_factory_exception_translation", wide.reserve(std::numeric_limits<std::size_t>::max() / sizeof(wchar_t)), OutOfBoundsError);
	ASSERT_TRUE("test_factory_exception_translation", std::wstring(wide) == L"unchanged");
	RETURN_TEST("test_factory_exception_translation", result);
}

// -------------------
// Cast
// -------------------

int test_static_pointer_cast_keeps_derived() {
	int result = 0;
	Derived::destroyed = 0;
	Shared<Derived> derived = Heap::MakeShared<Derived>(6);
	Shared<Base> base = StaticPointerCast<Base>(derived);
	ASSERT_EQUAL("test_static_pointer_cast_keeps_derived", 2, base->id());
	derived.reset();
	base.reset();
	ASSERT_EQUAL("test_static_pointer_cast_keeps_derived", 1, Derived::destroyed);
	RETURN_TEST("test_static_pointer_cast_keeps_derived", result);
}

template<class T, class Target>
concept UniqueDefaultMake = requires {
	Unique<T>::template MakePointer<Target>();
};

int test_unique_make_pointer_requires_virtual_destructor() {
	int result = 0;
	static_assert(UniqueDefaultMake<Plain, Plain>);
	static_assert(!UniqueDefaultMake<Plain, PlainChild>);
	static_assert(requires { Unique<Base>::MakePointer<Derived>(1); });
	ASSERT_TRUE("test_unique_make_pointer_requires_virtual_destructor", true);
	RETURN_TEST("test_unique_make_pointer_requires_virtual_destructor", result);
}

int main() {
	int result = 0;

	// -------------------
	// Shared
	// -------------------
	result += test_shared_make_shared_constructs();
	result += test_shared_converts_to_std_shared_ptr();
	result += test_shared_make_pointer_keeps_derived();
	result += test_shared_swap_and_compare();
	result += test_shared_rejects_std_shared_ptr();

	// -------------------
	// Unique
	// -------------------
	result += test_unique_make_unique_constructs();
	result += test_unique_converts_to_std_unique_ptr();
	result += test_unique_make_pointer_keeps_derived();
	result += test_unique_rejects_std_unique_ptr();

	// -------------------
	// Weak
	// -------------------
	result += test_weak_locks_and_expires();
	result += test_weak_rejects_std_weak_ptr();
	result += test_stormbyte_pointer_failures();
	result += test_factory_exception_translation();

	// -------------------
	// Cast
	// -------------------
	result += test_static_pointer_cast_keeps_derived();
	result += test_unique_make_pointer_requires_virtual_destructor();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
