#ifndef WESTERNLANGUAGESPLUGIN_H
#define WESTERNLANGUAGESPLUGIN_H

#include "languageplugininterface.h"
#include "candidatescallback.h"
#include "westernlanguagefeatures.h"
#include "spellchecker.h"

#include <presage.h>

#include <QObject>

class WesternLanguageFeatures;
class CandidatesCallback;

class WesternLanguagesPlugin : /*public QObject,*/ public LanguagePluginInterface
{
    //Q_OBJECT
    Q_INTERFACES(LanguagePluginInterface)
    Q_DISABLE_COPY(WesternLanguagesPlugin)

public:
    explicit WesternLanguagesPlugin(/*QObject *parent = 0*/);
    ~WesternLanguagesPlugin() override;

    void parse(const QString& surroundingLeft, const QString& preedit) override;
    QStringList getWordCandidates() override;
    void wordCandidateSelected(QString word) override;
    AbstractLanguageFeatures* languageFeature() override;

    //! spell checker
    bool spellCheckerEnabled() override;
    bool setSpellCheckerEnabled(bool enabled) override;
    bool spell(const QString& word) override;
    QStringList spellCheckerSuggest(const QString& word, int limit) override;
    void addToSpellCheckerUserWordList(const QString& word) override;
    bool setSpellCheckerLanguage(const QString& languageId) override;

signals:

public slots:

protected:
    void _useDatabase(const QString& locale);
private:
    std::string m_candidatesContext;
    CandidatesCallback m_presageCandidates;
    Presage m_presage;
    WesternLanguageFeatures* m_languageFeatures;

    SpellChecker m_spellChecker;
};

#endif // WESTERNLANGUAGESPLUGIN_H
