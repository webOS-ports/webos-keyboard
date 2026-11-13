// What SpellChecker uses of luna-service2, for a test without the bus: each
// call is recorded (ut_spellchecker.cpp), and a db8 find answered at once.
#ifndef UT_SPELLCHECKER_LUNASERVICE_STUB_H
#define UT_SPELLCHECKER_LUNASERVICE_STUB_H

#include <glib.h>

typedef struct LSHandle LSHandle;
typedef struct LSMessage LSMessage;
typedef unsigned long LSMessageToken;

typedef struct LSError {
    int error_code;
    char *message;
} LSError;

typedef bool (*LSFilterFunc)(LSHandle *sh, LSMessage *reply, void *ctx);

bool LSErrorInit(LSError *error);
void LSErrorFree(LSError *error);
bool LSRegister(const char *name, LSHandle **sh, LSError *error);
bool LSUnregister(LSHandle *sh, LSError *error);
bool LSGmainAttach(LSHandle *sh, GMainLoop *loop, LSError *error);
bool LSCall(LSHandle *sh, const char *uri, const char *payload, LSFilterFunc callback, void *ctx,
            LSMessageToken *token, LSError *error);
const char *LSMessageGetPayload(LSMessage *message);

#endif
