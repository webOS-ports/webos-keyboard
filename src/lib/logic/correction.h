/*
 * This file is part of the LuneOS keyboard.
 *
 * Copyright (C) 2026 Alan Morford <alan-morford@users.noreply.github.com>
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

#ifndef MALIIT_KEYBOARD_CORRECTION_H
#define MALIIT_KEYBOARD_CORRECTION_H

#include <QString>
#include <QStringList>

#include <functional>

namespace MaliitKeyboard {
namespace Logic {

/*!
 * What space puts in for a word typed: the word, or what it was meant to be.
 *
 * Presage's first guess was the correction, and it guesses by the letters
 * typed so far, so a word spelt right became a longer one ("hes" ->
 * "hesitated", "merci" -> "mercier") and one spelt wrong whatever began like
 * it ("hund" -> "hundert", "perche" -> "percherons"). The spelling checker
 * usually has the right word, but not always first. This picks, in order:
 *
 * 1. English contractions typed without the apostrophe ("dont" -> "don't",
 *    "im" -> "I'm"), and "i" -> "I": always, as the Android Pre Keyboard does.
 * 2. A word the dictionary knows stays as it is.
 * 3. The checker's suggestion that differs only in capitals, accents,
 *    apostrophes or a hyphen: German nouns ("haus" -> "Haus"), "ecole" ->
 *    "école", "didnt" -> "didn't", "cest" -> "c'est".
 * 4. The word with one letter given its accent ("fur" -> "für", "nino" ->
 *    "niño"), when the dictionary knows it.
 * 5. A word presage expects that the checker suggests too ("becaus" -> "because").
 * 6. Two letters the wrong way round ("taht" -> "that"), or two words run
 *    together ("ofthe" -> "of the"), whichever the checker has first.
 * 7. An elision without its apostrophe, French and Italian ("lecole" -> "l'école").
 * 8. The checker's first suggestion that isn't two words; with none, presage's
 *    first guess.
 *
 * Whatever it picks gets the typed word's first capital.
 */
class Correction
{
public:
    struct Dictionary {
        std::function<bool(const QString &word)> spell;
        std::function<QStringList(const QString &word)> suggest;
    };

    //! The correction for [typed] in [language] ("en", "de_AT"); [typed] itself to keep it.
    static QString choose(const QString &typed, const QString &language, const Dictionary &dictionary,
                          const QStringList &predictions);

    //! [word] lower case without accents, apostrophes, hyphens or spaces: what was typed for it.
    static QString fold(const QString &word);
    //! [word] with its first letter a capital when [like]'s is.
    static QString casedLike(const QString &like, const QString &word);
};

} // namespace Logic
} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_CORRECTION_H
