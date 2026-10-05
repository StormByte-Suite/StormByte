#include <StormByte/safe/map.hxx>
#include <StormByte/test_handlers.h>

#include <utility>

using namespace StormByte;

// -------------------
// Empty ranges
// -------------------

int test_safe_map_empty_mutable_begin() {
	int result = 0;
	Safe::Map<int, int> values;
	ASSERT_NO_THROW("test_safe_map_empty_mutable_begin", result = values.begin() != values.end());
	RETURN_TEST("test_safe_map_empty_mutable_begin", result);
}

int test_safe_map_empty_const_begin() {
	int result = 0;
	const Safe::Map<int, int> values;
	ASSERT_NO_THROW("test_safe_map_empty_const_begin", result = values.begin() != values.end());
	RETURN_TEST("test_safe_map_empty_const_begin", result);
}

int test_safe_map_empty_cbegin() {
	int result = 0;
	const Safe::Map<int, int> values;
	ASSERT_NO_THROW("test_safe_map_empty_cbegin", result = values.cbegin() != values.cend());
	RETURN_TEST("test_safe_map_empty_cbegin", result);
}

int test_safe_map_empty_lookup() {
	int result = 0;
	Safe::Map<int, int> values;
	ASSERT_NO_THROW("test_safe_map_empty_lookup", result = !(values.find(7) == values.end() && !values.contains(7)));
	RETURN_TEST("test_safe_map_empty_lookup", result);
}

// -------------------
// Ordered traversal
// -------------------

int test_safe_map_first_key_is_not_zero() {
	int result = 0;
	Safe::Map<int, int> values;
	ASSERT_NO_THROW("test_safe_map_first_key_is_not_zero", values.emplace(7, 70));
	ASSERT_NO_THROW("test_safe_map_first_key_is_not_zero", result = [&] {
		const auto entry = *std::as_const(values).begin();
		ASSERT_EQUAL("test_safe_map_first_key_is_not_zero", 7, entry.first);
		ASSERT_EQUAL("test_safe_map_first_key_is_not_zero", 70, entry.second);
		return 0;
	}());
	RETURN_TEST("test_safe_map_first_key_is_not_zero", result);
}

int test_safe_map_zero_is_not_first_key() {
	int result = 0;
	Safe::Map<int, int> values;
	ASSERT_NO_THROW("test_safe_map_zero_is_not_first_key", values.emplace(-3, 30));
	ASSERT_NO_THROW("test_safe_map_zero_is_not_first_key", values.emplace(0, 0));
	ASSERT_NO_THROW("test_safe_map_zero_is_not_first_key", result = [&] {
		const auto entry = *std::as_const(values).begin();
		ASSERT_EQUAL("test_safe_map_zero_is_not_first_key", -3, entry.first);
		ASSERT_EQUAL("test_safe_map_zero_is_not_first_key", 30, entry.second);
		return 0;
	}());
	RETURN_TEST("test_safe_map_zero_is_not_first_key", result);
}

int test_safe_map_ordered_traversal_without_zero() {
	int result = 0;
	Safe::Map<int, int> values;
	ASSERT_NO_THROW("test_safe_map_ordered_traversal_without_zero", values.emplace(7, 70));
	ASSERT_NO_THROW("test_safe_map_ordered_traversal_without_zero", values.emplace(-3, 30));
	ASSERT_NO_THROW("test_safe_map_ordered_traversal_without_zero", values.emplace(11, 110));
	ASSERT_NO_THROW("test_safe_map_ordered_traversal_without_zero", result = [&] {
		const auto& view = std::as_const(values);
		auto current = view.begin();
		ASSERT_TRUE("test_safe_map_ordered_traversal_without_zero", current != view.end());
		ASSERT_TRUE("test_safe_map_ordered_traversal_without_zero", current->first == -3);
		++current;
		ASSERT_TRUE("test_safe_map_ordered_traversal_without_zero", current != view.end());
		ASSERT_TRUE("test_safe_map_ordered_traversal_without_zero", current->first == 7);
		++current;
		ASSERT_TRUE("test_safe_map_ordered_traversal_without_zero", current != view.end());
		ASSERT_TRUE("test_safe_map_ordered_traversal_without_zero", current->first == 11);
		++current;
		ASSERT_TRUE("test_safe_map_ordered_traversal_without_zero", current == view.end());
		return 0;
	}());
	RETURN_TEST("test_safe_map_ordered_traversal_without_zero", result);
}

// -------------------
// Clear
// -------------------

int test_safe_map_clear_restores_empty_range() {
	int result = 0;
	Safe::Map<int, int> values;
	ASSERT_NO_THROW("test_safe_map_clear_restores_empty_range", values.emplace(0, 10));
	ASSERT_NO_THROW("test_safe_map_clear_restores_empty_range", values.clear());
	ASSERT_NO_THROW("test_safe_map_clear_restores_empty_range", result = values.cbegin() != values.cend());
	RETURN_TEST("test_safe_map_clear_restores_empty_range", result);
}

int main() {
	int result = 0;

	// -------------------
	// Empty ranges
	// -------------------
	result += test_safe_map_empty_mutable_begin();
	result += test_safe_map_empty_const_begin();
	result += test_safe_map_empty_cbegin();
	result += test_safe_map_empty_lookup();

	// -------------------
	// Ordered traversal
	// -------------------
	result += test_safe_map_first_key_is_not_zero();
	result += test_safe_map_zero_is_not_first_key();
	result += test_safe_map_ordered_traversal_without_zero();

	// -------------------
	// Clear
	// -------------------
	result += test_safe_map_clear_restores_empty_range();

	return result;
}