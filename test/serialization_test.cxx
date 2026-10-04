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

#include <StormByte/binary_data.hxx>
#include <StormByte/byte_size.hxx>
#include <StormByte/helpers.hxx>
#include <StormByte/serializable.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <vector>

using namespace StormByte;

namespace {
	std::size_t ByteCount(const BinaryData& buf) {
		return static_cast<std::size_t>(buf.size());
	}

	void CorruptByte(BinaryData& buf, std::size_t index, std::byte value) {
		if (index < ByteCount(buf))
			buf[ByteSize{ static_cast<std::uint64_t>(index) }] = value;
	}

	void FlipBit(BinaryData& buf, std::size_t byte_index, unsigned bit) {
		if (byte_index >= ByteCount(buf) || bit > 7)
			return;
		auto& b = reinterpret_cast<unsigned char&>(buf[ByteSize{ static_cast<std::uint64_t>(byte_index) }]);
		b ^= static_cast<unsigned char>(1u << bit);
	}

	BinaryData Truncate(const BinaryData& buf, std::size_t new_size) {
		if (new_size >= ByteCount(buf))
			return buf;
		return BinaryData(buf.data(), ByteSize{ static_cast<std::uint64_t>(new_size) });
	}

	BinaryData MakeStringVectorBuffer() {
		std::vector<std::string> data = {"Hello", "StormByte", "World"};
		return Serializable<std::vector<std::string>>(data).Serialize();
	}

	BinaryData MakeStringBuffer() {
		return Serializable<std::string>("StormByte serialization test").Serialize();
	}

	BinaryData MakeBinaryPayload() {
		BinaryData data;
		data.push_back(std::byte{0});
		data.push_back(std::byte{1});
		data.push_back(std::byte{255});
		data.push_back(std::byte{128});
		return data;
	}

	BinaryData MakeBinaryDataBuffer() {
		return Serializable<BinaryData>(MakeBinaryPayload()).Serialize();
	}

	struct Tag {
		int a;
		std::string b;
		bool operator==(const Tag& other) const noexcept {
			return a == other.a && b == other.b;
		}
	};
}

template<>
struct StormByte::Detail::Codec<Tag> {
	static std::size_t Size(const Tag& v) noexcept {
		return Serializable<int>::Size(v.a) + Serializable<std::string>::Size(v.b);
	}

	static BinaryData Write(const Tag& v) noexcept {
		BinaryData buf;
		append_bytes(buf, Serializable<int>(v.a).Serialize());
		append_bytes(buf, Serializable<std::string>(v.b).Serialize());
		return buf;
	}

	static Expected<Tag, DeserializeError> Read(std::span<const std::byte> data) noexcept {
		auto expected_a = Serializable<int>::Deserialize(data);
		if (!expected_a)
			return Unexpected(expected_a.error());
		const std::size_t a_size = Serializable<int>::Size(expected_a.value());
		if (a_size > data.size())
			return Unexpected<DeserializeError>("Insufficient data for Tag::b");
		auto expected_b = Serializable<std::string>::Deserialize(data.subspan(a_size));
		if (!expected_b)
			return Unexpected(expected_b.error());
		return Tag{ expected_a.value(), expected_b.value() };
	}
};

// -------------------
// BinaryData
// -------------------

int test_serialize_binary_data() {
	const BinaryData data = MakeBinaryPayload();
	auto buffer = Serializable<BinaryData>(data).Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_binary_data", 1);
	auto expected = Serializable<BinaryData>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_binary_data", 1);
	}
	ASSERT_TRUE("test_serialize_binary_data", data == expected.value());
	ASSERT_EQUAL("test_serialize_binary_data", Serializable<BinaryData>::Size(data), ByteCount(buffer));
	RETURN_TEST("test_serialize_binary_data", 0);
}

int test_serialize_binary_data_empty() {
	const BinaryData data;
	auto buffer = Serializable<BinaryData>(data).Serialize();
	auto expected = Serializable<BinaryData>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_binary_data_empty", 1);
	}
	ASSERT_TRUE("test_serialize_binary_data_empty", expected.value().empty());
	RETURN_TEST("test_serialize_binary_data_empty", 0);
}

int test_serialize_binary_data_shares_vector_byte_wire() {
	const BinaryData owned = MakeBinaryPayload();
	std::vector<std::byte> as_vector;
	as_vector.assign(owned.begin(), owned.end());
	ASSERT_TRUE("test_serialize_binary_data_shares_vector_byte_wire",
		Serializable<BinaryData>(owned).Serialize() == Serializable<std::vector<std::byte>>(as_vector).Serialize());
	RETURN_TEST("test_serialize_binary_data_shares_vector_byte_wire", 0);
}

int test_serialize_binary_data_trailing_garbage() {
	auto clean = MakeBinaryDataBuffer();
	auto dirty = clean;
	dirty.push_back(std::byte{0xDE});
	dirty.push_back(std::byte{0xAD});
	auto result = Serializable<BinaryData>::Deserialize(dirty);
	if (!result) {
		std::cerr << "test_serialize_binary_data_trailing_garbage: trailing bytes should be ignored by Read\n";
		RETURN_TEST("test_serialize_binary_data_trailing_garbage", 1);
	}
	ASSERT_TRUE("test_serialize_binary_data_trailing_garbage", result.value() == MakeBinaryPayload());
	RETURN_TEST("test_serialize_binary_data_trailing_garbage", 0);
}

int test_serialize_binary_data_truncated() {
	auto clean = MakeBinaryDataBuffer();
	for (std::size_t len = 0; len < ByteCount(clean); ++len) {
		auto truncated = Truncate(clean, len);
		auto result = Serializable<BinaryData>::Deserialize(truncated);
		if (result) {
			std::cerr << "test_serialize_binary_data_truncated: size " << len << " accepted\n";
			RETURN_TEST("test_serialize_binary_data_truncated", 1);
		}
	}
	RETURN_TEST("test_serialize_binary_data_truncated", 0);
}

int test_serialize_optional_binary_data() {
	std::optional<BinaryData> empty;
	auto empty_buf = Serializable<std::optional<BinaryData>>(empty).Serialize();
	auto empty_got = Serializable<std::optional<BinaryData>>::Deserialize(empty_buf);
	if (!empty_got) {
		std::cerr << empty_got.error()->what() << std::endl;
		RETURN_TEST("test_serialize_optional_binary_data", 1);
	}
	ASSERT_FALSE("test_serialize_optional_binary_data", empty_got.value().has_value());

	std::optional<BinaryData> filled = MakeBinaryPayload();
	auto filled_buf = Serializable<std::optional<BinaryData>>(filled).Serialize();
	auto filled_got = Serializable<std::optional<BinaryData>>::Deserialize(filled_buf);
	if (!filled_got) {
		std::cerr << filled_got.error()->what() << std::endl;
		RETURN_TEST("test_serialize_optional_binary_data", 1);
	}
	ASSERT_TRUE("test_serialize_optional_binary_data", filled_got.value().has_value());
	ASSERT_TRUE("test_serialize_optional_binary_data", filled_got.value().value() == filled.value());
	RETURN_TEST("test_serialize_optional_binary_data", 0);
}

int test_binary_data_corruption_huge_size() {
	auto clean = MakeBinaryDataBuffer();
	int accepted = 0;
	if (ByteCount(clean) >= sizeof(std::uint64_t)) {
		auto buf = clean;
		std::uint64_t huge = static_cast<std::uint64_t>(-1);
		std::memcpy(buf.data(), &huge, sizeof(huge));
		auto result = Serializable<BinaryData>::Deserialize(buf);
		if (result)
			++accepted;
	}
	if (accepted > 0) {
		std::cerr << "test_binary_data_corruption_huge_size: huge size field was accepted\n";
		RETURN_TEST("test_binary_data_corruption_huge_size", 1);
	}
	RETURN_TEST("test_binary_data_corruption_huge_size", 0);
}

int test_binary_data_corruption_no_crash_bit_flip() {
	auto clean = MakeBinaryDataBuffer();
	for (std::size_t i = 0; i < ByteCount(clean); ++i) {
		for (unsigned bit = 0; bit < 8; ++bit) {
			auto buf = clean;
			FlipBit(buf, i, bit);
			auto result = Serializable<BinaryData>::Deserialize(buf);
			(void)result;
		}
	}
	RETURN_TEST("test_binary_data_corruption_no_crash_bit_flip", 0);
}

int test_binary_data_corruption_no_crash_byte_overwrite() {
	auto clean = MakeBinaryDataBuffer();
	for (std::size_t i = 0; i < ByteCount(clean); ++i) {
		for (int v = 0; v < 256; v += 31) {
			auto buf = clean;
			CorruptByte(buf, i, static_cast<std::byte>(v));
			auto result = Serializable<BinaryData>::Deserialize(buf);
			(void)result;
		}
	}
	RETURN_TEST("test_binary_data_corruption_no_crash_byte_overwrite", 0);
}

int test_binary_data_corruption_random_stress() {
	auto clean = MakeBinaryDataBuffer();
	std::mt19937 rng(0xB10A);
	std::uniform_int_distribution<std::size_t> pos_dist(0, ByteCount(clean) - 1);
	std::uniform_int_distribution<int> val_dist(0, 255);
	constexpr int ITERATIONS = 400;
	for (int i = 0; i < ITERATIONS; ++i) {
		auto buf = clean;
		int count = 1 + (i % 4);
		for (int c = 0; c < count; ++c)
			CorruptByte(buf, pos_dist(rng), static_cast<std::byte>(val_dist(rng)));
		auto result = Serializable<BinaryData>::Deserialize(buf);
		(void)result;
	}
	RETURN_TEST("test_binary_data_corruption_random_stress", 0);
}

int test_serialize_binary_data_from_blob_and_span() {
	const BinaryData data = MakeBinaryPayload();
	auto buffer = Serializable<BinaryData>(data).Serialize();
	auto from_blob = Serializable<BinaryData>::Deserialize(buffer);
	auto from_span = Serializable<BinaryData>::Deserialize(buffer.span());
	ASSERT_TRUE("test_serialize_binary_data_from_blob_and_span", static_cast<bool>(from_blob));
	ASSERT_TRUE("test_serialize_binary_data_from_blob_and_span", static_cast<bool>(from_span));
	ASSERT_TRUE("test_serialize_binary_data_from_blob_and_span", from_blob.value() == data);
	ASSERT_TRUE("test_serialize_binary_data_from_blob_and_span", from_span.value() == data);
	RETURN_TEST("test_serialize_binary_data_from_blob_and_span", 0);
}

// -------------------
// Codec
// -------------------

int test_codec_custom_type() {
	const Tag original{ 7, "codec" };
	auto buffer = Serializable<Tag>(original).Serialize();
	if (buffer.empty())
		RETURN_TEST("test_codec_custom_type", 1);
	auto expected = Serializable<Tag>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_codec_custom_type", 1);
	}
	ASSERT_TRUE("test_codec_custom_type", original == expected.value());
	ASSERT_EQUAL("test_codec_custom_type", Serializable<Tag>::Size(original), ByteCount(buffer));
	RETURN_TEST("test_codec_custom_type", 0);
}

int test_codec_custom_type_truncated() {
	const Tag original{ 7, "codec" };
	auto buffer = Serializable<Tag>(original).Serialize();
	for (std::size_t len = 0; len < ByteCount(buffer); ++len) {
		auto truncated = Truncate(buffer, len);
		auto result = Serializable<Tag>::Deserialize(truncated);
		if (result) {
			std::cerr << "test_codec_custom_type_truncated: size " << len << " accepted\n";
			RETURN_TEST("test_codec_custom_type_truncated", 1);
		}
	}
	RETURN_TEST("test_codec_custom_type_truncated", 0);
}

// -------------------
// Corruption
// -------------------

int test_base_corruption_empty_buffer() {
	auto r1 = Serializable<int>::Deserialize(BinaryData{});
	auto r2 = Serializable<std::string>::Deserialize(BinaryData{});
	auto r3 = Serializable<std::vector<std::string>>::Deserialize(BinaryData{});
	auto r4 = Serializable<std::pair<int, double>>::Deserialize(BinaryData{});
	auto r5 = Serializable<BinaryData>::Deserialize(BinaryData{});
	if (r1 || r2 || r3 || r4 || r5) {
		std::cerr << "test_base_corruption_empty_buffer: empty buffer was accepted\n";
		RETURN_TEST("test_base_corruption_empty_buffer", 1);
	}
	RETURN_TEST("test_base_corruption_empty_buffer", 0);
}

int test_base_corruption_huge_container_size() {
	auto clean = MakeStringVectorBuffer();
	int accepted = 0;
	for (std::size_t i = 0; i < std::min<std::size_t>(16, ByteCount(clean)); ++i) {
		if (i + sizeof(std::uint64_t) > ByteCount(clean)) break;
		auto buf = clean;
		std::uint64_t huge = static_cast<std::uint64_t>(-1);
		std::memcpy(buf.data() + i, &huge, sizeof(huge));
		auto result = Serializable<std::vector<std::string>>::Deserialize(buf);
		if (result)
			++accepted;
	}
	if (accepted > 0) {
		std::cerr << "test_base_corruption_huge_container_size: " << accepted
			<< " buffers with huge size were accepted\n";
		RETURN_TEST("test_base_corruption_huge_container_size", 1);
	}
	RETURN_TEST("test_base_corruption_huge_container_size", 0);
}

int test_base_corruption_huge_string_size() {
	auto clean = MakeStringBuffer();
	int accepted = 0;
	if (ByteCount(clean) >= sizeof(std::uint64_t)) {
		auto buf = clean;
		std::uint64_t huge = static_cast<std::uint64_t>(-1);
		std::memcpy(buf.data(), &huge, sizeof(huge));
		auto result = Serializable<std::string>::Deserialize(buf);
		if (result)
			++accepted;
	}
	if (accepted > 0) {
		std::cerr << "test_base_corruption_huge_string_size: huge size field was accepted\n";
		RETURN_TEST("test_base_corruption_huge_string_size", 1);
	}
	RETURN_TEST("test_base_corruption_huge_string_size", 0);
}

int test_base_corruption_no_crash_bit_flip() {
	auto clean = MakeStringVectorBuffer();
	for (std::size_t i = 0; i < ByteCount(clean); ++i) {
		for (unsigned bit = 0; bit < 8; ++bit) {
			auto buf = clean;
			FlipBit(buf, i, bit);
			auto result = Serializable<std::vector<std::string>>::Deserialize(buf);
			(void)result;
		}
	}
	RETURN_TEST("test_base_corruption_no_crash_bit_flip", 0);
}

int test_base_corruption_no_crash_byte_overwrite() {
	auto clean = MakeStringBuffer();
	for (std::size_t i = 0; i < ByteCount(clean); ++i) {
		for (int v = 0; v < 256; v += 31) {
			auto buf = clean;
			CorruptByte(buf, i, static_cast<std::byte>(v));
			auto result = Serializable<std::string>::Deserialize(buf);
			(void)result;
		}
	}
	RETURN_TEST("test_base_corruption_no_crash_byte_overwrite", 0);
}

int test_base_corruption_random_stress() {
	auto clean = MakeStringVectorBuffer();
	std::mt19937 rng(0xC0FFEE);
	std::uniform_int_distribution<std::size_t> pos_dist(0, ByteCount(clean) - 1);
	std::uniform_int_distribution<int> val_dist(0, 255);
	constexpr int ITERATIONS = 400;
	for (int i = 0; i < ITERATIONS; ++i) {
		auto buf = clean;
		int count = 1 + (i % 4);
		for (int c = 0; c < count; ++c)
			CorruptByte(buf, pos_dist(rng), static_cast<std::byte>(val_dist(rng)));
		auto result = Serializable<std::vector<std::string>>::Deserialize(buf);
		(void)result;
	}
	RETURN_TEST("test_base_corruption_random_stress", 0);
}

int test_base_corruption_string_truncated_all() {
	auto clean = MakeStringBuffer();
	for (std::size_t len = 0; len < ByteCount(clean); ++len) {
		auto truncated = Truncate(clean, len);
		auto result = Serializable<std::string>::Deserialize(truncated);
		if (result) {
			std::cerr << "test_base_corruption_string_truncated_all: truncated size "
				<< len << " was accepted\n";
			RETURN_TEST("test_base_corruption_string_truncated_all", 1);
		}
	}
	RETURN_TEST("test_base_corruption_string_truncated_all", 0);
}

int test_base_corruption_vector_truncated_all() {
	auto clean = MakeStringVectorBuffer();
	for (std::size_t len = 0; len < ByteCount(clean); ++len) {
		auto truncated = Truncate(clean, len);
		auto result = Serializable<std::vector<std::string>>::Deserialize(truncated);
		if (result) {
			std::cerr << "test_base_corruption_vector_truncated_all: truncated size "
				<< len << " was accepted\n";
			RETURN_TEST("test_base_corruption_vector_truncated_all", 1);
		}
	}
	RETURN_TEST("test_base_corruption_vector_truncated_all", 0);
}

int test_base_cross_type_vector_as_string() {
	auto vec_buf = MakeStringVectorBuffer();
	auto as_string = Serializable<std::string>::Deserialize(vec_buf);
	(void)as_string;
	RETURN_TEST("test_base_cross_type_vector_as_string", 0);
}

int test_base_double_corruption() {
	auto clean = MakeStringVectorBuffer();
	if (ByteCount(clean) < 8)
		RETURN_TEST("test_base_double_corruption", 0);
	auto buf = clean;
	for (std::size_t i = 0; i < 4; ++i)
		buf[ByteSize{ static_cast<std::uint64_t>(i) }] = std::byte{0xFF};
	for (std::size_t i = 0; i < 4; ++i)
		buf[ByteSize{ static_cast<std::uint64_t>(ByteCount(buf) - 1 - i) }] = std::byte{0xAA};
	auto result = Serializable<std::vector<std::string>>::Deserialize(buf);
	(void)result;
	RETURN_TEST("test_base_double_corruption", 0);
}

// -------------------
// Nested
// -------------------

int test_base_idempotent_roundtrip_pair() {
	std::pair<int, std::string> original{42, "answer"};
	auto buf1 = Serializable<std::pair<int, std::string>>(original).Serialize();
	auto d1 = Serializable<std::pair<int, std::string>>::Deserialize(buf1);
	if (!d1) {
		std::cerr << d1.error()->what() << std::endl;
		RETURN_TEST("test_base_idempotent_roundtrip_pair", 1);
	}
	auto buf2 = Serializable<std::pair<int, std::string>>(d1.value()).Serialize();
	ASSERT_TRUE("test_base_idempotent_roundtrip_pair", original == d1.value());
	ASSERT_TRUE("test_base_idempotent_roundtrip_pair", buf1 == buf2);
	RETURN_TEST("test_base_idempotent_roundtrip_pair", 0);
}

int test_safe_pair_roundtrip() {
	using Pair = Safe::Pair<Safe::String, Safe::String>;
	const Pair original(Safe::String("key"), Safe::String("value"));
	const auto buffer = Serializable<Pair>(original).Serialize();
	const auto decoded = Serializable<Pair>::Deserialize(buffer);
	if (!decoded) {
		std::cerr << decoded.error()->what() << std::endl;
		RETURN_TEST("test_safe_pair_roundtrip", 1);
	}
	ASSERT_TRUE("test_safe_pair_roundtrip", decoded.value() == original);
	RETURN_TEST("test_safe_pair_roundtrip", 0);
}

int test_base_idempotent_roundtrip_vector() {
	std::vector<std::string> original = {"a", "b", "StormByte"};
	auto buf1 = Serializable<std::vector<std::string>>(original).Serialize();
	auto d1 = Serializable<std::vector<std::string>>::Deserialize(buf1);
	if (!d1) {
		std::cerr << d1.error()->what() << std::endl;
		RETURN_TEST("test_base_idempotent_roundtrip_vector", 1);
	}
	auto buf2 = Serializable<std::vector<std::string>>(d1.value()).Serialize();
	ASSERT_TRUE("test_base_idempotent_roundtrip_vector", original == d1.value());
	ASSERT_TRUE("test_base_idempotent_roundtrip_vector", buf1 == buf2);
	RETURN_TEST("test_base_idempotent_roundtrip_vector", 0);
}

int test_base_nested_vector_of_pairs() {
	std::vector<std::pair<int, std::string>> data = {
		{1, "one"}, {2, "two"}, {3, "three"}
	};
	auto buf = Serializable<std::vector<std::pair<int, std::string>>>(data).Serialize();
	auto expected = Serializable<std::vector<std::pair<int, std::string>>>::Deserialize(buf);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_base_nested_vector_of_pairs", 1);
	}
	ASSERT_TRUE("test_base_nested_vector_of_pairs", data == expected.value());
	RETURN_TEST("test_base_nested_vector_of_pairs", 0);
}

int test_serialize_deep_nested_vector() {
	using Value = std::vector<std::vector<std::vector<int>>>;
	const Value data = {
		{{1, 2}, {3}},
		{{4, 5, 6}}
	};
	auto buffer = Serializable<Value>(data).Serialize();
	auto expected = Serializable<Value>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_deep_nested_vector", 1);
	}
	ASSERT_TRUE("test_serialize_deep_nested_vector", data == expected.value());
	RETURN_TEST("test_serialize_deep_nested_vector", 0);
}

// -------------------
// Optional
// -------------------

int test_serialize_nested_optional() {
	using Value = std::optional<std::optional<int>>;
	const std::vector<Value> values = { std::nullopt, std::optional<int>{std::nullopt}, std::optional<int>{42} };
	for (const auto& data : values) {
		auto buffer = Serializable<Value>(data).Serialize();
		auto expected = Serializable<Value>::Deserialize(buffer);
		if (!expected || expected.value() != data) {
			std::cerr << "test_serialize_nested_optional: nested optional mismatch\n";
			RETURN_TEST("test_serialize_nested_optional", 1);
		}
	}
	RETURN_TEST("test_serialize_nested_optional", 0);
}

int test_serialize_optional_empty() {
	std::optional<int> data;
	Serializable<std::optional<int>> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_optional_empty", 1);
	auto expected_data = Serializable<std::optional<int>>::Deserialize(buffer);
	if (!expected_data) {
		std::cerr << expected_data.error()->what() << std::endl;
		RETURN_TEST("test_serialize_optional_empty", 1);
	}
	ASSERT_FALSE("test_serialize_optional_empty", expected_data.value().has_value());
	RETURN_TEST("test_serialize_optional_empty", 0);
}

int test_serialize_optional_notempty() {
	std::optional<int> data = 42;
	Serializable<std::optional<int>> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_optional_notempty", 1);
	auto expected_data = Serializable<std::optional<int>>::Deserialize(buffer);
	if (!expected_data) {
		std::cerr << expected_data.error()->what() << std::endl;
		RETURN_TEST("test_serialize_optional_notempty", 1);
	}
	ASSERT_EQUAL("test_serialize_optional_notempty", data.value(), expected_data.value().value());
	RETURN_TEST("test_serialize_optional_notempty", 0);
}

int test_serialize_optional_string() {
	std::optional<std::string> data = "Hello, World!";
	Serializable<std::optional<std::string>> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_optional_string", 1);
	auto expected_data = Serializable<std::optional<std::string>>::Deserialize(buffer);
	if (!expected_data) {
		std::cerr << expected_data.error()->what() << std::endl;
		RETURN_TEST("test_serialize_optional_string", 1);
	}
	ASSERT_EQUAL("test_serialize_optional_string", data.value(), expected_data.value().value());
	RETURN_TEST("test_serialize_optional_string", 0);
}

// -------------------
// Roundtrip
// -------------------

int test_serialize_array() {
	const std::array<int, 3> data = { 10, 20, 30 };
	auto buffer = Serializable<std::array<int, 3>>(data).Serialize();
	auto expected = Serializable<std::array<int, 3>>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_array", 1);
	}
	ASSERT_TRUE("test_serialize_array", data == expected.value());
	const auto expected_size = Serializable<std::array<int, 3>>::Size(data);
	ASSERT_EQUAL("test_serialize_array", expected_size, ByteCount(buffer));
	RETURN_TEST("test_serialize_array", 0);
}

int test_serialize_array_rejects_wrong_element_count() {
	const std::vector<int> data = { 10, 20 };
	auto buffer = Serializable<std::vector<int>>(data).Serialize();
	auto expected = Serializable<std::array<int, 3>>::Deserialize(buffer);
	ASSERT_FALSE("test_serialize_array_rejects_wrong_element_count", expected.has_value());
	RETURN_TEST("test_serialize_array_rejects_wrong_element_count", 0);
}

int test_serialize_deserialize_big_string() {
	const std::string fn_name = "test_serialize_deserialize_big_string";
	const std::string data(10 * 1024 * 1024, 'A');
	Serializable<std::string> serialization(data);
	BinaryData buffer = serialization.Serialize();
	ASSERT_FALSE(fn_name, buffer.empty());
	auto expected_data = Serializable<std::string>::Deserialize(buffer);
	if (!expected_data) {
		std::cerr << expected_data.error()->what() << std::endl;
		RETURN_TEST(fn_name.c_str(), 1);
	}
	ASSERT_EQUAL(fn_name, data, expected_data.value());
	RETURN_TEST(fn_name.c_str(), 0);
}

int test_serialize_deserialize_with_span() {
	const std::string fn_name = "test_serialize_deserialize_with_span";
	int data = 123456;
	Serializable<int> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST(fn_name.c_str(), 1);
	auto expected_data = Serializable<int>::Deserialize(buffer.span());
	if (!expected_data) {
		std::cerr << expected_data.error()->what() << std::endl;
		RETURN_TEST(fn_name.c_str(), 1);
	}
	ASSERT_EQUAL(fn_name.c_str(), data, expected_data.value());
	RETURN_TEST(fn_name.c_str(), 0);
}

int test_serialize_double() {
	double data = 777.777;
	Serializable<double> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_double", 1);
	auto expected_data = Serializable<double>::Deserialize(buffer);
	if (!expected_data)
		RETURN_TEST("test_serialize_double", 1);
	ASSERT_EQUAL("test_serialize_double", data, expected_data.value());
	RETURN_TEST("test_serialize_double", 0);
}

int test_serialize_int() {
	int data = 42;
	Serializable<int> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_int", 1);
	auto expected_data = Serializable<int>::Deserialize(buffer);
	if (!expected_data)
		RETURN_TEST("test_serialize_int", 1);
	ASSERT_EQUAL("test_serialize_int", data, expected_data.value());
	RETURN_TEST("test_serialize_int", 0);
}

int test_serialize_map() {
	std::map<int, std::string> data = { { 1, "Hello" }, { 2, "World!" } };
	Serializable<std::map<int, std::string>> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_map", 1);
	auto expected_data = Serializable<std::map<int, std::string>>::Deserialize(buffer);
	if (!expected_data) {
		std::cerr << expected_data.error()->what() << std::endl;
		RETURN_TEST("test_serialize_map", 1);
	}
	ASSERT_TRUE("test_serialize_map", data == expected_data.value());
	RETURN_TEST("test_serialize_map", 0);
}

int test_serialize_pair() {
	std::pair<int, double> data = { 42, 777.777 };
	Serializable<std::pair<int, double>> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_pair", 1);
	auto expected_data = Serializable<std::pair<int, double>>::Deserialize(buffer);
	if (!expected_data) {
		std::cerr << expected_data.error()->what() << std::endl;
		RETURN_TEST("test_serialize_pair", 1);
	}
	ASSERT_TRUE("test_serialize_pair", data == expected_data.value());
	RETURN_TEST("test_serialize_pair", 0);
}

int test_serialize_size_t() {
	std::string data = "Hello, World!";
	std::size_t size = data.size();
	Serializable<std::size_t> serialization(size);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_size_t", 1);
	auto expected_data = Serializable<std::size_t>::Deserialize(buffer);
	if (!expected_data)
		RETURN_TEST("test_serialize_size_t", 1);
	ASSERT_EQUAL("test_serialize_size_t", data.size(), expected_data.value());
	RETURN_TEST("test_serialize_size_t", 0);
}

int test_serialize_string() {
	std::string data = "Hello, World!";
	Serializable<std::string> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_string", 1);
	auto expected_data = Serializable<std::string>::Deserialize(buffer);
	if (!expected_data)
		RETURN_TEST("test_serialize_string", 1);
	ASSERT_EQUAL("test_serialize_string", data, expected_data.value());
	RETURN_TEST("test_serialize_string", 0);
}

int test_serialize_string_vector() {
	std::vector<std::string> data = { "Hello", "World!" };
	Serializable<std::vector<std::string>> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_string_vector", 1);
	auto expected_data = Serializable<std::vector<std::string>>::Deserialize(buffer);
	if (!expected_data) {
		std::cerr << expected_data.error()->what() << std::endl;
		RETURN_TEST("test_serialize_string_vector", 1);
	}
	ASSERT_TRUE("test_serialize_string_vector", data == expected_data.value());
	RETURN_TEST("test_serialize_string_vector", 0);
}

// -------------------
// Truncated
// -------------------

int test_serialize_deserialize_with_span_truncated() {
	const std::string fn_name = "test_serialize_deserialize_with_span_truncated";
	int data = 123456;
	Serializable<int> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST(fn_name.c_str(), 1);
	std::size_t truncated_len = sizeof(int) / 2;
	std::span<const std::byte> truncated_span(buffer.data(), truncated_len);
	auto expected_data = Serializable<int>::Deserialize(truncated_span);
	if (expected_data) {
		std::cerr << "Expected failure, but got value: " << expected_data.value() << std::endl;
		RETURN_TEST(fn_name.c_str(), 1);
	}
	RETURN_TEST(fn_name.c_str(), 0);
}

int test_serialize_int_truncated() {
	int data = 42;
	Serializable<int> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_int_truncated", 1);
	auto truncated_buffer = Truncate(buffer, sizeof(int) / 2);
	auto expected_data = Serializable<int>::Deserialize(truncated_buffer);
	if (expected_data) {
		std::cerr << "Expected failure, but got value: " << expected_data.value() << std::endl;
		RETURN_TEST("test_serialize_int_truncated", 1);
	}
	RETURN_TEST("test_serialize_int_truncated", 0);
}

int test_serialize_pair_truncated() {
	std::pair<int, double> data = { 42, 777.777 };
	Serializable<std::pair<int, double>> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_pair_truncated", 1);
	auto truncated_buffer = Truncate(buffer, sizeof(int));
	auto expected_data = Serializable<std::pair<int, double>>::Deserialize(truncated_buffer);
	if (expected_data) {
		std::cerr << "Expected failure, but got value" << std::endl;
		RETURN_TEST("test_serialize_pair_truncated", 1);
	}
	RETURN_TEST("test_serialize_pair_truncated", 0);
}

int test_serialize_string_vector_truncated() {
	std::vector<std::string> data = { "Hello", "World!" };
	Serializable<std::vector<std::string>> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_string_vector_truncated", 1);
	auto truncated_buffer = Truncate(buffer, sizeof(std::size_t) + 2);
	auto expected_data = Serializable<std::vector<std::string>>::Deserialize(truncated_buffer);
	if (expected_data) {
		std::cerr << "Expected failure, but got value" << std::endl;
		RETURN_TEST("test_serialize_string_vector_truncated", 1);
	}
	RETURN_TEST("test_serialize_string_vector_truncated", 0);
}

// -------------------
// Unicode
// -------------------

int test_cstring_shares_string_wire() {
	const Safe::CString owned("StormByte");
	const std::string text = "StormByte";
	ASSERT_TRUE("test_cstring_shares_string_wire",
		Serializable<Safe::CString>(owned).Serialize() == Serializable<std::string>(text).Serialize());
	RETURN_TEST("test_cstring_shares_string_wire", 0);
}

int test_serialize_cstring() {
	const Safe::CString data("Hello, StormByte!");
	auto buffer = Serializable<Safe::CString>(data).Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_cstring", 1);
	auto expected = Serializable<Safe::CString>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_cstring", 1);
	}
	ASSERT_TRUE("test_serialize_cstring", data == expected.value());
	ASSERT_EQUAL("test_serialize_cstring", Serializable<Safe::CString>::Size(data), ByteCount(buffer));
	RETURN_TEST("test_serialize_cstring", 0);
}

int test_serialize_cstring_empty() {
	const Safe::CString data("");
	auto buffer = Serializable<Safe::CString>(data).Serialize();
	auto expected = Serializable<Safe::CString>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_cstring_empty", 1);
	}
	ASSERT_TRUE("test_serialize_cstring_empty", expected.value() == "");
	ASSERT_TRUE("test_serialize_cstring_empty", static_cast<bool>(expected.value()));
	RETURN_TEST("test_serialize_cstring_empty", 0);
}

int test_serialize_cstring_null_is_empty_wire() {
	const Safe::CString missing;
	const Safe::CString empty("");
	auto a = Serializable<Safe::CString>(missing).Serialize();
	auto b = Serializable<Safe::CString>(empty).Serialize();
	auto c = Serializable<std::string>(std::string()).Serialize();
	ASSERT_TRUE("test_serialize_cstring_null_is_empty_wire", a == b);
	ASSERT_TRUE("test_serialize_cstring_null_is_empty_wire", a == c);
	auto expected = Serializable<Safe::CString>::Deserialize(a);
	if (!expected)
		RETURN_TEST("test_serialize_cstring_null_is_empty_wire", 1);
	ASSERT_TRUE("test_serialize_cstring_null_is_empty_wire", expected.value() == "");
	RETURN_TEST("test_serialize_cstring_null_is_empty_wire", 0);
}

int test_serialize_cstring_truncated() {
	auto buffer = Serializable<Safe::CString>(Safe::CString("TruncationTest")).Serialize();
	for (std::size_t len = 0; len < ByteCount(buffer); ++len) {
		auto truncated = Truncate(buffer, len);
		auto result = Serializable<Safe::CString>::Deserialize(truncated);
		if (result) {
			std::cerr << "test_serialize_cstring_truncated: size " << len << " accepted\n";
			RETURN_TEST("test_serialize_cstring_truncated", 1);
		}
	}
	RETURN_TEST("test_serialize_cstring_truncated", 0);
}

int test_serialize_u16string() {
	std::u16string data = u"Hello, StormByte!";
	Serializable<std::u16string> serialization(data);
	BinaryData buffer = serialization.Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_u16string", 1);
	auto expected = Serializable<std::u16string>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_u16string", 1);
	}
	ASSERT_TRUE("test_serialize_u16string", data == expected.value());
	RETURN_TEST("test_serialize_u16string", 0);
}

int test_serialize_u16string_empty() {
	std::u16string data;
	auto buffer = Serializable<std::u16string>(data).Serialize();
	auto expected = Serializable<std::u16string>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_u16string_empty", 1);
	}
	ASSERT_TRUE("test_serialize_u16string_empty", expected.value().empty());
	RETURN_TEST("test_serialize_u16string_empty", 0);
}

int test_serialize_u16string_huge_size() {
	auto clean = Serializable<std::u16string>(u"safe").Serialize();
	if (ByteCount(clean) < sizeof(std::uint64_t))
		RETURN_TEST("test_serialize_u16string_huge_size", 1);
	auto buf = clean;
	std::uint64_t huge = static_cast<std::uint64_t>(-1);
	std::memcpy(buf.data(), &huge, sizeof(huge));
	auto result = Serializable<std::u16string>::Deserialize(buf);
	if (result) {
		std::cerr << "test_serialize_u16string_huge_size: huge size was accepted\n";
		RETURN_TEST("test_serialize_u16string_huge_size", 1);
	}
	RETURN_TEST("test_serialize_u16string_huge_size", 0);
}

int test_serialize_u16string_non_bmp() {
	std::u16string data = u"\U0001F4A9";
	auto buffer = Serializable<std::u16string>(data).Serialize();
	auto expected = Serializable<std::u16string>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_u16string_non_bmp", 1);
	}
	ASSERT_TRUE("test_serialize_u16string_non_bmp", data == expected.value());
	RETURN_TEST("test_serialize_u16string_non_bmp", 0);
}

int test_serialize_u16string_truncated() {
	std::u16string data = u"TruncationTest";
	auto buffer = Serializable<std::u16string>(data).Serialize();
	for (std::size_t len = 0; len < ByteCount(buffer); ++len) {
		auto truncated = Truncate(buffer, len);
		auto result = Serializable<std::u16string>::Deserialize(truncated);
		if (result) {
			std::cerr << "test_serialize_u16string_truncated: size " << len << " accepted\n";
			RETURN_TEST("test_serialize_u16string_truncated", 1);
		}
	}
	RETURN_TEST("test_serialize_u16string_truncated", 0);
}

int test_serialize_unicode_boundary_codepoints() {
	const std::u32string data = { U'\0', U'\x7F', U'\x80', U'\x7FF', U'\x800', U'\xFFFF', U'\U00010000', U'\U0010FFFF' };
	auto buffer = Serializable<std::u32string>(data).Serialize();
	auto expected = Serializable<std::u32string>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_unicode_boundary_codepoints", 1);
	}
	ASSERT_TRUE("test_serialize_unicode_boundary_codepoints", data == expected.value());
	RETURN_TEST("test_serialize_unicode_boundary_codepoints", 0);
}

int test_serialize_wcstring() {
	const Safe::WCString data(L"Hello, StormByte!");
	auto buffer = Serializable<Safe::WCString>(data).Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_wcstring", 1);
	auto expected = Serializable<Safe::WCString>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_wcstring", 1);
	}
	ASSERT_TRUE("test_serialize_wcstring", data == expected.value());
	ASSERT_EQUAL("test_serialize_wcstring", Serializable<Safe::WCString>::Size(data), ByteCount(buffer));
	RETURN_TEST("test_serialize_wcstring", 0);
}

int test_serialize_wcstring_empty() {
	const Safe::WCString data(L"");
	auto buffer = Serializable<Safe::WCString>(data).Serialize();
	auto expected = Serializable<Safe::WCString>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_wcstring_empty", 1);
	}
	ASSERT_TRUE("test_serialize_wcstring_empty", expected.value() == L"");
	RETURN_TEST("test_serialize_wcstring_empty", 0);
}

int test_serialize_wcstring_non_bmp() {
	const Safe::WCString data(L"\U0001F4A9");
	auto buffer = Serializable<Safe::WCString>(data).Serialize();
	auto expected = Serializable<Safe::WCString>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_wcstring_non_bmp", 1);
	}
	ASSERT_TRUE("test_serialize_wcstring_non_bmp", data == expected.value());
	RETURN_TEST("test_serialize_wcstring_non_bmp", 0);
}

int test_serialize_wcstring_null_is_empty_wire() {
	const Safe::WCString missing;
	const Safe::WCString empty(L"");
	auto a = Serializable<Safe::WCString>(missing).Serialize();
	auto b = Serializable<Safe::WCString>(empty).Serialize();
	auto c = Serializable<std::wstring>(std::wstring()).Serialize();
	ASSERT_TRUE("test_serialize_wcstring_null_is_empty_wire", a == b);
	ASSERT_TRUE("test_serialize_wcstring_null_is_empty_wire", a == c);
	auto expected = Serializable<Safe::WCString>::Deserialize(a);
	if (!expected)
		RETURN_TEST("test_serialize_wcstring_null_is_empty_wire", 1);
	ASSERT_TRUE("test_serialize_wcstring_null_is_empty_wire", expected.value() == L"");
	RETURN_TEST("test_serialize_wcstring_null_is_empty_wire", 0);
}

int test_serialize_wcstring_truncated() {
	auto buffer = Serializable<Safe::WCString>(Safe::WCString(L"TruncationTest")).Serialize();
	for (std::size_t len = 0; len < ByteCount(buffer); ++len) {
		auto truncated = Truncate(buffer, len);
		auto result = Serializable<Safe::WCString>::Deserialize(truncated);
		if (result) {
			std::cerr << "test_serialize_wcstring_truncated: size " << len << " accepted\n";
			RETURN_TEST("test_serialize_wcstring_truncated", 1);
		}
	}
	RETURN_TEST("test_serialize_wcstring_truncated", 0);
}

int test_serialize_wstring() {
	std::wstring data = L"Hello, StormByte!";
	auto buffer = Serializable<std::wstring>(data).Serialize();
	if (buffer.empty())
		RETURN_TEST("test_serialize_wstring", 1);
	auto expected = Serializable<std::wstring>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_wstring", 1);
	}
	ASSERT_TRUE("test_serialize_wstring", data == expected.value());
	RETURN_TEST("test_serialize_wstring", 0);
}

int test_serialize_wstring_non_bmp() {
	std::wstring data = L"\U0001F4A9";
	auto buffer = Serializable<std::wstring>(data).Serialize();
	auto expected = Serializable<std::wstring>::Deserialize(buffer);
	if (!expected) {
		std::cerr << expected.error()->what() << std::endl;
		RETURN_TEST("test_serialize_wstring_non_bmp", 1);
	}
	ASSERT_TRUE("test_serialize_wstring_non_bmp", data == expected.value());
	RETURN_TEST("test_serialize_wstring_non_bmp", 0);
}

int test_wcstring_shares_wstring_wire() {
	const Safe::WCString owned(L"StormByte");
	const std::wstring text = L"StormByte";
	ASSERT_TRUE("test_wcstring_shares_wstring_wire",
		Serializable<Safe::WCString>(owned).Serialize() == Serializable<std::wstring>(text).Serialize());
	RETURN_TEST("test_wcstring_shares_wstring_wire", 0);
}

int test_safe_strings_share_standard_wire() {
	int result = 0;
	const Safe::String narrow("Hello, StormByte!");
	const Safe::WString wide(L"ca\u00f1\u00f3n");
	const auto narrow_wire = Serializable<Safe::String>(narrow).Serialize();
	const auto wide_wire = Serializable<Safe::WString>(wide).Serialize();
	ASSERT_TRUE("test_safe_strings_share_standard_wire", narrow_wire == Serializable<std::string>(std::string("Hello, StormByte!")).Serialize());
	ASSERT_TRUE("test_safe_strings_share_standard_wire", wide_wire == Serializable<std::wstring>(std::wstring(L"ca\u00f1\u00f3n")).Serialize());
	const auto decoded_narrow = Serializable<Safe::String>::Deserialize(narrow_wire);
	const auto decoded_wide = Serializable<Safe::WString>::Deserialize(wide_wire);
	ASSERT_TRUE("test_safe_strings_share_standard_wire", decoded_narrow.has_value());
	ASSERT_TRUE("test_safe_strings_share_standard_wire", decoded_wide.has_value());
	if (decoded_narrow && decoded_wide) {
		ASSERT_TRUE("test_safe_strings_share_standard_wire", decoded_narrow.value() == narrow);
		ASSERT_TRUE("test_safe_strings_share_standard_wire", decoded_wide.value() == wide);
	}
	RETURN_TEST("test_safe_strings_share_standard_wire", result);
}

int test_wide_and_u16_share_utf8_wire() {
	const std::wstring wide = L"StormByte";
	const std::u16string u16 = u"StormByte";
	auto a = Serializable<std::wstring>(wide).Serialize();
	auto b = Serializable<std::u16string>(u16).Serialize();
	ASSERT_TRUE("test_wide_and_u16_share_utf8_wire", a == b);
	RETURN_TEST("test_wide_and_u16_share_utf8_wire", 0);
}

// -------------------
// Wire
// -------------------

int test_base_bool_accepts_0_and_1() {
	auto z = Serializable<bool>::Deserialize(BinaryData{ std::byte{0} });
	auto o = Serializable<bool>::Deserialize(BinaryData{ std::byte{1} });
	if (!z || z.value() != false) {
		std::cerr << "test_base_bool_accepts_0_and_1: 0 not decoded as false\n";
		RETURN_TEST("test_base_bool_accepts_0_and_1", 1);
	}
	if (!o || o.value() != true) {
		std::cerr << "test_base_bool_accepts_0_and_1: 1 not decoded as true\n";
		RETURN_TEST("test_base_bool_accepts_0_and_1", 1);
	}
	RETURN_TEST("test_base_bool_accepts_0_and_1", 0);
}

int test_base_bool_rejects_invalid_byte() {
	BinaryData buf{ std::byte{17} };
	bool threw = false;
	bool accepted = false;
	try {
		auto result = Serializable<bool>::Deserialize(buf);
		if (result)
			accepted = true;
	} catch (...) {
		threw = true;
	}
	if (threw) {
		std::cerr << "test_base_bool_rejects_invalid_byte: threw instead of returning error\n";
		RETURN_TEST("test_base_bool_rejects_invalid_byte", 1);
	}
	if (accepted) {
		std::cerr << "test_base_bool_rejects_invalid_byte: invalid byte 17 was accepted\n";
		RETURN_TEST("test_base_bool_rejects_invalid_byte", 1);
	}
	RETURN_TEST("test_base_bool_rejects_invalid_byte", 0);
}

int test_base_optional_flag_rejects_invalid_bool() {
	BinaryData buf{ std::byte{17} };
	bool threw = false;
	bool accepted = false;
	try {
		auto result = Serializable<std::optional<int>>::Deserialize(buf);
		if (result)
			accepted = true;
	} catch (...) {
		threw = true;
	}
	if (threw || accepted) {
		std::cerr << "test_base_optional_flag_rejects_invalid_bool: invalid flag not rejected cleanly\n";
		RETURN_TEST("test_base_optional_flag_rejects_invalid_bool", 1);
	}
	RETURN_TEST("test_base_optional_flag_rejects_invalid_bool", 0);
}

int test_base_trailing_garbage() {
	auto clean = MakeStringBuffer();
	auto dirty = clean;
	dirty.push_back(std::byte{0xDE});
	dirty.push_back(std::byte{0xAD});
	auto result = Serializable<std::string>::Deserialize(dirty);
	if (!result) {
		std::cerr << "test_base_trailing_garbage: trailing bytes should be ignored by Read\n";
		RETURN_TEST("test_base_trailing_garbage", 1);
	}
	ASSERT_EQUAL("test_base_trailing_garbage", std::string("StormByte serialization test"), result.value());
	RETURN_TEST("test_base_trailing_garbage", 0);
}

int test_wire_int_is_little_endian() {
	const int data = 0x01020304;
	auto buffer = Serializable<int>(data).Serialize();
	if (ByteCount(buffer) != sizeof(int)) {
		std::cerr << "test_wire_int_is_little_endian: unexpected size\n";
		RETURN_TEST("test_wire_int_is_little_endian", 1);
	}
	const unsigned char b0 = static_cast<unsigned char>(buffer[ByteSize{0}]);
	const unsigned char b1 = static_cast<unsigned char>(buffer[ByteSize{1}]);
	const unsigned char b2 = static_cast<unsigned char>(buffer[ByteSize{2}]);
	const unsigned char b3 = static_cast<unsigned char>(buffer[ByteSize{3}]);
	if (b0 != 0x04 || b1 != 0x03 || b2 != 0x02 || b3 != 0x01) {
		std::cerr << "test_wire_int_is_little_endian: got "
			<< static_cast<int>(b0) << " " << static_cast<int>(b1) << " "
			<< static_cast<int>(b2) << " " << static_cast<int>(b3) << "\n";
		RETURN_TEST("test_wire_int_is_little_endian", 1);
	}
	RETURN_TEST("test_wire_int_is_little_endian", 0);
}

int test_wire_string_length_is_uint64_le() {
	const std::string data = "AB";
	auto buffer = Serializable<std::string>(data).Serialize();
	if (ByteCount(buffer) != sizeof(std::uint64_t) + data.size()) {
		std::cerr << "test_wire_string_length_is_uint64_le: unexpected size\n";
		RETURN_TEST("test_wire_string_length_is_uint64_le", 1);
	}
	if (static_cast<unsigned char>(buffer[ByteSize{0}]) != 2 ||
			static_cast<unsigned char>(buffer[ByteSize{1}]) != 0 ||
			static_cast<unsigned char>(buffer[ByteSize{7}]) != 0 ||
			static_cast<char>(buffer[ByteSize{8}]) != 'A' ||
			static_cast<char>(buffer[ByteSize{9}]) != 'B') {
		std::cerr << "test_wire_string_length_is_uint64_le: layout mismatch\n";
		RETURN_TEST("test_wire_string_length_is_uint64_le", 1);
	}
	RETURN_TEST("test_wire_string_length_is_uint64_le", 0);
}

int main() {
	int result = 0;

	// -------------------
	// BinaryData
	// -------------------
	result += test_serialize_binary_data();
	result += test_serialize_binary_data_empty();
	result += test_serialize_binary_data_shares_vector_byte_wire();
	result += test_serialize_binary_data_trailing_garbage();
	result += test_serialize_binary_data_truncated();
	result += test_serialize_optional_binary_data();
	result += test_binary_data_corruption_huge_size();
	result += test_binary_data_corruption_no_crash_bit_flip();
	result += test_binary_data_corruption_no_crash_byte_overwrite();
	result += test_binary_data_corruption_random_stress();
	result += test_serialize_binary_data_from_blob_and_span();

	// -------------------
	// Codec
	// -------------------
	result += test_codec_custom_type();
	result += test_codec_custom_type_truncated();

	// -------------------
	// Corruption
	// -------------------
	result += test_base_corruption_empty_buffer();
	result += test_base_corruption_huge_container_size();
	result += test_base_corruption_huge_string_size();
	result += test_base_corruption_no_crash_bit_flip();
	result += test_base_corruption_no_crash_byte_overwrite();
	result += test_base_corruption_random_stress();
	result += test_base_corruption_string_truncated_all();
	result += test_base_corruption_vector_truncated_all();
	result += test_base_cross_type_vector_as_string();
	result += test_base_double_corruption();

	// -------------------
	// Nested
	// -------------------
	result += test_base_idempotent_roundtrip_pair();
	result += test_safe_pair_roundtrip();
	result += test_base_idempotent_roundtrip_vector();
	result += test_base_nested_vector_of_pairs();
	result += test_serialize_deep_nested_vector();

	// -------------------
	// Optional
	// -------------------
	result += test_serialize_nested_optional();
	result += test_serialize_optional_empty();
	result += test_serialize_optional_notempty();
	result += test_serialize_optional_string();

	// -------------------
	// Roundtrip
	// -------------------
	result += test_serialize_array();
	result += test_serialize_array_rejects_wrong_element_count();
	result += test_serialize_deserialize_big_string();
	result += test_serialize_deserialize_with_span();
	result += test_serialize_double();
	result += test_serialize_int();
	result += test_serialize_map();
	result += test_serialize_pair();
	result += test_serialize_size_t();
	result += test_serialize_string();
	result += test_serialize_string_vector();

	// -------------------
	// Truncated
	// -------------------
	result += test_serialize_deserialize_with_span_truncated();
	result += test_serialize_int_truncated();
	result += test_serialize_pair_truncated();
	result += test_serialize_string_vector_truncated();

	// -------------------
	// Unicode
	// -------------------
	result += test_cstring_shares_string_wire();
	result += test_serialize_cstring();
	result += test_serialize_cstring_empty();
	result += test_serialize_cstring_null_is_empty_wire();
	result += test_serialize_cstring_truncated();
	result += test_serialize_u16string();
	result += test_serialize_u16string_empty();
	result += test_serialize_u16string_huge_size();
	result += test_serialize_u16string_non_bmp();
	result += test_serialize_u16string_truncated();
	result += test_serialize_unicode_boundary_codepoints();
	result += test_serialize_wcstring();
	result += test_serialize_wcstring_empty();
	result += test_serialize_wcstring_non_bmp();
	result += test_serialize_wcstring_null_is_empty_wire();
	result += test_serialize_wcstring_truncated();
	result += test_serialize_wstring();
	result += test_serialize_wstring_non_bmp();
	result += test_wcstring_shares_wstring_wire();
		result += test_safe_strings_share_standard_wire();
	result += test_wide_and_u16_share_utf8_wire();

	// -------------------
	// Wire
	// -------------------
	result += test_base_bool_accepts_0_and_1();
	result += test_base_bool_rejects_invalid_byte();
	result += test_base_optional_flag_rejects_invalid_bool();
	result += test_base_trailing_garbage();
	result += test_wire_int_is_little_endian();
	result += test_wire_string_length_is_uint64_le();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}
