#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace MaliitKeyboard {
class Key;

namespace CoreUtils {
const QString &pluginLanguageDirectory();
const QString &pluginDataDirectory();
const QString &maliitKeyboardDataDirectory();
const QString &maliitKeyboardStyleProfilesDirectory();
QString idFromKey(const Key &key);

//! Id Maliit uses for the Return key override. The application sets its label
//! through this, which is the equivalent of the reference keyboard's
//! PalmIME::EditorState::enterKeyLabel.
const char *actionKeyId();
}} // namespace MaliitKeyboard, CoreUtils

#endif // UTILS_H
