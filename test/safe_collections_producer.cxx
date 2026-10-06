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

#include "safe_collections_plugin.hxx"

#include <StormByte/exception.hxx>

#include <atomic>
#include <memory>
#include <string>

using namespace StormByte;
using namespace StormByte::Safe;
using namespace SafeCollectionsFixture;

ProviderException::~ProviderException() noexcept = default;

namespace {
	std::atomic<std::uint64_t> liveContexts = 0;
	std::atomic<std::uint64_t> destroyedContexts = 0;
	std::atomic<std::uint64_t> liveProgressContexts = 0;
	std::atomic<std::uint64_t> destroyedProgressContexts = 0;
	std::atomic<double> lastProgress = 0.0;
	std::atomic<std::uint64_t> liveMaybeValues = 0;

	struct Context {
		std::string text;

		Context(): text(8192, 'x') {
			++liveContexts;
		}

		Context(const Context& other): text(other.text) {
			++liveContexts;
		}

		~Context() noexcept {
			--liveContexts;
			++destroyedContexts;
		}
	};

	Status InvokeContext(void* context, const Text& text) noexcept {
		const auto value = static_cast<std::string_view>(text);
		if (value == "callback failure")
			return Status::Failure;
		try {
			auto& stored = static_cast<Context*>(context)->text;
			if (value == "check untouched clone")
				return stored == std::string(8192, 'x') ? Status::Success : Status::Failure;
			stored.assign(value.data(), value.size());
			return value == "callback payload" ? Status::Success : Status::Missing;
		} catch (...) {
			return Status::Failure;
		}
	}

	void* CloneContext(const void* context) noexcept {
		try {
			return new Context(*static_cast<const Context*>(context));
		} catch (...) {
			return nullptr;
		}
	}

	void ReleaseContext(void* context) noexcept {
		std::unique_ptr<Context> owner(static_cast<Context*>(context));
	}

	struct ProgressContext {
		ProgressContext() {
			++liveProgressContexts;
		}

		ProgressContext(const ProgressContext&) {
			++liveProgressContexts;
		}

		~ProgressContext() noexcept {
			--liveProgressContexts;
			++destroyedProgressContexts;
		}
	};

	Status InvokeProgress(void*, double value) {
		if (value == -1.0)
			throw ProviderException("typed callback exception from producer DLL");
		if (value == -2.0)
			throw 7;
		lastProgress = value;
		return Status::Success;
	}

	void* CloneProgressContext(const void*) noexcept {
		try {
			return new ProgressContext();
		} catch (...) {
			return nullptr;
		}
	}

	Status InvokeSizeSelector(void* context, StormByte::Size* output, StormByte::Size value) {
		if (value == StormByte::Size{0})
			return Status::Failure;
		*output = value / *static_cast<const unsigned int*>(context);
		return Status::Success;
	}

	void* CloneSizeSelector(const void* context) noexcept {
		try {
			return new unsigned int(*static_cast<const unsigned int*>(context));
		} catch (...) {
			return nullptr;
		}
	}

	void ReleaseProgressContext(void* context) noexcept {
		std::unique_ptr<ProgressContext> owner(static_cast<ProgressContext*>(context));
	}

	void ReleaseSizeSelector(void* context) noexcept {
		delete static_cast<unsigned int*>(context);
	}

	void* CloneMaybeValue(const void* state) noexcept {
		try {
			return new MaybeValue(*static_cast<const MaybeValue*>(state));
		} catch (...) {
			return nullptr;
		}
	}

	void DestroyMaybeValue(void* state) noexcept {
		delete static_cast<MaybeValue*>(state);
	}

	Owner MakeMaybeOwner(std::string_view value) {
		auto state = std::make_unique<MaybeValue>(value);
		Owner owner(state.get(), &CloneMaybeValue, &DestroyMaybeValue);
		state.release();
		return owner;
	}
}

MaybeValue::MaybeValue(): m_text{} {
	++liveMaybeValues;
}

MaybeValue::MaybeValue(std::string_view value): m_text(value) {
	++liveMaybeValues;
}

MaybeValue::MaybeValue(const MaybeValue& other): m_text(other.m_text) {
	++liveMaybeValues;
}

MaybeValue::MaybeValue(MaybeValue&& other) noexcept: m_text(std::move(other.m_text)) {
	++liveMaybeValues;
}

MaybeValue::~MaybeValue() noexcept {
	--liveMaybeValues;
}

MaybeValue& MaybeValue::operator=(const MaybeValue& other) {
	if (this != &other)
		m_text = other.m_text;
	return *this;
}

MaybeValue& MaybeValue::operator=(MaybeValue&& other) noexcept {
	if (this != &other)
		m_text = std::move(other.m_text);
	return *this;
}

std::string_view MaybeValue::Text() const noexcept {
	return m_text;
}

Sequence SafeCollectionsFixture::MakeSequence() {
	Sequence values;
	const Text longText(std::string(8192, 'x'));
	values.push_back(longText);
	values.push_back(Text("second"));
	return values;
}

MaybeValues SafeCollectionsFixture::MakeMaybeValues() {
	MaybeValues values;
	values.emplace_back("provider maybe value");
	values.emplace_back("second provider value");
	return values;
}

MaybeOwners SafeCollectionsFixture::MakeMaybeOwners() {
	MaybeOwners values;
	values.push_back(MakeMaybeOwner("opaque owner value"));
	values.push_back(MakeMaybeOwner("second opaque owner"));
	return values;
}

Dictionary SafeCollectionsFixture::MakeDictionary() {
	Dictionary values;
	values.insert_or_assign(Text("beta"), Text("second"));
	values.insert_or_assign(Text("alpha"), Text("first"));
	return values;
}

MaybeText SafeCollectionsFixture::MakeOptional() {
	MaybeText value;
	value.emplace(Text(std::string(8192, 'y')));
	return value;
}

MaybeLevel SafeCollectionsFixture::MakeOptionalLevel() {
	return OptionalTestLevel::Warning;
}

Nested SafeCollectionsFixture::MakeNested() {
	Nested values;
	values.push_back(MakeSequence());
	return values;
}

Callback SafeCollectionsFixture::MakeCallback() {
	auto context = std::make_unique<Context>();
	Callback callback(context.get(), &InvokeContext, &CloneContext, &ReleaseContext);
	context.release();
	return callback;
}

CallbackFunction SafeCollectionsFixture::MakeProgressCallback() {
	auto context = std::make_unique<ProgressContext>();
	CallbackFunction callback(context.get(), &InvokeProgress, &CloneProgressContext, &ReleaseProgressContext);
	context.release();
	return callback;
}

SizeSelector SafeCollectionsFixture::MakeSizeSelector() {
	auto divisor = std::make_unique<unsigned int>(2);
	SizeSelector selector(divisor.get(), &InvokeSizeSelector, &CloneSizeSelector, &ReleaseSizeSelector);
	divisor.release();
	return selector;
}

TokenQueue SafeCollectionsFixture::MakeTokenQueue() {
	return Text::Explode("producer|queue|value", '|');
}

void SafeCollectionsFixture::ThrowProducerException() {
	throw StormByte::Exception("exception from producer DLL");
}

void SafeCollectionsFixture::ThrowProviderException() {
	throw ProviderException("derived exception from producer DLL");
}

std::uint64_t SafeCollectionsFixture::LiveContexts() noexcept {
	return liveContexts.load();
}

std::uint64_t SafeCollectionsFixture::DestroyedContexts() noexcept {
	return destroyedContexts.load();
}

std::uint64_t SafeCollectionsFixture::LiveProgressContexts() noexcept {
	return liveProgressContexts.load();
}

std::uint64_t SafeCollectionsFixture::DestroyedProgressContexts() noexcept {
	return destroyedProgressContexts.load();
}

double SafeCollectionsFixture::LastProgress() noexcept {
	return lastProgress.load();
}

std::uint64_t SafeCollectionsFixture::LiveMaybeValues() noexcept {
		return liveMaybeValues.load();
}
