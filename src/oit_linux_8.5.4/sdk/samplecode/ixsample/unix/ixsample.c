/*
|   Image Export sample application
|
|   Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.
|
|   You have a royalty-free right to use, modify, reproduce and
|   distribute the Sample Applications (and/or any modified version)
|   in any way you find useful, provided that you agree that Oracle
|   has no warranty obligations or liability for any Sample Application
|   files which are modified.
*/

#ifdef WIN32                /* Defined by my Win32 compiler */
#define     WINDOWS         /* Required for Windows */
#include    <windows.h>
#else
#ifndef UNIX
#define     UNIX            /* Required for UNIX */
#endif
#endif

/*
   Oracle does version tracking of the Outside In sample applications.
   OIT_VERSTRING is defined by internal build scripts so you can delete
   the following line at your leisure as it performs no real function.
*/
#ifdef OIT_VERSTRING
char oitsamplever[32] = OIT_VERSTRING;
#endif

#include    <stdio.h>

#include    "sccex.h"       /* Required */


#ifdef UNIX
#define PATH_TYPE   IOTYPE_UNIXPATH
#else
#define PATH_TYPE   IOTYPE_ANSIPATH
#endif


/* To demonstrate callbacks, we will keep count of how many output files
 * we create.
 */
VTDWORD gdwFileCount = 0;


/* Function prototypes */
SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData, VTDWORD dwCommandID, VTLPVOID pCommandData);

#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

int main(int argc, char *argv[])
{

    SCCERR      seResult;       /* Error code returned. */
    VTHDOC      hDoc;           /* Input doc handle returned by DAOpenDocument(). */
    VTHEXPORT   hExport;        /* Handle to the export returned by EXOpenExport(). */
    VTDWORD     dwFIFlags;      /* Used in setting the SCCOPT_FIFLAGS option. */
    VTCHAR      szError[256];   /* Error string buffer. */

    if (argc != 3)
    {
        fprintf(stderr, "%s - Converts the input file to a TIFF graphic.\n", argv[0]);
        fprintf(stderr, "Usage:\t%s InputFile OutputFile \n", argv[0]);
        fprintf(stderr, "Where:\t\"InputFile\" is the name of the file to be converted.\n");
        fprintf(stderr, "\t\"OutputFile\" is the name of the output converted file.\n");
        return (-1);
    }

    /* Step 1:  Initialize the Data Access module. (required) */
    seResult = DAInitEx(SCCOPT_INIT_NOTHREADS, OI_INIT_DEFAULT);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DAInitEx() failed: %s (0x%04X)\n", szError, seResult);
        return (seResult);
    }


    /* Step 2: Set options. (optional, no pun intended)
     *
     * Here is an example of how to set an option.  Note that we have to 
     * pass a pointer to dwFIFlags rather than just specifying SCCUT_FI_NORMAL
     * as a parameter to DASetOption().
     */
    dwFIFlags = SCCUT_FI_NORMAL;
    if ((seResult = DASetOption((VTHDOC)NULL, SCCOPT_FIFLAGS, (VTLPVOID)&dwFIFlags, sizeof(VTDWORD))) != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DASetOption() failed: %s (%04X)\n", szError, seResult);

        /* From this point forward, if there are problems running a module, we 
         * have to close all modules that initialized successfully.  This is
         * required to prevent various types of leaks.
         */
        DADeInit();
        return (seResult);
    }


    /* Step 3: Open the document to be converted (given here by argv[1]). (required) */
    seResult = DAOpenDocument(&hDoc, PATH_TYPE, (VTLPVOID)argv[1], 0);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DAOpenDocument() failed: %s (0x%04X)\n", szError, seResult);
        DADeInit();
        return (seResult);
    }


    /* Step 4: Open the export. (required)
     * 
     * For Image Export, the second parameter of EXOpenExport()
     * (dwOutputID) may be any of the FIs listed in the product manual.
     * In this example we will use FI_TIFF. 
     */
    seResult = EXOpenExport(hDoc, FI_TIFF, PATH_TYPE, argv[2], 0, 0,
                            (EXCALLBACKPROC)ExportOemCallback, 0, &hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "EXOpenExport() failed: %s (0x%04X)\n", szError, seResult);
        DACloseDocument(hDoc);
        DADeInit();
        return (seResult);
    }

    
    /* Step 5: Do the actual conversion. (required) */
    seResult = EXRunExport(hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "EXRunExport() failed: %s (0x%04X)\n", szError, seResult);
        EXCloseExport(hExport);
        DACloseDocument(hDoc);
        DADeInit();
        return (seResult);
    }

    /* Print the results of the demonstration of how to write a
     * callback in the ExportOemCallback() routine below.  The demo
     * is just a count of the number of files created.
     */
    fprintf(stdout, "Export successful: %d output file(s) created.\n", gdwFileCount);
 

    /* Step 6: Close the export. (required) */
    seResult = EXCloseExport(hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "EXCloseExport() failed: %s (0x%04X)\n", szError, seResult);
        DACloseDocument(hDoc);
        DADeInit();
        return (seResult);
    }


    /* Step 7: Close the input document. (required) */
    seResult = DACloseDocument(hDoc);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DACloseDocument() failed: %s (0x%04X)\n", szError, seResult);
        DADeInit();
        return (seResult);
    }


    /* Step 8: Shut down the Data Access module. (required) */
    seResult = DADeInit();
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DADeInit() failed: %s (0x%04X)\n", szError, seResult);
        return (seResult);
    }

    return (0);
}




SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData,
                         VTDWORD dwCommandID, VTLPVOID pCommandData)
{
    SCCERR seResult = SCCERR_NOTHANDLED;
    EXFILEIOCALLBACKDATA *pNewFileInfo;
    EXURLFILEIOCALLBACKDATA *pExportData;
    UNUSED(hExport);
    UNUSED(dwCallbackData);

    /* This is a simple callback routine just to show how they are
     * coded.  The callback routine should always return
     * SCCERR_NOTHANDLED unless the user is returning data to the
     * conversion.
     */

    switch (dwCommandID)
    {
        case EX_CALLBACK_ID_NEWFILEINFO:

            /* Count files that are created by the conversion. */
            gdwFileCount++;

            /* Print the name of each output file generated. */
            pNewFileInfo = (EXFILEIOCALLBACKDATA *)pCommandData;
            pExportData  = (EXURLFILEIOCALLBACKDATA *)pNewFileInfo->pExportData;
            printf("Creating file: \"%s\"\n", pExportData->szURLString);

            break;

        case EX_CALLBACK_ID_CREATENEWFILE:
        case EX_CALLBACK_ID_OEMOUTPUT:
            /* These are just two of many callbacks that are not examined by
             * this routine.  The user is not required to list ignored callbacks.
             */
        default:
            break;
    }
    
    return seResult;
}
