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

namespace {
	std::atomic<std::uint64_t> liveContexts = 0;
	std::atomic<std::uint64_t> destroyedContexts = 0;

	struct Context {
		std::string text;

		Context(): text(8192, 'x') {
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
			stored.assign(value.data(), value.size());
			return value == "callback payload" ? Status::Success : Status::Missing;
		} catch (...) {
			return Status::Failure;
		}
	}

	void ReleaseContext(void* context) noexcept {
		std::unique_ptr<Context> owner(static_cast<Context*>(context));
	}
}

Sequence SafeCollectionsFixture::MakeSequence() {
	Sequence values;
	const Text longText(std::string(8192, 'x'));
	if (values.PushBack(longText) != Status::Success || values.PushBack(Text("second")) != Status::Success)
		throw StormByte::Exception("Could not create collection fixture");
	return values;
}

Dictionary SafeCollectionsFixture::MakeDictionary() {
	Dictionary values;
	if (values.Set(Text("beta"), Text("second")) != Status::Success || values.Set(Text("alpha"), Text("first")) != Status::Success)
		throw StormByte::Exception("Could not create dictionary fixture");
	return values;
}

MaybeText SafeCollectionsFixture::MakeOptional() {
	MaybeText value;
	if (value.Set(Text(std::string(8192, 'y'))) != Status::Success)
		throw StormByte::Exception("Could not create optional fixture");
	return value;
}

Nested SafeCollectionsFixture::MakeNested() {
	Nested values;
	if (values.PushBack(MakeSequence()) != Status::Success)
		throw StormByte::Exception("Could not create nested fixture");
	return values;
}

Callback SafeCollectionsFixture::MakeCallback() {
	auto context = std::make_unique<Context>();
	Callback callback(context.get(), &InvokeContext, &ReleaseContext);
	context.release();
	return callback;
}

TokenQueue SafeCollectionsFixture::MakeTokenQueue() {
	TokenQueue tokens;
	if (Text::Explode("producer|queue|value", '|', tokens) != Status::Success)
		throw StormByte::Exception("Could not create queue fixture");
	return tokens;
}

void SafeCollectionsFixture::ThrowProducerException() {
	throw StormByte::Exception("exception from producer DLL");
}

std::uint64_t SafeCollectionsFixture::LiveContexts() noexcept {
	return liveContexts.load();
}

std::uint64_t SafeCollectionsFixture::DestroyedContexts() noexcept {
	return destroyedContexts.load();
}