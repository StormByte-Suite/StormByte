/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 *
 * Full PGO training workload. Touches every explicit template
 * instantiation exported by the library (Vector, Deque, List, Set,
 * UnorderedSet, Map, UnorderedMap, Queue, Atomic, plus text, Binary,
 * Optional, Variant and smart pointers) so LTO+PGO can optimize them.
 */

#include <StormByte/safe/atomic.hxx>
#include <StormByte/safe/binary.hxx>
#include <StormByte/safe/deque.hxx>
#include <StormByte/safe/list.hxx>
#include <StormByte/safe/map.hxx>
#include <StormByte/safe/optional.hxx>
#include <StormByte/safe/pair.hxx>
#include <StormByte/safe/pointers.hxx>
#include <StormByte/safe/queue.hxx>
#include <StormByte/safe/set.hxx>
#include <StormByte/safe/string.hxx>
#include <StormByte/safe/unordered_map.hxx>
#include <StormByte/safe/unordered_set.hxx>
#include <StormByte/safe/variant.hxx>
#include <StormByte/safe/vector.hxx>
#include <StormByte/safe/wstring.hxx>
#include <StormByte/serializable.hxx>
#include <StormByte/size.hxx>
#include <StormByte/test_handlers.h>

#include <string>
#include <utility>

using namespace StormByte;
using namespace StormByte::Safe;

namespace {

constexpr int kIterations = 200;
constexpr int kN = 32;

template<class T>
T Make(int i) {
	if constexpr (std::is_same_v<T, String>) {
		String s("v");
		s.append(std::to_string(i % 20));
		return s;
	} else if constexpr (std::is_same_v<T, bool>) {
		return (i & 1) != 0;
	} else {
		return static_cast<T>(i);
	}
}

template<class T>
void TouchVector() {
	Vector<T> v;
	v.reserve(Size{kN});
	for (int i = 0; i < kN; ++i) v.push_back(Make<T>(i));
	for (const auto& x : v) (void)x;
}

template<class T>
void TouchDeque() {
	Deque<T> d;
	for (int i = 0; i < kN; ++i) {
		d.push_back(Make<T>(i));
		d.push_front(Make<T>(i));
	}
	while (!d.empty()) d.pop_front();
}

template<class T>
void TouchList() {
	List<T> l;
	for (int i = 0; i < kN; ++i) l.push_back(Make<T>(i));
	while (!l.empty()) l.pop_front();
}

template<class T>
void TouchSet() {
	Set<T> s;
	UnorderedSet<T> us;
	for (int i = 0; i < kN; ++i) {
		s.insert(Make<T>(i));
		us.insert(Make<T>(i));
	}
	for (int i = 0; i < kN; ++i) {
		(void)s.find(Make<T>(i));
		(void)us.find(Make<T>(i));
	}
}

template<class T>
void TouchQueue() {
	Queue<T> q;
	for (int i = 0; i < kN; ++i) q.push(Make<T>(i));
	while (!q.empty()) {
		(void)q.front();
		q.pop();
	}
}

template<class T>
void TouchAtomic() {
	Atomic<T> a(Make<T>(1));
	a.store(Make<T>(2));
	(void)a.load();
	a.fetch_add(Make<T>(1));
}

template<class K, class V>
void TouchMap() {
	Map<K, V> m;
	UnorderedMap<K, V> um;
	for (int i = 0; i < 16; ++i) {
		m.insert({Make<K>(i), Make<V>(i)});
		um.insert({Make<K>(i), Make<V>(i)});
	}
	for (int i = 0; i < 16; ++i) {
		(void)m.find(Make<K>(i));
		(void)um.find(Make<K>(i));
	}
}

void TouchTextAndPointers() {
	Vector<String> shorts, longs;
	Vector<WString> wides;
	for (int i = 0; i < kN; ++i) {
		String s("sso");
		s.push_back('x');
		shorts.push_back(std::move(s));
		String l(std::string(80, 'L'));
		l.append("z");
		longs.push_back(std::move(l));
		WString w(L"w");
		w.push_back(L'z');
		wides.push_back(std::move(w));
	}
	for (auto& s : longs) s.push_back('!');
	for (const auto& w : wides) (void)w.size();

	Vector<Binary> bins;
	for (int i = 0; i < 16; ++i) {
		Binary b;
		b.reserve(ByteSize{24});
		b.push_back(std::byte{1});
		bins.push_back(std::move(b));
	}

	Vector<Optional<String>> opts;
	Vector<Variant<int, String>> vars;
	for (int i = 0; i < kN; ++i) {
		Optional<String> o;
		if (i & 1) o.emplace(String("o"));
		opts.push_back(std::move(o));
		Variant<int, String> var;
		if (i & 1) var.emplace<int>(i);
		else var.emplace<String>(String("v"));
		vars.push_back(std::move(var));
	}

	Vector<Shared<String>> sh;
	
	for (int i = 0; i < 16; ++i) {
		sh.push_back(MakeShared<String>("p"));
		
	}
	for (const auto& s : sh) if (s) (void)s->size();
	

	Vector<Pair<String, int>> ps;
	for (int i = 0; i < kN; ++i)
		ps.push_back(Pair<String, int>{String("p"), i});
}

void TouchSerializable() {
	// Exercise the exported Serializable specializations lightly
	(void)Serializable<int>::Size(1);
	(void)Serializable<String>::Size(String("s"));
	(void)Serializable<WString>::Size(WString(L"w"));
	Binary b;
	b.push_back(std::byte{0});
	(void)Serializable<Binary>::Size(b);
}

#define TOUCH_SCALAR(T) \
	TouchVector<T>(); \
	TouchDeque<T>(); \
	TouchList<T>(); \
	TouchSet<T>(); \
	TouchQueue<T>(); \
	TouchAtomic<T>();

void AllExported() {
	TOUCH_SCALAR(bool)
	TOUCH_SCALAR(char)
	TOUCH_SCALAR(signed char)
	TOUCH_SCALAR(unsigned char)
	TOUCH_SCALAR(wchar_t)
	TOUCH_SCALAR(char8_t)
	TOUCH_SCALAR(char16_t)
	TOUCH_SCALAR(char32_t)
	TOUCH_SCALAR(short)
	TOUCH_SCALAR(unsigned short)
	TOUCH_SCALAR(int)
	TOUCH_SCALAR(unsigned int)
	TOUCH_SCALAR(long)
	TOUCH_SCALAR(unsigned long)
	TOUCH_SCALAR(long long)
	TOUCH_SCALAR(unsigned long long)
	TouchVector<String>(); TouchDeque<String>(); TouchList<String>(); TouchSet<String>(); TouchQueue<String>();

	TouchMap<int, int>();
	TouchMap<int, String>();
	TouchMap<String, int>();
	TouchMap<String, String>();

	TouchTextAndPointers();
	TouchSerializable();
}

} // namespace

int main() {
	int result = 0;
	for (int n = 0; n < kIterations; ++n)
		AllExported();
	std::cout << "Full PGO instrumentation completed (" << kIterations << " iterations)\n";
	RETURN_TEST(result);
}
