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

#include "prelanguage.h"

namespace Pre {

QStringList layoutRows(Layout layout)
{
    switch (layout) {
    case Layout::Qwertz:
        // German: Y and Z change places.
        return { QStringLiteral("qwertzuiop"), QStringLiteral("asdfghjkl"), QStringLiteral("yxcvbnm") };
    case Layout::Azerty:
        // French: A and Q, Z and W change places; M stays on the bottom row, as
        // on the Pre's AZERTY.
        return { QStringLiteral("azertyuiop"), QStringLiteral("qsdfghjkl"), QStringLiteral("wxcvbnm") };
    case Layout::Own:
        return {};
    case Layout::Qwerty:
    default:
        return { QStringLiteral("qwertyuiop"), QStringLiteral("asdfghjkl"), QStringLiteral("zxcvbnm") };
    }
}

namespace {

//! The accents that come first in a letter's holds, for the languages on the Pre's grid.
QString preAccents(const QString &id)
{
    if (id == QLatin1String("fr")) return QStringLiteral("éèàçêùâôîëïûüœæÿ");
    if (id == QLatin1String("de")) return QStringLiteral("äöüß");
    if (id == QLatin1String("es")) return QStringLiteral("ñéáóíúü");
    if (id == QLatin1String("it")) return QStringLiteral("èàùòìéóí");
    if (id == QLatin1String("nl")) return QStringLiteral("ëéèïöüá");
    return QString();
}

} // namespace

Language Language::preGrid(const QString &id, const QString &name, Layout layout)
{
    Language l;
    l.id = id;
    l.name = name;
    l.layout = layout;
    l.accents = preAccents(id);
    return l;
}

Language Language::ownRows(const QString &id, const QString &name, bool rightToLeft, const LetterRows &rows)
{
    Language l;
    l.id = id;
    l.name = name;
    l.layout = Layout::Own;
    l.letters = rows;
    l.rightToLeft = rightToLeft;
    return l;
}

QString Language::accentsOf(QChar letter) const
{
    QString out;
    for (QChar c : accents)
        if (base(c) == letter)
            out += c;
    return out;
}

Language Language::byId(const QString &id)
{
    for (const Language &l : all())
        if (l.id == id)
            return l;
    return Language();
}

Language Language::orEnglish(const QString &id)
{
    const Language l = byId(id);
    return l.isValid() ? l : byId(QStringLiteral("en"));
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
