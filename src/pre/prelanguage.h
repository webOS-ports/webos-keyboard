/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
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

#ifndef PRE_LANGUAGE_H
#define PRE_LANGUAGE_H

#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

namespace Pre {

/*!
 * Where the letters go. Qwerty, Qwertz and Azerty are the Pre's own grid, with
 * its printed number and symbol alternates; Own is a language whose letters
 * come from its phone layout (tools/gen_pre_keymaps.py).
 */
enum class Layout { Qwerty, Qwertz, Azerty, Own };

//! The three letter rows of the Pre's grid for [layout]: "qwertyuiop", "asdfghjkl", "zxcvbnm".
QStringList layoutRows(Layout layout);

//! A letter key of a language with its own rows, as its phone layout has it.
struct LetterKey
{
    QString text;
    //! What shift types; empty for the letter's capital.
    QString shifted;
    //! The press-and-hold list, the letter first; empty for none.
    QStringList extended;
    QStringList extendedShifted;
};

typedef QList<QList<LetterKey>> LetterRows;

/*!
 * A keyboard language, by the code LuneOS's keyboard uses for it ("en", "ru",
 * "zh"): one per language plugin. Its words - prediction, spelling, the owner's
 * dictionary - are the plugin's; this is only where its letters go.
 */
struct Language
{
    Language() = default;

    QString id;
    //! In its own language: "Deutsch", "Русский".
    QString name;
    Layout layout = Layout::Qwerty;
    //! On the Pre's grid: the language's own accents, first in a letter's holds.
    QString accents;
    //! Layout::Own: its letter rows.
    LetterRows letters;
    bool rightToLeft = false;
    //! Punctuation as the language writes it, for the keys' own: Arabic's ، ؟ ؛ for , ? ;.
    QHash<QString, QString> punctuation;

    //! [s] as the language writes it: its own punctuation mark, or [s].
    QString written(const QString &s) const { return punctuation.value(s, s); }
    bool isValid() const { return !id.isEmpty(); }
    bool operator==(const Language &o) const { return id == o.id; }
    bool operator!=(const Language &o) const { return id != o.id; }

    //! The accents of [accents] whose letter is [letter], in order: e -> "éèêë".
    QString accentsOf(QChar letter) const;

    //! Every language with a plugin, by id.
    static const QList<Language> &all();
    //! The language [id], or an invalid one.
    static Language byId(const QString &id);
    //! The language for [id] whatever it is: English for one without a plugin.
    static Language orEnglish(const QString &id);

    //! [c] without its accent: é -> e, ß -> s; anything else as it is.
    static QChar base(QChar c);
    //! [word] in plain letters: "être" -> "etre", "straße" -> "strasse".
    static QString fold(const QString &word);

private:
    // For the generated table (prelanguages_generated.cpp).
    static Language preGrid(const QString &id, const QString &name, Layout layout);
    static Language ownRows(const QString &id, const QString &name, bool rightToLeft, const LetterRows &rows);
};

} // namespace Pre

#endif // PRE_LANGUAGE_H
