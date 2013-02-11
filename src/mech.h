/*
 * mech.h
 *
 * Master include file for Mech Tech Demo.
 * All other files should include this one.
 */

#ifndef __mech_h
#define __mech_h

/* We expect to use Crystal Space everywhere. */
#include <cssysdef.h>

/* MSVC users do not run configure, so use special MSVC configuration file. */
#if defined(CS_WIN32_CSCONFIG)
#include "config-msvc.h"
#else
#include "config.h"
#endif

/* Insert additional project-specific declarations here. */

#endif // __mech_h
