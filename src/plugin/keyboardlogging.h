/*
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; version 3.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LUNEOS_KEYBOARD_LOGGING_H
#define LUNEOS_KEYBOARD_LOGGING_H

#include <QLoggingCategory>

//! Every key the plugin is handed, and what the T9 multi-tap machine decides
//! to do with it. Off unless asked for:
//!
//!   QT_LOGGING_RULES="luneos.keyboard.keys.debug=true"
//!
//! Set it on maliit-server (a systemd drop-in on maliit-server@.service does
//! the job) and the lines land in that unit's journal. Worth having as a
//! switch rather than a patch: the keys are grabbed by the compositor's evdev
//! plugin, so evtest and friends see nothing whatever is pressed, and this is
//! the only place the real event stream can be observed on a device whose
//! screen cannot be read.
Q_DECLARE_LOGGING_CATEGORY(lcKeys)

//! Hardware keyboard profile matching: which profiles loaded, what input
//! devices were found, and which profile won.
//!
//!   QT_LOGGING_RULES="luneos.keyboard.hw.debug=true"
Q_DECLARE_LOGGING_CATEGORY(lcHwKeyboard)

#endif // LUNEOS_KEYBOARD_LOGGING_H
