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

#include <StormByte/error.hxx>
#include <StormByte/error.txx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <cstring>
#include <ranges>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace StormByte;

enum class Inventory {
	Success = 0,
	Missing,
	Locked
};

enum class Transit {
	Success = 0,
	Timeout,
	Refused
};

template<>
struct Error::Domain<Inventory> {
	static constexpr const char* Name = "StormByte::Inventory";

	static std::string Message(Inventory e) {
		switch (e) {
			case Inventory::Success:
				return "Success";
			case Inventory::Missing:
				return "Item not in stock";
			case Inventory::Locked:
				return "Item is locked";
		}
		return "Unknown inventory error";
	}
};

template<>
struct Error::Domain<Transit> {
	static constexpr const char* Name = "StormByte::Transit";

	static std::string Message(Transit e) {
		switch (e) {
			case Transit::Success:
				return "Success";
			case Transit::Timeout:
				return "Delivery timed out";
			case Transit::Refused:
				return "Delivery refused";
		}
		return "Unknown transit error";
	}
};

std::error_code make_error_code(Inventory e) noexcept {
	static Error::Category<Inventory> instance;
	return std::error_code(static_cast<int>(e), instance);
}

std::error_code make_error_code(Transit e) noexcept {
	static Error::Category<Transit> instance;
	return std::error_code(static_cast<int>(e), instance);
}

namespace std {
	template<>
	struct is_error_code_enum<Inventory>: true_type {};

	template<>
	struct is_error_code_enum<Transit>: true_type {};
}

class Warehouse {
	public:
		void Stock(std::string sku) {
			m_stock.push_back(std::move(sku));
		}

		void Lock(std::string sku) {
			m_locked.push_back(std::move(sku));
		}

		bool Contains(const std::string& sku, std::error_code& ec) const {
			if (std::ranges::find(m_locked, sku) != m_locked.end()) {
				ec = Inventory::Locked;
				return false;
			}
			if (std::ranges::find(m_stock, sku) == m_stock.end()) {
				ec = Inventory::Missing;
				return false;
			}
			ec = Inventory::Success;
			return true;
		}

		bool ContainsOrThrow(const std::string& sku) const {
			std::error_code ec;
			if (Contains(sku, ec))
				return true;
			throw std::system_error(ec);
		}

	private:
		std::vector<std::string> m_stock;
		std::vector<std::string> m_locked;
};

class Carrier {
	public:
		void Accept(std::string dest) {
			m_ok.push_back(std::move(dest));
		}

		void Refuse(std::string dest) {
			m_refused.push_back(std::move(dest));
		}

		bool Send(const std::string& dest, std::error_code& ec) const {
			if (std::ranges::find(m_refused, dest) != m_refused.end()) {
				ec = Transit::Refused;
				return false;
			}
			if (std::ranges::find(m_ok, dest) == m_ok.end()) {
				ec = Transit::Timeout;
				return false;
			}
			ec = Transit::Success;
			return true;
		}

	private:
		std::vector<std::string> m_ok;
		std::vector<std::string> m_refused;
};

// -------------------
// Domain
// -------------------

int test_domains_do_not_share_category() {
	Warehouse warehouse;
	Carrier carrier;
	std::error_code stock;
	std::error_code ship;
	warehouse.Contains("ghost", stock);
	carrier.Send("nowhere", ship);
	ASSERT_TRUE(stock == Inventory::Missing);
	ASSERT_TRUE(ship == Transit::Timeout);
	ASSERT_TRUE(&stock.category() != &ship.category());
	ASSERT_TRUE(stock != ship);
	ASSERT_EQUAL(std::string{"StormByte::Inventory"}, std::string{stock.category().name()});
	ASSERT_EQUAL(std::string{"StormByte::Transit"}, std::string{ship.category().name()});
	ASSERT_EQUAL(std::string{"Item not in stock"}, stock.message());
	ASSERT_EQUAL(std::string{"Delivery timed out"}, ship.message());
	RETURN_TEST(0);
}

int test_same_numeric_value_different_domain() {
	Warehouse warehouse;
	Carrier carrier;
	std::error_code stock;
	std::error_code ship;
	warehouse.Contains("ghost", stock);
	carrier.Send("nowhere", ship);
	ASSERT_EQUAL(stock.value(), ship.value());
	ASSERT_TRUE(stock != ship);
	RETURN_TEST(0);
}

// -------------------
// Fault
// -------------------

int test_fault_copy_is_independent() {
	Warehouse warehouse;
	std::error_code ec;
	warehouse.Contains("ghost", ec);
	Error::Fault original{ec};
	Error::Fault copy{original};
	ASSERT_TRUE(copy.code() == original.code());
	ASSERT_EQUAL(0, std::strcmp(copy.what(), original.what()));
	ASSERT_TRUE(copy.what() != original.what());
	Error::Fault taken(std::move(original));
	ASSERT_NOT_NULL(taken.what());
	ASSERT_CONTAINS(std::string_view{taken.what()}, "Item not in stock");
	RETURN_TEST(0);
}

int test_fault_default_is_success() {
	Error::Fault fault;
	ASSERT_FALSE(static_cast<bool>(fault));
	ASSERT_TRUE(fault.code() == Error::Code::Success);
	ASSERT_EQUAL(std::string{"StormByte: Success"}, std::string{fault.what()});
	RETURN_TEST(0);
}

int test_fault_from_enumerator() {
	const Error::Fault fault{Error::Code::Unknown};
	ASSERT_TRUE(static_cast<bool>(fault));
	ASSERT_TRUE(fault.code() == Error::Code::Unknown);
	ASSERT_EQUAL(std::string{"StormByte: Unknown StormByte error"}, std::string{fault.what()});
	RETURN_TEST(0);
}

int test_fault_from_warehouse() {
	Warehouse warehouse;
	std::error_code ec;
	warehouse.Lock("safe");
	warehouse.Contains("safe", ec);
	Error::Fault fault{ec};
	ASSERT_TRUE(static_cast<bool>(fault));
	ASSERT_TRUE(fault.code() == Inventory::Locked);
	ASSERT_EQUAL(std::string{"StormByte::Inventory: Item is locked"}, std::string{fault.what()});
	RETURN_TEST(0);
}

// -------------------
// Range
// -------------------

int test_any_of_stops_on_first_fault() {
	Warehouse warehouse;
	warehouse.Stock("nail");
	const std::vector<std::string> want{"ghost", "nail"};
	std::error_code ec;
	std::size_t calls = 0;
	const bool failed = std::ranges::any_of(want, [&](const std::string& sku) {
		++calls;
		return !warehouse.Contains(sku, ec);
	});
	ASSERT_TRUE(failed);
	ASSERT_EQUAL(std::size_t{1}, calls);
	ASSERT_TRUE(ec == Inventory::Missing);
	RETURN_TEST(0);
}

int test_for_each_collects_transit_codes() {
	Carrier carrier;
	carrier.Accept("port");
	carrier.Refuse("gate");
	const std::vector<std::string> dest{"port", "gate", "moon"};
	std::vector<std::error_code> codes;
	std::ranges::for_each(dest, [&](const std::string& stop) {
		std::error_code ec;
		carrier.Send(stop, ec);
		codes.push_back(ec);
	});
	ASSERT_EQUAL(std::size_t{3}, codes.size());
	ASSERT_TRUE(codes[0] == Transit::Success);
	ASSERT_FALSE(static_cast<bool>(codes[0]));
	ASSERT_TRUE(codes[1] == Transit::Refused);
	ASSERT_TRUE(codes[2] == Transit::Timeout);
	RETURN_TEST(0);
}

int test_ranges_all_of_reports_locked() {
	Warehouse warehouse;
	warehouse.Stock("nail");
	warehouse.Stock("bolt");
	warehouse.Lock("bolt");
	const std::vector<std::string> want{"nail", "bolt"};
	std::error_code ec;
	const bool ok = std::ranges::all_of(want, [&](const std::string& sku) {
		return warehouse.Contains(sku, ec);
	});
	ASSERT_FALSE(ok);
	ASSERT_TRUE(ec == Inventory::Locked);
	ASSERT_EQUAL(std::string{"Item is locked"}, ec.message());
	RETURN_TEST(0);
}

int test_ranges_find_if_reports_missing() {
	Warehouse warehouse;
	warehouse.Stock("nail");
	const std::vector<std::string> want{"nail", "ghost", "bolt"};
	std::error_code ec;
	const auto it = std::ranges::find_if(want, [&](const std::string& sku) {
		return !warehouse.Contains(sku, ec);
	});
	ASSERT_TRUE(it != want.end());
	ASSERT_EQUAL(std::string{"ghost"}, *it);
	ASSERT_TRUE(ec == Inventory::Missing);
	RETURN_TEST(0);
}

int test_system_error_propagates() {
	Warehouse warehouse;
	warehouse.Stock("nail");
	warehouse.Lock("bolt");
	const std::vector<std::string> want{"nail", "bolt"};
	ASSERT_THROWS(std::ranges::for_each(want, [&](const std::string& sku) {
		warehouse.ContainsOrThrow(sku);
	}), std::system_error);
	RETURN_TEST(0);
}

// -------------------
// Suite
// -------------------

int test_suite_category_is_singleton() {
	const std::error_code a = Error::Code::Success;
	const std::error_code b = Error::make_error_code(Error::Code::Unknown);
	ASSERT_TRUE(&a.category() == &b.category());
	ASSERT_TRUE(&a.category() == &Error::category());
	RETURN_TEST(0);
}

int test_suite_messages() {
	const std::error_code success = Error::Code::Success;
	const std::error_code unknown = Error::Code::Unknown;
	ASSERT_EQUAL(std::string{"Success"}, success.message());
	ASSERT_EQUAL(std::string{"Unknown StormByte error"}, unknown.message());
	ASSERT_EQUAL(0, success.value());
	ASSERT_EQUAL(1, unknown.value());
	RETURN_TEST(0);
}

int test_suite_success_is_falsy() {
	const std::error_code code = Error::Code::Success;
	ASSERT_FALSE(static_cast<bool>(code));
	ASSERT_EQUAL(std::string{"StormByte"}, std::string{code.category().name()});
	RETURN_TEST(0);
}

int test_suite_unknown() {
	const std::error_code code = Error::Code::Unknown;
	ASSERT_TRUE(static_cast<bool>(code));
	ASSERT_TRUE(code == Error::Code::Unknown);
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Domain
	// -------------------
	result += test_domains_do_not_share_category();
	result += test_same_numeric_value_different_domain();

	// -------------------
	// Fault
	// -------------------
	result += test_fault_copy_is_independent();
	result += test_fault_default_is_success();
	result += test_fault_from_enumerator();
	result += test_fault_from_warehouse();

	// -------------------
	// Range
	// -------------------
	result += test_any_of_stops_on_first_fault();
	result += test_for_each_collects_transit_codes();
	result += test_ranges_all_of_reports_locked();
	result += test_ranges_find_if_reports_missing();
	result += test_system_error_propagates();

	// -------------------
	// Suite
	// -------------------
	result += test_suite_category_is_singleton();
	result += test_suite_messages();
	result += test_suite_success_is_falsy();
	result += test_suite_unknown();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
