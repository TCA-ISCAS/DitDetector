/*
|   Image Export
|   Annotation API Sample Application
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
#include    <stdlib.h>
#include    <string.h>
#ifdef UNIX
#include    <unistd.h>
#endif

#include    "sccca.h"       /* Required */
#include    "sccex.h"       /* Required */


#ifdef UNIX
#define PATH_TYPE   IOTYPE_UNIXPATH
#else
#define PATH_TYPE   IOTYPE_ANSIPATH
#endif


#ifndef TRUE
#define TRUE    (1 == 1)
#endif

#ifndef FALSE
#define FALSE   (1 == 0)
#endif

#define MAXDABUFSIZE 0x1000

VTDWORD gdwFileCount;               /* Global count of files created. */
VTDWORD gdwGraphicExportErrors;     /* Global count of graphics export failures */

#define MAX_SEARCH_STRING_LEN   1024
#define MAX_FONT_NAME_LEN        128

typedef struct COMMANDOPTStag
{
    VTCHAR        szInputFile[VT_MAX_FILEPATH];
    VTCHAR        szOutputFile[VT_MAX_FILEPATH];
    VTCHAR        szJSONFile[VT_MAX_FILEPATH];
    VTCHAR        szSearchString[MAX_SEARCH_STRING_LEN];
    VTCHAR        szRedactionLabelText[EXANNO_MAXLABEL];
    VTCHAR        szRedactionLabelFontName[MAX_FONT_NAME_LEN];
    VTDWORD       dwRedactionLabelFontSize;
    VTBOOL        bRedaction;
    VTBOOL        bUseGD;
    VTBOOL        bMultiPageTiff;
    SCCVWCOLORREF sColor;
} COMMANDOPTS, *PCOMMANDOPTS;

/* Function prototypes */
SCCERR ExportOemCallback(
            VTHEXPORT  hExport, 
            VTSYSPARAM dwCallbackData, 
            VTDWORD    dwCommandID, 
            VTLPVOID   pCommandData);
VTLPSTR FindMatches(
            VTLPSTR   pSearchBuffer, 
            VTLPSTR   pSearchText, 
            VTLPDWORD pCharCount, 
            VTLPDWORD pMatchLen);
SCCERR InitializeExport(
            VTLPHDOC    phDoc, 
            VTLPHEXPORT phExport,
            PCOMMANDOPTS pCommandOpts);
SCCERR DoConversion(
            VTHDOC    hDoc, 
            VTHEXPORT hExport);
SCCERR ExportOemCallback(
            VTHEXPORT  hExport, 
            VTSYSPARAM dwCallbackData, 
            VTDWORD    dwCommandID, 
            VTLPVOID   pCommandData);
SCCERR PerformHilite(
            VTHEXPORT hExport,
            VTLPSTR   pSearchBuffer, 
            VTLPSTR   pSearchStartBuffer, 
            VTDWORD   dwChunkStartACC,
            VTDWORD   dwMatchLen);
SCCERR PerformRedaction(
            VTHEXPORT    hExport,
            VTHDOC       hDoc,
            VTLPSTR      pSearchBuffer, 
            VTLPSTR      pSearchStartBuffer, 
            VTDWORD      dwChunkStartACC,
            VTDWORD      dwMatchLen,
            PCOMMANDOPTS pCommandOpts);
int ParseCommandLine(
            int          argc,
            char       **argv,
            PCOMMANDOPTS pCommandOpts);
SCCERR SetRedactionOptions(
            VTHDOC hDoc, 
            PCOMMANDOPTS pCommandOpts);
SCCERR SetTrackAnnoOptions(VTHDOC hDoc, PCOMMANDOPTS pCommandOpts);

#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

/* ixanno ERRORS */
#define IXANNO_ERROR_NOT_ENOUGH_PARAMETERS       -1
#define IXANNO_ERROR_INVALID_COMMAND_LINE_OPTION -2
#define IXANNO_ERROR_INVALID_REDACTION_COLOR     -3

#ifdef WINDOWS
/* Copyright (c) Microsoft Corporation. All rights reserved. */
/*
 * Copyright (c) 1987, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

int opterr = 1,             /* if error message should be printed */
    optind = 1,             /* index into parent argv vector */
    optopt,                 /* character checked for validity */
    optreset;               /* reset getopt */
char  *optarg;              /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

/*
 |  getopt -- Parse argc/argv argument vector.
 */
int getopt(int nargc, char * const *nargv, const char *ostr)
{
    static char *place = EMSG;                /* option letter processing */
    char        *oli;                         /* option letter list index */
                            
    if (optreset || !*place)                  /* update scanning pointer */ 
    {
        optreset = 0;

        if (optind >= nargc || *(place = nargv[optind]) != '-') 
        {
            place = EMSG;
            return EOF;
        }

        if (place[1] && *++place == '-') 
        {      
            /* found "--" */
            ++optind;
            place = EMSG;
            return (EOF);
        }
    }

    /* option letter okay? */
    if ((optopt = (int)*place++) == (int)':' ||
        !(oli = strchr(ostr, optopt))) 
    {
        /* if the user didn't specify '-' as an option, assume it means EOF. */
        if (optopt == (int)'-')
            return (EOF);
        if (!*place)
            ++optind;
        if (opterr && *ostr != ':')
            fprintf(
                stderr, 
                "ixanno: illegal option -- %c\n", optopt);
        return (BADCH);
    }

    if (*++oli != ':') 
    {
        /* don't need argument */
        optarg = NULL;
        if (!*place)
            ++optind;
    }
    else 
    {
        /* need an argument */
        if (*place)
        {
            /* no white space */
            optarg = place;
        }
        else if (nargc <= ++optind) 
        {   
            /* no arg */
            place = EMSG;
            if (*ostr == ':')
                return (BADARG);
            if (opterr)
                fprintf(
                    stderr,
                    "ixanno: option requires an argument -- %c\n",
                    optopt);
            return (BADCH);
        }
        else                            
        {
            /* white space */
            optarg = nargv[optind];
        }
        
        place = EMSG;
        ++optind;
    }

    return (optopt);                        /* dump back option letter */
}
#endif

SCCERR SetRedactionOptions(VTHDOC hDoc, PCOMMANDOPTS pCommandOpts)
{
    SCCERR       seResult = SCCERR_OK;
    VTCHAR       szError[256];
    unsigned int i;

    /* Set the background color */
    if ((seResult = DASetOption(
                        hDoc, 
                        SCCOPT_REDACTION_COLOR, 
                        (VTLPVOID)&(pCommandOpts->sColor), 
                        sizeof(SCCVWCOLORREF))) != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
        goto Exit;
    }

    /* See if we need to set the redaction label font name */
    if (pCommandOpts->szRedactionLabelFontName[0])
    {
        VTWORD wszRedactionLabelFontName[MAX_FONT_NAME_LEN];

        for (i = 0; i < strlen(pCommandOpts->szRedactionLabelFontName); ++i)
            wszRedactionLabelFontName[i] = 
                (VTWORD)(pCommandOpts->szRedactionLabelFontName[i]);

        if ((seResult = DASetOption(
                            hDoc, 
                            SCCOPT_REDACTION_LABEL_FONT_NAME,
                            (VTLPVOID)wszRedactionLabelFontName,
                            (VTDWORD)strlen(pCommandOpts->szRedactionLabelFontName))) != SCCERR_OK)
        {
            DAGetErrorString(seResult, szError, sizeof(szError));
            fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
            goto Exit; 
        }
    }

    /* See if we need to set the redaction label font size */
    if (pCommandOpts->dwRedactionLabelFontSize > 0)
    {
        if ((seResult = DASetOption(
                            hDoc, 
                            SCCOPT_REDACTION_LABEL_FONT_SIZE,
                            (VTLPVOID)&(pCommandOpts->dwRedactionLabelFontSize),
                            sizeof(VTDWORD))) != SCCERR_OK)
        {
            DAGetErrorString(seResult, szError, sizeof(szError));
            fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
            goto Exit;
        }
    }
    
    if (pCommandOpts->szRedactionLabelText[0])
    {
        VTBOOL bShowLabels = TRUE;

        if ((seResult = DASetOption(
                            hDoc, 
                            SCCOPT_SHOW_REDACTION_LABELS,
                            (VTLPVOID)&bShowLabels,
                            sizeof(VTBOOL))) != SCCERR_OK)
        {
            DAGetErrorString(seResult, szError, sizeof(szError));
            fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
            goto Exit;
        }
    }



Exit:
    return seResult;
}

int main(int argc, char **argv)
{
    VTHDOC              hDoc;                       /* Input doc handle returned by DAOpenDocument(). */
    VTHEXPORT           hExport;                    /* Handle to the export returned by EXOpenExport(). */
    VTCHAR              szError[256];               /* Error string buffer. */
    SCCERR              seResult = SCCERR_OK;       /* Error code returned. */
    VTHCONTENT          hContent;                   /* Content Access handle for the document */
    SCCCAGETCONTENT     locContent;                 /* Content Access structure */
    VTBOOL              bDoneFile = FALSE;          /* Flag for the finishing search the whole document */
    VTBOOL              bDoneChunk = FALSE;         /* Flag for the finishing search the whole chunk */
    VTDWORD             dwCharCount = 0;            /* Character count for one chunk */
    VTLPSTR             pBuffer = NULL;             /* Buffer for the content infomation, must be at least 1k */    
    VTLPSTR             pChunkTextBuffer = NULL;    /* Buffer for the text in one chunk */    
    VTDWORD             dwChunkTextBufferSize = 0;  /* size of the chunk text buffer */
    VTLPSTR             pSearchBuffer, pSearchStartBuffer;
    VTDWORD             dwMatchLen = 0;             /* Hilite text string length */
    VTDWORD             dwChunkStartACC = 0;        /* Actual Character Count for the start of the current chunk */
    VTDWORD             dwTextCount = 0;
    COMMANDOPTS         commandOpts;                /* Command line options */
    int                 err = 0;
    
    /* This sample app demonstrates how to annotate an input document.
     * To make annotations, character count information is required.
     * This app uses the Outside In Technology's Content Access ("CA")
     * API to get that information.  Other Outside In Technology
     * products may be licensed to get the character count information
     * required.  Contact your sales representative for more details.
     * 
     * NOTE: Users who wish to use CA in the way demonstrated here must
     *       purchase a separate license for CA in addition to their
     *       IMAGE Export license.
     */
    
    /* The basic flow of this program is to scan the document using
     * CA to extract the text.  As we extract the text we will check it
     * to see if it matches our search string.  If it does, we will add
     * it to the list of annotations.  After all the text has been scanned,
     * we will do the conversion with all instances of the search
     * string highlighted.
     */

    /* Step 0: Read the command line options */
    memset(&commandOpts, 0, sizeof(COMMANDOPTS));
    if ((err = ParseCommandLine(argc, argv, &commandOpts)) < 0)
    {
        if (err < IXANNO_ERROR_NOT_ENOUGH_PARAMETERS)
            fprintf(stderr, "One or more illegal command line options specified!\n");
        seResult = SCCERR_UNKNOWN;
        goto Exit;
    }

    /* Step 1: Start by initializing the export.   For more details on
     * how this code works, check out the hxsample.c sample code.
     */

    if ((seResult = InitializeExport(&hDoc, &hExport, &commandOpts)) != SCCERR_OK)
    {
        fprintf(stderr, "Could not initialize export!\n");
        goto Exit;
    }

    /* Step 2: Open the document for CA
     *
     * Use the hDoc we got from an earlier call to DAOpenDocuement()
     * (inside InitializeExport()) to pass to this routine.
     */

    seResult = CAOpenContent(hDoc, &hContent);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "CAOpenContent() failed: %s (0x%04X)\n", szError, seResult);
        EXCloseExport(hExport);
        DACloseDocument(hDoc);
        DADeInit();
        goto Exit;
    }

    /* Step 3: Initialize CA so we can read data.
     *
     * Start by initialize SCCCAGETCONTENT structure.
     */

    pBuffer = malloc(MAXDABUFSIZE);
    memset(pBuffer, 0, sizeof(MAXDABUFSIZE));        
    locContent.dwStructSize = sizeof(SCCCAGETCONTENT);
    locContent.dwFlags = 0;                            /* must be set to 0 */
    locContent.dwMaxBufSize = MAXDABUFSIZE;            /* Initialized by caller to size of buffer in pBuffer. */    
    locContent.pDataBuf = pBuffer;                    /* Buffer to be filled with content (must be >= 1k in size). */

    /* Set the read point to the begining of the document's content.
     * Note that despite the name, CAReadFirst() doesn't actually read
     * the first block of content.
     */

    seResult = CAReadFirst(hContent, &locContent);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "CAReadFirst() failed: %s (0x%04X)\n", szError, seResult);
        free(pBuffer);                        /* free the buffer */
        CACloseContent(hContent);            /* close Content Access */
        EXCloseExport(hExport);
        DACloseDocument(hDoc);
        DADeInit();
        goto Exit;
    }
    
    /* Create buffer for blocks of one "chunk" of text */
    pChunkTextBuffer = malloc(MAXDABUFSIZE);
    dwChunkTextBufferSize = MAXDABUFSIZE;
    pSearchStartBuffer = pSearchBuffer = pChunkTextBuffer;
    
    /* Step 4: Loop through the whole document on chunks of data. */
    while (!bDoneFile)
    {
        bDoneChunk = FALSE;

        /* Initialize the character count and text buffer for one chunk */
        dwCharCount = 0;

        while (!bDoneChunk)
        {
            /* Retrieve text from document */
            seResult = CAReadNext(hContent, &locContent);
            if (seResult == DAERR_EOF)
            {
                /* We are finished searching the whole document. */
                bDoneFile = TRUE;
                break;
            }
            else if (seResult == DAERR_ABORT)
            {
                DAGetErrorString(seResult, szError, sizeof(szError));
                fprintf(stderr, "CAReadNext() aborted: %s (0x%04X)\n", szError, seResult);
                free(pBuffer);
                free(pChunkTextBuffer);
                CACloseContent(hContent);
                EXCloseExport(hExport);
                DACloseDocument(hDoc);
                DADeInit();
                goto Exit;
            }
            
            if (locContent.dwType == SCCCA_TEXT)
            {
                /* Increment the character count in this chunk. */
                dwCharCount += locContent.dwData1;

                /* Copy the text to the pChunkText buffer.  Since CA will 
                 * sometimes give us text in single byte strings and then at 
                 * other times give us DBCS strings, we have to check every 
                 * string to see how wide a char is.  The text an be either
                 * single- or double-byte, so we have to check every string 
                 * to see how wide a char is.
                 */

                /* Copy the text to the pChunkText buffer */
                if (locContent.dwData1 * 2 == locContent.dwDataBufSize)
                {
                    /* check the chunk text buffer size */
                    if ((dwCharCount * 2) > dwChunkTextBufferSize)
                    {
                        VTDWORD dwOffset = (VTDWORD)(pChunkTextBuffer - pSearchStartBuffer);
                        /* move the chunk text buffer to the start position */

                        pChunkTextBuffer = pSearchStartBuffer;
                        pChunkTextBuffer = realloc(pChunkTextBuffer, dwCharCount * 2);
                        dwChunkTextBufferSize = dwCharCount * 2;
                        /* reset the search start buffer and search buffer */

                        pSearchStartBuffer = pSearchBuffer = pChunkTextBuffer;
                        pChunkTextBuffer += dwOffset;
                    }
                    memcpy(pChunkTextBuffer, locContent.pDataBuf, locContent.dwData1 * 2);
                    pChunkTextBuffer += locContent.dwData1 * 2;
                }
                else
                {
                    /* check the chunk text buffer size */
                    if (dwCharCount > dwChunkTextBufferSize)
                    {
                        VTDWORD dwOffset = (VTDWORD)(pChunkTextBuffer - pSearchStartBuffer);
                        /* move the chunk text buffer to the start position */

                        pChunkTextBuffer = pSearchStartBuffer;
                        pChunkTextBuffer = realloc(pChunkTextBuffer, dwCharCount);
                        dwChunkTextBufferSize = dwCharCount;
                        /* reset the search start buffer and search buffer */

                        pSearchStartBuffer = pSearchBuffer = pChunkTextBuffer;
                        pChunkTextBuffer += dwOffset;
                    }
                    memcpy(pChunkTextBuffer, locContent.pDataBuf, locContent.dwData1);
                    pChunkTextBuffer += locContent.dwData1;
                }
            }
            if (locContent.dwType == SCCCA_BREAK)
            {
                /* find the chunk break. finished getting the text from the chunk */
                bDoneChunk = TRUE;
            }
        }
       
        dwTextCount = dwCharCount;
        pSearchBuffer = pSearchStartBuffer;
                
        if (commandOpts.szJSONFile[0])
        {
            unsigned long size = 0;
            char*         pHiliteBuffer = NULL;
            FILE*         fp = fopen(commandOpts.szJSONFile, "rb");

            if ((err = SetRedactionOptions(hDoc, &commandOpts)) == SCCERR_OK)
            {
                if (fp)
                {
                    /* Determine the size of the JSON data in the file */
                    fseek(fp, 0, SEEK_END);
                    size = ftell(fp);
                    fseek(fp, 0, SEEK_SET);

                    /* Then, read in the JSON data and apply the annotation */
                    pHiliteBuffer = (char *)malloc((size_t)(size + 1));
                    if (pHiliteBuffer)
                    {
                        fread((void *)pHiliteBuffer, size, 1, fp);
                        pHiliteBuffer[size] = 0;

                        EXApplyHilites(hExport, (VTBYTE *)pHiliteBuffer);

                        free(pHiliteBuffer);
                    }

                    fclose(fp);
                }
            }
        }
        else if (dwCharCount)
        {
            /* Scan the chunk text buffer for the string to hilite. */
            while ((pSearchBuffer = FindMatches(
                                        pSearchBuffer, 
                                        commandOpts.szSearchString, 
                                        &dwTextCount, 
                                        &dwMatchLen)) != NULL)
            {
				SetTrackAnnoOptions(hDoc, &commandOpts);
                if (commandOpts.bRedaction)
                {
                    err = PerformRedaction(
                                hExport,
                                hDoc,
                                pSearchBuffer, 
                                pSearchStartBuffer, 
                                dwChunkStartACC, 
                                dwMatchLen,
                                &commandOpts);
                }
                else
                {
                    err = PerformHilite(
                                 hExport, 
                                 pSearchBuffer, 
                                 pSearchStartBuffer, 
                                 dwChunkStartACC, 
                                 dwMatchLen);
                }

                /* Adjust the text count and search buffer pointer. */
                pSearchBuffer++;
                dwTextCount--;
            }
        }

        /* Move to the next chunk. */
        /* Update the Actual Character Count to the start of the next chunk. */
        dwChunkStartACC += dwCharCount;

        /* Put the chunk text buffer to the start position. */
        pChunkTextBuffer = pSearchStartBuffer;
    }

    /* Step 5:  terminate Content Access */
    seResult = CACloseContent(hContent);
    free(pChunkTextBuffer);
    free(pBuffer);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "CACloseContent() failed: %s (0x%04X)\n", szError, seResult);
        EXCloseExport(hExport);
        DACloseDocument(hDoc);
        DADeInit();
        goto Exit;
    }  


    /* Step 6: Convert the document.   For more details on how
     * this code works, check out the hxsample.c sample code.
     */
    seResult = DoConversion(hDoc, hExport);
    
Exit:
    return seResult;
}

void PrintUsage()
{
    fprintf(stderr, "ixanno\tConverts InputFile to TIFF Image while hiliting or\n");
    fprintf(stderr, "\tredacting instances of SearchString.\n\n");
    fprintf(stderr, "Usage:\tixanno -i In -o Out -t TEXT [-r] [-c COLOR] [-l LABEL] [-f FONT] [-s SIZE] [-g]\n");
    fprintf(stderr, "Where:\n");
    fprintf(stderr, "  -i In\t\tis the name of the file to be converted.\n");
    fprintf(stderr, "  -o Out\tis the name of the output converted TIFF graphic.\n");
    fprintf(stderr, "  -t TEXT\tis the text to be hilited or redacted.\n");
    fprintf(stderr, "  [-j JSON]\tis a JSON file containing annotation data.\n");
    fprintf(stderr, "  [-r]\t\tmeans perform redaction instead of hiliting.\n");
    fprintf(stderr, "  [-c COLOR]\tis a color other than black for redaction (white, green, blue, red).\n");
    fprintf(stderr, "  [-l LABEL]\tis a label for redaction.\n");
    fprintf(stderr, "  [-f FONT]\tis the name of a font to be used for redaction label.\n");
    fprintf(stderr, "  [-s SIZE]\tis the point size to be used for redaction label.\n");
    fprintf(stderr, "  [-g]\t\tmeans to use internal OIT rendering code (limited availability).\n");
    fprintf(stderr, "  [-m]\t\tmeans produce multi-page TIFF.\n");
}

int ParseCommandLine(
            int          argc,
            char       **argv,
            PCOMMANDOPTS pCommandOpts)
{
    int    err        = 0;
    int    c          = 0;
    char  *cvalue     = NULL;
    char  *color      = NULL;
    VTBOOL bInSet     = FALSE;
    VTBOOL bOutSet    = FALSE;
    VTBOOL bSearchSet = FALSE;
    VTBOOL bJSONSet   = FALSE;

    opterr = 0;

    while ((c = getopt(argc, argv, "i:o:j:t:c:l:f:s:rgm")) != -1)
    {
        switch(c)
        {
            case 'i':
                cvalue = optarg;
                strncpy(pCommandOpts->szInputFile, cvalue, VT_MAX_FILEPATH);
                bInSet = TRUE;
                break;
            case 'o':
                cvalue = optarg;
                strncpy(pCommandOpts->szOutputFile, cvalue, VT_MAX_FILEPATH);
                bOutSet = TRUE; 
                break;
            case 'j':
                cvalue = optarg;
                strncpy(pCommandOpts->szJSONFile, cvalue, VT_MAX_FILEPATH);
                bJSONSet = TRUE;
                break;
            case 't':
                cvalue = optarg;
                strncpy(pCommandOpts->szSearchString, cvalue, MAX_SEARCH_STRING_LEN);
                bSearchSet = TRUE; 
                break;
            case 'c':
                color = optarg;
                break;
            case 'l':
                cvalue = optarg;
                strncpy(pCommandOpts->szRedactionLabelText, cvalue, EXANNO_MAXLABEL);
                break;
            case 'f':
                cvalue = optarg;
                strncpy(pCommandOpts->szRedactionLabelFontName, cvalue, MAX_FONT_NAME_LEN); 
                break;
            case 's':
                pCommandOpts->dwRedactionLabelFontSize = (VTDWORD)atol(optarg);
                break;
            case 'r':
                pCommandOpts->bRedaction = TRUE;
                break;
            case 'g':
                pCommandOpts->bUseGD = TRUE;
                break;
            case 'm':
                pCommandOpts->bMultiPageTiff = TRUE;
                break;
            default:
                err = IXANNO_ERROR_INVALID_COMMAND_LINE_OPTION;
                goto Exit;
                break;
        }
    }

    /* Check minimum requirements */
    if (!bInSet || !bOutSet || !(bSearchSet || bJSONSet))
    {
        PrintUsage();
        err = IXANNO_ERROR_NOT_ENOUGH_PARAMETERS;
        goto Exit;
    }

    /* Handle the optional redaction color */
    if (color)
    {
        if (strncmp(color, "red", 3) == 0)
        {
            pCommandOpts->sColor = SCCVWRGB(0xFF, 0x00, 0x00);
        }
        else if (strncmp(color, "green", 5) == 0)
        {
            pCommandOpts->sColor = SCCVWRGB(0x00, 0xFF, 0x00);
        }
        else if (strncmp(color, "blue", 4) == 0)
        {
            pCommandOpts->sColor = SCCVWRGB(0x00, 0x00, 0xFF);
        }
        else if (strncmp(color, "white", 5) == 0)
        {
            pCommandOpts->sColor = SCCVWRGB(0xFF, 0xFF, 0xFF);
        }
        else
        {
            err = IXANNO_ERROR_INVALID_REDACTION_COLOR;
            goto Exit;
        }
    }

Exit:
    return err;
}

SCCERR PerformHilite(
            VTHEXPORT hExport,
            VTLPSTR   pSearchBuffer, 
            VTLPSTR   pSearchStartBuffer, 
            VTDWORD   dwChunkStartACC,
            VTDWORD   dwMatchLen)
{
    EXANNOHILITETEXT sHiliteText;

    /* Found a match, setup the hilite info. */
    memset(&sHiliteText, 0, sizeof(EXANNOHILITETEXT));
    sHiliteText.dwSize = sizeof(EXANNOHILITETEXT);

    sHiliteText.dwOptions = SCCVW_USECHARATTR   | 
                            SCCVW_USEFOREGROUND | 
                            SCCVW_USEBACKGROUND;
    sHiliteText.sForeground = SCCVWRGB(0,0x80,0);
    sHiliteText.sBackground = SCCVWRGB(0xFF,0xFF,0x80);
    sHiliteText.wCharAttr   = SCCVW_CHARATTR_UNDERLINE;
    sHiliteText.wCharAttrMask = SCCVW_CHARATTR_UNDERLINE;

    /* 
     | Set the start and end Actual Character Count for the hilite.  Subtract 1 
     | from dwMatchLen for the NULL terminator, but then we need to add 1 back 
     | because the end ACC needs to point to the spot just past the last 
     | character to highlight.
     */
   
    sHiliteText.dwStartACC = (VTDWORD)((dwChunkStartACC + 
                                        pSearchBuffer - 
                                        pSearchStartBuffer));
    sHiliteText.dwEndACC = (VTDWORD)((dwChunkStartACC + 
                                      pSearchBuffer - 
                                      pSearchStartBuffer)) + dwMatchLen;

    /* Print the Actual Character Count of the start and end of the hilite text */
    fprintf(
        stdout, 
        "Match found, StartACC=%d, EndACC=%d\n", 
        sHiliteText.dwStartACC, 
        sHiliteText.dwEndACC);

    /* Make the call to create a hilite annotation. */
    return EXHiliteText(hExport, &sHiliteText);
}

SCCERR PerformRedaction(
            VTHEXPORT     hExport,
            VTHDOC        hDoc,
            VTLPSTR       pSearchBuffer, 
            VTLPSTR       pSearchStartBuffer, 
            VTDWORD       dwChunkStartACC,
            VTDWORD       dwMatchLen,
            PCOMMANDOPTS  pCommandOpts)
{
    EXANNOREDACTTEXT sRedactText;
    SCCERR           seResult = SCCERR_OK; 
    unsigned int     i;

    /* Found a match, setup the redaction info. */
    memset(&sRedactText, 0, sizeof(EXANNOREDACTTEXT));
    sRedactText.dwSize = sizeof(EXANNOREDACTTEXT);

    /* 
     | Set the start and end Actual Character Count for the hilite.  Subtract 1 
     | from dwMatchLen for the NULL terminator, but then we need to add 1 back 
     | because the end ACC needs to point to the spot just past the last 
     | character to highlight.
     */

    sRedactText.dwStartACC = (VTDWORD)((dwChunkStartACC + 
                                        pSearchBuffer - 
                                        pSearchStartBuffer));
    sRedactText.dwEndACC = (VTDWORD)((dwChunkStartACC + 
                                      pSearchBuffer - 
                                      pSearchStartBuffer)) + dwMatchLen;
   
    if ((seResult = SetRedactionOptions(hDoc, pCommandOpts)) != SCCERR_OK)
    {
        goto Exit;
    }
    
    /* See if we have a redaction label, and if so load it into the annotation struct */
    if (pCommandOpts->szRedactionLabelText[0])
    {
        /* Put the byte string label into the requisite word string field */
        for (i = 0; i < strlen(pCommandOpts->szRedactionLabelText) && i < EXANNO_MAXLABEL; ++i)
        {
            sRedactText.wzLabel[i] = (VTWORD)(pCommandOpts->szRedactionLabelText[i]);
        }
    }

    /* Print the Actual Character Count of the start and end of the redaction text */
    fprintf(
        stdout, 
        "Match found, StartACC=%d, EndACC=%d\n", 
        sRedactText.dwStartACC, 
        sRedactText.dwEndACC);

Exit:
    if (seResult != SCCERR_OK)
        return seResult;

    return EXRedactText(hExport, &sRedactText);
}

VTLPSTR FindMatches(
            VTLPSTR pBuffer, 
            VTLPSTR pText, 
            VTLPDWORD pCharCount, 
            VTLPDWORD pMatchLen)
{
    VTLPSTR pSearchText = NULL;        
    VTLPSTR pTextBuffer = NULL;
    VTDWORD locCount    = 0; 
    VTBOOL  bDone       = FALSE;

    /* The only special thing to note here is that we have to watch out
     * for nulls in the buffer as we are doing our string compares.
     */

    *pMatchLen = 0;
    while (*pCharCount != 0)
    {
        pTextBuffer = pBuffer;
        pSearchText = pText;
        locCount = *pCharCount;
        while (!bDone)
        {
            if (*pSearchText == '\0') /* done */
            {
                *pMatchLen = (*pCharCount) - locCount;
                return(pBuffer);
            }
            while ((*pTextBuffer == '\0') && (locCount)) /* Skip nulls in buffer */
            {
                pTextBuffer++;
                locCount--;
            }
            if ((*pTextBuffer != *pSearchText) || (!locCount))
                break;
            pTextBuffer++;
            pSearchText++;
            locCount--;
        }
        pBuffer++;
        (*pCharCount)--;
    }
    return(NULL);
}


SCCERR InitializeExport(
        VTLPHDOC    phDoc, 
        VTLPHEXPORT phExport, 
        PCOMMANDOPTS pCommandOpts)
{
    VTCHAR szError[256];            
    SCCERR seResult = SCCERR_OK; 

    seResult = DAInitEx(SCCOPT_INIT_NOTHREADS, OI_INIT_DEFAULT);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DAInitEx() failed: %s (0x%04X)\n", szError, seResult);
        goto Exit;
    }

    seResult = DAOpenDocument(phDoc, PATH_TYPE, (VTLPVOID)(pCommandOpts->szInputFile), 0);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DAOpenDocument() failed: %s (0x%04X)\n", szError, seResult);
        goto Exit;
    }
    
    /* Set the option for using (or not) OIT's internal rendering solution. */
    if ((seResult = DASetOption(
                        *phDoc, 
                        SCCOPT_RENDERING_PREFER_OIT,
                        (VTLPVOID)&pCommandOpts->bUseGD,
                        sizeof(VTBOOL))) != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
        goto Exit;
    }

    seResult = EXOpenExport(
                   *phDoc, 
                   FI_TIFF, 
                   PATH_TYPE, 
                   pCommandOpts->szOutputFile, 
                   0, 
                   0,
                   (EXCALLBACKPROC)ExportOemCallback, 
                   0, 
                   phExport);

    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "EXOpenExport() failed: %s (0x%04X)\n", szError, seResult);
        goto Exit;
    }

    /* If we are producing multi-page TIFF, set the option */
    if (pCommandOpts->bMultiPageTiff)
    {
        EXTIFFOPTIONS exTIFFOptions = { 0 };

        exTIFFOptions.dwSize = sizeof(EXTIFFOPTIONS);
        exTIFFOptions.dwTIFFFlags |= SCCOPT_GRAPHIC_MULTIPAGE;

        if ((seResult = DASetOption(
                            *phDoc, 
                            SCCOPT_IMAGEX_TIFFOPTIONS,
                            (VTLPVOID)&exTIFFOptions,
                            sizeof(EXTIFFOPTIONS))) != SCCERR_OK)
        {
            DAGetErrorString(seResult, szError, sizeof(szError));
            fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
            goto Exit;
        }
    }

Exit:
    return seResult;
}


SCCERR DoConversion(VTHDOC hDoc, VTHEXPORT hExport)
{
    SCCERR seResult        = SCCERR_OK;
    SCCERR seOriginalError = SCCERR_OK; /* There may be multiple failures - we'll return the first error */
    VTCHAR szError[256];       

    seResult = EXRunExport(hExport);

    if (gdwGraphicExportErrors != 0)
    {
        fprintf(stderr, "Failed to convert %d graphic image(s).\n", gdwGraphicExportErrors);
    }

    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "EXRunExport() failed: %s (0x%04X)\n", szError, seResult);
        if (seOriginalError == SCCERR_OK)
            seOriginalError = seResult;
    }
    else
    {
        gdwFileCount++;
        fprintf(stdout, "Export successful: %d output file(s) created.\n", gdwFileCount);
    }

    seResult = EXCloseExport(hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "EXCloseExport() failed: %s (0x%04X)\n", szError, seResult);
        if (seOriginalError == SCCERR_OK)
            seOriginalError = seResult;
    }

    seResult = DACloseDocument(hDoc);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DACloseDocument() failed: %s (0x%04X)\n", szError, seResult);
        if (seOriginalError == SCCERR_OK)
            seOriginalError = seResult;
    }

    seResult = DADeInit();
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, szError, sizeof(szError));
        fprintf(stderr, "DADeInit() failed: %s (0x%04X)\n", szError, seResult);
        if (seOriginalError == SCCERR_OK)
            seOriginalError = seResult;
    }

    return seOriginalError;
}


SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData,
                         VTDWORD dwCommandID, VTLPVOID pCommandData)
{
    SCCERR seResult = SCCERR_NOTHANDLED;
    EXGRAPHICEXPORTINFO *pGraphicExportInfo;
	VTDWORD* pageNum;
    UNUSED(hExport);
    UNUSED(dwCallbackData);

    /* This is a simple callback routine just to show how they are
     * coded.  The callback routine should always return
     * SCCERR_NOTHANDLED unless the user is returning data to the
     * conversion.
     */

    switch (dwCommandID)
    {
        case EX_CALLBACK_ID_CREATENEWFILE:
            /* This counts the number of files that are created by the
             * conversion.  Note that this callback is not done on the
             * root output file of the conversion.  We will have to add 
             * 1 to this later to account for that.
             */
            gdwFileCount++;
            break;

        case EX_CALLBACK_ID_GRAPHICEXPORTFAILURE:
            /* Increment the count of graphics that failed to convert. */
            pGraphicExportInfo = (EXGRAPHICEXPORTINFO *)pCommandData;
            if (pGraphicExportInfo->ExportGraphicStatus != SCCERR_OK)
            {
                ++gdwGraphicExportErrors;
            }
            break;
		case EX_CALLBACK_ID_PAGEHASREDACTION:
			/* This gives page index which has redaction applied
			 */
			pageNum = (VTDWORD*)pCommandData;
			printf("Redaction applied on page (index) : \"%d\"\n", *pageNum);
			break;
        default:
            break;
    }
    return seResult;
}

SCCERR SetTrackAnnoOptions(VTHDOC hDoc, PCOMMANDOPTS pCommandOpts)
{

	SCCERR       seResult = SCCERR_OK;
	VTCHAR       szError[256];
	static char flag = 0 ;

	/* See if  redaction enabled bRedaction then set SCCOPT_FLAG_PAGEREDACTIONS_ENABLE
	*  User will get call back for each page only once on which redaction is applied.
	*/
	if (flag) return;
	if (pCommandOpts->bRedaction == 1)
	{
		if ((seResult = DASetOption(
			hDoc,
			SCCOPT_FLAG_PAGEREDACTIONS_ENABLE,
			(VTLPVOID)&(pCommandOpts->bRedaction),
			sizeof(pCommandOpts->bRedaction))) != SCCERR_OK)
		{
			DAGetErrorString(seResult, szError, sizeof(szError));
			fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
			return SCCERR_UNKNOWN;
		}
		// option to set track anno
		{
			VTDWORD       trackanno = SCCOPT_TRACK_REDACTIONS;
			if ((seResult = DASetOption(
				hDoc,
				SCCOPT_TRACK_ANNOTATIONS,
				(VTLPVOID)&(trackanno),
				sizeof(VTDWORD))) != SCCERR_OK)
			{
				DAGetErrorString(seResult, szError, sizeof(szError));
				fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
				return SCCERR_UNKNOWN;
			}
			printf("\nIXANNO : trackanno option set is  = %d\n\n", trackanno);
		}
		flag = 1;

	}
	else
	{

		// option to set track anno
		{
			VTDWORD       trackanno = SCCOPT_TRACK_HIGHLIGHTS;
			if ((seResult = DASetOption(
				hDoc,
				SCCOPT_TRACK_ANNOTATIONS,
				(VTLPVOID)&(trackanno),
				sizeof(VTDWORD))) != SCCERR_OK)
			{
				DAGetErrorString(seResult, szError, sizeof(szError));
				fprintf(stderr, "DASetOption failed: %s (%04X)\n", szError, seResult);
				return SCCERR_UNKNOWN;
			}
			printf("\nIXANNO : trackanno option set is  = %d\n\n", trackanno);
		}

	}
	flag = 1;

}
