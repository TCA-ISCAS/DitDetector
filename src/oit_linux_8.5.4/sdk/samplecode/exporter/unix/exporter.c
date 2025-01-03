/*
 |	C EXPORT SDKS
 | EXECUTABLE MODULE FOR JAVA INTERFACE.
 |
 | OUTSIDEIN SAMPLE APPLICATION
 | COPYRIGHT (C) 1991,2009, ORACLE. 
 | ALL RIGHTS RESERVED.
 |
 | ORACLE PROVIDES THIS CODE AS IS AND PROVIDES NO WARRANTIES
 | OR GUARANTEES OF ANY KIND.  YOU ARE FREE TO USE, MODIFY,
 | REPRODUCE, AND DISTRIBUTE THIS CODE AND/OR ANY MODIFIED VERSION
 | THERE OF SO LONG AS YOU ACKNOWLEDGE THAT ORACLE HAS NO WARRANTY
 | OBLIGATIONS OR LIABILITY FOR SUCH USE AND SO LONG AS SUCH USE IS
 | IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF AN EXISITING AND VALID
 | LICENSE AGREEMENT BETWEEN YOU AND ORACLE
 |
*/

#define EX_EXPORTER

#ifdef WIN32                /* Win32 is defined by Visual Studio */
#ifndef WINDOWS
#define WINDOWS
#include <windows.h>
#endif
#include <windows.h>
#else
#ifndef UNIX
#define UNIX                /* Required for UNIX                 */
#endif
#endif

#ifdef UNIX
/*
   Oracle does version tracking of the Outside In sample applications.
   OIT_VERSTRING is defined by internal build scripts so you can delete
   the following line at your leisure as it performs no real function.
*/
#ifdef OIT_VERSTRING
char oitsamplever[32] = OIT_VERSTRING;
#endif

#define PATH_TYPE IOTYPE_UNIXPATH
#include <unistd.h>
#define _access access
#else
#define PATH_TYPE IOTYPE_ANSIPATH
#include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sccex.h"           /* Required */
#include "cfg.c"        

VTBYTE  szFirstPrev[16] = "firstprevhref";
VTDWORD dwFirstPrev     = 13;                 /* Length of the "firstprevhref" string */

/* Function Prototypes */
SCCERR ParseCommandLine(int argc, char* argv[], Option* pOptions);
SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData, VTDWORD dwCommandID, VTLPVOID pCommandData);
SCCERR HandleCreateNewFile(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleNewFileInfo(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleProcessLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleCustomElementList(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleGraphicExportFailure(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleOEMOutput2(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleProcessElementStr2(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
SCCERR HandleAltLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData);
char*  GetErrorDescription(SCCERR sccerr);

/* callback handler array */
typedef SCCERR (*CALLBACKHANDLER)(VTSYSPARAM, VTLPVOID);
typedef struct
{
    VTDWORD dwID;
    CALLBACKHANDLER pfn;
} CALLBACKMSGSTRUCT;

const CALLBACKMSGSTRUCT gCBHandlers[] =
{
    {EX_CALLBACK_ID_CREATENEWFILE, HandleCreateNewFile},
    {EX_CALLBACK_ID_NEWFILEINFO, HandleNewFileInfo},
    {EX_CALLBACK_ID_PROCESSLINK, HandleProcessLink},
    {EX_CALLBACK_ID_CUSTOMELEMENTLIST, HandleCustomElementList},
    {EX_CALLBACK_ID_GRAPHICEXPORTFAILURE, HandleGraphicExportFailure},
    {EX_CALLBACK_ID_OEMOUTPUT_VER2, HandleOEMOutput2},
    {EX_CALLBACK_ID_PROCESSELEMENTSTR_VER2, HandleProcessElementStr2},
    {EX_CALLBACK_ID_ALTLINK, HandleAltLink}
};

#define NUMHANDLERS (sizeof(gCBHandlers) / sizeof(CALLBACKMSGSTRUCT))

Option      exOptions;

#ifdef WIN32
#include <windows.h>

int GetArgumentCount(LPSTR);
LPSTR GetArgumentList(LPSTR, int*);
#endif



#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCommandLine, int nCmdShow)
{
	LPSTR realCommandLine;
	char** argv;
	int argc = 0;
    SCCERR      rc = SCCERR_OK;
    static const char* pstr = "ba";
	int			i;
	int			pos = 0;
    VTDWORD     dwDaInitExFlags = OI_INIT_DEFAULT;
	VTLPVOID    pInSpec = NULL, pOutSpec = NULL;
    VTDWORD     dwInputSpecType = PATH_TYPE, dwOutputSpecType = PATH_TYPE;

    UNUSED(hInstance);
    UNUSED(hPrevInstance);
    UNUSED(lpCommandLine);
    UNUSED(nCmdShow);

	realCommandLine = GetCommandLine();
	argc = GetArgumentCount(realCommandLine);
	argv = malloc(sizeof(char *) * argc);

	for ( i = 0; i < argc; i++)
	{
		realCommandLine += pos;
		pos = 0;
		argv[i] = GetArgumentList(realCommandLine, &pos);
	}
#else
int main(int argc, char* argv[])
{
    SCCERR      rc = SCCERR_OK;
    static const char* pstr = "ba";
	VTDWORD     dwDaInitExFlags = OI_INIT_DEFAULT;
	VTLPVOID    pInSpec, pOutSpec;
	VTDWORD     dwInputSpecType, dwOutputSpecType;
#endif


    UnicodeToAsciiFunc = (*(VTLPWORD)pstr == 0x6261) ? ConvertBigUnicodeToASCII : ConvertLittleUnicodeToASCII;

    memset(&exOptions, 0, sizeof(Option));

    rc = ParseCommandLine(argc, argv, &exOptions);

    if (rc != SCCERR_OK)
        goto ErrorExit;

#ifdef WIN32
	free(argv);
#endif

    if((TRUE == exOptions.abSetOptions[SAMPLE_OPTION_LOAD_OPTIONS])&&(FALSE == exOptions.bLoadOptions))
    {
        dwDaInitExFlags |= OI_INIT_NOLOADOPTIONS;
    }

    if((TRUE == exOptions.abSetOptions[SAMPLE_OPTION_SAVE_OPTIONS])&&(FALSE == exOptions.bSaveOptions))
    {
        dwDaInitExFlags |= OI_INIT_NOSAVEOPTIONS;
    }

    /* Initialize the Data Access module. */
    rc = DAInitEx(SCCOPT_INIT_NOTHREADS, dwDaInitExFlags);

    if (rc != SCCERR_OK)
    {
#ifndef DISABLE_REPORTING
       {
           fprintf( stdout, "DAInitEx() returned error code %d (%s)\n", rc, GetErrorDescription(rc) );
       }
#endif
    }
    else
    {
        VTHDOC      hDoc;
        VTHEXPORT   hExport;

        /* Open the document to be converted */
        if (exOptions.abSetOptions[SAMPLE_OPTION_INPUTPATH] == FALSE)
        {
            VTDWORD dwSize = (Swcslen(exOptions.wzInFile)*6) + 1; /* Has to be big enough for UTF-8, which can turn each char into 6 bytes */
            ALLOC(VTCHAR, exOptions.szInFile, dwSize);

            dwSize = ConvertUnicodeToUTF8 (exOptions.szInFile, dwSize, exOptions.wzInFile,
                                         Swcslen(exOptions.wzInFile) * sizeof(VTWORD));
            *(exOptions.szInFile+dwSize) = 0x00;
        }
        
        if (!g_szInitFilePath)
        {
            ALLOC(VTBYTE, g_szInitFilePath, 3);
#ifdef WINDOWS
            strcpy((char *)g_szInitFilePath, ".\\");
#else
            strcpy((char *)g_szInitFilePath, "./");
#endif
        }

        if (!g_szInitFileName)
        {
            VTLPBYTE ptr = (VTLPBYTE)exOptions.szInFile;
				while(*ptr) ptr++;
				while((*ptr != '/') && (*ptr != '\\') && (ptr > (VTLPBYTE)exOptions.szInFile)) ptr--;
				if ((*ptr == '/') || (*ptr == '\\')) ptr++;
            ALLOC(VTBYTE, g_szInitFileName, strlen((char *)ptr)+1);
            strcpy((char *)g_szInitFileName, (char *)ptr);

				ptr = g_szInitFileName;
				while(*ptr) ptr++;
				while((*ptr != '.') && (ptr > g_szInitFileName)) ptr--;
				if (ptr > g_szInitFileName) *ptr = '\0';
        }

        /* We need to set a couple of options before calling DAOpenDocument. */
        if( exOptions.abSetOptions[ SAMPLE_OPTION_TEMPDIRECTORY ] )
        {
            SCCUTTEMPDIRSPEC   tdsSpec;

            tdsSpec.dwSize     = sizeof( tdsSpec );
            tdsSpec.dwSpecType = IOTYPE_DEFAULT;
            strcpy( tdsSpec.szTempDirName, exOptions.szTempDir );
            rc = DASetOption( (VTHDOC)NULL, SCCOPT_TEMPDIR, 
                              (VTLPVOID)&tdsSpec, sizeof( tdsSpec ) );
            if( rc != SCCERR_OK )
            {
#ifndef DISABLE_REPORTING
                {
                    fprintf( stdout, 
                             "DASetOption() returned error code %d (%s)\n",
                             rc, GetErrorDescription( rc ) );
                }
#endif
                DADeInit();
                goto ErrorExit;
            }
        }

        if( exOptions.abSetOptions[ SAMPLE_OPTION_TEMPDIRECTORY_U ] )
        {
            SCCUTTEMPDIRSPEC   tdsSpec;
            VTDWORD            length;

            tdsSpec.dwSize     = sizeof( tdsSpec );
            tdsSpec.dwSpecType = IOTYPE_ANSIPATH;
            length             = ConvertUnicodeToUTF8( tdsSpec.szTempDirName,
                                                     SCCUT_FILENAMEMAX,
                                                     exOptions.wzTempDir,
                                                     Swcslen( exOptions.wzTempDir ) * sizeof(VTWORD) );
            if( length > 0 )
            {
                tdsSpec.szTempDirName[ length ] = 0x00;
                rc = DASetOption( (VTHDOC)NULL, SCCOPT_TEMPDIR, 
                                  (VTLPVOID)&tdsSpec, sizeof( tdsSpec ) );
                if( rc != SCCERR_OK )
                {
#ifndef DISABLE_REPORTING
                    {
                        fprintf( stdout,
                                 "DASetOption() returned error code %d (%s)\n",
                                 rc, GetErrorDescription( rc ) );
					}
#endif   
                    DADeInit();
                    goto ErrorExit;
                }
            }
        }

        if( exOptions.abSetOptions[ SAMPLE_OPTION_FIFLAGS ] )
        {
            rc = DASetOption( (VTHDOC)NULL, SCCOPT_FIFLAGS,
                              (VTLPVOID)&exOptions.dwFIFlags,
                              sizeof( VTDWORD ) );
            if( rc != SCCERR_OK )
            {
#ifndef DISABLE_REPORTING
                {
                    fprintf( stdout,
                             "DASetOption() returned error code %d (%s)\n",
                             rc, GetErrorDescription( rc ) );
                }
#endif
                DADeInit();
                goto ErrorExit;
            }
        }

        SetBufferSize((VTHDOC)NULL, &exOptions);   /* Set the IO Buffer sizes (if required) before DAOpenDocument() */

#ifdef WIN32
		if (exOptions.abSetOptions[SAMPLE_OPTION_INPUTPATH] == TRUE)
		{
#endif
			pInSpec = exOptions.szInFile;
			dwInputSpecType = PATH_TYPE;
#ifdef WIN32
		}
		else if (exOptions.abSetOptions[SAMPLE_OPTION_INPUTPATH_U] == TRUE)
		{
			pInSpec =  exOptions.wzInFile;
			dwInputSpecType = IOTYPE_UNICODEPATH;
		}
#endif
		rc = DAOpenDocument(&hDoc, dwInputSpecType, pInSpec, 0);
        if (rc != SCCERR_OK)
        {
#ifndef DISABLE_REPORTING
            {
               fprintf(stdout, "DAOpenDocument() returned error code %d (%s)\n", rc, GetErrorDescription(rc));
            }  
#endif
            DADeInit();
            goto ErrorExit;
        }

        rc = SetOptions(hDoc, &exOptions);

        if (rc != SCCERR_OK)
        {
#ifndef DISABLE_REPORTING
            {
				   fprintf(stdout, "SetOptions() returned error code %d (%s)\n", rc, GetErrorDescription(rc));
            }
#endif
            DACloseDocument(hDoc);
            DADeInit();
            goto ErrorExit;
        }

        if (rc == SCCERR_OK)
        {
            VTBOOL bUnicodeCallbackStr = FALSE;

            /* Unicode callback string not enabled at this time. */
            rc = DASetOption(hDoc, SCCOPT_EX_UNICODECALLBACKSTR,
                                   (VTLPVOID)&bUnicodeCallbackStr, sizeof(VTBOOL));
        }

        if ( exOptions.abSetOptions[SAMPLE_OPTION_OUTPUTPATH] == FALSE )
        {
            VTDWORD dwSize = (Swcslen(exOptions.wzOutFile)*6) + 1; /* Has to be big enough for UTF-8, which can turn each char into 6 bytes */
			ALLOC(VTCHAR, exOptions.szOutFile, dwSize);

            dwSize = ConvertUnicodeToUTF8(exOptions.szOutFile, dwSize, exOptions.wzOutFile,
                                        Swcslen(exOptions.wzOutFile)* sizeof(VTWORD));
            *(exOptions.szOutFile + dwSize) = 0x00;
        }

#ifdef WIN32
		if (exOptions.abSetOptions[SAMPLE_OPTION_OUTPUTPATH] == TRUE)
		{
#endif
			pOutSpec = exOptions.szOutFile;
			dwOutputSpecType = PATH_TYPE;
#ifdef WIN32
		}
		else if (exOptions.abSetOptions[SAMPLE_OPTION_OUTPUTPATH_U] == TRUE)
		{
			pOutSpec = exOptions.wzOutFile;
			dwOutputSpecType = IOTYPE_UNICODEPATH;
		}
#endif
        /* Open a handle to export */
        rc = EXOpenExport(hDoc, exOptions.dwOutputID, dwOutputSpecType, 
                          pOutSpec, 0, 0,
                          (EXCALLBACKPROC)ExportOemCallback, 0, &hExport);
                            
        if (rc != SCCERR_OK)
        {
#ifndef DISABLE_REPORTING
            {
                fprintf(stdout, "EXOpenExport() returned error code %d (%s)\n", rc, GetErrorDescription(rc));
            }
#endif
            DACloseDocument(hDoc);
            DADeInit();
            goto ErrorExit;
        }

        /* Do the actual conversion */            
        rc = EXRunExport(hExport);

        if (rc == SCCERR_OK) 
        {
#ifndef DISABLE_REPORTING
            {
               fprintf(stdout, "Export complete.\n");
            }
#endif
        }
        else 
        {
#ifndef DISABLE_REPORTING
            {
               fprintf(stdout, "EXRunExport() returned error code %d (%s)\n", rc, GetErrorDescription(rc));
            }
#endif
            EXCloseExport(hExport);
            DACloseDocument(hDoc);
            DADeInit();
            goto ErrorExit;
        }

        /* Close the handle to export */
        rc = EXCloseExport(hExport);

        if (rc != SCCERR_OK)
        {
#ifndef DISABLE_REPORTING
            {
				    fprintf(stdout, "EXCloseExport() returned error code %d (%s)\n", rc, GetErrorDescription(rc));
            }
#endif           
            DACloseDocument(hDoc);
            DADeInit();
            goto ErrorExit;
        }

        /* Close the document to be converted */
        rc = DACloseDocument(hDoc);

        if (rc != SCCERR_OK)
        {
#ifndef DISABLE_REPORTING
				   fprintf(stdout, "DACloseDocument() returned error code %d (%s)\n", rc, GetErrorDescription(rc));
#endif
            DADeInit();
            goto ErrorExit;
        }

        /* Shutdown the Data Access module. */
        rc = DADeInit();

        if (rc != SCCERR_OK)
        {
#ifndef DISABLE_REPORTING
				   fprintf(stdout, "DADeInit() returned error code %d (%s)\n", rc, GetErrorDescription(rc));
#endif
        }

    }

ErrorExit:
    Cleanup(&exOptions);

    /*
      Linux/Unix systems can only handle exit codes less than 255, so now we will return 1 if there
      was an error, and we will write the actual error code to stderr.  Users of this app will need
      to read stderr if they are interested in the actual error code.  com.outsideinsdk.Export.java
      demonstrates how to do this.
     */
    fprintf(stderr, "%d", rc);

    return (rc != 0);
}

/*
    Parse the keyword=value pairs and place it in the appropriate field of the ExOptions struct.
*/
SCCERR ParseCommandLine(int argc, char* argv[], Option* pOptions)
{
    SCCERR  rc          = SCCERR_OK;
    VTBYTE* ptr         = NULL;
    VTBYTE  keyword[BUF_SIZE];
    VTBYTE  value[BUF_SIZE];
    VTDWORD dwOptCode;
    int     length;
    int     i, j;
    static const char* pstrTest = "ba";

    /* Determine what the byte ordering of the current platform is. */
    ConvFunc = (*(VTLPWORD)pstrTest == 0x6261) ? ConvertASCIIToBigUnicode : ConvertASCIIToLittleUnicode;

    memset(pOptions->abSetOptions, 0, sizeof(pOptions->abSetOptions));

    /* Parse the keyword=value pairs */
    for (i = 0; i < argc && rc == SCCERR_OK; i++)
    {
        j = 0;
		memset(value, 0, BUF_SIZE);
        ptr = (VTBYTE *)argv[i];

        while(*ptr != '=' && *ptr != '\0')
        {
            ++j;
            ++ptr;
        }

        /* Either no keyword or no = sign, both cases are invalid as command line argument */
        if (j <= 1 || *ptr == '\0')
        {
            /* The first item in the argv array may be "exporter.exe". */
            if (i == 0)
                continue;
            else 
            {
#ifndef DISABLE_REPORTING
                {
                    fprintf(stdout, "Error: Invalid command line argument\n");
                }
#endif
                return SCCERR_BADPARAM;
            }
        }

        strncpy((char *)keyword, argv[i], j);
        keyword[j] = '\0';

        ptr++;

        /*
            Skip the " character if it is the first character in the value part of the
            keyword=value pair in the current command line argument.
        */
        if (*ptr == '\"')
        {
            ++ptr;
        }

        strcpy ((char *)value, (char *)ptr);
        length = (int)strlen((char *)ptr);

        /*
            Remove the " character if it is the last character in the value part of the
            keyword=value pair in the current command line argument.
        */
        if (length > 1)
        {
            if (value[length-1] == '\"')                
                value[length-1] = '\0';
        }

        StringToDW(Options, (VTLPSTR)keyword, &dwOptCode);
        /*  
        |   Store the current option in the ExOptions struct, ignoring error conditions.
        */
        StoreOption((VTLPSTR)keyword, (VTLPSTR)value, dwOptCode, 0, pOptions);
    }

    /*
         If command line arguments doesn't specify either the input path or the 
        output path or the output id then export can't continue.
    */
    if (pOptions->abSetOptions[SAMPLE_OPTION_INPUTPATH] == FALSE && 
        pOptions->abSetOptions[SAMPLE_OPTION_INPUTPATH_U] == FALSE)
    {
#ifndef DISABLE_REPORTING
        {
            fprintf(stdout, "Error: no input file was specified\n");
        }
#endif
        rc = SCCERR_BADPARAM;
    }
	 
    if (pOptions->abSetOptions[SAMPLE_OPTION_OUTPUTPATH] == FALSE &&
        pOptions->abSetOptions[SAMPLE_OPTION_OUTPUTPATH_U] == FALSE)
    {
#ifndef DISABLE_REPORTING
        {
            fprintf(stdout, "Error: no output file was specified\n");
        }
#endif      
        rc = SCCERR_BADPARAM;
    }
	 
    if (pOptions->abSetOptions[SAMPLE_OPTION_OUTPUTID] == FALSE)
    {
#ifndef DISABLE_REPORTING
        {
            fprintf( stdout, "Error: No output id was specified\n" );
        }
#endif        
        rc = SCCERR_BADPARAM;
    }

    return rc;
}

/* Callback handler */
SCCERR ExportOemCallback(VTHEXPORT hExport, VTSYSPARAM dwCallbackData,
                         VTDWORD dwCommandID, VTLPVOID pCommandData)
{
    SCCERR  rc = SCCERR_NOTHANDLED;
    int     i;
    UNUSED(hExport);

    for ( i = 0; i < NUMHANDLERS; i++ )
    {
        if ( dwCommandID == gCBHandlers[i].dwID )
        {
            rc = gCBHandlers[i].pfn ( dwCallbackData, pCommandData );
            break;
        }
    }

    return rc;
}

SCCERR HandleCreateNewFile(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXFILEIOCALLBACKDATA*       pFileIOData = (EXFILEIOCALLBACKDATA*) pCommandOrInfoData;
    EXURLFILEIOCALLBACKDATA*    pURLData    = (EXURLFILEIOCALLBACKDATA*) pFileIOData->pExportData;
    VTBOOL                      bIsRoot     = (pFileIOData->dwAssociation == CU_ROOT)? TRUE : FALSE;
    VTBOOL                      done        = FALSE;
    VTBYTE                      szExtension[32]  = {0};
    VTBYTE                      *szPath;
    VTBYTE                      *pTemp;
    UNUSED(dwCallbackData);

    szPath = (VTBYTE *)malloc(strlen((char *)g_szInitFilePath)+strlen((char *)g_szInitFileName) + 1);
    if (!szPath)
        return SCCERR_ALLOCFAILED;

    strcpy((char *)szPath, (char *)g_szInitFilePath);
    strcat((char *)szPath, (char *)g_szInitFileName);

     if( !DWToString ( CustomFileExtensions, pFileIOData->dwOutputId, (VTLPSTR)szExtension ) )
     {
        DWToString(FileExtensions, pFileIOData->dwOutputId, (VTLPSTR)szExtension);
     }
     if (szExtension[0] == 0)
     {
         /* there is no file extenstion for the output file, we will use the original file extension if it has */
         pTemp = pURLData->szURLString + strlen((char*)(pURLData->szURLString));
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
     }

        if (bIsRoot)
        {
			VTLPBYTE pch = (VTLPBYTE)exOptions.szOutFile;
			while( *pch ) pch++;
			while( (pch > (VTLPBYTE)exOptions.szOutFile) && (*pch != '/') && (*pch != '\\') ) pch--;
			if( (*pch == '/') || (*pch == '\\') ) pch++;

			sprintf( (char *)pURLData->szURLString, "%s%s", exOptions.szURLPrefix, pch );

         strcpy( exOptions.szCreateFilePath, exOptions.szOutFile );


         /* sprintf ( exOptions.szCreateFilePath, "%s%s%s", g_szInitFilePath, g_szInitFileName, szExtension ); */
         /* sprintf ( pURLData->szURLString, "%s%s%s", exOptions.szURLPrefix, g_szInitFileName, szExtension ); */
        }

        else
        {
            while(!done)
            {
                sprintf(exOptions.szCreateFilePath, "%s%.4x%s", szPath, ++g_FileCount, szExtension);

                if (_access(exOptions.szCreateFilePath, 0) == -1)
                {
                    sprintf((char *)pURLData->szURLString, "%s%s%.4x%s", exOptions.szURLPrefix, g_szInitFileName, g_FileCount, szExtension);
                    done = TRUE;
                }
            }

            pFileIOData->dwSpecType = IOTYPE_ANSIPATH;
            pFileIOData->pSpec      = exOptions.szCreateFilePath;
            pURLData->dwSize        = sizeof(EXURLFILEIOCALLBACKDATA);
        }

    free(szPath);
    return SCCERR_OK;
}

SCCERR HandleNewFileInfo(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    UNUSED(dwCallbackData);
    UNUSED(pCommandOrInfoData);
    return SCCERR_NOTHANDLED;
}

SCCERR HandleProcessLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    UNUSED(dwCallbackData);
    UNUSED(pCommandOrInfoData);
    return SCCERR_NOTHANDLED;
}

SCCERR HandleCustomElementList(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    VTLPVOID* pElementData = (VTLPVOID*)pCommandOrInfoData;
    UNUSED(dwCallbackData);

	*pElementData = CustomElements;

    /* We return SCCERR_OK all the time here because the "firstprevhref" is always set
       as a custom element entry. */		
    return SCCERR_OK;

}

SCCERR HandleGraphicExportFailure(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    SCCERR rc = SCCERR_NOTHANDLED;
    UNUSED(dwCallbackData);

    /*  
        Read and write out the error graphic image, if it has been specified 
        using the "substitutegraphic" option.
    */
    if (exOptions.abSetOptions[SAMPLE_OPTION_SUBSTITUTEGRAPHIC] == TRUE ||
        exOptions.abSetOptions[SAMPLE_OPTION_SUBSTITUTEGRAPHIC_U] == TRUE)
    {
        EXGRAPHICEXPORTINFO *pGraphicExportInfo = (EXGRAPHICEXPORTINFO *) pCommandOrInfoData;
        HIOFILE             hOutFile;
        VTDWORD             length=0;
        
        /* Convert unicode path to ascii */
        if (exOptions.abSetOptions[SAMPLE_OPTION_SUBSTITUTEGRAPHIC_U] == TRUE)
        {
            length = ConvertLittleUnicodeToASCII(exOptions.szSubstituteGraphic, VT_MAX_URL,
                                                 exOptions.wzSubstituteGraphic, Swcslen(exOptions.wzSubstituteGraphic));                
        }

        if (length > 0)
        {
            FILE*   pInFile;

            pInFile  = fopen (exOptions.szSubstituteGraphic, "w");
            hOutFile = pGraphicExportInfo->hFile;

            if (pInFile != NULL)
            {
                VTBYTE    buffer[BUF_SIZE];
                VTDWORD   dwCount;
                VTDWORD   dwBytesWritten = 0;
                VTDWORD   dwBytesCopied;
                VTDWORD   dwFileSize;

                fseek(pInFile, 0, SEEK_END);

                if ((dwFileSize = ftell(pInFile)) == (VTDWORD)-1)
                    return SCCERR_NOTHANDLED;

                if (*pGraphicExportInfo->pImageSize > 0)
                {
                    if (dwFileSize > *pGraphicExportInfo->pImageSize)
                    {
                        return SCCERR_NOTHANDLED;
                    }
                }

                IOSeek(hOutFile, IOSEEK_TOP, 0);
                rewind(pInFile);

                while(!feof(pInFile))
                {
                    dwCount = (VTDWORD)fread(buffer, sizeof(VTBYTE), BUF_SIZE, pInFile);

                    if (dwCount == 0)
                        break;

                    rc = IOWrite(hOutFile, buffer, dwCount, &dwBytesCopied);

                    dwBytesWritten += dwBytesCopied;

                    if (rc != SCCERR_OK)
                        break;
                }

                if (rc == SCCERR_OK && (dwBytesWritten == dwFileSize))
                {
                    *pGraphicExportInfo->pXSize = 0;
                    *pGraphicExportInfo->pYSize = 0;
                    *pGraphicExportInfo->pImageSize = dwFileSize;
                }

                else
                    rc = SCCERR_NOTHANDLED;
                fclose (pInFile);
            }
        }                
    }

    return rc;
}

SCCERR HandleOEMOutput2(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    SCCERR rc = SCCERR_NOTHANDLED;
    UNUSED(dwCallbackData);

    if (exOptions.abSetOptions[SAMPLE_OPTION_FIRSTPREVHREF] == TRUE)
    {
        EXOEMOUTCALLBACKDATA_VER2* pOEMData = (EXOEMOUTCALLBACKDATA_VER2*)pCommandOrInfoData;
            
        if (strlen (pOEMData->pOEMString) == dwFirstPrev)
        {
            VTLPBYTE pStr = (VTLPBYTE)pOEMData->pOEMString;
            VTDWORD i;

            for (i = 0; i < dwFirstPrev; i++)
            {
                if (pStr[i] <= 0x7F)
                {
                    if (tolower (pStr[i]) != szFirstPrev[i])
                        i = dwFirstPrev + 1;
                }

                else
                    if (pStr[i] != szFirstPrev[i])
                        i = dwFirstPrev + 1;
            }

            if (i == dwFirstPrev)
            {
                pOEMData->dwSize    = sizeof(EXOEMOUTCALLBACKDATA_VER2);
                pOEMData->dwLength  = Swcslen(exOptions.wzFirstPrevURL);
                pOEMData->dwCharset = SO_UNICODE;
                pOEMData->pwBuffer  = exOptions.wzFirstPrevURL;
                rc                  = SCCERR_OK;            
            }
        }
    }

    return rc;
}

SCCERR HandleProcessElementStr2(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    SCCERR rc = SCCERR_NOTHANDLED;
    UNUSED(dwCallbackData);

    if (exOptions.abSetOptions[SAMPLE_OPTION_FIRSTPREVHREF] == TRUE)
    {
        EXCUSTOMELEMENTCALLBACKDATA_VER2* pCustomData = (EXCUSTOMELEMENTCALLBACKDATA_VER2*)pCommandOrInfoData;

        if (strlen (pCustomData->pKeyStr) == dwFirstPrev)
        {
            VTLPBYTE pStr = (VTLPBYTE)pCustomData->pKeyStr;
            VTDWORD i;

            for (i = 0; i < dwFirstPrev; i++)
            {
                if (pStr[i] <= 0x7F)
                {
                    if (tolower (pStr[i]) != szFirstPrev[i])
                        i = dwFirstPrev + 1;
                }

                else
                    if (pStr[i] != szFirstPrev[i])
                        i = dwFirstPrev + 1;
            }

            if (i == dwFirstPrev)
            {
                pCustomData->dwSize     = sizeof(EXCUSTOMELEMENTCALLBACKDATA_VER2);
                pCustomData->dwCharset  = SO_UNICODE;
                pCustomData->dwLength   = Swcslen(exOptions.wzFirstPrevURL);
                pCustomData->pwBuffer   = exOptions.wzFirstPrevURL;
                rc                      = SCCERR_OK;
            }
        }
    }

    return rc;
}

SCCERR HandleAltLink(VTSYSPARAM dwCallbackData, VTLPVOID pCommandOrInfoData)
{
    EXALTLINKCALLBACKDATA* pALData = (EXALTLINKCALLBACKDATA*)pCommandOrInfoData;
    UNUSED(dwCallbackData);

    if (!exOptions.abSetOptions[SAMPLE_OPTION_ALTLINK_PREV + pALData->dwType])
        return SCCERR_NOTHANDLED;

    strncpy(pALData->pAltURLStr, exOptions.aszAltLinkURLs[pALData->dwType], BUF_SIZE);

    return SCCERR_OK;
}


char g_errorbuf[2048];

char* GetErrorDescription(SCCERR sccerr)
{
    DAGetErrorString(sccerr, g_errorbuf, 2047);
    return g_errorbuf;
}

#ifdef WIN32

int GetArgumentCount(LPSTR lpCommandLine)
{
	int count = 0;
	char* p = lpCommandLine;
	int quota = 0;

	while (*p != '\0') 
	{
		p++;
		while (*p != '"' && *p != '\0') {
			p++;
		}
		if (quota == 0)
			quota = 1;
		else
		{
		count++;
			quota = 0;
		}
	}

	return (count);
}



LPSTR GetArgumentList(LPSTR lpCommandLine, int* pos)
{
	char* p = lpCommandLine;
	int quota = 0;
	char *argv;
	int startquotapos = 0;
	argv = p;
	do
	{
		while (*p != '"' && *p != '\0')
		{
			p++;
			(*pos)++;
		}
		if (quota == 0)
		{
			startquotapos = *pos;
			quota = 1;
			p++;
			(*pos)++;
		}
		else
		{
		*p++ = '\0';
			(*pos)++;
			return (argv+startquotapos+1);
		}
	}while (*p != '\0');
	return argv;
}

#endif
