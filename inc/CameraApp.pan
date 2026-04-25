/*
 * Copyright © 2008 Nokia Corporation.
 */

#ifndef __CAMERAAPP_PAN__
#define __CAMERAAPP_PAN__

#include <e32std.h>

inline void Panic(TInt aPanicCode)
    {
    _LIT(KApplicationName, "ARDemo");
    User::Panic(KApplicationName, aPanicCode);
    }

inline void PanicIfError(TInt aPanicCode)
    {
    if (aPanicCode != KErrNone)
        {
        Panic(aPanicCode);
        }
    }

#endif // __CAMERAAPP_PAN__

