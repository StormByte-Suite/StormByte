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

#include "clonable_plugin.hxx"

#include <utility>

using namespace StormByte::Safe;

PluginItem::PluginItem(int value): value(value) {}

PluginItem::~PluginItem() noexcept = default;

PluginItem::PointerType PluginItem::Clone() const {
	return MakePointer<PluginItem>(*this);
}

PluginItem::PointerType PluginItem::Move() {
	return MakePointer<PluginItem>(std::move(*this));
}

PluginItem::PointerType MakePluginItem(int value) {
	return PluginItem::MakePointer<PluginItem>(value);
}

const std::type_info& PluginClonableType() noexcept {
	return typeid(Clonable<PluginItem>);
}

Clonable<PluginItem>* PluginAsClonable(PluginItem& item) noexcept {
	return &item;
}
