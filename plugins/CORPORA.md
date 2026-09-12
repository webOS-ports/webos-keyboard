# Word-prediction corpora

Each language plugin ships one plain-text corpus next to its sources. At build
time `<lang>/src/src.pro` runs presage's `text2ngram` over it three times (1-,
2- and 3-grams) to produce `database_<lang>.db`, which is installed to
`/usr/share/maliit/plugins/org/luneos/lib/<lang>/`.

`WesternLanguagesPlugin::_useDatabase()` points presage's
`DefaultSmoothedNgramPredictor.DBFILENAME` at that file on every language
switch, so these databases — not the demo ones presage itself ships in
`/usr/share/presage/` — are what actually drives prediction.

## Provenance

| Languages | Source | Licence |
|---|---|---|
| de, en, es, fr, it, pl, pt | Project Gutenberg, one public-domain novel each, kept verbatim including the PG header | Public domain |
| ar, cs, da, fi, he, hu, nl, no, ru, sv, uk | [Tatoeba](https://tatoeba.org) per-language sentence exports | CC BY 2.0 FR |

The Tatoeba set replaced nine 0-byte `free_ebook.txt` placeholders inherited
from ubuntu-keyboard, which had been producing empty 28 KB databases — those
languages had no prediction at all. Norwegian and Ukrainian were added later,
alongside their keyboard layouts, and were built the same way.

Short everyday sentences also model phone typing considerably better than a
19th-century novel does; compare what the two sources predict after "I":
Tatoeba gives *have / am / will / know / can*, Don Quixote does not.

## Regenerating the Tatoeba corpora

Sources are `https://downloads.tatoeba.org/exports/per_language/<iso639-3>/<iso639-3>_sentences.tsv.bz2`
(`ara ces dan fin heb hun nld nob rus swe ukr`). From the TSV's third column, take
sentences in file order until 500 KB, dropping exact duplicates — 500 KB keeps
each database around 6 MB, in line with the 5.5 MB English one.

Two normalisations are applied that `text2ngram` will not do for you:

- **Lowercase in Unicode, not ASCII.** `text2ngram -l` calls `tolower()` on
  bytes, so capitalised non-ASCII forms end up in separate, unreachable rows —
  `database_fr.db` has `à`=4708 alongside `À`=214, and presage's
  `ContextTracker` lowercases queries before lookup, so those 214 can never be
  matched. The Gutenberg-derived databases still have this defect.
- **Fold Unicode punctuation to ASCII.** presage's tokenizer splits on a
  compiled-in ASCII-only separator set, so anything else stays glued to the
  word: the Arabic question mark was leaving `الكتاب؟` as a 1-gram distinct
  from `الكتاب`. Curly quotes, en/em dashes, ellipsis and the Arabic comma,
  semicolon and full stop are affected the same way.
