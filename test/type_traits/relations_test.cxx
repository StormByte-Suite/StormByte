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

#include <StormByte/exception.hxx>
#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>
#include <StormByte/type_traits/relations.hxx>

#include <iostream>
#include <string>

using namespace StormByte;

namespace {
	struct Base {};
	struct Derived: Base {};
	struct PrivateDerived: private Base {};
	struct Sibling {};

	int Add(int left, int right) {
		return left + right;
	}

	void Ignore(int) {}

	struct Pred {
		bool operator()(int value) const { return value > 0; }
	};
}

// -------------------
// Inheritance
// -------------------

int test_inheritance_is_base_of() {
	static_assert(Type::DerivedFrom<Derived, Base>);
	static_assert(Type::DerivedFrom<Derived, Derived>);
	static_assert(Type::DerivedFrom<PrivateDerived, Base>);
	static_assert(Type::DerivedFrom<const Derived, Base>);
	static_assert(Type::DerivedFrom<Base64Error, Exception>);
	static_assert(Type::DerivedFrom<Safe::Exception, Exception>);
	static_assert(!Type::DerivedFrom<Base, Derived>);
	static_assert(!Type::DerivedFrom<Sibling, Base>);
	static_assert(!Type::DerivedFrom<int, Base>);
	static_assert(!Type::DerivedFrom<Exception, std::exception>);
	ASSERT_TRUE((Type::DerivedFrom<PrivateDerived, Base>));
	ASSERT_FALSE((Type::DerivedFrom<Exception, std::exception>));
	RETURN_TEST(0);
}

// -------------------
// Invoke
// -------------------

int test_invoke_callable_and_predicate() {
	static_assert(Type::Callable<decltype(Add), int, int>);
	static_assert(Type::Callable<Pred, int>);
	static_assert(!Type::Callable<decltype(Add), Safe::String>);
	static_assert(!Type::Callable<int, int>);
	static_assert(Type::Invocable<decltype(Add), int, int>);
	static_assert(Type::Invocable<Pred, int>);
	static_assert(!Type::Invocable<decltype(Add), std::string>);
	static_assert(Type::InvocableR<int, decltype(Add), int, int>);
	static_assert(Type::InvocableR<bool, Pred, int>);
	static_assert(Type::InvocableR<long, decltype(Add), int, int>);
	static_assert(!Type::InvocableR<Safe::String, decltype(Add), int, int>);
	static_assert(Type::Predicate<Pred, int>);
	static_assert(Type::Predicate<bool(*)(int), int>);
	static_assert(Type::Predicate<decltype(Add), int, int>);
	static_assert(!Type::Predicate<decltype(Ignore), int>);
	ASSERT_EQUAL(5, Add(2, 3));
	ASSERT_TRUE(Pred{}(1));
	ASSERT_FALSE(Pred{}(0));
	ASSERT_TRUE((Type::Predicate<Pred, int>));
	ASSERT_FALSE((Type::Predicate<decltype(Ignore), int>));
	RETURN_TEST(0);
}

// -------------------
// Same
// -------------------

int test_same_strips_cvref() {
	static_assert(Type::SameAs<int, int>);
	static_assert(Type::SameAs<int, const int&>);
	static_assert(Type::SameAs<const volatile int&&, int>);
	static_assert(Type::SameAs<Size, const Size&>);
	static_assert(Type::SameAs<Safe::String, Safe::String&&>);
	static_assert(!Type::SameAs<int, long>);
	static_assert(!Type::SameAs<int, const int*>);
	static_assert(!Type::SameAs<Size, Safe::String>);
	static_assert(!Type::SameAs<std::string, Safe::String>);
	ASSERT_TRUE((Type::SameAs<const Safe::String&, Safe::String>));
	ASSERT_FALSE((Type::SameAs<int, Size>));
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Inheritance
	// -------------------
	result += test_inheritance_is_base_of();

	// -------------------
	// Invoke
	// -------------------
	result += test_invoke_callable_and_predicate();

	// -------------------
	// Same
	// -------------------
	result += test_same_strips_cvref();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
