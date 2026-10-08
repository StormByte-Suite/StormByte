#include <StormByte/safe/exception.hxx>
#include <StormByte/safe/function.hxx>
#include <StormByte/safe/thread.hxx>
#include <StormByte/test_handlers.h>

#include <atomic>
#include <iostream>
#include <utility>

using namespace StormByte;

namespace {
	struct LifetimeCounts {
		std::atomic<int> called{0};
		std::atomic<int> cloned{0};
		std::atomic<int> released{0};
	};

	struct CallbackState {
		LifetimeCounts* counts;
		bool throws;
	};

	void* clone_callback(const void* raw) noexcept {
		const auto* source = static_cast<const CallbackState*>(raw);
		auto* copy = new CallbackState{source->counts, source->throws};
		source->counts->cloned.fetch_add(1);
		return copy;
	}

	void release_callback(void* raw) noexcept {
		auto* state = static_cast<CallbackState*>(raw);
		state->counts->released.fetch_add(1);
		delete state;
	}

	Safe::Status invoke_callback(void* raw) {
		auto* state = static_cast<CallbackState*>(raw);
		state->counts->called.fetch_add(1);
		if (state->throws)
			throw Safe::Exception("Thread lifetime regression");
		return Safe::Status::Success;
	}

	Safe::Function<void()> make_callback(LifetimeCounts& counts, bool throws = false) {
		return Safe::Function<void()>(new CallbackState{&counts, throws},
			invoke_callback, clone_callback, release_callback);
	}
}

// -------------------
// Move
// -------------------

int test_self_move_and_moved_from_thread_reuse() {
	LifetimeCounts counts;
	Safe::Thread source(make_callback(counts));
	const auto original_id = source.get_id();
	auto* same = &source;
	source = std::move(*same);
	const bool self_move_preserved = source.joinable() && source.get_id() == original_id;
	Safe::Thread destination(std::move(source));
	const bool source_empty = !source.joinable() && source.get_id() == Safe::Thread::Id{} && source.native_handle() == 0;
	source = Safe::Thread(make_callback(counts));
	destination.join();
	source.join();
	destination = std::move(source);
	ASSERT_TRUE(self_move_preserved);
	ASSERT_TRUE(source_empty);
	ASSERT_FALSE(source.joinable());
	ASSERT_FALSE(destination.joinable());
	ASSERT_EQUAL(2, counts.called.load());
	ASSERT_EQUAL(0, counts.cloned.load());
	ASSERT_EQUAL(2, counts.released.load());
	RETURN_TEST(0);
}

// -------------------
// Callback Lifetime
// -------------------

int test_callback_survives_source_destruction_and_thread_move() {
	LifetimeCounts counts;
	Safe::Thread source;
	{
		auto callback = make_callback(counts);
		source = Safe::Thread(callback);
	}
	Safe::Thread destination;
	destination = std::move(source);
	destination.join();
	ASSERT_FALSE(source.joinable());
	ASSERT_TRUE(source.get_id() == Safe::Thread::Id{});
	ASSERT_EQUAL(1, counts.called.load());
	ASSERT_EQUAL(1, counts.cloned.load());
	ASSERT_EQUAL(2, counts.released.load());
	RETURN_TEST(0);
}

int test_throwing_callback_released_once_after_move_and_join() {
	LifetimeCounts counts;
	auto callback = make_callback(counts, true);
	Safe::Thread source(std::move(callback));
	Safe::Thread destination(std::move(source));
	destination.join();
	ASSERT_FALSE(callback.HasValue());
	ASSERT_FALSE(source.joinable());
	ASSERT_FALSE(destination.joinable());
	ASSERT_EQUAL(1, counts.called.load());
	ASSERT_EQUAL(0, counts.cloned.load());
	ASSERT_EQUAL(1, counts.released.load());
	RETURN_TEST(0);
}

int main() {
	int result = 0;

	// -------------------
	// Move
	// -------------------
	result += test_self_move_and_moved_from_thread_reuse();

	// -------------------
	// Callback Lifetime
	// -------------------
	result += test_callback_survives_source_destruction_and_thread_move();
	result += test_throwing_callback_released_once_after_move_and_join();

	if (result == 0)
		std::cout << "All tests passed!" << std::endl;
	else
		std::cout << result << " tests failed." << std::endl;
	return result;
}