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
#include <StormByte/safe/wstring.hxx>

#include <map>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

using namespace StormByte;
using namespace StormByte::Safe;
using namespace SafeCollectionsFixture;

// -------------------
// DLL ownership
// -------------------

bool SafeCollectionsFixture::ExerciseCollections() {
	const auto destroyedBefore = DestroyedContexts();
	bool caughtProducerException = false;
	try {
		ThrowProducerException();
	} catch (const Exception& exception) {
		caughtProducerException = std::string_view(exception.what()) == "StormByte: exception from producer DLL";
	} catch (...) {
		return false;
	}
	if (!caughtProducerException)
		return false;

	Sequence emptySequence;
	if (!emptySequence.empty())
		return false;
	bool boundsThrown = false;
	try {
		(void)emptySequence.at(0);
	} catch (const Exception&) {
		boundsThrown = true;
	}
	if (!boundsThrown)
		return false;
	emptySequence.clear();
	emptySequence.push_back(Text("value"));
	Text output("empty unchanged");

	Dictionary emptyDictionary;
	if (!emptyDictionary.empty() || emptyDictionary.erase(Text("absent")) != 0)
		return false;
	emptyDictionary.clear();
	bool missingKeyThrown = false;
	try {
		(void)emptyDictionary.at(Text("absent"));
	} catch (const Exception&) {
		missingKeyThrown = true;
	}
	if (!missingKeyThrown)
		return false;

	MaybeText emptyOptional;
	if (emptyOptional.has_value())
		return false;
	emptyOptional.reset();
	emptyOptional.emplace(Text("value"));
	if (emptyOptional.value() != "value")
		return false;
	emptyOptional.reset();

	TokenQueue safeQueue;
	if (!safeQueue.empty())
		return false;
	safeQueue.push(Text("queue one"));
	safeQueue.push(Text("queue two"));
	std::queue<Text> standardQueue = static_cast<std::queue<Text>>(safeQueue);
	TokenQueue importedQueue(standardQueue);
	if (importedQueue.size() != 2 || importedQueue.front() != "queue one")
		return false;
	importedQueue.pop();
	if (importedQueue.front() != "queue two")
		return false;

	const std::vector<Text> standardVector{Text("vector one"), Text("vector two")};
	Sequence importedVector(standardVector);
	const auto exportedVector = static_cast<std::vector<Text>>(importedVector);
	if (importedVector.size() != 2 || exportedVector.size() != 2 || exportedVector[1] != "vector two")
		return false;
	auto producerVector = MakeSequence();
	const auto producerSTLVector = static_cast<std::vector<Text>>(producerVector);
	if (producerSTLVector.size() != 2 || producerSTLVector[1] != "second")
		return false;

	const std::map<Text, Text> standardMap{{Text("alpha"), Text("first")}, {Text("beta"), Text("second")}};
	Dictionary importedMap(standardMap);
	const auto exportedMap = static_cast<std::map<Text, Text>>(importedMap);
	if (importedMap.size() != 2 || exportedMap.size() != 2 || exportedMap.at(Text("beta")) != "second")
		return false;
	auto producerMap = MakeDictionary();
	const auto producerSTLMap = static_cast<std::map<Text, Text>>(producerMap);
	if (producerSTLMap.size() != 2 || producerSTLMap.at(Text("alpha")) != "first")
		return false;

	const std::optional<Text> standardOptional{Text("optional value")};
	MaybeText importedOptional(standardOptional);
	const auto exportedOptional = static_cast<std::optional<Text>>(importedOptional);
	const std::optional<Text> standardEmpty;
	MaybeText importedEmpty(standardEmpty);
	const auto exportedEmpty = static_cast<std::optional<Text>>(importedEmpty);
	if (!exportedOptional || *exportedOptional != "optional value" || importedEmpty.has_value() || exportedEmpty)
		return false;
	auto producerOptional = MakeOptional();
	const auto producerSTLOptional = static_cast<std::optional<Text>>(producerOptional);
	if (!producerSTLOptional || producerSTLOptional->size() != Size(8192))
		return false;
	auto producerLevel = MakeOptionalLevel();
	MaybeLevel copiedLevel;
	copiedLevel = producerLevel;
	MaybeLevel movedLevel(std::move(producerLevel));
	if (producerLevel.has_value() || copiedLevel != OptionalTestLevel::Warning || movedLevel.value_or(OptionalTestLevel::Info) != OptionalTestLevel::Warning)
		return false;
	producerLevel.emplace(OptionalTestLevel::Info);
	if (producerLevel != OptionalTestLevel::Info || movedLevel != OptionalTestLevel::Warning)
		return false;

	TokenQueue exploded;
	if (Text("left||right|").Explode('|', exploded) != Status::Success || exploded.size() != 4)
		return false;
	if (exploded.front() != "left")
		return false;
	exploded.pop();
	if (exploded.front() != "")
		return false;
	exploded.pop();
	if (exploded.front() != "right")
		return false;
	exploded.pop();
	if (exploded.front() != "" || exploded.size() != 1)
		return false;
	exploded.pop();
	if (!exploded.empty())
		return false;
	Safe::Queue<WString> wideTokens;
	WString wideOutput;
	if (WString(L"wide|text").Explode(L'|', wideTokens) != Status::Success || wideTokens.size() != 2)
		return false;
	if (wideTokens.front() != L"wide")
		return false;

	Safe::Vector<Text> words;
	if (Text(" one\t two ").Split(words) != Status::Success || words.size() != 2)
		return false;
	if (static_cast<Text>(words[0]) != "one")
		return false;
	Safe::Vector<WString> wideWords;
	if (WString(L" wide\t text ").Split(wideWords) != Status::Success || wideWords.size() != 2)
		return false;
	if (static_cast<WString>(wideWords[1]) != L"text")
		return false;

	auto producerQueue = MakeTokenQueue();
	if (producerQueue.size() != 3 || producerQueue.front() != "producer")
		return false;
	const auto producerSTLQueue = static_cast<std::queue<Text>>(producerQueue);
	if (producerSTLQueue.size() != 3 || producerSTLQueue.front() != "producer")
		return false;
	auto producerQueueCopy = producerQueue;
	auto producerQueueMoved = std::move(producerQueueCopy);
	if (!producerQueueCopy.empty() || producerQueueMoved.size() != 3)
		return false;
	producerQueueCopy.push(Text("reused after move"));
	if (producerQueueCopy.front() != "reused after move")
		return false;
	producerQueue.pop();
	if (producerQueue.front() != "queue")
		return false;

	auto queue = Text("first|second|third").Explode('|');
	if (queue.size() != 3 || queue.front() != "first")
		return false;
	queue.pop();
	if (queue.size() != 2 || queue.front() != "second")
		return false;
	queue.pop();
	if (queue.size() != 1 || queue.front() != "third")
		return false;

	for (std::uint64_t iteration = 0; iteration < 128; ++iteration) {
		auto original = MakeSequence();
		auto copy = original;
		Sequence assigned = original;
		auto moved = std::move(copy);
		if (!copy.empty() || moved.size() != 2)
			return false;
		copy.push_back(Text("reused"));
		if (copy.front() != "reused" || static_cast<Text>(moved.front()).size() != Size(8192))
			return false;
		moved[0] = Text("replacement");
		if (static_cast<Text>(original[0]).size() != Size(8192))
			return false;
		assigned.erase(assigned.begin());
		if (assigned.size() != 1 || static_cast<Text>(assigned.at(0)) != "second")
			return false;
		assigned.clear();
		assigned = std::move(moved);
		if (!moved.empty() || static_cast<Text>(assigned.at(0)) != "replacement")
			return false;
		moved.push_back(Text("reused"));
		if (moved.front() != "reused")
			return false;

		auto map = MakeDictionary();
		auto mapCopy = map;
		Dictionary mapAssigned = map;
		auto mapMoved = std::move(mapCopy);
		if (!mapCopy.empty() || static_cast<Text>(mapMoved.at(Text("alpha"))) != "first")
			return false;
		mapCopy.insert_or_assign(Text("reused"), Text("after move"));
		if (static_cast<Text>(mapCopy.at(Text("reused"))) != "after move")
			return false;
		mapMoved.at(Text("alpha")) = Text("changed");
		mapMoved.insert_or_assign(Text("gamma"), Text("third"));
		if (static_cast<Text>(map.at(Text("alpha"))) != "first" || mapMoved.size() != 3)
			return false;
		if (mapMoved.erase(Text("beta")) != 1 || mapMoved.erase(Text("absent")) != 0)
			return false;
		mapAssigned = std::move(mapMoved);
		if (!mapMoved.empty())
			return false;
		mapMoved.insert_or_assign(Text("reused"), Text("value"));
		if (static_cast<Text>(mapMoved.at(Text("reused"))) != "value")
			return false;
		mapAssigned.clear();

		auto optional = MakeOptional();
		auto optionalCopy = optional;
		MaybeText optionalAssigned = optional;
		auto optionalMoved = std::move(optionalCopy);
		if (optionalCopy.has_value() || optionalMoved.value().size() != Size(8192))
			return false;
		optionalCopy.emplace(Text("reused"));
		optionalMoved.emplace(Text("changed"));
		if (optionalCopy.value() != "reused" || optional.value().size() != Size(8192))
			return false;
		optionalAssigned = std::move(optionalMoved);
		if (optionalMoved.has_value() || optionalAssigned.value() != "changed")
			return false;
		optionalAssigned.reset();
		optionalAssigned.emplace(Text("self"));
		if (optionalAssigned.value() != "self")
			return false;

		auto nested = MakeNested();
		auto nestedCopy = nested;
		Sequence extracted = static_cast<Sequence>(nestedCopy[0]);
		if (extracted.size() != 2)
			return false;
		extracted[0] = Text("local");
		Sequence nestedOriginal = static_cast<Sequence>(nested[0]);
		if (static_cast<Text>(nestedOriginal[0]).size() != Size(8192))
			return false;

		{
			auto callback = MakeCallback();
			auto callbackMoved = std::move(callback);
			if (callback.HasValue() || callback.Call(output) != Status::Missing)
				return false;
			if (callbackMoved.Call(Text("callback payload")) != Status::Success ||
				callbackMoved.Call(Text("unexpected payload")) != Status::Missing ||
				callbackMoved.Call(Text("callback failure")) != Status::Failure)
				return false;
			auto callbackAssigned = MakeCallback();
			callbackAssigned = std::move(callbackMoved);
			if (callbackMoved.HasValue() || LiveContexts() != 1)
				return false;
		}
		if (LiveContexts() != 0)
			return false;
	}
	return DestroyedContexts() == destroyedBefore + 256;
}