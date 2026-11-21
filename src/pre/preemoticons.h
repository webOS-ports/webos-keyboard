/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PRE_EMOTICONS_H
#define PRE_EMOTICONS_H

#include <QString>

namespace Pre {

/*!
 * Text faces and the emoji each stands for (":)" -> 🙂), for the bar to offer
 * one after a face is typed, as the Android Pre Keyboard's 1.4.3 does.
 */
namespace Emoticons {

//! A face at the end of the text: how many characters it takes (with a space after it), and its emoji.
struct Face
{
    int length = 0;
    QString emoji;
    //! What goes in front of the emoji in its place (a space a waiting face's colon had moved).
    QString lead;
    bool isNull() const { return length == 0; }
    bool operator==(const Face &o) const { return length == o.length && emoji == o.emoji && lead == o.lead; }
};

//! The emoji for exactly [face], or nothing.
QString emojiFor(const QString &face);

/*!
 * The face [before] ends in, a space after it allowed. It stands on its own
 * (at the start, or after a space or an opening bracket), or is glued to a
 * word when it is a colon face that closes (":)", ":(", ":]"), as in "hi:)";
 * not "https:/".
 */
Face at(const QString &before);

//! The most characters a face takes.
int longest();

} // namespace Emoticons
} // namespace Pre

#endif // PRE_EMOTICONS_H
