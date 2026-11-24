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

#include "preemoticons.h"

#include <QHash>
#include <QStringList>

namespace Pre {
namespace Emoticons {
namespace {

QHash<QString, QString> build()
{
    QHash<QString, QString> m;
    auto face = [&m](const char *emoji, std::initializer_list<const char *> texts) {
        for (const char *t : texts) m.insert(QString::fromUtf8(t), QString::fromUtf8(emoji));
    };
    face("🙂", { ":)", ":-)", "=)", ":]", "(:" });
    face("🙁", { ":(", ":-(", "=(", ":[" });
    face("😃", { ":D", ":-D", "=D" });
    face("😆", { "XD", "xD" });
    face("😉", { ";)", ";-)", ";D", ";]" });
    face("😛", { ":P", ":-P", ":p", ":-p", "=P", ":b" });
    face("😜", { ";P", ";-P", ";p", ";-p" });
    face("😮", { ":O", ":-O", ":o", ":-o" });
    face("😢", { ":'(", ":'-(" });
    face("😂", { ":')", ":'D" });
    face("😕", { ":/", ":-/", ":\\", ":-\\" });
    face("😐", { ":|", ":-|" });
    face("😘", { ":*", ":-*" });
    face("😖", { ":S", ":s" });
    face("😠", { ">:(", ">:-(" });
    face("😈", { ">:)", ">:-)", ">:D" });
    face("😇", { "O:)", "O:-)", "0:)" });
    face("😎", { "B-)", "8-)" });
    face("😳", { ":$", "o_O", "O_o", "o.O", "O.o" });
    face("😡", { ":@" });
    face("😶", { ":X", ":x", ":-X", ":-x" });
    face("😞", { ":c" });
    face("😺", { ":3" });
    face("😊", { "^_^", "^^", "^.^" });
    face("😑", { "-_-" });
    face("😭", { "T_T", ";_;" });
    face("❤️", { "<3" });
    face("💔", { "</3" });
    return m;
}

const QHash<QString, QString> &faces()
{
    static const QHash<QString, QString> f = build();
    return f;
}

} // namespace

int longest()
{
    static const int n = [] {
        int l = 0;
        for (auto it = faces().cbegin(); it != faces().cend(); ++it) l = qMax(l, int(it.key().size()));
        return l;
    }();
    return n;
}

QString emojiFor(const QString &face) { return faces().value(face); }

Face at(const QString &before)
{
    const bool spaced = before.endsWith(QLatin1Char(' '));
    const QString b = spaced ? before.left(before.size() - 1) : before;
    for (int n = qMin(longest(), int(b.size())); n >= 2; --n) {
        const QString text = b.right(n);
        const QString emoji = faces().value(text);
        if (emoji.isEmpty()) continue;
        const int p = int(b.size()) - n - 1;
        const QChar prev = p >= 0 ? b.at(p) : QChar();
        const bool alone = p < 0 || prev.isSpace() || QStringLiteral("(\"[{“").contains(prev);
        const bool glued = p >= 0 && prev.isLetter() && (text.at(0) == QLatin1Char(':') || text.at(0) == QLatin1Char(';'))
                           && QStringLiteral(")(][").contains(text.back());
        if (alone || glued) return Face{ n + (spaced ? 1 : 0), emoji, QString() };
    }
    return Face();
}

} // namespace Emoticons
} // namespace Pre
