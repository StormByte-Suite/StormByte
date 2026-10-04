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
	Text output("empty unchanged");
	if (emptySequence.Size() != Size(0) || emptySequence.Get(Size(0), output) != Status::Missing || output != "empty unchanged")
		return false;
	if (emptySequence.Set(Size(0), Text("value")) != Status::Missing || emptySequence.Erase(Size(0)) != Status::Missing)
		return false;
	if (emptySequence.Clear() != Status::Success || emptySequence.Clear() != Status::Success || emptySequence.PushBack(Text("value")) != Status::Success)
		return false;

	Dictionary emptyDictionary;
	Text key("key unchanged");
	if (emptyDictionary.Size() != Size(0) || emptyDictionary.Get(Text("absent"), output) != Status::Missing || output != "empty unchanged")
		return false;
	if (emptyDictionary.GetAt(Size(0), key, output) != Status::Missing || key != "key unchanged" || output != "empty unchanged")
		return false;
	if (emptyDictionary.Erase(Text("absent")) != Status::Missing || emptyDictionary.Clear() != Status::Success || emptyDictionary.Clear() != Status::Success)
		return false;

	MaybeText emptyOptional;
	if (emptyOptional.HasValue() || emptyOptional.Value(output) != Status::Missing || output != "empty unchanged")
		return false;
	if (emptyOptional.Reset() != Status::Success || emptyOptional.Reset() != Status::Success || emptyOptional.Set(Text("value")) != Status::Success)
		return false;
	if (emptyOptional.Value(output) != Status::Success || output != "value" || emptyOptional.Reset() != Status::Success)
		return false;

	TokenQueue safeQueue;
	if (!safeQueue.Empty() || safeQueue.Front(output) != Status::Missing || safeQueue.Pop() != Status::Missing)
		return false;
	if (safeQueue.Push(Text("queue one")) != Status::Success || safeQueue.Push(Text("queue two")) != Status::Success)
		return false;
	std::queue<Text> standardQueue = static_cast<std::queue<Text>>(safeQueue);
	TokenQueue importedQueue(standardQueue);
	if (importedQueue.Size() != Size(2) || importedQueue.Front(output) != Status::Success || output != "queue one")
		return false;
	if (importedQueue.Pop() != Status::Success || importedQueue.Front(output) != Status::Success || output != "queue two")
		return false;

	const std::vector<Text> standardVector{Text("vector one"), Text("vector two")};
	Sequence importedVector(standardVector);
	const auto exportedVector = static_cast<std::vector<Text>>(importedVector);
	if (importedVector.Size() != Size(2) || exportedVector.size() != 2 || exportedVector[1] != "vector two")
		return false;
	auto producerVector = MakeSequence();
	const auto producerSTLVector = static_cast<std::vector<Text>>(producerVector);
	if (producerSTLVector.size() != 2 || producerSTLVector[1] != "second")
		return false;

	const std::map<Text, Text> standardMap{{Text("alpha"), Text("first")}, {Text("beta"), Text("second")}};
	Dictionary importedMap(standardMap);
	const auto exportedMap = static_cast<std::map<Text, Text>>(importedMap);
	if (importedMap.Size() != Size(2) || exportedMap.size() != 2 || exportedMap.at(Text("beta")) != "second")
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
	if (!exportedOptional || *exportedOptional != "optional value" || importedEmpty.HasValue() || exportedEmpty)
		return false;
	auto producerOptional = MakeOptional();
	const auto producerSTLOptional = static_cast<std::optional<Text>>(producerOptional);
	if (!producerSTLOptional || producerSTLOptional->size() != Size(8192))
		return false;

	TokenQueue exploded;
	if (Text("left||right|").Explode('|', exploded) != Status::Success || exploded.Size() != Size(4))
		return false;
	if (exploded.Front(output) != Status::Success || output != "left" || exploded.Pop() != Status::Success)
		return false;
	if (exploded.Front(output) != Status::Success || output != "" || exploded.Pop() != Status::Success)
		return false;
	if (exploded.Front(output) != Status::Success || output != "right" || exploded.Pop() != Status::Success)
		return false;
	if (exploded.Front(output) != Status::Success || output != "" || exploded.Pop() != Status::Success || !exploded.Empty())
		return false;
	Safe::Queue<WString> wideTokens;
	WString wideOutput;
	if (WString(L"wide|text").Explode(L'|', wideTokens) != Status::Success || wideTokens.Size() != Size(2))
		return false;
	if (wideTokens.Front(wideOutput) != Status::Success || wideOutput != L"wide")
		return false;

	Safe::Vector<Text> words;
	if (Text(" one\t two ").Split(words) != Status::Success || words.Size() != Size(2))
		return false;
	if (words.Get(Size(0), output) != Status::Success || output != "one")
		return false;
	Safe::Vector<WString> wideWords;
	if (WString(L" wide\t text ").Split(wideWords) != Status::Success || wideWords.Size() != Size(2))
		return false;
	if (wideWords.Get(Size(1), wideOutput) != Status::Success || wideOutput != L"text")
		return false;

	auto producerQueue = MakeTokenQueue();
	if (producerQueue.Size() != Size(3) || producerQueue.Front(output) != Status::Success || output != "producer")
		return false;
	const auto producerSTLQueue = static_cast<std::queue<Text>>(producerQueue);
	if (producerSTLQueue.size() != 3 || producerSTLQueue.front() != "producer")
		return false;
	auto producerQueueCopy = producerQueue;
	auto producerQueueMoved = std::move(producerQueueCopy);
	if (!producerQueueCopy.Empty() || producerQueueMoved.Size() != Size(3))
		return false;
	if (producerQueue.Pop() != Status::Success || producerQueue.Front(output) != Status::Success || output != "queue")
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
		Sequence assigned;
		assigned = original;
		assigned = assigned;
		auto moved = std::move(copy);
		copy = std::move(copy);
		if (copy.Size() != Size(0) || moved.Size() != Size(2))
			return false;
		if (copy.PushBack(Text("moved-from")) != Status::Failure || copy.Clear() != Status::Failure)
			return false;
		output = Text("unchanged");
		if (copy.Get(Size(0), output) != Status::Missing || output != "unchanged")
			return false;
		if (moved.Get(Size(0), output) != Status::Success || output.size() != Size(8192))
			return false;
		if (moved.Set(Size(0), Text("replacement")) != Status::Success)
			return false;
		if (original.Get(Size(0), output) != Status::Success || output.size() != Size(8192))
			return false;
		if (assigned.Erase(Size(0)) != Status::Success || assigned.Size() != Size(1))
			return false;
		if (assigned.Get(Size(99), output) != Status::Missing || assigned.Set(Size(99), output) != Status::Missing)
			return false;
		if (assigned.Clear() != Status::Success || assigned.Size() != Size(0))
			return false;
		assigned = std::move(moved);
		if (moved.Size() != Size(0) || assigned.Get(Size(0), output) != Status::Success || output != "replacement")
			return false;
		if (moved.PushBack(Text("moved-from")) != Status::Failure || moved.Clear() != Status::Failure)
			return false;

		auto map = MakeDictionary();
		auto mapCopy = map;
		Dictionary mapAssigned;
		mapAssigned = map;
		mapAssigned = mapAssigned;
		auto mapMoved = std::move(mapCopy);
		Text key;
		if (mapCopy.Size() != Size(0) || mapCopy.Get(Text("alpha"), output) != Status::Missing || output != "replacement")
			return false;
		if (mapCopy.Erase(Text("alpha")) != Status::Missing || mapMoved.GetAt(Size(0), key, output) != Status::Success || key != "alpha" || output != "first")
			return false;
		if (mapMoved.Set(Text("alpha"), Text("changed")) != Status::Success || map.Get(Text("alpha"), output) != Status::Success || output != "first")
			return false;
		if (mapMoved.Set(Text("gamma"), Text("third")) != Status::Success || mapMoved.Size() != Size(3))
			return false;
		if (mapMoved.Erase(Text("beta")) != Status::Success || mapMoved.Erase(Text("absent")) != Status::Missing)
			return false;
		if (mapMoved.GetAt(Size(0), key, output) != Status::Success || key != "alpha" || output != "changed")
			return false;
		if (mapMoved.GetAt(Size(1), key, output) != Status::Success || key != "gamma" || output != "third")
			return false;
		output = Text("get unchanged");
		if (mapMoved.Get(Text("absent"), output) != Status::Missing || output != "get unchanged")
			return false;
		key = Text("key unchanged");
		output = Text("value unchanged");
		if (mapMoved.GetAt(Size(99), key, output) != Status::Missing || key != "key unchanged" || output != "value unchanged")
			return false;
		mapAssigned = std::move(mapMoved);
		if (mapMoved.Size() != Size(0) || mapAssigned.Clear() != Status::Success || mapAssigned.Size() != Size(0))
			return false;
		if (mapMoved.Set(Text("key"), Text("value")) != Status::Failure || mapMoved.Clear() != Status::Failure)
			return false;
		if (mapAssigned.Set(Text("self"), Text("assignment")) != Status::Success)
			return false;
		mapAssigned = std::move(mapAssigned);
		if (mapAssigned.Get(Text("self"), output) != Status::Success || output != "assignment")
			return false;

		auto optional = MakeOptional();
		auto optionalCopy = optional;
		MaybeText optionalAssigned;
		optionalAssigned = optional;
		optionalAssigned = optionalAssigned;
		auto optionalMoved = std::move(optionalCopy);
		if (optionalCopy.HasValue() || optionalCopy.Value(output) != Status::Missing)
			return false;
		if (optionalCopy.Set(Text("moved-from")) != Status::Failure || optionalCopy.Reset() != Status::Failure)
			return false;
		if (!optionalMoved.HasValue() || optionalMoved.Value(output) != Status::Success || output.size() != Size(8192))
			return false;
		if (optionalMoved.Set(Text("changed")) != Status::Success || optional.Value(output) != Status::Success || output.size() != Size(8192))
			return false;
		optionalAssigned = std::move(optionalMoved);
		if (optionalMoved.HasValue() || optionalAssigned.Reset() != Status::Success || optionalAssigned.HasValue())
			return false;
		if (optionalAssigned.Set(Text("self")) != Status::Success)
			return false;
		optionalAssigned = std::move(optionalAssigned);
		if (!optionalAssigned.HasValue() || optionalAssigned.Value(output) != Status::Success || output != "self")
			return false;

		auto nested = MakeNested();
		auto nestedCopy = nested;
		Sequence extracted;
		if (nestedCopy.Get(Size(0), extracted) != Status::Success || extracted.Size() != Size(2))
			return false;
		if (extracted.Set(Size(0), Text("local")) != Status::Success || nested.Get(Size(0), extracted) != Status::Success)
			return false;
		if (extracted.Get(Size(0), output) != Status::Success || output.size() != Size(8192))
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