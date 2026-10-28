#include <StormByte/platform.h>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/safe/wstring.hxx>

#ifdef WINDOWS
#define BOUNDARY_EXPORT __declspec(dllexport)
#else
#define BOUNDARY_EXPORT __attribute__((visibility("default")))
#endif

extern "C" BOUNDARY_EXPORT void FillAcrossBoundary(
	StormByte::Safe::Vector<std::byte>& bytes,
	StormByte::Safe::Vector<StormByte::Safe::String>& words,
	StormByte::Safe::String& text,
	StormByte::Safe::WString& wide) {
	using namespace StormByte;
	Safe::Vector<std::byte> replacement(Size{128 * 1024}, std::byte{0x5a});
	bytes.swap(replacement);
	Safe::Vector<Safe::String> replacement_words(Size{128}, Safe::String{Size{128}, 'x'});
	words.swap(replacement_words);
	text.assign(Size{128}, 'y');
	wide.assign(Size{128}, L'z');
}

extern "C" BOUNDARY_EXPORT void GrowAcrossBoundary(
	StormByte::Safe::Vector<std::byte>& bytes,
	StormByte::Safe::Vector<StormByte::Safe::String>& words) {
	bytes.resize(bytes.capacity() + 1, bytes.front());
	words.resize(words.capacity() + 1, words.front());
}
