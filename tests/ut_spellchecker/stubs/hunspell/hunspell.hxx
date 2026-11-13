// Hunspell as SpellChecker uses it, for a test without hunspell: it knows the
// words it was given with add(), and remove() takes them back out.
#ifndef UT_SPELLCHECKER_HUNSPELL_STUB_H
#define UT_SPELLCHECKER_HUNSPELL_STUB_H

#include <set>
#include <string>

class Hunspell
{
public:
    Hunspell(const char *, const char *) {}
    bool spell(const std::string &word) const { return words.count(word) != 0; }
    int suggest(char ***list, const char *) { *list = nullptr; return 0; }
    void free_list(char ***, int) {}
    int add(const std::string &word) { words.insert(word); return 0; }
    int remove(const std::string &word) { words.erase(word); return 0; }
    const char *get_dic_encoding() const { return "UTF-8"; }

    std::set<std::string> words;
};

#endif
