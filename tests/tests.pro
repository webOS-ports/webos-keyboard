TEMPLATE = subdirs
CONFIG += ordered

# One binary per unit under test. Keep this list alphabetical.
SUBDIRS = \
    ut_hardwarekeyboard \

# "make check" recurses into every subdirectory.
QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

OTHER_FILES += tests.pri README.md
