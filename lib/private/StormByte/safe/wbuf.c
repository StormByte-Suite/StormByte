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

#include "wbuf.h"

#include <stdlib.h>
#include <string.h>

struct WBuf {
	size_t size;
	size_t capacity;
	wchar_t data[];
};

static int Fits(size_t count) {
	return count <= (size_t)-1 / sizeof(wchar_t) - 1;
}

WBuf* wbuf_new(const wchar_t* text, size_t count) {
	WBuf* buffer;
	if (!Fits(count))
		return NULL;
	buffer = malloc(sizeof(WBuf) + (count + 1) * sizeof(wchar_t));
	if (!buffer)
		return NULL;
	buffer->size = count;
	buffer->capacity = count;
	if (count != 0 && text != NULL)
		memcpy(buffer->data, text, count * sizeof(wchar_t));
	buffer->data[count] = L'\0';
	return buffer;
}

void wbuf_free(WBuf* buffer) {
	free(buffer);
}

wchar_t* wbuf_data(WBuf* buffer) {
	return buffer->data;
}

size_t wbuf_size(const WBuf* buffer) {
	return buffer->size;
}

size_t wbuf_avail(const WBuf* buffer) {
	return buffer->capacity - buffer->size;
}

WBuf* wbuf_make_room(WBuf* buffer, size_t extra) {
	size_t needed;
	size_t capacity;
	WBuf* grown;
	if (!buffer)
		return NULL;
	if (extra <= wbuf_avail(buffer))
		return buffer;
	if (extra > (size_t)-1 - buffer->size - 1)
		return NULL;
	needed = buffer->size + extra;
	if (!Fits(needed))
		return NULL;
	capacity = buffer->capacity < 8 ? 8 : buffer->capacity;
	while (capacity < needed) {
		if (capacity > ((size_t)-1 / sizeof(wchar_t) - 1) / 2)
			return NULL;
		capacity *= 2;
	}
	grown = realloc(buffer, sizeof(WBuf) + (capacity + 1) * sizeof(wchar_t));
	if (!grown)
		return NULL;
	grown->capacity = capacity;
	grown->data[grown->size] = L'\0';
	return grown;
}

WBuf* wbuf_append(WBuf* buffer, const wchar_t* text, size_t count) {
	WBuf* grown;
	if (count == 0)
		return buffer;
	grown = wbuf_make_room(buffer, count);
	if (!grown)
		return NULL;
	if (text != NULL)
		memcpy(grown->data + grown->size, text, count * sizeof(wchar_t));
	grown->size += count;
	grown->data[grown->size] = L'\0';
	return grown;
}

void wbuf_set_len(WBuf* buffer, size_t count) {
	if (!buffer || count > buffer->capacity)
		return;
	buffer->size = count;
	buffer->data[count] = L'\0';
}
