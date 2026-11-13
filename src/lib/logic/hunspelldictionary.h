/*
 * This file is part of the LuneOS keyboard
 *
 * Copyright (C) 2026 Herman van Hazendonk <github.com@herrie.org>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 */

#ifndef MALIIT_KEYBOARD_HUNSPELLDICTIONARY_H
#define MALIIT_KEYBOARD_HUNSPELLDICTIONARY_H

#include <QString>
#include <QStringList>

namespace MaliitKeyboard {
namespace Logic {

/*
 * Which hunspell dictionary a keyboard language is spelled with in a country.
 *
 * Several are installed for some languages (en_AU, en_CA, en_GB, en_US, en_ZA;
 * de, de_AT, de_CH; pt, pt_BR), and taking the first in the directory gave
 * every English speaker Australian spelling. Now:
 *
 *  1. the language as spoken in the country, when there is a dictionary for
 *     it: en_GB in the UK, de_CH in Switzerland, pt_BR in Brazil;
 *  2. the language's own dictionary (de, pt);
 *  3. English elsewhere: American spelling where it is the custom (the US and
 *     the places that follow it), British spelling everywhere else, and
 *     American when the country is not known;
 *  4. the language's home variant (xx_XX), then the first there is.
 *
 * [languageId] is the keyboard's ("en", "no"); [country] an ISO 3166 code in
 * either case, or empty; [available] the dictionaries' base names ("en_GB",
 * for en_GB.aff and en_GB.dic). Empty when none is for the language.
 */
inline QString hunspellDictionary(const QString &languageId, const QString &country,
                                  const QStringList &available)
{
    // Hunspell's name for a language whose keyboard is called otherwise:
    // Norwegian is Bokmål (nb.aff), there is no no.aff.
    const QString id = languageId == QLatin1String("no") ? QStringLiteral("nb") : languageId;
    if (id.isEmpty()) return QString();
    const QString cc = country.toUpper();
    const QString prefix = id + QLatin1Char('_');

    if (!cc.isEmpty() && available.contains(prefix + cc)) return prefix + cc;
    if (available.contains(id)) return id;

    if (id == QLatin1String("en")) {
        // American spelling: the US, its territories, and the countries whose
        // schools and press use it. Elsewhere English is spelled the British way.
        static const QStringList american = {
            QStringLiteral("US"), QStringLiteral("AS"), QStringLiteral("GU"),
            QStringLiteral("MP"), QStringLiteral("PR"), QStringLiteral("UM"),
            QStringLiteral("VI"), QStringLiteral("PH"), QStringLiteral("LR"),
            QStringLiteral("MH"), QStringLiteral("FM"), QStringLiteral("PW"),
        };
        const bool us = cc.isEmpty() || american.contains(cc);
        const QString first = us ? QStringLiteral("en_US") : QStringLiteral("en_GB");
        const QString second = us ? QStringLiteral("en_GB") : QStringLiteral("en_US");
        if (available.contains(first)) return first;
        if (available.contains(second)) return second;
    }

    if (available.contains(prefix + id.toUpper())) return prefix + id.toUpper();

    QStringList any;
    for (const QString &name : available)
        if (name.startsWith(prefix)) any << name;
    any.sort();
    return any.isEmpty() ? QString() : any.first();
}

//! The dictionaries in [dir]'s listing [files]: the base names with both a .aff and a .dic.
inline QStringList hunspellDictionaries(const QStringList &files)
{
    QStringList out;
    for (const QString &f : files) {
        if (!f.endsWith(QLatin1String(".aff"))) continue;
        const QString base = f.left(f.size() - 4);
        if (files.contains(base + QStringLiteral(".dic"))) out << base;
    }
    return out;
}

}} // namespace Logic, MaliitKeyboard

#endif // MALIIT_KEYBOARD_HUNSPELLDICTIONARY_H
