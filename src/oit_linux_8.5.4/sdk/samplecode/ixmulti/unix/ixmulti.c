/*
|   Multiple Export
|   Sample Application
|   Demonstrates performing multiple conversions.
|
|   Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.
|
|   You have a royalty-free right to use, modify, reproduce and
|   distribute the Sample Applications (and/or any modified version)
|   in any way you find useful, provided that you agree that Stellent, Inc.
|   has no warranty obligations or liability for any Sample Application
|   files which are modified.
*/

#ifdef WIN32                /* Defined by my Win32 compiler */
#define     WINDOWS         /* Required for Windows */
#include    <windows.h>
#include    <tchar.h>
#include    <io.h>          /* access()  */
#else
#ifndef UNIX
#define     UNIX            /* Required for UNIX */
#endif
#include    <unistd.h>      /* access()  */
#include    <sys/time.h>
#endif

#ifndef WINDOWS
#include <sys/stat.h>
#include <dirent.h>
#endif

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <ctype.h>       /* isupper() */

#include    "sccex.h"       /* Required */

/*
   Oracle does version tracking of the Outside In sample applications.
   OIT_VERSTRING is defined by internal build scripts.
*/
#ifdef OIT_VERSTRING
char oitsamplever[32] = OIT_VERSTRING;
#endif

#ifdef UNIX
#define PATH_TYPE   IOTYPE_UNIXPATH
#else
#define PATH_TYPE   IOTYPE_ANSIPATH
#endif

#if 0
#define PRINT_OPTIONS       /* When defined, PrintOpts() prints the values of all the configuration options. */
#endif

/* buffer for error strings */
char _szError[256];
VTDWORD gdwFileCount;               /* Global count of files created. */

#if !defined (PATH_MAX)
#if defined (_MAX_PATH)
#define PATH_MAX _MAX_PATH
#elif defined (MAX_PATH)
#define PATH_MAX MAX_PATH
#else /* !_MAX_PATH */
#define PATH_MAX 1024
#endif /* _MAX_PATH */
#endif /* !PATH_MAX */

typedef struct
{
    int numFiles;
    VTLPSTR* strInputFilenameArray;
    VTLPSTR  strOutputFilename;
} ConversionSpec;

SCCERR  PopulateConversionSpec(int argc, char *argv[], ConversionSpec* pConversionSpec);
SCCERR  FreeConversionSpec(ConversionSpec* pConversionSpec);

/**  \brief Show usage notes.  */
VTVOID showUsage();
/**  \brief Set options related to PX.  */
DAERR setOptions(VTHDOC const hDoc);
/* Function prototypes */
SCCERR DoExport(ConversionSpec* pConvSpec);
SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData, VTDWORD dwCommandID, VTLPVOID pCommandData);

#if defined(PXMULTI)
#define OUTPUT_ID FI_PDFA
#define APPNAME "pxmulti"
#elif defined(IXMULTI)
#define OUTPUT_ID FI_TIFF
#define APPNAME "ixmulti"
#endif

#ifndef TRUE
#define TRUE    (1 == 1)
#endif

#ifndef FALSE
#define FALSE   (1 == 0)
#endif

#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

int main(int argc, char *argv[])
{
    SCCERR         seResult;
    ConversionSpec ConvSpec; 

    /* Uses a customized version of the ReadCommandLine method */
    seResult = PopulateConversionSpec(argc, argv, &ConvSpec);
    if (seResult != SCCERR_OK)
    {
        showUsage();
        return (0);
    }
    seResult = DoExport(&ConvSpec);

    //Cleanup any memory used for the command line paramaters.
    FreeConversionSpec(&ConvSpec);

    return (seResult);
}

SCCERR DoExport (ConversionSpec* pConvSpec)
{
    SCCERR      seResult;
    VTHDOC      hDoc = 0;
    VTHEXPORT   hExport;
    VTCHAR* szInputFilename;
    VTCHAR* szOutputFilename;
    VTSHORT     i = 0;
    VTDWORD		optionValue;


    /* Initialize the Data Access module.  Required */
    seResult = DAInitEx(SCCOPT_INIT_NOTHREADS, OI_INIT_DEFAULT);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DAInitEx() failed: %s (%04X)\n", _szError, seResult);

        return (seResult);
    }

    gdwFileCount = 0;

    /* Note that the input and output files stored in the Opts data structure
       are not used. */
    szInputFilename  = pConvSpec->strInputFilenameArray[0];
    szOutputFilename = pConvSpec->strOutputFilename;

    /*
        Open our input document.
        
        DAOpenDocument can attempt to identify the type of the input file
        so before calling it for the first time we set the file
        identification option to extended testing.   Most other options, 
		except SCCOPT_TEMPDIR, should be set after the document is opened.
		See setOptions in this sample.

        DASetOption Parameters:
            1)  A handle to the document for which the option should be set.
                We have no document handle yet, so we use 0 to indicate that
                this option should be set for all documents.
            2)  The id of the option to set.
            3)  A pointer to the value the option will be set to.
            4)  The size of the option's value.

        DASetOption returns a DAERR code.  DAERR_OK indicates success.
    */
    optionValue = SCCUT_FI_EXTENDEDTEST;
    seResult = DASetOption(0, SCCOPT_FIFLAGS, &optionValue, sizeof(optionValue));
    if (seResult != DAERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DASetOption() failed for SCCOPT_FIFLAGS\n");
        DADeInit();

        return (seResult);
    }

    /*
        DAOpenDocument creates a handle to a document suitable for use in
        various Outside In API calls.  Successful calls to DAOpenDocument
        should be matched by calls to DACloseDocument.

        DAOpenDocument Parameters:
            1)  The address of a document handle that will contain the
                created document handle when the call returns successfully.
            2)  Type indicating what information is being provided to open
                the document, in this case a file path. (The preprocessor
                symbol PATH_TYPE was defined as either IOTYPE_ANSIPATH or 
                IOTYPE_UNIXPATH depending on the platform.)
            3)  A pointer to information needed to open the document (a
                pointer to the path of the file in this case).
            4)  Flags.  The 0 for flags forces DA to identify the file type
                for us.
            
        DAOpenDocument returns a DAERR code.  DAERR_OK indicates success.
    */
    seResult = DAOpenDocument(&hDoc, PATH_TYPE, (VTLPVOID)szInputFilename, 0);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DAOpenDocument() failed for document %s: %s (%04X)\n", szInputFilename, _szError, seResult);
        DADeInit();

        return (seResult);
    }

    /* If the output file already exists, delete it. */
    if (access(szOutputFilename, 0) != -1)
    {
        unlink(szOutputFilename);
    }

	/*  Set the document specific options.  */
    seResult = setOptions(hDoc);
    if (seResult != DAERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "Failed to set options for %s: %s (%04X)\n", szInputFilename, _szError, seResult);
        DACloseDocument(hDoc);
        DADeInit();

        return (seResult);
    }

    /*
        EXOpenExport initiates an export.  Successful calls to EXOpenExport
        should be matched by calls to EXCloseExport.

        EXOpenExport Parameters:
            1)  The document handle created by DAOpenDocument
            2)  Type of Export to perform.  FI_XML_FLEXIONDOC5_2 in this case.
            3)  Type indicating what information is being provided to open
                the document, in this case a file path. (The preprocessor
                symbol PATH_TYPE was defined as either IOTYPE_ANSIPATH or 
                IOTYPE_UNIXPATH depending on the platform.)
            4)  Path to the output file.
			5)	dwFlags.  None defined, so set to 0.
			6)	dwReserved.  Must be set to 0.
			7)	pCallbackFunc.  Not implemented in this sample app.
			8)	dwCallbackData.  Not implemented in this sample app.
			9)	The address of an export handle that will contain the
                created export handle when the call returns successfully.
            
        EXOpenExport returns a SCCERR code.  DAERR_OK indicates success.
    */
    seResult = EXOpenExport(hDoc, OUTPUT_ID, PATH_TYPE, szOutputFilename, 0, 0,
                            (EXCALLBACKPROC)ExportOemCallback, 0, &hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "EXOpenExport() failed for %s: %s (%04X)\n", szInputFilename, _szError, seResult);
        DACloseDocument(hDoc);
        DADeInit();

        return (seResult);
    }

    for (i=0; i < pConvSpec->numFiles; i++)
    {
        if (i > 0)
        {
            szInputFilename = pConvSpec->strInputFilenameArray[i];
            seResult = DAOpenNextDocument(hExport, PATH_TYPE, (VTLPVOID)szInputFilename, 0);
            if (seResult != SCCERR_OK)
            {
                DAGetErrorString(seResult, _szError, sizeof(_szError));
                fprintf(stderr, "DAOpenNextDocument() failed for document %s: %s (%04X)\n", szInputFilename, _szError, seResult);
            }
        }

        /* Do the actual conversion.  Required */
        if (seResult == SCCERR_OK)
        {
            seResult = EXRunExport(hExport);

            if (seResult != SCCERR_OK)
            {
                DAGetErrorString(seResult, _szError, sizeof(_szError));
                fprintf(stderr, "EXRunExport() failed for %s: %s (%04X)\n", szInputFilename, _szError, seResult);

                //break out of our loop and close the export.
                break;
            }
            else
            {
                gdwFileCount++; /* EX_CALLBACK_ID_CREATENEWFILE not called for root file. */
                fprintf(stderr, "Export of %s successful.\n", szInputFilename);
            }
        }
    }

    fprintf(stderr, "Export of %d files succesful", gdwFileCount);
    if ((VTDWORD)pConvSpec->numFiles > gdwFileCount)
        fprintf(stderr, "; Export of %d files failed", pConvSpec->numFiles - gdwFileCount);
    fprintf(stderr, ".\n");

    /* Close the export on the document.  Required */
    seResult = EXCloseExport(hExport);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "EXCloseExport() failed: %s (%04X)\n", _szError, seResult);
        DACloseDocument(hDoc);
        DADeInit();

        return (seResult);
    }

    /* Close the document.  Required */
    seResult = DACloseDocument(hDoc);
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DACloseDocument() failed: %s (%04X)\n", _szError, seResult);
        DADeInit();

        return (seResult);
    }

    /* Shutdown the Data Access module.  Required */
    seResult = DADeInit();
    if (seResult != SCCERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DADeInit() failed: %s (%04X)\n", _szError, seResult);
    }

    return (seResult);
}

/**
    This function displays the help associated with this sample application.
*/
VTVOID showUsage()
{
    fprintf(stderr, "%s - Converts a directory of input files to one output file.\n", APPNAME);
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage:\t%s InDir OutFile\n", APPNAME);
    fprintf(stderr, "Where:\t\"InDir\" contains the files to be converted.\n");
    fprintf(stderr, "\t\"OutFile\" is the single output file in which to place the converted files.\n");
    fprintf(stderr, "\n");
}

/**
    This function sets document specific options of interest to PX.
    One option that must always be set for PX is the location of the font directory
	containing the fonts to be used by PX.  For this example, we will assume that
	the fonts are located in C:\WINDOWS\FONTS for Windows, and $HOME/fonts for Unix.

    A second option that is useful in PX is the option on whether or not to embed
	fonts in the output.  Here, we'll set the option to tell the technology to NOT
	embed the fonts.

	Note: this is not meant to be a thorough representations of the options
	available in PX.  Many customers will set additional options that are
	demonstrated in other sample apps.  A complete listing can be found
	in the documentation.

    \param  hDoc    A handle to the document for which options are being set.

    \return A DA related error code.  DAERR_OK for success.
*/
DAERR setOptions(VTHDOC const hDoc)
{
    SCCERR seResult;
    VTBYTE byteOptionValue[VT_MAX_FILEPATH];

/*
        DASetOption Parameters:
            1)  A handle to the document for which the option should be set.
            2)  The id of the option to set
            3)  A pointer to the value the option will be set to.
            4)  The size of the option's value.

        DASetOption returns a DAERR code.  DAERR_OK indicates success.
*/

	memset( byteOptionValue, 0, sizeof(byteOptionValue) );;
#ifdef UNIX
    {
	    char *szHome = getenv( "HOME" );
	    char szFontDirectory[VT_MAX_FILEPATH];

	    if (szHome)
	    {
		    strcpy( (char *)byteOptionValue, szHome );
		    strcat( (char *)byteOptionValue, "/fonts" );
	    }
	    else
	    {
		    /* 
		     | A valid font directory must be set; return an error if the directory was not found
		     | or could not be read
		     */
		    return DAERR_BADPARAM;
	    }
    }
#else
	strcpy( (char *)byteOptionValue, "c:\\WINDOWS\\FONTS" );
#endif
    seResult = DASetOption(hDoc, SCCOPT_FONTDIRECTORY, byteOptionValue, sizeof(byteOptionValue));
    if (seResult != DAERR_OK)
    {
        DAGetErrorString(seResult, _szError, sizeof(_szError));
        fprintf(stderr, "DASetOption() failed: %s (%04X)\n", _szError, seResult);
    }

#ifdef IXMULTI
    {
        EXTIFFOPTIONS sEXTiffOptions = {0};

        sEXTiffOptions.dwSize = sizeof(EXTIFFOPTIONS);
        sEXTiffOptions.dwColorSpace = SCCGRAPHIC_TIFF24BITRGB;
        sEXTiffOptions.dwCompression = SCCGRAPHIC_TIFFCOMPNONE;
        sEXTiffOptions.dwByteOrder = SCCGRAPHIC_TIFFBOBIGE;
        sEXTiffOptions.dwTIFFFlags |= SCCGRAPHIC_TIFFFLAGS_ONEFILE;
        sEXTiffOptions.dwFillOrder = SCCGRAPHIC_TIFF_FILLORDER1;

        if ((seResult = DASetOption(hDoc, SCCOPT_IMAGEX_TIFFOPTIONS, &sEXTiffOptions, sEXTiffOptions.dwSize)) != SCCERR_OK)
            return seResult;
    }
#endif

    return seResult;
}

SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData,
                         VTDWORD dwCommandID, VTLPVOID pCommandData)
{
    SCCERR seResult;

    UNUSED(hExport);
    UNUSED(dwCallbackData);
    UNUSED(pCommandData);

    /*
    |   This is a dummy callback routine just to show how they are
    |   coded.  All it does
    |   is keep a count of how many output files we created.
    |
    |   Note that XML Export does not use callbacks so this is not
    |   required or used.
    */
    seResult = SCCERR_NOTHANDLED; /* assume we don't handle the callback! */
    switch (dwCommandID)
    {
        case EX_CALLBACK_ID_CREATENEWFILE:
            gdwFileCount++;
            break;

        case EX_CALLBACK_ID_NEWFILEINFO:
        default:
            break;
    }
    return seResult;
}

#ifdef WINDOWS
void CopyDBtoSB(VTLPSTR szDest, VTLPWSTR wszSource, VTDWORD dwMaxSize)
{
    VTDWORD i;

    for(i=0; wszSource[i] && (i<dwMaxSize-1); i++)
        szDest[i] = (char)wszSource[i];

    szDest[i] = '\0';
}

VTLPWSTR GenerateAbsolutePath(const char* szName)
{
    LPWSTR wszName = (LPWSTR)malloc((strlen(szName)+5) * sizeof(WORD));
    DWORD i;
    DWORD dwLen;

    if(!wszName)
        return wszName;

    /* convert the path to double byte prepend "//?/" */
    wszName[0] = '\\';
    wszName[1] = '\\';
    wszName[2] = '?';
    wszName[3] = '\\';
    for(i=0; szName[i]; i++)
        wszName[i+4] = (WORD)szName[i];
    wszName[i+4] = '\0';

    /* attempt to get the full path name for the argument in case it's relative */
    dwLen = GetFullPathNameW(&wszName[4], 0, NULL, NULL);
    if(dwLen)
    {
        LPWSTR wszFullName = (LPWSTR)malloc((dwLen+5) * sizeof(WORD));
        if(!wszFullName)
        {
            free(wszName);
            return wszFullName;
        }
        wszFullName[0] = '\\';
        wszFullName[1] = '\\';
        wszFullName[2] = '?';
        wszFullName[3] = '\\';
        dwLen = GetFullPathNameW(&wszName[4], dwLen+1, &wszFullName[4], NULL);
        if(dwLen)
        {
            free(wszName);
            return wszFullName;
        }
        else
            free(wszFullName);
    }

    return wszName;
}

void DestroyAbsolutePath(VTLPWSTR* pwszPath)
{
    if(pwszPath && *pwszPath)
    {
        free(*pwszPath);
        *pwszPath = NULL;
    }
}
#endif

/*
 * Examines the command line parameters and returns an array of ConversionSpec
 * structures.
 * When the caller is finished using the array of ConversionSpecs, 
 * the corresponding ReadCommandLine_mult_deinit method must be called.
 */
SCCERR PopulateConversionSpec(int argc, char *argv[], ConversionSpec* pConvSpec)
{
    SCCERR seResult = SCCERR_OK;

    memset(pConvSpec, 0, sizeof (ConversionSpec));
    
    if ((argc < 3))
    {
        seResult = SCCERR_BADPARAM;
    }
    else
    {
        int i = 0;

        pConvSpec->strOutputFilename = (char*) malloc(strlen(argv[2])+1);
        strcpy(pConvSpec->strOutputFilename, argv[2]);

        /* enumerate all the files in InDir. */
        /* for each file, init a conv spec appropriately */
#ifdef WINDOWS
        {
            WIN32_FIND_DATAW findData;
            HANDLE fh = INVALID_HANDLE_VALUE;

            VTLPWSTR wszAbsPath;
            VTLPWSTR pstrPattern;
            VTDWORD dwLen;

            wszAbsPath = GenerateAbsolutePath(argv[1]);
            dwLen = (VTDWORD)wcslen(wszAbsPath);
            pstrPattern = (VTWORD*)malloc(sizeof(VTWORD) * (dwLen+5));
            wcscpy(pstrPattern, wszAbsPath);
            DestroyAbsolutePath(&wszAbsPath);

            pstrPattern[dwLen] = (VTWORD)'\\';
            pstrPattern[dwLen+1] = (VTWORD)'*';
            pstrPattern[dwLen+2] = (VTWORD)'.';
            pstrPattern[dwLen+3] = (VTWORD)'*';
            pstrPattern[dwLen+4] = (VTWORD)'\0';

            fh = FindFirstFileW(pstrPattern, &findData);
            if (fh == INVALID_HANDLE_VALUE)
            {
                fprintf(stderr, "Invalid path to input files: %s.\n", argv[1]);
                return SCCERR_INVALIDPATH;
            }

            do
            {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                    pConvSpec->numFiles++;

            } while (FindNextFileW(fh,&findData));

            FindClose(fh);
            pConvSpec->strInputFilenameArray = malloc(sizeof(VTLPSTR*) * pConvSpec->numFiles);

            fh = FindFirstFileW(pstrPattern, &findData);
            free(pstrPattern);
            do
            {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    dwLen = (VTDWORD)strlen(argv[1]);
                    pConvSpec->strInputFilenameArray[i] = (char*) malloc(dwLen+1+wcslen(findData.cFileName)+1);
                    strcpy(pConvSpec->strInputFilenameArray[i], argv[1]);
                    pConvSpec->strInputFilenameArray[i][dwLen] = '\\';
                    CopyDBtoSB(&pConvSpec->strInputFilenameArray[i][dwLen+1], findData.cFileName,(VTDWORD)wcslen(findData.cFileName)+1);
                    i++;
                }

            } while (FindNextFileW(fh,&findData));

            FindClose(fh);
        }
#else
        {
            /* For all other platforms, use opendir, readdir to
             enumerate the files in the directory.
            */
            char curpathBuf[PATH_MAX];
            char *qualifiedIndir;
            char *pstrInDir = argv[1];
            DIR *indir = NULL;
            struct stat status;
            struct dirent * direntry;

            if (pstrInDir[0] =='/')
            {
              qualifiedIndir = pstrInDir;
            }
            else
            {
              memset(curpathBuf, 0, PATH_MAX);
              getcwd(curpathBuf, PATH_MAX);
              strcat(curpathBuf, "/");
              strcat(curpathBuf, pstrInDir);
              qualifiedIndir = curpathBuf;
            } 

            indir = opendir(qualifiedIndir);
            if (indir == NULL)
            {
              return (SCCERR_INVALIDPATH);
            }
            for (direntry = readdir(indir); direntry != NULL; direntry = readdir(indir))
            {
                char qualifiedFilename[PATH_MAX];
                int staterr = 0;
                qualifiedFilename[0]=0;
                strcat(qualifiedFilename, qualifiedIndir);
                strcat(qualifiedFilename, "/");
                strcat(qualifiedFilename, direntry->d_name);     
                  
                staterr = stat(qualifiedFilename, &status);
                if (staterr == 0)
                {
                  if (!S_ISDIR(status.st_mode))
                  {
                    pConvSpec->numFiles++;
                  }
                }
            }
            pConvSpec->strInputFilenameArray = malloc(sizeof(VTLPSTR*) * pConvSpec->numFiles);
            closedir(indir);

            indir = opendir(qualifiedIndir);
            for (direntry = readdir(indir); direntry != NULL; direntry = readdir(indir))
            {
                char qualifiedFilename[PATH_MAX];
                int staterr = 0;
                qualifiedFilename[0]=0;
                strcat(qualifiedFilename, qualifiedIndir);
                strcat(qualifiedFilename, "/");
                strcat(qualifiedFilename, direntry->d_name);     
                  
                staterr = stat(qualifiedFilename, &status);
                if (staterr == 0)
                {
                  if (!S_ISDIR(status.st_mode))
                  {
                      pConvSpec->strInputFilenameArray[i] = (char*) malloc(PATH_MAX);
                      strcpy(pConvSpec->strInputFilenameArray[i], qualifiedFilename);
                      i++;
                  }
                }
            }
            closedir(indir);
        }
#endif
    }

    return seResult;
}


/*
 *  Cleans up any memory that was allocated by ReadCommandLine_mult_init.
 */ 
SCCERR  FreeConversionSpec(ConversionSpec* pConversionSpec)
{
    int i;

    for (i=0; i < pConversionSpec->numFiles; i++)
    {
        free(pConversionSpec->strInputFilenameArray[i]);
    }
    free(pConversionSpec->strInputFilenameArray);
    free(pConversionSpec->strOutputFilename);

    return SCCERR_OK;
}
