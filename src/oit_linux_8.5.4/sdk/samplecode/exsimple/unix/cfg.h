/*
 |  C EXPORT SDKS
 |
 | ORACLE SAMPLE APPLICATION
 | Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved. 
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

/*
 | The code in this file is strictly sample code and is not 
 | needed in order to use the Outsidein SDK products.
 */

#ifndef _CFG_H__
#define _CFG_H__

#include <string.h>
#ifndef TRUE
#define TRUE (1 == 1)
#endif

#ifndef FALSE
#define FALSE (1 == 0)
#endif

#define DEFAULT_CONFIG_FILE "default.cfg"

#define BUF_SIZE 4096 /* Size of various buffers used in the sample app */

#ifdef UNIX
#define IOTYPE_DEFAULT IOTYPE_UNIXPATH
#elif NLM
#define IOTYPE_DEFAULT IOTYPE_DOSPATH
#else
#define IOTYPE_DEFAULT IOTYPE_ANSIPATH
#endif


/* Export option configuration typedefs, #defines and variables */
#define MAP_STRINGSIZE 128
#define MAP_MAXELEMENTS 10
#define MAX_PASSWORD 10
#define MAX_NOTESID  10

#define MAX_FONT_PATH_LENGTH 8192     /* largest "safe" path length for fonts */

/* Structs needed for the 2 versions of the OEMOUTPUT callback */
typedef struct OEMELEMtag
{
    VTCHAR szKey[MAP_STRINGSIZE];
    VTCHAR szVal[MAP_STRINGSIZE];
    VTWORD wzVal[MAP_STRINGSIZE];
} OEMELEM;

typedef struct OEMSTRMAPtag
{
    OEMELEM aElements[MAP_MAXELEMENTS];
    VTWORD  wCount;
} OEMSTRMAP;

/* Structs needed for the 2 versions of the PROCESSELEMENTSTR callback */
typedef struct ELEMENTVALUEtag
{
    VTCHAR szElementValue[MAP_STRINGSIZE]; /* Name of element value          */
    VTCHAR szVal[MAP_STRINGSIZE];          /* Corresponding insertion string */
    VTWORD wzVal[MAP_STRINGSIZE];
} ELEMENTVALUE;

typedef struct KEYVALUEtag
{
    VTCHAR       szKey[MAP_STRINGSIZE];
    ELEMENTVALUE aElements[MAP_MAXELEMENTS];
    VTCHAR       szVal[MAP_STRINGSIZE];      /* In case of no element string */
    VTWORD       wzVal[MAP_STRINGSIZE];      /* In case of no element string */
    VTWORD       wCount;
} KEYVALUE;

typedef struct ELEMENTMAPtag
{
    KEYVALUE aKeys[MAP_MAXELEMENTS];
    VTWORD   wCount;
} ELEMENTMAP;

typedef struct FONTALIAStag *PFONTALIAS;
typedef struct FONTALIAStag 
{
    SCCUTFONTALIAS PrintFontAlias;
    PFONTALIAS     pNextFontAlias;
}FONTALIAS;

/* For the tons of email headers to choose from */
typedef struct EMAILHEADERINFOtag *PEMAILHEADERINFO;
typedef struct EMAILHEADERINFOtag
{
    SCCUTEMAILHEADERINFO sHeader;
    PEMAILHEADERINFO     next;
} EMAILHEADERINFO;

typedef struct MAILHEADERtag
{
    PEMAILHEADERINFO pCurHid;
    PEMAILHEADERINFO pCurVis;
    PEMAILHEADERINFO pHeadHid;
    PEMAILHEADERINFO pHeadVis;
} MAILHEADER, *PMAILHEADER;

typedef struct FONTOUTPUTtag
{
    SCCFONTOUTPUT         sFO;
    struct FONTOUTPUTtag* pNext;
} FONTOUTPUT;

typedef struct
{
    VTDWORD     dwOutput;
    FONTOUTPUT* pFontOutput;
} FONTOUTPUTLIST;

typedef struct FONTOUTPUTURLtag
{
    SCCFONTOUTPUTURL         sFOU;
    struct FONTOUTPUTURLtag* pNext;
} FONTOUTPUTURL;

typedef struct
{
    VTBYTE*        pszName;
    FONTOUTPUTURL* pFontOutputUrl;
} FONTOUTPUTURLLIST;

/* 
 |   The Options structure serves as places where new option values can be
 |   cached before the actual OIT options are set.  This allows the application
 |   to do things such as read in all new values and if parsing errors occur to 
 |   exit wihtout affecting the OIT option settings.   It also would allow an 
 |   application to load an option set and then continue to use this option
 |   set with multiple files without having to change the OIT global option
 |   space and without having to again parse the original source for the
 |   option values.
 */

#define SAMPLE_OPTION_OUTPUTID                                                1
#define SAMPLE_OPTION_TEMPLATE                                                2
#define SAMPLE_OPTION_FLAVOR                                                  3
#define SAMPLE_OPTION_GRAPHICTYPE                                             4
#define SAMPLE_OPTION_INTERLACEDGIF                                           5
#define SAMPLE_OPTION_JPEGQUALITY                                             6
#define SAMPLE_OPTION_CHARSET                                                 7
#define SAMPLE_OPTION_STRICT_DTD                                              8
#define SAMPLE_OPTION_GRAPHICSIZEMETHOD                                       9
#define SAMPLE_OPTION_CHARBYTEORDER                                          10
#define SAMPLE_OPTION_FALLBACKFORMAT                                         11
#define SAMPLE_OPTION_NOSOURCEFORMATTING                                     12
#define SAMPLE_OPTION_GRAPHICOUTPUTDPI                                       13
#define SAMPLE_OPTION_FALLBACKFONT                                           14
#define SAMPLE_OPTION_SIMPLESTYLENAMES                                       15
#define SAMPLE_OPTION_GENBULLETSANDNUMS                                      16
#define SAMPLE_OPTION_GRAPHICSIZELIMIT                                       17
#define SAMPLE_OPTION_GRAPHICWIDTHLIMIT                                      18
#define SAMPLE_OPTION_GRAPHICHEIGHTLIMIT                                     19
#define SAMPLE_OPTION_LABELWPCELLS                                           20
#define SAMPLE_OPTION_LABELSSDBCELLS                                         21
#define SAMPLE_OPTION_TEXTBUFFERSIZE                                         22
#define SAMPLE_OPTION_UNMAPPABLECHAR                                         23
#define SAMPLE_OPTION_HANDLENEWFILEINFO                                      24
#define SAMPLE_OPTION_OEMSTRING                                              25
#define SAMPLE_OPTION_OEMOUTPUT                                              26
#define SAMPLE_OPTION_LINKACTION                                             27
#define SAMPLE_OPTION_LINKLOCATION                                           28
#define SAMPLE_OPTION_CUSTOMELEMENT                                          29
#define SAMPLE_OPTION_CUSTOMELEMENTVALUE                                     30
#define SAMPLE_OPTION_PROCESSELEMENT                                         31
#define SAMPLE_OPTION_GRAPHICSKIPSIZE                                        32
#define SAMPLE_OPTION_WELLFORMED                                             33
#define SAMPLE_OPTION_GRAPHICBUFFERSIZE                                      34
#define SAMPLE_OPTION_COLLAPSEWHITESPACE                                     35
#define SAMPLE_OPTION_JAVASCRIPTTABS                                         36
#define SAMPLE_OPTION_MAXURLLENGTH                                           37
#define SAMPLE_OPTION_PAGESIZE                                               38
#define SAMPLE_OPTION_SEPARATEGRAPHICSBUFFER                                 39
#define SAMPLE_OPTION_CMCALLBACKVER                                          40
#define SAMPLE_OPTION_CMCALLBACKCHARSET                                      41
#define SAMPLE_OPTION_TEMPDIRECTORY                                          42
#define SAMPLE_OPTION_ALTLINK_PREV                                           43
#define SAMPLE_OPTION_ALTLINK_NEXT                                           44
#define SAMPLE_OPTION_CBALLDISABLED                                          45
#define SAMPLE_OPTION_CBCREATENEWFILE                                        46
#define SAMPLE_OPTION_CBNEWFILEINFO                                          47
#define SAMPLE_OPTION_CBPROCESSLINK                                          48
#define SAMPLE_OPTION_CBCUSTOMELEMENT                                        49
#define SAMPLE_OPTION_CBGRAPHICEXPORTFAILURE                                 50
#define SAMPLE_OPTION_CBOEMOUTPUT                                            51
#define SAMPLE_OPTION_CBALLENABLED                                           52
#define SAMPLE_OPTION_CBALTLINK                                              53
#define SAMPLE_OPTION_CBARCHIVE                                              54
#define SAMPLE_OPTION_GRIDROWS                                               55
#define SAMPLE_OPTION_GRIDCOLS                                               56
#define SAMPLE_OPTION_GRIDADVANCE                                            57
#define SAMPLE_OPTION_GRIDWRAP                                               58
#define SAMPLE_OPTION_SUPPRESSFONTSIZE                                       59
#define SAMPLE_OPTION_SUPPRESSFONTCOLOR                                      60
#define SAMPLE_OPTION_SUPPRESSFONTFACE                                       61
#define SAMPLE_OPTION_PREVENTGRAPHICOVERLAP                                  62
#define SAMPLE_OPTION_TEMPDIRECTORY_U                                        63
#define SAMPLE_OPTION_SUBSTITUEGRAPHIC                                       64
#define SAMPLE_OPTION_SUBSTITUTEGRAPHIC                                      65
#define SAMPLE_OPTION_SUBSTITUTEGRAPHIC_U                                    66
#define SAMPLE_OPTION_INPUTPATH                                              67
#define SAMPLE_OPTION_INPUTPATH_U                                            68
#define SAMPLE_OPTION_OUTPUTPATH                                             69
#define SAMPLE_OPTION_OUTPUTPATH_U                                           70
#define SAMPLE_OPTION_HREFPREFIX                                             71
#define SAMPLE_OPTION_FIRSTPREVHREF                                          72
#define SAMPLE_OPTION_TEMPLATE_U                                             73
#define SAMPLE_OPTION_ZTOQ                                                   74
#define SAMPLE_OPTION_REFLINK                                                75
#define SAMPLE_OPTION_HTML_EXTENSION                                         76
#define SAMPLE_OPTION_WML_EXTENSION                                          77
#define SAMPLE_OPTION_HDML_EXTENSION                                         78
#define SAMPLE_OPTION_XHTMLB_EXTENSION                                       79
#define SAMPLE_OPTION_CHTML_EXTENSION                                        80
#define SAMPLE_OPTION_HTMLWCA_EXTENSION                                      81
#define SAMPLE_OPTION_HTMLAG_EXTENSION                                       82
#define SAMPLE_OPTION_WIRELESSHTML_EXTENSION                                 83
#define SAMPLE_OPTION_TEXT_EXTENSION                                         84
#define SAMPLE_OPTION_HTML_CSS_EXTENSION                                     85
#define SAMPLE_OPTION_JAVASCRIPT_EXTENSION                                   86
#define SAMPLE_OPTION_JPEGFIF_EXTENSION                                      87
#define SAMPLE_OPTION_GIF_EXTENSION                                          88
#define SAMPLE_OPTION_WBMP_EXTENSION                                         89
#define SAMPLE_OPTION_BMP_EXTENSION                                          90
#define SAMPLE_OPTION_PNG_EXTENSION                                          91
#define SAMPLE_OPTION_MHTML_EXTENSION                                        92
#define SAMPLE_OPTION_XHTML_EXTENSION                                        93
#define SAMPLE_OPTION_FIFLAGS                                                94
#define SAMPLE_OPTION_XMLDEFMETHOD                                           95
#define SAMPLE_OPTION_XMLDEFREFERENCE                                        96
#define SAMPLE_OPTION_PSTYLENAMESFLAG                                        97
#define SAMPLE_OPTION_EMBEDDINGSFLAG                                         98
#define SAMPLE_OPTION_NOXMLDECLARATIONFLAG                                   99
#define SAMPLE_OPTION_OFFSETTRACKED                                         100
#define SAMPLE_OPTION_CHARBOLD                                              101
#define SAMPLE_OPTION_CHARITALIC                                            102
#define SAMPLE_OPTION_CHARUNDERLINE                                         103
#define SAMPLE_OPTION_CHARDUNDERLINE                                        104
#define SAMPLE_OPTION_CHAROUTLINE                                           105
#define SAMPLE_OPTION_CHARSTRIKEOUT                                         106
#define SAMPLE_OPTION_CHARSMALLCAPS                                         107
#define SAMPLE_OPTION_CHARALLCAPS                                           108
#define SAMPLE_OPTION_CHARHIDDEN                                            109
#define SAMPLE_OPTION_PARASPACING                                           110
#define SAMPLE_OPTION_PARAHEIGHT                                            111
#define SAMPLE_OPTION_PARALEFTINDENT                                        112
#define SAMPLE_OPTION_PARARIGHTINDENT                                       113
#define SAMPLE_OPTION_PARAFIRSTINDENT                                       114
#define SAMPLE_OPTION_PRINTERNAME                                           115
#define SAMPLE_OPTION_SHOWHIDDENTEXT                                        116
#define SAMPLE_OPTION_TIFFCOLORSPACE                                        117
#define SAMPLE_OPTION_TIFFCOMPRESSION                                       118
#define SAMPLE_OPTION_TIFFBYTEORDER                                         119
#define SAMPLE_OPTION_TIFFMULTIPAGE                                         120
#define SAMPLE_OPTION_DBPRINTFITTOPAGE                                      121
#define SAMPLE_OPTION_DBPRINTGRIDLINES                                      122
#define SAMPLE_OPTION_DBPRINTHEADINGS                                       123
#define SAMPLE_OPTION_PRINTMARGINSTOP                                       124
#define SAMPLE_OPTION_PRINTMARGINSBOTTOM                                    125
#define SAMPLE_OPTION_PRINTMARGINSLEFT                                      126
#define SAMPLE_OPTION_PRINTMARGINSRIGHT                                     127
#define SAMPLE_OPTION_PRINTENDPAGE                                          128
#define SAMPLE_OPTION_PRINTSTARTPAGE                                        129
#define SAMPLE_OPTION_SSPRINTDIRECTION                                      130
#define SAMPLE_OPTION_SSPRINTFITTOPAGE                                      131
#define SAMPLE_OPTION_SSPRINTGRIDLINES                                      132
#define SAMPLE_OPTION_SSPRINTHEADINGS                                       133
#define SAMPLE_OPTION_SSPRINTSCALEPERCENT                                   134
#define SAMPLE_OPTION_SSPRINTSCALEXHIGH                                     135
#define SAMPLE_OPTION_SSPRINTSCALEXWIDE                                     136
#define SAMPLE_OPTION_USEDOCPAGESETTINGS                                    137
#define SAMPLE_OPTION_VECPRINTASPECT                                        138
#define SAMPLE_OPTION_VECPRINTBACKGROUND                                    139
#define SAMPLE_OPTION_WHATTOPRINT                                           140
#define SAMPLE_OPTION_ACCEPT_ALT_GRAPHICS                                   141
#define SAMPLE_OPTION_SUBSTREAMROOTS_OBSOLETE                               142 
#define SAMPLE_OPTION_REMOVEFONTGROUPS_OBSOLETE                             143
#define SAMPLE_OPTION_INCLUDETEXTOFFSETS_OBSOLETE                           144
#define SAMPLE_OPTION_DEFAULTPRINTFONTFACE                                  145
#define SAMPLE_OPTION_DEFAULTPRINTFONTHEIGHT                                146
#define SAMPLE_OPTION_DEFAULTPRINTFONTATTR                                  147
#define SAMPLE_OPTION_DEFAULTPRINTFONTTYPE                                  148
#define SAMPLE_OPTION_PRINTFONTALIASID                                      149
#define SAMPLE_OPTION_PRINTFONTALIASFLAGS                                   150
#define SAMPLE_OPTION_PRINTFONTALIASORIGINAL                                151
#define SAMPLE_OPTION_PRINTFONTALIAS                                        152
#define SAMPLE_OPTION_TIFFMSB                                               153
#define SAMPLE_OPTION_GRAPHICWIDTH                                          154
#define SAMPLE_OPTION_GRAPHICHEIGHT                                         155
#define SAMPLE_OPTION_JPEGCOMPRESSION                                       156
#define SAMPLE_OPTION_LZWCOMPRESSION                                        157
#define SAMPLE_OPTION_EXTRACTEMBEDDEDFORMAT                                 158
#define SAMPLE_OPTION_CHAROCE                                               159
#define SAMPLE_OPTION_SHOWHIDDENSSDATA                                      160
#define SAMPLE_OPTION_SHOWCHANGETRACKING                                    161
#define SAMPLE_OPTION_SHOWSSDBROWCOLHEADINGS                                162
#define SAMPLE_OPTION_EXEPATH                                               163 
#define SAMPLE_OPTION_SUPPRESSPROPERTIES                                    164
#define SAMPLE_OPTION_PROCESSGENERATEDTEXT                                  165
#define SAMPLE_OPTION_FLATTENSTYLES                                         166
#define SAMPLE_OPTION_BITMAPASBITMAP                                        167
#define SAMPLE_OPTION_CHARTASBITMAP                                         168
#define SAMPLE_OPTION_PRESENTATIONASBITMAP                                  169
#define SAMPLE_OPTION_VECTORASBITMAP                                        170
#define SAMPLE_OPTION_NOBITMAPELEMENTS                                      171
#define SAMPLE_OPTION_NOCHARTELEMENTS                                       172
#define SAMPLE_OPTION_NOPRESENTATIONELEMENTS                                173
#define SAMPLE_OPTION_NOVECTORELEMENTS                                      174
#define SAMPLE_OPTION_ISODATES                                              175
#define SAMPLE_OPTION_REMOVEFONTGROUPS                                      176
#define SAMPLE_OPTION_INCLUDETEXTOFFSETS                                    177
#define SAMPLE_OPTION_SUPPRESSATTACHMENTS                                   178
#define SAMPLE_OPTION_SUPPRESSARCHIVESUBDOCS                                179
#define SAMPLE_OPTION_USEFULLFILEPATHS                                      180
#define SAMPLE_OPTION_TEXTOUT                                               181
#define SAMPLE_OPTION_MAXSSDBPAGEWIDTH                                      182
#define SAMPLE_OPTION_MAXSSDBPAGEHEIGHT                                     183
#define SAMPLE_OPTION_CHARREVISIONDELETE                                    184
#define SAMPLE_OPTION_CHARREVISIONADD                                       185
#define SAMPLE_OPTION_NULLREPLACEMENTCHAR                                   186
#define SAMPLE_OPTION_METADATAONLY                                          187
#define SAMPLE_OPTION_ANNOTATIONS                                           188
#define SAMPLE_OPTION_PRODUCEURLS                                           189
#define SAMPLE_OPTION_TRANSPARENCYCOLORRED                                  190
#define SAMPLE_OPTION_TRANSPARENCYCOLORGREEN                                191
#define SAMPLE_OPTION_TRANSPARENCYCOLORBLUE                                 192
#define SAMPLE_GRAPHIC_CROPPING                                             193
#define SAMPLE_OPTION_DELIMITERS                                            194
#define SAMPLE_OPTION_CA_XMLOUTPUT                                          195
#define SAMPLE_RENDERING_PREFER_OIT                                         196
#define SAMPLE_OPTION_SEPARATESTYLETABLES                                   197
#define SAMPLE_OPTION_WATERMARK                                             198
#define SAMPLE_OPTION_WATERMARKPOS                                          199
#define SAMPLE_OPTION_WATERMARKVERTPOS                                      200
#define SAMPLE_OPTION_WATERMARKHORZPOS                                      201
#define SAMPLE_OPTION_DEFAULTHEIGHT                                         202
#define SAMPLE_OPTION_DEFAULTWIDTH                                          203
#define SAMPLE_OPTION_UNITS                                                 204
#define SAMPLE_OPTION_WATERMARKPATH                                         205
#define SAMPLE_OPTION_FONTDIRECTORY                                         206
#define SAMPLE_OPTION_COMPRESS                                              207
#define SAMPLE_OPTION_OBJECTALL                                             208
#define SAMPLE_OPTION_PRODUCEOBJECTINFO                                     209
#define SAMPLE_OPTION_ENABLEERRORINFO                                       210
#define SAMPLE_OPTION_CELLINFO                                              211
#define SAMPLE_OPTION_WATERMARKIOTYPE                                       212
#define SAMPLE_OPTION_WATERMARKIOSCALE                                      213
#define SAMPLE_OPTION_WATERMARKIOPERCENT                                    214
#define SAMPLE_OPTION_FONTFILTERTYPE                                        215
#define SAMPLE_OPTION_FONTFILTERLIST                                        216
#define SAMPLE_OPTION_ODGRAPHICOPTIONS                                      217
#define SAMPLE_OPTION_FORMULAS                                              218
#define SAMPLE_OPTION_UNMAPPEDTEXT                                          219
#define SAMPLE_OPTION_TIMEZONE                                              220
#define SAMPLE_OPTION_DEFAULTINPUTCHARSET                                   221
#define SAMPLE_OPTION_NUMBEROFSTATCALLBACKS                                 222
#define SAMPLE_GRAPHIC_WATERMARK_OPACITY                                    223
#define SAMPLE_GRAPHIC_WATERMARK_POSITION                                   224
#define SAMPLE_GRAPHIC_WATERMARK_SCALETYPE                                  225
#define SAMPLE_GRAPHIC_WATERMARK_SCALEPERCENT                               226
#define SAMPLE_GRAPHIC_WATERMARK_HORIZONTALPOS                              227
#define SAMPLE_GRAPHIC_WATERMARK_VERTICALPOS                                228
#define SAMPLE_GRAPHIC_WATERMARK_PATH                                       229
#define SAMPLE_OPTION_SSDBBORDEROPTIONS                                     230
#define SAMPLE_OPTION_EMBEDFONTS                                            231
#define SAMPLE_OPTION_OPTIMIZESECTIONS                                      232
#define SAMPLE_OPTION_CHARMAPPING                                           233
#define SAMPLE_OPTION_IOBUFFERSIZEFLAG                                      234
#define SAMPLE_OPTION_READBUFFERSIZE                                        235
#define SAMPLE_OPTION_MAPBUFFERSIZE                                         236
#define SAMPLE_OPTION_TEMPBUFFERSIZE                                        237
#define SAMPLE_OPTION_EXTRACTXMPMETADATA                                    238
#define SAMPLE_OPTION_PARSEXMPMETADATA                                      239
#define SAMPLE_OPTION_OCRQUALITY                                            240
#define SAMPLE_OPTION_OCRTECH                                               241
#define SAMPLE_OPTION_REDIRECTTEMPFILE                                      242
#define SAMPLE_OPTION_LINEARUNITS                                           243
#define SAMPLE_OPTION_MIMEHEADER                                            244
#define SAMPLE_OPTION_DOCUMENTMEMORYMODE                                    245
#define SAMPLE_OPTION_DOLINEARIZATION                                       246
#define SAMPLE_OPTION_ISODATETIMES                                          247
#define SAMPLE_OPTION_QUICKTHUMBNAIL                                        248
#define SAMPLE_OPTION_SUPPRESSMETADATA                                      249
#define SAMPLE_OPTION_TIFFONESTRIP                                          250
#define SAMPLE_OPTION_NOTESID                                               251
#define SAMPLE_OPTION_PASSWORD                                              252
#define SAMPLE_OPTION_PASSWORD_U                                            253
#define SAMPLE_OPTION_REORDERMETHOD                                         254
#define SAMPLE_OPTION_LOTUSNOTESDIRECTORY                                   255                                              
#define SAMPLE_OPTION_EMAILHEADER                                           256
#define SAMPLE_OPTION_SSSHOWHIDDENCELLS                                     257
#define SAMPLE_OPTION_GENERATESYSTEMMETADATA                                258
#define SAMPLE_OPTION_UNIXOPTIONSFILE                                       259
#define SAMPLE_OPTION_PROCESSARCHIVESUBDOCS                                 260
#define SAMPLE_OPTION_PROCESSEMBEDDINGSUBDOCS                               261
#define SAMPLE_OPTION_PROCESSATTACHMENTSUBDOCS                              262
#define SAMPLE_OPTION_PDF_FILTER_REORDER_BIDI                               263
#define SAMPLE_OPTION_STRICTFILEACCESS                                      264
#define SAMPLE_OPTION_OLEEMBEDDINGS                                         265
#define SAMPLE_OPTION_HTML_COND_COMMENT_MODE                                266
#define SAMPLE_OPTION_UNRESTRICTEDFONTEMBEDDING                             267
#define SAMPLE_OPTION_SKIPSTYLES                                            268
#define SAMPLE_GRAPHIC_RENDERASPAGE                                         269
#define SAMPLE_OPTION_EXPORTATTACHMENTS                                     270
#define SAMPLE_OPTION_LOAD_OPTIONS                                          271
#define SAMPLE_OPTION_SAVE_OPTIONS                                          272
#define SAMPLE_OPTION_COMPRESS_24BIT_TYPE                                   273
#define SAMPLE_OPTION_EXPORT_STATUS_TYPE                                    274
#define SAMPLE_OPTION_MAILHEADERVISIBLE                                     275
#define SAMPLE_OPTION_MAILHEADERHIDDEN                                      276
#define SAMPLE_OPTION_MAILHEADERFLAGS                                       277
#define SAMPLE_OPTION_MAILHEADERNAME                                        278
#define SAMPLE_OPTION_MAILHEADERLABEL                                       279
#define SAMPLE_OPTION_IMAGE_PASSTHROUGH                                     280
#define SAMPLE_OPTION_FONTEMBEDPOLICY                                       281
#define SAMPLE_OPTION_IGNOREPASSWORD                                        282
#define SAMPLE_OPTION_PDF_FILTER_DROPHYPHENS                                283
#define SAMPLE_OPTION_PERFORMANCEMODE                                       284
#define SAMPLE_OPTION_CELLHIDDEN                                            285
#define SAMPLE_OPTION_STROKETEXT                                            286
#define SAMPLE_OPTION_OUTPUT_STRUCTURE                                      287
#define SAMPLE_OPTION_RAWTEXT                                               288
#define SAMPLE_OPTION_URLPATH_RESOURCES                                     289
#define SAMPLE_OPTION_URLPATH_OUTPUT                                        290
#define SAMPLE_OPTION_POST_LIBRARY_SCRIPT                                   291
#define SAMPLE_OPTION_PRE_LIBRARY_SCRIPT                                    292
#define SAMPLE_OPTION_FONTPERMISSIONS                                       293
#define SAMPLE_OPTION_FONTBASEURL                                           294
#define SAMPLE_OPTION_GENERATE_EXCEL_REVISIONS                              295
#define SAMPLE_OPTION_FONTREFERENCEMETHOD                                   296
#define SAMPLE_OPTION_ATTACHMENTHANDLING                                    297
#define SAMPLE_OPTION_EXTERNAL_CSS                                          298
#define SAMPLE_OPTION_FILTERNOBLANK                                         299
#define SAMPLE_OPTION_RENDER_EMBEDDED_FONTS                                 300
#define SAMPLE_RENDER_DISABLEALPHABLENDING                                  301
#define SAMPLE_OPTION_IMAGESTAMP_FILE                                       302
#define SAMPLE_OPTION_IMAGESTAMP_URL                                        303
#define SAMPLE_OPTION_IMAGESTAMP_SPECTYPE                                   304
#define SAMPLE_OPTION_IMAGESTAMP_NAME                                       305
#define SAMPLE_OPTION_WVLIBRARYNAME                                         306
#define SAMPLE_OPTION_WVSTYLESHEETNAME                                      307
#define SAMPLE_RENDER_ENABLEALPHABLENDING                                   308
#define SAMPLE_OPTION_TIFFGRAY2COLOR                                        309
#define SAMPLE_OPTION_REDACTIONCOLOR                                        310
#define SAMPLE_OPTION_SHOWREDACTIONLABELS                                   311
#define SAMPLE_OPTION_REDACTION_LABEL_FONT_NAME                             312
#define SAMPLE_OPTION_REDACTIONS_ENABLED                                    313
#define SAMPLE_OPTION_HANDLEENDPAGE                                         314
#define SAMPLE_OPTION_VECTOROBJECTLIMIT                                     315
#define SAMPLE_OPTION_PDF_FILTER_WORD_DELIM_FRACTION                        316
#define SAMPLE_OPTION_PDF_FILTER_MAX_EMBEDDED_OBJECTS                       317
#define SAMPLE_OPTION_PDF_FILTER_MAX_VECTOR_PATHS                           318
#define SAMPLE_GRAPHIC_WATERMARK_HORIZONTAL_OFFSET                          319
#define SAMPLE_GRAPHIC_WATERMARK_VERTICAL_OFFSET                            320
#define SAMPLE_GRAPHIC_WATERMARK_HORIZONTAL_OFFSETFROM                      321
#define SAMPLE_GRAPHIC_WATERMARK_VERTICAL_OFFSETFROM                        322
#define SAMPLE_GRAPHIC_WATERMARK_IOTYPE                                     323
#define SAMPLE_OPTION_EMAIL_FIXEDWIDTH                                      324
#define SAMPLE_OPTION_HTML_FIXEDWIDTH                                       325
#define SAMPLE_OPTION_PLAINTEXT_PAGINATION                                  326
#define SAMPLE_OPTION_PAGES_PER_FILE                                        327
#define SAMPLE_OPTION_RECIPIENT_DELIVERY_INFORMATION						328
#define SAMPLE_OPTION_BACKGROUND_COLOR										329
/* tvarshne Pdf Space Improvement Plan-1 11/28/2020*/
#define SAMPLE_OPTION_PDF_FILTER_NEW_SPACING_ALGORITHM                      330
#define SAMPLE_OPTION_END                                                   330

#define NUM_OPTIONS (SAMPLE_OPTION_END + 1)


typedef struct Option_tag
{
    VTCHAR              *szTemplate;                                        /* Template file name                                                   */
    VTWORD              *wzTemplate;                                        /* Template file name in Unicode                                        */
    VTCHAR              *szInFile;                                          /* Input file to convert                                                */
    VTWORD              *wzInFile;                                          /* Input file to convert in Unicode                                     */
    VTCHAR              *szOutFile;                                         /* Output file                                                          */
    VTWORD              *wzOutFile;                                         /* Output file in Unicode                                               */
    VTDWORD             dwOutputID;                                         /* Output file ID                                                       */
    VTDWORD             dwFlavor;                                           /* flavor to output                                                     */
    VTDWORD             dwGraphicType;                                      /* Output graphics type                                                 */
    VTBOOL              bInterlacedGIF;                                     /* Set to TRUE to export interlaced GIFs                                */
    VTDWORD             dwFlags;                                            /* Various SCCHTML_FLAG bits.                                           */
    VTDWORD             dwJPEGQuality;                                      /* Output JPEG quality 100=Highest quality, lowest compression.         */
    VTDWORD             dwCharset;                                          /* Output character set to use.                                         */
    VTDWORD             dwCharByteOrder;                                    /* Character byte ordering to be used with Unicode                      */
    VTDWORD             dwSizeMethod;                                       /* Which sizing algorithm to use.                                       */
    VTDWORD             dwFallbackFormat;                                   /* Assumed file format when file cannot be identified                   */
    VTDWORD             dwFIFlags;                                          /* Do an extended check for 7-bit ASCII input files?                    */
    VTBOOL              bNoSourceFormatting;                                /* Set to TRUE to disable source formatting                             */
    VTDWORD             dwGraphicOutputDPI;                                 /* DPI to output graphics at                                            */
    SCCUTFALLBACKFONT   FallbackFont;                                       /* Alternate font for browser to use if preferred font is not available */
    VTBOOL              bPreventGraphicOverlap;                             /* Prevent graphics from overlapping                                    */
    VTBOOL              bSimpleStyleNames;                                  /* Discard invalid style name characters                                */
    VTCHAR              szFallbackFontName[SCCUT_MAXFALLBACKFONTLEN * 2];   /* May be a DBCS string                                                 */
    VTBOOL              bGenBulletsAndNums;                                 /* Generate bullets and numbers instead of using list tags              */
    VTDWORD             dwGraphicSizeLimit;                                 /* Maximum # of pixels in exported graphics                             */
    VTDWORD             dwGraphicWidthLimit;                                /* Maximum width of exported graphics (pixels)                          */
    VTDWORD             dwGraphicHeightLimit;                               /* Maximum height of exported graphics (pixels)                         */
    VTDWORD             dwLabelFlags;                                       /* Flags for labeling list items and table cells                        */
    VTDWORD             dwTextBufferSize;                                   /* Wireless Export text buffer size                                     */
    VTWORD              wUnmappableChar;                                    /* Unmappable character                                                 */
    VTBOOL              bHandleNewFileInfo;                                 /* Set to TRUE to handle EX_CALLBACK_ID_NEWFILEINFO                     */
    VTBOOL              bHandleEndPage;                                     /* Set to TRUE to handle EX_CALLBACK_ID_ENDPAGE                         */
    VTBOOL              bZtoQ;                                              /* Set to TRUE to map z's to q's on output                              */
    VTBOOL              bRefLink;                                           /* Set to TRUE to handle EX_CALLBACK_ID_REFLINK                         */
    VTDWORD             dwLinkAction;
    VTCHAR              szLinkLocation[VT_MAX_URL];
    OEMSTRMAP           mapOEM;                                             /* Map of OEM strings                                                   */
    ELEMENTMAP          mapCustom;                                          /* Map of custom element keywords and values                            */
    VTLPSTR             apstrCustomElements[MAP_MAXELEMENTS + 1];
    VTDWORD             dwGraphicSkipSize; 
    VTDWORD             dwGraphicBufferLimit;                               /* Maximum size of the graphics buffer in bytes                         */
    VTBOOL              bCollapseWhitespace;                                /* Set to TRUE to remove excess whitespace                              */
    VTDWORD             dwMaxURLLength;                                     /* Max. length of URLs in characters                                    */
    VTBOOL              bJSTabs;                                            /* Using Javascript to handle the tab stop                              */
    VTDWORD             dwPageSize;                                         /* Page size in characters                                              */
    VTBOOL              bSeparateGraphicsBuffer;                            /* Device has separate buffers for graphics and text                    */
    VTDWORD             wCMCallbackVersion;                                 /* Either 1 or 0 (default)                                              */
    VTDWORD             dwCMCallbackCharset;                                /* character set used with char mapping callbacks                       */
    VTCHAR              szTempDir[VT_MAX_URL];                              /* Directory to use for storing temporary files                         */
    VTWORD              wzTempDir[VT_MAX_URL];                              /* Directory to use for storing temporary files in Unicode              */
    VTCHAR              szSubstituteGraphic[VT_MAX_URL];                    /* URL of graphic image to use in case of graphic conversion failure    */
    VTWORD              wzSubstituteGraphic[VT_MAX_URL];                    /* Unicode version of the szSubstituteGraphic parameter                 */
    VTCHAR              szURLPrefix[VT_MAX_URL];                            /* URL prefix for generated links                                       */
    VTWORD              wzFirstPrevURL[VT_MAX_URL];                         /* URL for the "previous" link for the first "previous" link            */
    VTCHAR              szCreateFilePath[BUF_SIZE];                         /* Output path to use for the EX_CALLBACK_ID_CREATENEWFILE              */
    VTCHAR              aszAltLinkURLs[2][VT_MAX_URL];                      /* URLs for EX_CALLBACK_ID_ALTLINK                                      */
    VTDWORD             dwCallbackFlags;                                    /* Flags indicating which callbacks are enabled                         */
    VTDWORD             dwGridRows;                                         /* Number of rows in the grid                                           */
    VTDWORD             dwGridCols;                                         /* Number of columns in the grid                                        */
    VTDWORD             dwGridAdvance;                                      /* Traversal path of the spreadsheet                                    */
    VTBOOL              bGridWrap;                                          /* Indicates whether grid output should continue when the edge          */
    VTDWORD             dwFontFlags;                                        /* Font attribute suppression flags                                     */
    VTDWORD             dwUnmappedText;                                     /* Control production of unmapped text in SearchML.                     */
    VTDWORD             dwXMLDefMethod;                                     /* XML Definition method (DTD, XSD, NONE)                               */
    VTWORD              wzXMLDefReference[MAX_DEF_REFERENCE_LENGTH];        /* XML Definition reference (Unicode).                                  */
    VTBOOL              bShowHiddenText;                                    /* Flag indicating if we will show hidden text or not                   */
    VTDWORD             dwExtractEmbeddedFormat;                            /* Extract attachment format                                            */
    VTBOOL              bShowHiddenSSData;                                  /* Flag: will we export spreadsheet hidden columns/rows/sheets          */
    VTBOOL              bFilterNoBlank;                                     /* Produce (or not) blank spreadsheet pages in the export               */
    VTBOOL              bShowSpreadSheetBorder;                             /* Flag: will we export spread sheet border                             */
    VTBOOL              bShowChangeTracking;                                /* Flag: will we export change tracking for word processing document    */
    VTBOOL              bShowSSDBRowColHeadings;                            /* Flag: will we export spreadsheet row and column heading              */
    VTDWORD             dwSSDBBorderOptions;                                /* SS/DB border options                                                 */
    VTLONG              lTimeZone;                                          /* Value for the time zone                                              */
    VTDWORD             dwDefaultInputCharset;                              /* Default input character set for the filter                           */
    SCCBUFFEROPTIONS    bufferOpts;                                         /* an option to define the size to use for a number of I/O buffers.     */
    VTBOOL              bRedirectTempFile;                                  /* Redirect temp file                                                   */
    VTBOOL              bParseXMPMetadata;                                  /* Parse xmp metadata                                                   */
    VTWORD              wNullReplacementChar;                               /* Null replacement character                                           */

    /* XML Export - SearchML Specific Options */
    VTDWORD             dwSearchMLFlags;                                    /* SearchML Export flags                                                */
    VTBOOL              bOffsetTracked;                                     /* SearchML Offset tracked                                              */
    VTDWORD             dwCharAttrs;                                        /* SearchML Character Attributes                                        */
    VTDWORD             dwParaAttrs;                                        /* SearchML Paragraph Attributes                                        */
    VTBOOL              bXMLOutput;                                         /* CATest produces UTF8 encoded XML.                                    */
    VTBOOL              bExtractXMPMetadata;
	VTDWORD				dExtractRecipientDeliveryInfo;

	/* PDF Export - Document Background Color Option */
	VTBOOL				bBackgroundColor;
        
    /* XML Export - PageML Specific Options   */                             
    VTDWORD             dwPageMLFlags;                                      /* PageML Export flags                                                  */
    VTCHAR              szPrinterName[BUF_SIZE];                            /* PageML Printer whose metrics will be used for formatting the output  */
    
    /* Image Export Specific Options          */
    EXTIFFOPTIONS       sEXTiffOptions;                                     /* Struct that holds the tiff options                                   */
    VTDWORD             dwDBFitToPage;                                      /* Specifies the type of scaling to be done for database files          */
    VTBOOL              bDBShowGridlines;                                   /* Specifies if we should show gridlines in database files              */
    VTBOOL              bDBShowHeadings;                                    /* Specifies if we should show headers in database files                */
    SCCVWPRINTMARGINS   sDefaultMargins;                                    /* Margins to use if not using the margins specified in the document    */
    VTDWORD             dwExportEndPage;                                    /* Last page in range to be exported                                    */   
    VTDWORD             dwExportStartPage;                                  /* First page in range to be exported                                   */
    VTDWORD             dwSSDirection;                                      /* Direction in which spreadsheets data should be export                */
    VTDWORD             dwSSFitToPage;                                      /* Specifies the type of scaling to be done for spreadsheet files       */ 
    VTBOOL              bSSShowGridlines;                                   /* Specifies if we should show gridlines in spreadsheet files           */
    VTBOOL              bSSShowHeadings;                                    /* Specifies if we should show headings in spreadsheet files            */
    VTDWORD             dwSSScalePercent;                                   /* Percentage by which to scale spreadsheet pages                       */
    VTDWORD             dwSSScaleXHigh;                                     /* Number of vertical pages to scale spreadsheet output to              */
    VTDWORD             dwSSScaleXWide;                                     /* Number of horizontal pages to scale spreadsheet output to            */
    VTBOOL              bUseDocPageSettings;                                /* Specifies if the native document page layout should be used          */
    VTDWORD             dwVecAspect;                                        /* Maintain Aspect Ratio of vector formats or Stretch to fill area      */
    VTBOOL              bVecShowBackground;                                 /* Display background of vector images                                  */ 
    VTDWORD             dwWhatToExport;                                     /* Specifies which pages should be exported                             */
    SCCUTFONTSPEC       DefaultPrintFont;                                   /* Set the default print font                                           */
    SCCUTFONTALIAS      PrintFontAlias;                                     /* Set the print font alias                                             */
    FONTALIAS           FontAlias;                                          /* List of font aliases to be used by OIT                               */
    VTDWORD             dwFillOrder;                                        /* Set the FillOrder of the tiff image                                  */
    VTDWORD             dwGraphicWidth;
    VTDWORD             dwGraphicHeight; 
    VTDWORD             dwJpegCompression;
    VTDWORD             dwLzwCompression;
    
    /* XML Writer options */
    VTLPCWSTR           pwstrSubstreamRoots;                                /* [OBSOLETE - See CCFLEX_FORMATOPTIONS_SEPARATESTYLETABLES]            */
    VTBOOL              bAcceptAltGraphics;                                 /* Set to TRUE to cause {GIF,JPG,PNG} to be left as-is.                 */
    VTDWORD             dwCCFlexFormatOptions;                              /* Controls how non-raster embeddings are exported.                     */
    VTDWORD             dwRed;
    VTDWORD             dwGreen;
    VTDWORD             dwBlue;
    VTDWORD             dwImageCropping;
    VTDWORD             dwImageWatermarkOpacity;
    VTDWORD             dwImageWatermarkScaleType;
    VTDWORD             dwImageWatermarkScalePercent;
    VTLONG              lImageWatermarkHorizontalPos;
    VTLONG              lImageWatermarkVerticalPos;
	VTDWORD             dwImageWatermarkHorizontalOffsetFrom;
	VTDWORD             dwImageWatermarkVerticalOffsetFrom;
	VTLONG              lImageWatermarkHorizontalOffset;
	VTLONG              lImageWatermarkVerticalOffset;
    VTLPBYTE            pWatermarkSpec;
    VTDWORD             dwWatermarkSpecSize;
    VTDWORD		        dwImageWatermarkPathType;
    VTBOOL              bRenderAsPage;
    VTDWORD             dwMaxSSDBPageWidth;
    VTDWORD             dwMaxSSDBPageHeight; 
    VTBOOL              bOutputSolution;
    VTBOOL              bRenderDisableAlphaBlending;
    VTBOOL              bRenderEnableAlphaBlending;
    VTBOOL              bEnableWatermark;
    WATERMARKPOS        WatermarkPos;
    DEFAULTPAGESIZE     DefaultPageSize;
    WATERMARKIO         WatermarkIO;
    VTCHAR              *szFontDirectory; 
    VTBOOL              bCompress;
    VTBOOL              bAllObjects;
    FONTFILTERLIST      FontFilterList;
    VTBOOL              bEmbedFonts;
    VTDWORD             dwFontEmbedPolicy;
    VTBOOL              bDoLinearization;
    VTDWORD             dwCompress24BitType;
    VTBOOL              bExportAttachments;
    VTBOOL              bDirectImagePassThrough;
    VTBOOL              bUnrestrictedFontEmbedding;
    VTBOOL              bPrintOpts;
    VTDWORD             dwLinearUnits;
    VTDWORD             dwEmailHeader;
    
    /* Open Document Export Options */
    VTDWORD             dwODGraphicOptions;

    VTDWORD             dwNumStatCallbacks;    
    SCCUTNUMBERFORMAT   sNumberFormat;
    VTDWORD             dwOcrTech;
    VTDWORD             dwOcrQuality;
    VTDWORD             dwDocumentMemoryMode;                               /* Allowable amount of chunker memory.                                  */
    VTBOOL              bSuppressMetadata;
    VTDWORD             dwPerformanceMode;
    VTDWORD             dwGenFlags;                                         /* Generic flags that can apply to any product.                         */
    VTBOOL              bQuickThumbnail;
    VTWORD              wOLEEmbeddingMode;

    /* File access */
    VTCHAR              szPassword[MAX_PASSWORD][VT_MAX_FILEPATH];          /* Non-unicode password                                                 */
    VTWORD              wzPassword[MAX_PASSWORD][VT_MAX_FILEPATH];          /* Unicode password                                                     */
    VTCHAR              szNotesId[MAX_NOTESID][VT_MAX_FILEPATH];
    VTWORD              wPasswordNum;
    VTWORD              wNotesIdNum;
    VTDWORD             dwReorderMethod;
    VTCHAR              szLotusNotesDirectory[VT_MAX_FILEPATH]; 
    VTBOOL              bSSShowHiddenCells;
    VTDWORD             dwPDF_Filter_Reorder_BIDI;
    VTDWORD             dwHtmlCondCommentMode;
    VTDWORD             dwExportStatusType;                                 /* value passed into EXExportStatus().                                  */
    VTBOOL              bLoadOptions;                                       /* If TRUE, Load the options from disk during DAInitEx                  */
    VTBOOL              bSaveOptions;                                       /* If TRUE, save the options to disk during DADeInit                    */
    MAILHEADER          sMailHeader;
    VTDWORD             dwMailHeaderFlags;
    VTBOOL              bHeaderVisible;
    VTBOOL              bIgnorePassword;
    VTDWORD             dwPDF_Filter_DropHyphens;
    VTBOOL              bStrokeText;
    VTBOOL              bGenerateExcelRevisions;

    /* Options file name */
    VTBOOL              bPrintUnixOptionsFile;
    SCCUTUNIXOPTIONSFILEINFO sUnixOptionsFileInfo;

    VTDWORD             dwOutputStructure;
    VTBOOL              bRawText;
    VTBYTE              *pszResourceURL;
    VTBYTE              *pszOutputURL;
    VTBYTE              **ppExternalCss;
    VTBYTE              **ppPostLibScript;
    VTBYTE              **ppPreLibScript;
    EXANNOSTAMPIMAGE    **ppImageStampFile;
    EXANNOSTAMPIMAGE    **ppImageStampUrl;
    VTDWORD             dwCountPostLibScript;
    VTDWORD             dwCountPreLibScript;
    VTDWORD             dwCountImageStampFile;
    VTDWORD             dwCountImageStampUrl;
    VTDWORD             dwCountExternalCss;
    VTBYTE              *pszBaseURL;
    VTBYTE              *pszLibraryName;
    VTBYTE              *pszStylesheetName;
    VTDWORD             dwFontPermissions;
    VTDWORD             dwFontReferenceMethod;
    VTDWORD             dwEmailAttachmentHandling;
    VTBOOL              bRenderEmbeddedFonts;
    VTDWORD             dwImageStampSpecType;
    VTBYTE              *pszImageStampName;
    VTDWORD             dwRedactionColor;
    VTBOOL              bShowRedactionLabels;
    VTBOOL              bEnableRedactions;
    VTWORD              *pwzRedactionLabelFontName;
    VTDWORD             dwVectorObjectLimit;
    VTFLOAT             fPdfWordDelimFraction;
    VTDWORD             dwPdfMaxEmbeddedObjects;
    VTDWORD             dwPdfMaxVectorPaths;
    VTBOOL              bEmailFixedWidth;
    VTBOOL              bHTMLFixedWidth;
    VTBOOL              bPlainTextPagination;
    VTDWORD             dwPagesPerFile;
	VTBOOL              bNewPDFspacing;  /*tvarshne Pdf Space Improvement Plan-1 11/28/2020 */
    /* Needs to be here to avoid threading conflicts. */
    VTBOOL abSetOptions[NUM_OPTIONS];
} Option;

typedef struct
{
    VTLPSTR Str;
    VTDWORD Num;
} StringDW;

typedef struct
{
    VTLPCSTR  Str;
    VTLPCWSTR pwStr;
} StringWS;

#define STRINGDW_LISTEND  0xFFFFFFFFL


/*  Function prototypes  */
VTVOID  SetBufferSize(VTHDOC hDoc, Option *pOptions);
SCCERR  SetOptions(VTHDOC hDoc, Option *pOptions);
SCCERR  ReadConfiguration(int argc, char *argv[], Option *pOptions);
SCCERR  ReadCommandLine(int argc, char *argv[], Option *pOptioins, FILE **pFile);
SCCERR  ParseConfigFile(FILE *fp, Option *pOptions);
SCCERR  StoreOption(VTLPSTR keyword, VTLPSTR value, VTDWORD dwOptCode, VTDWORD dwLine, Option *pOptions);
SCCERR  StringToDWEx(StringDW *array, VTLPSTR Str, VTDWORD *Num, VTLPSTR ErrMsg, VTDWORD dwLine);
SCCERR  StringToDW(StringDW* array, VTLPSTR keyword, VTDWORD* dwOption);
SCCERR  StringToWideStr(StringWS *array, VTLPSTR keyword, VTLPCWSTR *ppOption);
VTBOOL  DWToString(const StringDW* array, const VTDWORD dwId, VTLPSTR string);
VTVOID  PrintOpts(Option *pOptions);
VTDWORD ConvertASCIIToLittleUnicode(VTLPVOID pOut, VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize);
VTDWORD ConvertASCIIToBigUnicode(VTLPVOID pOut, VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize);
VTDWORD ConvertLittleUnicodeToASCII(VTLPVOID pOut,VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize);
VTDWORD ConvertBigUnicodeToASCII(VTLPVOID pOut,VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize);
VTDWORD ConvertUnicodeToUTF8(VTLPVOID pOut,VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize);
VTBYTE  GetDigit(char ch);
VTVOID  StrToWord(VTLPSTR pString, VTLPWORD pwVal);
VTDWORD Swcslen(VTLPCWSTR pwStr);
VTDWORD base64decode(VTLPBYTE pInput, VTLPBYTE pOutput, VTDWORD inSize, VTLPDWORD pOutSize);
VTDWORD decodeUnicode(VTLPCWSTR pInput, VTLPWSTR pOutput, VTDWORD inSize, VTLPDWORD pOutSize);
VTVOID  SetCustomFileExtension(VTWORD fi, VTLPSTR value);
SCCERR  HexStrToUnicodeStr(VTLPSTR pString, VTLPWORD pwVal);
VTVOID  CleanupOpts(Option *pOptions);
SCCERR  HandleEndPage();

#endif /* _CFG_H__ */

