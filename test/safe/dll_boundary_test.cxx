#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/test_handlers.h>

#include <algorithm>
#include <utility>

extern "C" void FillAcrossBoundary(
	StormByte::Safe::Vector<std::byte>&,
	StormByte::Safe::Vector<StormByte::Safe::String>&,
	StormByte::Safe::String&,
	StormByte::Safe::WString&);
extern "C" void GrowAcrossBoundary(
	StormByte::Safe::Vector<std::byte>&,
	StormByte::Safe::Vector<StormByte::Safe::String>&);

int test_owned_values_cross_module_and_return() {
	using namespace StormByte;
	Safe::Vector<std::byte> bytes;
	Safe::Vector<Safe::String> words;
	Safe::String text;
	Safe::WString wide;
	for (int round = 0; round < 2; ++round) {
		FillAcrossBoundary(bytes, words, text, wide);
		ASSERT_EQUAL(Size{128 * 1024}, bytes.size());
		ASSERT_EQUAL(Size{128}, words.size());
		Safe::Vector<std::byte> copy(bytes);
		bytes.assign(copy.begin() + 1, copy.end());
		bytes.resize(bytes.capacity() + 1, bytes.front());
		words.assign(Size{64}, words.front());
		GrowAcrossBoundary(bytes, words);
		ASSERT_TRUE(std::all_of(bytes.begin(), bytes.end(), [](std::byte value) {
			return value == std::byte{0x5a};
		}));
		ASSERT_TRUE(std::all_of(words.begin(), words.end(), [](const Safe::String& value) {
			return value == Safe::String{Size{128}, 'x'};
		}));
		Safe::String moved(std::move(text));
		moved.append(static_cast<std::string_view>(moved));
		text = std::move(moved);
		text.assign(static_cast<std::string_view>(text).substr(1, 3));
		text.shrink_to_fit();
		wide.append(static_cast<std::wstring_view>(wide));
		wide.assign(static_cast<std::wstring_view>(wide).substr(1, 2));
		wide.shrink_to_fit();
		ASSERT_EQUAL(Safe::String{"yyy"}, text);
		ASSERT_EQUAL(Safe::WString{L"zz"}, wide);
	}
	RETURN_TEST(0);
}

int main() {
	RETURN_TEST(test_owned_values_cross_module_and_return());
}
