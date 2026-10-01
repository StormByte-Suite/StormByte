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

#pragma once

#include <StormByte/platform.h>
#include <StormByte/safe/clonable.hxx>

#include <typeinfo>

/**
 * @file clonable_plugin.hxx
 * @brief Test-only shared library that derives from @ref StormByte::Safe::Clonable in its own module.
 *
 * Built with hidden visibility, like a typical consumer DLL.
 */

#ifdef WINDOWS
	#ifdef ClonablePlugin_EXPORTS
		#define CLONABLE_PLUGIN_PUBLIC	__declspec(dllexport)
	#else
		#define CLONABLE_PLUGIN_PUBLIC	__declspec(dllimport)
	#endif
#else
	#define CLONABLE_PLUGIN_PUBLIC		__attribute__((visibility("default")))
#endif

/**
 * @class PluginItem
 * @brief Clonable whose vtable, `Clone` and `Move` live in the plugin module.
 */
class CLONABLE_PLUGIN_PUBLIC PluginItem: public StormByte::Safe::Clonable<PluginItem> {
	public:
		/**
		 * @brief Construct with @p value.
		 * @param value Payload.
		 */
		explicit PluginItem(int value);

		/**
		 * @brief Copy constructor.
		 */
		PluginItem(const PluginItem&) = default;

		/**
		 * @brief Move constructor.
		 */
		PluginItem(PluginItem&&) noexcept = default;

		/**
		 * @brief Copy assignment.
		 * @return @c *this.
		 */
		PluginItem& operator=(const PluginItem&) = default;

		/**
		 * @brief Move assignment.
		 * @return @c *this.
		 */
		PluginItem& operator=(PluginItem&&) noexcept = default;

		/**
		 * @brief Destructor. Out of line so the plugin owns the key function.
		 */
		~PluginItem() noexcept override;

		/**
		 * @brief Deep copy built inside the plugin.
		 * @return Owner on Base's heap.
		 */
		PointerType Clone() const override;

		/**
		 * @brief Move built inside the plugin.
		 * @return Owner on Base's heap.
		 */
		PointerType Move() override;

		int value;	///< Payload.
};

/**
 * @brief Allocate a @ref PluginItem inside the plugin.
 * @param value Payload.
 * @return Owner on Base's heap.
 */
CLONABLE_PLUGIN_PUBLIC PluginItem::PointerType MakePluginItem(int value);

/**
 * @brief `typeid` of `Clonable<PluginItem>` as seen by the plugin.
 * @return The plugin's `type_info`.
 */
CLONABLE_PLUGIN_PUBLIC const std::type_info& PluginClonableType() noexcept;

/**
 * @brief The `Clonable<PluginItem>` subobject of @p item, cast inside the plugin.
 * @param item Plugin item.
 * @return Base subobject.
 */
CLONABLE_PLUGIN_PUBLIC StormByte::Safe::Clonable<PluginItem>* PluginAsClonable(PluginItem& item) noexcept;
