#include <StormByte/safe/map.hxx>
#include <StormByte/test_handlers.h>

#include <cstddef>

using namespace StormByte;

// -------------------
// Difference-type keys
// -------------------

int test_safe_map_difference_type_empty_range() {
	int result = 0;
	Safe::Map<std::ptrdiff_t, int> values;
	ASSERT_NO_THROW("test_safe_map_difference_type_empty_range", result = values.begin() != values.end());
	ASSERT_EQUAL("test_safe_map_difference_type_empty_range", 0, result);
	const auto& view = values;
	ASSERT_NO_THROW("test_safe_map_difference_type_empty_range", result = view.cbegin() != view.cend());
	RETURN_TEST("test_safe_map_difference_type_empty_range", result);
}

int main() {
	int result = 0;

	// -------------------
	// Difference-type keys
	// -------------------
	result += test_safe_map_difference_type_empty_range();

	return result;
}