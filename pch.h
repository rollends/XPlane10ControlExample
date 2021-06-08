// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

// Windows Build (On)
#define IBM 1

// Apple Build (Off)
#define APL 0

// Linux Build (Off)
#define LIN 0

// We aren't Building X-Plane
#define XPLM 0

// Support XPlane 10 & Newer
#define XPLM200
#define XPLM210
#include "XPLM/XPLMPlugin.h"
#include "XPLM/XPLMDataAccess.h"
#include "XPLM/XPLMProcessing.h"

#endif //PCH_H
