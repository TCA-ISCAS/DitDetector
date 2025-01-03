   /*
    |	Copyright (c) Oracle Corporation 2001 - 2007.  All Rights Reserved.
    |
    |	This file contains material copyrighted by Oracle Corp. and is the
    |	confidential unpublished property of Oracle Corp., and may not be
    |	copied in whole or part, or disclosed to any third party, without the
    |	written prior consent of Oracle Corp.
    |
    |  SCC Viewer Technology - Source file
    |
    |  Code:	scclo_u.h (included in scclo_u.c)
    |  Module:	SCCLO
    |  Developer:	Randal Chao				   
    |  Environment:	UNIX
    |
    |  Revising History:
    |  02/08/95     :  Initial Development
    |  08/19/96  MZ :  Adapted to 5.0 Technology
    |  12/16/98 HSL :  Moved the strings a while ago to scclostr_u.c; moved the string table
    |				:  structure here from scclo.h
    */

#if defined(_DARWIN_SOURCE)
#import <Foundation/Foundation.h>
#endif /* _DARWIN_SOURCE */

#include <scctype.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct tagSCCSTRINGTABLE
{
    VTDWORD   dwId;
#if defined(_DARWIN_SOURCE)
    const VTCHAR   *key;
    const VTCHAR   *defaultString;
#else    
    const VTCHAR    *szString;
#endif /* _DARWIN_SOURCE */    
} SCCSTRINGTABLE, *PSCCSTRINGTABLE;

#if defined(_DARWIN_SOURCE)
NSBundle   *LOGetBundle();
#endif /* _DARWIN_SOURCE */

#if defined(__cplusplus)
}
#endif
