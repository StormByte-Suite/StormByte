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

namespace StormByte::Safe {
	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	template<class T>
	constexpr std::size_t Variant<Ts...>::AlternativeIndex() noexcept {
		std::size_t exact = npos;
		std::size_t current = 0;
		(([&] {
			if constexpr (Type::SameAs<std::remove_cvref_t<T>, Ts>)
				exact = current;
			++current;
		}()), ...);
		if (exact != npos)
			return exact;
		std::size_t found = npos;
		current = 0;
		bool ambiguous = false;
		(([&] {
			if constexpr (Type::ConstructibleFrom<Ts, T> && !Type::SameAs<std::remove_cvref_t<T>, Variant>) {
				if (found == npos)
					found = current;
				else
					ambiguous = true;
			}
			++current;
		}()), ...);
		return ambiguous ? npos : found;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	template<class Alternative, class... Args>
	Alternative* Variant<Ts...>::Make(Args&&... args) {
		void* block = Heap::Allocate(sizeof(Alternative));
		try {
			return new (block) Alternative(std::forward<Args>(args)...);
		} catch (...) {
			Heap::Free(block);
			throw;
		}
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	void Variant<Ts...>::Release() noexcept {
		if (m_storage == nullptr)
			return;
		std::size_t current = 0;
		auto destroy = [&](auto* unused) {
			using Alternative = std::remove_pointer_t<decltype(unused)>;
			if (current == m_index)
				static_cast<Alternative*>(m_storage)->~Alternative();
			++current;
		};
		(destroy(static_cast<Ts*>(nullptr)), ...);
		Heap::Free(m_storage);
		m_storage = nullptr;
		m_index = npos;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	Variant<Ts...>::Variant(): m_storage(nullptr), m_index(npos) {
		using First = std::tuple_element_t<0, std::tuple<Ts...>>;
		m_storage = Make<First>();
		m_index = 0;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	template<std::size_t I, class... Args>
	Variant<Ts...>::Variant(std::in_place_index_t<I>, Args&&... args): m_storage(nullptr), m_index(npos) {
		using Alternative = std::tuple_element_t<I, std::tuple<Ts...>>;
		m_storage = Make<Alternative>(std::forward<Args>(args)...);
		m_index = I;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	template<class T, class... Args>
	Variant<Ts...>::Variant(std::in_place_type_t<T>, Args&&... args): m_storage(nullptr), m_index(npos) {
		m_storage = Make<T>(std::forward<Args>(args)...);
		m_index = AlternativeIndex<T>();
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	Variant<Ts...>::Variant(const std::variant<Ts...>& value): m_storage(nullptr), m_index(npos) {
		if (value.valueless_by_exception())
			return;
		std::visit([&](const auto& alternative) {
			using Alternative = std::remove_cvref_t<decltype(alternative)>;
			m_storage = Make<Alternative>(alternative);
			m_index = value.index();
		}, value);
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	Variant<Ts...>::Variant(std::variant<Ts...>&& value): m_storage(nullptr), m_index(npos) {
		if (value.valueless_by_exception())
			return;
		std::visit([&](auto& alternative) {
			using Alternative = std::remove_cvref_t<decltype(alternative)>;
			m_storage = Make<Alternative>(std::move(alternative));
			m_index = value.index();
		}, value);
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	Variant<Ts...>::Variant(const Variant& other): m_storage(nullptr), m_index(npos) {
		if (other.valueless_by_exception())
			return;
		std::size_t current = 0;
		auto copy = [&](auto* unused) {
			using Alternative = std::remove_pointer_t<decltype(unused)>;
			if (current == other.m_index) {
				m_storage = Make<Alternative>(*static_cast<const Alternative*>(other.m_storage));
				m_index = other.m_index;
			}
			++current;
		};
		(copy(static_cast<Ts*>(nullptr)), ...);
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	Variant<Ts...>::Variant(Variant&& other) noexcept: m_storage(other.m_storage), m_index(other.m_index) {
		other.m_storage = nullptr;
		other.m_index = npos;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	Variant<Ts...>::~Variant() noexcept {
		Release();
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	Variant<Ts...>& Variant<Ts...>::operator=(const Variant& other) {
		if (this == &other)
			return *this;
		Variant copied(other);
		swap(copied);
		return *this;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	Variant<Ts...>& Variant<Ts...>::operator=(Variant&& other) noexcept {
		if (this != &other) {
			Release();
			m_storage = other.m_storage;
			m_index = other.m_index;
			other.m_storage = nullptr;
			other.m_index = npos;
		}
		return *this;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	std::size_t Variant<Ts...>::index() const noexcept {
		return m_index;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	bool Variant<Ts...>::valueless_by_exception() const noexcept {
		return m_index == npos;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	template<std::size_t I, class... Args>
	auto& Variant<Ts...>::emplace(Args&&... args) {
		using Alternative = std::tuple_element_t<I, std::tuple<Ts...>>;
		Alternative* created = Make<Alternative>(std::forward<Args>(args)...);
		Release();
		m_storage = created;
		m_index = I;
		return *created;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	template<class T, class... Args>
	T& Variant<Ts...>::emplace(Args&&... args) {
		T* created = Make<T>(std::forward<Args>(args)...);
		Release();
		m_storage = created;
		m_index = AlternativeIndex<T>();
		return *created;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	void Variant<Ts...>::swap(Variant& other) noexcept {
		std::swap(m_storage, other.m_storage);
		std::swap(m_index, other.m_index);
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	bool Variant<Ts...>::operator==(const Variant& other) const {
		if (m_index != other.m_index)
			return false;
		if (valueless_by_exception())
			return true;
		bool equal = false;
		std::size_t current = 0;
		auto compare = [&](auto* unused) {
			using Alternative = std::remove_pointer_t<decltype(unused)>;
			if (current == m_index)
				equal = *static_cast<const Alternative*>(m_storage) == *static_cast<const Alternative*>(other.m_storage);
			++current;
		};
		(compare(static_cast<Ts*>(nullptr)), ...);
		return equal;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	std::strong_ordering Variant<Ts...>::operator<=>(const Variant& other) const {
		if (m_index != other.m_index)
			return m_index <=> other.m_index;
		if (valueless_by_exception())
			return std::strong_ordering::equal;
		std::strong_ordering order = std::strong_ordering::equal;
		std::size_t current = 0;
		auto compare = [&](auto* unused) {
			using Alternative = std::remove_pointer_t<decltype(unused)>;
			if (current == m_index)
				order = *static_cast<const Alternative*>(m_storage) <=> *static_cast<const Alternative*>(other.m_storage);
			++current;
		};
		(compare(static_cast<Ts*>(nullptr)), ...);
		return order;
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	template<std::size_t I, class Visitor>
	decltype(auto) Variant<Ts...>::VisitIndex(Visitor&& visitor) const {
		using Alternative = std::tuple_element_t<I, std::tuple<Ts...>>;
		if (m_index == I)
			return std::invoke(std::forward<Visitor>(visitor), *static_cast<const Alternative*>(m_storage));
		if constexpr (I + 1 < sizeof...(Ts))
			return VisitIndex<I + 1>(std::forward<Visitor>(visitor));
		else
			throw BadVariantAccess("wrong alternative");
	}

	template<Type::SafeValue... Ts>
	requires (sizeof...(Ts) > 0)
	template<class Visitor>
	decltype(auto) Variant<Ts...>::Visit(Visitor&& visitor) const {
		if (valueless_by_exception())
			throw BadVariantAccess("wrong alternative");
		return VisitIndex<0>(std::forward<Visitor>(visitor));
	}

	template<class T, Type::SafeValue... Ts>
	bool holds_alternative(const Variant<Ts...>& value) noexcept {
		return value.index() == Variant<Ts...>::template AlternativeIndex<T>();
	}

	template<std::size_t I, Type::SafeValue... Ts>
	auto& get(Variant<Ts...>& value) {
		if (value.index() != I)
			throw BadVariantAccess("wrong alternative");
		return *get_if<I>(&value);
	}

	template<std::size_t I, Type::SafeValue... Ts>
	const auto& get(const Variant<Ts...>& value) {
		if (value.index() != I)
			throw BadVariantAccess("wrong alternative");
		return *get_if<I>(&value);
	}

	template<class T, Type::SafeValue... Ts>
	T& get(Variant<Ts...>& value) {
		T* alternative = get_if<T>(&value);
		if (alternative == nullptr)
			throw BadVariantAccess("wrong alternative");
		return *alternative;
	}

	template<class T, Type::SafeValue... Ts>
	const T& get(const Variant<Ts...>& value) {
		const T* alternative = get_if<T>(&value);
		if (alternative == nullptr)
			throw BadVariantAccess("wrong alternative");
		return *alternative;
	}

	template<std::size_t I, Type::SafeValue... Ts>
	auto* get_if(Variant<Ts...>* value) noexcept {
		using Alternative = std::tuple_element_t<I, std::tuple<Ts...>>;
		if (value == nullptr || value->index() != I)
			return static_cast<Alternative*>(nullptr);
		return static_cast<Alternative*>(value->m_storage);
	}

	template<std::size_t I, Type::SafeValue... Ts>
	const auto* get_if(const Variant<Ts...>* value) noexcept {
		using Alternative = std::tuple_element_t<I, std::tuple<Ts...>>;
		if (value == nullptr || value->index() != I)
			return static_cast<const Alternative*>(nullptr);
		return static_cast<const Alternative*>(value->m_storage);
	}

	template<class T, Type::SafeValue... Ts>
	T* get_if(Variant<Ts...>* value) noexcept {
		if (value == nullptr || value->index() != Variant<Ts...>::template AlternativeIndex<T>())
			return nullptr;
		return static_cast<T*>(value->m_storage);
	}

	template<class T, Type::SafeValue... Ts>
	const T* get_if(const Variant<Ts...>* value) noexcept {
		if (value == nullptr || value->index() != Variant<Ts...>::template AlternativeIndex<T>())
			return nullptr;
		return static_cast<const T*>(value->m_storage);
	}

	template<class Visitor, Type::SafeValue... Ts>
	decltype(auto) visit(Visitor&& visitor, Variant<Ts...>& value) {
		if (value.valueless_by_exception())
			throw BadVariantAccess("wrong alternative");
		return value.Visit(std::forward<Visitor>(visitor));
	}

	template<class Visitor, Type::SafeValue... Ts>
	decltype(auto) visit(Visitor&& visitor, const Variant<Ts...>& value) {
		if (value.valueless_by_exception())
			throw BadVariantAccess("wrong alternative");
		return value.Visit(std::forward<Visitor>(visitor));
	}

	template<Type::SafeValue... Ts>
	void swap(Variant<Ts...>& left, Variant<Ts...>& right) noexcept {
		left.swap(right);
	}
}
