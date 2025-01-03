/*
|   EXRedir
|
|   Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.
|   All rights reserved.
|
|   You have a royalty-free right to use, modify, reproduce and
|   distribute the Sample Applications (and/or any modified version)
|   in any way you find useful, provided that you agree that Oracle
|   has no warranty obligations or liability for any
|   Sample Application files which are modified.
|
|   Purpose:
|   Shows how to use redirected IO in Export.
|   As a trivial example, it redirects the Viewer's IO routines through
|   the C runtime FILE routines (fopen, fread, etc...)
|   It is based on the EXSimple sample app, and maintains the same user
|   interface.
|
*/

/*
*   This app deliberately does not support IOGETINFO_DPATHNAME and
*   IOGETINFO_GENSECONDARYDP in order to provide a test platform for
*   proper fallback.
*   jrw  8/15/12
*/


#ifdef WIN32                /* Defined by my Win32 compiler */
#ifdef _DEBUG
#define WIN32DEBUG
#endif
#define     WINDOWS         /* Required for Windows         */
#include <windows.h>
#else
#ifndef UNIX
#define     UNIX            /* Required for UNIX            */
#endif
#endif

/* Required by functions used in this program, but not by export */
#include <stdio.h>          /* io routines */
#include <stdlib.h>
#include <string.h>         /* strcpy() and strncpy(), strrchr() */
#include <time.h>

#ifdef WINDOWS
#include <direct.h>         /* mkdir(), getcwd() */
#else
#include <sys/types.h>      /* mkdir() */
#include <sys/stat.h>
#include <unistd.h>         /* getcwd() */

/*
   Oracle does version tracking of the Outside In sample applications.
   OIT_VERSTRING is defined by internal build scripts so you can delete
   the following line at your leisure as it performs no real function.
*/
#ifdef OIT_VERSTRING
char oitsamplever[32] = OIT_VERSTRING;
#endif

#endif

#include <sccex.h>          /* Required by export     */
#include "cfg.c"            /* User interface code,   */
                            /* NOT needed by export.  */

#ifdef UNIX
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#if 0
#define PRINT_OPTIONS       /* When defined, PrintOpts() prints the values of all the configuration options. */
#endif


/*****************************************************************
 *
 * Data Structure for Redirected I/O
 *
 *****************************************************************/

/*
|   This struct can contain whatever the developer determines to be necessary
|   for the purposes of the application being developed.  The first element
|   MUST be a BASEIO struct.  Beyond that it is up to the developer.  Here
|   we have a pointer to a FILE, a buffer for a the filename, a handle used
|   to hold a reference to itself, and a buffer used by MyTextWrite().
*/
typedef struct MYFILEtag
{
    BASEIO      sBaseIO;
    FILE        *pFile;
    VTCHAR      szFileName[BUF_SIZE];
    VTHANDLE    hThis;
    VTCHAR      szBuf[BUF_SIZE];
    VTCHAR      szSubdocSpec[DA_MAXSUBDOCSPEC];
    VTDWORD     dwFileFlags;
    VTLPVOID    pSpec;
} MYFILE, * LPMYFILE;

#define IO_READ         1
#define IO_WRITE_BINARY 2
#define IO_WRITE_TEXT   3


/*
|   This is the maximum amount of nesting this sample app will allow
|   for archives inside archives.  Don't make this too big or we will
|   be likely to overrun the max length of a file name.
*/
#define MAX_ARCHIVE_STACK_COUNT 10

typedef struct
{
    VTLPSTR     pstrFile;
    VTCHAR      szArchiveSubDirs[VT_MAX_URL];
    VTDWORD     dwArchiveStackCount;
    VTLPSTR     pArchiveStack[MAX_ARCHIVE_STACK_COUNT];
}CALLBACKDATASTRUCT;


/*  Function prototypes  */

SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData, VTDWORD dwCommandID, VTLPVOID pCommandOrInfoData);
SCCERR HandleCreateNewFile(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleNewFileInfo(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleOcrError(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleOEMOutput(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleProcessLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleCustomElementList(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleProcessElementStr(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleGraphicExportFailure(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleOEMOutput2(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleProcessElementStr2(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleAltLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleEnterArchive(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleLeaveArchive(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleRefLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
VTVOID URLEncode(VTLPSTR pOut, VTLPSTR pIn);
VTCHAR *FakeURLEncode(VTCHAR *pOut, VTWORD wChar);
VTVOID GetOutputPath(CALLBACKDATASTRUCT *pExredirData);

LPMYFILE MyOpen(VTDWORD dwRWFlag, VTLPSTR szFileName);
IOERR MyClose(HIOFILE hFile);
IOERR MyTempFileClose(HIOFILE hFile);
IOERR MyRead(HIOFILE hFile, VTLPBYTE pData, VTDWORD dwSize, VTLPDWORD pdwCount);
IOERR MyTextWrite(HIOFILE hFile, VTLPBYTE pData, VTDWORD dwSize, VTLPDWORD pdwCount);
IOERR MyBinaryWrite(HIOFILE hFile, VTLPBYTE pData, VTDWORD dwSize, VTLPDWORD pdwCount);
IOERR MySeek(HIOFILE hFile, VTWORD wFrom, VTLONG lOffset);
IOERR MyTell(HIOFILE hFile, VTLPDWORD pOffset);
IOERR MySeek64(HIOFILE hFile, VTWORD wFrom, VTOFF_T Offset);
IOERR MyTell64(HIOFILE hFile, VTOFF_T *  Offset);
IOERR MyGetInfo(HIOFILE hFile, VTDWORD dwInfoID, VTLPVOID pInfo);

VTSHORT EXwcslen(VTLPWORD pString);
VTDWORD pRedirtTempFileCallbackFunc(HIOFILE *phFile,  VTLPVOID pSpec,  VTDWORD dwFileFlags);
VTDWORD MyFileAccessCallback (VTDWORD dwID, VTSYSVAL pRequestData, VTSYSVAL pReturnData, VTDWORD dwReturnDataSize);



/* callback handler array */
typedef SCCERR (*CALLBACKHANDLER)(VTSYSPARAM, VTLPVOID);
typedef struct
{
    VTDWORD dwID;
    CALLBACKHANDLER pfn;
} CALLBACKMSGSTRUCT;
typedef struct tagKEY
{
    VTLPSTR pstrText;
    int nLength;
} KEY;

const CALLBACKMSGSTRUCT _aCBHandlers[] =
{
    {EX_CALLBACK_ID_CREATENEWFILE, HandleCreateNewFile},
    {EX_CALLBACK_ID_NEWFILEINFO, HandleNewFileInfo},
    {EX_CALLBACK_ID_OEMOUTPUT, HandleOEMOutput},
    {EX_CALLBACK_ID_PROCESSLINK, HandleProcessLink},
    {EX_CALLBACK_ID_CUSTOMELEMENTLIST, HandleCustomElementList},
    {EX_CALLBACK_ID_PROCESSELEMENTSTR, HandleProcessElementStr},
    {EX_CALLBACK_ID_GRAPHICEXPORTFAILURE, HandleGraphicExportFailure},
    {EX_CALLBACK_ID_OEMOUTPUT_VER2, HandleOEMOutput2},
    {EX_CALLBACK_ID_PROCESSELEMENTSTR_VER2, HandleProcessElementStr2},
    {EX_CALLBACK_ID_ALTLINK, HandleAltLink},
    {EX_CALLBACK_ID_ENTERARCHIVE, HandleEnterArchive},
    {EX_CALLBACK_ID_LEAVEARCHIVE, HandleLeaveArchive},
    {EX_CALLBACK_ID_REFLINK, HandleRefLink},
    {EX_CALLBACK_ID_OCRERROR, HandleOcrError}
};
#define NUMHANDLERS (sizeof(_aCBHandlers) / sizeof(CALLBACKMSGSTRUCT))



Option Opts;       /* Export configuration parameters.  Application-specific implementation.   */
char _szError[256]; /* Buffer for error strings.                                                */
VTDWORD gdwFileCount;               /* Global count of files created.                           */
VTDWORD gdwGraphicExportErrors;     /* Global count of graphics export failures                 */
VTDWORD gdwItemCount;               /* Global count of archive items has been exported          */
VTDWORD gdwItemExportErrors;        /* Global count of archive items export failures            */
VTDWORD gNumStatCallbacks = 0;		/* Global count of the number of callbacks made */
VTDWORD gdwTempFileCount;           /* Global temp file count                                   */

VTCHAR  gpHex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };


#define IsDirSep(c)     (((c) == '\\') || ((c) == '/') || ((c) == ':'))

/*
|
|   function:   Stats Callback
|				used to verify that OIT is still producing data
|
|   parameters: hUnique			handle to the export
|               dwID			id of the callback
|				pCallbackData	reserved
|				pAppData		pointer to user set data (gNumStatCallbacks here)
|   returns:    VOID
|
|
*/
VTDWORD MyStatCallBack( VTHANDLE hUnique, VTDWORD dwID, VTSYSVAL pCallbackData, VTSYSVAL pAppData)
{
	/* pAppData can be any user set data.  In this example, it is just a pointer to gNumStatCallbacks */
	VTLPDWORD	lpCount = (VTLPDWORD)pAppData;
    UNUSED(hUnique);
    UNUSED(pCallbackData);

	/* only one ID is currently supported, abort if anything else is passed in */
	if(dwID != OIT_STATUS_WORKING)
		return (OIT_STATUS_ABORT);

	(*lpCount)++;		/* increment the count */

	/* if the current count (lpCount) is more than the desired count (Opts.dwNumStatCallbacks), then abort */
	if(*lpCount > Opts.dwNumStatCallbacks)
	{
		fprintf(stdout, "MyStatCallBack exiting on call number %d\n", *lpCount);
		return (OIT_STATUS_ABORT);
	}
	else
	{
		fprintf(stdout, "MyStatCallBack call number %d\n", *lpCount);
		return (OIT_STATUS_CONTINUE);
	}
}

/*
|
|   function:   Validate Format
|				used to validate that the sample app supports the desired output format
|
|   parameters: dwFormat		requested format
|   returns:    VTBOOL
|
|   purpose:	There are two major paradigms for handling redirected I/O.
|				There are two sample apps, exredir and xxredir, that handle the different paradigms.
|				This function verifies that the correct app is being used.
|
*/
VTBOOL ValidateFormat(VTDWORD dwFormat)
{
	VTBOOL bValid = FALSE;

	/* check if dwFormat is one the Image Export formats */
	if((FI_TIFF == dwFormat) || (FI_PNG == dwFormat) || (FI_BMP == dwFormat) ||
		(FI_GIF == dwFormat) || (FI_JPEGFIF == dwFormat) || (FI_JPEG2000 == dwFormat ))
		bValid = TRUE;

	/* check if dwFormat is one the other PDF Export formats */
	if((FI_PDF == dwFormat) || (FI_PDFA == dwFormat) || (FI_PDFA_2 == dwFormat))
		bValid = TRUE;

	/* check if dwFormat is one the HTML Export formats */
    if((FI_HTML == dwFormat) || (FI_MHTML == dwFormat) || (FI_HTML5 == dwFormat))
		bValid = TRUE;

	/* output an error message */
	if (!bValid)
		fprintf(stdout, "The requested output format isn't valid with this sample app\n");

	return (bValid);
}

/*
|
|   function:   main
|   parameters: int argc
|               char *argv[]
|   returns:    int
|
|   purpose:    duh...
|
*/

int main(int argc, char *argv[])
{
    SCCERR      seResult;
    VTHDOC      hDoc, hWatermarkDoc;
    VTHEXPORT   hExport;
    LPMYFILE    pInFile, pWatermarkFile;
    FILE        *pFile = NULL;
    VTBOOL      bUnicodeCallbackStr = FALSE;
    CALLBACKDATASTRUCT CallbackData;
    REDIRECTTEMPFILECALLBACKPROC pCallbackFunc;
    VTDWORD     dwDaInitExFlags = OI_INIT_DEFAULT;

#ifdef WIN32DEBUG
    SYSTEMTIME st;
#endif

    memset(&Opts, 0, sizeof(Option));
    seResult = ReadCommandLine(argc, argv, &Opts, &pFile);
    if (seResult != SCCERR_OK)
    {
        goto ErrorExit;
    }

    gdwFileCount = 0;
    gdwItemExportErrors = 0;
    gdwItemCount = 0;
    gdwTempFileCount = 0;
    CallbackData.pstrFile = Opts.szOutFile;
    *CallbackData.szArchiveSubDirs = '\0';
    CallbackData.dwArchiveStackCount = 0;
    CallbackData.pArchiveStack[0] = &CallbackData.szArchiveSubDirs[0];


    /*
    |   When using redirected I/O, it is necessary to make your own open calls,
    |   as the technology never opens the files.  If you specify Redirected I/O
    |   the technology assumes it is receiving a reference to an already open file.
    */

    pInFile = MyOpen(IO_READ, Opts.szInFile);
    if (pInFile == NULL)
    {
        seResult = SCCERR_FILEOPENFAILED;
        goto ErrorExit;
    }

    /*Read the configuration file.*/
    if (ParseConfigFile(pFile, &Opts) != SCCERR_OK)
    {
        seResult = SCCERR_BADPARAM;
        goto ErrorExit;
    }


	/* check if the requested output format is supported by this sample app) */
	if (!ValidateFormat(Opts.dwOutputID))
    {
        seResult = SCCERR_FEATURENOTAVAIL;
        goto ErrorExit;
    }

    if(FALSE == Opts.bLoadOptions)
    {
        dwDaInitExFlags |= OI_INIT_NOLOADOPTIONS;
    }

    if(FALSE == Opts.bSaveOptions)
    {
        dwDaInitExFlags |= OI_INIT_NOSAVEOPTIONS;
    }

    /* Initialize the Data Access module.  Required */
    seResult = DAInitEx(SCCOPT_INIT_NOTHREADS, dwDaInitExFlags);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DAInitEx() failed: %s (0x%04X)\n", _szError, seResult);
        goto ErrorExit;
    }


    /*
    |   Set the global options.  Most
    |   options must be set using the hDoc returned by DAOpenDocument().
    |   However, a couple options we handle such as the FIFLAGS option
    |   require a NULL handle.
    */

    if (Opts.abSetOptions[SAMPLE_OPTION_TEMPDIRECTORY])
    {
        SCCUTTEMPDIRSPEC tdsSpec;
        tdsSpec.dwSize = sizeof(tdsSpec);
        tdsSpec.dwSpecType = IOTYPE_DEFAULT;
        strcpy(tdsSpec.szTempDirName, Opts.szTempDir);

        if ((seResult = DASetOption((VTHDOC)NULL, SCCOPT_TEMPDIR, (VTLPVOID)&tdsSpec, sizeof(tdsSpec))) != SCCERR_OK)
        {
            DAGetErrorString(seResult, _szError, sizeof(_szError));
            fprintf(stderr, "DASetOption() failed: %s (0x%04X)\n", _szError, seResult);
            DADeInit();
            goto ErrorExit;
        }
    }

	if (Opts.abSetOptions[SAMPLE_OPTION_WATERMARKIOTYPE])
	{
		if ( Opts.bEnableWatermark && Opts.WatermarkIO.dwType == IOTYPE_REDIRECT )
		{
			pWatermarkFile = MyOpen(IO_READ, Opts.WatermarkIO.Path.szWaterMarkPath);
			Opts.WatermarkIO.phDoc = &hWatermarkDoc;
			seResult = DAOpenDocument(Opts.WatermarkIO.phDoc, IOTYPE_REDIRECT, (VTLPVOID)pWatermarkFile, 0);
			if (seResult != SCCERR_OK)
			{
                seResult = SCCERR_WATERMARKFILEFAILURE;
				DAGetErrorString(seResult, _szError, sizeof(_szError));
				fprintf(stderr, "DAOpenDocument() failed: %s (0x%04X)\n", _szError, seResult);
				DADeInit();
				goto ErrorExit;
			}
		}
	}


    if (Opts.abSetOptions[SAMPLE_OPTION_FIFLAGS])
    {
        if ((seResult = DASetOption((VTHDOC)NULL, SCCOPT_FIFLAGS, (VTLPVOID)&Opts.dwFIFlags, sizeof(VTDWORD))) != SCCERR_OK)
        {
            DAGetErrorString(seResult, _szError, sizeof(_szError));
            fprintf(stderr, "DASetOption() failed: %s (0x%04X)\n", _szError, seResult);
            DADeInit();
            goto ErrorExit;
        }
    }

	if (Opts.abSetOptions[SAMPLE_OPTION_EMAILHEADER])
	{
		if ((seResult = DASetOption((VTHDOC)NULL, SCCOPT_WPEMAILHEADEROUTPUT, (VTLPVOID)&Opts.dwEmailHeader, sizeof(VTDWORD))) != SCCERR_OK)
		{
			DAGetErrorString(seResult, _szError, sizeof(_szError));
            fprintf(stderr, "DASetOption() failed: %s (0x%04X)\n", _szError, seResult);
            DADeInit();
            goto ErrorExit;
		}
	}

    SetBufferSize((VTHDOC)NULL, &Opts);   /* Set the IO Buffer sizes (if required) before DAOpenDocument() */


    /* Set the option for the temp file callback */
    if (Opts.abSetOptions[SAMPLE_OPTION_REDIRECTTEMPFILE])
    {
        if (Opts.bRedirectTempFile == TRUE)
        {
            pCallbackFunc = &pRedirtTempFileCallbackFunc;
            seResult = DASetOption((VTHDOC)NULL, SCCOPT_REDIRECTTEMPFILE, (VTLPVOID)(&pCallbackFunc), sizeof(REDIRECTTEMPFILECALLBACKPROC));
        }
    }

    /* Set the file access callback */
    if (Opts.abSetOptions[SAMPLE_OPTION_PASSWORD] || Opts.abSetOptions[SAMPLE_OPTION_PASSWORD_U] || Opts.abSetOptions[SAMPLE_OPTION_NOTESID])
        DASetFileAccessCallback(MyFileAccessCallback);
    /*
    |   Note that when IOTYPE_REDIRECT is specified, the pSpec parameter is a
    |   pointer to a MyFile data structure containing an already open file
    |   handle and all of the new redirected I/O functions.  See MyOpen().
    */
    seResult = DAOpenDocument(&hDoc, IOTYPE_REDIRECT, (VTLPVOID)pInFile, 0);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DAOpenDocument() failed: %s (0x%04X)\n", _szError, seResult);
        DADeInit();
        goto ErrorExit;
    }

	/* if desired, enable the stat callback. */
	if (Opts.abSetOptions[SAMPLE_OPTION_NUMBEROFSTATCALLBACKS] && Opts.dwNumStatCallbacks > 0)
    {
		seResult = DASetStatCallback(MyStatCallBack, (VTHANDLE) hDoc, (VTLPVOID)(&gNumStatCallbacks) );
		if (seResult != SCCERR_OK)
		{
			DAGetErrorString(seResult, _szError, sizeof(_szError));
			fprintf(stdout, "DASetStatCallback() failed: %s (0x%04X)\n", _szError, seResult);
		}
	}

    /* Set the rest of the options */
    seResult = SetOptions(hDoc, &Opts);

    if (seResult == SCCERR_OK)
    {
        /* This sample app is not Unicode callback string enabled at this time. */
        seResult = DASetOption(hDoc, SCCOPT_EX_UNICODECALLBACKSTR,
                               (VTLPVOID)&bUnicodeCallbackStr, sizeof(VTBOOL));
    }

    /*
    |   Override the default template handling in cfg.c by setting it again,
    |   this time re-directing the I/O for the template files
    */
    if ((seResult == SCCERR_OK) && Opts.abSetOptions[SAMPLE_OPTION_TEMPLATE])
    {
        LPMYFILE pTemplateSpec = MyOpen(IO_READ, Opts.szTemplate);
        seResult = DASetFileSpecOption(hDoc, SCCOPT_EX_TEMPLATE, IOTYPE_REDIRECT, (VTLPVOID)pTemplateSpec);
    }

    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DASetOption() failed: %s (0x%04X)\n", _szError, seResult);
        DACloseDocument(hDoc);
        DADeInit();
        goto ErrorExit;
    }

    PrintOpts(&Opts);
#ifdef WIN32DEBUG
    GetLocalTime(&st);
    /* printf("%d/%02d/%d  %d:%02d:%02d - Start of conversion\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond); */
    printf("%d:%02d:%02d - Start of conversion\n", st.wHour, st.wMinute, st.wSecond);
#endif


    /*
    |   EXOpenExport() the document to be converted (given by Opts.szInFile).  Required
    |
    |   In the EXOpenExport() call, the pSpec must be either NULL or a pointer
    |   to a buffer whose content is determined by the dwSpecType flag.  Since
    |   we want the EX_CALLBACK_ID_CREATENEWFILE callback to specify the initial
    |   output file, we will pass a NULL.  Note that passing a NULL pSpec
    |   requires that the developer fill in all of the file information in the
    |   callback routine pointed to by parameter #7.  Also, parameter #8 is a
    |   value big enough to hold a pointer (and may be bigger than a DWORD) that
    |   is passed straight to the callback routine.  This application uses the
    |   parameter to pass a pointer to the main output filename (Opts.szOutFile).
    */
    seResult = EXOpenExport(hDoc, Opts.dwOutputID, IOTYPE_REDIRECT, NULL, 0, 0,
                            (EXCALLBACKPROC)ExportOemCallback, (VTSYSPARAM)(&CallbackData),
                            &hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "EXOpenExport() failed: %s (0x%04X)\n", _szError, seResult);
        DACloseDocument(hDoc);
        DADeInit();
        goto ErrorExit;
    }

    /* Do the actual conversion.  Required */
    seResult = EXRunExport(hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "EXRunExport() failed: %s (0x%04X)\n", _szError, seResult);
        EXCloseExport(hExport);
        DACloseDocument(hDoc);
        DADeInit();
        goto ErrorExit;
    }
    else
    {
        gdwFileCount++;

        fprintf(stdout, "Export successful: %d output file(s) created.\n", gdwFileCount);
    }
    if (gdwItemCount)
    {
        fprintf(stderr, "Archive Export: %d item(s) exported.\n", gdwItemCount);
        if (gdwItemExportErrors)
            fprintf(stderr, "Archive Export failed: %d items(s) failed.\n", gdwItemExportErrors);
    }

    /* Close the export on the document.  Required */
    seResult = EXCloseExport(hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "EXCloseExport() failed: %s (0x%04X)\n", _szError, seResult);
        DACloseDocument(hDoc);
        DADeInit();
        goto ErrorExit;
    }

    /* Close the document.  Required */
    seResult = DACloseDocument(hDoc);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DACloseDocument() failed: %s (0x%04X)\n", _szError, seResult);
        DADeInit();
        goto ErrorExit;
    }

    /* Shut down the Data Access module.  Required */
    seResult = DADeInit();
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DADeInit() failed: %s (0x%04X)\n", _szError, seResult);
    }

ErrorExit:
    Cleanup(&Opts);

    return(seResult);
}




/*
|
|   function:   ExportOemCallback
|   parameters: VTHEXPORT   hExport
|               VTSYSPARAM  dwCallbackData
|               VTDWORD     dwCommandID
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This callback routine is the entry point for all of the OEM
|               callback functions.
*/

SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData, VTDWORD dwCommandID, VTLPVOID pCommandOrInfoData)
{
    SCCERR seResult = SCCERR_NOTHANDLED;
    int i;
    UNUSED(hExport);

    for (i = 0; i < NUMHANDLERS; i++)
    {
        if (dwCommandID == _aCBHandlers[i].dwID)
        {
            seResult = _aCBHandlers[i].pfn(dwCallbackData, pCommandOrInfoData);
            break;
        }
    }
    return seResult;
}




/*
|
|   function:   HandleCreateNewFile
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function is called any time a new file is created.  First,
|               it increments gdwFileCount, a global var that keeps track of
|               how many output files have been created.  The other purpose of
|               the callback routine is to allow the developer to fill in the
|               name and details of an output file before it is created.  While
|               that can be a useful function in any case, it is essential when
|               using redirected I/O.  If the pSpec passed to EXOpenExport is
|               NULL, it is up to the developer to fill in all of the file
|               details in this function.  If this function returns SCCERR_OK
|               then the pSpec, dwSpecType, and szURLString must contain valid
|               data.  In this case, pSpec points to a newly opened MYFILE.
|
*/

SCCERR HandleCreateNewFile(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    SCCERR      seResult = SCCERR_OK;
    VTCHAR      szPath[VT_MAX_URL] = { '\0' };
    VTCHAR      szFile[VT_MAX_URL] = { '\0' };
    VTLPSTR     ptr, ptr2, pDirSep;
    VTLPSTR     pExt = NULL;        /* Points to where we should add a file extension. */
    VTLPSTR     pBaseName = NULL;   /* Points to the base name of the output file. */
    VTBOOL      bImage = FALSE;
    VTBOOL      bZtoQ = FALSE;
    CALLBACKDATASTRUCT      *pExredirData = (CALLBACKDATASTRUCT *)dwCallbackData;
    EXFILEIOCALLBACKDATA    *pFileData    = (EXFILEIOCALLBACKDATA*)pCommandOrInfoData;
    EXURLFILEIOCALLBACKDATA *pURLData     = pFileData->pExportData;

    if (pFileData->dwAssociation != CU_ROOT)
        gdwFileCount++;     /* increment file count */

    if (pFileData->dwAssociation != CU_COPY)
    {
        /*
        |   If we are doing an archive and there was a path given in the
        |   original output file name, skip over it as we copy the file
        |   name into szFile.
        */
        if (pExredirData->dwArchiveStackCount > 0)
        {
            for (pBaseName=szFile,ptr=pExredirData->pstrFile; *ptr; ptr++)
            {
                if (IsDirSep(*ptr))
                    pBaseName = szFile;     /* Start the copy over. */
                else
                    *pBaseName++ = *ptr;
            }

            *pBaseName = '\0';

            /* Add any archive directories to the szPath */
            strcpy(szPath, pExredirData->szArchiveSubDirs);
            strcat(szPath, "/");
        }
        else
        {
            /* Copy the input file name (with path) into szPath. */
            pDirSep = ptr2 = szPath;
            ptr = pExredirData->pstrFile;

            while ((*ptr2 = *ptr) != 0)
            {
                if (IsDirSep(*ptr2))
                    pDirSep = ptr2 + 1;       /* Mark the byte after the last dir seperator. */

                ptr++;
                ptr2++;
            }

            /* Now copy everything after the DirSep to the szFile buf. */
            strcpy(szFile, pDirSep);
            *pDirSep = 0;      /* Null terminate szPath at pDirSep. */
        }


        /* If this is the main output file, just give it the base name, otherwise... */
        if (pFileData->dwAssociation != CU_ROOT )
        {
            /* ...use the base name, file count and file type to build a new filename. */
            for (ptr=szFile; *ptr; ptr++)
            {
                if (IsDirSep(*ptr))
                {
                    pExt = NULL;    /* Reset the extension pointer. */
                }
                else if (*ptr == '.')
                {
                    pExt = ptr;     /* Might be the extension, but keep going. */
                }
            }

            if (pExt == NULL)
            {
                pExt = ptr;     /* Original file didn't have an extension */
            }

            /* Before appending the extension, append the file count to the base file name. */
            sprintf(pExt, "%04x", (int)gdwFileCount);

			switch (pFileData->dwOutputId)
			{
                case FI_HTML:
                case FI_XHTMLB:
                case FI_HTMLWCA:
                case FI_HTMLAG:
                case FI_WIRELESSHTML:
                    strcat(pExt, ".htm");
                    bZtoQ = TRUE;
                    break;

                case FI_HTML_CSS:
                    strcat(pExt, ".css");
                    bZtoQ = TRUE;
                    break;

                case FI_JAVASCRIPT:
                    strcat(pExt, ".js");
                    bZtoQ = TRUE;
                    break;


                case FI_WML:
                    strcat(pExt, ".wml");
                    bZtoQ = TRUE;
                    break;

                case FI_XML:
                    strcat(pExt, ".xml");
                    bZtoQ = TRUE;
                    break;

                case FI_HDML:
                    strcat(pExt, ".hdml");
                    bZtoQ = TRUE;
                    break;

                case FI_CHTML:
                    strcat(pExt, ".chtml");
                    bZtoQ = TRUE;
                    break;

                case FI_TEXT:
                    strcat(pExt, ".txt");
                    bZtoQ = TRUE;
                    break;

				case FI_GIF:
                    strcat(pExt, ".gif");
					if(pFileData->dwParentOutputId != pFileData->dwOutputId)
						bImage = TRUE;
                    break;

				case FI_JPEGFIF:
                    strcat(pExt, ".jpg");
                    if(pFileData->dwParentOutputId != pFileData->dwOutputId)
						bImage = TRUE;
                    break;

				case FI_WBMP:
                    strcat(pExt, ".wbmp");
                    bImage = TRUE;
                    break;

				case FI_PNG:
                    strcat(pExt, ".png");
                    if(pFileData->dwParentOutputId != pFileData->dwOutputId)
						bImage = TRUE;
                    break;

				case FI_TIFF:
                    strcat(pExt, ".tif");
                    if(pFileData->dwParentOutputId != pFileData->dwOutputId)
						bImage = TRUE;
                    break;

                case FI_UNKNOWN:
                    {
                        VTBYTE *pTemp;
                        VTBYTE szExtension[32];

                        memset(szExtension, 0, 32);
                        /* Using the original file extension */
                         pTemp = (VTBYTE *)(pURLData->szURLString + strlen((char *)pURLData->szURLString));
                         while(pTemp != pURLData->szURLString)
                         {
                             if(*pTemp == '.')
                             {
                                 strcpy((char *)szExtension, (char *)pTemp);
                                 break;
                             }
                             else
                                 pTemp--;
                         }
                        strcat((char *)pExt, (char *)szExtension);
                    }
                    break;

				default:
                    seResult = SCCERR_FILECREATE;
            }

            if (bImage)
            {
                /*
                |   For demonstration purposes, we are putting images in an
                |   'images' subdirectory.  Start by creating the directory.
                |   Assume that if mkdir() fails, that the dir already exists.
                |   Note that even though the slash is facing the wrong way
                |   for a windows path, the windows C libraries will still
                |   accept it without problems, and this way the URL in the
                |   output file will follow convention.
                */

                strcat(szPath, "images");

#ifdef UNIX /* UNIX mkdir takes different args than Windows */
                if (mkdir(szPath, 0744) == 0)
#else
                if (mkdir(szPath) == 0)
#endif
                    fprintf(stderr, "Creating directory: %s\n", szPath);

                strcat(szPath, "/");
            }
        }
		else
		{
			/* We are processing the main output file.  bZtoQ should
			 * be TRUE only for text files as binary files would become
			 * hopelessly corrupted.  A side effect of this is that the
			 * ZtoQ option in the config file can never work for Image
			 * Export users.  Since I cannot contrive another way to
			 * prove we are redirecting IO for Image Export, hopefully
			 * reading the sample code will suffice for those users.
			 */
			switch (pFileData->dwOutputId)
			{
				case FI_HTML:
				case FI_XHTMLB:
				case FI_HTMLWCA:
				case FI_HTMLAG:
				case FI_WIRELESSHTML:
				case FI_WML:
				case FI_XML:
				case FI_HDML:
				case FI_CHTML:
				case FI_TEXT:
					bZtoQ = TRUE;
					break;

				/* Added Javascript, css file types because we probably don't
				   want z to q translations in those.
		        */
				case FI_JAVASCRIPT:
				case FI_HTML_CSS:
				case FI_GIF:
				case FI_JPEGFIF:
				case FI_WBMP:
				case FI_PNG:
				case FI_TIFF:
				default:
					bZtoQ = FALSE;
					break;
			}
		}

        /* Append the file name we have to any extra path stuff we have. */
        strcat(szPath, szFile);

        /*
        |   Now set the output type (dwSpecType), open the output file (pSpec),
        |   and set the URL that other files will use to refer to it.
        */
        pFileData->dwSpecType = IOTYPE_REDIRECT;
        pFileData->pSpec = MyOpen((VTBYTE)(bZtoQ ? IO_WRITE_TEXT : IO_WRITE_BINARY), szPath);
    }
    else
    {
        /*
        |   In this case, the file has already been created in the
        |   IOGENSECONDARY IOGetInfo() call (see MyGetInfo() below).
        |   Just retrieve the filename from the spec.
        */
        strcpy(szPath, ((LPMYFILE)pFileData->pSpec)->szFileName);
    }

    URLEncode((VTLPSTR)pURLData->szURLString, szPath);

    if (pFileData->pSpec != NULL)
    {
        fprintf(stderr, "Opening for write: %s\n", szPath);
    }
    else
    {
        seResult = SCCERR_FILECREATE;
    }

    return seResult;
}




/*
|
|   function:   HandleNewFileInfo
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function is called any time a new file is created.  In
|               this program, we're simply displaying a message if the config
|               file indicates that we are to handle this callback.
|
*/

SCCERR HandleNewFileInfo(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    SCCERR seResult = SCCERR_NOTHANDLED;
    UNUSED(dwCallbackData);

    if (Opts.bHandleNewFileInfo)
    {
        EXURLFILEIOCALLBACKDATA* pURLData;
        EXFILEIOCALLBACKDATA* pFileData = (EXFILEIOCALLBACKDATA*)pCommandOrInfoData;
        pURLData = pFileData->pExportData;
        fprintf(stderr, "\tEX_CALLBACK_ID_NEWFILEINFO: %s\n", (VTLPSTR)pURLData->szURLString);
        seResult = SCCERR_OK;
    }
    return seResult;
}

SCCERR HandleOcrError(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    SCCERR *ocrErr = (SCCERR *)pCommandOrInfoData;
    UNUSED(dwCallbackData);

    fprintf(stderr, "\tEX_CALLBACK_ID_OCRERROR: (%04X)\n", *ocrErr);

    return SCCERR_OK;
}


/*
|
|   function:   HandleOEMOutput
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function is called to handle OEM output strings.  The
|               string corresponding to pOEMData->pOutStr is written to the
|               file specified by pOEMData->hFile.
|
*/

SCCERR HandleOEMOutput(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXOEMOUTCALLBACKDATA* pOEMData = (EXOEMOUTCALLBACKDATA*)pCommandOrInfoData;
    SCCERR seResult = SCCERR_NOTHANDLED;
    VTWORD i;
    VTDWORD dwCount;
    UNUSED(dwCallbackData);

    if (Opts.wCMCallbackVersion != 1)
        return seResult;

    for (i = 0; i < Opts.mapOEM.wCount; i++)
    {
        if (stricmp(Opts.mapOEM.aElements[i].szKey, (const char *)pOEMData->pOutStr) == 0)
        {
            IOWrite(pOEMData->hFile, Opts.mapOEM.aElements[i].szVal,
                    (VTDWORD)strlen(Opts.mapOEM.aElements[i].szVal), &dwCount);
            seResult = SCCERR_OK;
            break;
        }
    }
    return seResult;
}




/*
|
|   function:   HandleProcessLink
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function is called in response to a link being converted.
|               The user can specify a particular action (convert, create, or
|               skip), and a URL to output if the action is "create".
|
*/

SCCERR HandleProcessLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXPROCESSLINKCALLBACKDATA* pLinkData = (EXPROCESSLINKCALLBACKDATA*)pCommandOrInfoData;
    UNUSED(dwCallbackData);
    pLinkData->dwAction = Opts.dwLinkAction;
    if (pLinkData->dwAction == EX_ACTION_CREATELINK)
    {
        strcpy(pLinkData->pLocatorStr, Opts.szLinkLocation);
        pLinkData->dwLocatorStrCharset = SO_ASCII;
        pLinkData->dwLinkFlags = EX_FLAG_USEOUTPUTCHARSET;
    }
    return SCCERR_OK;
}




/*
|
|   function:   HandleCustomElementList
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This routine provides the list of custom elements that are
|               available.
|
*/

SCCERR HandleCustomElementList(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    VTLPVOID* pElementData = (VTLPVOID*)pCommandOrInfoData;
    SCCERR seResult = SCCERR_NOTHANDLED;
    KEYVALUE* pKey;
    UNUSED(dwCallbackData);

    if (Opts.mapCustom.wCount > 0)
    {
        VTWORD i, j, k;
        VTWORD wCount = 0;
        VTBOOL bFound;

        for (i = 0; i < Opts.mapCustom.wCount; i++)
        {
            Opts.apstrCustomElements[wCount++] = Opts.mapCustom.aKeys[i].szKey;

            /*
            |   Since the element strings associated with key values also need to be registered,
            |   loop through and add non duplicate entries.
            */
            pKey = &Opts.mapCustom.aKeys[i];
            for (j = 0; j < pKey->wCount; j++)
            {
                bFound = FALSE;

                for (k = 0; k < wCount; k++)
                {
                    if (stricmp(Opts.apstrCustomElements[k], pKey->aElements[j].szElementValue) == 0)
                    {
                        bFound = TRUE;
                        break;
                    }
                }

                if (!bFound)
                {
                    Opts.apstrCustomElements[wCount++] = pKey->aElements[j].szElementValue;
                }
            }
        }
        *pElementData = Opts.apstrCustomElements;
        seResult = SCCERR_OK;
    }
    return seResult;
}




/*
|
|   function:   HandleProcessElementStr
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function handles custom element processing.  The string
|               corresponding to pCustomData->pKeyStr is written to the output
|               file.  Note that in this demo, pCustomData->pElementStr is
|               ignored.
|
*/

SCCERR HandleProcessElementStr(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXCUSTOMELEMENTCALLBACKDATA* pCustomData = (EXCUSTOMELEMENTCALLBACKDATA*)pCommandOrInfoData;
    SCCERR seResult = SCCERR_NOTHANDLED;
    VTWORD i;
    VTDWORD dwCount;
    KEYVALUE* pKey;
    UNUSED(dwCallbackData);

    if (Opts.wCMCallbackVersion != 1)
        return seResult;

    for (i = 0; i < Opts.mapCustom.wCount; i++)
    {
        pKey = &Opts.mapCustom.aKeys[i];

        if ((stricmp(pKey->szKey, pCustomData->pKeyStr) == 0))
        {
            if (strlen(pCustomData->pElementStr) != 0)
            {
                int j;

                for (j = 0; j < pKey->wCount; j++)
                {
                    if ((stricmp(pKey->aElements[j].szElementValue, pCustomData->pElementStr) == 0))
                    {
                        IOWrite(pCustomData->hFile, pKey->aElements[j].szVal,
                                (VTDWORD)strlen(pKey->aElements[j].szVal), &dwCount);
                        seResult = SCCERR_OK;
                        break;
                    }
                }
            }
            else
            {
                IOWrite(pCustomData->hFile, pKey->szVal, (VTDWORD)strlen(pKey->szVal), &dwCount);
                seResult = SCCERR_OK;
            }
            break;
        }
    }
    return seResult;
}




/*
|
|   function:   HandleGraphicExportFailure
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function is called when an error has occurred in exporting
|               a graphic.  For the purposes of this demo, we simply display the
|               error message corresponding to the error code.  Since we don't
|               write an image to pInfo->hFile, SCCERR_NOTHANDLED is returned.
|
*/

SCCERR HandleGraphicExportFailure(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXGRAPHICEXPORTINFO* pInfo = (EXGRAPHICEXPORTINFO*)pCommandOrInfoData;
    UNUSED(dwCallbackData);
    DAGetErrorString(pInfo->ExportGraphicStatus, _szError, sizeof(_szError));
    fprintf(stderr, "\tEX_CALLBACK_ID_GRAPHICEXPORTFAILURE: %s (0x%04X)\n",
            _szError, pInfo->ExportGraphicStatus);
    return SCCERR_NOTHANDLED;
}




/*
|
|   function:   HandleOEMOutput2
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function is called to handle OEM output strings.  The
|               string corresponding to pOEMData->pOutStr is written to the
|               buffer in the EXOEMOUTCALLBACKDATA_VER2 structure.
|
*/

SCCERR HandleOEMOutput2(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXOEMOUTCALLBACKDATA_VER2* pOEMData = (EXOEMOUTCALLBACKDATA_VER2*)pCommandOrInfoData;
    SCCERR seResult = SCCERR_NOTHANDLED;
    VTWORD i;
    UNUSED(dwCallbackData);

    if (Opts.wCMCallbackVersion != 2)
        return seResult;

    for (i = 0; i < Opts.mapOEM.wCount; i++)
    {
        if (stricmp(Opts.mapOEM.aElements[i].szKey, pOEMData->pOEMString) == 0)
        {
            pOEMData->dwSize    = sizeof(EXOEMOUTCALLBACKDATA_VER2);
            pOEMData->dwLength  = EXwcslen(Opts.mapOEM.aElements[i].wzVal);
            pOEMData->dwCharset = Opts.abSetOptions[SAMPLE_OPTION_CMCALLBACKCHARSET] ? Opts.dwCMCallbackCharset : SO_UNICODE;
            pOEMData->pwBuffer  = Opts.mapOEM.aElements[i].wzVal;

            seResult = SCCERR_OK;
            break;
        }
    }
    return seResult;
}




/*
|
|   function:   HandleProcessElementStr2
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function handles custom element processing.  The string
|               corresponding to pCustomData->pKeyStr, pCustomData->ElementStr pair
|               is written to the output file.
|
*/

SCCERR HandleProcessElementStr2(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXCUSTOMELEMENTCALLBACKDATA_VER2* pCustomData = (EXCUSTOMELEMENTCALLBACKDATA_VER2*)pCommandOrInfoData;
    SCCERR seResult = SCCERR_NOTHANDLED;
    VTWORD i, j;
    KEYVALUE* pKey;
    UNUSED(dwCallbackData);

    if (Opts.wCMCallbackVersion != 2)
        return seResult;

    for (i = 0; i < Opts.mapCustom.wCount; i++)
    {
        pKey = &Opts.mapCustom.aKeys[i];

        if (stricmp(pKey->szKey, pCustomData->pKeyStr) == 0)
        {
            if (strlen(pCustomData->pElementStr) == 0)
            {
                pCustomData->dwSize     = sizeof(EXCUSTOMELEMENTCALLBACKDATA_VER2);
                pCustomData->dwCharset  = Opts.abSetOptions[SAMPLE_OPTION_CMCALLBACKCHARSET] ? Opts.dwCMCallbackCharset : SO_UNICODE;
                pCustomData->dwLength   = EXwcslen(pKey->wzVal);
                pCustomData->pwBuffer   = pKey->wzVal;
                seResult = SCCERR_OK;
            }
            else
            {
                for (j = 0; j < pKey->wCount; j++)
                {
                    if (stricmp(pKey->aElements[j].szElementValue, pCustomData->pElementStr) == 0)
                    {
                        pCustomData->dwSize     = sizeof(EXCUSTOMELEMENTCALLBACKDATA_VER2);
                        pCustomData->dwCharset  = Opts.abSetOptions[SAMPLE_OPTION_CMCALLBACKCHARSET] ? Opts.dwCMCallbackCharset : SO_UNICODE;
                        pCustomData->dwLength   = EXwcslen(pKey->aElements[j].wzVal);
                        pCustomData->pwBuffer   = pKey->aElements[j].wzVal;
                        seResult = SCCERR_OK;
                        break;
                    }
                }
            }
            break;
        }
    }
    return seResult;
}




SCCERR HandleAltLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXALTLINKCALLBACKDATA* pALData = (EXALTLINKCALLBACKDATA*)pCommandOrInfoData;
    UNUSED(dwCallbackData);

    if (!Opts.abSetOptions[SAMPLE_OPTION_ALTLINK_PREV + pALData->dwType])
        return SCCERR_NOTHANDLED;

    strncpy(pALData->pAltURLStr, Opts.aszAltLinkURLs[pALData->dwType], 1024);

    return SCCERR_OK;
}


/*
|
|   function:   HandleEnterArchive
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function is called to when we enter archive
|
*/

SCCERR HandleEnterArchive(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    VTCHAR                      *ptr;
    VTWORD                      *wPtr, *wFileName;
    CALLBACKDATASTRUCT          *pExredirData      = (CALLBACKDATASTRUCT *)dwCallbackData;
    EXENTERARCHIVECALLBACKDATA  *pEnterArchiveData = (EXENTERARCHIVECALLBACKDATA*)pCommandOrInfoData;

    /*
    |   Because this is only a sample app designed to demonstrate how
    |   to use Export, a couple of corners have been cut in this
    |   routine:
    |   1) File names are sloppily translated from Unicode when
    |      creating directory names.
    |   2) No checks are made to prevent overrunning the
    |      szArchiveSubDirs buffer.
    */

    GetOutputPath(pExredirData);

    pExredirData->dwArchiveStackCount++;

    if (pExredirData->dwArchiveStackCount <= MAX_ARCHIVE_STACK_COUNT)
    {
        /*
        |   Put the files generated by the export of this archive in a
        |   directory whose name is based on the wzFullName and
        |   dwItemNum of the archive entry.  wzFullName is in Unicode,
        |   to avoid problems with generating dir names in Unicode,
        |   translate all problem characters to tildes ('~').
        */

        ptr = pExredirData->pArchiveStack[pExredirData->dwArchiveStackCount-1];

        if (!IsDirSep(*(ptr-1)))
        {
            *ptr++ = '/';
        }

        /*
        |   To avoid naming conflicts with archive entries of the same
        |   file name, use the item number as a prefix to all the dir names.
        */
        sprintf(ptr, "%d_", pEnterArchiveData->dwItemNum);
        while (*ptr)
            ptr++;

        /* Chop off any path info from wzFullName */
        wFileName = wPtr = pEnterArchiveData->wzFullName;
        while (*wPtr)
        {
            if (IsDirSep(*wPtr))
                wFileName = wPtr+1;

            wPtr++;
        }

        for (wPtr=wFileName; *wPtr; wPtr++)
        {
            /*
            |   These characters seem troublesome on one OS or another
            |   to me...
            */
            if ((*wPtr  > '~') || (*wPtr  < ' ')  ||
                (*wPtr == '"') || (*wPtr == '\'') ||
                (*wPtr == '*') || (*wPtr == '?')  ||
                (*wPtr == '<') || (*wPtr == '>')  ||
                (*wPtr == '&') || (*wPtr == '|'))
            {
                *ptr++ = '~';
            }
            else
            {
                *ptr++ = (VTCHAR)*wPtr;
            }
        }

        *ptr = '\0';
        pExredirData->pArchiveStack[pExredirData->dwArchiveStackCount] = ptr;

#ifdef UNIX /* UNIX mkdir takes different args than Windows */
        if (mkdir(pExredirData->szArchiveSubDirs, 0744) == 0)
#else
        if (mkdir(pExredirData->szArchiveSubDirs) == 0)
#endif
            fprintf(stderr, "Creating directory: %s\n", pExredirData->szArchiveSubDirs);
    }
	pEnterArchiveData->pSpec = NULL;
    pEnterArchiveData->dwSpecType = IOTYPE_REDIRECT;
    return SCCERR_OK;
}




/*
|
|   function:   HandleLeaveArchive
|   parameters: VTSYSPARAM  dwCallbackData
|               VTLPVOID    pCommandOrInfoData
|   returns:    SCCERR
|
|   purpose:    This function is called to when we leave archive
|
*/

SCCERR HandleLeaveArchive(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    CALLBACKDATASTRUCT          *pExredirData      = (CALLBACKDATASTRUCT *)dwCallbackData;
    EXLEAVEARCHIVECALLBACKDATA  *pLeaveArchiveData = (EXLEAVEARCHIVECALLBACKDATA*)pCommandOrInfoData;

    DAGetErrorString(pLeaveArchiveData->ExportResult, _szError, sizeof(_szError));
    if (pLeaveArchiveData->ExportResult)
    {
        fprintf(stderr, "\tEX_CALLBACK_ID_LEAVEARCHIVE: %s (0x%04X)\n", _szError, pLeaveArchiveData->ExportResult);
        gdwItemExportErrors ++;
    }
    gdwItemCount ++;

    *pExredirData->pArchiveStack[pExredirData->dwArchiveStackCount] = '\0';
    pExredirData->dwArchiveStackCount--;

    return SCCERR_OK;
}




SCCERR HandleRefLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    VTWORD  *wPtr;
    VTCHAR  *pOut;
    EXREFLINKCALLBACKDATA* pRefLinkData = (EXREFLINKCALLBACKDATA*)pCommandOrInfoData;
    SCCERR  seResult = SCCERR_NOTHANDLED;
    UNUSED(dwCallbackData);

    if (Opts.abSetOptions[SAMPLE_OPTION_REFLINK] && Opts.bRefLink)
    {
        pOut = pRefLinkData->URL;

        for (wPtr=pRefLinkData->wzFullName; *wPtr; wPtr++)
            pOut = FakeURLEncode(pOut, *wPtr);

        *pOut = '\0';

        strcat(pRefLinkData->URL, "=");
        strcat(pRefLinkData->URL, pRefLinkData->pSubdocSpec);

        seResult = SCCERR_OK;
    }

    return seResult;
}




VTVOID URLEncode(VTLPSTR pOut, VTLPSTR pIn)
{
    /* URLEncode pIn and write the results to pOut. */
    while (*pIn)
    {
        if ( ((*pIn >= 'a') && (*pIn <= 'z')) ||
             ((*pIn >= 'A') && (*pIn <= 'Z')) ||
             ((*pIn >= '0') && (*pIn <= '9')) ||
              (*pIn == '$') || (*pIn == '-')  ||
              (*pIn == '_') || (*pIn == '.')  ||
              (*pIn == '+') || (*pIn == '!')  ||
              (*pIn == '*') || (*pIn == '\'') ||
              (*pIn == '(') || (*pIn == ')')  )
        {
            /* Legal URL character. */
            *pOut++ = *pIn++;
        }
        else if (IsDirSep(*pIn))
        {
            /* Not a legal URL char, but don't worry about it. */
            *pOut++ = *pIn++;
        }
        else
        {
            *pOut++ = '%';
            *pOut++ = gpHex[(*pIn & 0xF0) >> 4];
            *pOut++ = gpHex[*pIn & 0x0F];

            pIn++;
        }
    }

    *pOut = '\0';
}




VTCHAR *FakeURLEncode(VTCHAR *pOut, VTWORD wChar)
{
    VTCHAR  c;

    /*
    |   True URL encoding would translate this to UTF-8 rather than
    |   just writing it as two successive bytes.  Since this is only a
    |   demonstration though, we won't worry about that.
    */
    if (wChar > 0xFF)
    {
        c = (VTCHAR)(wChar >> 8);

        if ( ((c >= 'a') && (c <= 'z')) ||
             ((c >= 'A') && (c <= 'Z')) ||
             ((c >= '0') && (c <= '9')) ||
              (c == '$') || (c == '-')  ||
              (c == '_') || (c == '.')  ||
              (c == '+') || (c == '!')  ||
              (c == '*') || (c == '\'') ||
              (c == '(') || (c == ')')  )
        {
            *pOut++ = c;
        }
        else
        {
            *pOut++ = '%';
            *pOut++ = gpHex[(c & 0xF0) >> 4];
            *pOut++ = gpHex[c & 0x0F];
        }
    }

    c = (VTCHAR)(wChar & 0x00FF);

    if ( ((c >= 'a') && (c <= 'z')) ||
         ((c >= 'A') && (c <= 'Z')) ||
         ((c >= '0') && (c <= '9')) ||
          (c == '$') || (c == '-')  ||
          (c == '_') || (c == '.')  ||
          (c == '+') || (c == '!')  ||
          (c == '*') || (c == '\'') ||
          (c == '(') || (c == ')')  )
    {
        *pOut++ = c;
    }
    else
    {
        *pOut++ = '%';
        *pOut++ = gpHex[(c & 0xF0) >> 4];
        *pOut++ = gpHex[c & 0x0F];
    }

    return pOut;
}




VTVOID GetOutputPath(CALLBACKDATASTRUCT *pExredirData)
{
    VTCHAR  *ptr, *ptr2;
    VTCHAR  *szBaseName;

    /*
    |   This gets the absolute path to the initial output file and
    |   stores it in szArchiveSubDirs.  When putting files into
    |   subdirectories, we use absolute paths to prevent problems with
    |   using the same path string to handle links down into the sub
    |   directory and links between files within a sub directory.
    |   The resulting path may or may not end in a slash.
    */

    if (!*pExredirData->szArchiveSubDirs)
    {
        /* We haven't done this already. */
        for (szBaseName=ptr=pExredirData->pstrFile; *ptr; ptr++)
        {
            if (IsDirSep(*ptr))
                szBaseName = ptr + 1;
        }

        if (szBaseName == pExredirData->pstrFile)
        {
            /*
            |   The output file name we got does not include an
            |   absolute path, get one.
            */
#ifdef UNIX
            getcwd(pExredirData->szArchiveSubDirs, VT_MAX_URL);
#else
            VTCHAR buf[VT_MAX_URL];

            if (_getcwd(buf, VT_MAX_URL) != NULL)
            {
                /*
                |   Skip over the drive letter to prevent confusion in
                |   the browsers caused by URL encoding.
                */
                strcpy(pExredirData->szArchiveSubDirs, strchr(buf, ':') + 1);
            }
            else
            {
                /* getcwd() had an error */
                pExredirData->szArchiveSubDirs[0] = '\0';
            }
#endif
        }
        else
        {
            /* Copy the path part of the output file to szArchiveSubDirs. */
            ptr2 = pExredirData->szArchiveSubDirs;
            for (ptr = pExredirData->pstrFile; ptr != szBaseName; ptr++, ptr2++)
                *ptr2 = *ptr;
            *ptr2 = '\0';
        }

        pExredirData->pArchiveStack[0] = pExredirData->szArchiveSubDirs + strlen(pExredirData->szArchiveSubDirs);
    }
}




/*********************************************************************************
|
|   These functions are the Redirected I/O functions.  These are trivial examples
|   and merely redirect the I/O through the C runtime FILE routines (fopen, fread,
|   etc...)
|
|   With the exception of pOpen, the prototypes and descriptions for these
|   functions are located in the Export documentation.
|
**********************************************************************************/

/*
|
|   function:   MyOpen
|   parameters: VTDWORD         dwRWFlag
|               VTCHAR     *szFileName
|   returns:    LPMYFILE
|
|   purpose:    This function opens all of the files for input and output.
|               It allocates memory for a MyFile struct and fills in the
|               filename, opens the file, and assigns the redirected I/O
|               functions in the sBaseIO struct.  Note that I/O can be
|               redirected on a file by file basis if desired.  If the file
|               is succesfully opened this routine returns a pointer to the
|               MyFile struct, if not it returns NULL.  MyOpen is not a
|               function used by the technology, this is just one example of
|               a way to open files in a program utilizing redirected I/O.
|               The developer is free to open the filehandle/stream any way
|               he/she sees fit.
|
*/

LPMYFILE MyOpen(VTDWORD dwRWFlag, VTLPSTR szFileName)
{
    LPMYFILE pMyFile;
    char * pszOpenType;

    int namelen = (int)strlen(szFileName);

#ifdef WIN32
    int maxpath = MAX_PATH;
#else
    int maxpath = BUF_SIZE;
#endif

    if (namelen >= maxpath)
    {
        fprintf(stderr, "\nError:  Attempted to open file whose name is %d characters long.\n",namelen);
        fprintf(stderr, "        exredir does not support pathnames longer than %d characters.\n\n",maxpath-1);
        fprintf(stderr, "For information about supporting very long pathnames, please consult\n");
        fprintf(stderr, "the source code for the batch_process_ca application (redir.c).\n\n");
        return NULL;
    }

    pMyFile = (LPMYFILE)malloc(sizeof(MYFILE));

    /*
    |   Give the structure a reference to itself.  this will be used later
    |   when we want to free the structure.
    */
    pMyFile->hThis = (VTHANDLE)pMyFile;

    /* initialize the fields in pMyFile */
    pMyFile->szBuf[0] = '\0';
    pMyFile->szSubdocSpec[0] = '\0';

    strcpy(pMyFile->szFileName, szFileName);
    pMyFile->dwFileFlags = 0;

    /* open for reading or writing */
    switch (dwRWFlag)
    {
        case IO_READ:
            pszOpenType = "rb";
            break;

        case IO_WRITE_BINARY:
        case IO_WRITE_TEXT:
            pszOpenType = "w+b";
            break;

        default:
            /* if we don't recognize the flag, return NULL, which is treated as an error */
            free(pMyFile);
            return NULL;
    }

    pMyFile->pFile = fopen(pMyFile->szFileName, pszOpenType);
    if (pMyFile->pFile == NULL)
    {
        /*
        |   File open failed, perhaps due to extra parameters appended to the path.
        |   This app supports very limited parameters appended to the input file path,
        |   separated by a question mark.  Currently the only parameter supported
        |   is the "subdocument specification", which allows the call to
        |   DAOpenDocument to directly open an item stored within an archive file.
        |   See the comments for IOGETINFO_SUBDOC_SPEC in MyGetInfo for more
        |   information on what the subdocument specification does.
        */
        VTLPBYTE pParms = (VTLPBYTE)strrchr(pMyFile->szFileName, '?');
        if (pParms)
        {
            /* It is not valid input to append a ? to the path and not specify a
               "subdocument specification." */
            if (*(pParms + 1) == 0 )
            {
                fprintf (stderr, "Invalid input path\n" );
                free(pMyFile);
                pMyFile = NULL;
            }

            else
            {
                /* strip off the parameters and try again. */
                *pParms++ = 0;
                pMyFile->pFile = fopen(pMyFile->szFileName, pszOpenType);

                if (pMyFile->pFile != NULL)
                {
                    /* There was a real file in there. Save the parameters. */
                    strcpy((char *)pMyFile->szSubdocSpec, (char *)pParms);
                }
            }
        }
    }

    if (pMyFile != NULL)
    {
        /* assign redirected I/O routines */
        if (pMyFile->pFile != NULL)
        {
            pMyFile->sBaseIO.pClose = MyClose;
            pMyFile->sBaseIO.pRead = MyRead;

            /*
            |   For this sample application, two different output routines are
            |   being used.  This is because we don't want to convert 'q's to
            |   'z's in graphics files.  Here we specify which one to use based
            |   on whether or not we are writing a text file.
            */
            pMyFile->sBaseIO.pWrite = (dwRWFlag == IO_WRITE_TEXT) ? MyTextWrite : MyBinaryWrite;
            pMyFile->sBaseIO.pSeek = MySeek;
            pMyFile->sBaseIO.pTell = MyTell;
            pMyFile->sBaseIO.pSeek64 = MySeek64;
            pMyFile->sBaseIO.pTell64 = MyTell64;
            pMyFile->sBaseIO.pGetInfo = MyGetInfo;

            /*
            |   pOpen never gets called (we are passing the file handle to the
            |   technology already opened) so this is NULL
            */
            pMyFile->sBaseIO.pOpen = NULL;
        }
        else
        {
            fprintf(stderr, "Could not open input file for %s!\n", (dwRWFlag & IO_READ) ? "reading" : "writing");
            free(pMyFile);
            pMyFile = NULL;
        }
    }
    return pMyFile;
}




IOERR MyClose(HIOFILE hFile)
{
    LPMYFILE pMyFile = (LPMYFILE)hFile;

    IOERR ieResult;
#ifdef WIN32DEBUG
    SYSTEMTIME st;
#endif

    if (pMyFile->pFile != NULL)
    {
        fclose(pMyFile->pFile);
#ifdef WIN32DEBUG
        GetLocalTime(&st);
		/*printf("%d/%02d/%d  %d:%02d:%02d - %s closed\n", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, pMyFile->szFileName); */
		printf("%d:%02d:%02d - %s closed\n", st.wHour, st.wMinute, st.wSecond, pMyFile->szFileName);
#endif
        ieResult = IOERR_OK;
    }
    else
    {
        ieResult = IOERR_BADPARAM;
    }

    /* Do not forget to free the structure allocated when the file is opened */
    free((LPMYFILE)pMyFile->hThis);
    return ieResult;
}


IOERR MyTempFileClose(HIOFILE hFile)
{
    LPMYFILE pMyFile = (LPMYFILE)hFile;
    VTCHAR      szFileName[BUF_SIZE];

    IOERR ieResult;

    if (pMyFile->pFile != NULL)
    {
        fclose(pMyFile->pFile);
        MyGetInfo(hFile, IOGETINFO_FILENAME, szFileName);
        remove(szFileName);
        ieResult = IOERR_OK;
    }
    else
    {
        ieResult = IOERR_BADPARAM;
    }

    /* Do not forget to free the structure allocated when the file is opened */
    free((LPMYFILE)pMyFile->hThis);
    return ieResult;
}

IOERR MyRead(HIOFILE hFile, VTLPBYTE pData, VTDWORD dwSize, VTLPDWORD pdwCount)
{
    LPMYFILE  pMyFile = (LPMYFILE)hFile;
    IOERR ieResult;

		/*
			Get this, the following two lines used to be combined but this caused an
			error where IOERR_OK would be returned but *pdwCount would be 0 ONLY on
			solaris, and ONLY with release builds.  Seperating into two steps fixed
			the problem.
		*/
    *pdwCount = (VTDWORD)fread(pData, sizeof(VTBYTE), dwSize, pMyFile->pFile);
    if (*pdwCount != 0)
    {
        ieResult = IOERR_OK;
    }
    else
    {
        if (feof(pMyFile->pFile))
        {
            clearerr(pMyFile->pFile);
            ieResult = IOERR_EOF;
        }
        else
        {
            ieResult = IOERR_UNKNOWN;
        }
    }
    return ieResult;
}




/*
|   Just so that the sample redirected I/O functions aren't completely trivial
|   (and to show that it really does do something) this function can convert all
|   occurrences of the letter 'z' (respecting case) to the letter 'q'.
*/
IOERR MyTextWrite(HIOFILE hFile, VTLPBYTE pData, VTDWORD dwSize, VTLPDWORD pdwCount)
{
    LPMYFILE pMyFile = (LPMYFILE)hFile;
    VTLPSTR pstrSrc = (VTLPSTR)pData;
    VTLPSTR pstrDest = pMyFile->szBuf;
    VTDWORD i;
    VTDWORD dwCount = 0;
    VTBOOL bFlush = FALSE;
    IOERR ieResult = IOERR_OK;

    *pdwCount = 0;
    for (i = 0; i < dwSize; i++)
    {
        if (Opts.abSetOptions[SAMPLE_OPTION_ZTOQ] && Opts.bZtoQ)
        {
            switch (*pstrSrc)
            {
                case 'z':
                    *pstrDest++ = 'q';
                    break;

                case 'Z':
                    *pstrDest++ = 'Q';
                    break;

                default:
                    *pstrDest++ = *pstrSrc;
            }
        }
        else
        {
            *pstrDest++ = *pstrSrc;
        }
        pstrSrc++;

        bFlush = (++dwCount == sizeof(pMyFile->szBuf)) || (i == dwSize - 1);

        /* write out the text when the buffer is full */
        if (bFlush)
        {
            if (fwrite(pMyFile->szBuf, sizeof(VTCHAR), dwCount, pMyFile->pFile) == dwCount)
            {
                *pdwCount += dwCount;
                pstrDest = pMyFile->szBuf;
                dwCount = 0;
            }
            else
            {
                ieResult = IOERR_UNKNOWN;
                break;
            }
        }
    }
    return ieResult;
}




IOERR MyBinaryWrite(HIOFILE hFile, VTLPBYTE pData, VTDWORD dwSize, VTLPDWORD pdwCount)
{
    LPMYFILE pMyFile = (LPMYFILE)hFile;
    return ((*pdwCount = (VTDWORD)fwrite(pData, sizeof(VTBYTE), dwSize, pMyFile->pFile)) == dwSize) ? IOERR_OK : IOERR_UNKNOWN;
}




IOERR MySeek(HIOFILE hFile, VTWORD wFrom, VTLONG lOffset)
{
    LPMYFILE  pMyFile = (LPMYFILE)hFile;
    int nOrigin;
    IOERR ieResult;

    if (pMyFile->pFile != NULL)
    {
        switch (wFrom)
        {
            case IOSEEK_CURRENT:
                nOrigin = SEEK_CUR;
                break;

            case IOSEEK_BOTTOM:
                nOrigin = SEEK_END;
                break;

            case IOSEEK_TOP:
            default:
                nOrigin = SEEK_SET;
        }
        if (fseek(pMyFile->pFile, lOffset, nOrigin) == 0)
			ieResult = IOERR_OK;
		else
			ieResult = IOERR_UNKNOWN;
    }
    else
    {
        ieResult = IOERR_BADPARAM;
    }
    return ieResult;
}




IOERR MyTell(HIOFILE hFile, VTLPDWORD pOffset)
{
    LPMYFILE  pMyFile = (LPMYFILE)hFile;
    IOERR ieResult;

    if (pMyFile->pFile != NULL)
    {
        *pOffset = (VTDWORD)ftell(pMyFile->pFile);
        ieResult = IOERR_OK;
    }
    else
    {
        ieResult = IOERR_BADPARAM;
    }
    return ieResult;
}


IOERR MySeek64(HIOFILE hFile, VTWORD wFrom, VTOFF_T Offset)
{
	/* Defer to 32-bit MySeek function
	   (64-bit seeking will not be supported by this sample application) */
	return MySeek( hFile, wFrom, (VTLONG)Offset );
}


IOERR MyTell64(HIOFILE hFile, VTOFF_T *  Offset)
{
	/* Defer to 32-bit MyTell function
	   (64-bit seeking will not be supported by this sample application) */
	VTDWORD dwOffset = 0;
	IOERR ieResult = MyTell( hFile, &dwOffset );
	*Offset = (VTOFF_T)dwOffset;
	return ieResult;
}




IOERR MyGetInfo(HIOFILE hFile, VTDWORD dwInfoID, VTLPVOID pInfo)
{
    LPMYFILE pMyFile = (LPMYFILE)hFile;
    IOERR ieResult = IOERR_OK;
    PIOGENSECONDARY pGenSec;
    VTCHAR szPath[BUF_SIZE];
    VTLPSTR pstrStart;
    VTLPSTR pstrScan;
    VTCHAR ch;

    switch (dwInfoID)
    {
        case IOGETINFO_PATHNAME:
            if (strlen(pMyFile->szFileName) >= MAX_PATH)
                ieResult = IOERR_INSUFFICIENTBUFFER;
            else
                strcpy((VTLPSTR)pInfo, pMyFile->szFileName);
            break;

        case IOGETINFO_FILENAME:
            /* Find just the file name part of the path. */
            pstrStart = pstrScan = pMyFile->szFileName;

            while (*pstrScan != '\0')
                pstrScan++;

            for (; pstrScan != pstrStart; pstrScan--)
            {
                ch = *(pstrScan - 1);
                if (IsDirSep(ch))
                    break;
            }
            if (strlen(pstrScan) >= MAX_PATH)
                ieResult = IOERR_INSUFFICIENTBUFFER;
            else
                strcpy((VTLPSTR)pInfo, pstrScan);
            break;

        case IOGETINFO_ISOLE2STORAGE:
            ieResult = IOERR_FALSE;
            break;

        case IOGETINFO_GENSECONDARY:
            pGenSec = (PIOGENSECONDARY)pInfo;

            /*
            |   Build a new path using the path of the original file and the file name requested
            |   by the viewer.
            */
            strncpy(szPath, pMyFile->szFileName, BUF_SIZE);
            pstrStart = pstrScan = szPath;

            while (*pstrScan != 0x00)
                pstrScan++;

            for (; pstrScan != pstrStart; pstrScan--)
            {
                ch = *(pstrScan - 1);
                if (IsDirSep(ch))
                    break;
            }
            strcpy(pstrScan, (VTLPSTR)pGenSec->pFileName);

            /* Generate a new redirected IO */
            pGenSec->pSpec = MyOpen(((pGenSec->dwOpenFlags & IOOPEN_READ) == IOOPEN_READ) ? IO_READ : IO_WRITE_BINARY, szPath);
            pGenSec->dwSpecType = IOTYPE_REDIRECT;
            if (pGenSec->pSpec == NULL)
            {
                ieResult = IOERR_BADINFOID;
            }
            break;

        case IOGETINFO_PATHTYPE:
            /*
            |   We're using redirected I/O for processing the output files,
            |   not for customizing file access.  Because of this the path type
            |   for all files is IOTYPE_ANSIPATH.
            */
#ifdef UNIX
	        *(VTLPDWORD)pInfo = IOTYPE_UNIXPATH;
#else
            *(VTLPDWORD)pInfo = IOTYPE_ANSIPATH;
#endif
            break;

        case IOGETINFO_SUBDOC_SPEC:
            /*
            |   This message retrieves the "subdocument specification", if any, for
            |   the current document.  This message will only be called once, during
            |   the DAOpenDocument function, to see if you actually want to open a
            |   subdocument within the specified document, rather than the specified
            |   document itself.
            |
            |   An example is the case where you wish to directly open a compressed
            |   item within an archive file.  (This is the only currently supported type
            |   of subdocument specification.)
            |
            |   A subdocument specification for an archive item has the syntax
            |   "item.<num>", where <num> represents the 1-based number of the item to
            |   be opened.  For example, if the current file being opened is a .ZIP file
            |   containing 5 items, and you want to your DAOpenDocument call to return
            |   a handle to the 3rd item within the .ZIP file, you would respond to this
            |   GetInfo message with the string "item.3".  If that 3rd item in the ZIP
            |   file is itself an archive, and you want DAOpenDocument to return a handle
            |   to the first item inside that file (which is itself the 3rd item in the
            |   "current" file) you'd return a subdocument spec of "item.3.1".  Any level
            |   of nested archives is supported; the item numbers should start from the
            |   outermost archive, and be separated from each other with '.' characters.
            */
            strcpy(pInfo, pMyFile->szSubdocSpec);
            break;

        case IOGETINFO_ISDELETEONCLOSE:
            if (pMyFile->dwFileFlags & IOOPEN_DELETEONCLOSE)
                *(VTBOOL *)pInfo = TRUE;
            else
                *(VTBOOL *)pInfo = FALSE;
            break;
        case IOGETINFO_64BITIO:
             ieResult = IOERR_FALSE;
             break;
        default:
            ieResult = IOERR_BADINFOID;
    }
    return ieResult;
}




VTSHORT EXwcslen(VTLPWORD pString)
{
    VTSHORT sLen = 0;
    while (*pString++ != 0)
    {
        sLen++;
    }
    return sLen;
}


VTDWORD pRedirtTempFileCallbackFunc(HIOFILE *phFile,  VTLPVOID pSpec,  VTDWORD dwFileFlags)
{
    LPMYFILE pIOFile;
    char tempFileName[256];
    char szInFile[256];
    char * pch = NULL;
    pIOFile = (LPMYFILE)malloc(sizeof(MYFILE));
    memset(pIOFile, 0, sizeof(MYFILE));
    memset(tempFileName, 0, 256);
    memset(szInFile, 0, 256);

    /* if creating a temp file on disk, make sure it has a unique name, for example archive files
        could have the same filename in different locations */
    if (pIOFile)
    {
        fprintf(stderr, "***************************************\n");
        fprintf(stderr, "Setting temp file call back function \n");

        pIOFile->hThis = (VTHANDLE)pIOFile;
        if ((pSpec == NULL) || (*(VTLPBYTE)pSpec == 0))
        {
            pch = strrchr(Opts.szInFile, '\\');
            if (pch == NULL)
                pch =  strrchr(Opts.szInFile, '/');
            if (pch != NULL)
            {
                pch ++;
                strcpy(szInFile, pch);
            }
            else
                strcpy(szInFile, Opts.szInFile);
            sprintf (tempFileName, "%s%d.tmp", szInFile, gdwTempFileCount);
            pIOFile->pFile = fopen(tempFileName, "w+b");
            if (pIOFile->pFile == NULL)
            {
                /* The tempFileName may have some DBCS characters which can not be opened,
                |  we need to update it to the normal temp name
                */
                memset(tempFileName, 0, 256);
                sprintf (tempFileName, "%s%d.tmp", "mytemp", gdwTempFileCount);
                pIOFile->pFile = fopen(tempFileName, "w+b");
                if (pIOFile->pFile == NULL)
                {
                    fprintf(stderr, "the file %s can not be opened, check it \n", tempFileName);
                    return SCCERR_NOTHANDLED;
                }
            }
            strncpy(pIOFile->szFileName, tempFileName, BUF_SIZE);
            fprintf(stderr, "the temp file is %s\n", tempFileName);
        }
        else
        {
            sprintf(tempFileName, "%s%d.tmp", pSpec, gdwTempFileCount);
            pIOFile->pFile = fopen(tempFileName, "w+b");
            if (pIOFile->pFile == NULL)
            {
                memset(tempFileName, 0, 256);
                sprintf (tempFileName, "%s%d.tmp", "mytemp", gdwTempFileCount);
                pIOFile->pFile = fopen(tempFileName, "w+b");
                if (pIOFile->pFile == NULL)
                {
                    fprintf(stderr, "the file %s can not be opened, check it \n", tempFileName);
                    return SCCERR_NOTHANDLED;
                }
            }
            strncpy(pIOFile->szFileName, tempFileName, BUF_SIZE);
            fprintf(stderr, "the temp file is %s\n", tempFileName);
        }
        gdwTempFileCount ++;

        fprintf(stderr, "***************************************\n");

        pIOFile->sBaseIO.pClose   = MyTempFileClose;
        pIOFile->sBaseIO.pRead    = MyRead;
        pIOFile->sBaseIO.pWrite   = MyBinaryWrite;
        pIOFile->sBaseIO.pSeek    = MySeek;
        pIOFile->sBaseIO.pTell    = MyTell;
        pIOFile->sBaseIO.pGetInfo = MyGetInfo;
        pIOFile->sBaseIO.pOpen    = NULL;
        pIOFile->dwFileFlags = dwFileFlags;
        pIOFile->pSpec = pSpec;
        *phFile = (HIOFILE)pIOFile;
        return SCCERR_OK;
    }
    else
 	    return SCCERR_NOTHANDLED;
}


VTDWORD MyFileAccessCallback (VTDWORD dwID, VTSYSVAL pRequestData, VTSYSVAL pReturnData, VTDWORD dwReturnDataSize)
{
    VTDWORD     dwRet = SCCERR_CANCEL;  /* return value */
    VTLPWORD    lpwszBuf = (VTLPWORD)pReturnData;
    VTDWORD     i, dwLength;
    PIOREQUESTDATA  pIORequestData = (PIOREQUESTDATA)pRequestData;

    switch (dwID)
    {
    case OIT_FILEACCESS_PASSWORD:
        if ( pIORequestData->dwAttemptNumber >= Opts.wPasswordNum)
        {
            /* all the passwords failed, return SCCERR_CANCEL and tell the filter stop to ask for the file access */
            return dwRet;
        }
        else
        {
            /* Copy the password into the request data buffer
             *
             * NOTE: The return data buffer for the OIT_FILEACCESS_PASSWORD
             *       id is a wide-character buffer, so make sure to copy the
             *       characters correctly.
             * If the szPassword is set, use it , otherwise use the Unicode password
             */
            if (strlen(Opts.szPassword[pIORequestData->dwAttemptNumber]))
            {
                dwLength = (VTDWORD)strlen(Opts.szPassword[pIORequestData->dwAttemptNumber]);
                for (i = 0; i <dwLength; i++)
                    lpwszBuf[i] = (VTWORD)Opts.szPassword[pIORequestData->dwAttemptNumber][i];
            }
            else
            {
                dwLength = (VTDWORD)EXwcslen (Opts.wzPassword[pIORequestData->dwAttemptNumber]);
                for (i = 0; i <dwLength; i++)
                    lpwszBuf[i] = (VTWORD)Opts.wzPassword[pIORequestData->dwAttemptNumber][i];
            }
            dwReturnDataSize = i;
            dwRet = SCCERR_OK;
        }
        break;

    case OIT_FILEACCESS_NOTESID:
        if (pIORequestData->dwAttemptNumber >= Opts.wNotesIdNum)
        {
            /* all the notesid failed, return SCCERR_CANCEL and tell the filter stop to ask for the file access */
            return dwRet;
        }
        else
        {
            dwLength = (VTDWORD)strlen(Opts.szNotesId[pIORequestData->dwAttemptNumber]);
            for (i = 0; i <dwLength; i++)
                lpwszBuf[i] = (VTWORD)Opts.szNotesId[pIORequestData->dwAttemptNumber][i];
            dwReturnDataSize = i;
            dwRet = SCCERR_OK;
        }
        break;
    }

    return (dwRet);

}

