/*
|   Extract Archive 
|   Sample Application
|
|   Copyright (c) 2009, 2016, Oracle and/or its affiliates. All rights reserved.
|   All rights reserved.
|
|   You have a royalty-free right to use, modify, reproduce and
|   distribute the Sample Applications (and/or any modified version)
|   in any way you find useful, provided that you agree that Oracle
|   has no warranty obligations or liability for any Sample Application
|   files which are modified.
*/

/*
    This sample app is usable with the 8.3.0 or later versions of the 
    Outside In Technologies.
*/

/**
    \file   Extract_archive.c

    \brief  A sample application for demonstrating extraction of tree nodes from 
            the archive file
*/

#ifdef      WIN32                         /* Defined by Win32 and win64 compiler */
#define     WINDOWS                       /**<  WINDOWS must be defined for Windows platforms */
#include    <windows.h>
#define     PATH_TYPE    IOTYPE_ANSIPATH  /**<   ANSIPATH as the WINDOW file path type */
#define     PATH_SEP     '\\'             /**<   Window file path seperator */
#else
#ifndef UNIX
#define     UNIX                          /**<  UNIX must be defined for Unix platforms  */
#endif
#define     PATH_TYPE    IOTYPE_UNIXPATH  /**<   UNIXPATH  as the UNIX file path type */
#define     PATH_SEP     '/'              /**<  Unix file path seperator */
#endif

/*
    ---------------------------------------------------------------------------
    Includes
    ---------------------------------------------------------------------------
*/

/*
    scctype.h provides general Outside In data types and is typically
    included in all code using Outside In technologies.  sccda.h includes
    datatypes, declarations, and constants specific to DA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sccda.h>
#include <scctype.h>
#include <sccvw.h>
/*
    ---------------------------------------------------------------------------
    Constants and declarations.
    ---------------------------------------------------------------------------
*/
/**
    \brief  The size of a buffer holding error description output.

    The descriptive text of the error will be truncated if needed.
*/
#define ERROR_BUFFER_SIZE 0x100
#define CASTWORD(ptr) (*((VTWORD *)ptr))

/**
    \brief  Some platforms fail to define TRUE and FALSE.
            Make sure that these are defined.
*/
#ifndef TRUE
#define TRUE  1    /**< define TRUE for those platforms where it is missing */
#define FALSE 0    /**< define FALSE for those platforms where it is missing */
#endif

/**
    \brief The is the entry point for the application.  This program accepts an
    input file to be processed and the output directory for the extracted 
    archive tree nodes

    \param  argc    Number of arguments including the name of the
                    application.
    \param  argv    Array of pointers to characters representing command
                    line arguments.
    \return The exit status of the application.  0 for success.
*/
int main(int argc, char *argv[])
{

    DAERR         daErr;                                             /* Error code                                                            */
    VTHDOC        hDoc;                                              /* Input document handle                                                 */
    VTCHAR        *szInFile = 0;                                     /* Input file to convert                                                 */
    VTCHAR        *szOutPath = 0;                                    /* Output path                                                           */
    VTCHAR        *lpPath;                                           /* Point to the output path                                              */
    VTDWORD       dwNumRecords;                                      /* Number of stored archive records.                                     */
    char          _szError[ERROR_BUFFER_SIZE];                       /* Error message                                                         */
    VTDWORD       dwRecord;                                          /* The record in the archive file to be extracted                        */
    SCCDATREENODE sTreeNode;                                         /* Tree node structure filled with information about the selected record.*/
                                                                     /* SCCDATREENODE is defined in sccda.h                                   */
    VTWORD        wIndex = 0;                                        /* index for the for loop                                                */
    VTWORD        wLength = 0;                                       /* archive tree node name length                                         */
    VTCHAR        *sPath = 0;                                        /* Saved tree record location                                            */
    VTWORD        wPathLoc = 0;

    if (argc != 3)
    {
        fprintf(stderr, "%s - Extract tree nodes from the archive file.\n", argv[0]);
        fprintf(stderr, "Usage:\t%s InputFile OutputDir \n", argv[0]);
        fprintf(stderr, "Where:\t\"InputFile\" is the file to be extracted.\n");
        fprintf(stderr, "\t\"OutputDir\" is where to place the extracted archive tree nodes.\n");
        exit(0);
    }
    else
    {
        szInFile = (VTCHAR*)malloc(strlen(argv[1])+1);
        szOutPath = (VTCHAR*)malloc(strlen(argv[2])+2);
        if(!szInFile || !szOutPath)
        {
            fprintf(stderr, "Path/filename allocation failed\n");
            exit(0);
        }
        /* Save the input and output file names. */
        strcpy(szInFile,  argv[1]);
        strcpy(szOutPath, argv[2]);

        /* make sure that the output path ends in a PATH_SEP or a : */
        lpPath = szOutPath;
        while (*lpPath != 0x00)
            lpPath++;
        lpPath--;
        if(*lpPath != PATH_SEP && *lpPath != ':')
        {
            *++lpPath = PATH_SEP;
            *++lpPath = 0x00;
        }
    }

    /*
        DAInitEx()
        Initialize the Data Access module.  DAInitEx should be called once
        before any conversions are attempted.  DAInitEx does not need to be
        called for each individual conversion, but instead should be called
        once for a set of conversions.

        Successful calls to DAInitEx should be matched by calls to DADeInit.

        DAInitEx returns a DAERR code.  DAERR_OK indicates success.
    */    
    daErr = DAInitEx(SCCOPT_INIT_NOTHREADS, OI_INIT_DEFAULT);
    if (daErr != DAERR_OK)
    {
        /*
        While there is no way to know how big an error message is going to be,
        DAGetErrorString will truncate if needed.

        DAGetErrorString Parameters:
            1)  A DA related error code.
            2)  A pointer to a buffer to receive the error related message.
            3)  The size of the buffer in parameter 2.
        */
        DAGetErrorString(daErr, _szError, sizeof(_szError));
        fprintf(stderr, "DAInitEx() failed: %s (0x%04X)\n", _szError, daErr);
        exit(daErr);
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
    daErr = DAOpenDocument(&hDoc, PATH_TYPE, (VTLPVOID)szInFile, 0);
    if (daErr != DAERR_OK)
    {
        DAGetErrorString(daErr, _szError, sizeof(_szError));
        fprintf(stderr, "DAOpenDocument() failed: %s (0x%04X)\n", _szError, daErr);
        /*
        Deinitialize DA.

        DADeInit returns a DAERR code.  DAERR_OK indicates success.
        */  
        DADeInit();
        exit(daErr);
    }
	//Uncomment for unicode file names
	/*
 	VTDWORD dwSystemFlags = SCCVW_SYSTEM_UNICODE;
	daErr = DASetOption(hDoc, SCCOPT_SYSTEMFLAGS, (VTLPVOID)&dwSystemFlags, sizeof(dwSystemFlags));
	*/
    /* extracting archive tree nodes */
    /*
        DAGetTreeCount is called to retrieve the number of records in an archive file.
        DAGetTreeCount Parameters:
            1)  hDoc: returned by DAOpenDocument
            2)  lpRecordCount: A pointer to a VTLPDWORD that will be filled with the number
                of stored archive records.         
        DAGetTreeCount returns a DAERR code.  DAERR_OK indicates success.
                DAERR_BADPARAM indicates selected file does not contain an archive section
    */    
    daErr = DAGetTreeCount (hDoc, &dwNumRecords);
    if ( daErr != DAERR_OK)
    {
        if ( daErr == DAERR_BADPARAM )
        {
            fprintf(stderr, "The selected file, %s, isn't an archive\n", szInFile);
        }
        else
        {
            DAGetErrorString(daErr, _szError, sizeof(_szError));
            fprintf(stderr, "DAGetTreeCount() failed: %s (0x%04X)\n", _szError, daErr);
        }
        /*
        Close the document.

        DACloseDocument Parameters:
            1)  The handle of the document to be closed.

        DACloseDocument returns a DAERR code.  DAERR_OK indicates success.
        */    
        DACloseDocument(hDoc);
        DADeInit();
        exit(daErr);
    }

    for(dwRecord = 0 ; dwRecord < dwNumRecords; dwRecord++)
    {
        sTreeNode.dwSize = sizeof(sTreeNode);
        sTreeNode.dwNode = dwRecord;
        /*
            DAGetTreeRecord is called to retrieve information about a record in an archive file.
            DAGetTreeRecord Parameters:
                1)  hDoc: returned by DAOpenDocument
                2)  pTreeNode: A pointer to an SCCDATREENODE structure that will be filled with
                    information about the selected record.        
            DAGetTreeRecord returns a DAERR code.  DAERR_OK indicates success.
            DAERR_BADPARAM indicates selected file does not contain an archive section, or the
            requested record does not exist
        */  
        daErr = DAGetTreeRecord(hDoc, &sTreeNode);
        /* if the sTreeNode.dwFlags is set to SCCDA_TREENODEFLAG_FOLDER, this means that the node 
        |  is a folder and can't be extracted 
        */
        if ((daErr != DAERR_OK) || (sTreeNode.dwFlags == SCCDA_TREENODEFLAG_FOLDER))
        {
            if (daErr != DAERR_OK)
            {
                DAGetErrorString(daErr, _szError, sizeof(_szError));
                fprintf(stderr, "DAGetTreeRecord() failed for record %d with error %s \n", dwRecord, _szError);
            }
            continue;
        }

        printf("Node number: %d \n", sTreeNode.dwNode);
        printf("Name: %s \n", sTreeNode.szName);
        printf("Charset: 0x%08x \n", sTreeNode.dwCharSet );
        printf("Size: %u \n", sTreeNode.dwFileSize);
        printf("Time: %d \n", sTreeNode.dwTime);
        if(sTreeNode.dwFlags == 0)
            printf("No flags are set \n");
        if(sTreeNode.dwFlags & SCCDA_TREENODEFLAG_SELECTED)
            printf("Selection flag is set \n");
        if(sTreeNode.dwFlags & SCCDA_TREENODEFLAG_FOCUS)
            printf("Focus flag is set \n");
		if(sTreeNode.dwFlags & SCCDA_TREENODEFLAG_ENCRYPT)
            printf("Encryption flag is set \n");
		if(sTreeNode.dwFlags & SCCDA_TREENODEFLAG_ARCKNOWNENCRYPT)
            printf("Known Encryption flag is set \n");
        printf(" \n\n");

        /* Copy the output path, szOutPath, into a local path variable, sPath.
        */
		VTDWORD dwOption = 0;
		VTDWORD dwLength = sizeof(VTDWORD);
		VTWCHAR wszOutPath[DA_PATHSIZE];
		if (DAGetOption(hDoc, SCCID_SYSTEMFLAGS, &dwOption, &dwLength) == SCCERR_OK)
		if (dwOption & SCCVW_SYSTEM_UNICODE)
		{
			wPathLoc = (VTWORD)strlen(szOutPath);
			mbstowcs(wszOutPath, szOutPath, wPathLoc);
			VTDWORD dwInc = 0;
			VTDWORD dwCnt = 0;
			for (dwCnt = 0; dwCnt < sTreeNode.dwFileNameLen; dwCnt++)
			{
				wszOutPath[wPathLoc + dwCnt] = CASTWORD(&(sTreeNode.szName[dwInc]));
				dwInc += 2;
			}
		}
		wPathLoc = (VTWORD)strlen(szOutPath);
		wLength = wPathLoc + (VTWORD)strlen((char *)sTreeNode.szName);
        sPath = (char*)malloc(wLength + 1);
        if(!sPath)
        {
            fprintf(stderr, "Path allocation failed. Skipping archive record %d.\n", dwRecord);
            continue;
        }
        strcpy((char *)sPath, szOutPath);

        /* Some times the file name stored in the archive contains one of the following characters < > : " / \ | ? * 
        |  These character are not allowed on one or more file systems, so we will replace them with a space character.
        */
        strcat(sPath, (char *)sTreeNode.szName);
        for (wIndex = wPathLoc; wIndex < wLength; wIndex++)
        {
            switch (sPath[wIndex])
            {
            case '<':
            case '>':
            case ':':
            case '"':
            case'\\':
            case '/':
            case '|':
            case '?':
            case '*':
			case '\t':
                sPath[wIndex] = ' ';
                break;
            default:
                break;
            }
        }

        /*
        DASaveTreeRecord is is called to extract a record in an archive file to disk..
        DASaveTreeRecord Parameters:
            1)  hDoc: returned by DAOpenDocument
            2)  dwRecord: The record in the archive file to be extracted.        
            3)  Type indicating what information is being provided to save
                tree record, in this case a file path. (The preprocessor
                symbol PATH_TYPE was defined as either IOTYPE_ANSIPATH or 
                IOTYPE_UNIXPATH depending on the platform.)
            4)  pSpec: File location specification.
            5)  dwFlags: Currently not used. Should be set to 0.
        DASaveTreeRecord returns a DAERR code.  DAERR_OK indicates success.
        */ 
		if (dwOption & SCCVW_SYSTEM_UNICODE)
			daErr = DASaveTreeRecord(hDoc, dwRecord, IOTYPE_UNICODEPATH, wszOutPath, 0);
		else
			daErr = DASaveTreeRecord(hDoc, dwRecord, PATH_TYPE, sPath, 0);
        if(daErr != DAERR_OK )
        {
            DAGetErrorString(daErr, _szError, sizeof(_szError));
            fprintf(stderr, "DASaveTreeRecord() failed for record %d with error %s \n", dwRecord, _szError);
        }

        free(sPath);
    }

    free(szOutPath);
    free(szInFile);
    /*
        Close the document.

        DACloseDocument Parameters:
            1)  The handle of the document to be closed.

        DACloseDocument returns a DAERR code.  DAERR_OK indicates success.
    */    
    daErr = DACloseDocument(hDoc);
    if (daErr != DAERR_OK)
    {
        DAGetErrorString(daErr, _szError, sizeof(_szError));
        fprintf(stderr, "DACloseDocument() failed: %s (0x%04X)\n", _szError, daErr);
        DADeInit();
        exit(daErr);
    }

   /*
        Deinitialize DA.

        DADeInit returns a DAERR code.  DAERR_OK indicates success.
    */  
    daErr = DADeInit();
    if (daErr != DAERR_OK)
    {
        DAGetErrorString(daErr, _szError, sizeof(_szError));
        fprintf(stderr, "DADeInit() failed: %s (0x%04X)\n", _szError, daErr);
        exit(daErr);
    }
    exit(0);
}
