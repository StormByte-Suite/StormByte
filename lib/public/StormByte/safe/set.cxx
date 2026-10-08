/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This is the base for StormByte Suite libraries.
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

#include <StormByte/safe/string.hxx>
#include <StormByte/safe/set.hxx>

namespace StormByte {
	namespace Safe {
		template class STORMBYTE_INSTANTIATE Set<bool>;
		template class STORMBYTE_INSTANTIATE Set<char>;
		template class STORMBYTE_INSTANTIATE Set<signed char>;
		template class STORMBYTE_INSTANTIATE Set<unsigned char>;
		template class STORMBYTE_INSTANTIATE Set<wchar_t>;
		template class STORMBYTE_INSTANTIATE Set<char8_t>;
		template class STORMBYTE_INSTANTIATE Set<char16_t>;
		template class STORMBYTE_INSTANTIATE Set<char32_t>;
		template class STORMBYTE_INSTANTIATE Set<short>;
		template class STORMBYTE_INSTANTIATE Set<unsigned short>;
		template class STORMBYTE_INSTANTIATE Set<int>;
		template class STORMBYTE_INSTANTIATE Set<unsigned int>;
		template class STORMBYTE_INSTANTIATE Set<long>;
		template class STORMBYTE_INSTANTIATE Set<unsigned long>;
		template class STORMBYTE_INSTANTIATE Set<long long>;
		template class STORMBYTE_INSTANTIATE Set<unsigned long long>;
		template class STORMBYTE_INSTANTIATE Set<String>;
	}
}