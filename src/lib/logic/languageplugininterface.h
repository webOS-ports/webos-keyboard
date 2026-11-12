#ifndef LANGUAGEPLUGININTERFACE_H
#define LANGUAGEPLUGININTERFACE_H

#include <QString>
#include <QStringList>
#include <QObject>
#include <presage.h>

class AbstractLanguageFeatures;

class LanguagePluginInterface
{
public:
    virtual ~LanguagePluginInterface() {}

    virtual void parse(const QString& surroundingLeft, const QString& preedit) = 0;
    virtual QStringList getWordCandidates() = 0;
    virtual void wordCandidateSelected(QString word) = 0;

    virtual AbstractLanguageFeatures* languageFeature() = 0;

    //! spell checker
    virtual bool spellCheckerEnabled() = 0;
    virtual bool setSpellCheckerEnabled(bool enabled) = 0;
    virtual bool spell(const QString& word) = 0;
    virtual QStringList spellCheckerSuggest(const QString& word, int limit) = 0;
    virtual void addToSpellCheckerUserWordList(const QString& word) = 0;
    virtual bool setSpellCheckerLanguage(const QString& languageId) = 0;
    //! Takes [word] out of the owner's dictionary; a plugin without one has nothing to do.
    //! Last, so the slots before it keep their places.
    virtual void removeFromSpellCheckerUserWordList(const QString& word) { Q_UNUSED(word); }
    //! The words in the owner's dictionary, as they were saved (their capitals kept).
    virtual QStringList spellCheckerUserWords() { return QStringList(); }
};

#define LanguagePluginInterface_iid "org.webosports.LuneOSKeyboard.LanguagePluginInterface"

Q_DECLARE_INTERFACE(LanguagePluginInterface, LanguagePluginInterface_iid)

#endif // LANGUAGEPLUGININTERFACE_H

