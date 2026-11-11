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

#include "preswipedecoder.h"
#include "prelanguage.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace Pre {
namespace {

const int POINTS = 40;
const float TURN = 50.0f;
const int MAX_LETTERS = 32;
//! Shorter than this, in keys, is a tap that slipped, not a swipe.
const float MIN_LENGTH = 0.6f;
//! Words rarer than this are left out: swiping them is guesswork anyway.
const int MIN_SCORE = 5;
const float END_R2 = 1.45f * 1.45f;
//! How far, in keys, a letter may be from the path before it starts to cost.
const float FREE = 0.35f;
const float MISS_WEIGHT = 2.5f;
const float END_WEIGHT = 2.0f;
//! The same for a sharp turn in the path, from the nearest of the word's letters.
const float CORNER_FREE = 0.45f;
const float CORNER_WEIGHT = 4.0f;
const float LONG_WEIGHT = 1.0f;
//! How many of the cheap pass's best are lined up properly.
const int SHORTLIST = 250;
//! How far (in points) one path may run ahead of the other when they are lined up.
const int BAND = 8;
//! The share of the distance measured point for point; the rest is elastic.
const float STRAIGHT = 0.5f;
/*
 * log2 frequency to natural log, and a Gaussian on the distance: sigma 0.15
 * keys. These weights were tuned on simulated quick swipes over the 400
 * commonest words.
 */
const float FREQ_WEIGHT = 0.69f;
//! What following the word before is worth, at most: about a step and a half of frequency.
const float CONTEXT_BOOST = 1.0f;
const float LOC_WEIGHT = 1.0f / (2 * 0.15f * 0.15f);

inline float dist2(float ax, float ay, float bx, float by)
{
    const float dx = ax - bx, dy = ay - by;
    return dx * dx + dy * dy;
}

float pathLength(const float *x, const float *y, int n)
{
    float l = 0;
    for (int i = 1; i < n; ++i)
        l += std::sqrt(dist2(x[i - 1], y[i - 1], x[i], y[i]));
    return l;
}

//! The polyline ([x], [y], first [n] points) as [count] points evenly spaced along it.
void resample(const float *x, const float *y, int n, float *ox, float *oy, int count)
{
    const float total = pathLength(x, y, n);
    if (total == 0) {
        std::fill(ox, ox + count, x[0]);
        std::fill(oy, oy + count, y[0]);
        return;
    }
    const float step = total / (count - 1);
    ox[0] = x[0];
    oy[0] = y[0];
    int out = 1;
    float travelled = 0;   // along the path up to point i-1
    int i = 1;
    while (i < n && out < count) {
        const float seg = std::sqrt(dist2(x[i - 1], y[i - 1], x[i], y[i]));
        while (out < count && out * step <= travelled + seg) {
            const float t = seg == 0 ? 0 : (out * step - travelled) / seg;
            ox[out] = x[i - 1] + (x[i] - x[i - 1]) * t;
            oy[out] = y[i - 1] + (y[i] - y[i - 1]) * t;
            out++;
        }
        travelled += seg;
        i++;
    }
    while (out < count) {
        ox[out] = x[n - 1];
        oy[out] = y[n - 1];
        out++;
    }
}

//! The most keys a layout may have: a key is a byte in m_keys.
const int MAX_KEYS = 255;

int bucket(int first, int last) { return first * 256 + last; }

//! Keys within reach of a path's end, nearest first; the nearest one at least.
QVector<int> near(float x, float y, const float *kx, const float *ky, int keys)
{
    QVector<int> order(keys);
    for (int i = 0; i < keys; ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        return dist2(x, y, kx[a], ky[a]) < dist2(x, y, kx[b], ky[b]);
    });
    QVector<int> out;
    for (int i = 0; i < keys; ++i)
        if (i == 0 || dist2(x, y, kx[order[i]], ky[order[i]]) < END_R2)
            out << order[i];
    return out;
}

//! A Hebrew letter's final form is the same key as its other form.
QChar unfinal(QChar c)
{
    switch (c.unicode()) {
    case 0x05DA: return QChar(0x05DB);   // ך כ
    case 0x05DD: return QChar(0x05DE);   // ם מ
    case 0x05DF: return QChar(0x05E0);   // ן נ
    case 0x05E3: return QChar(0x05E4);   // ף פ
    case 0x05E5: return QChar(0x05E6);   // ץ צ
    default: return c;
    }
}

/*!
 * Where the path turns sharply (by more than TURN degrees over a few points), as
 * indexes into the resampled path. A quick finger rounds a corner, but it still
 * turns there.
 */
QVector<int> corners(const float *x, const float *y)
{
    QVector<int> out;
    float turn[POINTS] = {};
    const int k = 2;
    for (int i = k; i < POINTS - k; ++i) {
        const float ax = x[i] - x[i - k], ay = y[i] - y[i - k];
        const float bx = x[i + k] - x[i], by = y[i + k] - y[i];
        const double la = std::hypot(ax, ay), lb = std::hypot(bx, by);
        if (la < 0.15 || lb < 0.15) continue;
        const double cosv = std::max(-1.0, std::min(1.0, (ax * bx + ay * by) / (la * lb)));
        turn[i] = float(std::acos(cosv) * 180.0 / M_PI);
    }
    for (int i = k; i < POINTS - k; ++i) {
        if (turn[i] < TURN) continue;
        bool peak = true;
        for (int j = i - k; j < i; ++j) if (turn[j] > turn[i]) peak = false;
        for (int j = i + 1; j <= i + k; ++j) if (turn[j] >= turn[i]) peak = false;
        if (peak) out << i;
    }
    return out;
}

struct Candidate
{
    const void *w = nullptr;    // the Word
    int letters = 0;
    float score = 0;
    //! The word's ideal path, resampled; its point-for-point distance from the finger's.
    float wx[POINTS];
    float wy[POINTS];
    float straight = 0;
    //! The score before the distance counts, and the best it can be once it does.
    float base = 0;
    float bound = 0;
};

//! Letters only (and an apostrophe, as in "we're", or a mark), two letters at least.
bool swipeable(QStringView word)
{
    int letters = 0;
    for (QChar c : word) {
        if (c.isLetter()) letters++;
        else if (c != QLatin1Char('\'') && !c.isMark()) return false;
    }
    return letters >= 2;
}

//! A contraction can't be told from its bare form ("we'll", "well"): the bare one leads.
float scored(QStringView word, int score)
{
    return word.contains(QLatin1Char('\'')) ? score - 2.0f : float(score);
}

} // namespace

SwipeDecoder::SwipeDecoder(QVector<QPair<QString, int>> entries)
{
    entries.erase(std::remove_if(entries.begin(), entries.end(), [](const QPair<QString, int> &e) {
                      return e.second < MIN_SCORE || e.first.size() > 0xFFFF || !swipeable(e.first);
                  }), entries.end());
    // In order, each word once, at its higher score.
    std::sort(entries.begin(), entries.end(), [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
        return a.first < b.first || (a.first == b.first && a.second > b.second);
    });
    qsizetype chars = 0;
    for (const auto &e : entries) chars += e.first.size();
    m_text.reserve(chars);
    m_words.reserve(entries.size());
    for (int i = 0; i < entries.size(); ++i) {
        if (i > 0 && entries[i].first == entries[i - 1].first) continue;
        append(entries[i].first, scored(entries[i].first, entries[i].second), false);
    }
    m_sorted = int(m_words.size());
}

void SwipeDecoder::append(const QString &word, float score, bool learned)
{
    Word w;
    w.at = quint32(m_text.size());
    w.length = quint16(word.size());
    w.learned = learned;
    w.score = score;
    m_text += word;
    m_words.append(w);
    if (!m_alphabet.isEmpty()) index(int(m_words.size()) - 1);
}

int SwipeDecoder::find(QStringView word) const
{
    int lo = 0, hi = m_sorted;
    while (lo < hi) {
        const int mid = (lo + hi) / 2;
        const int c = text(m_words[mid]).compare(word);
        if (c == 0) return mid;
        if (c < 0) lo = mid + 1;
        else hi = mid;
    }
    for (int i = m_sorted; i < m_words.size(); ++i)
        if (text(m_words[i]) == word) return i;
    return -1;
}

void SwipeDecoder::add(const QString &word, int score)
{
    QMutexLocker l(&m_lock);
    if (score < MIN_SCORE || word.size() > 0xFFFF || !swipeable(word)) return;
    const float s = scored(word, score);
    const int i = find(word);
    if (i >= 0) {
        // Back, if it was taken out, and at least as likely as now.
        Word &w = m_words[i];
        if (w.gone || s > w.score) w.score = s;
        w.gone = false;
        w.learned = true;
        return;
    }
    append(word, s, true);
}

int SwipeDecoder::size() const
{
    QMutexLocker l(&m_lock);
    int n = 0;
    for (const Word &w : m_words)
        if (!w.gone && (m_alphabet.isEmpty() || w.keys > 0)) n++;
    return n;
}

/*
 * The keys a word is swiped over. A letter with a key of its own is that key
 * ("й" is not "и" on a Russian keyboard). One without, an accented letter or
 * a ligature, is the key of what it is made from: "être" is swiped e-t-r-e,
 * "straße" s-t-r-a-s-e, Arabic's "أ" on alif, Hebrew's final "ם" on mem.
 * Marks (Arabic's vowel signs) and apostrophes are no keys at all.
 */
QByteArray SwipeDecoder::spell(QStringView w) const
{
    QByteArray out;
    auto put = [&](int k) {
        if (out.isEmpty() || out.at(out.size() - 1) != char(k)) out.append(char(k));
    };
    for (QChar c : w) {
        c = c.toLower();
        if (c == QLatin1Char('\'') || c.isMark()) continue;
        const int k = int(m_alphabet.indexOf(c));
        if (k >= 0) { put(k); continue; }
        const QString folded = Language::fold(QString(unfinal(c)));
        for (QChar f : folded) {
            const int fk = int(m_alphabet.indexOf(f));
            if (fk < 0) return QByteArray();
            put(fk);
        }
    }
    return out.size() >= 2 && out.size() <= MAX_LETTERS ? out : QByteArray();
}

void SwipeDecoder::index(int i)
{
    Word &w = m_words[i];
    const QByteArray keys = spell(text(w));
    w.keys = quint8(keys.size());
    if (keys.isEmpty()) return;
    w.keysAt = quint32(m_keys.size());
    m_keys += keys;
    m_buckets[bucket(uchar(keys.front()), uchar(keys.back()))].append(i);
}

void SwipeDecoder::useKeys(const QString &alphabet)
{
    if (alphabet == m_alphabet) return;
    m_alphabet = alphabet;
    m_buckets.clear();
    m_keys.clear();
    for (int i = 0; i < m_words.size(); ++i) index(i);
}

void SwipeDecoder::remove(const QString &word)
{
    QMutexLocker l(&m_lock);
    for (Word &w : m_words)
        if (w.learned && text(w).compare(word, Qt::CaseInsensitive) == 0) w.gone = true;
}

void SwipeDecoder::forget(const QString &word)
{
    QMutexLocker l(&m_lock);
    for (Word &w : m_words)
        if (text(w).compare(word, Qt::CaseInsensitive) == 0) w.gone = true;
}

QStringList SwipeDecoder::decode(const QVector<float> &px, const QVector<float> &py, const Layout &layout, int max,
                                 const QStringList &likely, const std::function<bool()> &stale)
{
    QMutexLocker locker(&m_lock);
    const int n = px.size();
    if (n < 2 || !layout.isValid() || layout.keys.size() > MAX_KEYS)
        return {};
    useKeys(layout.keys);
    const int keys = int(layout.keys.size());
    // Into key units.
    QVector<float> ux(n), uy(n);
    for (int i = 0; i < n; ++i) {
        ux[i] = px[i] / layout.keyWidth;
        uy[i] = py[i] / layout.keyHeight;
    }
    QVector<float> kxv(keys), kyv(keys);
    float *kx = kxv.data(), *ky = kyv.data();
    for (int i = 0; i < keys; ++i) {
        kx[i] = layout.x[i] / layout.keyWidth;
        ky[i] = layout.y[i] / layout.keyHeight;
    }
    const float length = pathLength(ux.constData(), uy.constData(), n);
    if (length < MIN_LENGTH)
        return {};
    float rx[POINTS], ry[POINTS];
    resample(ux.constData(), uy.constData(), n, rx, ry, POINTS);

    // What a letter costs depends only on its key: worked out once for every key.
    // How far past FREE each key is from the nearest point the finger passed,
    // and how far past CORNER_FREE each sharp turn is from each key.
    QVector<float> keyMiss(keys);
    for (int k = 0; k < keys; ++k) {
        float best = FLT_MAX;
        for (int j = 0; j < POINTS; ++j) {
            const float d = dist2(rx[j], ry[j], kx[k], ky[k]);
            if (d < best) best = d;
        }
        const float over = std::sqrt(best) - FREE;
        keyMiss[k] = over > 0 ? over * over : 0;
    }
    const QVector<int> turns = corners(rx, ry);
    QVector<QVector<float>> turnAt(turns.size(), QVector<float>(keys));
    for (int c = 0; c < turns.size(); ++c) {
        for (int k = 0; k < keys; ++k) {
            const float over = std::sqrt(dist2(rx[turns[c]], ry[turns[c]], kx[k], ky[k])) - CORNER_FREE;
            turnAt[c][k] = over > 0 ? over * over : 0;
        }
    }
    const QVector<int> starts = near(ux[0], uy[0], kx, ky, keys);
    const QVector<int> ends = near(ux[n - 1], uy[n - 1], kx, ky, keys);
    float tx[MAX_LETTERS], ty[MAX_LETTERS];
    std::vector<Candidate> pool;
    for (int s : starts) {
        for (int e : ends) {
            if (stale()) return {};
            auto it = m_buckets.constFind(bucket(s, e));
            if (it == m_buckets.cend()) continue;
            for (int wi : it.value()) {
                const Word &w = m_words[wi];
                if (w.gone) continue;
                const int m = w.keys;
                const uchar *L = keysOf(w);
                for (int i = 0; i < m; ++i) { tx[i] = kx[L[i]]; ty[i] = ky[L[i]]; }
                const float ideal = pathLength(tx, ty, m);
                // A quick finger cuts corners, so its path may be well short of
                // the word's; but not this short, nor wander this far.
                if (ideal > length * 1.6f + 1.2f || length > ideal * 2.0f + 2.5f) continue;
                // How far each letter is from where the finger went.
                float miss = 0;
                for (int i = 1; i < m - 1; ++i) miss += keyMiss[L[i]];
                const float endMiss = dist2(rx[0], ry[0], tx[0], ty[0]) +
                                      dist2(rx[POINTS - 1], ry[POINTS - 1], tx[m - 1], ty[m - 1]);
                // Every sharp turn the finger made should be at one of the word's letters.
                float turn = 0;
                for (const QVector<float> &t : turnAt) {
                    float best = FLT_MAX;
                    for (int i = 0; i < m; ++i) { const float d = t[L[i]]; if (d < best) best = d; }
                    turn += best;
                }
                const float cost = MISS_WEIGHT * miss + END_WEIGHT * endMiss + CORNER_WEIGHT * turn;
                Candidate cand;
                cand.w = &w;
                cand.letters = m;
                cand.score = w.score * FREQ_WEIGHT - cost;
                pool.push_back(cand);
            }
        }
    }
    if (pool.empty())
        return {};
    std::stable_sort(pool.begin(), pool.end(), [](const Candidate &a, const Candidate &b) { return a.score > b.score; });
    const int shortN = std::min<int>(SHORTLIST, int(pool.size()));
    // Each word's best possible score, from its point-for-point distance alone:
    // the elastic one is never less than nothing, so this is as well as the word can do.
    for (int idx = 0; idx < shortN; ++idx) {
        if (idx % 64 == 0 && stale()) return {};
        Candidate &c = pool[idx];
        const Word &w = *static_cast<const Word *>(c.w);
        const int m = c.letters;
        const uchar *L = keysOf(w);
        for (int i = 0; i < m; ++i) { tx[i] = kx[L[i]]; ty[i] = ky[L[i]]; }
        resample(tx, ty, m, c.wx, c.wy, POINTS);
        float straight = 0;
        for (int i = 0; i < POINTS; ++i) straight += std::sqrt(dist2(rx[i], ry[i], c.wx[i], c.wy[i]));
        c.straight = straight / POINTS;
        // A path well longer than the word's went somewhere the word doesn't ("there", not "the").
        const float longer = std::max(0.0f, length - pathLength(tx, ty, m) * 1.1f - 0.4f);
        float score = c.score - LONG_WEIGHT * longer * longer;
        // "were", not "we're", when the path can't say which.
        if (text(w).contains(QLatin1Char('\''))) {
            QString b = text(w).toString().toLower();
            b.remove(QLatin1Char('\''));
            if (find(b) >= 0) score -= 2.5f;
        }
        int rank = -1;
        for (int i = 0; i < likely.size(); ++i)
            if (text(w).compare(likely[i], Qt::CaseInsensitive) == 0) { rank = i; break; }
        if (rank >= 0) score += CONTEXT_BOOST * (1.0f - rank / (likely.size() + 1.0f));
        c.base = score;
        const float atBest = STRAIGHT * c.straight;
        c.bound = score - atBest * atBest * LOC_WEIGHT;
    }
    // Then lined up elastically, likeliest first, till no word left could make the list.
    std::stable_sort(pool.begin(), pool.begin() + shortN,
                     [](const Candidate &a, const Candidate &b) { return a.bound > b.bound; });
    QList<QPair<QString, float>> best;
    auto byScore = [](const QPair<QString, float> &a, const QPair<QString, float> &b) { return a.second > b.second; };
    for (int idx = 0; idx < shortN; ++idx) {
        if (idx % 32 == 0 && stale()) return {};
        const Candidate &c = pool[idx];
        if (best.size() >= max && c.bound < best[max - 1].second) break;
        const Word &w = *static_cast<const Word *>(c.w);
        const float warped = warp(rx, ry, c.wx, c.wy);
        // Half point for point, as the word's shape, half lined up elastically.
        const float loc = STRAIGHT * c.straight + (1 - STRAIGHT) * warped;
        const float score = c.base - loc * loc * LOC_WEIGHT;
        const QString word = text(w).toString();
        bool dup = false;
        for (const auto &b : best) if (b.first == word) { dup = true; break; }
        if (dup) continue;
        best.append({ word, score });
        std::stable_sort(best.begin(), best.end(), byScore);
    }
    std::stable_sort(best.begin(), best.end(), byScore);
    QStringList out;
    for (const auto &b : best) {
        if (out.contains(b.first)) continue;
        out << b.first;
        if (out.size() >= max) break;
    }
    return out;
}

float SwipeDecoder::warp(const float *ax, const float *ay, const float *bx, const float *by)
{
    const float inf = FLT_MAX / 4;
    float prev[POINTS], row[POINTS];
    for (int i = 0; i < POINTS; ++i) {
        const int lo = std::max(0, i - BAND), hi = std::min(POINTS - 1, i + BAND);
        for (int j = 0; j < POINTS; ++j) {
            if (j < lo || j > hi) { row[j] = inf; continue; }
            const float d = std::sqrt(dist2(ax[i], ay[i], bx[j], by[j]));
            float from;
            if (i == 0 && j == 0) from = 0;
            else if (i == 0) from = row[j - 1];
            else if (j == 0) from = prev[j];
            else from = std::min(prev[j - 1], std::min(prev[j], row[j - 1]));
            row[j] = from + d;
        }
        std::copy(row, row + POINTS, prev);
    }
    return prev[POINTS - 1] / POINTS;
}

} // namespace Pre
