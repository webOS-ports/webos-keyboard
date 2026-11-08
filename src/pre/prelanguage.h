/*
 * This file is part of the LuneOS keyboard: the Pre Keyboard.
 *
 * Copyright (C) 2026 Alan Morford <alan-morford@users.noreply.github.com>
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

#include <QList>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>

namespace Pre {

//! Where the letters go on the Pre's grid: the layout each language's typists know.
enum class Layout { Qwerty, Qwertz, Azerty };

//! The three letter rows of [layout]: "qwertyuiop", "asdfghjkl", "zxcvbnm".
QStringList layoutRows(Layout layout);

/*!
 * A language the keyboard types: its keys, and the words it corrects to and
 * suggests. The owner checks the ones they want in Text Assist and flicks the
 * space bar to go from one to the next.
 *
 * The four Englishes share everything but their dictionaries (colour or
 * color): the owner's own words, what they type after what, and the emoji.
 * Each other language has its own.
 */
class Language
{
public:
    Language() = default;
    Language(const QString &id, const QString &name, const QString &shortName, Layout layout,
             const QString &dictionaryDir, const QString &dataDir,
             const QString &accents = QString(), const QSet<QString> &elisions = {});

    //! Stored in the settings: "en_US", "fr".
    QString id;
    //! In Text Assist, in the language itself.
    QString name;
    //! Flashed on the space bar when it comes up.
    QString shortName;
    Layout layout = Layout::Qwerty;
    //! The language's own accented letters, commonest first.
    QString accents;
    //! Words written against the next one with an apostrophe ("l'homme").
    QSet<QString> elisions;

    bool isValid() const { return !id.isEmpty(); }
    bool english() const { return id.startsWith(QLatin1String("en")); }
    //! Words the owner adds, and what they type after what, are kept per family: "en", "fr".
    QString family() const { return id.section(QLatin1Char('_'), 0, 0); }

    //! The file [name] for this language, relative to the data directory.
    QString asset(const QString &name) const;
    //! The dictionary's files, read one after the other (a regional English after the US one).
    QStringList dictionaries() const;
    //! Whether there is a file [name] for this language (contractions are English only).
    bool has(const QString &name) const;
    //! [letter]'s accented forms in this language ("e": é è ê ë), commonest first.
    QString accentsOf(QChar letter) const;
    /*!
     * [word] split at its elision: ("l'", "homme") for "l'homme"; an empty pair
     * when it has none. The prefix keeps its apostrophe and its case.
     */
    QPair<QString, QString> elided(const QString &word) const;
    //! The word to look [word] up by: without its elision.
    QString bare(const QString &word) const;

    bool operator==(const Language &other) const { return id == other.id; }
    bool operator!=(const Language &other) const { return id != other.id; }

    static const QString DICTIONARY;
    static const QString NEXT_WORDS;
    static const QString EMOJI_WORDS;
    static const QString CONTRACTIONS;

    //! Every language, in Text Assist's order.
    static const QList<Language> &all();
    static Language byId(const QString &id);
    static const Language &enUS();

    //! [c] without its accent: é -> e, ß -> s; anything else as it is.
    static QChar base(QChar c);
    //! [word] in plain letters: "être" -> "etre", "straße" -> "strasse".
    static QString fold(const QString &word);

private:
    QString m_dictionaryDir;
    QString m_dataDir;
};

} // namespace Pre

#endif // PRE_LANGUAGE_H
