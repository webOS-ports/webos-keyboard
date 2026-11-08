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

#include "prelanguage.h"

namespace Pre {

const QString Language::DICTIONARY = QStringLiteral("dictionary.txt");
const QString Language::NEXT_WORDS = QStringLiteral("next_words.txt");
const QString Language::EMOJI_WORDS = QStringLiteral("emoji_words.txt");
const QString Language::CONTRACTIONS = QStringLiteral("contractions.txt");

QStringList layoutRows(Layout layout)
{
    switch (layout) {
    case Layout::Qwertz:
        // German: Y and Z change places.
        return { QStringLiteral("qwertzuiop"), QStringLiteral("asdfghjkl"), QStringLiteral("yxcvbnm") };
    case Layout::Azerty:
        // French: A and Q, Z and W change places; M stays on the bottom row, as on
        // BlackBerry's AZERTY.
        return { QStringLiteral("azertyuiop"), QStringLiteral("qsdfghjkl"), QStringLiteral("wxcvbnm") };
    case Layout::Qwerty:
    default:
        return { QStringLiteral("qwertyuiop"), QStringLiteral("asdfghjkl"), QStringLiteral("zxcvbnm") };
    }
}

Language::Language(const QString &id_, const QString &name_, const QString &shortName_, Layout layout_,
                   const QString &dictionaryDir, const QString &dataDir,
                   const QString &accents_, const QSet<QString> &elisions_)
    : id(id_), name(name_), shortName(shortName_), layout(layout_), accents(accents_),
      elisions(elisions_), m_dictionaryDir(dictionaryDir), m_dataDir(dataDir)
{
}

QString Language::asset(const QString &name) const
{
    const QString dir = (name == DICTIONARY) ? m_dictionaryDir : m_dataDir;
    return dir.isEmpty() ? name : dir + QLatin1Char('/') + name;
}

QStringList Language::dictionaries() const
{
    if (english() && !m_dictionaryDir.isEmpty())
        return { DICTIONARY, asset(DICTIONARY) };
    return { asset(DICTIONARY) };
}

bool Language::has(const QString &name) const
{
    return name != CONTRACTIONS || english();
}

QString Language::accentsOf(QChar letter) const
{
    QString out;
    for (QChar c : accents)
        if (base(c) == letter)
            out += c;
    return out;
}

QPair<QString, QString> Language::elided(const QString &word) const
{
    if (elisions.isEmpty())
        return {};
    int at = -1;
    for (int i = 0; i < word.length(); ++i) {
        if (word[i] == QLatin1Char('\'') || word[i] == QChar(0x2019)) { at = i; break; }
    }
    if (at <= 0 || at == word.length() - 1)
        return {};
    if (!elisions.contains(word.left(at).toLower()))
        return {};
    return { word.left(at + 1), word.mid(at + 1) };
}

QString Language::bare(const QString &word) const
{
    const auto e = elided(word);
    return e.first.isEmpty() ? word : e.second;
}

const QList<Language> &Language::all()
{
    static const QList<Language> languages = {
        enUS(),
        Language(QStringLiteral("en_GB"), QStringLiteral("English (UK)"), QStringLiteral("English (UK)"),
                 Layout::Qwerty, QStringLiteral("en_GB"), QString()),
        Language(QStringLiteral("en_CA"), QStringLiteral("English (Canada)"), QStringLiteral("English (Canada)"),
                 Layout::Qwerty, QStringLiteral("en_CA"), QString()),
        Language(QStringLiteral("en_AU"), QStringLiteral("English (Australia)"), QStringLiteral("English (Australia)"),
                 Layout::Qwerty, QStringLiteral("en_AU"), QString()),
        Language(QStringLiteral("fr"), QStringLiteral("Français"), QStringLiteral("Français"),
                 Layout::Azerty, QStringLiteral("fr"), QStringLiteral("fr"),
                 QStringLiteral("éèàçêùâôîëïûüœæÿ"),
                 { QStringLiteral("l"), QStringLiteral("d"), QStringLiteral("j"), QStringLiteral("m"),
                   QStringLiteral("n"), QStringLiteral("s"), QStringLiteral("t"), QStringLiteral("c"),
                   QStringLiteral("qu"), QStringLiteral("jusqu"), QStringLiteral("lorsqu"),
                   QStringLiteral("puisqu"), QStringLiteral("quoiqu"), QStringLiteral("quelqu"),
                   QStringLiteral("presqu") }),
        Language(QStringLiteral("de"), QStringLiteral("Deutsch"), QStringLiteral("Deutsch"),
                 Layout::Qwertz, QStringLiteral("de"), QStringLiteral("de"), QStringLiteral("äöüß")),
        Language(QStringLiteral("es"), QStringLiteral("Español"), QStringLiteral("Español"),
                 Layout::Qwerty, QStringLiteral("es"), QStringLiteral("es"), QStringLiteral("ñéáóíúü")),
        Language(QStringLiteral("it"), QStringLiteral("Italiano"), QStringLiteral("Italiano"),
                 Layout::Qwerty, QStringLiteral("it"), QStringLiteral("it"),
                 QStringLiteral("èàùòìéóí"),
                 { QStringLiteral("l"), QStringLiteral("d"), QStringLiteral("dell"), QStringLiteral("all"),
                   QStringLiteral("dall"), QStringLiteral("nell"), QStringLiteral("sull"), QStringLiteral("coll"),
                   QStringLiteral("un"), QStringLiteral("c"), QStringLiteral("quest"), QStringLiteral("quell"),
                   QStringLiteral("bell"), QStringLiteral("sant"), QStringLiteral("tutt"), QStringLiteral("senz"),
                   QStringLiteral("anch"), QStringLiteral("cos"), QStringLiteral("dov"), QStringLiteral("com"),
                   QStringLiteral("nient"), QStringLiteral("s"), QStringLiteral("m"), QStringLiteral("t"),
                   QStringLiteral("v"), QStringLiteral("n") }),
    };
    return languages;
}

const Language &Language::enUS()
{
    static const Language us(QStringLiteral("en_US"), QStringLiteral("English (US)"),
                             QStringLiteral("English (US)"), Layout::Qwerty, QString(), QString());
    return us;
}

Language Language::byId(const QString &id)
{
    for (const Language &l : all())
        if (l.id == id)
            return l;
    return Language();
}

QChar Language::base(QChar c)
{
    if (c.unicode() < 0xC0)   // 'À'
        return c;
    if (c == QChar(0xDF))     // ß
        return QLatin1Char('s');
    if (c == QChar(0x153))    // œ
        return QLatin1Char('o');
    if (c == QChar(0xE6))     // æ
        return QLatin1Char('a');
    const QString d = QString(c).normalized(QString::NormalizationForm_D);
    if (!d.isEmpty() && (d.length() > 1 || d[0] != c))
        return d[0];
    return c;
}

QString Language::fold(const QString &word)
{
    QString sb;
    sb.reserve(word.length());
    for (QChar c : word) {
        if (c == QChar(0xDF)) sb += QLatin1String("ss");
        else if (c == QChar(0x153)) sb += QLatin1String("oe");
        else if (c == QChar(0x152)) sb += QLatin1String("OE");
        else if (c == QChar(0xE6)) sb += QLatin1String("ae");
        else if (c == QChar(0xC6)) sb += QLatin1String("AE");
        else if (c.isUpper()) sb += base(c.toLower()).toUpper();
        else sb += base(c);
    }
    return sb;
}

} // namespace Pre
