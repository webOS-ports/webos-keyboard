TEMPLATE = subdirs
CONFIG += ordered

# One binary per unit under test. Keep this list alphabetical.
SUBDIRS = \
    ut_correction \
    ut_hardwarekeyboard \
    ut_keyrepeat \
    ut_klipy \
    ut_panelstate \
    ut_prekeyboard \
    ut_symbolpage \
    ut_t9multitap \

# Dictation listens through PulseAudio; without its headers it isn't built.
packagesExist(libpulse-simple): SUBDIRS += ut_dictation

# "make check" recurses into every subdirectory.
QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

OTHER_FILES += tests.pri README.md tools/vkbd.py
