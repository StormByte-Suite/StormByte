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

#pragma once

#include <algorithm>
#include <vector>

namespace StormByte {
	template<typename T>
	Safe::Binary Serializable<T>::Serialize() const noexcept {
		if constexpr (Type::Optional<T>) {
			return SerializeOptional();
		} else if constexpr (Type::Queue<T>) {
			return SerializeQueue();
		} else if constexpr (Type::Pair<T>) {
			return SerializePair();
		} else if constexpr (Type::Container<T>) {
			return SerializeContainer();
		} else if constexpr (Type::TriviallyCopyable<T>) {
			return SerializeTrivial();
		} else {
			return Detail::Codec<DecayedT>::Write(m_data);
		}
	}

	template<typename T>
	Expected<T, DeserializeError> Serializable<T>::Deserialize(std::span<const std::byte> data) noexcept {
		if constexpr (Type::Optional<T>) {
			return DeserializeOptional(data);
		} else if constexpr (Type::Queue<T>) {
			return DeserializeQueue(data);
		} else if constexpr (Type::Pair<T>) {
			return DeserializePair(data);
		} else if constexpr (Type::Container<T>) {
			return DeserializeContainer(data);
		} else if constexpr (Type::TriviallyCopyable<T>) {
			return DeserializeTrivial(data);
		} else {
			return Detail::Codec<DecayedT>::Read(data);
		}
	}

	template<typename T>
	Expected<T, DeserializeError> Serializable<T>::Deserialize(const Safe::Binary& data) noexcept {
		return Deserialize(data.span());
	}

	template<typename T>
	ByteSize Serializable<T>::Size(const DecayedT& data) noexcept {
		if constexpr (Type::Optional<T>) {
			return SizeOptional(data);
		} else if constexpr (Type::Queue<T>) {
			return SizeQueue(data);
		} else if constexpr (Type::Pair<T>) {
			return SizePair(data);
		} else if constexpr (Type::Container<T>) {
			return SizeContainer(data);
		} else if constexpr (Type::TriviallyCopyable<T>) {
			return ByteSize{sizeof(DecayedT)};
		} else {
			return Detail::Codec<DecayedT>::Size(data);
		}
	}

	template<typename T>
	template<typename U>
	Safe::Binary Serializable<T>::SerializeTrivial() const noexcept
	requires Type::TriviallyCopyable<U> {
		if constexpr (Type::SameAs<T, bool>) {
			return Safe::Binary(ByteSize{1}, std::byte{m_data ? std::byte{1} : std::byte{0}});
		}
		auto value = m_data;
		if constexpr (std::is_integral_v<T> && sizeof(T) > 1 &&
				std::endian::native != std::endian::little) {
			value = Type::Detail::swap_endian(value);
		}

		const auto* raw = reinterpret_cast<const std::byte*>(&value);
		return Safe::Binary(raw, ByteSize{sizeof(value)});
	}

	template<typename T>
	template<typename U>
	Safe::Binary Serializable<T>::SerializeContainer() const noexcept
	requires Type::Container<U> {
		const std::uint64_t size = static_cast<std::uint64_t>(m_data.size());
		Safe::Binary buffer = Serializable<std::uint64_t>(size).Serialize();
		buffer.reserve(ByteSize{static_cast<std::size_t>(buffer.size())} + SizeContainer(m_data));
		using ElementT = typename DecayedT::value_type;
		if constexpr (requires { typename DecayedT::hasher; }) {
			std::vector<const ElementT*> ordered;
			ordered.reserve(m_data.size());
			for (const auto& element : m_data)
				ordered.push_back(&element);
			std::sort(ordered.begin(), ordered.end(), [](const ElementT* left, const ElementT* right) {
				if constexpr (requires { left->first < right->first; })
					return left->first < right->first;
				else
					return *left < *right;
			});
			for (const ElementT* element : ordered)
				append_bytes(buffer, Serializable<ElementT>(*element).Serialize());
		} else {
			for (const auto& element : m_data) {
				ElementT snapshot = element;
				Serializable<ElementT> element_serial(snapshot);
				append_bytes(buffer, element_serial.Serialize());
			}
		}
		return buffer;
	}

	template<typename T>
	template<typename U>
	Safe::Binary Serializable<T>::SerializePair() const noexcept
	requires Type::Pair<U> {
		Serializable<std::remove_cvref_t<typename T::first_type>> first_serial(m_data.first);
		Serializable<std::remove_cvref_t<typename T::second_type>> second_serial(m_data.second);
		Safe::Binary buffer;
		buffer.reserve(SizePair(m_data));
		append_bytes(buffer, first_serial.Serialize());
		append_bytes(buffer, second_serial.Serialize());
		return buffer;
	}

	template<typename T>
	template<typename U>
	Safe::Binary Serializable<T>::SerializeOptional() const noexcept
	requires Type::Optional<U> {
		const bool has_value = m_data.has_value();
		Safe::Binary buffer;
		buffer.reserve(SizeOptional(m_data));
		append_bytes(buffer, Serializable<bool>(has_value).Serialize());
		if (m_data.has_value()) {
			using ValueT = typename DecayedT::value_type;
			ValueT value = m_data.value();
			Serializable<ValueT> value_serial(value);
			append_bytes(buffer, value_serial.Serialize());
		}
		return buffer;
	}

	template<typename T>
	template<typename U>
	Safe::Binary Serializable<T>::SerializeQueue() const noexcept
	requires Type::Queue<U> {
		const std::uint64_t count = static_cast<std::uint64_t>(m_data.size());
		Safe::Binary buffer = Serializable<std::uint64_t>(count).Serialize();
		buffer.reserve(ByteSize{static_cast<std::size_t>(buffer.size())} + SizeQueue(m_data));
		DecayedT queue = m_data;
		using ElementT = typename DecayedT::value_type;
		while (!queue.empty()) {
			ElementT element = queue.front();
			queue.pop();
			Serializable<ElementT> element_serial(element);
			append_bytes(buffer, element_serial.Serialize());
		}
		return buffer;
	}

	template<typename T>
	template<typename U>
	ByteSize Serializable<T>::SizeContainer(const DecayedT& data) noexcept
	requires Type::Container<U> {
		ByteSize size{sizeof(std::uint64_t)};
		for (const auto& element : data) {
			typename DecayedT::value_type snapshot = element;
			size += Serializable<typename DecayedT::value_type>::Size(snapshot);
		}
		return size;
	}

	template<typename T>
	template<typename U>
	ByteSize Serializable<T>::SizePair(const DecayedT& data) noexcept
	requires Type::Pair<U> {
		return
			Serializable<std::remove_cvref_t<typename T::first_type>>::Size(data.first) +
			Serializable<std::remove_cvref_t<typename T::second_type>>::Size(data.second);
	}

	template<typename T>
	template<typename U>
	ByteSize Serializable<T>::SizeOptional(const DecayedT& data) noexcept
	requires Type::Optional<U> {
		ByteSize size{sizeof(bool)};
		if (data.has_value()) {
			size += Serializable<std::remove_cvref_t<decltype(data.value())>>::Size(data.value());
		}
		return size;
	}

	template<typename T>
	template<typename U>
	ByteSize Serializable<T>::SizeQueue(const DecayedT& data) noexcept
	requires Type::Queue<U> {
		ByteSize size{sizeof(std::uint64_t)};
		DecayedT queue = data;
		using ElementT = typename DecayedT::value_type;
		while (!queue.empty()) {
			ElementT element = queue.front();
			queue.pop();
			size += Serializable<ElementT>::Size(element);
		}
		return size;
	}

	template<typename T>
	template<typename U>
	Expected<T, DeserializeError> Serializable<T>::DeserializeTrivial(std::span<const std::byte> data) noexcept
	requires Type::TriviallyCopyable<U> {
		if constexpr (Type::SameAs<T, bool>) {
			if (data.empty())
				return Unexpected<DeserializeError>("Insufficient data for bool");

			const auto raw = static_cast<unsigned char>(data[0]);
			if (raw != 0 && raw != 1)
				return Unexpected<DeserializeError>("Invalid bool value in stream");

			return raw != 0;
		} else {
			if (data.size() < sizeof(T))
				return Unexpected<DeserializeError>("Insufficient data for deserialization");

			T result;
			std::memcpy(&result, data.data(), sizeof(T));

			if constexpr (std::endian::native != std::endian::little) {
				result = Type::Detail::swap_endian(result);
			}

			return result;
		}
	}

	template<typename T>
	template<typename U>
	Expected<T, DeserializeError> Serializable<T>::DeserializeContainer(std::span<const std::byte> data) noexcept
	requires Type::Container<U> {
		std::size_t offset = 0;

		if (offset + sizeof(std::uint64_t) > data.size())
			return Unexpected<DeserializeError>("Insufficient data for container size");

		auto expected_container_size = Serializable<std::uint64_t>::Deserialize(
			data.subspan(offset, sizeof(std::uint64_t)));
		if (!expected_container_size)
			return Unexpected(expected_container_size.error());

		const std::uint64_t size = expected_container_size.value();
		offset += sizeof(std::uint64_t);

		if (size > static_cast<std::uint64_t>(data.size() - offset))
			return Unexpected<DeserializeError>("Claimed container size exceeds remaining buffer");
		if constexpr (Type::Array<T>) {
			if (size != static_cast<std::uint64_t>(std::tuple_size_v<T>))
				return Unexpected<DeserializeError>("Array size does not match serialized element count");
		}

		T container;
		for (std::uint64_t i = 0; i < size; ++i) {
			using ElementT = std::remove_cvref_t<typename T::value_type>;

			if (offset >= data.size())
				return Unexpected<DeserializeError>("Insufficient data for container element");

			auto expected_element = Serializable<ElementT>::Deserialize(data.subspan(offset));
			if (!expected_element)
				return Unexpected(expected_element.error());

			const ByteSize element_size = Serializable<ElementT>::Size(expected_element.value());
			if constexpr (Type::Array<T>) {
				container[static_cast<std::size_t>(i)] = std::move(expected_element.value());
			} else if constexpr (requires(T& target, ElementT value) { target.push_back(std::move(value)); }) {
				container.push_back(std::move(expected_element.value()));
			} else if constexpr (requires(T& target, ElementT value) {
				target.insert_or_assign(value.first, value.second);
			}) {
				container.insert_or_assign(expected_element.value().first, expected_element.value().second);
			} else {
				container.insert(container.end(), std::move(expected_element.value()));
			}
			offset += static_cast<std::size_t>(element_size);
		}
		return container;
	}

	template<typename T>
	template<typename U>
	Expected<T, DeserializeError> Serializable<T>::DeserializePair(std::span<const std::byte> data) noexcept
	requires Type::Pair<U> {
		using FirstT = std::remove_cvref_t<typename T::first_type>;
		using SecondT = std::remove_cvref_t<typename T::second_type>;

		auto expected_first = Serializable<FirstT>::Deserialize(data);
		if (!expected_first)
			return Unexpected(expected_first.error());

		const ByteSize first_size = Serializable<FirstT>::Size(expected_first.value());
		if (static_cast<std::size_t>(first_size) > data.size())
			return Unexpected<DeserializeError>("Insufficient data for pair second");

		auto expected_second = Serializable<SecondT>::Deserialize(data.subspan(static_cast<std::size_t>(first_size)));
		if (!expected_second)
			return Unexpected(expected_second.error());

		return T{ std::move(expected_first.value()), std::move(expected_second.value()) };
	}

	template<typename T>
	template<typename U>
	Expected<T, DeserializeError> Serializable<T>::DeserializeOptional(std::span<const std::byte> data) noexcept
	requires Type::Optional<U> {
		auto expected_has = Serializable<bool>::Deserialize(data);
		if (!expected_has)
			return Unexpected(expected_has.error());

		if (!expected_has.value())
			return T{};

		const ByteSize flag_size = Serializable<bool>::Size(true);
		if (static_cast<std::size_t>(flag_size) > data.size())
			return Unexpected<DeserializeError>("Insufficient data for optional value");

		using ValueT = std::remove_cvref_t<typename T::value_type>;
		auto expected_value = Serializable<ValueT>::Deserialize(data.subspan(static_cast<std::size_t>(flag_size)));
		if (!expected_value)
			return Unexpected(expected_value.error());

		return T{ std::move(expected_value.value()) };
	}

	template<typename T>
	template<typename U>
	Expected<T, DeserializeError> Serializable<T>::DeserializeQueue(std::span<const std::byte> data) noexcept
	requires Type::Queue<U> {
		if (data.size() < sizeof(std::uint64_t))
			return Unexpected<DeserializeError>("Insufficient data for queue size");
		auto expected_count = Serializable<std::uint64_t>::Deserialize(data.first(sizeof(std::uint64_t)));
		if (!expected_count)
			return Unexpected(expected_count.error());

		std::size_t offset = sizeof(std::uint64_t);
		const std::uint64_t count = expected_count.value();
		using ElementT = typename DecayedT::value_type;
		constexpr std::uint64_t MaximumQueueElements = 1'048'576;
		if (count > MaximumQueueElements)
			return Unexpected<DeserializeError>("Queue element count exceeds the decoding resource limit");

		try {
			const auto readElement = [&]() -> Expected<ElementT, DeserializeError> {
				if (offset > data.size())
					return Unexpected<DeserializeError>("Queue element offset exceeds input");
				auto expected_element = Serializable<ElementT>::Deserialize(data.subspan(offset));
				if (!expected_element)
					return Unexpected(expected_element.error());
				const ByteSize element_size = Serializable<ElementT>::Size(expected_element.value());
				if (static_cast<std::size_t>(element_size) > data.size() - offset)
					return Unexpected<DeserializeError>("Queue element exceeds remaining buffer");
				offset += static_cast<std::size_t>(element_size);
				return std::move(expected_element.value());
			};

			if constexpr (Type::IsSafeQueue<DecayedT>::value) {
				std::queue<ElementT> staged;
				for (std::uint64_t i = 0; i < count; ++i) {
					auto element = readElement();
					if (!element)
						return Unexpected(element.error());
					staged.push(std::move(element.value()));
				}
				return T(std::move(staged));
			} else {
				T queue;
				for (std::uint64_t i = 0; i < count; ++i) {
					auto element = readElement();
					if (!element)
						return Unexpected(element.error());
					queue.push(std::move(element.value()));
				}
				return queue;
			}
		} catch (...) {
			return Unexpected<DeserializeError>("Queue allocation or insertion failed");
		}
	}
}
