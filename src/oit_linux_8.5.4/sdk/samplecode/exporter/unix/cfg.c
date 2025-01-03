/*
 | C EXPORT SDKS
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
 | Note that not all of the options handled here are used by all
 | the products supported by this sample app.
 */

#include "cfg.h"
#include <sccca.h>

#ifndef UNUSED
#define UNUSED(x) ((x) = (x))
#endif

typedef VTDWORD(*ASCIITOUNICODEFUNC)(VTLPVOID pOutput, VTDWORD dwOutSize, VTLPVOID pInput, VTDWORD dwInSize);
typedef VTDWORD(*UNICODETOASCIIFUNC)(VTLPVOID pOutput, VTDWORD dwOutSize, VTLPVOID pInput, VTDWORD dwInSize);

#define FREEAR(ptr) if (ptr) free(ptr)
#define ALLOC(type,ptr,bytes) FREEAR(ptr); (ptr) = (type *)malloc(bytes); if (!(ptr)) return SCCERR_ALLOCFAILED; (void)0

ASCIITOUNICODEFUNC ConvFunc;
UNICODETOASCIIFUNC UnicodeToAsciiFunc;

/* defines need for the base-64 decoding */
#define BASE64_OK               0
#define BASE64_BUFFERTOOSMALL   1
#define BASE64_BADENCODING      2

VTDWORD g_FileCount = 0;   /* Counter used for the hrefprefix option */
VTBYTE  *g_szInitFilePath; /* Path of the initial output file excluding the file name */
VTBYTE  *g_szInitFileName; /* file name of the initial output file, excluding the extension */


StringDW Options[] =
{
	{(char *)"outputid", SAMPLE_OPTION_OUTPUTID},							/* Output ID                                                               */
	{ (char *)"template", SAMPLE_OPTION_TEMPLATE},							/* Name of the template file                                               */
	{ (char *)"template_u", SAMPLE_OPTION_TEMPLATE_U},						/* Name of the template file in Unicode                                    */
	{ (char *)"flavor", SAMPLE_OPTION_FLAVOR},								/* HTML/Wireless output flavor (from Flavors[] list)                       */
	{ (char *)"graphictype", SAMPLE_OPTION_GRAPHICTYPE},						/* Output graphic type (from GTypes[] list)                                */
	{ (char *)"gifinterlace", SAMPLE_OPTION_INTERLACEDGIF},					/* Export interlaced GIFs                                                  */
	{(char *)"jpegquality", SAMPLE_OPTION_JPEGQUALITY},						/* 0-100, 100=Highest quality and lowest compression.                      */
	{(char *)"charset", SAMPLE_OPTION_CHARSET},								/* Output character set (from Charsets[] list)                             */
	{(char *)"strictdtd", SAMPLE_OPTION_STRICT_DTD},						/* TRUE to maintain strict conformance to the flavor's DTD                 */
	{(char *)"graphicsizemethod", SAMPLE_OPTION_GRAPHICSIZEMETHOD},			/* Graphic sizing method to use                                            */
	{(char *)"charbyteorder", SAMPLE_OPTION_CHARBYTEORDER},					/* Character byte ordering for Unicode output                              */
	{(char *)"fallbackformat", SAMPLE_OPTION_FALLBACKFORMAT},				/* Assumed file format when file cannot be identified                      */
	{(char *)"fiflags", SAMPLE_OPTION_FIFLAGS},								/* Do an extended check on 7-bit ASCII files?                              */
	{(char *)"nosourceformatting", SAMPLE_OPTION_NOSOURCEFORMATTING},		/* Use source formatting or not                                            */
	{(char *)"graphicoutputdpi", SAMPLE_OPTION_GRAPHICOUTPUTDPI},			/* Output resolution used when converting graphics                         */
	{(char *)"fallbackfont", SAMPLE_OPTION_FALLBACKFONT},					/* Alternate font for browser to try.                                      */
	{(char *)"preventgraphicoverlap", SAMPLE_OPTION_PREVENTGRAPHICOVERLAP}, /* Prevent graphics from overlapping                                       */
	{(char *)"simplestylenames", SAMPLE_OPTION_SIMPLESTYLENAMES},			/* Discard invalid style name characters                                   */
	{(char *)"genbulletsandnums", SAMPLE_OPTION_GENBULLETSANDNUMS},			/* Generate bullets and numbers instead of using list tags                 */
	{(char *)"graphicsizelimit", SAMPLE_OPTION_GRAPHICSIZELIMIT},			/* Max. size of exported graphics (pixels)                                 */
	{(char *)"graphicwidthlimit", SAMPLE_OPTION_GRAPHICWIDTHLIMIT},			/* Max. width of exported graphics (pixels)                                */
	{(char *)"graphicheightlimit", SAMPLE_OPTION_GRAPHICHEIGHTLIMIT},		/* Max. width of exported graphics (pixels)                                */
	{(char *)"labelwpcells", SAMPLE_OPTION_LABELWPCELLS},					/* Label table cells in word-processing documents                          */
	{(char *)"labelssdbcells", SAMPLE_OPTION_LABELSSDBCELLS},				/* Label spreadsheet and database cells                                    */
	{(char *)"textbuffersize", SAMPLE_OPTION_TEXTBUFFERSIZE},				/* Wireless Export text buffer size                                        */
	{(char *)"unmappablechar", SAMPLE_OPTION_UNMAPPABLECHAR},				/* Unmappable character                                                    */
	{(char *)"handlenewfileinfo", SAMPLE_OPTION_HANDLENEWFILEINFO},			/* Handle EX_CALLBACK_ID_NEWFILEINFO                                       */
	{(char *)"handleendpage", SAMPLE_OPTION_HANDLEENDPAGE},					/* Handle EX_CALLBACK_ID_ENDPAGE                                           */
	{(char *)"ztoq", SAMPLE_OPTION_ZTOQ},									/* Map z's to q's on output                                                */
	{(char *)"reflink", SAMPLE_OPTION_REFLINK},								/* Handle EX_CALLBACK_ID_REFLINK                                           */
	{(char *)"oem", SAMPLE_OPTION_OEMSTRING},								/* Add a string to the OEM string list                                     */
	{(char *)"oemoutput", SAMPLE_OPTION_OEMOUTPUT},							/* Add a string to the OEM output string list                              */
	{(char *)"linkaction", SAMPLE_OPTION_LINKACTION},						/* The action to take in EX_CALLBACK_ID_PROCESSLINK                        */
	{(char *)"linklocation", SAMPLE_OPTION_LINKLOCATION},					/* The URL to output when the link action is "create"                      */
	{(char *)"customelement", SAMPLE_OPTION_CUSTOMELEMENT},					/* Add a string to the custom element list                                 */
	{(char *)"customelementvalue", SAMPLE_OPTION_CUSTOMELEMENTVALUE},		/* List of element value strings that may be associated                    */
																	/* with a string in the custom element list                                */
	{(char *)"processelement", SAMPLE_OPTION_PROCESSELEMENT},				/* Add a string to the process element list                                */
	{(char *)"graphicskipsize", SAMPLE_OPTION_GRAPHICSKIPSIZE},				/* Skip graphics with a width or height >= this value                      */
	{(char *)"wellformed", SAMPLE_OPTION_WELLFORMED},						/* TRUE to make HTML well-formed                                           */
	{(char *)"graphicbuffersize", SAMPLE_OPTION_GRAPHICBUFFERSIZE},			/* Size of the graphics buffer in bytes                                    */
	{(char *)"collapsewhitespace", SAMPLE_OPTION_COLLAPSEWHITESPACE},		/* Set to TRUE to remove excess whitespace                                 */
	{(char *)"javascripttabs", SAMPLE_OPTION_JAVASCRIPTTABS},				/* Max. length of URLs in characters                                       */
	{(char *)"maxurllength", SAMPLE_OPTION_MAXURLLENGTH},					/* TRUE to use JavaScript to handle tabstops                               */
	{(char *)"pagesize", SAMPLE_OPTION_PAGESIZE},							/* Page size in characters                                                 */
	{(char *)"separategraphicsbuffer", SAMPLE_OPTION_SEPARATEGRAPHICSBUFFER}, /* TRUE if graphics buffer is separate from text buffer                    */
	{(char *)"cmcallbackversion", SAMPLE_OPTION_CMCALLBACKVER},				/* Indicates which callback functions to use                               */
	{(char *)"cmcallbackcharset", SAMPLE_OPTION_CMCALLBACKCHARSET},			/* Indicates character set of char-mapping callback strings                */
	{(char *)"tempdir", SAMPLE_OPTION_TEMPDIRECTORY},						/* Location of temp directory                                              */
	{(char *)"altlinkprev", SAMPLE_OPTION_ALTLINK_PREV},					/* Prev altlink string                                                     */
	{(char *)"altlinknext", SAMPLE_OPTION_ALTLINK_NEXT},					/* Next altlink string                                                     */
	{(char *)"cballdisabled", SAMPLE_OPTION_CBALLDISABLED},					/* All callbacks are disabled                                              */
	{(char *)"cbcreatenewfile", SAMPLE_OPTION_CBCREATENEWFILE},				/* This flag enables the CreateNewFile callbacks                           */
	{(char *)"cbnewfileinfo", SAMPLE_OPTION_CBNEWFILEINFO},					/* This flag enables the NewFileInfo callbacks                             */
	{(char *)"cbprocesslink", SAMPLE_OPTION_CBPROCESSLINK},					/* This flag enables the ProcessLink callbacks                             */
	{(char *)"cbcustomelement", SAMPLE_OPTION_CBCUSTOMELEMENT},				/* This flag enables the CustomElementList and                             */
																	/* ProcessElementStr callbacks                                             */
	{(char *)"cbgraphicexportfailure", SAMPLE_OPTION_CBGRAPHICEXPORTFAILURE}, /* This flag enables the GraphicExportFailure callbacks                    */
	{(char *)"cboemoutput", SAMPLE_OPTION_CBOEMOUTPUT},						/* This flag enables the OemOutPut callbacks                               */
	{(char *)"cbaltlink", SAMPLE_OPTION_CBALTLINK},							/* This flag enables the AltLink callback                                  */
	{(char *)"cbarchive", SAMPLE_OPTION_CBARCHIVE},							/* This flag enables the Enter/LeaveArchive callbacks                      */
	{(char *)"cballenabled", SAMPLE_OPTION_CBALLENABLED},					/* This flag enables all callbacks.                                        */

	{(char *)"tempdir_u", SAMPLE_OPTION_TEMPDIRECTORY_U},					/* Location of temp directory in Unicode                                   */
	{(char *)"substitutegraphic", SAMPLE_OPTION_SUBSTITUTEGRAPHIC},			/* URL of image to use in case graphic conversion fails                    */
	{(char *)"substitutegraphic_u", SAMPLE_OPTION_SUBSTITUTEGRAPHIC},		/* Unicode version of the szSubstituteGraphic parameter                    */
	{(char *)"inputpath", SAMPLE_OPTION_INPUTPATH},							/* Input file to convert                                                   */
	{(char *)"inputpath_u", SAMPLE_OPTION_INPUTPATH_U},						/* Input file to convert in Unicode                                        */
	{(char *)"outputpath", SAMPLE_OPTION_OUTPUTPATH},						/* Output file                                                             */
	{(char *)"outputpath_u", SAMPLE_OPTION_OUTPUTPATH_U},					/* Output file in Unicode                                                  */
	{(char *)"hrefprefix", SAMPLE_OPTION_HREFPREFIX},						/* URL prefix for generated links                                          */
	{(char *)"firstprevhref", SAMPLE_OPTION_FIRSTPREVHREF},					/* URL for the first "previous" link                                       */
	{(char *)"gridrows", SAMPLE_OPTION_GRIDROWS},							/* Number of rows in the grid                                              */
	{(char *)"gridcols", SAMPLE_OPTION_GRIDCOLS},							/* Number of columns in the grid                                           */
	{(char *)"gridadvance", SAMPLE_OPTION_GRIDADVANCE},						/* Traversal path of the spreadsheet                                       */
	{(char *)"gridwrap", SAMPLE_OPTION_GRIDWRAP},							/* Set to indicate whether grid ouput should continue when                 */
																	/* the edge of the spreadsheet is reached                                  */
	{(char *)"suppressfontsize", SAMPLE_OPTION_SUPPRESSFONTSIZE},			/* Suppress font size attribute                                            */
	{(char *)"suppressfontcolor", SAMPLE_OPTION_SUPPRESSFONTCOLOR},			/* Suppress font color attribute                                           */
	{(char *)"suppressfontface", SAMPLE_OPTION_SUPPRESSFONTFACE},			/* Suppress font face attribute                                            */

	{(char *)"htmlext", SAMPLE_OPTION_HTML_EXTENSION},						/* Extension for FI_HTML files                                             */
	{(char *)"wmlext", SAMPLE_OPTION_WML_EXTENSION},						/* Extension for FI_WML files                                              */
	{(char *)"hdmlext", SAMPLE_OPTION_HDML_EXTENSION},						/* Extension for FI_HDML files                                             */
	{(char *)"xhtmlbext", SAMPLE_OPTION_XHTMLB_EXTENSION},					/* Extension for FI_XHTMLB files                                           */
	{(char *)"chtmlext", SAMPLE_OPTION_CHTML_EXTENSION},					/* Extension for FI_CHTML files                                            */
	{(char *)"htmlwcaext", SAMPLE_OPTION_HTMLWCA_EXTENSION},				/* Extension for FI_HTMLWCA files                                          */
	{(char *)"htmlagext", SAMPLE_OPTION_HTMLAG_EXTENSION},					/* Extension for FI_HTMLAG files                                           */
	{(char *)"wirelesshtmlext", SAMPLE_OPTION_WIRELESSHTML_EXTENSION},		/* Extension for FI_WIRELESSHTML files                                     */
	{(char *)"textext", SAMPLE_OPTION_TEXT_EXTENSION},						/* Extension for FI_TEXT files                                             */
	{(char *)"htmlcssext", SAMPLE_OPTION_HTML_CSS_EXTENSION},				/* Extension for FI_HTML_CSS files                                         */
	{(char *)"javascriptext", SAMPLE_OPTION_JAVASCRIPT_EXTENSION},			/* Extension for FI_JAVASCRIPT files                                       */
	{(char *)"jpegfifext", SAMPLE_OPTION_JPEGFIF_EXTENSION},				/* Extension for FI_JPEGFIF files                                          */
	{(char *)"gifext", SAMPLE_OPTION_GIF_EXTENSION},						/* Extension for FI_GIF files                                              */
	{(char *)"wbmpext", SAMPLE_OPTION_WBMP_EXTENSION},						/* Extension for FI_WBMP files                                             */
	{(char *)"bmpext", SAMPLE_OPTION_BMP_EXTENSION},						/* Extension for FI_BMP files                                              */
	{(char *)"pngext", SAMPLE_OPTION_PNG_EXTENSION},						/* Extension for FI_PNG files                                              */
	{(char *)"mhtmlext", SAMPLE_OPTION_MHTML_EXTENSION},					/* Extension for FI_MHTML files                                            */

	{(char *)"xmldefmethod", SAMPLE_OPTION_XMLDEFMETHOD},					/* XML Definition method (DTD, XSD, NONE)                                  */
	{(char *)"xmldefreference", SAMPLE_OPTION_XMLDEFREFERENCE},				/* UTF-8 encoded XML Definition reference.                                 */
	{(char *)"pstylenamesflag", SAMPLE_OPTION_PSTYLENAMESFLAG},				/* Include paragraph style name reference to <p> tags.                     */
	{(char *)"embeddingsflag", SAMPLE_OPTION_EMBEDDINGSFLAG},				/* Include embeddings.                                                     */
	{(char *)"noxmldeclarationflag", SAMPLE_OPTION_NOXMLDECLARATIONFLAG},	/* Exclude the XML declaration.                                            */
	{(char *)"offsettracked", SAMPLE_OPTION_OFFSETTRACKED},					/* Include character offset information in output                          */
	{(char *)"bold", SAMPLE_OPTION_CHARBOLD},								/* Include bold character attribute information in output                  */
	{(char *)"italic", SAMPLE_OPTION_CHARITALIC},							/* Include italic character attribute information in output                */
	{(char *)"underline", SAMPLE_OPTION_CHARUNDERLINE},						/* Include underline character attribute information in output             */
	{(char *)"doubleunderline", SAMPLE_OPTION_CHARDUNDERLINE},				/* Include underline character attribute information in output             */
	{(char *)"outline", SAMPLE_OPTION_CHAROUTLINE},							/* Include outline character attribute information in output               */
	{(char *)"strikeout", SAMPLE_OPTION_CHARSTRIKEOUT},						/* Include strikeout character attribute information in output             */
	{(char *)"smallcaps", SAMPLE_OPTION_CHARSMALLCAPS},						/* Include smallcaps character attribute information in output             */
	{(char *)"allcaps", SAMPLE_OPTION_CHARALLCAPS},							/* Include allcaps character attribute information in output               */
	{(char *)"hidden", SAMPLE_OPTION_CHARHIDDEN},							/* Include hidden character attribute information in output                */
	{(char *)"revisiondelete", SAMPLE_OPTION_CHARREVISIONDELETE},			/* Include revision delete character attribute information in output       */
	{(char *)"revisionadd", SAMPLE_OPTION_CHARREVISIONADD},					/* Include revision add character attribute information in output          */
	{(char *)"linespacing", SAMPLE_OPTION_PARASPACING},						/* Include line spacing paragraph attribute information in output          */
	{(char *)"lineheight", SAMPLE_OPTION_PARAHEIGHT},						/* Include line height paragraph attribute information in output           */
	{(char *)"leftindent", SAMPLE_OPTION_PARALEFTINDENT},					/* Include left indent paragraph attribute information in output           */
	{(char *)"rightindent", SAMPLE_OPTION_PARARIGHTINDENT},					/* Include right indent paragraph attribute information in output          */
	{(char *)"firstindent", SAMPLE_OPTION_PARAFIRSTINDENT},					/* Include first paragraph line paragraph attribute information in output  */
	{(char *)"printername", SAMPLE_OPTION_PRINTERNAME},						/* Printer whose device metrics are to be used for creating PageML output  */
	{(char *)"showhiddentext", SAMPLE_OPTION_SHOWHIDDENTEXT},				/* Include hidden text in output                                           */
	{(char *)"filternoblank", SAMPLE_OPTION_FILTERNOBLANK},					/* Include blank spreadsheet pages in the output (or not)                  */
	{(char *)"tiffcolorspace", SAMPLE_OPTION_TIFFCOLORSPACE},				/* Specifies the TIFF color depth                                          */
	{(char *)"tiffcompression", SAMPLE_OPTION_TIFFCOMPRESSION},				/* Specifies the TIFF coompression method to be used                       */
	{(char *)"tiffbyteorder", SAMPLE_OPTION_TIFFBYTEORDER},					/* Specifies the byte order of the TIFF image data                         */
	{(char *)"tiffmultipage", SAMPLE_OPTION_TIFFMULTIPAGE},					/* Output multiple pages to a single multi-page TIFF file                  */
	{(char *)"tiffgray2color", SAMPLE_OPTION_TIFFGRAY2COLOR},				/* Specifies if grayscale images get converted to JPEG in FI_TIFFANDJPEG   */
	{(char *)"dbfittopage", SAMPLE_OPTION_DBPRINTFITTOPAGE},				/* Specifies the type of scaling to be done for database files             */
	{(char *)"dbshowgridlines", SAMPLE_OPTION_DBPRINTGRIDLINES},			/* Specifies if whether to show gridlines for database files or not        */
	{(char *)"dbshowheadings", SAMPLE_OPTION_DBPRINTHEADINGS},				/* Specifies if whether to show headers for database files or not          */
	{(char *)"defaultmargintop", SAMPLE_OPTION_PRINTMARGINSTOP},			/* Top Margin to use if not using the margins specified in the document    */
	{(char *)"defaultmarginbottom", SAMPLE_OPTION_PRINTMARGINSBOTTOM},		/* Bottom Margin to use if not using the margins specified in the document */
	{(char *)"defaultmarginleft", SAMPLE_OPTION_PRINTMARGINSLEFT},			/* Left Margin to use if not using the margins specified in the document   */
	{(char *)"defaultmarginright", SAMPLE_OPTION_PRINTMARGINSRIGHT},		/* Right Margin to use if not using the margins specified in the document  */
	{(char *)"exportendpage", SAMPLE_OPTION_PRINTENDPAGE},					/* Last page in range to be exported                                       */
	{(char *)"exportstartpage", SAMPLE_OPTION_PRINTSTARTPAGE},				/* First page in range to be exported                                      */
	{(char *)"ssdirection", SAMPLE_OPTION_SSPRINTDIRECTION},				/* Direction in which spreadsheets data should be export                   */
	{(char *)"ssfittopage", SAMPLE_OPTION_SSPRINTFITTOPAGE},				/* Specifies the type of scaling to be done for spreadsheet files          */
	{(char *)"ssshowgridlines", SAMPLE_OPTION_SSPRINTGRIDLINES},			/* Specifies if we should show gridlines in spreadsheet files              */
	{(char *)"ssshowheadings", SAMPLE_OPTION_SSPRINTHEADINGS},				/* Specifies if we should show headings in spreadsheet files               */
	{(char *)"ssscalepercent", SAMPLE_OPTION_SSPRINTSCALEPERCENT},			/* Percentage by which to scale spreadsheet pages                          */
	{(char *)"ssscalexhigh", SAMPLE_OPTION_SSPRINTSCALEXHIGH},				/* Number of vertical pages to scale spreadsheet output to                 */
	{(char *)"ssscalexwide", SAMPLE_OPTION_SSPRINTSCALEXWIDE},				/* Number of horizontal pages to scale spreadsheet output to               */
	{(char *)"usedocpagesettings", SAMPLE_OPTION_USEDOCPAGESETTINGS},		/* Specifies if the native document page layout should be used             */
	{(char *)"vecshowbackground", SAMPLE_OPTION_VECPRINTBACKGROUND},		/* Maintain Aspect Ratio of vector formats or Stretch to fill area         */
	{(char *)"vecaspect", SAMPLE_OPTION_VECPRINTASPECT},					/* Display background of vector images                                     */
	{(char *)"whattoexport", SAMPLE_OPTION_WHATTOPRINT},					/* Specifies which pages should be exported                                */
	{(char *)"acceptaltgraphics", SAMPLE_OPTION_ACCEPT_ALT_GRAPHICS},		/* If "yes", leave {GIF, JPG, PNG} as-is.  Convert all else to GRAPHICTYPE.*/
	{(char *)"substreamrootsobsolete", SAMPLE_OPTION_SUBSTREAMROOTS_OBSOLETE}, /* obsolete option                                                         */
	{(char *)"removefontgroupsobsolete", SAMPLE_OPTION_REMOVEFONTGROUPS_OBSOLETE}, /* obsolete option                                                       */
	{(char *)"includetextoffsetsobsolete", SAMPLE_OPTION_INCLUDETEXTOFFSETS_OBSOLETE}, /* obsolete option                                                     */
	{(char *)"defaultprintfontface", SAMPLE_OPTION_DEFAULTPRINTFONTFACE},	/* Default print font face                                                 */
	{(char *)"defaultprintfontheight", SAMPLE_OPTION_DEFAULTPRINTFONTHEIGHT}, /* Default print font height                                               */
	{(char *)"defaultprintfontattribute", SAMPLE_OPTION_DEFAULTPRINTFONTATTR}, /* Default print font attribute                                            */
	{(char *)"defaultprintfonttype", SAMPLE_OPTION_DEFAULTPRINTFONTTYPE},	/* Default print font type                                                 */
	{(char *)"printfontaliasid", SAMPLE_OPTION_PRINTFONTALIASID},			/* Print font alias ID                                                     */
	{(char *)"printfontaliasflag", SAMPLE_OPTION_PRINTFONTALIASFLAGS},		/* Print font alias flag                                                   */
	{(char *)"printfontaliasoriginal", SAMPLE_OPTION_PRINTFONTALIASORIGINAL}, /* Print font alias original                                               */
	{(char *)"printfontalias", SAMPLE_OPTION_PRINTFONTALIAS},				/* Print font alias                                                        */
	{(char *)"tifffillorder", SAMPLE_OPTION_TIFFMSB},						/* Set the fill order                                                      */
	{(char *)"graphicwidth", SAMPLE_OPTION_GRAPHICWIDTH},					/* Set the output graphic width in pixels                                  */
	{(char *)"graphicheight", SAMPLE_OPTION_GRAPHICHEIGHT},					/* Set the output graphic height in pixels                                 */
	{(char *)"jpegcompression", SAMPLE_OPTION_JPEGCOMPRESSION},
	{(char *)"lzwcompression", SAMPLE_OPTION_LZWCOMPRESSION},
	{(char *)"originalcharset", SAMPLE_OPTION_CHAROCE},						/* Include original character encoding character attribute information in output */
	{(char *)"extractembeddedformat", SAMPLE_OPTION_EXTRACTEMBEDDEDFORMAT},
	{(char *)"exepath", SAMPLE_OPTION_EXEPATH},
	{(char *)"showhiddenssdata", SAMPLE_OPTION_SHOWHIDDENSSDATA},			/* hidden spread sheet columns/rows/sheets                                       */
	{(char *)"filternoblank", SAMPLE_OPTION_FILTERNOBLANK},					/* produce blank spreadsheet pages in the export (or not)                        */
	{(char *)"showchangetracking", SAMPLE_OPTION_SHOWCHANGETRACKING},		/* change tracking of the word processing document                               */
	{(char *)"showssdbrowcolheadings", SAMPLE_OPTION_SHOWSSDBROWCOLHEADINGS}, /* show row and column headings                                                  */
	{(char *)"suppressproperties", SAMPLE_OPTION_SUPPRESSPROPERTIES},		/* Suppress Production of document properties.                                   */
	{(char *)"processgeneratedtext", SAMPLE_OPTION_PROCESSGENERATEDTEXT},	/* Produce text generated from numbers.                                          */
	{(char *)"flattenstyles", SAMPLE_OPTION_FLATTENSTYLES},					/* Flatten styles to eliminate 'based-on' processing.                            */
	{(char *)"bitmapasbitmap", SAMPLE_OPTION_BITMAPASBITMAP},				/* Export bitmap embeddings as rasters                                           */
	{(char *)"chartasbitmap", SAMPLE_OPTION_CHARTASBITMAP},					/* Export chart embeddings as rasters                                            */
	{(char *)"presentationasbitmap", SAMPLE_OPTION_PRESENTATIONASBITMAP},	/* Export presentation embeddings as rasters                                     */
	{(char *)"vectorasbitmap", SAMPLE_OPTION_VECTORASBITMAP},				/* Export vector embeddings as rasters                                           */
	{(char *)"nobitmapelements", SAMPLE_OPTION_NOBITMAPELEMENTS},			/* do not export bitmap embeddings                                               */
	{(char *)"nochartelements", SAMPLE_OPTION_NOCHARTELEMENTS},				/* do not export chart embeddings                                                */
	{(char *)"nopresentationelements", SAMPLE_OPTION_NOPRESENTATIONELEMENTS}, /* do not export presentation embeddings                                         */
	{(char *)"novectorelements", SAMPLE_OPTION_NOVECTORELEMENTS},			/* do not export vector embeddings                                               */
	{(char *)"isodates", SAMPLE_OPTION_ISODATES},							/* Format dates and date/times with ISO 8601 standard                            */
	{(char *)"removefontgroups", SAMPLE_OPTION_REMOVEFONTGROUPS},			/* Replace font groups with references to individual fonts                       */
	{(char *)"includetextoffsets", SAMPLE_OPTION_INCLUDETEXTOFFSETS},		/* Include text_offset attribute on tx.p and tx.r elements                       */
	{(char *)"suppressattachments", SAMPLE_OPTION_SUPPRESSATTACHMENTS},		/* Indexing - Suppress the production of attachments                             */
	{(char *)"suppressarchivesubdocs", SAMPLE_OPTION_SUPPRESSARCHIVESUBDOCS}, /* Indexing - Suppress the production of subdocs in archives                     */
	{(char *)"usefullfilepaths", SAMPLE_OPTION_USEFULLFILEPATHS},			/* XML Export - Use full absolute file paths in locators                         */
	{(char *)"textout", SAMPLE_OPTION_TEXTOUT},								/* PageML - produce text for each text range element                             */
	{(char *)"maxssdbpagewidth", SAMPLE_OPTION_MAXSSDBPAGEWIDTH},
	{(char *)"maxssdbpageheight", SAMPLE_OPTION_MAXSSDBPAGEHEIGHT},
	{(char *)"red", SAMPLE_OPTION_TRANSPARENCYCOLORRED},					/* Red portion of the color the user wants to make transparent                   */
	{(char *)"green", SAMPLE_OPTION_TRANSPARENCYCOLORGREEN},				/* Green portion of the color the user wants to make transparent                 */
	{(char *)"blue", SAMPLE_OPTION_TRANSPARENCYCOLORBLUE},					/* Blue portion of the color the user wants to make transparent                  */
	{(char *)"imagecropping", SAMPLE_GRAPHIC_CROPPING},						/* Tells IX if the image should be cropped to the content of the page            */
	{(char *)"delimiters", SAMPLE_OPTION_DELIMITERS},						/* XML Export - Produce delimiters when available                                */
	{(char *)"xmloutput", SAMPLE_OPTION_CA_XMLOUTPUT},						/* CATest.  Produce UTF8 encoded XML output.                                     */
	{(char *)"nullreplacementchar", SAMPLE_OPTION_NULLREPLACEMENTCHAR},
	{(char *)"metadataonly", SAMPLE_OPTION_METADATAONLY},
	{(char *)"annotations", SAMPLE_OPTION_ANNOTATIONS},
	{(char *)"preferoitrendering", SAMPLE_RENDERING_PREFER_OIT},
	{(char *)"separatestyletables", SAMPLE_OPTION_SEPARATESTYLETABLES},		/* XML Export - Place style_tables subtree in a separate output unit.            */
	{(char *)"enablewatermark", SAMPLE_OPTION_WATERMARK},
	{(char *)"watermarkposition", SAMPLE_OPTION_WATERMARKPOS},
	{(char *)"verticalpos", SAMPLE_OPTION_WATERMARKVERTPOS},
	{(char *)"horizontalpos", SAMPLE_OPTION_WATERMARKHORZPOS},
	{(char *)"defaultheight", SAMPLE_OPTION_DEFAULTHEIGHT},
	{(char *)"defaultwidth", SAMPLE_OPTION_DEFAULTWIDTH},
	{(char *)"units", SAMPLE_OPTION_UNITS},
	{(char *)"watermarkpath", SAMPLE_OPTION_WATERMARKPATH},
	{(char *)"fontdirectory", SAMPLE_OPTION_FONTDIRECTORY},
	{(char *)"compress", SAMPLE_OPTION_COMPRESS},
	{(char *)"watermarkiotype", SAMPLE_OPTION_WATERMARKIOTYPE},
	{(char *)"watermarkscale", SAMPLE_OPTION_WATERMARKIOSCALE},
	{(char *)"watermarkscalepercent", SAMPLE_OPTION_WATERMARKIOPERCENT},
	{(char *)"filtertype", SAMPLE_OPTION_FONTFILTERTYPE},
	{(char *)"fontname", SAMPLE_OPTION_FONTFILTERLIST},
	{(char *)"embedfonts", SAMPLE_OPTION_EMBEDFONTS},
	{(char *)"fontembedpolicy", SAMPLE_OPTION_FONTEMBEDPOLICY},
	{(char *)"dolinearization", SAMPLE_OPTION_DOLINEARIZATION},
	{(char *)"exportattachments", SAMPLE_OPTION_EXPORTATTACHMENTS},
	{(char *)"imagepassthrough", SAMPLE_OPTION_IMAGE_PASSTHROUGH},
	{(char *)"unrestrictedfontembedding", SAMPLE_OPTION_UNRESTRICTEDFONTEMBEDDING},
	{(char *)"produceurls", SAMPLE_OPTION_PRODUCEURLS},
	{(char *)"produceobjectinfo", SAMPLE_OPTION_PRODUCEOBJECTINFO},			/* Produce information allowing for reference of sub-document objects.           */
	{(char *)"enableerrorinfo", SAMPLE_OPTION_ENABLEERRORINFO},				/* Output sub-document error information.                                        */
	{(char *)"producecellinfo", SAMPLE_OPTION_CELLINFO},					/* Output spreadsheet row column information.                                    */
	{(char *)"produceformulas", SAMPLE_OPTION_FORMULAS},					/* Output spreadsheet formula information.                                       */
	{(char *)"produceunmappedtext", SAMPLE_OPTION_UNMAPPEDTEXT},			/* Control production of unmapped text in SearchML.                              */
	{(char *)"allobjects", SAMPLE_OPTION_OBJECTALL},						/* Extract all objects in a file                                                 */
	{(char *)"ssdbborderoptions", SAMPLE_OPTION_SSDBBORDEROPTIONS},
	{(char *)"odgraphicoptions", SAMPLE_OPTION_ODGRAPHICOPTIONS},
	{(char *)"timezone", SAMPLE_OPTION_TIMEZONE},
	{(char *)"defaultinputcharset", SAMPLE_OPTION_DEFAULTINPUTCHARSET},
	{(char *)"readbuffersize", SAMPLE_OPTION_READBUFFERSIZE},				/* size of the I/O Read buffer in KB                                             */
	{(char *)"mapbuffersize", SAMPLE_OPTION_MAPBUFFERSIZE},					/* maximum size for the I/O Memory Map buffer in KB                              */
	{(char *)"tempbuffersize", SAMPLE_OPTION_TEMPBUFFERSIZE},				/* maximum size for the memory-mapped temp files in KB                           */
	{(char *)"numberofstatcallbacks", SAMPLE_OPTION_NUMBEROFSTATCALLBACKS}, /* number of callbacks from the stats callback                                   */
	{(char *)"imagewatermarkopacity", SAMPLE_GRAPHIC_WATERMARK_OPACITY},	/* Tells IX the opacity the watermark image should have.                         */
	{(char *)"imagewatermarkscaletype", SAMPLE_GRAPHIC_WATERMARK_SCALETYPE}, /* Tells IX the type of scaling for the watermark image.                         */
	{(char *)"imagewatermarkscalepercent", SAMPLE_GRAPHIC_WATERMARK_SCALEPERCENT}, /* Tells IX the watermark image's percent of scale.                              */
	{(char *)"imagewatermarkhorizontalpos", SAMPLE_GRAPHIC_WATERMARK_HORIZONTALPOS}, /* Tells IX the watermark horizontal offset from position.                       */
	{(char *)"imagewatermarkverticalpos", SAMPLE_GRAPHIC_WATERMARK_VERTICALPOS}, /* Tells IX the watermark Vertical offset from position.                         */
	{(char *)"imagewatermarkpath", SAMPLE_GRAPHIC_WATERMARK_PATH},			/* Tells IX the watermark's path .                                               */
	{(char *)"imagewatermarkhorizontaloffset", SAMPLE_GRAPHIC_WATERMARK_HORIZONTAL_OFFSET}, /* Tells what percent of page size to offset watermark */
	{(char *)"imagewatermarkverticaloffset", SAMPLE_GRAPHIC_WATERMARK_VERTICAL_OFFSET}, /* Tells what percent of page size to offset watermark */
	{(char *)"imagewatermarkhorizontaloffsetfrom", SAMPLE_GRAPHIC_WATERMARK_HORIZONTAL_OFFSETFROM }, /* Tells which edge to offset the watermark from */
	{(char *)"imagewatermarkverticaloffsetfrom", SAMPLE_GRAPHIC_WATERMARK_VERTICAL_OFFSETFROM}, /* Tells which edge to offset the watermark from */
	{(char *)"imagewatermarkpathtype", SAMPLE_GRAPHIC_WATERMARK_IOTYPE }, /* Tell what type of path the watermakr path will be. */
	{(char *)"renderaspage", SAMPLE_GRAPHIC_RENDERASPAGE },					/* Tells IX that page settings will be used for bitmaps. */
	{(char *)"optimizesections", SAMPLE_OPTION_OPTIMIZESECTIONS},
	{(char *)"charmapping", SAMPLE_OPTION_CHARMAPPING},
	{(char *)"extractxmpmetadata", SAMPLE_OPTION_EXTRACTXMPMETADATA},
	{(char *)"parsexmpmetadata", SAMPLE_OPTION_PARSEXMPMETADATA},
	{(char *)"ocrtech", SAMPLE_OPTION_OCRTECH},
	{(char *)"ocrquality", SAMPLE_OPTION_OCRQUALITY},
	{(char *)"redirecttempfile", SAMPLE_OPTION_REDIRECTTEMPFILE},
	{(char *)"lotusnotesdirectory", SAMPLE_OPTION_LOTUSNOTESDIRECTORY},		/* for vsnsf filter, contains location of 3rd party libraries */
	{(char *)"linearunits", SAMPLE_OPTION_LINEARUNITS},
	{(char *)"emailheader", SAMPLE_OPTION_EMAILHEADER},
	{(char *)"mimeheader", SAMPLE_OPTION_MIMEHEADER},
	{(char *)"isodateformatting", SAMPLE_OPTION_ISODATETIMES},				/* Format dates and times according to ISO 8601.                                 */
	{(char *)"documentmemorymode", SAMPLE_OPTION_DOCUMENTMEMORYMODE},		/* Total amount of memory available to the chunker.                              */
	{(char *)"suppressmetadata", SAMPLE_OPTION_SUPPRESSMETADATA},			/* PHTML */
	{(char *)"quickthumbnail", SAMPLE_OPTION_QUICKTHUMBNAIL},				/* This is a windows only option. When set to false we render the file at its original size and we scale the image using our smooth scaling algorithm */
	{(char *)"onestriptiff", SAMPLE_OPTION_TIFFONESTRIP},					/* When this flag is set the number of rows per strip is equal to the image height */
	{(char *)"notesid", SAMPLE_OPTION_NOTESID},
	{(char *)"password", SAMPLE_OPTION_PASSWORD},
	{(char *)"password_u", SAMPLE_OPTION_PASSWORD_U},
	{(char *)"reordermethod", SAMPLE_OPTION_REORDERMETHOD},
	{(char *)"showhiddensscells", SAMPLE_OPTION_SSSHOWHIDDENCELLS},
	{(char *)"generatesystemmetadata", SAMPLE_OPTION_GENERATESYSTEMMETADATA},
	{(char *)"unixoptionsfile", SAMPLE_OPTION_UNIXOPTIONSFILE},
	{(char *)"processarchivesubdocs", SAMPLE_OPTION_PROCESSARCHIVESUBDOCS},
	{(char *)"processembeddingsubdocs", SAMPLE_OPTION_PROCESSEMBEDDINGSUBDOCS},
	{(char *)"processattachmentsubdocs", SAMPLE_OPTION_PROCESSATTACHMENTSUBDOCS},
	{(char *)"pdffilterreorderbidi", SAMPLE_OPTION_PDF_FILTER_REORDER_BIDI},
	{(char *)"strictfileaccess", SAMPLE_OPTION_STRICTFILEACCESS},
	{(char *)"oleembeddings", SAMPLE_OPTION_OLEEMBEDDINGS},
	{(char *)"htmlcondcommentmode", SAMPLE_OPTION_HTML_COND_COMMENT_MODE},
	{(char *)"skipstyles", SAMPLE_OPTION_SKIPSTYLES},
	{(char *)"optionsload", SAMPLE_OPTION_LOAD_OPTIONS},
	{(char *)"optionssave", SAMPLE_OPTION_SAVE_OPTIONS},
	{(char *)"compress24bittype", SAMPLE_OPTION_COMPRESS_24BIT_TYPE},
	{(char *)"exportstatustype", SAMPLE_OPTION_EXPORT_STATUS_TYPE},
	{(char *)"mailheadervisible", SAMPLE_OPTION_MAILHEADERVISIBLE},
	{(char *)"mailheaderhidden", SAMPLE_OPTION_MAILHEADERHIDDEN},
	{(char *)"mailheaderflags", SAMPLE_OPTION_MAILHEADERFLAGS},
	{(char *)"mailheadername", SAMPLE_OPTION_MAILHEADERNAME},
	{(char *)"mailheaderlabel", SAMPLE_OPTION_MAILHEADERLABEL},
	{(char *)"ignorepassword", SAMPLE_OPTION_IGNOREPASSWORD},
	{(char *)"stroketext", SAMPLE_OPTION_STROKETEXT},
	{(char *)"generateexcelrevisions", SAMPLE_OPTION_GENERATE_EXCEL_REVISIONS},
	{(char *)"pdffilterdrophyphens", SAMPLE_OPTION_PDF_FILTER_DROPHYPHENS},
	{(char *)"performancemode", SAMPLE_OPTION_PERFORMANCEMODE},
	{(char *)"producecellhidden", SAMPLE_OPTION_CELLHIDDEN},
	{(char *)"outputstructure", SAMPLE_OPTION_OUTPUT_STRUCTURE},
	{(char *)"rawtext", SAMPLE_OPTION_RAWTEXT},
	{(char *)"resourceurl", SAMPLE_OPTION_URLPATH_RESOURCES},
	{(char *)"outputurl", SAMPLE_OPTION_URLPATH_OUTPUT},
	{(char *)"postlibscript", SAMPLE_OPTION_POST_LIBRARY_SCRIPT},
	{(char *)"prelibscript", SAMPLE_OPTION_PRE_LIBRARY_SCRIPT},
	{(char *)"fontpermissions", SAMPLE_OPTION_FONTPERMISSIONS},
	{(char *)"fontbaseurl", SAMPLE_OPTION_FONTBASEURL},
	{(char *)"fontreferencemethod", SAMPLE_OPTION_FONTREFERENCEMETHOD},
	{(char *)"attachmenthandling", SAMPLE_OPTION_ATTACHMENTHANDLING},
	{(char *)"externalstylesheet", SAMPLE_OPTION_EXTERNAL_CSS},
	{(char *)"filternoblank", SAMPLE_OPTION_FILTERNOBLANK},
	{(char *)"renderembeddedfonts", SAMPLE_OPTION_RENDER_EMBEDDED_FONTS},
	{(char *)"disablealphablending", SAMPLE_RENDER_DISABLEALPHABLENDING},
	{(char *)"enablealphablending", SAMPLE_RENDER_ENABLEALPHABLENDING},
	{(char *)"imagestampfile", SAMPLE_OPTION_IMAGESTAMP_FILE},
	{(char *)"imagestampurl", SAMPLE_OPTION_IMAGESTAMP_URL},
	{(char *)"imagestampspec", SAMPLE_OPTION_IMAGESTAMP_SPECTYPE},
	{(char *)"imagestampname", SAMPLE_OPTION_IMAGESTAMP_NAME},
	{(char *)"wvlibraryname", SAMPLE_OPTION_WVLIBRARYNAME},
	{(char *)"wvstylesheetname", SAMPLE_OPTION_WVSTYLESHEETNAME},
	{(char *)"redactioncolor", SAMPLE_OPTION_REDACTIONCOLOR},
	{(char *)"showredactionlabels", SAMPLE_OPTION_SHOWREDACTIONLABELS},
	{(char *)"redactionlabelfontname", SAMPLE_OPTION_REDACTION_LABEL_FONT_NAME},
	{(char *)"enableredactions", SAMPLE_OPTION_REDACTIONS_ENABLED},
	{(char *)"vectorobjectlimit", SAMPLE_OPTION_VECTOROBJECTLIMIT},
	{(char *)"pdffilterworddelimfraction", SAMPLE_OPTION_PDF_FILTER_WORD_DELIM_FRACTION},
	{(char *)"pdffiltermaxembeddings", SAMPLE_OPTION_PDF_FILTER_MAX_EMBEDDED_OBJECTS},
	{(char *)"pdffiltermaxvectorpaths", SAMPLE_OPTION_PDF_FILTER_MAX_VECTOR_PATHS},
	{(char *)"emailfixedwidth", SAMPLE_OPTION_EMAIL_FIXEDWIDTH},
	{(char *)"htmlfixedwidth", SAMPLE_OPTION_HTML_FIXEDWIDTH},
	{(char *)"plaintextpagination", SAMPLE_OPTION_PLAINTEXT_PAGINATION},
	{(char *)"pagesperfile", SAMPLE_OPTION_PAGES_PER_FILE},
	{(char *)"recipientdeliveryinformation", SAMPLE_OPTION_RECIPIENT_DELIVERY_INFORMATION },
	{(char *)"backgroundcolor", SAMPLE_OPTION_BACKGROUND_COLOR },
	{(char *)"pdfnewspacing", SAMPLE_OPTION_PDF_FILTER_NEW_SPACING_ALGORITHM }, /* tvarshne Pdf Space Improvement Plan-1 11/28/2020 */
	{(char *)NULL, STRINGDW_LISTEND}, /* End of list marker                                                            */
};

StringDW FilterType[] =
{
	{(char *)"exclude", TRUE},
	{(char *)"include", FALSE},
	{NULL, STRINGDW_LISTEND}
};

StringDW WatermarkIOType[] =
{
	{(char *)"ansi", IOTYPE_ANSIPATH},
	{(char *)"unix", IOTYPE_UNIXPATH},
	{(char *)"redirect", IOTYPE_REDIRECT},
	{NULL, STRINGDW_LISTEND}
};

StringDW WatermarkScale[] =
{
	{(char *)"noscaling", SCCGRAPHIC_NOMAP},
	{(char *)"scaletopercent", SCCGRAPHIC_SCALE},
	{NULL, STRINGDW_LISTEND}
};


StringDW JpegCompression[] =
{
	{(char *)"enabled", SCCVW_FILTER_JPG_ENABLED},
	{(char *)"disabled", SCCVW_FILTER_JPG_DISABLED},
	{NULL, STRINGDW_LISTEND}
};

StringDW ImageCrop[] =
{
	{(char *)"nocropping", (VTDWORD)SCCGRAPHIC_NOCROPPING},
	{(char *)"croptocontent", SCCGRAPHIC_CROPTOCONTENT},
	{NULL, STRINGDW_LISTEND}
};

StringDW WatermarkPos[] =
{
	{(char *)"offsetfromcenter", SCCGRAPHIC_OFFSETFROMCENTER},
	{NULL, STRINGDW_LISTEND}
};

StringDW ImageWatermarkScaleType[] =
{
	{(char *)"noscaling", SCCGRAPHIC_WATERMARK_SCALETYPE_NONE},
	{(char *)"scaletopercent", SCCGRAPHIC_WATERMARK_SCALETYPE_PERCENT},
	{NULL, STRINGDW_LISTEND}
};

StringDW Compress24BitType[] =
{
	{(char *)"jpeg", SCCCMP24BIT_JPEG},
	{(char *)"jpeg2000", SCCCMP24BIT_JPEG2000},
	{NULL, STRINGDW_LISTEND}
};

StringDW FontEmbedPolicyType[] =
{
	{(char *)"reducesize", SCCFONTS_REDUCESIZE},
	{(char *)"embedall", SCCFONTS_EMBEDALL},
	{NULL, STRINGDW_LISTEND}
};

StringDW Units[] =
{
	{(char *)"inches", SCCGRAPHIC_INCHES},
	{(char *)"points", SCCGRAPHIC_POINTS},
	{(char *)"centimeters", SCCGRAPHIC_CENTIMETERS},
	{(char *)"picas", SCCGRAPHIC_PICAS},
	{NULL, STRINGDW_LISTEND}
};

StringDW ImageWatermarkHorizontalOffsetFrom[] = 
{
	{(char *)"center", SCCOPT_GRAPHIC_WATERMARK_HORIZONTAL_OFFSETFROM_CENTER},
	{(char *)"left", SCCOPT_GRAPHIC_WATERMARK_HORIZONTAL_OFFSETFROM_LEFT},
	{(char *)"right", SCCOPT_GRAPHIC_WATERMARK_HORIZONTAL_OFFSETFROM_RIGHT},
	{NULL, STRINGDW_LISTEND}
};

StringDW ImageWatermarkVerticalOffsetFrom[] =
{
	{ (char *)"center", SCCOPT_GRAPHIC_WATERMARK_VERTICAL_OFFSETFROM_CENTER },
	{ (char *)"top", SCCOPT_GRAPHIC_WATERMARK_VERTICAL_OFFSETFROM_TOP },
	{ (char *)"bottom", SCCOPT_GRAPHIC_WATERMARK_VERTICAL_OFFSETFROM_BOTTOM },
	{ NULL, STRINGDW_LISTEND }
};

StringDW LzwCompression[] =
{
	{(char *)"enabled", SCCVW_FILTER_LZW_ENABLED},
	{(char *)"disabled", SCCVW_FILTER_LZW_DISABLED},
	{NULL, STRINGDW_LISTEND}
};

StringDW PrintDirection[] =
{
	{(char *)"acrossanddown", SCCVW_SSPRINTDIRECTION_ACROSS},
	{(char *)"downandacross", SCCVW_SSPRINTDIRECTION_DOWN},
	{NULL, STRINGDW_LISTEND}
};

StringDW VectorPrintAspect[] =
{
	{(char *)"original", SCCVW_PRINTASPECT_ORIGINAL},
	{(char *)"stretch", SCCVW_PRINTASPECT_STRETCH},
	{NULL, STRINGDW_LISTEND}
};

StringDW WhatToPrint[] =
{
	{(char *)"all", SCCVW_PRINT_ALLPAGES},
	{(char *)"range", SCCVW_PRINT_PAGERANGE},
	{NULL, STRINGDW_LISTEND}
};

StringDW DBFitToPage[] =
{
	{(char *)"noscaling", SCCVW_DBPRINTFITMODE_NOMAP},
	{(char *)"fittopage", SCCVW_DBPRINTFITMODE_FITTOPAGE},
	{(char *)"fittowidth", SCCVW_DBPRINTFITMODE_FITTOWIDTH},
	{(char *)"fittoheight", SCCVW_DBPRINTFITMODE_FITTOHEIGHT},
	{NULL, STRINGDW_LISTEND}
};

StringDW SSFitToPage[] =
{
	{(char *)"noscaling", SCCVW_DBPRINTFITMODE_NOMAP},
	{(char *)"fittopage", SCCVW_DBPRINTFITMODE_FITTOPAGE},
	{(char *)"fittowidth", SCCVW_DBPRINTFITMODE_FITTOWIDTH},
	{(char *)"fittoheight", SCCVW_DBPRINTFITMODE_FITTOHEIGHT},
	{(char *)"scaletopercent", SCCVW_SSPRINTFITMODE_SCALE},
	{(char *)"scaletopagesspecified", SCCVW_SSPRINTFITMODE_FITTOPAGES},
	{NULL, STRINGDW_LISTEND}
};

StringDW TiffColorSpaceOpts[] =
{
	{(char *)"blackandwhite", SCCGRAPHIC_TIFF1BITBW},
	{(char *)"8bitpalette", SCCGRAPHIC_TIFF8BITPAL},
	{(char *)"24bitrgb", SCCGRAPHIC_TIFF24BITRGB},
	{(char *)"8bitgrayscale", SCCGRAPHIC_TIFF8BITGRAYSCALE},
	{NULL, STRINGDW_LISTEND}
};

StringDW TiffCompression[] =
{
	{(char *)"none", SCCGRAPHIC_TIFFCOMPNONE},
	{(char *)"packbits", SCCGRAPHIC_TIFFCOMPPB},
	{(char *)"lzwcompression", SCCGRAPHIC_TIFFCOMPLZW},
	{(char *)"ccitt-1d", SCCGRAPHIC_TIFFCOMP1D},
	{(char *)"ccitt-group3fax", SCCGRAPHIC_TIFFCOMPGRP3},
	{(char *)"ccitt-group4fax", SCCGRAPHIC_TIFFCOMPGRP4},
	{NULL, STRINGDW_LISTEND}
};

StringDW TiffFillOrder[] =
{
	{(char *)"fillorder1", SCCGRAPHIC_TIFF_FILLORDER1},
	{(char *)"fillorder2", SCCGRAPHIC_TIFF_FILLORDER2},
	{NULL, STRINGDW_LISTEND}
};

StringDW TiffEndian[] =
{
	{(char *)"bigendian", SCCGRAPHIC_TIFFBOBIGE},
	{(char *)"littleendian", SCCGRAPHIC_TIFFBOLITTLEE},
	{NULL, STRINGDW_LISTEND}
};

StringDW OutputIDs[] =
{
	{(char *)"fi_html", FI_HTML},
	{(char *)"fi_wml", FI_WML},
	{(char *)"fi_hdml", FI_HDML},
	{(char *)"fi_xhtmlb", FI_XHTMLB},
	{(char *)"fi_chtml", FI_CHTML},
	{(char *)"fi_htmlwca", FI_HTMLWCA},
	{(char *)"fi_htmlag", FI_HTMLAG},
	{(char *)"fi_text", FI_TEXT},
	{(char *)"fi_catest", FI_CATEST},
	{(char *)"fi_searchml30", FI_SEARCHML30},
	{(char *)"fi_searchml31", FI_SEARCHML31},
	{(char *)"fi_searchml32", FI_SEARCHML32},
	{(char *)"fi_searchml33", FI_SEARCHML33},
	{(char *)"fi_searchml34", FI_SEARCHML34},
	{(char *)"fi_searchml35", FI_SEARCHML35},
	{(char *)"fi_searchml36", FI_SEARCHML36},
	{(char *)"fi_searchml_latest", FI_SEARCHML_LATEST},
	{(char *)"fi_searchhtml", FI_SEARCHHTML},
	{(char *)"fi_phtml", FI_PHTML},
	{(char *)"fi_searchtext", FI_SEARCHTEXT},
	{(char *)"fi_wirelesshtml", FI_WIRELESSHTML},
	{(char *)"fi_pageml", FI_PAGEML},
	{(char *)"fi_xml_flexiondoc4", FI_XML_FLEXIONDOC4},
	{(char *)"fi_xml_flexiondoc5", FI_XML_FLEXIONDOC5},
	{(char *)"fi_xml_flexiondoc5_2", FI_XML_FLEXIONDOC5_2},
	{(char *)"fi_xml_flexiondoc5_3", FI_XML_FLEXIONDOC5_3},
	{(char *)"fi_xml_flexiondoc5_4", FI_XML_FLEXIONDOC5_4},
	{(char *)"fi_xml_flexiondoc5_5", FI_XML_FLEXIONDOC5_5},
	{(char *)"fi_xml_flexiondoc5_6", FI_XML_FLEXIONDOC5_6},
	{(char *)"fi_xml_flexiondoc5_7", FI_XML_FLEXIONDOC5_7},
	{(char *)"fi_xml_flexiondoc_latest", FI_XML_FLEXIONDOC_LATEST},
	{(char *)"fi_xml_opendocument_1_0", FI_XML_OPENDOCUMENT_1_0},
	{(char *)"fi_gif", FI_GIF},
	{(char *)"fi_jpeg", FI_JPEGFIF},
	{(char *)"fi_jpegfif", FI_JPEGFIF},
	{(char *)"fi_wbmp", FI_WBMP},
	{(char *)"fi_bmp", FI_BMP},
	{(char *)"fi_png", FI_PNG},
	{(char *)"fi_tiff", FI_TIFF},
	{(char *)"fi_tiffandjpg", FI_TIFFANDJPG},
	{(char *)"fi_mhtml", FI_MHTML},
	{(char *)"fi_pdf", FI_PDF},
	{(char *)"fi_pdfa", FI_PDFA},
	{(char *)"fi_pdfa_2", FI_PDFA_2},
	{(char *)"fi_ocrtext", FI_OCRTEXT},
	{(char *)"fi_jpeg2000", FI_JPEG2000},
	{(char *)"fi_edrm1", FI_EDRM1},
	{(char *)"fi_xml_bitform", FI_XML_BITFORM},
	{(char *)"fi_html5", FI_HTML5},
	{(char *)"fi_intf", FI_INTF },
	{NULL, STRINGDW_LISTEND}
};


StringDW Flavors[] =
{
	{(char *)"netscape30", SCCEX_FLAVOR_NS30},
	{(char *)"netscape40", SCCEX_FLAVOR_NS40},
	{(char *)"msie30", SCCEX_FLAVOR_MS30},
	{(char *)"msie40", SCCEX_FLAVOR_MS40},
	{(char *)"html20", SCCEX_FLAVOR_HTML20},
	{(char *)"html30", SCCEX_FLAVOR_HTML30},
	{(char *)"html40", SCCEX_FLAVOR_HTML40},
	{(char *)"generic", SCCEX_FLAVOR_GENERICHTML},
	{(char *)"wml11", SCCEX_FLAVOR_WML11},
	{(char *)"wml11tbl", SCCEX_FLAVOR_WML11_TBL},
	{(char *)"wml20", SCCEX_FLAVOR_WML20},
	{(char *)"hdml", SCCEX_FLAVOR_HDML},
	{(char *)"xhtmlb1", SCCEX_FLAVOR_XHTMLB1},
	{(char *)"xhtmlb1notbl", SCCEX_FLAVOR_XHTMLB1_NOTBL},
	{(char *)"chtml2", SCCEX_FLAVOR_CHTML2},
	{(char *)"wca11", SCCEX_FLAVOR_WCA11},
	{(char *)"wca11notbl", SCCEX_FLAVOR_WCA11_NOTBL},
	{(char *)"ag33palm", SCCEX_FLAVOR_AG33PALM},
	{(char *)"ag33palmnotbl", SCCEX_FLAVOR_AG33PALM_NOTBL},
	{(char *)"ag33ce", SCCEX_FLAVOR_AG33CE},
	{(char *)"ag33cenotbl", SCCEX_FLAVOR_AG33CE_NOTBL},
	{(char *)"wgenhtml", SCCEX_FLAVOR_WGENERICHTML},
	{(char *)"text", SCCEX_FLAVOR_TEXT},
	{NULL, STRINGDW_LISTEND}
};

StringDW GraphicTypes[] =
{
	{(char *)"gif", FI_GIF},
	{(char *)"jpeg", FI_JPEGFIF},
	{(char *)"wbmp", FI_WBMP},
	{(char *)"bmp", FI_BMP},
	{(char *)"png", FI_PNG},
	{(char *)"tiff", FI_TIFF},
	{(char *)"pdf", FI_PDF},
	{(char *)"pdfa", FI_PDF},
	{(char *)"jpeg2000", FI_JPEG2000},
	{(char *)"any", FI_ANY},
	{(char *)"none", FI_NONE},
	{NULL, STRINGDW_LISTEND}
};


StringDW YesNo[] =
{
	{(char *)"true", TRUE},
	{(char *)"false", FALSE},
	{(char *)"yes", TRUE},
	{(char *)"no", FALSE},
	{NULL, STRINGDW_LISTEND}
};


StringDW SizingOpts[] =
{
	{(char *)"quick", SCCGRAPHIC_QUICKSIZING},
	{(char *)"smooth", SCCGRAPHIC_SMOOTHSIZING},
	{(char *)"grayscale", SCCGRAPHIC_SMOOTHGRAYSCALESIZING},
	{NULL, STRINGDW_LISTEND}
};


StringDW Charsets[] =
{
	{(char *)"iso8859-1", CS_ISO8859_1},    /* Latin-1              */
	{(char *)"iso8859-2", CS_ISO8859_2},    /* Latin-2              */
	{(char *)"iso8859-3", CS_ISO8859_3},    /* Latin-3              */
	{(char *)"iso8859-4", CS_ISO8859_4},    /* Latin-4              */
	{(char *)"iso8859-5", CS_ISO8859_5},    /* Cyrillic             */
	{(char *)"iso8859-6", CS_ISO8859_6},    /* Arabic               */
	{(char *)"iso8859-7", CS_ISO8859_7},    /* Greek                */
	{(char *)"iso8859-8", CS_ISO8859_8},    /* Hebrew               */
	{(char *)"iso8859-9", CS_ISO8859_9},    /* Turkish              */
	{(char *)"macroman", CS_MAC_ROMAN},    /* Mac Roman            */
	{(char *)"macce", CS_MAC_CE},    /* Mac CE               */
	{(char *)"macgreek", CS_MAC_GREEK},    /* Mac Greek            */
	{(char *)"maccyrillic", CS_MAC_CYRILLIC},    /* Mac Cyrillic         */
	{(char *)"macturkish", CS_MAC_TURKISH},    /* Mac Turkish          */
	{(char *)"gb2312", CS_WINDOWS_936},    /* Simplified Chinese   */
	{(char *)"big5", CS_WINDOWS_950},    /* Traditional Chinese  */
	{(char *)"shiftjis", CS_WINDOWS_932},    /* Japanese             */
	{(char *)"eucjp", CS_JAPANESE_EUC},    /* Japanese             */
	{(char *)"iso2022-jp", CS_JAPANESE},    /* Japanese             */
	{(char *)"koi8r", CS_RUSSIAN_KOI8},    /* Russian              */
	{(char *)"windows1250", CS_WINDOWS_1250},    /* Eastern European     */
	{(char *)"windows1251", CS_WINDOWS_1251},    /* Cyrillic             */
	{(char *)"windows1252", CS_WINDOWS_1252},    /* Western              */
	{(char *)"windows1253", CS_WINDOWS_1253},    /* Greek                */
	{(char *)"windows1254", CS_WINDOWS_1254},    /* Turkish              */
	{(char *)"windows1255", CS_WINDOWS_1255},    /* Hebrew               */
	{(char *)"windows1256", CS_WINDOWS_1256},    /* Arabic               */
	{(char *)"windows1257", CS_WINDOWS_1257},    /* Baltic               */
	{(char *)"thai874", CS_WINDOWS_874},    /* Thai                 */
	{(char *)"koreanhangul", CS_WINDOWS_949},    /* Korean Hangul        */
	{(char *)"utf8", CS_UTF8},    /* UTF-8                */
	{(char *)"unicode", CS_UNICODE},    /* Unicode              */
	{NULL, STRINGDW_LISTEND}
};

StringDW DefaultInputCharsets[] =
{
	{(char *)"systemdefault", CS_SYSTEMDEFAULT},
	{(char *)"jis", CS_JAPANESE},
	{(char *)"euc_jp", CS_JAPANESE_EUC},
	{(char *)"cns11643_1", CS_CHINESE_TRAD1},
	{(char *)"euc_cns_1", CS_CHINESE_EUC_TRAD1},
	{(char *)"cns11643_2", CS_CHINESE_TRAD2},
	{(char *)"euc_cns_2", CS_CHINESE_EUC_TRAD2},
	{(char *)"ksc1987", CS_KOREAN},
	{(char *)"gb2312", CS_CHINESE_SIMPLE},
	{(char *)"ebcdic37", CS_EBCDIC_37},
	{(char *)"ebcdic273", CS_EBCDIC_273},
	{(char *)"ebcdic274", CS_EBCDIC_274},
	{(char *)"ebcdic277", CS_EBCDIC_277},
	{(char *)"ebcdic278", CS_EBCDIC_278},
	{(char *)"ebcdic280", CS_EBCDIC_280},
	{(char *)"ebcdic282", CS_EBCDIC_282},
	{(char *)"ebcdic284", CS_EBCDIC_284},
	{(char *)"ebcdic285", CS_EBCDIC_285},
	{(char *)"ebcdic297", CS_EBCDIC_297},
	{(char *)"ebcdic500", CS_EBCDIC_500},
	{(char *)"ebcdic1026", CS_EBCDIC_1026},
	{(char *)"ascii", CS_ASCII},
	{(char *)"ansi437", CS_DOS_437},
	{(char *)"ansi737", CS_DOS_737},
	{(char *)"ansi850", CS_DOS_850},
	{(char *)"ansi852", CS_DOS_852},
	{(char *)"ansi855", CS_DOS_855},
	{(char *)"ansi857", CS_DOS_857},
	{(char *)"ansi860", CS_DOS_860},
	{(char *)"ansi861", CS_DOS_861},
	{(char *)"ansi863", CS_DOS_863},
	{(char *)"ansi865", CS_DOS_865},
	{(char *)"ansi866", CS_DOS_866},
	{(char *)"ansi869", CS_DOS_869},
	{(char *)"ansi874", CS_WINDOWS_874},
	{(char *)"ansi932", CS_WINDOWS_932},
	{(char *)"ansi936", CS_WINDOWS_936},
	{(char *)"ansi949", CS_WINDOWS_949},
	{(char *)"ansi950", CS_WINDOWS_950},
	{(char *)"ansi1250", CS_WINDOWS_1250},
	{(char *)"ansi1251", CS_WINDOWS_1251},
	{(char *)"ansi1252", CS_WINDOWS_1252},
	{(char *)"ansi1253", CS_WINDOWS_1253},
	{(char *)"ansi1254", CS_WINDOWS_1254},
	{(char *)"ansi1255", CS_WINDOWS_1255},
	{(char *)"ansi1256", CS_WINDOWS_1256},
	{(char *)"ansi1257", CS_WINDOWS_1257},
	{(char *)"unicode", CS_UNICODE},
	{(char *)"iso8859_1", CS_ISO8859_1},
	{(char *)"iso8859_2", CS_ISO8859_2},
	{(char *)"iso8859_3", CS_ISO8859_3},
	{(char *)"iso8859_4", CS_ISO8859_4},
	{(char *)"iso8859_5", CS_ISO8859_5},
	{(char *)"iso8859_6", CS_ISO8859_6},
	{(char *)"iso8859_7", CS_ISO8859_7},
	{(char *)"iso8859_8", CS_ISO8859_8},
	{(char *)"iso8859_9", CS_ISO8859_9},
	{(char *)"macroman", CS_MAC_ROMAN},
	{(char *)"maccroatian", CS_MAC_CROATIAN},
	{(char *)"macromanian", CS_MAC_ROMANIAN},
	{(char *)"macturkish", CS_MAC_TURKISH},
	{(char *)"macicelandic", CS_MAC_ICELANDIC},
	{(char *)"maccyrillic", CS_MAC_CYRILLIC},
	{(char *)"macgreek", CS_MAC_GREEK},
	{(char *)"macce", CS_MAC_CE},
	{(char *)"hebrew", CS_MAC_HEBREW},
	{(char *)"arabic", CS_MAC_ARABIC},
	{(char *)"macjis", CS_MAC_JAPANESE},
	{(char *)"hproman8", CS_HPROMAN8},
	{(char *)"bidi_oldcode", CS_BIDI_OLDCODE},
	{(char *)"bidi_pc8", CS_BIDI_PC8},
	{(char *)"bidi_e0", CS_BIDI_E0},
	{(char *)"htmlkoi8", CS_RUSSIAN_KOI8},
	{(char *)"jis_roman", CS_JAPANESE_X0201},
	{(char *)"utf8", CS_UTF8},
	{(char *)"utf7", CS_UTF7},
	{(char *)"utf7", CS_UTF7},
	{(char *)"littleendianunicode", CS_LITTLEENDIAN_UNICODE},
	{(char *)"bigendianunicode", CS_BIGENDIAN_UNICODE},
	{NULL, STRINGDW_LISTEND}
};

StringDW CharByteOrder[] =
{
	{(char *)"template", SCCEX_CHARBYTEORDER_TEMPLATE},
	{(char *)"bigendian", SCCEX_CHARBYTEORDER_BIGENDIAN},
	{(char *)"littleendian", SCCEX_CHARBYTEORDER_LITTLEENDIAN},
	{NULL, STRINGDW_LISTEND}
};

StringDW FallbackFormat[] =
{
	{(char *)"fi_ansi", FI_ANSI},
	{(char *)"fi_ansi8", FI_ANSI8},
	{(char *)"fi_ascii", FI_ASCII},
	{(char *)"fi_ascii8", FI_ASCII8},
	{(char *)"fi_centraleu_1250", FI_CENTRALEU_1250},
	{(char *)"fi_cyrillic1251", FI_CYRILLIC1251},
	{(char *)"fi_cyrillickoi8", FI_CYRILLICKOI8},
	{(char *)"fi_latin2", FI_LATIN2},
	{(char *)"fi_mac", FI_MAC},
	{(char *)"fi_mac8", FI_MAC8},
	{(char *)"fi_chinesegb", FI_CHINESEGB},
	{(char *)"fi_chinesebig5", FI_CHINESEBIG5},
	{(char *)"fi_hangeul", FI_HANGEUL},
	{(char *)"fi_japanese_euc", FI_JAPANESE_EUC},
	{(char *)"fi_japanese_jis", FI_JAPANESE_JIS},
	{(char *)"fi_shiftjis", FI_SHIFTJIS},
	{(char *)"fi_hebrew_oldcode", FI_HEBREW_OLDCODE},
	{(char *)"fi_hebrew_windows", FI_HEBREW_WINDOWS},
	{(char *)"fi_arabic_windows", FI_ARABIC_WINDOWS},
	{(char *)"fi_utf8", FI_UTF8},
	{(char *)"fi_unicode", FI_UNICODE},
	{(char *)"fi_none", FI_NONE},
	{(char *)"fi_greek_windows", FI_GREEK_WINDOWS},
	{(char *)"fi_turkish_windows", FI_TURKISH_WINDOWS},
	{(char *)"fi_baltic_windows", FI_BALTIC_WINDOWS},
	{(char *)"fi_thai_windows", FI_THAI_WINDOWS},
	{(char *)"fi_arabic_iso", FI_ARABIC_ISO},
	{(char *)"fi_vietnamese_windows", FI_VIETNAMESE_WINDOWS},
	{(char *)"fi_text", FI_TEXT},
	{NULL, STRINGDW_LISTEND}
};

StringDW FIFlags[] =
{
	{(char *)"sccut_fi_normal", SCCUT_FI_NORMAL},
	{(char *)"sccut_fi_extendedtest", SCCUT_FI_EXTENDEDTEST},
	{NULL, STRINGDW_LISTEND}
};

StringDW LinkActions[] =
{
	{(char *)"convert", EX_ACTION_CONVERT},
	{(char *)"create", EX_ACTION_CREATELINK},
	{(char *)"skip", EX_ACTION_SKIP},
	{NULL, STRINGDW_LISTEND}
};

StringDW AcrossDown[] =
{
	{(char *)"across", SCCEX_GRIDADVANCE_ACROSS},
	{(char *)"down", SCCEX_GRIDADVANCE_DOWN},
	{NULL, STRINGDW_LISTEND}
};

StringDW XMLDefMethod[] =
{
	{(char *)"dtd", SCCEX_XML_XDM_DTD},
	{(char *)"xsd", SCCEX_XML_XDM_XSD},
	{(char *)"none", SCCEX_XML_XDM_NONE},
	{NULL, STRINGDW_LISTEND}
};

StringDW UnmappedTextProduction[] =
{
	{(char *)"onlyunmapped", SCCEX_XML_JUST_UNMAPPEDTEXT},
	{(char *)"none", SCCEX_XML_NO_UNMAPPEDTEXT},
	{(char *)"both", SCCEX_XML_BOTH_UNMAPPEDTEXT},
	{NULL, STRINGDW_LISTEND}
};

StringDW ExtractEmbeddedFormat[] =
{
	{(char *)"off", SCCEX_EXTRACT_OFF},
	{(char *)"convert", SCCEX_EXTRACT_CONVERT},
	{(char *)"binary", SCCEX_EXTRACT_BINARY},
	{NULL, STRINGDW_LISTEND}
};

StringDW SSDBBorderOptions[] =
{
	{(char *)"creatingifmissing", SCCEX_SSDBBORDERS_CREATEIFMISSING},
	{(char *)"off", SCCEX_SSDBBORDERS_OFF},
	{(char *)"usesource", SCCEX_SSDBBORDERS_USESOURCE},
	{NULL, STRINGDW_LISTEND}
};

StringDW ODGraphicOptions[] =
{
	{(char *)"standard", CCOD_GRAPHICOPTIONS_STANDARD},
	{(char *)"none", CCOD_GRAPHICOPTIONS_NONE},
	{(char *)"extract", CCOD_GRAPHICOPTIONS_EXTRACT},
	{(char *)"convert", CCOD_GRAPHICOPTIONS_CONVERT},
	{NULL, STRINGDW_LISTEND}
};

StringDW CCFlexCharMapping[] =
{
	{(char *)"default", CCFLEX_CHARMAPPING_DEFAULT},
	{(char *)"nomapping", CCFLEX_CHARMAPPING_NOMAPPING},
	{(char *)"maptext", CCFLEX_CHARMAPPING_MAPTEXT},
	{(char *)"both", CCFLEX_CHARMAPPING_BOTH},
	{NULL, STRINGDW_LISTEND}
};

StringDW OcrQualityMapping[] =
{
	{(char *)"fast", OCR_QUAL_FAST},
	{(char *)"slow", OCR_QUAL_SLOW},
	{(char *)"balanced", OCR_QUAL_BALANCED},
	{NULL, STRINGDW_LISTEND}
};

StringDW OcrTechMapping[] =
{
	{(char *)"none", OCR_TECH_NONE},
	{(char *)"nuance", OCR_TECH_NUANCE},
	{(char *)"other", OCR_TECH_OTHER},
	{NULL, STRINGDW_LISTEND}
};

StringDW LinearUnits[] =
{
	{(char *)"default", CCFLEX_LINEARUNITS_DEFAULT},
	{(char *)"cm", CCFLEX_LINEARUNITS_CM},
	{(char *)"co", CCFLEX_LINEARUNITS_CO},
	{(char *)"dd", CCFLEX_LINEARUNITS_DD},
	{(char *)"ft", CCFLEX_LINEARUNITS_FT},
	{(char *)"in", CCFLEX_LINEARUNITS_IN},
	{(char *)"km", CCFLEX_LINEARUNITS_KM},
	{(char *)"m", CCFLEX_LINEARUNITS_M},
	{(char *)"mi", CCFLEX_LINEARUNITS_MI},
	{(char *)"mm", CCFLEX_LINEARUNITS_MM},
	{(char *)"pc", CCFLEX_LINEARUNITS_PC},
	{(char *)"pt", CCFLEX_LINEARUNITS_PT},
	{(char *)"tp", CCFLEX_LINEARUNITS_TP},
	{(char *)"yd", CCFLEX_LINEARUNITS_YD},
	{NULL, STRINGDW_LISTEND}
};

StringDW EmailHeader[] =
{
	{(char *)"all", SCCUT_WP_EMAILHEADERALL},
	{(char *)"standard", SCCUT_WP_EMAILHEADERSTANDARD},
	{(char *)"none", SCCUT_WP_EMAILHEADERNONE},
	{NULL, STRINGDW_LISTEND}
};

StringDW DocumentMemoryMode[] =
{
	{(char *)"smallest", SCCDOCUMENTMEMORYMODE_SMALLEST},
	{(char *)"small", SCCDOCUMENTMEMORYMODE_SMALL},
	{(char *)"medium", SCCDOCUMENTMEMORYMODE_MEDIUM},
	{(char *)"large", SCCDOCUMENTMEMORYMODE_LARGE},
	{(char *)"largest", SCCDOCUMENTMEMORYMODE_LARGEST},
	{NULL, STRINGDW_LISTEND}
};

StringDW OLEEmbeddingProcessMode[] =
{
	{(char *)"standard", SCCOPT_PROCESS_OLEEMBED_STANDARD},
	{(char *)"all", SCCOPT_PROCESS_OLEEMBED_ALL},
	{(char *)"none", SCCOPT_PROCESS_OLEEMBED_NONE},
	{NULL, STRINGDW_LISTEND}
};

StringDW BiDiReorderMethod[] =
{
	{(char *)"off", SCCUT_REORDER_UNICODE_OFF},
	{(char *)"lefttoright", SCCUT_REORDER_UNICODE_LTOR},
	{(char *)"righttoleft", SCCUT_REORDER_UNICODE_RTOL},
	{NULL, STRINGDW_LISTEND}
};

StringDW HtmlCondCommentMode[] =
{
	{(char *)"ie5", HTML_COND_COMMENT_IE5},
	{(char *)"ie6", HTML_COND_COMMENT_IE6},
	{(char *)"ie7", HTML_COND_COMMENT_IE7},
	{(char *)"ie8", HTML_COND_COMMENT_IE8},
	{(char *)"ie9", HTML_COND_COMMENT_IE9},
	{(char *)"none", HTML_COND_COMMENT_NONE},
	{(char *)"all", HTML_COND_COMMENT_ALL},
};

StringDW ExportStatusType[] =
{
	{(char *)"information", EXSTATUS_INFORMATION},
	{NULL, STRINGDW_LISTEND}
};

StringDW EmailHeaderId[] =
{
	{(char *)"nonstandard0", NONSTANDARD_HEADER_ID_BASE},
	{(char *)"nonstandard1", NONSTANDARD_HEADER_ID_BASE + 1},
	{(char *)"nonstandard2", NONSTANDARD_HEADER_ID_BASE + 2},
	{(char *)"nonstandard3", NONSTANDARD_HEADER_ID_BASE + 3},
	{(char *)"nonstandard4", NONSTANDARD_HEADER_ID_BASE + 4},
	{(char *)"nonstandard5", NONSTANDARD_HEADER_ID_BASE + 5},
	{(char *)"nonstandard6", NONSTANDARD_HEADER_ID_BASE + 6},
	{(char *)"nonstandard7", NONSTANDARD_HEADER_ID_BASE + 7},
	{(char *)"nonstandard8", NONSTANDARD_HEADER_ID_BASE + 8},
	{(char *)"nonstandard9", NONSTANDARD_HEADER_ID_BASE + 9},
	{(char *)"nonstandard10", NONSTANDARD_HEADER_ID_BASE + 10},
	{(char *)"nonstandard11", NONSTANDARD_HEADER_ID_BASE + 11},
	{(char *)"nonstandard12", NONSTANDARD_HEADER_ID_BASE + 12},
	{(char *)"nonstandard13", NONSTANDARD_HEADER_ID_BASE + 13},
	{(char *)"nonstandard14", NONSTANDARD_HEADER_ID_BASE + 14},
	{(char *)"nonstandard15", NONSTANDARD_HEADER_ID_BASE + 15},
	{(char *)"nonstandard16", NONSTANDARD_HEADER_ID_BASE + 16},
	{(char *)"nonstandard17", NONSTANDARD_HEADER_ID_BASE + 17},
	{(char *)"nonstandard18", NONSTANDARD_HEADER_ID_BASE + 18},
	{(char *)"nonstandard19", NONSTANDARD_HEADER_ID_BASE + 19},
	{(char *)"nonstandard20", NONSTANDARD_HEADER_ID_BASE + 20},
	{(char *)"nonstandard21", NONSTANDARD_HEADER_ID_BASE + 21},
	{(char *)"nonstandard22", NONSTANDARD_HEADER_ID_BASE + 22},
	{(char *)"nonstandard23", NONSTANDARD_HEADER_ID_BASE + 23},
	{(char *)"nonstandard24", NONSTANDARD_HEADER_ID_BASE + 24},
	{(char *)"nonstandard25", NONSTANDARD_HEADER_ID_BASE + 25},
	{(char *)"nonstandard26", NONSTANDARD_HEADER_ID_BASE + 26},
	{(char *)"nonstandard27", NONSTANDARD_HEADER_ID_BASE + 27},
	{(char *)"nonstandard28", NONSTANDARD_HEADER_ID_BASE + 28},
	{(char *)"nonstandard29", NONSTANDARD_HEADER_ID_BASE + 29},
	{(char *)"nonstandard30", NONSTANDARD_HEADER_ID_BASE + 30},
	{(char *)"nonstandard31", NONSTANDARD_HEADER_ID_BASE + 31},
	{(char *)"nonstandard32", NONSTANDARD_HEADER_ID_BASE + 32},
	{(char *)"nonstandard33", NONSTANDARD_HEADER_ID_BASE + 33},
	{(char *)"nonstandard34", NONSTANDARD_HEADER_ID_BASE + 34},
	{(char *)"nonstandard35", NONSTANDARD_HEADER_ID_BASE + 35},
	{(char *)"nonstandard36", NONSTANDARD_HEADER_ID_BASE + 36},
	{(char *)"nonstandard37", NONSTANDARD_HEADER_ID_BASE + 37},
	{(char *)"nonstandard38", NONSTANDARD_HEADER_ID_BASE + 38},
	{(char *)"nonstandard39", NONSTANDARD_HEADER_ID_BASE + 39},
	{(char *)"nonstandard40", NONSTANDARD_HEADER_ID_BASE + 40},
	{(char *)"nonstandard41", NONSTANDARD_HEADER_ID_BASE + 41},
	{(char *)"nonstandard42", NONSTANDARD_HEADER_ID_BASE + 42},
	{(char *)"nonstandard43", NONSTANDARD_HEADER_ID_BASE + 43},
	{(char *)"nonstandard44", NONSTANDARD_HEADER_ID_BASE + 44},
	{(char *)"nonstandard45", NONSTANDARD_HEADER_ID_BASE + 45},
	{(char *)"nonstandard46", NONSTANDARD_HEADER_ID_BASE + 46},
	{(char *)"nonstandard47", NONSTANDARD_HEADER_ID_BASE + 47},
	{(char *)"nonstandard48", NONSTANDARD_HEADER_ID_BASE + 48},
	{(char *)"nonstandard49", NONSTANDARD_HEADER_ID_BASE + 49},
	{(char *)"nonstandard50", NONSTANDARD_HEADER_ID_BASE + 50},
	{(char *)"nonstandard51", NONSTANDARD_HEADER_ID_BASE + 51},
	{(char *)"nonstandard52", NONSTANDARD_HEADER_ID_BASE + 52},
	{(char *)"nonstandard53", NONSTANDARD_HEADER_ID_BASE + 53},
	{(char *)"nonstandard54", NONSTANDARD_HEADER_ID_BASE + 54},
	{(char *)"nonstandard55", NONSTANDARD_HEADER_ID_BASE + 55},
	{(char *)"nonstandard56", NONSTANDARD_HEADER_ID_BASE + 56},
	{(char *)"nonstandard57", NONSTANDARD_HEADER_ID_BASE + 57},
	{(char *)"nonstandard58", NONSTANDARD_HEADER_ID_BASE + 58},
	{(char *)"nonstandard59", NONSTANDARD_HEADER_ID_BASE + 59},
	{(char *)"nonstandard60", NONSTANDARD_HEADER_ID_BASE + 60},
	{(char *)"nonstandard61", NONSTANDARD_HEADER_ID_BASE + 61},
	{(char *)"nonstandard62", NONSTANDARD_HEADER_ID_BASE + 62},
	{(char *)"nonstandard63", NONSTANDARD_HEADER_ID_BASE + 63},
	{(char *)"nonstandard64", NONSTANDARD_HEADER_ID_BASE + 64},
	{(char *)"nonstandard65", NONSTANDARD_HEADER_ID_BASE + 65},
	{(char *)"nonstandard66", NONSTANDARD_HEADER_ID_BASE + 66},
	{(char *)"nonstandard67", NONSTANDARD_HEADER_ID_BASE + 67},
	{(char *)"nonstandard68", NONSTANDARD_HEADER_ID_BASE + 68},
	{(char *)"nonstandard69", NONSTANDARD_HEADER_ID_BASE + 69},
	{(char *)"nonstandard70", NONSTANDARD_HEADER_ID_BASE + 70},
	{(char *)"nonstandard71", NONSTANDARD_HEADER_ID_BASE + 71},
	{(char *)"nonstandard72", NONSTANDARD_HEADER_ID_BASE + 72},
	{(char *)"nonstandard73", NONSTANDARD_HEADER_ID_BASE + 73},
	{(char *)"nonstandard74", NONSTANDARD_HEADER_ID_BASE + 74},
	{(char *)"nonstandard75", NONSTANDARD_HEADER_ID_BASE + 75},
	{(char *)"nonstandard76", NONSTANDARD_HEADER_ID_BASE + 76},
	{(char *)"nonstandard77", NONSTANDARD_HEADER_ID_BASE + 77},
	{(char *)"nonstandard78", NONSTANDARD_HEADER_ID_BASE + 78},
	{(char *)"nonstandard79", NONSTANDARD_HEADER_ID_BASE + 79},
	{(char *)"nonstandard80", NONSTANDARD_HEADER_ID_BASE + 80},
	{(char *)"nonstandard81", NONSTANDARD_HEADER_ID_BASE + 81},
	{(char *)"nonstandard82", NONSTANDARD_HEADER_ID_BASE + 82},
	{(char *)"nonstandard83", NONSTANDARD_HEADER_ID_BASE + 83},
	{(char *)"nonstandard84", NONSTANDARD_HEADER_ID_BASE + 84},
	{(char *)"nonstandard85", NONSTANDARD_HEADER_ID_BASE + 85},
	{(char *)"nonstandard86", NONSTANDARD_HEADER_ID_BASE + 86},
	{(char *)"nonstandard87", NONSTANDARD_HEADER_ID_BASE + 87},
	{(char *)"nonstandard88", NONSTANDARD_HEADER_ID_BASE + 88},
	{(char *)"nonstandard89", NONSTANDARD_HEADER_ID_BASE + 89},
	{(char *)"nonstandard90", NONSTANDARD_HEADER_ID_BASE + 90},
	{(char *)"nonstandard91", NONSTANDARD_HEADER_ID_BASE + 91},
	{(char *)"nonstandard92", NONSTANDARD_HEADER_ID_BASE + 92},
	{(char *)"nonstandard93", NONSTANDARD_HEADER_ID_BASE + 93},
	{(char *)"nonstandard94", NONSTANDARD_HEADER_ID_BASE + 94},
	{(char *)"nonstandard95", NONSTANDARD_HEADER_ID_BASE + 95},
	{(char *)"nonstandard96", NONSTANDARD_HEADER_ID_BASE + 96},
	{(char *)"nonstandard97", NONSTANDARD_HEADER_ID_BASE + 97},
	{(char *)"nonstandard98", NONSTANDARD_HEADER_ID_BASE + 98},
	{(char *)"nonstandard99", NONSTANDARD_HEADER_ID_BASE + 99},
	{(char *)"to", SCCCA_MAIL_TO},
	{(char *)"cc", SCCCA_MAIL_CC},
	{(char *)"bcc", SCCCA_MAIL_BCC},
	{(char *)"subject", SCCCA_MAIL_SUBJECT},
	{(char *)"msgflag", SCCCA_MAIL_MSGFLAG},
	{(char *)"SCCCA_MAIL_FLAGSTS", SCCCA_MAIL_FLAGSTS},
	{(char *)"expires", SCCCA_MAIL_EXPIRES},
	{(char *)"categories", SCCCA_MAIL_CATEGORIES},
	{(char *)"importance", SCCCA_MAIL_IMPORTANCE},
	{(char *)"sensitivity", SCCCA_MAIL_SENSITIVITY},
	{(char *)"location", SCCCA_MAIL_LOCATION},
	{(char *)"fullname", SCCCA_MAIL_FULLNAME},
	{(char *)"jobtitle", SCCCA_MAIL_JOBTITLE},
	{(char *)"company", SCCCA_MAIL_COMPANY},
	{(char *)"email", SCCCA_MAIL_EMAIL},
	{(char *)"webpage", SCCCA_MAIL_WEBPAGE},
	{(char *)"workphone", SCCCA_MAIL_WORKPHONE},
	{(char *)"homephone", SCCCA_MAIL_HOMEPHONE},
	{(char *)"from", SCCCA_MAIL_FROM},
	{(char *)"attachment", SCCCA_MAIL_ATTACHMENT},
	{(char *)"rtfbody", SCCCA_MAIL_RTFBODY},
	{(char *)"received", SCCCA_MAIL_RECEIVED},
	{(char *)"size", SCCCA_MAIL_SIZE},
	{(char *)"lastmodified", SCCCA_MAIL_LASTMODIFIED},
	{(char *)"newsgroups", SCCCA_MAIL_NEWSGROUPS},
	{(char *)"submittime", SCCCA_MAIL_SUBMITTIME},
	{(char *)"ccme", SCCCA_MAIL_CCME},
	{(char *)"alternate_recipient_allowed", SCCCA_MAIL_ALTERNATE_RECIPIENT_ALLOWED},
	{(char *)"client_submit_time", SCCCA_MAIL_CLIENT_SUBMIT_TIME},
	{(char *)"creation_time", SCCCA_MAIL_CREATION_TIME},
	{(char *)"conversation_index", SCCCA_MAIL_CONVERSATION_INDEX},
	{(char *)"conversation_topic", SCCCA_MAIL_CONVERSATION_TOPIC},
	{(char *)"message_submission_id", SCCCA_MAIL_MESSAGE_SUBMISSION_ID},
	{(char *)"message_class", SCCCA_MAIL_MESSAGE_CLASS},
	{(char *)"originator_delivery_report_requested", SCCCA_MAIL_ORIGINATOR_DELIVERY_REPORT_REQUESTED},
	{(char *)"read_receipt_requested", SCCCA_MAIL_READ_RECEIPT_REQUESTED},
	{(char *)"rcvd_representing_addrtype", SCCCA_MAIL_RCVD_REPRESENTING_ADDRTYPE},
	{(char *)"rcvd_representing_email_address", SCCCA_MAIL_RCVD_REPRESENTING_EMAIL_ADDRESS},
	{(char *)"rcvd_representing_entryid", SCCCA_MAIL_RCVD_REPRESENTING_ENTRYID},
	{(char *)"rcvd_representing_name", SCCCA_MAIL_RCVD_REPRESENTING_NAME},
	{(char *)"rcvd_representing_search_key", SCCCA_MAIL_RCVD_REPRESENTING_SEARCH_KEY},
	{(char *)"received_by_addrtype", SCCCA_MAIL_RECEIVED_BY_ADDRTYPE},
	{(char *)"received_by_email_address", SCCCA_MAIL_RECEIVED_BY_EMAIL_ADDRESS},
	{(char *)"received_by_entryid", SCCCA_MAIL_RECEIVED_BY_ENTRYID},
	{(char *)"received_by_name", SCCCA_MAIL_RECEIVED_BY_NAME},
	{(char *)"received_by_search_key", SCCCA_MAIL_RECEIVED_BY_SEARCH_KEY},
	{(char *)"rtf_in_sync", SCCCA_MAIL_RTF_IN_SYNC},
	{(char *)"rtf_sync_body_count", SCCCA_MAIL_RTF_SYNC_BODY_COUNT},
	{(char *)"rtf_sync_body_crc", SCCCA_MAIL_RTF_SYNC_BODY_CRC},
	{(char *)"rtf_sync_body_tag", SCCCA_MAIL_RTF_SYNC_BODY_TAG},
	{(char *)"rtf_sync_prefix_count", SCCCA_MAIL_RTF_SYNC_PREFIX_COUNT},
	{(char *)"rtf_sync_trailing_count", SCCCA_MAIL_RTF_SYNC_TRAILING_COUNT},
	{(char *)"search_key", SCCCA_MAIL_SEARCH_KEY},
	{(char *)"sender_addrtype", SCCCA_MAIL_SENDER_ADDRTYPE},
	{(char *)"sender_email_address", SCCCA_MAIL_SENDER_EMAIL_ADDRESS},
	{(char *)"sender_entryid", SCCCA_MAIL_SENDER_ENTRYID},
	{(char *)"sender_name", SCCCA_MAIL_SENDER_NAME},
	{(char *)"sender_search_key", SCCCA_MAIL_SENDER_SEARCH_KEY},
	{(char *)"sent_representing_addrtype", SCCCA_MAIL_SENT_REPRESENTING_ADDRTYPE},
	{(char *)"sent_representing_email_address", SCCCA_MAIL_SENT_REPRESENTING_EMAIL_ADDRESS},
	{(char *)"sent_representing_entryid", SCCCA_MAIL_SENT_REPRESENTING_ENTRYID},
	{(char *)"sent_representing_search_key", SCCCA_MAIL_SENT_REPRESENTING_SEARCH_KEY},
	{(char *)"transport_message_headers", SCCCA_MAIL_TRANSPORT_MESSAGE_HEADERS},
	{(char *)"priority", SCCCA_MAIL_PRIORITY},
	{(char *)"auto_forwarded", SCCCA_MAIL_AUTO_FORWARDED},
	{(char *)"deferred_delivery_time", SCCCA_MAIL_DEFERRED_DELIVERY_TIME},
	{(char *)"expiry_time", SCCCA_MAIL_EXPIRY_TIME},
	{(char *)"latest_delivery_time", SCCCA_MAIL_LATEST_DELIVERY_TIME},
	{(char *)"recipient_reassignment_prohibited", SCCCA_MAIL_RECIPIENT_REASSIGNMENT_PROHIBITED},
	{(char *)"reply_time", SCCCA_MAIL_REPLY_TIME},
	{(char *)"report_tag", SCCCA_MAIL_REPORT_TAG},
	{(char *)"response_requested", SCCCA_MAIL_RESPONSE_REQUESTED},
	{(char *)"reply_requested", SCCCA_MAIL_REPLY_REQUESTED},
	{(char *)"delete_after_submit", SCCCA_MAIL_DELETE_AFTER_SUBMIT},
	{(char *)"message_locale_id", SCCCA_MAIL_MESSAGE_LOCALE_ID},
	{(char *)"creator_name", SCCCA_MAIL_CREATOR_NAME},
	{(char *)"creator_entryid", SCCCA_MAIL_CREATOR_ENTRYID},
	{(char *)"last_modifier_name", SCCCA_MAIL_LAST_MODIFIER_NAME},
	{(char *)"last_modifier_entryid", SCCCA_MAIL_LAST_MODIFIER_ENTRYID},
	{(char *)"internet_article_number", SCCCA_MAIL_INTERNET_ARTICLE_NUMBER},
	{(char *)"nt_security_descriptor", SCCCA_MAIL_NT_SECURITY_DESCRIPTOR},
	{(char *)"trust_sender", SCCCA_MAIL_TRUST_SENDER},
	{(char *)"internet_message_id", SCCCA_MAIL_INTERNET_MESSAGE_ID},
	{(char *)"attr_hidden", SCCCA_MAIL_ATTR_HIDDEN},
	{(char *)"attr_system", SCCCA_MAIL_ATTR_SYSTEM},
	{(char *)"attr_readonly", SCCCA_MAIL_ATTR_READONLY},
	{(char *)"internet_cpid", SCCCA_MAIL_INTERNET_CPID},
	{(char *)"message_codepage", SCCCA_MAIL_MESSAGE_CODEPAGE},
	{(char *)"sender_flags", SCCCA_MAIL_SENDER_FLAGS},
	{(char *)"sent_representing_flags", SCCCA_MAIL_SENT_REPRESENTING_FLAGS},
	{(char *)"rcvd_by_flags", SCCCA_MAIL_RCVD_BY_FLAGS},
	{(char *)"rcvd_representing_flags", SCCCA_MAIL_RCVD_REPRESENTING_FLAGS},
	{(char *)"inet_mail_override_format", SCCCA_MAIL_INET_MAIL_OVERRIDE_FORMAT},
	{(char *)"msg_editor_format", SCCCA_MAIL_MSG_EDITOR_FORMAT},
	{(char *)"profile_connect_flags", SCCCA_MAIL_PROFILE_CONNECT_FLAGS},
	{(char *)"sent_representing_name", SCCCA_MAIL_SENT_REPRESENTING_NAME},
	{(char *)"entryid", SCCCA_MAIL_ENTRYID},
	{(char *)"normalized_subject", SCCCA_MAIL_NORMALIZED_SUBJECT},
	{(char *)"attendees", SCCCA_MAIL_ATTENDEES},
	{(char *)"reqattendee", SCCCA_MAIL_REQATTENDEE},
	{(char *)"optattendee", SCCCA_MAIL_OPTATTENDEE},
	{(char *)"fileas", SCCCA_MAIL_FILEAS},
	{(char *)"displayas", SCCCA_MAIL_DISPLAYAS},
	{(char *)"title", SCCCA_MAIL_TITLE},
	{(char *)"suffix", SCCCA_MAIL_SUFFIX},
	{(char *)"nickname", SCCCA_MAIL_NICKNAME},
	{(char *)"profession", SCCCA_MAIL_PROFESSION},
	{(char *)"department", SCCCA_MAIL_DEPARTMENT},
	{(char *)"office", SCCCA_MAIL_OFFICE},
	{(char *)"anniversary", SCCCA_MAIL_ANNIVERSARY},
	{(char *)"birthdate", SCCCA_MAIL_BIRTHDATE},
	{(char *)"assistantsname", SCCCA_MAIL_ASSISTANTSNAME},
	{(char *)"spousesname", SCCCA_MAIL_SPOUSESNAME},
	{(char *)"managersname", SCCCA_MAIL_MANAGERSNAME},
	{(char *)"businessaddress", SCCCA_MAIL_BUSINESSADDRESS},
	{(char *)"businessphone", SCCCA_MAIL_BUSINESSPHONE},
	{(char *)"homeaddress", SCCCA_MAIL_HOMEADDRESS},
	{(char *)"otheraddress", SCCCA_MAIL_OTHERADDRESS},
	{(char *)"mobilephone", SCCCA_MAIL_MOBILEPHONE},
	{(char *)"businessfax", SCCCA_MAIL_BUSINESSFAX},
	{(char *)"imaddress", SCCCA_MAIL_IMADDRESS},
	{(char *)"internetfreebusyaddr", SCCCA_MAIL_INTERNETFREEBUSYADDR},
	{(char *)"remindertopic", SCCCA_MAIL_REMINDERTOPIC},
	{(char *)"contacts", SCCCA_MAIL_CONTACTS},
	{(char *)"callbackphone", SCCCA_MAIL_CALLBACKPHONE},
	{(char *)"first", SCCCA_MAIL_FIRST},
	{(char *)"family", SCCCA_MAIL_FAMILY},
	{(char *)"telenumber", SCCCA_MAIL_TELENUMBER},
	{(char *)"businessphone2", SCCCA_MAIL_BUSINESSPHONE2},
	{(char *)"radiophone", SCCCA_MAIL_RADIOPHONE},
	{(char *)"carphone", SCCCA_MAIL_CARPHONE},
	{(char *)"otherphone", SCCCA_MAIL_OTHERPHONE},
	{(char *)"pagerphone", SCCCA_MAIL_PAGERPHONE},
	{(char *)"otherfax", SCCCA_MAIL_OTHERFAX},
	{(char *)"homefax", SCCCA_MAIL_HOMEFAX},
	{(char *)"telexphone", SCCCA_MAIL_TELEXPHONE},
	{(char *)"isdn", SCCCA_MAIL_ISDN},
	{(char *)"assistantphone", SCCCA_MAIL_ASSISTANTPHONE},
	{(char *)"homephone2", SCCCA_MAIL_HOMEPHONE2},
	{(char *)"middle", SCCCA_MAIL_MIDDLE},
	{(char *)"ttyttdphone", SCCCA_MAIL_TTYTTDPHONE},
	{(char *)"gender", SCCCA_MAIL_GENDER},
	{(char *)"personalhomepage", SCCCA_MAIL_PERSONALHOMEPAGE},
	{(char *)"companyphone", SCCCA_MAIL_COMPANYPHONE},
	{(char *)"homecity", SCCCA_MAIL_HOMECITY},
	{(char *)"homecountry", SCCCA_MAIL_HOMECOUNTRY},
	{(char *)"postalcode", SCCCA_MAIL_POSTALCODE},
	{(char *)"homestate", SCCCA_MAIL_HOMESTATE},
	{(char *)"homestreet", SCCCA_MAIL_HOMESTREET},
	{(char *)"homepobox", SCCCA_MAIL_HOMEPOBOX},
	{(char *)"othercity", SCCCA_MAIL_OTHERCITY},
	{(char *)"othercountry", SCCCA_MAIL_OTHERCOUNTRY},
	{(char *)"otherpostalcode", SCCCA_MAIL_OTHERPOSTALCODE},
	{(char *)"otherstate", SCCCA_MAIL_OTHERSTATE},
	{(char *)"otherstreet", SCCCA_MAIL_OTHERSTREET},
	{(char *)"otherpobox", SCCCA_MAIL_OTHERPOBOX},
	{(char *)"businessstreet", SCCCA_MAIL_BUSINESSSTREET},
	{(char *)"businesscity", SCCCA_MAIL_BUSINESSCITY},
	{(char *)"businessstate", SCCCA_MAIL_BUSINESSSTATE},
	{(char *)"businesspostalcode", SCCCA_MAIL_BUSINESSPOSTALCODE},
	{(char *)"businesscountry", SCCCA_MAIL_BUSINESSCOUNTRY},
	{(char *)"businesspobox", SCCCA_MAIL_BUSINESSPOBOX},
	{(char *)"displayas2", SCCCA_MAIL_DISPLAYAS2},
	{(char *)"email2", SCCCA_MAIL_EMAIL2},
	{(char *)"displayas3", SCCCA_MAIL_DISPLAYAS3},
	{(char *)"email3", SCCCA_MAIL_EMAIL3},
	{(char *)"dtstart", SCCCA_MAIL_DTSTART},
	{(char *)"dtend", SCCCA_MAIL_DTEND},
	{(char *)"start", SCCCA_MAIL_START},
	{(char *)"duration", SCCCA_MAIL_DURATION},
	{(char *)"accureattype1", SCCCA_MAIL_ACCUREATTYPE1},
	{(char *)"entry_type", SCCCA_MAIL_ENTRY_TYPE},
	{(char *)"status", SCCCA_MAIL_STATUS},
	{(char *)"percent_complete", SCCCA_MAIL_PERCENT_COMPLETE},
	{(char *)"start_date", SCCCA_MAIL_START_DATE},
	{(char *)"due_date", SCCCA_MAIL_DUE_DATE},
	{(char *)"completed", SCCCA_MAIL_COMPLETED},
	{(char *)"actualwork", SCCCA_MAIL_ACTUALWORK},
	{(char *)"total_work", SCCCA_MAIL_TOTAL_WORK},
	{(char *)"owner", SCCCA_MAIL_OWNER},
	{(char *)"billing", SCCCA_MAIL_BILLING},
	{(char *)"mileage", SCCCA_MAIL_MILEAGE},
	{(char *)"proof_of_delivery_requested", SCCCA_MAIL_PROOF_OF_DELIVERY_REQUESTED},
	{(char *)"proof_of_submission_requested", SCCCA_MAIL_PROOF_OF_SUBMISSION_REQUESTED},
	{(char *)"in_reply_to_id", SCCCA_MAIL_IN_REPLY_TO_ID},
	{(char *)"task_complete", SCCCA_MAIL_TASK_COMPLETE},
	{(char *)"appointment_duration", SCCCA_MAIL_APPOINTMENT_DURATION},
	{NULL, STRINGDW_LISTEND}
};

StringDW PerformanceMode[] =
{
	{(char *)"normal", SCCEX_PERFORMANCE_NORMAL},
	{(char *)"textonly", SCCEX_PERFORMANCE_TEXTONLY},
	{(char *)"textandfonts", SCCEX_PERFORMANCE_TEXTANDFONTS},
	{NULL, STRINGDW_LISTEND}
};

StringDW FontUsage[] =
{
	{(char *)"subset", SCCOPT_SUBSETFONT},
	{(char *)"nameonly", SCCOPT_NAMEFONT},
	{(char *)"ignore", SCCOPT_IGNOREFONT},
	{NULL, STRINGDW_LISTEND}
};


StringDW FontEmbedType[] =
{
	{(char *)"named", SCCFONTS_REFERENCE_BY_NAME},
	{(char *)"exported", SCCFONTS_REFERENCE_EXPORTED},
	{(char *)"referenced", SCCFONTS_REFERENCE_BY_BASE_URL},
	{NULL, STRINGDW_LISTEND}
};

StringDW FontPermissions[] =
{
	{(char *)"default", SCCFONTS_DEFAULT_PERMISSIONS},
	{(char *)"requireinstallable", SCCFONTS_REQUIRE_INSTALLABLE},
	{(char *)"requirepreviewprint", SCCFONTS_REQUIRE_PREVIEW_PRINT},
	{(char *)"requireeditable", SCCFONTS_REQUIRE_EDITABLE},
	{NULL, STRINGDW_LISTEND}
};

StringDW AttachmentHandling[] =
{
	{(char *)"export", SCCOPT_EXPORT},
	{(char *)"exportrecursive", SCCOPT_EXPORT_RECURSIVE},
	{(char *)"extract", SCCOPT_EXTRACT},
	{(char *)"none", SCCOPT_NO_ATTACHMENTS},
	{NULL, STRINGDW_LISTEND}
};

StringDW OutputStructure[] =
{
	{(char *)"flat", SCCWV_STRUCTURE_FLAT},
	{(char *)"ajaxchunked", SCCWV_STRUCTURE_AJAX_CHUNKED},
	{(char *)"ajaxstreaming", SCCWV_STRUCTURE_AJAX_STREAMING},
	{(char *)"pagestreaming", SCCWV_STRUCTURE_PAGE_STREAMING},
	{NULL, STRINGDW_LISTEND}
};

StringDW ImageStampSpecTypes[] =
{
	{(char *)"ansipath", IOTYPE_ANSIPATH},
	{(char *)"unixpath", IOTYPE_UNIXPATH},
	{(char *)"unicodepath", IOTYPE_UNICODEPATH},
	{(char *)"redirect", IOTYPE_REDIRECT},
	{NULL, STRINGDW_LISTEND}
};

/****************************/
/* specific to exporter.exe */
/****************************/

const StringDW LinkFlags[] =
{
	{(char *)"useoutputcharset", EX_FLAG_USEOUTPUTCHARSET},
	{(char *)"putattributes", EX_FLAG_PUTATTRIBUTES},
	{(char *)"dimensionsknown", EX_FLAG_DIMENSIONSKNOWN},
	{(char *)"default", EX_FLAG_DEFAULT},
	{NULL, STRINGDW_LISTEND}
};

const StringDW FileExtensions[] =
{
	{(char *)".htm", FI_HTML},
	{(char *)".wml", FI_WML},
	{(char *)".hdml", FI_HDML},
	{(char *)".htm", FI_XHTMLB},
	{(char *)".chtm", FI_CHTML},
	{(char *)".htm", FI_HTMLWCA},
	{(char *)".htm", FI_HTMLAG},
	{(char *)".htm", FI_WIRELESSHTML},
	{(char *)".txt", FI_TEXT},
	{(char *)".css", FI_HTML_CSS},
	{(char *)".js", FI_JAVASCRIPT},
	{(char *)".jpg", FI_JPEGFIF},
	{(char *)".gif", FI_GIF},
	{(char *)".wbmp", FI_WBMP},
	{(char *)".bmp", FI_BMP},
	{(char *)".png", FI_PNG},
	{(char *)".tif", FI_TIFF},
	{(char *)".mht", FI_MHTML},
	{(char *)".pdf", FI_PDF},
	{(char *)".pdf", FI_PDFA},
	{(char *)".pdf", FI_PDFA_2},
	{(char *)".txt", FI_OCRTEXT},
	{(char *)".jp2", FI_JPEG2000},
	{(char *)".html", FI_HTML5},
	{NULL, STRINGDW_LISTEND}
};

StringDW CustomFileExtensions[] =
{
	{NULL, FI_HTML},
	{NULL, FI_WML},
	{NULL, FI_HDML},
	{NULL, FI_XHTMLB},
	{NULL, FI_CHTML},
	{NULL, FI_HTMLWCA},
	{NULL, FI_HTMLAG},
	{NULL, FI_WIRELESSHTML},
	{NULL, FI_TEXT},
	{NULL, FI_HTML_CSS},
	{NULL, FI_JAVASCRIPT},
	{NULL, FI_JPEGFIF},
	{NULL, FI_GIF},
	{NULL, FI_WBMP},
	{NULL, FI_BMP},
	{NULL, FI_PNG},
	{NULL, FI_MHTML},
	{NULL, STRINGDW_LISTEND}
};

char * CustomElements[] =
{
	(char *)"firstprevhref",
	NULL
};

/* Wide char declaration of the string "http://www.outsideinsdk.com/xmlns/flexiondoc5_2,style_tables" */
static VTWORD SUBTREES_OPTIONVAL[] = {'h', 't', 't', 'p', ':', '/', '/', 'w', 'w', 'w', '.', 'o', 'u', 't', 's', 'i', 'd', 'e', 'i', 'n', 's', 'd', 'k', '.', 'c', 'o', 'm', '/', 'x', 'm', 'l', 'n', 's', '/', 'f', 'l', 'e', 'x', 'i', 'o', 'n', 'd', 'o', 'c', '5', '_', '2', ',', 's', 't', 'y', 'l', 'e', '_', 't', 'a', 'b', 'l', 'e', 's', '\0'};
static VTWORD EMPTY_STRING[] = {'\0'};
StringWS SubTrees[] =
{
	{(char *)"style_tables", SUBTREES_OPTIONVAL},
	{(char *)"", EMPTY_STRING}
};

/*
|
|   function:   SetBufferSize
|   parameters: Option *pOptions
|   returns:    VTVOID
|
|   purpose:    This sets the buffer size options in the technology as specified in the
|               ".cfg" file.  Options not set in the .cfg file use the
|               default value in the technology.
|
|   note:       exits on error
|
*/

VTVOID SetBufferSize(VTHDOC hDoc, Option *pOptions)
{
	UNUSED(hDoc);
	if (pOptions->abSetOptions[SAMPLE_OPTION_READBUFFERSIZE] ||
		pOptions->abSetOptions[SAMPLE_OPTION_MAPBUFFERSIZE] ||
		pOptions->abSetOptions[SAMPLE_OPTION_TEMPBUFFERSIZE])
	{
		SCCERR err = DASetOption((VTHDOC)NULL, SCCOPT_IO_BUFFERSIZE, (VTLPVOID)&pOptions->bufferOpts, sizeof(SCCBUFFEROPTIONS));
		if (err != SCCERR_OK)
		{
			char errString[256];
			DAGetErrorString(err, errString, sizeof(errString));
			fprintf(stderr, "DASetOption() failed: %s (0x%04X)\n", errString, err);
			DADeInit();
			exit(err);
		}
	}
}



/*
|
|   function:   SetOptions
|   parameters: VTHDOC hDoc
|               Option *pOptions
|   returns:    SCCERR
|
|   purpose:    This sets the options in the technology as specified in the
|               ".cfg" file.  Options not set in the .cfg file use the
|               default value in the technology.
|
*/

SCCERR SetOptions(VTHDOC hDoc, Option *pOptions)
{
	SCCERR seResult = SCCERR_OK;

	/*
	|   If the save object api is going to be used, then set SCCOPT_ENABLEALLSUBOBJECTS
	|   to SCCVW_FILTER_ENABLEALLSUBOBJECTS.  This will over-ride the normal filter
	|   optimizations that may skip some bitmap embeddings.
	*/
	if (pOptions->abSetOptions[SAMPLE_OPTION_OBJECTALL])
	{
		VTDWORD dwEnableSubObjects = (pOptions->bAllObjects) ? SCCVW_FILTER_ENABLEALLSUBOBJECTS : SCCVW_FILTER_NORMALSUBOBJECTS;
		if ((seResult = DASetOption(hDoc, SCCOPT_ENABLEALLSUBOBJECTS, (VTLPVOID)&dwEnableSubObjects, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_TEMPLATE])
		if ((seResult = DASetFileSpecOption(hDoc, SCCOPT_EX_TEMPLATE, IOTYPE_DEFAULT, (VTLPVOID)pOptions->szTemplate)) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_TEMPLATE_U])
		if ((seResult = DASetFileSpecOption(hDoc, SCCOPT_EX_TEMPLATE, IOTYPE_UNICODEPATH, (VTLPVOID)pOptions->wzTemplate)) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_FLAVOR])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_FLAVOR, (VTLPVOID)&pOptions->dwFlavor, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICTYPE])
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_TYPE, (VTLPVOID)&pOptions->dwGraphicType, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_JPEGCOMPRESSION])
		if ((seResult = DASetOption(hDoc, SCCOPT_FILTERJPG, (VTLPVOID)&pOptions->dwJpegCompression, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_LZWCOMPRESSION])
		if ((seResult = DASetOption(hDoc, SCCOPT_FILTERLZW, (VTLPVOID)&pOptions->dwLzwCompression, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PDF_FILTER_REORDER_BIDI])
		if ((seResult = DASetOption(hDoc, SCCOPT_PDF_FILTER_REORDER_BIDI, (VTLPVOID)&pOptions->dwPDF_Filter_Reorder_BIDI, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PDF_FILTER_DROPHYPHENS])
		if ((seResult = DASetOption(hDoc, SCCOPT_PDF_FILTER_DROPHYPHENS, (VTLPVOID)&pOptions->dwPDF_Filter_DropHyphens, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_INTERLACEDGIF])
		if ((seResult = DASetOption(hDoc, SCCOPT_GIF_INTERLACED, (VTLPVOID)&pOptions->bInterlacedGIF, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_QUICKTHUMBNAIL])
		if ((seResult = DASetOption(hDoc, SCCOPT_QUICKTHUMBNAIL, (VTLPVOID)&pOptions->bQuickThumbnail, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_JPEGQUALITY])
		if ((seResult = DASetOption(hDoc, SCCOPT_JPEG_QUALITY, (VTLPVOID)&pOptions->dwJPEGQuality, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_CHARSET])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_OUTPUTCHARACTERSET, (VTLPVOID)&pOptions->dwCharset, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_ISODATETIMES] || pOptions->abSetOptions[SAMPLE_OPTION_STRICTFILEACCESS])
		if ((seResult = DASetOption(hDoc, SCCOPT_FORMATFLAGS, (VTLPVOID)&pOptions->dwGenFlags, sizeof(pOptions->dwGenFlags))) != SCCERR_OK)
			return seResult;

	// NOTE: SCCOPT_EX_COMPLIANCEFLAGS has been deprecated.
	if (pOptions->abSetOptions[SAMPLE_OPTION_STRICT_DTD] || pOptions->abSetOptions[SAMPLE_OPTION_WELLFORMED])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_COMPLIANCEFLAGS, (VTLPVOID)&pOptions->dwFlags, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICSIZEMETHOD])
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_SIZEMETHOD, (VTLPVOID)&pOptions->dwSizeMethod, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_CHARBYTEORDER])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_CHARBYTEORDER, (VTLPVOID)&pOptions->dwCharByteOrder, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_NOSOURCEFORMATTING])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_NOSOURCEFORMATTING, (VTLPVOID)&pOptions->bNoSourceFormatting, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICOUTPUTDPI])
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_OUTPUTDPI, (VTLPVOID)&pOptions->dwGraphicOutputDPI, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PREVENTGRAPHICOVERLAP])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_PREVENTGRAPHICOVERLAP, (VTLPVOID)&pOptions->bPreventGraphicOverlap, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SIMPLESTYLENAMES])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_SIMPLESTYLENAMES, (VTLPVOID)&pOptions->bSimpleStyleNames, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GENBULLETSANDNUMS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_GENBULLETSANDNUMS, (VTLPVOID)&pOptions->bGenBulletsAndNums, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICSIZELIMIT])
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_SIZELIMIT, (VTLPVOID)&pOptions->dwGraphicSizeLimit, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICWIDTHLIMIT])
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WIDTHLIMIT, (VTLPVOID)&pOptions->dwGraphicWidthLimit, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICHEIGHTLIMIT])
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_HEIGHTLIMIT, (VTLPVOID)&pOptions->dwGraphicHeightLimit, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICWIDTH])
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WIDTH, (VTLPVOID)&pOptions->dwGraphicWidth, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICHEIGHT])
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_HEIGHT, (VTLPVOID)&pOptions->dwGraphicHeight, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_LABELWPCELLS] || pOptions->abSetOptions[SAMPLE_OPTION_LABELSSDBCELLS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_LABELFLAGS, (VTLPVOID)&pOptions->dwLabelFlags, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_TEXTBUFFERSIZE])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_TEXTBUFFERSIZE, (VTLPVOID)&pOptions->dwTextBufferSize, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_UNMAPPABLECHAR])
		if ((seResult = DASetOption(hDoc, SCCOPT_UNMAPPABLECHAR, (VTLPVOID)&pOptions->wUnmappableChar, sizeof(VTWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICSKIPSIZE])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_GRAPHICSKIPSIZE, (VTLPVOID)&pOptions->dwGraphicSkipSize, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRAPHICBUFFERSIZE])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_GRAPHICBUFFERSIZE, (VTLPVOID)&pOptions->dwGraphicBufferLimit, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_COLLAPSEWHITESPACE])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_COLLAPSEWHITESPACE, (VTLPVOID)&pOptions->bCollapseWhitespace, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_JAVASCRIPTTABS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_JAVASCRIPTTABS, (VTLPVOID)&pOptions->bJSTabs, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_COMPRESS])
		if ((seResult = DASetOption(hDoc, SCCOPT_APPLYFILTER, (VTLPVOID)&pOptions->bCompress, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SHOWHIDDENTEXT])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_SHOWHIDDENTEXT, (VTLPVOID)&pOptions->bShowHiddenText, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_FILTERNOBLANK])
		if ((seResult = DASetOption(hDoc, SCCOPT_FILTERNOBLANK, (VTLPVOID)&pOptions->bFilterNoBlank, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SHOWHIDDENSSDATA])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_SHOWHIDDENSSDATA, (VTLPVOID)&pOptions->bShowHiddenSSData, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SHOWCHANGETRACKING])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_CHANGETRACKING, (VTLPVOID)&pOptions->bShowChangeTracking, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SHOWSSDBROWCOLHEADINGS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_SSDBROWCOLHEADINGS, (VTLPVOID)&pOptions->bShowSSDBRowColHeadings, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSDBBORDEROPTIONS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_SSDBBORDERS, (VTLPVOID)&pOptions->dwSSDBBorderOptions, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	/* PHTML */
	if (pOptions->abSetOptions[SAMPLE_OPTION_SUPPRESSMETADATA])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_SUPPRESSMETADATA, (VTLPVOID)&pOptions->bSuppressMetadata, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;
	/**/

	if (pOptions->abSetOptions[SAMPLE_OPTION_PERFORMANCEMODE])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_PERFORMANCEMODE, (VTLPVOID)&pOptions->dwPerformanceMode, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_MAXURLLENGTH])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_MAXURLLENGTH, (VTLPVOID)&pOptions->dwMaxURLLength, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PAGESIZE])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_PAGESIZE, (VTLPVOID)&pOptions->dwPageSize, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SEPARATEGRAPHICSBUFFER])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_SEPARATEGRAPHICSBUFFER, (VTLPVOID)&pOptions->bSeparateGraphicsBuffer, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_TEMPDIRECTORY])
	{
		SCCUTTEMPDIRSPEC tdsSpec;
		tdsSpec.dwSize = sizeof(tdsSpec);
		tdsSpec.dwSpecType = IOTYPE_DEFAULT;
		strncpy(tdsSpec.szTempDirName, pOptions->szTempDir, SCCUT_FILENAMEMAX);
		tdsSpec.szTempDirName[SCCUT_FILENAMEMAX - 1] = '\0';
		if ((seResult = DASetOption(hDoc, SCCOPT_TEMPDIR, (VTLPVOID)&tdsSpec, sizeof(tdsSpec))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_TEMPDIRECTORY_U])
	{
		SCCUTTEMPDIRSPEC tdsSpec;
		VTDWORD            length;

		tdsSpec.dwSize = sizeof(tdsSpec);
		tdsSpec.dwSpecType = IOTYPE_ANSIPATH;

		length = UnicodeToAsciiFunc(tdsSpec.szTempDirName, SCCUT_FILENAMEMAX,
									pOptions->wzTempDir, Swcslen(pOptions->wzTempDir) * sizeof(VTWORD));

		if ( (length > 0) && (length != (VTDWORD)-1) )
		{
			tdsSpec.szTempDirName[length] = '\0';
			if ((seResult = DASetOption(hDoc, SCCOPT_TEMPDIR, (VTLPVOID)&tdsSpec, sizeof(tdsSpec))) != SCCERR_OK)
				return seResult;
		}
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_CBALLDISABLED] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBCREATENEWFILE] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBNEWFILEINFO] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBPROCESSLINK] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBCUSTOMELEMENT] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBGRAPHICEXPORTFAILURE] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBOEMOUTPUT] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBALTLINK] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBARCHIVE] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CBALLENABLED])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_CALLBACKS, (VTLPVOID)&pOptions->dwCallbackFlags, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRIDROWS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_GRIDROWS, (VTLPVOID)&pOptions->dwGridRows, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRIDCOLS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_GRIDCOLS, (VTLPVOID)&pOptions->dwGridCols, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRIDADVANCE])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_GRIDADVANCE, (VTLPVOID)&pOptions->dwGridAdvance, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GRIDWRAP])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_GRIDWRAP, (VTLPVOID)&pOptions->bGridWrap, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SUPPRESSFONTSIZE] || pOptions->abSetOptions[SAMPLE_OPTION_SUPPRESSFONTCOLOR] || pOptions->abSetOptions[SAMPLE_OPTION_SUPPRESSFONTFACE])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_FONTFLAGS, (VTLPVOID)&pOptions->dwFontFlags, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if ((pOptions->dwOutputID == FI_XML_FLEXIONDOC5_7) || (pOptions->dwOutputID == FI_XML_FLEXIONDOC_LATEST))
	{
		if (pOptions->abSetOptions[SAMPLE_OPTION_XMLDEFMETHOD])
			if ((seResult = DASetOption(hDoc, SCCOPT_EXXML_DEF_METHOD, (VTLPVOID)&pOptions->dwXMLDefMethod, sizeof(VTDWORD))) != SCCERR_OK)
				return seResult;

		if (pOptions->abSetOptions[SAMPLE_OPTION_XMLDEFREFERENCE])
			if ((seResult = DASetOption(hDoc, SCCOPT_EXXML_DEF_REFERENCE, (VTLPVOID)pOptions->wzXMLDefReference, (Swcslen(pOptions->wzXMLDefReference)) * sizeof(VTWCHAR))) != SCCERR_OK)
				return seResult;
	}
	else
	{
		if (pOptions->abSetOptions[SAMPLE_OPTION_XMLDEFMETHOD])
			if ((seResult = DASetOption(hDoc, SCCOPT_XML_DEF_METHOD, (VTLPVOID)&pOptions->dwXMLDefMethod, sizeof(VTDWORD))) != SCCERR_OK)
				return seResult;

		if (pOptions->abSetOptions[SAMPLE_OPTION_XMLDEFREFERENCE])
			if ((seResult = DASetOption(hDoc, SCCOPT_XML_DEF_REFERENCE, (VTLPVOID)pOptions->wzXMLDefReference, (Swcslen(pOptions->wzXMLDefReference)) * sizeof(VTWCHAR))) != SCCERR_OK)
				return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_NULLREPLACEMENTCHAR])
		if ((seResult = DASetOption(hDoc, SCCOPT_NULLREPLACECHAR, (VTLPVOID)&pOptions->wNullReplacementChar, sizeof(VTWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PSTYLENAMESFLAG] ||
		pOptions->abSetOptions[SAMPLE_OPTION_EMBEDDINGSFLAG] ||
		pOptions->abSetOptions[SAMPLE_OPTION_NOXMLDECLARATIONFLAG] ||
		pOptions->abSetOptions[SAMPLE_OPTION_SUPPRESSPROPERTIES] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PROCESSGENERATEDTEXT] ||
		pOptions->abSetOptions[SAMPLE_OPTION_SUPPRESSATTACHMENTS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_SUPPRESSARCHIVESUBDOCS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_METADATAONLY] ||
		pOptions->abSetOptions[SAMPLE_OPTION_ANNOTATIONS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PRODUCEURLS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PRODUCEOBJECTINFO] ||
		pOptions->abSetOptions[SAMPLE_OPTION_ENABLEERRORINFO] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CELLINFO] ||
		pOptions->abSetOptions[SAMPLE_OPTION_FORMULAS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_GENERATESYSTEMMETADATA] ||
		pOptions->abSetOptions[SAMPLE_OPTION_SKIPSTYLES] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CELLHIDDEN])
		if ((seResult = DASetOption(hDoc, SCCOPT_XML_SEARCHML_FLAGS, (VTLPVOID)&pOptions->dwSearchMLFlags, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_UNMAPPEDTEXT])
		if ((seResult = DASetOption(hDoc, SCCOPT_XML_SEARCHML_UNMAPPEDTEXT, (VTLPVOID)&pOptions->dwUnmappedText, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_CA_XMLOUTPUT])
		if ((seResult = DASetOption(hDoc, SCCOPT_CA_XMLOUTPUT, (VTLPVOID)&pOptions->bXMLOutput, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_BITMAPASBITMAP] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CHARTASBITMAP] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PRESENTATIONASBITMAP] ||
		pOptions->abSetOptions[SAMPLE_OPTION_VECTORASBITMAP] ||
		pOptions->abSetOptions[SAMPLE_OPTION_NOBITMAPELEMENTS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_NOCHARTELEMENTS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_NOPRESENTATIONELEMENTS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_NOVECTORELEMENTS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_ISODATES] ||
		pOptions->abSetOptions[SAMPLE_OPTION_FLATTENSTYLES] ||
		pOptions->abSetOptions[SAMPLE_OPTION_REMOVEFONTGROUPS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_INCLUDETEXTOFFSETS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_USEFULLFILEPATHS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_SEPARATESTYLETABLES] ||
		pOptions->abSetOptions[SAMPLE_OPTION_DELIMITERS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_OPTIMIZESECTIONS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CHARMAPPING] ||
		pOptions->abSetOptions[SAMPLE_OPTION_GENERATESYSTEMMETADATA] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PROCESSARCHIVESUBDOCS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PROCESSEMBEDDINGSUBDOCS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PROCESSATTACHMENTSUBDOCS])
		if ((seResult = DASetOption(hDoc, SCCOPT_CCFLEX_FORMATOPTIONS, (VTLPVOID)&pOptions->dwCCFlexFormatOptions, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	/*
	|    When the NOXMLDECLARATIONFLAG option is set we must set both the
	|    SearchML flags and the PageML flags.  This is because in the OIT
	|    technology both PageML and SearchML have this flag.  Instead of
	|    creating two options as was done there, it was decided that it
	|    actually better if we combine them in this code.
	*/

	if (pOptions->abSetOptions[SAMPLE_OPTION_NOXMLDECLARATIONFLAG] ||
		pOptions->abSetOptions[SAMPLE_OPTION_TEXTOUT])
		if ((seResult = DASetOption(hDoc, SCCOPT_XML_PAGEML_FLAGS, (VTLPVOID)&pOptions->dwPageMLFlags, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_FALLBACKFORMAT])
		if ((seResult = DASetOption(hDoc, SCCOPT_FALLBACKFORMAT, (VTLPVOID)&pOptions->dwFallbackFormat, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_CHARBOLD] || pOptions->abSetOptions[SAMPLE_OPTION_CHARITALIC] || pOptions->abSetOptions[SAMPLE_OPTION_CHARUNDERLINE] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CHARDUNDERLINE] || pOptions->abSetOptions[SAMPLE_OPTION_CHAROUTLINE] || pOptions->abSetOptions[SAMPLE_OPTION_CHARHIDDEN] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CHARSTRIKEOUT] || pOptions->abSetOptions[SAMPLE_OPTION_CHARSMALLCAPS] || pOptions->abSetOptions[SAMPLE_OPTION_CHARALLCAPS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_CHAROCE] || pOptions->abSetOptions[SAMPLE_OPTION_CHARREVISIONDELETE] || pOptions->abSetOptions[SAMPLE_OPTION_CHARREVISIONADD])
		if ((seResult = DASetOption(hDoc, SCCOPT_XML_SEARCHML_CHAR_ATTRS, (VTLPVOID)&pOptions->dwCharAttrs, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PARASPACING] || pOptions->abSetOptions[SAMPLE_OPTION_PARAHEIGHT] || pOptions->abSetOptions[SAMPLE_OPTION_PARALEFTINDENT] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PARARIGHTINDENT] || pOptions->abSetOptions[SAMPLE_OPTION_PARAFIRSTINDENT])
		if ((seResult = DASetOption(hDoc, SCCOPT_XML_SEARCHML_PARA_ATTRS, (VTLPVOID)&pOptions->dwParaAttrs, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PRINTERNAME])
		if ((seResult = DASetOption(hDoc, SCCOPT_XML_PAGEML_PRINTERNAME, &pOptions->szPrinterName, (VTDWORD)strlen(pOptions->szPrinterName) + 1)) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_OFFSETTRACKED])
		if ((seResult = DASetOption(hDoc, SCCOPT_XML_SEARCHML_OFFSET, (VTLPVOID)&pOptions->bOffsetTracked, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSPRINTDIRECTION])
		if ((seResult = DASetOption(hDoc, SCCOPT_SSPRINTDIRECTION, &pOptions->dwSSDirection, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSPRINTGRIDLINES])
		if ((seResult = DASetOption(hDoc, SCCOPT_SSPRINTGRIDLINES, &pOptions->bSSShowGridlines, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSPRINTHEADINGS])
		if ((seResult = DASetOption(hDoc, SCCOPT_SSPRINTHEADINGS, &pOptions->bSSShowHeadings, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSPRINTSCALEPERCENT])
		if ((seResult = DASetOption(hDoc, SCCOPT_SSPRINTSCALEPERCENT, &pOptions->dwSSScalePercent, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSPRINTSCALEXWIDE])
		if ((seResult = DASetOption(hDoc, SCCOPT_SSPRINTSCALEXWIDE, &pOptions->dwSSScaleXWide, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSPRINTSCALEXHIGH])
		if ((seResult = DASetOption(hDoc, SCCOPT_SSPRINTSCALEXHIGH, &pOptions->dwSSScaleXHigh, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_DBPRINTGRIDLINES])
		if ((seResult = DASetOption(hDoc, SCCOPT_DBPRINTGRIDLINES, &pOptions->bDBShowGridlines, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_DBPRINTHEADINGS])
		if ((seResult = DASetOption(hDoc, SCCOPT_DBPRINTHEADINGS, &pOptions->bDBShowHeadings, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_VECPRINTBACKGROUND])
		if ((seResult = DASetOption(hDoc, SCCOPT_VECPRINTBACKGROUND, &pOptions->bVecShowBackground, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSPRINTFITTOPAGE])
		if ((seResult = DASetOption(hDoc, SCCOPT_SSPRINTFITTOPAGE, &pOptions->dwSSFitToPage, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_RENDERING_PREFER_OIT])
		if ((seResult = DASetOption(hDoc, SCCOPT_RENDERING_PREFER_OIT, &pOptions->bOutputSolution, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_RENDER_DISABLEALPHABLENDING])
		if ((seResult = DASetOption(hDoc, SCCOPT_RENDER_DISABLEALPHABLENDING, &pOptions->bRenderDisableAlphaBlending, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_RENDER_ENABLEALPHABLENDING])
		if ((seResult = DASetOption(hDoc, SCCOPT_RENDER_ENABLEALPHABLENDING, &pOptions->bRenderEnableAlphaBlending, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_VECPRINTASPECT])
		if ((seResult = DASetOption(hDoc, SCCOPT_VECPRINTASPECT, &pOptions->dwVecAspect, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_WHATTOPRINT])
		if ((seResult = DASetOption(hDoc, SCCOPT_WHATTOPRINT, &pOptions->dwWhatToExport, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_DBPRINTFITTOPAGE])
		if ((seResult = DASetOption(hDoc, SCCOPT_DBPRINTFITTOPAGE, &pOptions->dwDBFitToPage, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_TIFFBYTEORDER] || pOptions->abSetOptions[SAMPLE_OPTION_TIFFCOLORSPACE] ||
		pOptions->abSetOptions[SAMPLE_OPTION_TIFFCOMPRESSION] || pOptions->abSetOptions[SAMPLE_OPTION_TIFFMULTIPAGE] ||
		pOptions->abSetOptions[SAMPLE_OPTION_TIFFMSB] || pOptions->abSetOptions[SAMPLE_OPTION_TIFFONESTRIP])
	{
		pOptions->sEXTiffOptions.dwSize = sizeof(EXTIFFOPTIONS);

		if ((seResult = DASetOption(hDoc, SCCOPT_IMAGEX_TIFFOPTIONS, &pOptions->sEXTiffOptions, pOptions->sEXTiffOptions.dwSize)) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_TIFFGRAY2COLOR])
	{
		VTBOOL bGray2Color = ((pOptions->sEXTiffOptions.dwTIFFFlags & SCCGRAPHIC_TIFFFLAGS_GRAY2COLOR) != 0);
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAYSCALE_TO_COLOR, &bGray2Color, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_PRINTMARGINSTOP] || pOptions->abSetOptions[SAMPLE_OPTION_PRINTMARGINSBOTTOM] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PRINTMARGINSLEFT] || pOptions->abSetOptions[SAMPLE_OPTION_PRINTMARGINSRIGHT])
		if ((seResult = DASetOption(hDoc, SCCOPT_DEFAULTPRINTMARGINS, &pOptions->sDefaultMargins, sizeof(SCCVWPRINTMARGINS))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PRINTSTARTPAGE])
		if ((seResult = DASetOption(hDoc, SCCOPT_PRINTSTARTPAGE, &pOptions->dwExportStartPage, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PRINTENDPAGE])
		if ((seResult = DASetOption(hDoc, SCCOPT_PRINTENDPAGE, &pOptions->dwExportEndPage, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_USEDOCPAGESETTINGS])
		if ((seResult = DASetOption(hDoc, SCCOPT_USEDOCPAGESETTINGS, &pOptions->bUseDocPageSettings, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_DEFAULTPRINTFONTFACE] || pOptions->abSetOptions[SAMPLE_OPTION_DEFAULTPRINTFONTHEIGHT] ||
		pOptions->abSetOptions[SAMPLE_OPTION_DEFAULTPRINTFONTATTR] || pOptions->abSetOptions[SAMPLE_OPTION_DEFAULTPRINTFONTTYPE])
	{
        if ((pOptions->DefaultPrintFont.wHeight == 0) && !pOptions->abSetOptions[SAMPLE_OPTION_DEFAULTPRINTFONTHEIGHT])
            pOptions->DefaultPrintFont.wHeight = 20;

		if ((seResult = DASetOption(hDoc, SCCOPT_DEFAULTPRINTFONT, (VTLPVOID)&pOptions->DefaultPrintFont, sizeof(SCCUTFONTSPEC))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_PRINTFONTALIASID] || pOptions->abSetOptions[SAMPLE_OPTION_PRINTFONTALIASFLAGS] ||
		pOptions->abSetOptions[SAMPLE_OPTION_PRINTFONTALIASORIGINAL] || pOptions->abSetOptions[SAMPLE_OPTION_PRINTFONTALIAS])
	{
		PFONTALIAS pFontAlias = &pOptions->FontAlias;
		SCCUTFONTALIAS    sPrintFontAlias;

		sPrintFontAlias.dwFlags = SCCVW_FONTALIAS_REMOVEALL;
		DASetOption(hDoc, SCCOPT_PRINTFONTALIAS, &sPrintFontAlias, sizeof(SCCUTFONTALIAS));

		while (pFontAlias)
		{
			pFontAlias->PrintFontAlias.dwSize = sizeof(SCCUTFONTALIAS);
			if ((seResult = DASetOption(hDoc, SCCOPT_PRINTFONTALIAS, (VTLPVOID)&pFontAlias->PrintFontAlias, sizeof(SCCUTFONTALIAS))) != SCCERR_OK)
				return seResult;

			pFontAlias = pFontAlias->pNextFontAlias;
		}
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_FONTFILTERTYPE] || pOptions->abSetOptions[SAMPLE_OPTION_FONTFILTERLIST])
		if ((seResult = DASetOption(hDoc, SCCOPT_FONTFILTER, (VTLPVOID)&pOptions->FontFilterList, sizeof(FONTFILTERLIST))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_ACCEPT_ALT_GRAPHICS])
		if ((seResult = DASetOption(hDoc, SCCOPT_ACCEPT_ALT_GRAPHICS, (VTLPVOID)&pOptions->bAcceptAltGraphics, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_EXTRACTEMBEDDEDFORMAT])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_EXTRACTEMBEDDEDFILES, (VTLPVOID)&pOptions->dwExtractEmbeddedFormat, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_TRANSPARENCYCOLORRED] || pOptions->abSetOptions[SAMPLE_OPTION_TRANSPARENCYCOLORGREEN] || pOptions->abSetOptions[SAMPLE_OPTION_TRANSPARENCYCOLORBLUE])
	{
		SCCVWCOLORREF rgbTransparent;

		if (pOptions->dwRed <= 0xff && pOptions->dwBlue <= 0xff && pOptions->dwGreen <= 0xff)
		{
			rgbTransparent = SCCVWRGB(pOptions->dwRed, pOptions->dwGreen, pOptions->dwBlue);

			if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_TRANSPARENCYCOLOR, (VTLPVOID)&rgbTransparent, sizeof(SCCVWCOLORREF))) != SCCERR_OK)
				return seResult;
		}
		else
		{
			rgbTransparent = (SCCVWCOLORREF)SCCGRAPHIC_DEFAULTTRANSPARENCYCOLOR;

			if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_TRANSPARENCYCOLOR, (VTLPVOID)&rgbTransparent, sizeof(SCCVWCOLORREF))) != SCCERR_OK)
				return seResult;
		}
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_CROPPING])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_CROPPING, (VTLPVOID)&pOptions->dwImageCropping, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_OPACITY])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_OPACITY, (VTLPVOID)&pOptions->dwImageWatermarkOpacity, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_PATH])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_PATH, pOptions->pWatermarkSpec, pOptions->dwWatermarkSpecSize)) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_SCALETYPE])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_SCALETYPE, (VTLPVOID)&pOptions->dwImageWatermarkScaleType, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_SCALEPERCENT])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_SCALEPERCENT, (VTLPVOID)&pOptions->dwImageWatermarkScalePercent, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_HORIZONTALPOS])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_HORIZONTALPOS, (VTLPVOID)&pOptions->lImageWatermarkHorizontalPos, sizeof(VTLONG))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_VERTICALPOS])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_VERTICALPOS, (VTLPVOID)&pOptions->lImageWatermarkVerticalPos, sizeof(VTLONG))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_HORIZONTAL_OFFSETFROM])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_HORIZONTAL_OFFSETFROM, (VTLPVOID)&pOptions->dwImageWatermarkHorizontalOffsetFrom, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_VERTICAL_OFFSETFROM])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_VERTICAL_OFFSETFROM, (VTLPVOID)&pOptions->dwImageWatermarkVerticalOffsetFrom, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_HORIZONTAL_OFFSET])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_HORIZONTAL_OFFSET, (VTLPVOID)&pOptions->lImageWatermarkHorizontalOffset, sizeof(VTLONG))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_VERTICAL_OFFSET])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_VERTICAL_OFFSET, (VTLPVOID)&pOptions->lImageWatermarkVerticalOffset, sizeof(VTLONG))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_WATERMARK_IOTYPE])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_WATERMARK_IOTYPE, (VTLPVOID)&pOptions->dwImageWatermarkPathType, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_GRAPHIC_RENDERASPAGE])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_GRAPHIC_RENDERASPAGE, (VTLPVOID)&pOptions->bRenderAsPage, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_MAXSSDBPAGEWIDTH])
		if ((seResult = DASetOption(hDoc, SCCOPT_MAXSSDBPAGEWIDTH, &pOptions->dwMaxSSDBPageWidth, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_MAXSSDBPAGEHEIGHT])
		if ((seResult = DASetOption(hDoc, SCCOPT_MAXSSDBPAGEHEIGHT, &pOptions->dwMaxSSDBPageHeight, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_ODGRAPHICOPTIONS])
		if ((seResult = DASetOption(hDoc, SCCOPT_CCOD_GRAPHICOPTIONS, &pOptions->dwODGraphicOptions, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;


	if (pOptions->abSetOptions[SAMPLE_OPTION_TIMEZONE])
		if ((seResult = DASetOption(hDoc, SCCOPT_TIMEZONE, &pOptions->lTimeZone, sizeof(VTLONG))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_DEFAULTINPUTCHARSET])
		if ((seResult = DASetOption(hDoc, SCCOPT_DEFAULTINPUTCHARSET, (VTLPVOID)&pOptions->dwDefaultInputCharset, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_FALLBACKFONT])
		if ((seResult = DASetOption(hDoc, SCCOPT_EX_FALLBACKFONT, (VTLPVOID)&pOptions->FallbackFont, sizeof(VTLPVOID))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_WATERMARK])
		if ((seResult = DASetOption(hDoc, SCCOPT_ENABLEWATERMARK, &pOptions->bEnableWatermark, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_WATERMARKPOS])
		if ((seResult = DASetOption(hDoc, SCCOPT_WATERMARKPOSITION, &pOptions->WatermarkPos, sizeof(WATERMARKPOS))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_WATERMARKIOTYPE])
		if ((seResult = DASetOption(hDoc, SCCOPT_WATERMARKIO, &pOptions->WatermarkIO, sizeof(WATERMARKIO))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_UNITS])
		if ((seResult = DASetOption(hDoc, SCCOPT_DEFAULTPAGESIZE, &pOptions->DefaultPageSize, sizeof(DEFAULTPAGESIZE))) != SCCERR_OK)
			return seResult;

	/*
	 |The SCCOPT_FONTDIRECTORY option must be set with some value
	 |and we are just using the Windows fonts as a sample.
	 |Customers are responsible for determining which fonts can legally be used by their application.
	 */
	if (pOptions->abSetOptions[SAMPLE_OPTION_FONTDIRECTORY] && pOptions->szFontDirectory)
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_FONTDIRECTORY, pOptions->szFontDirectory, (VTDWORD)strlen((VTCHAR *)pOptions->szFontDirectory))) != SCCERR_OK)
			return seResult;
	}
#if 0 //def WIN32
	/*
	*   It has been agreed that the config file parser should not be forcing this.
	*   Also, this value now overrides GDFONTPATH, so forcing it is not a good idea.
	*   jrw  9/17/14
	*/
	else
	{
		DWORD strLen;
		ALLOC(VTCHAR, pOptions->szFontDirectory, MAX_PATH);
		strLen = GetWindowsDirectory(pOptions->szFontDirectory, MAX_PATH);
		if (*(pOptions->szFontDirectory + strLen - 1) == '\\')
			strcat(pOptions->szFontDirectory, "fonts");
		else
			strcat(pOptions->szFontDirectory, "\\fonts");

		if ((seResult = DASetOption(hDoc, SCCOPT_FONTDIRECTORY, pOptions->szFontDirectory, MAX_FONT_PATH_LENGTH)) != SCCERR_OK)
			return seResult;
	}
#endif

	if (pOptions->abSetOptions[SAMPLE_OPTION_EMBEDFONTS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EMBEDFONTS, &pOptions->bEmbedFonts, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_RENDER_EMBEDDED_FONTS])
		if ((seResult = DASetOption(hDoc, SCCOPT_RENDER_EMBEDDED_FONTS, &pOptions->bRenderEmbeddedFonts, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_FONTEMBEDPOLICY])
		if ((seResult = DASetOption(hDoc, SCCOPT_FONTEMBEDPOLICY, &pOptions->dwFontEmbedPolicy, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_DOLINEARIZATION])
		if ((seResult = DASetOption(hDoc, SCCOPT_DOLINEARIZATION, &pOptions->bDoLinearization, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	/*
	if (pOptions->abSetOptions[SAMPLE_OPTION_COMPRESS_24BIT_TYPE])
	if ((seResult = DASetOption(hDoc, SCCOPT_CMP24BIT, &pOptions->dwCompress24BitType, sizeof(VTDWORD))) != SCCERR_OK)
	return seResult;
	*/

	if (pOptions->abSetOptions[SAMPLE_OPTION_EXPORTATTACHMENTS])
		if ((seResult = DASetOption(hDoc, SCCOPT_EXPORTEMAILATTACHMENTS, &pOptions->bExportAttachments, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_IMAGE_PASSTHROUGH])
		if ((seResult = DASetOption(hDoc, SCCOPT_IMAGE_PASSTHROUGH, &pOptions->bDirectImagePassThrough, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_UNRESTRICTEDFONTEMBEDDING])
		if ((seResult = DASetOption(hDoc, SCCOPT_UNRESTRICTEDFONTEMBEDDING, &pOptions->bUnrestrictedFontEmbedding, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_EXTRACTXMPMETADATA])
		if ((seResult = DASetOption(hDoc, SCCOPT_EXTRACTXMPMETADATA, (VTLPVOID)&pOptions->bExtractXMPMetadata, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PARSEXMPMETADATA])
		if ((seResult = DASetOption(hDoc, SCCOPT_PARSEXMPMETADATA, (VTLPVOID)&pOptions->bParseXMPMetadata, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;
	if (pOptions->abSetOptions[SAMPLE_OPTION_OCRQUALITY])
	{
		seResult = DASetOption(hDoc, SCCOPT_OCR_QUALITY, &pOptions->dwOcrQuality, sizeof(VTDWORD));
		if (seResult != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_OCRTECH])
	{
		seResult = DASetOption(hDoc, SCCOPT_OCR_TECH, &pOptions->dwOcrTech, sizeof(VTDWORD));
		if (seResult != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_LINEARUNITS])
	{
		seResult = DASetOption(hDoc, SCCOPT_CCFLEX_UNITS, &pOptions->dwLinearUnits, sizeof(VTDWORD));
		if (seResult != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_DOCUMENTMEMORYMODE])
		if ((seResult = DASetOption(hDoc, SCCOPT_DOCUMENTMEMORYMODE, (VTLPVOID)&pOptions->dwDocumentMemoryMode, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_OLEEMBEDDINGS])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_PROCESS_OLE_EMBEDDINGS, (VTLPVOID)&pOptions->wOLEEmbeddingMode, sizeof(VTWORD))) != SCCERR_OK)
			return seResult;
	}


	if (pOptions->abSetOptions[SAMPLE_OPTION_REORDERMETHOD])
		if ((seResult = DASetOption(hDoc, SCCOPT_REORDERMETHOD, (VTLPVOID)&pOptions->dwReorderMethod, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_LOTUSNOTESDIRECTORY])
	{
		if ((seResult = DASetOption(hDoc, SCCOPT_LOTUSNOTESDIRECTORY, pOptions->szLotusNotesDirectory, VT_MAX_FILEPATH)) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_EMAILHEADER])
		if ((seResult = DASetOption(hDoc, SCCOPT_WPEMAILHEADEROUTPUT, (VTLPVOID)&pOptions->dwEmailHeader, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;
	if (pOptions->abSetOptions[SAMPLE_OPTION_MIMEHEADER])
		if ((seResult = DASetOption(hDoc, SCCOPT_WPEMAILHEADEROUTPUT, (VTLPVOID)&pOptions->dwEmailHeader, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SSSHOWHIDDENCELLS])
		if ((seResult = DASetOption(hDoc, SCCOPT_SSSHOWHIDDENCELLS, (VTLPVOID)&pOptions->bSSShowHiddenCells, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_UNIXOPTIONSFILE])
	{
		VTDWORD dwSize = sizeof(SCCUTUNIXOPTIONSFILEINFO);
		if ((seResult = DAGetOption(hDoc, SCCOPT_GETUNIXOPTIONSFILENAME, (VTLPVOID)&pOptions->sUnixOptionsFileInfo, &dwSize)) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_HTML_COND_COMMENT_MODE])
		if ((seResult = DASetOption(hDoc, SCCOPT_HTML_COND_COMMENT_MODE, (VTLPVOID)&pOptions->dwHtmlCondCommentMode, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_MAILHEADERVISIBLE])
	{
		PEMAILHEADERINFO pInfo = pOptions->sMailHeader.pHeadVis;

		while (pInfo)
		{
			if ((seResult = DASetOption(hDoc, SCCOPT_MAILHEADERVISIBLE, (VTLPVOID)&pInfo->sHeader, sizeof(VTLPVOID))) != SCCERR_OK)
				return seResult;
			pInfo = pInfo->next;
		}
		/*  This lead to a memeory leak as we check it against NULL before freeing at the end.  14780876  */
		// pOptions->sMailHeader.pHeadVis = pOptions->sMailHeader.pCurVis = NULL;
		pOptions->sMailHeader.pCurVis = NULL;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_MAILHEADERHIDDEN])
	{
		PEMAILHEADERINFO pInfo = pOptions->sMailHeader.pHeadHid;

		while (pInfo)
		{
			if ((seResult = DASetOption(hDoc, SCCOPT_MAILHEADERHIDDEN, (VTLPVOID)&pInfo->sHeader, sizeof(VTLPVOID))) != SCCERR_OK)
				return seResult;
			pInfo = pInfo->next;
		}
		/*  This lead to a memeory leak as we check it against NULL before freeing at the end.  14780876  */
		//  pOptions->sMailHeader.pHeadHid = pOptions->sMailHeader.pCurHid = NULL;
		pOptions->sMailHeader.pCurHid = NULL;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_IGNOREPASSWORD])
		if ((seResult = DASetOption(hDoc, SCCOPT_IGNORE_PASSWORD, &pOptions->bIgnorePassword, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_STROKETEXT])
		if ((seResult = DASetOption(hDoc, SCCOPT_STROKE_TEXT, &pOptions->bStrokeText, sizeof(pOptions->bStrokeText))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_GENERATE_EXCEL_REVISIONS])
		if ((seResult = DASetOption(hDoc, SCCOPT_GENERATEEXCELREVISIONS, &pOptions->bGenerateExcelRevisions, sizeof(pOptions->bGenerateExcelRevisions))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_OUTPUT_STRUCTURE])
		if ((seResult = DASetOption(hDoc, SCCOPT_OUTPUT_STRUCTURE, (VTLPVOID)&pOptions->dwOutputStructure, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_RAWTEXT])
		if ((seResult = DASetOption(hDoc, SCCOPT_OUTPUT_RAWTEXT, (VTLPVOID)&pOptions->bRawText, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_URLPATH_RESOURCES])
		if ((seResult = DASetOption(hDoc, SCCOPT_URLPATH_RESOURCES, (VTLPVOID)pOptions->pszResourceURL, (VTDWORD)strlen((VTCHAR *)pOptions->pszResourceURL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_URLPATH_OUTPUT])
		if ((seResult = DASetOption(hDoc, SCCOPT_URLPATH_OUTPUT, (VTLPVOID)pOptions->pszOutputURL, (VTDWORD)strlen((VTCHAR *)pOptions->pszOutputURL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_EXTERNAL_CSS])
	{
		if (pOptions->ppExternalCss)
		{
			VTDWORD i;
			for (i = 0; i < pOptions->dwCountExternalCss; ++i)
			{
				if ((seResult = DASetOption(hDoc, SCCOPT_EXTERNAL_STYLESHEET,
					(VTLPVOID)pOptions->ppExternalCss[i], (VTDWORD)strlen((VTCHAR *)pOptions->ppExternalCss[i]))) != SCCERR_OK)
					return seResult;
			}
		}
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_POST_LIBRARY_SCRIPT])
	{
		if (pOptions->ppPostLibScript)
		{
			VTDWORD i;
			for (i = 0; i < pOptions->dwCountPostLibScript; ++i)
			{
				if ((seResult = DASetOption(hDoc, SCCOPT_POST_LIBRARY_SCRIPT,
					(VTLPVOID)pOptions->ppPostLibScript[i], (VTDWORD)strlen((VTCHAR *)pOptions->ppPostLibScript[i]))) != SCCERR_OK)
					return seResult;
			}
		}
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_PRE_LIBRARY_SCRIPT])
	{
		if (pOptions->ppPreLibScript)
		{
			VTDWORD i;
			for (i = 0; i < pOptions->dwCountPreLibScript; ++i)
			{
				if ((seResult = DASetOption(hDoc, SCCOPT_PRE_LIBRARY_SCRIPT,
					(VTLPVOID)pOptions->ppPreLibScript[i], (VTDWORD)strlen((VTCHAR *)pOptions->ppPreLibScript[i]))) != SCCERR_OK)
					return seResult;
			}
		}
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_IMAGESTAMP_FILE])
	{
		if (pOptions->ppImageStampFile)
		{
			VTDWORD i;
			for (i = 0; i < pOptions->dwCountImageStampFile; ++i)
			{
				if ((seResult = DASetOption(hDoc, SCCOPT_STAMP_IMAGE_FILE,
					(VTLPVOID)pOptions->ppImageStampFile[i], sizeof(EXANNOSTAMPIMAGE))) != SCCERR_OK)
					return seResult;
			}
		}
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_IMAGESTAMP_URL])
	{
		if (pOptions->ppImageStampUrl)
		{
			VTDWORD i;
			for (i = 0; i < pOptions->dwCountImageStampUrl; ++i)
			{
				if ((seResult = DASetOption(hDoc, SCCOPT_STAMP_IMAGE_URL,
					(VTLPVOID)pOptions->ppImageStampUrl[i], sizeof(EXANNOSTAMPIMAGE))) != SCCERR_OK)
					return seResult;
			}
		}
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_FONTBASEURL])
		if ((seResult = DASetOption(hDoc, SCCOPT_FONT_BASE_URL, (VTLPVOID)pOptions->pszBaseURL, (VTDWORD)strlen((VTCHAR *)pOptions->pszBaseURL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_WVLIBRARYNAME])
		if ((seResult = DASetOption(hDoc, SCCOPT_WV_LIBRARY_NAME, (VTLPVOID)pOptions->pszLibraryName, (VTDWORD)strlen((VTCHAR *)pOptions->pszLibraryName))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_WVSTYLESHEETNAME])
		if ((seResult = DASetOption(hDoc, SCCOPT_WV_STYLESHEET_NAME, (VTLPVOID)pOptions->pszStylesheetName, (VTDWORD)strlen((VTCHAR *)pOptions->pszStylesheetName))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_FONTPERMISSIONS])
		if ((seResult = DASetOption(hDoc, SCCOPT_FONT_PERMISSIONS_MODE, (VTLPVOID)&pOptions->dwFontPermissions, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_FONTREFERENCEMETHOD])
		if ((seResult = DASetOption(hDoc, SCCOPT_FONT_REFERENCE_METHOD, (VTLPVOID)&pOptions->dwFontReferenceMethod, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_ATTACHMENTHANDLING])
		if ((seResult = DASetOption(hDoc, SCCOPT_EMAIL_ATTACHMENT_HANDLING, (VTLPVOID)&pOptions->dwEmailAttachmentHandling, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_REDACTIONCOLOR])
		if ((seResult = DASetOption(hDoc, SCCOPT_REDACTION_COLOR, (VTLPVOID)&pOptions->dwRedactionColor, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_SHOWREDACTIONLABELS])
		if ((seResult = DASetOption(hDoc, SCCOPT_SHOW_REDACTION_LABELS, (VTLPVOID)&pOptions->bShowRedactionLabels, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_REDACTIONS_ENABLED])
		if ((seResult = DASetOption(hDoc, SCCOPT_REDACTIONS_ENABLED, (VTLPVOID)&pOptions->bEnableRedactions, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_REDACTION_LABEL_FONT_NAME])
	{
		VTDWORD lnLen = 0;
		VTWORD *tmp = pOptions->pwzRedactionLabelFontName;
		while (*tmp)
		{
			lnLen += 2;
			++tmp;
		}
		if ((seResult = DASetOption(hDoc, SCCOPT_REDACTION_LABEL_FONT_NAME, (VTLPVOID)pOptions->pwzRedactionLabelFontName, lnLen)) != SCCERR_OK)
			return seResult;
	}

	if (pOptions->abSetOptions[SAMPLE_OPTION_VECTOROBJECTLIMIT])
		if ((seResult = DASetOption(hDoc, SCCOPT_VECTOROBJECTLIMIT, (VTLPVOID)&pOptions->dwVectorObjectLimit, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PDF_FILTER_WORD_DELIM_FRACTION])
		if ((seResult = DASetOption(hDoc, SCCOPT_PDF_FILTER_WORD_DELIM_FRACTION, (VTLPVOID)&pOptions->fPdfWordDelimFraction, sizeof(VTFLOAT))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PDF_FILTER_MAX_EMBEDDED_OBJECTS])
		if ((seResult = DASetOption(hDoc, SCCOPT_PDF_FILTER_MAX_EMBEDDED_OBJECTS, (VTLPVOID)&pOptions->dwPdfMaxEmbeddedObjects, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PDF_FILTER_MAX_VECTOR_PATHS])
		if ((seResult = DASetOption(hDoc, SCCOPT_PDF_FILTER_MAX_VECTOR_PATHS, (VTLPVOID)&pOptions->dwPdfMaxVectorPaths, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_EMAIL_FIXEDWIDTH])
		if ((seResult = DASetOption(hDoc, SCCOPT_EMAIL_FIXEDWIDTH, (VTLPVOID)&pOptions->bEmailFixedWidth, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_HTML_FIXEDWIDTH])
		if ((seResult = DASetOption(hDoc, SCCOPT_HTML_FIXEDWIDTH, (VTLPVOID)&pOptions->bHTMLFixedWidth, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PLAINTEXT_PAGINATION])
		if ((seResult = DASetOption(hDoc, SCCOPT_PLAINTEXT_PAGINATION, (VTLPVOID)&pOptions->bPlainTextPagination, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_PAGES_PER_FILE])
		if ((seResult = DASetOption(hDoc, SCCOPT_PAGES_PER_FILE, (VTLPVOID)&pOptions->dwPagesPerFile, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_RECIPIENT_DELIVERY_INFORMATION])
		if ((seResult = DASetOption(hDoc, SCCOPT_READ_RECIPIENT_DELIVERY_INFO, (VTLPVOID)&pOptions->dExtractRecipientDeliveryInfo, sizeof(VTDWORD))) != SCCERR_OK)
			return seResult;

	if (pOptions->abSetOptions[SAMPLE_OPTION_BACKGROUND_COLOR])
		if ((seResult = DASetOption(hDoc, SCCOPT_FLAG_BACKGROUND_COLOR, (VTLPVOID)&pOptions->bBackgroundColor, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	/*tvarshne Pdf Space Improvement Plan-1 11/28/2020 */
	if (pOptions->abSetOptions[SAMPLE_OPTION_PDF_FILTER_NEW_SPACING_ALGORITHM])
		if ((seResult = DASetOption(hDoc, SCCOPT_PDF_FILTER_NEW_SPACING_ALGORITHM, (VTLPVOID)&pOptions->bNewPDFspacing, sizeof(VTBOOL))) != SCCERR_OK)
			return seResult;

	return SCCERR_OK;
}




/*
|
|   function:   ReadCommandLine
|   parameters: int     argc
|               char    *argv[]
|               Option  *pOptions
|   returns:    SCCERR
|
|   purpose:    This function reads the command line arguments.
|
|
*/
SCCERR ReadCommandLine(int argc, char *argv[], Option *pOptions, FILE **pFile)
{
	VTLPSTR pstrFile;
	SCCERR seResult = SCCERR_OK;

	if ((argc < 3) || (argc > 5))
	{
		seResult = SCCERR_BADPARAM;

#ifndef DISABLE_REPORTING
		fprintf(stderr, "%s - Converts the input file to a given output type.\n", argv[0]);
		fprintf(stderr, "Usage:\t%s InputFile OutputFile [ConfigurationFile] [-p].\n", argv[0]);
		fprintf(stderr, "Where:\t\"InputFile\" is the file to be converted.\n");
		fprintf(stderr, "\t\"OutputFile\" is where to place the converted file.\n");
		fprintf(stderr, "\t\"ConfigurationFile\" contains the option settings to be used\n");
		fprintf(stderr, "\tfor the conversion.  If a configuration file is not specified,\n");
		fprintf(stderr, "\tthe file \"%s\" in the current directory is used.\n", DEFAULT_CONFIG_FILE);
		fprintf(stderr, "\t -p prints the value of each option as specified in the config file.\n");
#endif
	}
	else
	{
		/* Save the input and output file names. */
		ALLOC(VTCHAR, pOptions->szInFile, strlen(argv[1]) + 1);
		ALLOC(VTCHAR, pOptions->szOutFile, strlen(argv[2]) + 1);
		strcpy(pOptions->szInFile, argv[1]);
		strcpy(pOptions->szOutFile, argv[2]);

		/* Open the configuration file. */
		pstrFile = (VTLPSTR)((argc == 4 || argc == 5) ? argv[3] : DEFAULT_CONFIG_FILE);

#if defined(WIN32) && !defined(DISABLE_REPORTING)
		if ((strlen(pstrFile) + 1) >= VT_MAX_FILEPATH)
		{
			fprintf(stderr, "Warning: The configuration pathname is very long - it may not be accessible\n");
		}
#endif

		/* Setting the print option */
		pOptions->bPrintOpts = FALSE;

		if (argc == 5)
		{
			if ((strcmp(argv[4], "-p") == 0) || (strcmp(argv[4], "-P") == 0))
				pOptions->bPrintOpts = TRUE;
		}

		if ((*pFile = fopen(pstrFile, "r")) == (FILE*)NULL)
		{
#ifndef DISABLE_REPORTING
			fprintf(stderr, "Error: Could not open configuration file %s\n", pstrFile);
#endif
			seResult = SCCERR_BADPARAM;
		}
	}

	return seResult;
}




/*
|
|   function:   ParseConfigFile
|   parameters: FILE    *fp
|               Option  *pOptions
|   returns:    VTVOID
|
|   purpose:    This is the part of ReadConfiguration that actually gets the
|               options from the config file, if one is specified.
|
*/
SCCERR ParseConfigFile(FILE *fp, Option *pOptions)
{
	static VTLPCSTR pstrTest = "ba";
	VTDWORD dwtest = 0;
	int     i;
	VTDWORD dwLine;                         /* Count of lines read from the file.           */
	char    szBuf[BUF_SIZE];                /* A single line from the config file.          */
	char    *pstrValue;                     /* Value of the option being set.               */
	VTDWORD dwOptCode;                      /* The option code.                             */
	VTBOOL  bSeparator;                     /* TRUE once we hit the token separator         */
	SCCERR  seResult = SCCERR_OK;

	memset(pOptions->abSetOptions, 0, sizeof(pOptions->abSetOptions));
	memset(&pOptions->sEXTiffOptions, 0, sizeof(EXTIFFOPTIONS));

	/* Determine what the byte ordering of the current platform is. */
	/* Modified to accomodate alignment-sensitive CPUs  Bug 18325336  jrw 4/28/14 */
	memcpy((char *)&dwtest, pstrTest, 2);
	ConvFunc = (dwtest == 0x6162) ? ConvertASCIIToLittleUnicode : ConvertASCIIToBigUnicode;

	pOptions->dwCallbackFlags = 0xFFFFFFFF; /* all callbacks are enabled by default. */
	pOptions->dwFontFlags = 0;              /* all font attributes are enabled by default. */
	pOptions->dwSearchMLFlags = 0;          /* all flags are disabled by default. */
	pOptions->dwGenFlags = 0;          /* all flags are disabled by default. */
	pOptions->dwCCFlexFormatOptions = CCFLEX_FORMATOPTIONS_REMOVEFONTGROUPS;    /* all other flags are disabled by default. */

	/* Read in the configuration file one line at a time. */
	for (dwLine = 1; fgets(szBuf, BUF_SIZE, fp) != NULL; dwLine++)
	{
		if (*szBuf == '#')    /* Skip comments. */
			continue;

		/*
		|   Each line consists of an option and a value separated by a
		|   whitespace character.  Note that the value may contain any character.
		|   Start by looking for that new line at the end of the line.
		|   Replace it and any white space that preceeds it with NULLs.
		*/
		for (i = 0; szBuf[i] != '\0'; i++)
		{
			if (szBuf[i] == '\n')
			{
				/* Found the end of the line. */
				szBuf[i--] = '\0';

				if ((i >= 0) && (szBuf[i] == '\r'))
				{
					/* Allow DOS style \r\n at EOL */
					szBuf[i--] = '\0';
				}

				/* Throw away any whitespace at EOL */
				for (; (i >= 0) && isspace(szBuf[i]); i--);

				szBuf[i + 1] = '\0';
				break;
			}
		}

		if (*szBuf == '\0')          /* Skip blank lines */
			continue;

		/* Now NULL terminate the option and find its value. */
		for (pstrValue = NULL, bSeparator = FALSE, i = 1; szBuf[i] != '\0'; i++)
		{
			if (isspace(szBuf[i]))
			{
				bSeparator = TRUE;
				szBuf[i] = '\0';
			}
			else if (bSeparator)
			{
				pstrValue = &szBuf[i];
				break;
			}
		}

		/* Convert the option to one of the SAMPLE_OPTION #defines */
		if (StringToDWEx(Options, szBuf, &dwOptCode, (char *)"option", dwLine) != SCCERR_OK)
			continue;

		if ((pstrValue == NULL) && (dwOptCode != SAMPLE_OPTION_FALLBACKFONT))
		{
			/* Never found the value for the option. */
#ifndef DISABLE_REPORTING
			fprintf(stderr, "Warning: line %d: Option missing value.\n", dwLine);
#endif
			continue;
		}

		seResult = StoreOption(szBuf, pstrValue, dwOptCode, dwLine, pOptions);
		if (seResult != SCCERR_OK)
			break;
	}

#ifndef DISABLE_REPORTING
	if (!feof(fp))
	{
		if (seResult != SCCERR_OK)
			fprintf(stderr, "Bad parameter in configuration file line %d.\n", dwLine);
		else
			fprintf(stderr, "Warning: line %d: Failed to read entire configuration file.\n", dwLine);
	}
#endif

	fclose(fp);
	return seResult;
}




SCCERR StoreOption(VTLPSTR keyword, VTLPSTR pstrValue, VTDWORD dwOptCode, VTDWORD dwLine, Option *pOptions)
{
	VTLONG      lValue = 0;
	VTDWORD   dwValue, dwSize;
	int       nLength, i;
	SCCERR    seResult = SCCERR_OK;
	UNUSED(keyword);


	switch (dwOptCode)
	{
		case SAMPLE_OPTION_OBJECTALL:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"extract all objects", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;
			pOptions->bAllObjects = (VTBOOL)dwValue;
			break;

		case SAMPLE_OPTION_INPUTPATH:
			ALLOC(VTCHAR, pOptions->szInFile, strlen(pstrValue) + 1);
			strcpy(pOptions->szInFile, pstrValue);
			if (pOptions->bPrintOpts)
				printf("Input Path:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_INPUTPATH_U:
			/* dsn bug 14841342 - the path is base64 encoded, so not yet a Unicode string. */
			dwSize = (VTDWORD)(strlen(pstrValue) + 1) * sizeof(VTWCHAR);
			ALLOC(VTWORD, pOptions->wzInFile, dwSize);
			if (decodeUnicode((VTLPCWSTR)pstrValue, pOptions->wzInFile, dwSize, &dwSize) != BASE64_OK)
				return seResult;
			break;

		case SAMPLE_OPTION_OUTPUTPATH:
		case SAMPLE_OPTION_OUTPUTPATH_U:
		{
			if (dwOptCode == SAMPLE_OPTION_OUTPUTPATH)
			{
				VTDWORD dwLen = (VTDWORD)strlen(pstrValue) + 1;
				ALLOC(VTCHAR, pOptions->szOutFile, dwLen);
				strcpy(pOptions->szOutFile, pstrValue);
				ALLOC(VTBYTE, g_szInitFilePath, dwLen);
				strcpy((char*)g_szInitFilePath, pstrValue);
			}
			else
			{
				dwSize = (Swcslen((VTLPCWSTR)pstrValue) + 1) * 4 * sizeof(VTWORD);   /* give it plenty of room because the output string can be bigger */

				ALLOC(VTWORD, pOptions->wzOutFile, dwSize);
				if (decodeUnicode((VTLPCWSTR)pstrValue, pOptions->wzOutFile, dwSize, &dwSize) != BASE64_OK)
					return SCCERR_INSUFFICIENTBUFFER;   /* BASE64_BUFFERTOOSMALL is the only other code which can be returned */

				dwSize = Swcslen(pOptions->wzOutFile);
				ALLOC(VTBYTE, g_szInitFilePath, dwSize + 1);
				UnicodeToAsciiFunc(g_szInitFilePath, dwSize + 1, pOptions->wzOutFile, dwSize * sizeof(VTWORD));
			}

			dwSize = (VTDWORD)strlen((char*)g_szInitFilePath);

			if (dwSize > 0)
			{
				VTLPBYTE ptr = g_szInitFilePath;
				ptr += dwSize - 1;

				while (ptr != g_szInitFilePath && *ptr != '\\' && *ptr != '/')
					ptr--;

				if (ptr != g_szInitFilePath)
					++ptr;

				ALLOC(VTBYTE, g_szInitFileName, strlen((char *)ptr) + 1);
				strcpy((char*)g_szInitFileName, (char*)ptr);
				*ptr = '\0';

				ptr = g_szInitFileName;
				ptr += strlen((char*)g_szInitFileName) - 1;

				while (ptr != g_szInitFileName && *ptr != '.')
					--ptr;

				if (ptr != g_szInitFileName)
					*ptr = '\0';
			}
			break;
		}

		case SAMPLE_OPTION_OUTPUTID:
			seResult = StringToDWEx(OutputIDs, pstrValue, &pOptions->dwOutputID, (char *)"output ID", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;
			if (pOptions->bPrintOpts)
				printf("Output ID:\t\t\t%s\n", pstrValue);

			break;

		case SAMPLE_OPTION_TEMPLATE:
			ALLOC(VTCHAR, pOptions->szTemplate, strlen(pstrValue) + 1);
			strcpy(pOptions->szTemplate, pstrValue);
			if (pOptions->bPrintOpts)
				printf("Template:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TEMPLATE_U:
			{
				dwSize = (Swcslen((VTLPCWSTR)pstrValue) + 1) * sizeof(VTWCHAR);
				ALLOC(VTWORD, pOptions->wzTemplate, dwSize);
				if (decodeUnicode((VTLPCWSTR)pstrValue, pOptions->wzTemplate, dwSize, &dwSize) != BASE64_OK)
					return seResult;
			}
			break;

		case SAMPLE_OPTION_FLAVOR:
			seResult = StringToDWEx(Flavors, pstrValue, &pOptions->dwFlavor, (char *)"flavor", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Flavor:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICTYPE:
			seResult = StringToDWEx(GraphicTypes, pstrValue, &pOptions->dwGraphicType, (char *)"output graphics type", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Graphic Type:\t\t\t%s\n", pstrValue);

			break;

		case SAMPLE_OPTION_INTERLACEDGIF:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"interlaced GIF setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bInterlacedGIF = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Interlaced GIF:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_QUICKTHUMBNAIL:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"quick thumbnail setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bQuickThumbnail = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("QuickThumbnail:\t\t\t%s\n", pstrValue);
			break;


		case SAMPLE_OPTION_COMPRESS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Apply Filter", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;
			pOptions->bCompress = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Apply Filter:\t\t\t%s\n", pstrValue);

			break;

		case SAMPLE_OPTION_ISODATETIMES:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Use ISO 8601 for date/time formatting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwGenFlags |= SCCOPT_FLAGS_ISODATETIMES;
			else
				pOptions->dwGenFlags &= ~SCCOPT_FLAGS_ISODATETIMES;
			if (pOptions->bPrintOpts)
				printf("Use ISO 8601 for date/time formatting:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_STRICT_DTD:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"strict DTD compliance setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
			{
				pOptions->dwFlags |= SCCEX_CFLAG_STRICTDTD;
			}
			if (pOptions->bPrintOpts)
				printf("Strict DTD:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WELLFORMED:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"well-formed HTML setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwFlags |= SCCEX_CFLAG_WELLFORMED;
			if (pOptions->bPrintOpts)
				printf("Wellformed:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICSIZEMETHOD:
			seResult = StringToDWEx(SizingOpts, pstrValue, &pOptions->dwSizeMethod, (char *)"graphic sizing method", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Graphic size method:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_JPEGCOMPRESSION:
			seResult = StringToDWEx(JpegCompression, pstrValue, &pOptions->dwJpegCompression, (char *)"set jpg compression on or off", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Jpeg Compression:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_LZWCOMPRESSION:
			seResult = StringToDWEx(LzwCompression, pstrValue, &pOptions->dwLzwCompression, (char *)"set lzw compression on or off", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("LZW Compression:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_JPEGQUALITY:
			if ((sscanf(pstrValue, "%d", &pOptions->dwJPEGQuality) != 1) ||
				(pOptions->dwJPEGQuality < 1) || (pOptions->dwJPEGQuality > 100))
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid JPEG quality value\n", dwLine);
				fprintf(stderr, "Valid JPEG quality values are 1 - 100\n");
				fprintf(stderr, "100 = Highest quality and lowest compression\n");
				fprintf(stderr, "  1 = Lowest quality and highest compression\n");
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Jpeg Quality:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARSET:
			seResult = StringToDWEx(Charsets, pstrValue, &pOptions->dwCharset, (char *)"output character set", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Character Set:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARBYTEORDER:
			seResult = StringToDWEx(CharByteOrder, pstrValue, &pOptions->dwCharByteOrder, (char *)"character byte order", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Character Byte Order:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FALLBACKFORMAT:
			seResult = StringToDWEx(FallbackFormat, pstrValue, &pOptions->dwFallbackFormat, (char *)"fallback format", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Fallback Format:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FIFLAGS:
			seResult = StringToDWEx(FIFlags, pstrValue, &pOptions->dwFIFlags, (char *)"FI flags", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("FI Flags:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_NOSOURCEFORMATTING:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"no source formatting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bNoSourceFormatting = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("No Source Formatting:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_COLLAPSEWHITESPACE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"collapse whitespace", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bCollapseWhitespace = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Collapse White Space:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_JAVASCRIPTTABS:
			StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"javascript tabs", dwLine);
			pOptions->bJSTabs = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Java Script:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SHOWHIDDENTEXT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show hidden text", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bShowHiddenText = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Show Hidden Text:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SHOWHIDDENSSDATA:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show spread sheet hidden columns/rows/sheets", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bShowHiddenSSData = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Show Hidden SS Data:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FILTERNOBLANK:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"produce (or not) blank spreadsheet pages", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bFilterNoBlank = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Filter no blank::\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SHOWCHANGETRACKING:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show change tracking", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bShowChangeTracking = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Show Change Tracking:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SHOWSSDBROWCOLHEADINGS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show SS/DB row and column headings", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bShowSSDBRowColHeadings = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Show SS/DB row and column headings:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SSDBBORDEROPTIONS:
			seResult = StringToDWEx(SSDBBorderOptions, pstrValue, &pOptions->dwSSDBBorderOptions, (char *)"SS/DB border options", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("SS/DB border options:\t\t%s\n", pstrValue);
			break;

			/* PHTML */
		case SAMPLE_OPTION_SUPPRESSMETADATA:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"suppress metadata", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bSuppressMetadata = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Suppress metadata:\t\t%s\n", pstrValue);
			break;
			/**/

		case SAMPLE_OPTION_PERFORMANCEMODE:
			seResult = StringToDWEx(PerformanceMode, pstrValue, &dwValue, (char *)"performance mode", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwPerformanceMode = dwValue;
			if (pOptions->bPrintOpts)
				printf("Performance mode:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SEPARATEGRAPHICSBUFFER:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"separate graphics buffer", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bSeparateGraphicsBuffer = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Separate Graphics Buffer:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICOUTPUTDPI:
			if ((sscanf(pstrValue, "%d", &pOptions->dwGraphicOutputDPI) != 1) ||
				(pOptions->dwGraphicOutputDPI > SCCGRAPHIC_MAX_SANE_BITMAP_DPI))
			{
#if !defined(EX_EXPORTER) && !defined(DISABLE_REPORTING)
				fprintf(stderr, "Error, line %d: Invalid output device DPI value (%s)\n", dwLine, pstrValue);
				fprintf(stderr, "Valid DPI values are 0 thru %d pixels per inch.\n", SCCGRAPHIC_MAX_SANE_BITMAP_DPI);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Graphic Output DPI:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FALLBACKFONT:
			if (pstrValue == NULL)
			{
				pOptions->szFallbackFontName[0] = '\0';
			}
			else
			{
				strncpy(pOptions->szFallbackFontName, pstrValue, SCCUT_MAXFALLBACKFONTLEN * 2);
				pOptions->szFallbackFontName[SCCUT_MAXFALLBACKFONTLEN * 2 - 1] = '\0';
			}
			pOptions->FallbackFont.pName = pOptions->szFallbackFontName;
			pOptions->FallbackFont.wType = SCCEX_FALLBACKFONT_SINGLEBYTE;
			if (pOptions->bPrintOpts)
				printf("Fallback Font:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PREVENTGRAPHICOVERLAP:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"prevent graphics overlap", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bPreventGraphicOverlap = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Prevent Graphic Overlap:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SIMPLESTYLENAMES:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"simple style names", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bSimpleStyleNames = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Simple Style Names:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GENBULLETSANDNUMS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"generate bullets and numbers for lists", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bGenBulletsAndNums = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Generate Bullets and Numbers:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICSIZELIMIT:
			if (sscanf(pstrValue, "%d", &pOptions->dwGraphicSizeLimit) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid graphic size limit (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Graphic Size Limit:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICWIDTH:
			if (sscanf(pstrValue, "%d", &pOptions->dwGraphicWidth) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid graphic width (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Graphic Width:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICHEIGHT:
			if (sscanf(pstrValue, "%d", &pOptions->dwGraphicHeight) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid graphic height (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Graphic Height:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICWIDTHLIMIT:
			if (sscanf(pstrValue, "%d", &pOptions->dwGraphicWidthLimit) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid graphic width limit (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Graphic Width Limit:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICHEIGHTLIMIT:
			if (sscanf(pstrValue, "%d", &pOptions->dwGraphicHeightLimit) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid graphic height limit (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Graphic Height Limit:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_LABELWPCELLS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"label word-processing table cells", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwLabelFlags |= SCCEX_LABELFLAGS_WPCELLSON;
			if (pOptions->bPrintOpts)
				printf("Label WP Cells:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_LABELSSDBCELLS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"label spreadsheet and database cells", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 0)
				pOptions->dwLabelFlags |= SCCEX_LABELFLAGS_SSDBCELLSOFF;
			if (pOptions->bPrintOpts)
				printf("Label SSDB Cells:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TEXTBUFFERSIZE:
			if (sscanf(pstrValue, "%d", &pOptions->dwTextBufferSize) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid text buffer size value (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Text Buffer Size:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_UNMAPPABLECHAR:
			if (sscanf(pstrValue, "%hx", &pOptions->wUnmappableChar) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid unmappable char value (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Unmappable Character:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_HANDLENEWFILEINFO:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_NEWFILEINFO", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bHandleNewFileInfo = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Handle New File Info:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_HANDLEENDPAGE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_ENDPAGE", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bHandleEndPage = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Handle End Page:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_ZTOQ:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"map all z's to q's", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bZtoQ = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("ZTOQ:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_REFLINK:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_REFLINK", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bRefLink = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("RefLink:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_OEMSTRING:
			for (i = pOptions->mapOEM.wCount; i < MAP_MAXELEMENTS; i++)
			{
				if (pOptions->mapOEM.aElements[i].szKey[0] == '\0')
				{
					strncpy(pOptions->mapOEM.aElements[i].szKey, pstrValue, MAP_STRINGSIZE - 1);
					pOptions->mapOEM.aElements[i].szKey[MAP_STRINGSIZE - 1] = '\0';
					if (pOptions->mapOEM.aElements[i].szVal[0] != '\0')
						pOptions->mapOEM.wCount++;
					break;
				}
			}
			if (pOptions->bPrintOpts)
				printf("OEM String:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CMCALLBACKVER:
			if (sscanf(pstrValue, "%d", &pOptions->wCMCallbackVersion) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid version number for the character mapping callback (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("CM Callback Version:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CMCALLBACKCHARSET:
			seResult = StringToDWEx(Charsets, pstrValue, &pOptions->dwCMCallbackCharset, (char *)"character mapping callback character set", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("CM Callback Character Set:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_OEMOUTPUT:
			for (i = pOptions->mapOEM.wCount; i < MAP_MAXELEMENTS; i++)
			{
				if (pOptions->mapOEM.aElements[i].szVal[0] == '\0')
				{
					strncpy(pOptions->mapOEM.aElements[i].szVal, pstrValue, MAP_STRINGSIZE - 1);
					pOptions->mapOEM.aElements[i].szVal[MAP_STRINGSIZE - 1] = '\0';
					if (pOptions->mapOEM.aElements[i].szKey[0] != '\0')
					{
						pOptions->mapOEM.wCount++;

						if (*pstrValue == ':')
						{
							pstrValue++;

							/* Hex input, store as is after converting from text to hex. */
							nLength = (int)strlen(pstrValue);

							/* Number of characters after the ':' must be divisible by 4. */
							if ((nLength > 0) && (nLength % 4 == 0))
							{
								VTLPSTR pstrNum = pstrValue;
								int j, k;

								k = 0;

								for (j = 0; j < nLength; j += 4)
								{
									StrToWord(pstrNum, &pOptions->mapOEM.aElements[i].wzVal[k++]);
									pstrNum += 4;
								}

								pOptions->mapOEM.aElements[i].wzVal[k] = 0;
							}
#ifndef DISABLE_REPORTING
							else
							{
								fprintf(stderr, "Error, line %d: Hex string length must be a multiple of 4\n", dwLine);
							}
#endif
						}
						else
						{
							ConvFunc(pOptions->mapOEM.aElements[i].wzVal, MAP_STRINGSIZE * 2, pstrValue, (VTDWORD)strlen(pstrValue));
						}
					}
					break;
				}
			}
			if (pOptions->bPrintOpts)
				printf("OEM Output:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_LINKACTION:
			seResult = StringToDWEx(LinkActions, pstrValue, &dwValue, (char *)"EX_CALLBACK_ID_PROCESSLINK action", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwLinkAction = dwValue;
			if (pOptions->bPrintOpts)
				printf("Link Action:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_LINKLOCATION:
			if (strlen(pstrValue) >= sizeof(pOptions->szLinkLocation))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->szLinkLocation, pstrValue, VT_MAX_URL);
			pOptions->szLinkLocation[VT_MAX_URL - 1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Link Location:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CUSTOMELEMENT:
			for (i = pOptions->mapCustom.wCount; i < MAP_MAXELEMENTS; i++)
			{
				if (pOptions->mapCustom.aKeys[i].szKey[0] == '\0')
				{
					strncpy(pOptions->mapCustom.aKeys[i].szKey, pstrValue, MAP_STRINGSIZE - 1);
					pOptions->mapCustom.aKeys[i].szKey[MAP_STRINGSIZE - 1] = '\0';
					if (pOptions->mapCustom.aKeys[i].szKey[0] != '\0')
						pOptions->mapCustom.wCount++;
					break;
				}
			}
			if (pOptions->bPrintOpts)
				printf("Custom Element:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CUSTOMELEMENTVALUE:
			if (pOptions->mapCustom.wCount > 0)
			{
				KEYVALUE *pKey = &pOptions->mapCustom.aKeys[pOptions->mapCustom.wCount - 1];
				int j;

				for (j = pKey->wCount; j < MAP_MAXELEMENTS; j++)
				{
					if (pKey->aElements[j].szElementValue[0] == '\0')
					{
						strncpy(pKey->aElements[j].szElementValue, pstrValue, MAP_STRINGSIZE - 1);
						pKey->aElements[j].szElementValue[MAP_STRINGSIZE - 1] = '\0';

						if (pKey->aElements[j].szElementValue[0] != '\0')
							pKey->wCount++;
						break;
					}
				}
			}
			if (pOptions->bPrintOpts)
				printf("Custom Element Value:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PROCESSELEMENT:
			if (pOptions->mapCustom.wCount > 0)
			{
				KEYVALUE *pKey = &pOptions->mapCustom.aKeys[pOptions->mapCustom.wCount - 1];
				VTLPSTR  pVal = pKey->szVal;
				VTLPWSTR pwVal = pKey->wzVal;

				if (pKey->wCount > 0)
				{
					ELEMENTVALUE *pElement = &pKey->aElements[pKey->wCount - 1];
					pVal = pElement->szVal;
					pwVal = pElement->wzVal;
				}

				strncpy(pVal, pstrValue, MAP_STRINGSIZE - 1);
				pVal[MAP_STRINGSIZE - 1] = '\0';

				if (*pstrValue == ':')
				{
					pstrValue++;

					/* Hex input, store as is after converting from text to hex. */
					nLength = (int)strlen(pstrValue);

					/* Number of characters after the ':' must be divisible by 4. */
					if ((nLength > 0) && (nLength % 4 == 0))
					{
						VTLPSTR pstrNum = pstrValue;
						int i, j;

						j = 0;

						for (i = 0; i < nLength; i += 4)
						{
							StrToWord(pstrNum, pwVal + j++);
							pstrNum += 4;
						}

						pwVal[j] = 0;
					}
#ifndef DISABLE_REPORTING
					else
					{
						fprintf(stderr, "Error, line %d: Hex string length must be a multiple of 4\n", dwLine);
					}
#endif
				}
				else
				{
					ConvFunc(pwVal, MAP_STRINGSIZE * 2, pstrValue, (VTDWORD)strlen(pstrValue));
				}
			}
			if (pOptions->bPrintOpts)
				printf("Process Element:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICSKIPSIZE:
			if (sscanf(pstrValue, "%d", &pOptions->dwGraphicSkipSize) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid graphic skip size value (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Graphic Skip Size:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRAPHICBUFFERSIZE:
			if (sscanf(pstrValue, "%d", &pOptions->dwGraphicBufferLimit) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid graphic device buffer size value (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Graphic Buffer Size:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_MAXURLLENGTH:
			if (sscanf(pstrValue, "%d", &pOptions->dwMaxURLLength) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid max URL length value (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("MAX URL Length:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PAGESIZE:
			if (sscanf(pstrValue, "%d", &pOptions->dwPageSize) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid HTML page size value (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Page Size:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TEMPDIRECTORY:
			if (strlen(pstrValue) >= sizeof(pOptions->szTempDir))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->szTempDir, pstrValue, VT_MAX_URL);
			pOptions->szTempDir[VT_MAX_URL - 1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Temp Directory:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TEMPDIRECTORY_U:
			dwSize = VT_MAX_URL * sizeof(VTWORD);
			if (decodeUnicode((VTLPCWSTR)pstrValue, pOptions->wzTempDir, (Swcslen((VTLPCWSTR)pstrValue) + 1) * sizeof(VTWCHAR), &dwSize) != BASE64_OK)
				return seResult;
			break;

		case SAMPLE_OPTION_HREFPREFIX:
			if (strlen(pstrValue) >= sizeof(pOptions->szURLPrefix))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->szURLPrefix, pstrValue, VT_MAX_URL);
			pOptions->szURLPrefix[VT_MAX_URL - 1] = '\0';
			dwSize = (VTDWORD)strlen(pstrValue);
			if (dwSize > 0)
			{
				VTLPBYTE ptr = (VTLPBYTE)pOptions->szURLPrefix;
				ptr += dwSize - 1;

				if (*ptr != '/')
					strcat(pOptions->szURLPrefix, "/");
			}
			if (pOptions->bPrintOpts)
				printf("Href Prefix:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FIRSTPREVHREF:
			ConvFunc(pOptions->wzFirstPrevURL, VT_MAX_URL * sizeof(VTWORD), pstrValue, (VTDWORD)strlen(pstrValue));
			if (pOptions->bPrintOpts)
				printf("First Previous Href:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SUBSTITUTEGRAPHIC:
			if (strlen(pstrValue) >= sizeof(pOptions->szSubstituteGraphic))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->szSubstituteGraphic, pstrValue, VT_MAX_URL);
			pOptions->szSubstituteGraphic[VT_MAX_URL - 1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Substitute Graphic:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SUBSTITUTEGRAPHIC_U:
			dwSize = VT_MAX_URL * sizeof(VTWORD);
			if (base64decode((VTLPBYTE)pstrValue, (VTLPBYTE)pOptions->wzSubstituteGraphic, (Swcslen((VTLPCWSTR)pstrValue) + 1) * sizeof(VTWCHAR), &dwSize) != BASE64_OK)
				return seResult;
			break;

		case SAMPLE_OPTION_ALTLINK_PREV:
			if (strlen(pstrValue) >= sizeof(pOptions->aszAltLinkURLs[0]))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->aszAltLinkURLs[0], pstrValue, VT_MAX_URL);
			pOptions->aszAltLinkURLs[0][VT_MAX_URL - 1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Altlink Previous:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_ALTLINK_NEXT:
			if (strlen(pstrValue) >= sizeof(pOptions->aszAltLinkURLs[1]))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->aszAltLinkURLs[1], pstrValue, VT_MAX_URL);
			pOptions->aszAltLinkURLs[1][VT_MAX_URL - 1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Altlink Next:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBALLDISABLED:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"disable all callbacks", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* disable all the callback */
				pOptions->dwCallbackFlags = SCCEX_CALLBACKFLAG_ALLDISABLED;
			if (pOptions->bPrintOpts)
				printf("CB All Disabled:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBCREATENEWFILE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_CREATENEWFILE", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* enable the create new file callback */
				pOptions->dwCallbackFlags |= SCCEX_CALLBACKFLAG_CREATENEWFILE;
			else
				pOptions->dwCallbackFlags &= ~SCCEX_CALLBACKFLAG_CREATENEWFILE;
			if (pOptions->bPrintOpts)
				printf("CB Create new file:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBNEWFILEINFO:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_NEWFILEINFO", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* enable the new file info callback */
				pOptions->dwCallbackFlags |= SCCEX_CALLBACKFLAG_NEWFILEINFO;
			else
				pOptions->dwCallbackFlags &= ~SCCEX_CALLBACKFLAG_NEWFILEINFO;
			if (pOptions->bPrintOpts)
				printf("CB New file Info:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBPROCESSLINK:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_PROCESSLINK", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* enable the processline callback */
				pOptions->dwCallbackFlags |= SCCEX_CALLBACKFLAG_PROCESSLINK;
			else
				pOptions->dwCallbackFlags &= ~SCCEX_CALLBACKFLAG_PROCESSLINK;
			if (pOptions->bPrintOpts)
				printf("CB Process Link:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBCUSTOMELEMENT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_CUSTOMELEMENTLIST", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* enable the curtomer element list callback */
				pOptions->dwCallbackFlags |= SCCEX_CALLBACKFLAG_CUSTOMELEMENT;
			else
				pOptions->dwCallbackFlags &= ~SCCEX_CALLBACKFLAG_CUSTOMELEMENT;
			if (pOptions->bPrintOpts)
				printf("CB Custom Element:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBGRAPHICEXPORTFAILURE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_GRAPHICEXPORTFAILURE", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)  /* enable the graphic export failure callback */
				pOptions->dwCallbackFlags |= SCCEX_CALLBACKFLAG_GRAPHICEXPORTFAILURE;
			else
				pOptions->dwCallbackFlags &= ~SCCEX_CALLBACKFLAG_GRAPHICEXPORTFAILURE;
			if (pOptions->bPrintOpts)
				printf("CB Graphic Export Failure:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBOEMOUTPUT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_OEMOUTPUT_VER2", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* enable the oemoutput callback */
				pOptions->dwCallbackFlags |= SCCEX_CALLBACKFLAG_OEMOUTPUT;
			else
				pOptions->dwCallbackFlags &= ~SCCEX_CALLBACKFLAG_OEMOUTPUT;
			if (pOptions->bPrintOpts)
				printf("CB OEM Output:\t%s\n", pstrValue);
			if (pOptions->bPrintOpts)
				printf("CB OEM Output:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBALTLINK:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_ALTLINK", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* enable the altlink callback */
				pOptions->dwCallbackFlags |= SCCEX_CALLBACKFLAG_ALTLINK;
			else
				pOptions->dwCallbackFlags &= ~SCCEX_CALLBACKFLAG_ALTLINK;
			if (pOptions->bPrintOpts)
				printf("CB Altlink:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBARCHIVE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"handle EX_CALLBACK_ID_ENTERARCHIVE", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* enable the archive callback */
				pOptions->dwCallbackFlags |= SCCEX_CALLBACKFLAG_ARCHIVE;
			else
				pOptions->dwCallbackFlags &= ~SCCEX_CALLBACKFLAG_ARCHIVE;
			if (pOptions->bPrintOpts)
				printf("CB Archive:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CBALLENABLED:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"enable all callbacks", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1) /* enable all the callback */
				pOptions->dwCallbackFlags = SCCEX_CALLBACKFLAG_ALLENABLED;
			if (pOptions->bPrintOpts)
				printf("Callback all Enables:\t%s\n", pstrValue);
			if (pOptions->bPrintOpts)
				printf("CB All Enabled:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRIDWRAP:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"set to indicate whether grid output should continue when the edge of the spreadsheet is reached", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bGridWrap = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Grid Wrap:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRIDROWS:
			if (sscanf(pstrValue, "%d", &pOptions->dwGridRows) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid grid row size (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Grid Rows:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRIDCOLS:
			if (sscanf(pstrValue, "%d", &pOptions->dwGridCols) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid grid column size (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Grid Cols:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GRIDADVANCE:
			seResult = StringToDWEx(AcrossDown, pstrValue, &pOptions->dwGridAdvance, (char *)"grid advance direction", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Grid Advance:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SUPPRESSFONTSIZE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"suppress font size", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwFontFlags |= SCCEX_FONTFLAGS_SUPPRESSSIZE;
			else
				pOptions->dwFontFlags &= ~SCCEX_FONTFLAGS_SUPPRESSSIZE;
			if (pOptions->bPrintOpts)
				printf("Suppress Font Size:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SUPPRESSFONTCOLOR:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"suppress font color", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwFontFlags |= SCCEX_FONTFLAGS_SUPPRESSCOLOR;
			else
				pOptions->dwFontFlags &= ~SCCEX_FONTFLAGS_SUPPRESSCOLOR;
			if (pOptions->bPrintOpts)
				printf("Suppress Font Color:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SUPPRESSFONTFACE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"suppress font face", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwFontFlags |= SCCEX_FONTFLAGS_SUPPRESSFACE;
			else
				pOptions->dwFontFlags &= ~SCCEX_FONTFLAGS_SUPPRESSFACE;
			if (pOptions->bPrintOpts)
				printf("Suppress Font Face:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_HTML_EXTENSION:
			SetCustomFileExtension(FI_HTML, pstrValue);
			if (pOptions->bPrintOpts)
				printf("HTML Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_MHTML_EXTENSION:
			SetCustomFileExtension(FI_MHTML, pstrValue);
			if (pOptions->bPrintOpts)
				printf("MHTML Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WML_EXTENSION:
			SetCustomFileExtension(FI_WML, pstrValue);
			if (pOptions->bPrintOpts)
				printf("WML Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_HDML_EXTENSION:
			SetCustomFileExtension(FI_HDML, pstrValue);
			if (pOptions->bPrintOpts)
				printf("HDML Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_XHTMLB_EXTENSION:
			SetCustomFileExtension(FI_XHTMLB, pstrValue);
			if (pOptions->bPrintOpts)
				printf("XHTMLB Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHTML_EXTENSION:
			SetCustomFileExtension(FI_CHTML, pstrValue);
			if (pOptions->bPrintOpts)
				printf("CHTML Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_HTMLWCA_EXTENSION:
			SetCustomFileExtension(FI_HTMLWCA, pstrValue);
			if (pOptions->bPrintOpts)
				printf("HTMLWCA Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_HTMLAG_EXTENSION:
			SetCustomFileExtension(FI_HTMLAG, pstrValue);
			if (pOptions->bPrintOpts)
				printf("HTMLAG Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WIRELESSHTML_EXTENSION:
			SetCustomFileExtension(FI_WIRELESSHTML, pstrValue);
			if (pOptions->bPrintOpts)
				printf("WIRELESSHTML Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TEXT_EXTENSION:
			SetCustomFileExtension(FI_TEXT, pstrValue);
			if (pOptions->bPrintOpts)
				printf("TEXT Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_HTML_CSS_EXTENSION:
			SetCustomFileExtension(FI_HTML_CSS, pstrValue);
			if (pOptions->bPrintOpts)
				printf("HTML CSS Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_JAVASCRIPT_EXTENSION:
			SetCustomFileExtension(FI_JAVASCRIPT, pstrValue);
			if (pOptions->bPrintOpts)
				printf("JAVA Script Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_JPEGFIF_EXTENSION:
			SetCustomFileExtension(FI_JPEGFIF, pstrValue);
			if (pOptions->bPrintOpts)
				printf("JPEGFIF Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GIF_EXTENSION:
			SetCustomFileExtension(FI_GIF, pstrValue);
			if (pOptions->bPrintOpts)
				printf("GIF Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WBMP_EXTENSION:
			SetCustomFileExtension(FI_WBMP, pstrValue);
			if (pOptions->bPrintOpts)
				printf("WBMP Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_BMP_EXTENSION:
			SetCustomFileExtension(FI_BMP, pstrValue);
			if (pOptions->bPrintOpts)
				printf("BMP Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PNG_EXTENSION:
			SetCustomFileExtension(FI_PNG, pstrValue);
			if (pOptions->bPrintOpts)
				printf("PNG Extension:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_XMLDEFMETHOD:
			seResult = StringToDWEx(XMLDefMethod, pstrValue, &pOptions->dwXMLDefMethod, (char *)"XML Definition Method", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("XML Def Method:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_XMLDEFREFERENCE:
			ConvFunc(pOptions->wzXMLDefReference, MAX_DEF_REFERENCE_LENGTH * sizeof(VTWORD), pstrValue, (VTDWORD)strlen(pstrValue));
			if (pOptions->bPrintOpts)
				printf("XML Def Reference:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_NULLREPLACEMENTCHAR:
			if (sscanf(pstrValue, "%hx", &pOptions->wNullReplacementChar) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid NULL replacement char value (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Null Replacement Char:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PSTYLENAMESFLAG:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"paragraph style names flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_XML_PSTYLENAMES;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_XML_PSTYLENAMES;
			if (pOptions->bPrintOpts)
				printf("Paragraph Style Name Flag:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_EMBEDDINGSFLAG:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"embeddings flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_XML_EMBEDDINGS;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_XML_EMBEDDINGS;
			if (pOptions->bPrintOpts)
				printf("Embedding Flag:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SUPPRESSPROPERTIES:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"suppress properties flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_IND_SUPPRESSPROPERTIES;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_IND_SUPPRESSPROPERTIES;
			if (pOptions->bPrintOpts)
				printf("Suppress Properties:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SUPPRESSATTACHMENTS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"suppress attachments flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_XML_SUPPRESSATTACHMENTS;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_XML_SUPPRESSATTACHMENTS;
			if (pOptions->bPrintOpts)
				printf("Suppress Attachments:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SUPPRESSARCHIVESUBDOCS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"suppress archive subdocs flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_XML_SUPPRESSARCHIVESUBDOCS;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_XML_SUPPRESSARCHIVESUBDOCS;
			if (pOptions->bPrintOpts)
				printf("Supppress Archive:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PROCESSGENERATEDTEXT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"process generated text flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_IND_GENERATED;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_IND_GENERATED;
			if (pOptions->bPrintOpts)
				printf("Process Generated Text:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_NOXMLDECLARATIONFLAG:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"no XML declaration flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
			{
				pOptions->dwSearchMLFlags |= SCCEX_XML_NO_XML_DECLARATION;
				pOptions->dwPageMLFlags |= SCCEX_XML_NO_XML_DECLARATION;
			}
			else
			{
				pOptions->dwSearchMLFlags &= ~SCCEX_XML_NO_XML_DECLARATION;
				pOptions->dwPageMLFlags &= ~SCCEX_XML_NO_XML_DECLARATION;
			}
			if (pOptions->bPrintOpts)
				printf("No XML Declaration Flag:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_METADATAONLY:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"metadata only flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_METADATAONLY;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_METADATAONLY;
			if (pOptions->bPrintOpts)
				printf("Meta Data Only:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_ANNOTATIONS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"annotation flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_ANNOTATIONS;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_ANNOTATIONS;
			if (pOptions->bPrintOpts)
				printf("Annotations:\t\t\t%s\n", pstrValue);
			break;


		case SAMPLE_OPTION_PRODUCEURLS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"produce URLs flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_PRODUCEURLS;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_PRODUCEURLS;
			if (pOptions->bPrintOpts)
				printf("Produce URLS:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRODUCEOBJECTINFO:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"produce Object info flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_XML_PRODUCEOBJECTINFO;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_XML_PRODUCEOBJECTINFO;
			if (pOptions->bPrintOpts)
				printf("Produce Object info:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_ENABLEERRORINFO:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"produce error info flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_XML_ENABLEERRORINFO;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_XML_ENABLEERRORINFO;
			if (pOptions->bPrintOpts)
				printf("Produce error info:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CELLINFO:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"produce cell row column info flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_IND_SS_CELLINFO;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_IND_SS_CELLINFO;
			if (pOptions->bPrintOpts)
				printf("Produce cell row column info:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FORMULAS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"produce formula info flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_IND_SS_FORMULAS;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_IND_SS_FORMULAS;
			if (pOptions->bPrintOpts)
				printf("Produce formula info:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SKIPSTYLES:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"skip sytles info flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_XML_SKIPSTYLES;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_XML_SKIPSTYLES;
			if (pOptions->bPrintOpts)
				printf("Skip Style info:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_UNMAPPEDTEXT:
			seResult = StringToDWEx(UnmappedTextProduction, pstrValue, &pOptions->dwUnmappedText, (char *)"SearchML unmapped text", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Unmapped Text:\t\t\t%s\n", pstrValue);
			break;


		case SAMPLE_OPTION_CHARBOLD:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"bold character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_BOLD;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_BOLD;
			if (pOptions->bPrintOpts)
				printf("Character Bold:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARITALIC:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"italic character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_ITALIC;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_ITALIC;
			if (pOptions->bPrintOpts)
				printf("Character Italic:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARUNDERLINE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"underline char attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_UNDERLINE | SCCEX_XML_SEARCHML_DOTUNDERLINE | SCCEX_XML_SEARCHML_WORDUNDERLINE;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_UNDERLINE | SCCEX_XML_SEARCHML_DOTUNDERLINE | SCCEX_XML_SEARCHML_WORDUNDERLINE;
			if (pOptions->bPrintOpts)
				printf("Character Underline:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARDUNDERLINE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"double underline character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_DUNDERLINE;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_DUNDERLINE;
			if (pOptions->bPrintOpts)
				printf("Character Double underline:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHAROUTLINE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"outline character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_OUTLINE;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_OUTLINE;
			if (pOptions->bPrintOpts)
				printf("Character Outline:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARHIDDEN:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"hidden character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_HIDDEN;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_HIDDEN;
			if (pOptions->bPrintOpts)
				printf("Character Hidden:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARSTRIKEOUT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"strikeout character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_STRIKEOUT;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_STRIKEOUT;
			if (pOptions->bPrintOpts)
				printf("Character Strikeout:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARSMALLCAPS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"small caps character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_SMALLCAPS;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_SMALLCAPS;
			if (pOptions->bPrintOpts)
				printf("Character Smallcaps:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARALLCAPS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"all caps character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_ALLCAPS;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_ALLCAPS;
			if (pOptions->bPrintOpts)
				printf("Character Allcaps:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHAROCE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"original character encoding character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_OCE;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_OCE;
			if (pOptions->bPrintOpts)
				printf("original character encoding:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARREVISIONDELETE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"revision delete character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_REVISIONDELETE;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_REVISIONDELETE;
			if (pOptions->bPrintOpts)
				printf("Character Revision Delete:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARREVISIONADD:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"revision add character attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCharAttrs |= SCCEX_XML_SEARCHML_REVISIONADD;
			else
				pOptions->dwCharAttrs &= ~SCCEX_XML_SEARCHML_REVISIONADD;
			if (pOptions->bPrintOpts)
				printf("Character revision Add:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PARASPACING:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"spacing paragraph attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwParaAttrs |= SCCEX_XML_SEARCHML_SPACING;
			else
				pOptions->dwParaAttrs &= ~SCCEX_XML_SEARCHML_SPACING;
			if (pOptions->bPrintOpts)
				printf("Paragraph Spacing:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PARAHEIGHT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"height paragraph attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwParaAttrs |= SCCEX_XML_SEARCHML_HEIGHT;
			else
				pOptions->dwParaAttrs &= ~SCCEX_XML_SEARCHML_HEIGHT;
			if (pOptions->bPrintOpts)
				printf("Paragraph Height:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PARALEFTINDENT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"left indent paragraph attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwParaAttrs |= SCCEX_XML_SEARCHML_LEFTINDENT;
			else
				pOptions->dwParaAttrs &= ~SCCEX_XML_SEARCHML_LEFTINDENT;
			if (pOptions->bPrintOpts)
				printf("Paragraph Left Indent:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PARARIGHTINDENT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"right indent paragraph attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwParaAttrs |= SCCEX_XML_SEARCHML_RIGHTINDENT;
			else
				pOptions->dwParaAttrs &= ~SCCEX_XML_SEARCHML_RIGHTINDENT;
			if (pOptions->bPrintOpts)
				printf("Paragraph Right Indent:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PARAFIRSTINDENT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"first indent paragraph attribute flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwParaAttrs |= SCCEX_XML_SEARCHML_FIRSTINDENT;
			else
				pOptions->dwParaAttrs &= ~SCCEX_XML_SEARCHML_FIRSTINDENT;
			if (pOptions->bPrintOpts)
				printf("Paragraph First Indent:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTERNAME:
			if (strlen(pstrValue) >= sizeof(pOptions->szPrinterName))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->szPrinterName, pstrValue, sizeof(pOptions->szPrinterName));
			pOptions->szPrinterName[sizeof(pOptions->szPrinterName) - 1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Printer Name:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_OFFSETTRACKED:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"offset tracked", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bOffsetTracked = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Offset Tracked:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CA_XMLOUTPUT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"XML output", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bXMLOutput = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("XML Output:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TIFFCOLORSPACE:
			seResult = StringToDWEx(TiffColorSpaceOpts, pstrValue, &pOptions->sEXTiffOptions.dwColorSpace, (char *)"Tiff ColorSpace", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("TIFF Color Space:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TIFFCOMPRESSION:
			seResult = StringToDWEx(TiffCompression, pstrValue, &pOptions->sEXTiffOptions.dwCompression, (char *)"Tiff Compresion", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("TIFF Compression:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TIFFBYTEORDER:
			seResult = StringToDWEx(TiffEndian, pstrValue, &pOptions->sEXTiffOptions.dwByteOrder, (char *)"Tiff Endian Type", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("TIFF Byte Order:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TIFFMULTIPAGE:
			{
				VTDWORD dwVal = 0;
				seResult = StringToDWEx(YesNo, pstrValue, &dwVal, (char *)"produce multi-page tiff flag", dwLine);
				if (dwVal)
					pOptions->sEXTiffOptions.dwTIFFFlags |= SCCGRAPHIC_TIFFFLAGS_ONEFILE;
			}
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("TIFF Multi Page:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TIFFONESTRIP:
			{
				VTDWORD dwVal = 0;
				seResult = StringToDWEx(YesNo, pstrValue, &dwVal, (char *)"produce one strip flag", dwLine);
				if (dwVal)
					pOptions->sEXTiffOptions.dwTIFFFlags |= SCCGRAPHIC_TIFFFLAGS_ONESTRIP;
			}
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("TIFF One Strip:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TIFFMSB:
			seResult = StringToDWEx(TiffFillOrder, pstrValue, &pOptions->sEXTiffOptions.dwFillOrder, (char *)"set fillorder", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("TIFF MSB:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TIFFGRAY2COLOR:
			{
				VTDWORD dwVal = 0;
				seResult = StringToDWEx(YesNo, pstrValue, &dwVal, (char *)"convert grayscale tiffs to jpeg", dwLine);
				if (dwVal)
					pOptions->sEXTiffOptions.dwTIFFFlags |= SCCGRAPHIC_TIFFFLAGS_GRAY2COLOR;
				if (seResult != SCCERR_OK)
					return seResult;

				if (pOptions->bPrintOpts)
					printf("TIFF Gray2Color:\t\t\t%s\n", pstrValue);
			}
			break;


		case SAMPLE_OPTION_SSPRINTGRIDLINES:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show spreadsheet gridlines flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bSSShowGridlines = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("SS Print Grid Lines:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SSPRINTHEADINGS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show spreadsheet headings flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bSSShowHeadings = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("SS Print Headings:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SSPRINTSCALEPERCENT:
			sscanf(pstrValue, "%d", &pOptions->dwSSScalePercent);
			if (pOptions->bPrintOpts)
				printf("SS Print Scale Percent:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SSPRINTSCALEXWIDE:
			sscanf(pstrValue, "%d", &pOptions->dwSSScaleXWide);
			if (pOptions->bPrintOpts)
				printf("SS Print Scale XWidth:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SSPRINTSCALEXHIGH:
			sscanf(pstrValue, "%d", &pOptions->dwSSScaleXHigh);
			if (pOptions->bPrintOpts)
				printf("SS Print Scale XHeigh:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DBPRINTGRIDLINES:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show database gridlines flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bDBShowGridlines = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("DB Print Grid Lines:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DBPRINTHEADINGS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show database headings flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bDBShowHeadings = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("DB Print Headings:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_IGNOREPASSWORD:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"ignore password", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bIgnorePassword = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Ignore Password:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_STROKETEXT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"stroke out text", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bStrokeText = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Stroke Text:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GENERATE_EXCEL_REVISIONS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"generate Excel revisions", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bGenerateExcelRevisions = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Generate Excel Revisions:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_RENDERING_PREFER_OIT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"select rendering engine", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bOutputSolution = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Rendering Prefer OIT:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_RENDER_DISABLEALPHABLENDING:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"disable alpha blending", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bRenderDisableAlphaBlending = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Disable Alpha Blending:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_RENDER_ENABLEALPHABLENDING:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"enable alpha blending on X", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bRenderEnableAlphaBlending = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Enable Alpha Blending on X:\t\t%s\n", pstrValue);
			break;


		case SAMPLE_OPTION_VECPRINTBACKGROUND:
			{
				VTDWORD dwTemp;
				seResult = StringToDWEx(YesNo, pstrValue, &dwTemp, (char *)"show vector background flag", dwLine);
				pOptions->bVecShowBackground = (VTBOOL)dwTemp;
				if (seResult != SCCERR_OK)
					return seResult;

				if (pOptions->bPrintOpts)
					printf("VEC Print Background:\t\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_SSPRINTFITTOPAGE:
			seResult = StringToDWEx(SSFitToPage, pstrValue, &pOptions->dwSSFitToPage, (char *)"spreadsheet fit to page setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("SS Print Fit to Page:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SSPRINTDIRECTION:
			seResult = StringToDWEx(PrintDirection, pstrValue, &pOptions->dwSSDirection, (char *)"spreadsheet direction", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("SS Print Direction:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_VECPRINTASPECT:
			seResult = StringToDWEx(VectorPrintAspect, pstrValue, &pOptions->dwVecAspect, (char *)"vector aspect ratio setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("VEC Print Aspect:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DBPRINTFITTOPAGE:
			seResult = StringToDWEx(DBFitToPage, pstrValue, &pOptions->dwDBFitToPage, (char *)"database fit to page setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("DB Print Fit to Page:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTMARGINSTOP:
			sscanf(pstrValue, "%d", &pOptions->sDefaultMargins.dwTop);
			if (pOptions->bPrintOpts)
				printf("Print Margins Top:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTMARGINSBOTTOM:
			sscanf(pstrValue, "%d", &pOptions->sDefaultMargins.dwBottom);
			if (pOptions->bPrintOpts)
				printf("Print Margins Bottom:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTMARGINSLEFT:
			sscanf(pstrValue, "%d", &pOptions->sDefaultMargins.dwLeft);
			if (pOptions->bPrintOpts)
				printf("Print Margins Left:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTMARGINSRIGHT:
			sscanf(pstrValue, "%d", &pOptions->sDefaultMargins.dwRight);
			if (pOptions->bPrintOpts)
				printf("Print Margins Right:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_MAXSSDBPAGEWIDTH:
			sscanf(pstrValue, "%d", &pOptions->dwMaxSSDBPageWidth);
			if (pOptions->bPrintOpts)
				printf("MAX SSDB Page Width:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_MAXSSDBPAGEHEIGHT:
			sscanf(pstrValue, "%d", &pOptions->dwMaxSSDBPageHeight);
			if (pOptions->bPrintOpts)
				printf("MAX SSDB Page Height:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTSTARTPAGE:
			sscanf(pstrValue, "%d", &pOptions->dwExportStartPage);
			if (pOptions->bPrintOpts)
				printf("Print Start Page:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTENDPAGE:
			sscanf(pstrValue, "%d", &pOptions->dwExportEndPage);
			if (pOptions->bPrintOpts)
				printf("Printed End Page:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WHATTOPRINT:
			seResult = StringToDWEx(WhatToPrint, pstrValue, &pOptions->dwWhatToExport, (char *)"what to export option", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("What to Print:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_USEDOCPAGESETTINGS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"use doc page setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bUseDocPageSettings = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Use DocPage Settings:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DEFAULTPRINTFONTFACE:
			if (strlen(pstrValue) >= sizeof(pOptions->DefaultPrintFont.szFace))
				return SCCERR_INSUFFICIENTBUFFER;

			if (*pstrValue == '"')
			{
				strncpy(pOptions->DefaultPrintFont.szFace, pstrValue + 1, strlen(pstrValue) - 2);
				pOptions->DefaultPrintFont.szFace[strlen(pstrValue) - 2] = '\0';
			}
			else
			{
				strncpy(pOptions->DefaultPrintFont.szFace, pstrValue, sizeof(pOptions->DefaultPrintFont.szFace));
				pOptions->DefaultPrintFont.szFace[sizeof(pOptions->DefaultPrintFont.szFace) - 1] = '\0';
			}
			if (pOptions->bPrintOpts)
				printf("Default Print Font Face:\t%s\n", pOptions->DefaultPrintFont.szFace);
			break;

		case SAMPLE_OPTION_DEFAULTPRINTFONTHEIGHT:
			{
				VTDWORD dwValue;
				sscanf(pstrValue, "%d", &dwValue);
				pOptions->DefaultPrintFont.wHeight = (VTWORD)dwValue;
				if (pOptions->bPrintOpts)
					printf("Default Print Font Height:\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_TRANSPARENCYCOLORRED:
			sscanf(pstrValue, "%d", &pOptions->dwRed);
			if (pOptions->bPrintOpts)
				printf("Transparency Color Red:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TRANSPARENCYCOLORGREEN:
			sscanf(pstrValue, "%d", &pOptions->dwGreen);
			if (pOptions->bPrintOpts)
				printf("Transparency Color Green:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TRANSPARENCYCOLORBLUE:
			sscanf(pstrValue, "%d", &pOptions->dwBlue);
			if (pOptions->bPrintOpts)
				printf("Transparency Color Blue:\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_CROPPING:
			seResult = StringToDWEx(ImageCrop, pstrValue, &pOptions->dwImageCropping, (char *)"set image cropping on or off", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Graphic Cropping:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_OPACITY:
			sscanf(pstrValue, "%d", &lValue);
			if (lValue < 0 || lValue > 255)
			{
				if (lValue < 0)
					pOptions->dwImageWatermarkOpacity = 0;
				else if (lValue > 255)
					pOptions->dwImageWatermarkOpacity = 255;

#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid watermark opacity value\n", dwLine);
				fprintf(stderr, "Valid watermark opacity values are 0 - 255\n");
				fprintf(stderr, "255 = Watermark is fully opaque\n");
				fprintf(stderr, "  0 = Watermark is fully transparent\n");
#endif
			}
			else
			{
				pOptions->dwImageWatermarkOpacity = (VTDWORD)lValue;
			}

			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Opacity:\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_HORIZONTAL_OFFSETFROM:
			seResult = StringToDWEx(ImageWatermarkHorizontalOffsetFrom, pstrValue, &pOptions->dwImageWatermarkHorizontalOffsetFrom, (char *)"graphic watermark horizontal offset setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Horizontal Offset From:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_VERTICAL_OFFSETFROM:
			seResult = StringToDWEx(ImageWatermarkVerticalOffsetFrom, pstrValue, &pOptions->dwImageWatermarkVerticalOffsetFrom, (char *)"graphic watermark vertical offset setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Vertical Offset From:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_HORIZONTAL_OFFSET:
			sscanf(pstrValue, "%d", &pOptions->lImageWatermarkHorizontalOffset);
			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Horizontal Offset:\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_VERTICAL_OFFSET:
			sscanf(pstrValue, "%d", &pOptions->lImageWatermarkVerticalOffset);
			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Vertical Offset:\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_SCALETYPE:
			seResult = StringToDWEx(ImageWatermarkScaleType, pstrValue, &pOptions->dwImageWatermarkScaleType, (char *)"graphic watermark scale type setting", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Scale Type:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_SCALEPERCENT:
			sscanf(pstrValue, "%d", &lValue);
			if (lValue < 0 || lValue > 100)
			{
				if (lValue < 0)
					pOptions->dwImageWatermarkScalePercent = 0;
				else if (lValue > 100)
					pOptions->dwImageWatermarkScalePercent = 100;

#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid watermark scale percentage value\n", dwLine);
				fprintf(stderr, "Valid watermark scale percentage values are 0 - 100\n");
				fprintf(stderr, "100 = Watermark is scaled to fit height or width of apge\n");
				fprintf(stderr, "  0 = Watermark is scaled to 0%% of page size (effectively no watermark)\n");
#endif
			}
			else
			{
				pOptions->dwImageWatermarkScalePercent = (VTDWORD)lValue;
			}

			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Scale Percent:\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_HORIZONTALPOS:
			sscanf(pstrValue, "%d", &pOptions->lImageWatermarkHorizontalPos);
			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Horizontal Offset:\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_VERTICALPOS:
			sscanf(pstrValue, "%d", &pOptions->lImageWatermarkVerticalPos);
			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Vertical Offset:\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_PATH:
			pOptions->dwWatermarkSpecSize = (VTDWORD)strlen(pstrValue) + 1;
			pOptions->pWatermarkSpec = (VTLPBYTE)malloc(pOptions->dwWatermarkSpecSize);
			if (pOptions->pWatermarkSpec == NULL)
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy((char*)pOptions->pWatermarkSpec, pstrValue, pOptions->dwWatermarkSpecSize);
			pOptions->pWatermarkSpec[pOptions->dwWatermarkSpecSize-1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Graphic Watermark Path:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_WATERMARK_IOTYPE:
			seResult = StringToDWEx(WatermarkIOType, pstrValue, &pOptions->dwImageWatermarkPathType, (char *)"graphic watermark path type", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Graphic Watermark path type:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_GRAPHIC_RENDERASPAGE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"render as page", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bRenderAsPage = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Graphic Render As Page:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DEFAULTPRINTFONTATTR:
			{
				VTDWORD dwValue;
				sscanf(pstrValue, "%x", &dwValue);
				pOptions->DefaultPrintFont.wAttr = (VTWORD)dwValue;
			}
			if (pOptions->bPrintOpts)
				printf("Default Print Font Attribute:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DEFAULTPRINTFONTTYPE:
			{
				VTDWORD dwValue;
				sscanf(pstrValue, "%d", &dwValue);
				pOptions->DefaultPrintFont.wType = (VTWORD)dwValue;
			}
			if (pOptions->bPrintOpts)
				printf("Default Print Font Type:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTFONTALIASID:
			{
				PFONTALIAS pFontAlias = &pOptions->FontAlias;

				while (pFontAlias->pNextFontAlias)
					pFontAlias = pFontAlias->pNextFontAlias;

				sscanf(pstrValue, "%d", &pFontAlias->PrintFontAlias.dwAliasID);
			}
			if (pOptions->bPrintOpts)
				printf("Print Font AliasID:\t\t\t%s\n", pstrValue);
			break;


		case SAMPLE_OPTION_PRINTFONTALIASFLAGS:
			{
				PFONTALIAS pFontAlias = &pOptions->FontAlias;

				while (pFontAlias->pNextFontAlias)
					pFontAlias = pFontAlias->pNextFontAlias;

				sscanf(pstrValue, "%d", &pFontAlias->PrintFontAlias.dwFlags);
			}
			if (pOptions->bPrintOpts)
				printf("Print Font Alias Flags:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTFONTALIASORIGINAL:
			{
				VTBYTE szTemp[SCCUT_FONTNAMEMAX] = {0};
				VTDWORD i;
				PFONTALIAS pFontAlias = &pOptions->FontAlias;

				strncpy((char*)szTemp, pstrValue, SCCUT_FONTNAMEMAX);
				szTemp[SCCUT_FONTNAMEMAX - 1] = '\0';
				if (pFontAlias->PrintFontAlias.szwOriginal[0])
				{
					while (pFontAlias->pNextFontAlias)
						pFontAlias = pFontAlias->pNextFontAlias;

					pFontAlias->pNextFontAlias = (PFONTALIAS)malloc(sizeof(FONTALIAS));
					memset(pFontAlias->pNextFontAlias, 0, sizeof(FONTALIAS));
					pFontAlias = pFontAlias->pNextFontAlias;
					pFontAlias->pNextFontAlias = NULL;

				}

				for (i = 0; i < SCCUT_FONTNAMEMAX; i++)
					pFontAlias->PrintFontAlias.szwOriginal[i] = (VTWORD)szTemp[i];
			}
			if (pOptions->bPrintOpts)
				printf("Print Font Alias Original:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRINTFONTALIAS:
			{
				VTBYTE szTemp[SCCUT_FONTNAMEMAX] = {0};
				VTWORD i;
				PFONTALIAS pFontAlias = &pOptions->FontAlias;

				strncpy((char*)szTemp, pstrValue, SCCUT_FONTNAMEMAX);
				szTemp[SCCUT_FONTNAMEMAX - 1] = '\0';
				while (pFontAlias->pNextFontAlias)
					pFontAlias = pFontAlias->pNextFontAlias;

				for (i = 0; i < SCCUT_FONTNAMEMAX; i++)
					pFontAlias->PrintFontAlias.szwAlias[i] = (VTWORD)szTemp[i];
				if (pOptions->bPrintOpts)
					printf("Print Font Alias:\t\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_FONTFILTERTYPE:
			seResult = StringToDWEx(FilterType, pstrValue, &dwValue, (char *)"type of font filter list", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->FontFilterList.bExclude = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Font Filter Type\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FONTFILTERLIST:
			{
				PFONTNAMELIST pFontList = pOptions->FontFilterList.pFontList;
				if (pFontList != NULL)
				{
					while (pFontList->pNextFont)
					{
						pFontList = pFontList->pNextFont;
					}
					pFontList->pNextFont = (PFONTNAMELIST)malloc(sizeof(FONTNAMELIST));
					memset(pFontList->pNextFont, 0, sizeof(FONTNAMELIST));
					pFontList = pFontList->pNextFont;
				}
				else
				{
					pFontList = pOptions->FontFilterList.pFontList = (PFONTNAMELIST)malloc(sizeof(FONTNAMELIST));
					memset(pFontList, 0, sizeof(FONTNAMELIST));
				}

				if (strlen(pstrValue) < SCCUT_FILENAMEMAX)
				{
					strncpy((char *)pFontList->szFontName, pstrValue, SCCUT_FONTNAMEMAX);
					pFontList->szFontName[SCCUT_FONTNAMEMAX - 1] = '\0';
				}
				else return SCCERR_BADPARAM;
			}
			break;

		case SAMPLE_OPTION_ACCEPT_ALT_GRAPHICS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"accept alt graphics", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bAcceptAltGraphics = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Accept Alt Graphics:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_BITMAPASBITMAP:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"export bitmaps as rasters", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_BITMAPASBITMAP;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_BITMAPASBITMAP;
			if (pOptions->bPrintOpts)
				printf("Bitmap As Bitmap:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARTASBITMAP:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"export charts as rasters", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_CHARTASBITMAP;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_CHARTASBITMAP;
			if (pOptions->bPrintOpts)
				printf("Chart As Bitmap:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PRESENTATIONASBITMAP:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"export presentations as rasters", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_PRESENTATIONASBITMAP;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_PRESENTATIONASBITMAP;
			if (pOptions->bPrintOpts)
				printf("Presentation As Bitmap:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_VECTORASBITMAP:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"export vectors as rasters", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_VECTORASBITMAP;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_VECTORASBITMAP;
			if (pOptions->bPrintOpts)
				printf("Vector As Bitmap:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_NOBITMAPELEMENTS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"do not export bitmaps", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_NOBITMAPELEMENTS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_NOBITMAPELEMENTS;
			if (pOptions->bPrintOpts)
				printf("No Bitmap Elements:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_NOCHARTELEMENTS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"do not export charts", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_NOCHARTELEMENTS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_NOCHARTELEMENTS;
			if (pOptions->bPrintOpts)
				printf("No Chart Elements:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_NOPRESENTATIONELEMENTS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"do not export presentations", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_NOPRESENTATIONELEMENTS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_NOPRESENTATIONELEMENTS;
			if (pOptions->bPrintOpts)
				printf("No Presentation Elements:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_NOVECTORELEMENTS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"do not export vectors", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_NOVECTORELEMENTS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_NOVECTORELEMENTS;
			if (pOptions->bPrintOpts)
				printf("No Vector Elements:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_ISODATES:
			/* deprecated.  Use SCCOPT_FORMATFLAGS instead */
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"use ISO 8601 Date standard", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_ISODATES;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_ISODATES;
			if (pOptions->bPrintOpts)
				printf("Iso dates:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FLATTENSTYLES:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"flatten styles, no based-on", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_FLATTENSTYLES;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_FLATTENSTYLES;
			if (pOptions->bPrintOpts)
				printf("Flatten Styles:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_REMOVEFONTGROUPS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"remove font groups", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_REMOVEFONTGROUPS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_REMOVEFONTGROUPS;
			if (pOptions->bPrintOpts)
				printf("Remove Font Group:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_INCLUDETEXTOFFSETS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"include text offsets", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_INCLUDETEXTOFFSETS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_INCLUDETEXTOFFSETS;
			if (pOptions->bPrintOpts)
				printf("Include Text Offsets:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_USEFULLFILEPATHS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Use Full File Paths", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_USEFULLFILEPATHS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_USEFULLFILEPATHS;
			if (pOptions->bPrintOpts)
				printf("Use Full File Paths:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TEXTOUT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"produce text in text ranges", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwPageMLFlags |= SCCEX_PAGEML_TEXTOUT;
			else
				pOptions->dwPageMLFlags &= ~SCCEX_PAGEML_TEXTOUT;
			if (pOptions->bPrintOpts)
				printf("Produce Text in Text Ranges:\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_EXTRACTEMBEDDEDFORMAT:
			seResult = StringToDWEx(ExtractEmbeddedFormat, pstrValue, &pOptions->dwExtractEmbeddedFormat, (char *)"extract attachment format", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Extract Attachment:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_EXEPATH:
			break;  /* Not an error if this is found, but it is only an option for the Java sample app.  Just ignore it here. */

		case SAMPLE_OPTION_WATERMARK:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Apply watermark", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bEnableWatermark = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Enable watermark:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WATERMARKPOS:
			seResult = StringToDWEx(WatermarkPos, pstrValue, &pOptions->WatermarkPos.dwWatermarkPos, (char *)"Watermark position", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Watermark Position:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WATERMARKVERTPOS:
			sscanf(pstrValue, "%d", &pOptions->WatermarkPos.lVerticalPos);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Watermark Vertical Position:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WATERMARKHORZPOS:
			sscanf(pstrValue, "%d", &pOptions->WatermarkPos.lHorizontalPos);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Watermark Horizontal Position:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WATERMARKIOSCALE:
			seResult = StringToDWEx(WatermarkScale, pstrValue, &pOptions->WatermarkIO.dwScalingMethod, (char *)"Watermark scale type", dwLine);
			if (pOptions->bPrintOpts)
				printf("Watermark Scale Type:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WATERMARKIOPERCENT:
			sscanf(pstrValue, "%d", &pOptions->WatermarkIO.dwScalePercent);
			if (pOptions->bPrintOpts)
				printf("Watermark Scale Percent:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WATERMARKPATH:
			if (strlen(pstrValue) >= sizeof(pOptions->WatermarkIO.Path.szWaterMarkPath))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->WatermarkIO.Path.szWaterMarkPath, pstrValue, SCCUT_FILENAMEMAX);
			pOptions->WatermarkIO.Path.szWaterMarkPath[SCCUT_FILENAMEMAX - 1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Watermark Path:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_WATERMARKIOTYPE:
			seResult = StringToDWEx(WatermarkIOType, pstrValue, &pOptions->WatermarkIO.dwType, (char *)"Watermark type", dwLine);
			if (pOptions->bPrintOpts)
				printf("Watermark Type:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FONTDIRECTORY:
			ALLOC(VTCHAR, pOptions->szFontDirectory, strlen(pstrValue) + 1);
			strcpy(pOptions->szFontDirectory, pstrValue);
			if (pOptions->bPrintOpts)
				printf("Font Path:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_EMBEDFONTS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Embed fonts", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bEmbedFonts = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Embed fonts:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_RENDER_EMBEDDED_FONTS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Render embedded fonts", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bRenderEmbeddedFonts = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Render embedded fonts:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FONTEMBEDPOLICY:
			seResult = StringToDWEx(FontEmbedPolicyType, pstrValue, &dwValue, (char *)"Font embedding policy", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwFontEmbedPolicy = (VTDWORD)dwValue;
			if (pOptions->bPrintOpts)
				printf("Font embedding policy:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DOLINEARIZATION:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Do Linearization", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bDoLinearization = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Do Linearization:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_COMPRESS_24BIT_TYPE:
			seResult = StringToDWEx(Compress24BitType, pstrValue, &dwValue, (char *)"compression 24 bit type", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwCompress24BitType = (VTDWORD)dwValue;
			if (pOptions->bPrintOpts)
				printf("Compression 24 bit type:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_EXPORTATTACHMENTS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Export Attachments", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bExportAttachments = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Export Attachments:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_IMAGE_PASSTHROUGH:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Pass Through Bitmap Images", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bDirectImagePassThrough = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Pass Through Bitmap Images:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_UNRESTRICTEDFONTEMBEDDING:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Unrestricted Font Embedding", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bUnrestrictedFontEmbedding = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Unrestricted Font Embedding:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DEFAULTHEIGHT:
			pOptions->DefaultPageSize.fHeight = 0.0f;
			sscanf(pstrValue, "%f", &pOptions->DefaultPageSize.fHeight);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Default Page Height:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DEFAULTWIDTH:
			pOptions->DefaultPageSize.fWidth = 0.0f;
			sscanf(pstrValue, "%f", &pOptions->DefaultPageSize.fWidth);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Default Page Width:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_UNITS:
			seResult = StringToDWEx(Units, pstrValue, &pOptions->DefaultPageSize.wUnits, (char *)"Default page size units", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Default Page Size Units:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DELIMITERS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Produce delimiters", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_DELIMITERS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_DELIMITERS;
			if (pOptions->bPrintOpts)
				printf("Produce delimiters:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SEPARATESTYLETABLES:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Place style_tables in separate output", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_SEPARATESTYLETABLES;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_SEPARATESTYLETABLES;
			if (pOptions->bPrintOpts)
				printf("Separate style tables:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_ODGRAPHICOPTIONS:
			seResult = StringToDWEx(ODGraphicOptions, pstrValue, &pOptions->dwODGraphicOptions, (char *)"Open Document Export Graphics options", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Open Document Export Graphics options:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_TIMEZONE:
			if (!strcmp(pstrValue, "0xF000") || !strcmp(pstrValue, "0xf000"))
				pOptions->lTimeZone = 0xF000;
			else
			{
				if (sscanf(pstrValue, "%d", &pOptions->lTimeZone) != 1 ||
					((pOptions->lTimeZone < -96) || (pOptions->lTimeZone > 96)))
				{
#ifndef DISABLE_REPORTING
					fprintf(stderr, "Error, line %d: Invalid time zone value (%s)\n", dwLine, pstrValue);
					fprintf(stderr, "Valid time zone values are (-96) - 96\n");
#endif
				}
			}
			if (pOptions->bPrintOpts)
				printf("Time Zone value:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_RECIPIENT_DELIVERY_INFORMATION:
			sscanf(pstrValue, "%d", &pOptions->dExtractRecipientDeliveryInfo);
			if (seResult != SCCERR_OK)
				return seResult;
			break;

		case SAMPLE_OPTION_BACKGROUND_COLOR:
			sscanf(pstrValue, "%d", &pOptions->bBackgroundColor);
			if (seResult != SCCERR_OK)
				return seResult;
			break;

		case SAMPLE_OPTION_DEFAULTINPUTCHARSET:
			seResult = StringToDWEx(DefaultInputCharsets, pstrValue, &pOptions->dwDefaultInputCharset, (char *)"default input character set", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Default Input Character Set:\t\t\t%s\n", pstrValue);
			break;


		case SAMPLE_OPTION_READBUFFERSIZE:
			if ((sscanf(pstrValue, "%d", &lValue) == 1) && (0 <= lValue))
			{
				pOptions->bufferOpts.dwReadBufferSize = (VTDWORD)lValue;
				pOptions->bufferOpts.dwFlags |= SCCBUFOPT_SET_READBUFSIZE;
			}
			else
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid I/O read buffer size (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("I/O read buffer size:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_MAPBUFFERSIZE:
			if ((sscanf(pstrValue, "%d", &lValue) == 1) && (0 <= lValue))
			{
				pOptions->bufferOpts.dwMMapBufferSize = lValue;
				pOptions->bufferOpts.dwFlags |= SCCBUFOPT_SET_MMAPBUFSIZE;
			}
			else
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid I/O memory map buffer size (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("I/O memory map buffer size:\t\t\t%s\n", pstrValue);
			break;


		case SAMPLE_OPTION_TEMPBUFFERSIZE:
			if ((sscanf(pstrValue, "%d", &lValue) == 1) && (0 <= lValue))
			{
				pOptions->bufferOpts.dwTempBufferSize = lValue;
				pOptions->bufferOpts.dwFlags |= SCCBUFOPT_SET_TEMPBUFSIZE;
			}
			else
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid memory-mapped temp file size (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("I/O memory-mapped temp file size:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_NUMBEROFSTATCALLBACKS:
			{
				VTDWORD dwValue;
				sscanf(pstrValue, "%d", &dwValue);
				pOptions->dwNumStatCallbacks = (VTWORD)dwValue;
				if (pOptions->bPrintOpts)
					printf("Number of Callbacks:\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_OPTIMIZESECTIONS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Optimize word processing sections", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_OPTIMIZESECTIONS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_OPTIMIZESECTIONS;
			if (pOptions->bPrintOpts)
				printf("Optimize word processing sections:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_CHARMAPPING:
			seResult = StringToDWEx(CCFlexCharMapping, pstrValue, &dwValue, (char *)"Character mapping", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwCCFlexFormatOptions &= ~CCFLEX_CHARMAPPING_BOTH;
			pOptions->dwCCFlexFormatOptions |= dwValue;
			if (pOptions->bPrintOpts)
				printf("Character Mapping:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_OCRTECH:
			seResult = StringToDWEx(OcrTechMapping, pstrValue, &dwValue, (char *)"Ocr Tech mapping", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwOcrTech = OCR_TECH_NUANCE;
			else if (dwValue == 2)
				pOptions->dwOcrTech = OCR_TECH_OTHER;
			else
				pOptions->dwOcrTech = OCR_TECH_NONE;
			if (pOptions->bPrintOpts)
				printf("OCR Tech mapping:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_OCRQUALITY:
			seResult = StringToDWEx(OcrQualityMapping, pstrValue, &dwValue, (char *)"Ocr Quality mapping", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 0)
				pOptions->dwOcrQuality = OCR_QUAL_FAST;
			else if (dwValue == 1)
				pOptions->dwOcrQuality = OCR_QUAL_SLOW;
			else
				pOptions->dwOcrQuality = OCR_QUAL_BALANCED;
			if (pOptions->bPrintOpts)
				printf("OCR Quality mapping:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_EXTRACTXMPMETADATA:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Extract XMP Metadata", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bExtractXMPMetadata = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Extract XMP Metadata:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PARSEXMPMETADATA:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Parse XMP Metadata", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bParseXMPMetadata = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Parse XMP Metadata:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_REDIRECTTEMPFILE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Redirect temp file", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bRedirectTempFile = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Redirect temp file:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_LINEARUNITS:
			seResult = StringToDWEx(LinearUnits, pstrValue, &dwValue, (char *)"Linear Units", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwLinearUnits = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Linear units:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_EMAILHEADER:
		case SAMPLE_OPTION_MIMEHEADER:
			seResult = StringToDWEx(EmailHeader, pstrValue, &dwValue, (char *)"Email Header", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwEmailHeader = dwValue;
			if (pOptions->bPrintOpts)
				printf("Email Header:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_DOCUMENTMEMORYMODE:
			seResult = StringToDWEx(DocumentMemoryMode, pstrValue, &pOptions->dwDocumentMemoryMode, (char *)"Document Memory Mode", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Document Memory Mode:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_OLEEMBEDDINGS:
			{
				VTDWORD dwTemp = 0;
				seResult = StringToDWEx(OLEEmbeddingProcessMode, pstrValue, &dwTemp, (char *)"OLE Embeddings Process Mode", dwLine);
				pOptions->wOLEEmbeddingMode = (VTWORD)dwTemp;
				if (seResult != SCCERR_OK)
					return seResult;

				if (pOptions->bPrintOpts)
					printf("OLE Embeddings Process Mode:\t\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_REORDERMETHOD:
			seResult = StringToDWEx(BiDiReorderMethod, pstrValue, &pOptions->dwReorderMethod, (char *)"Reorder Method", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("Reorder Method:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PASSWORD:
			{
				VTDWORD dwLength;
				if (*pstrValue != '\"')
				{
					fprintf(stderr, "Warning: password must be in quotes.\n");
					return DAERR_BADPARAM;
				}
				if (pOptions->wPasswordNum >= MAX_PASSWORD)
				{
					fprintf(stderr, "Warning: you may only set %d passwords.\n", MAX_PASSWORD);
					return DAERR_BADPARAM;
				}
				dwLength = (VTDWORD)strlen(pstrValue) - 2;

				/* because the password is in quotes, the quotation marks need to be skipped */
				strncpy(pOptions->szPassword[pOptions->wPasswordNum], pstrValue + 1, dwLength);
				pOptions->szPassword[pOptions->wPasswordNum][dwLength] = '\0';
        // Convert to Word string
        {
          VTWORD  *wPtr = pOptions->wzPassword[pOptions->wPasswordNum];
          VTCHAR  *cPtr = pOptions->szPassword[pOptions->wPasswordNum];
          while (*cPtr)
            *wPtr++ = *cPtr++;
          *wPtr = 0;
        }
				pOptions->wPasswordNum++;
			}
			break;

		case SAMPLE_OPTION_PASSWORD_U:
			if (*pstrValue != '\"')
			{
				fprintf(stderr, "Warning: password must be in quotes.\n");
				return DAERR_BADPARAM;
			}
			if (pOptions->wPasswordNum >= MAX_PASSWORD)
			{
				fprintf(stderr, "Warning: you may only set %d passwords.\n", MAX_PASSWORD);
				return DAERR_BADPARAM;
			}
			HexStrToUnicodeStr(pstrValue, pOptions->wzPassword[pOptions->wPasswordNum]);
			pOptions->wPasswordNum++;
			break;

		case SAMPLE_OPTION_NOTESID:
			if (pOptions->wNotesIdNum >= MAX_NOTESID)
			{
				fprintf(stderr, "Warning: you may only set %d notesid.\n", MAX_NOTESID);
				return DAERR_BADPARAM;
			}
			strncpy(pOptions->szNotesId[pOptions->wNotesIdNum], pstrValue, VT_MAX_FILEPATH);
			pOptions->szNotesId[pOptions->wNotesIdNum][VT_MAX_FILEPATH - 1] = '\0';
			pOptions->wNotesIdNum++;
			break;

		case SAMPLE_OPTION_LOTUSNOTESDIRECTORY:
			if (strlen(pstrValue) >= sizeof(pOptions->szLotusNotesDirectory))
				return SCCERR_INSUFFICIENTBUFFER;

			strncpy(pOptions->szLotusNotesDirectory, pstrValue, VT_MAX_FILEPATH);
			pOptions->szLotusNotesDirectory[VT_MAX_FILEPATH - 1] = '\0';
			if (pOptions->bPrintOpts)
				printf("Lotus Notes directory:\t\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SSSHOWHIDDENCELLS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Show Hidden SS Cells", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bSSShowHiddenCells = dwValue;
			if (pOptions->bPrintOpts)
				printf("Show Hidden SS Cells:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_GENERATESYSTEMMETADATA:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"generate system metadata flag", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
			{
				pOptions->dwSearchMLFlags |= SCCEX_IND_GENERATESYSTEMMETADATA;
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_GENERATESYSTEMMETADATA;
			}
			else
			{
				pOptions->dwSearchMLFlags &= ~SCCEX_IND_GENERATESYSTEMMETADATA;
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_GENERATESYSTEMMETADATA;
			}
			if (pOptions->bPrintOpts)
				printf("Generate System Metadata:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_UNIXOPTIONSFILE:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"unix options file", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bPrintUnixOptionsFile = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Print unix options file:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PROCESSARCHIVESUBDOCS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"process archive subdocs", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_PROCESSARCHIVESUBDOCS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_PROCESSARCHIVESUBDOCS;
			if (pOptions->bPrintOpts)
				printf("Process archive subdocs:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PROCESSEMBEDDINGSUBDOCS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"process embedding subdocs", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_PROCESSEMBEDDINGSUBDOCS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_PROCESSEMBEDDINGSUBDOCS;
			if (pOptions->bPrintOpts)
				printf("Process embedding subdocs:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PROCESSATTACHMENTSUBDOCS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"process attachment subdocs", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwCCFlexFormatOptions |= CCFLEX_FORMATOPTIONS_PROCESSATTACHMENTSUBDOCS;
			else
				pOptions->dwCCFlexFormatOptions &= ~CCFLEX_FORMATOPTIONS_PROCESSATTACHMENTSUBDOCS;
			if (pOptions->bPrintOpts)
				printf("Process attachment subdocs:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PDF_FILTER_REORDER_BIDI:
			seResult = StringToDWEx(YesNo, pstrValue, &pOptions->dwPDF_Filter_Reorder_BIDI, (char *)"process BIDI text in the PDF filter", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("PDF Filter Reorder BIDI:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_PDF_FILTER_DROPHYPHENS:
			seResult = StringToDWEx(YesNo, pstrValue, &pOptions->dwPDF_Filter_DropHyphens, (char *)"Drop Hyphens at the end of line in PDF Document", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (pOptions->bPrintOpts)
				printf("PDF Filter Drop Hyphens:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_STRICTFILEACCESS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"strict file access", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwGenFlags |= SCCOPT_FLAGS_STRICTFILEACCESS;
			else
				pOptions->dwGenFlags &= ~SCCOPT_FLAGS_STRICTFILEACCESS;
			if (pOptions->bPrintOpts)
				printf("Strict file access:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_HTML_COND_COMMENT_MODE:
			{
				VTDWORD dwOptValue = 0;
				seResult = StringToDWEx(HtmlCondCommentMode, pstrValue, &dwOptValue, (char *)"HTML filter conditional comment mode", dwLine);

				/* this allows the cfg file to specify multiple values for this option, with each successive value being or-ed into the option.*/
				pOptions->dwHtmlCondCommentMode |= dwOptValue;

				if (seResult != SCCERR_OK)
					return seResult;

				if (pOptions->bPrintOpts)
					printf("HTML filter conditional comment mode:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_LOAD_OPTIONS:
			{
				seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"option load", dwLine);
				if (seResult != SCCERR_OK)
					return seResult;

				pOptions->bLoadOptions = (dwValue == 1);
				if (pOptions->bPrintOpts)
					printf("Option Load:\t\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_SAVE_OPTIONS:
			{
				seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"option save", dwLine);
				if (seResult != SCCERR_OK)
					return seResult;

				pOptions->bSaveOptions = (dwValue == 1);
				if (pOptions->bPrintOpts)
					printf("Option Save:\t\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_EXPORT_STATUS_TYPE:
			{
				seResult = StringToDWEx(ExportStatusType, pstrValue, &dwValue, (char *)"EXExportStatus type", dwLine);
				if (seResult != SCCERR_OK)
					return seResult;

				pOptions->dwExportStatusType = dwValue;
				if (pOptions->bPrintOpts)
					printf("EXExportStatus Type:\t\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_MAILHEADERFLAGS:
			sscanf(pstrValue, "%d", &pOptions->dwMailHeaderFlags);
			break;

		case SAMPLE_OPTION_MAILHEADERVISIBLE:
			{
				seResult = StringToDWEx(EmailHeaderId, pstrValue, &dwValue, (char *)"Email Header", dwLine);
				if (seResult != SCCERR_OK)
				{
					sscanf(pstrValue, "%d", &dwValue);
					if ((dwValue >= NONSTANDARD_HEADER_ID_BASE) && (dwValue < MAX_NONSTANDARD_HEADERS))
						seResult = SCCERR_OK;
				}
				if (seResult == SCCERR_OK)
				{
					PEMAILHEADERINFO pInfo = (PEMAILHEADERINFO)malloc(sizeof(EMAILHEADERINFO));
					pInfo->next = NULL;
					pInfo->sHeader.dwHeaderID = dwValue;
					pInfo->sHeader.dwSubtypeID = pOptions->dwMailHeaderFlags;
					pInfo->sHeader.wsMimeHeaderLabel[0] = 0;
					pInfo->sHeader.wsMimeHeaderName[0] = 0;
					if (pOptions->sMailHeader.pCurVis)
					{
						pOptions->sMailHeader.pCurVis->next = pInfo;
						pOptions->sMailHeader.pCurVis = pInfo;
					}
					else
						pOptions->sMailHeader.pHeadVis = pOptions->sMailHeader.pCurVis = pInfo;
					pOptions->bHeaderVisible = TRUE;
				}
			}
			break;

		case SAMPLE_OPTION_MAILHEADERHIDDEN:
			{
				seResult = StringToDWEx(EmailHeaderId, pstrValue, &dwValue, (char *)"Email Header", dwLine);
				if (seResult != SCCERR_OK)
				{
					sscanf(pstrValue, "%d", &dwValue);
					if ((dwValue >= NONSTANDARD_HEADER_ID_BASE) && (dwValue < MAX_NONSTANDARD_HEADERS))
						seResult = SCCERR_OK;
				}
				if (seResult == SCCERR_OK)
				{
					PEMAILHEADERINFO pInfo = (PEMAILHEADERINFO)malloc(sizeof(EMAILHEADERINFO));
					pInfo->next = NULL;
					pInfo->sHeader.dwHeaderID = dwValue;
					pInfo->sHeader.dwSubtypeID = pOptions->dwMailHeaderFlags;
					pInfo->sHeader.wsMimeHeaderLabel[0] = 0;
					pInfo->sHeader.wsMimeHeaderName[0] = 0;
					if (pOptions->sMailHeader.pCurHid)
					{
						pOptions->sMailHeader.pCurHid->next = pInfo;
						pOptions->sMailHeader.pCurHid = pInfo;
					}
					else
						pOptions->sMailHeader.pHeadHid = pOptions->sMailHeader.pCurHid = pInfo;
					pOptions->bHeaderVisible = FALSE;
				}
			}
			break;

		case SAMPLE_OPTION_MAILHEADERNAME:
			{
				size_t locCnt;
				size_t locLen = strlen(pstrValue);
				if (pOptions->bHeaderVisible)
				{
					if (pOptions->sMailHeader.pCurVis)
					{
						for (locCnt = 0; locCnt < locLen; ++locCnt)
							pOptions->sMailHeader.pCurVis->sHeader.wsMimeHeaderName[locCnt] = pstrValue[locCnt];
						pOptions->sMailHeader.pCurVis->sHeader.wsMimeHeaderName[locCnt] = 0;
					}

				}
				else
				{
					if (pOptions->sMailHeader.pCurHid)
					{
						for (locCnt = 0; locCnt < locLen; ++locCnt)
							pOptions->sMailHeader.pCurHid->sHeader.wsMimeHeaderName[locCnt] = pstrValue[locCnt];
						pOptions->sMailHeader.pCurHid->sHeader.wsMimeHeaderName[locCnt] = 0;
					}
				}
			}
			break;

		case SAMPLE_OPTION_MAILHEADERLABEL:
			{
				size_t locCnt;
				size_t locLen = strlen(pstrValue);
				if (pOptions->bHeaderVisible)
				{
					if (pOptions->sMailHeader.pCurVis)
					{
						for (locCnt = 0; locCnt < locLen; ++locCnt)
							pOptions->sMailHeader.pCurVis->sHeader.wsMimeHeaderLabel[locCnt] = pstrValue[locCnt];
						pOptions->sMailHeader.pCurVis->sHeader.wsMimeHeaderLabel[locCnt] = 0;
					}

				}
				else
				{
					if (pOptions->sMailHeader.pCurHid)
					{
						for (locCnt = 0; locCnt < locLen; ++locCnt)
							pOptions->sMailHeader.pCurHid->sHeader.wsMimeHeaderLabel[locCnt] = pstrValue[locCnt];
						pOptions->sMailHeader.pCurHid->sHeader.wsMimeHeaderLabel[locCnt] = 0;
					}
				}
			}
			break;

		case SAMPLE_OPTION_CELLHIDDEN:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"produce hidden cell attribute", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			if (dwValue == 1)
				pOptions->dwSearchMLFlags |= SCCEX_IND_SS_CELLHIDDEN;
			else
				pOptions->dwSearchMLFlags &= ~SCCEX_IND_SS_CELLHIDDEN;
			if (pOptions->bPrintOpts)
				printf("Produce hidden cell attribute:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_OUTPUT_STRUCTURE:
			seResult = StringToDWEx(OutputStructure, pstrValue, &dwValue, (char *)"Output Structure", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwOutputStructure = dwValue;
			if (pOptions->bPrintOpts)
				printf("Output Structure:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_RAWTEXT:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"Raw Text", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bRawText = (VTBOOL)dwValue;
			if (pOptions->bPrintOpts)
				printf("Raw Text:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FONTPERMISSIONS:
			seResult = StringToDWEx(FontPermissions, pstrValue, &dwValue, (char *)"Font Permissions", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwFontPermissions |= (VTDWORD)dwValue;
			if (pOptions->bPrintOpts)
				printf("Font Permissions:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_FONTREFERENCEMETHOD:
			seResult = StringToDWEx(FontEmbedType, pstrValue, &dwValue, (char *)"Font Reference Method", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwFontReferenceMethod = (VTDWORD)dwValue;
			if (pOptions->bPrintOpts)
				printf("Font Reference Method:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_ATTACHMENTHANDLING:
			seResult = StringToDWEx(AttachmentHandling, pstrValue, &dwValue, (char *)"Email Attachment Handling", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwEmailAttachmentHandling = (VTDWORD)dwValue;
			if (pOptions->bPrintOpts)
				printf("Email AttachmentHandling:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_URLPATH_RESOURCES:
			{
				VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
				if (pOptions->pszResourceURL)
					free(pOptions->pszResourceURL);
				pOptions->pszResourceURL = (VTBYTE *)malloc(dwSize);
				strncpy((VTCHAR *)pOptions->pszResourceURL, pstrValue, dwSize);
				pOptions->pszResourceURL[dwSize - 1] = '\0';
				if (pOptions->bPrintOpts)
					printf("Resource URL:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_URLPATH_OUTPUT:
			{
				VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
				if (pOptions->pszOutputURL)
					free(pOptions->pszOutputURL);
				pOptions->pszOutputURL = (VTBYTE *)malloc(dwSize);
				strncpy((VTCHAR *)pOptions->pszOutputURL, pstrValue, dwSize);
				pOptions->pszOutputURL[dwSize - 1] = '\0';
				if (pOptions->bPrintOpts)
					printf("Output URL:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_FONTBASEURL:
			{
				VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
				if (pOptions->pszBaseURL)
					free(pOptions->pszBaseURL);
				pOptions->pszBaseURL = (VTBYTE *)malloc(dwSize);
				strncpy((VTCHAR *)pOptions->pszBaseURL, pstrValue, dwSize);
				pOptions->pszBaseURL[dwSize - 1] = '\0';
				if (pOptions->bPrintOpts)
					printf("Resource URL:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_WVLIBRARYNAME:
			{
				VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
				if (pOptions->pszLibraryName)
					free(pOptions->pszLibraryName);
				pOptions->pszLibraryName = (VTBYTE *)malloc(dwSize);
				strncpy((VTCHAR *)pOptions->pszLibraryName, pstrValue, dwSize);
				pOptions->pszLibraryName[dwSize - 1] = '\0';
				if (pOptions->bPrintOpts)
					printf("WV Library Name:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_WVSTYLESHEETNAME:
			{
				VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
				if (pOptions->pszStylesheetName)
					free(pOptions->pszStylesheetName);
				pOptions->pszStylesheetName = (VTBYTE *)malloc(dwSize);
				strncpy((VTCHAR *)pOptions->pszStylesheetName, pstrValue, dwSize);
				pOptions->pszStylesheetName[dwSize - 1] = '\0';
				if (pOptions->bPrintOpts)
					printf("WV Stylesheet Name:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_EXTERNAL_CSS:
			{
				VTBOOL bSuccess = FALSE;
				if (pOptions->ppExternalCss == NULL)
				{
					pOptions->ppExternalCss = (VTBYTE **)malloc(sizeof(VTBYTE *));
					if (pOptions->ppExternalCss)
						bSuccess = TRUE;
				}
				else
				{
					VTBYTE **tmp = (VTBYTE **)realloc(pOptions->ppExternalCss, (pOptions->dwCountExternalCss + 1) * sizeof(VTBYTE *));
					if (tmp)
					{
						pOptions->ppExternalCss = tmp;
						bSuccess = TRUE;
					}
				}
				if (bSuccess)
				{
					VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
					pOptions->ppExternalCss[pOptions->dwCountExternalCss] = (VTBYTE *)malloc(dwSize);
					if (pOptions->ppExternalCss[pOptions->dwCountExternalCss])
					{
						strncpy((VTCHAR *)pOptions->ppExternalCss[pOptions->dwCountExternalCss], pstrValue, dwSize);
						pOptions->ppExternalCss[pOptions->dwCountExternalCss][dwSize - 1] = '\0';
						++pOptions->dwCountExternalCss;
					}
				}
				if (pOptions->bPrintOpts)
					printf("External Stylesheet:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_POST_LIBRARY_SCRIPT:
			{
				VTBOOL bSuccess = FALSE;
				if (pOptions->ppPostLibScript == NULL)
				{
					pOptions->ppPostLibScript = (VTBYTE **)malloc(sizeof(VTBYTE *));
					if (pOptions->ppPostLibScript)
						bSuccess = TRUE;
				}
				else
				{
					VTBYTE **tmp = (VTBYTE **)realloc(pOptions->ppPostLibScript,
													  (pOptions->dwCountPostLibScript + 1) * sizeof(VTBYTE *));
					if (tmp)
					{
						pOptions->ppPostLibScript = tmp;
						bSuccess = TRUE;
					}
				}
				if (bSuccess)
				{
					VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
					pOptions->ppPostLibScript[pOptions->dwCountPostLibScript] = (VTBYTE *)malloc(dwSize);
					if (pOptions->ppPostLibScript[pOptions->dwCountPostLibScript])
					{
						strncpy((VTCHAR *)pOptions->ppPostLibScript[pOptions->dwCountPostLibScript], pstrValue, dwSize);
						pOptions->ppPostLibScript[pOptions->dwCountPostLibScript][dwSize - 1] = '\0';
						++pOptions->dwCountPostLibScript;
					}
				}
				if (pOptions->bPrintOpts)
					printf("Post Library Script path:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_PRE_LIBRARY_SCRIPT:
			{
				VTBOOL bSuccess = FALSE;
				if (pOptions->ppPreLibScript == NULL)
				{
					pOptions->ppPreLibScript = (VTBYTE **)malloc(sizeof(VTBYTE *));
					if (pOptions->ppPreLibScript)
						bSuccess = TRUE;
				}
				else
				{
					VTBYTE **tmp = (VTBYTE **)realloc(pOptions->ppPreLibScript, (pOptions->dwCountPreLibScript + 1) * sizeof(VTBYTE *));
					if (tmp)
					{
						pOptions->ppPreLibScript = tmp;
						bSuccess = TRUE;
					}
				}
				if (bSuccess)
				{
					VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
					pOptions->ppPreLibScript[pOptions->dwCountPreLibScript] = (VTBYTE *)malloc(dwSize);
					if (pOptions->ppPreLibScript[pOptions->dwCountPreLibScript])
					{
						strncpy((VTCHAR *)pOptions->ppPreLibScript[pOptions->dwCountPreLibScript], pstrValue, dwSize);
						pOptions->ppPreLibScript[pOptions->dwCountPreLibScript][dwSize - 1] = '\0';
						++pOptions->dwCountPreLibScript;
					}
				}
				if (pOptions->bPrintOpts)
					printf("Pre Library Script path:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_IMAGESTAMP_SPECTYPE:
			seResult = StringToDWEx(ImageStampSpecTypes, pstrValue, &dwValue, (char *)"Image stamp spec type", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->dwImageStampSpecType = (VTDWORD)dwValue;
			if (pOptions->bPrintOpts)
				printf("Image stamp spec type:\t\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_IMAGESTAMP_NAME:
			{
				VTDWORD dwSize = (VTDWORD)strlen(pstrValue);
				if (pOptions->pszImageStampName)
				{
					free(pOptions->pszImageStampName);
					pOptions->pszImageStampName = NULL;
				}
				if (dwSize)
				{
					int mult = 1;
					if (pOptions->dwImageStampSpecType == IOTYPE_UNICODEPATH)
						mult = 2;
					pOptions->pszImageStampName = (VTBYTE *)malloc((dwSize + 1) * mult);
					if (pOptions->pszImageStampName)
					{
						memcpy(pOptions->pszImageStampName, pstrValue, dwSize * mult);
						pOptions->pszImageStampName[dwSize * mult] = 0;
						if (mult == 2)
							pOptions->pszImageStampName[dwSize * mult + 1] = 0;
					}

				}
			}
			break;

		case SAMPLE_OPTION_IMAGESTAMP_FILE:
		case SAMPLE_OPTION_IMAGESTAMP_URL:
			{
				PEXANNOSTAMPIMAGE pIN = NULL;
				if (dwOptCode == SAMPLE_OPTION_IMAGESTAMP_FILE)
				{
					if (pOptions->ppImageStampFile == NULL)
					{
						pOptions->ppImageStampFile = (EXANNOSTAMPIMAGE **)malloc(sizeof(EXANNOSTAMPIMAGE *));
					}
					else
					{
						EXANNOSTAMPIMAGE **tmp = (EXANNOSTAMPIMAGE **)realloc(pOptions->ppImageStampFile, (pOptions->dwCountImageStampFile + 1) * sizeof(EXANNOSTAMPIMAGE *));
						if (tmp)
							pOptions->ppImageStampFile = tmp;
					}
					if (pOptions->ppImageStampFile)
					{
						pOptions->ppImageStampFile[pOptions->dwCountImageStampFile] = (EXANNOSTAMPIMAGE *)malloc(sizeof(EXANNOSTAMPIMAGE));
						if (pOptions->ppImageStampFile[pOptions->dwCountImageStampFile])
						{
							pIN = (PEXANNOSTAMPIMAGE)pOptions->ppImageStampFile[pOptions->dwCountImageStampFile];
							++pOptions->dwCountImageStampFile;
						}
					}
				}
				else
				{
					if (pOptions->ppImageStampUrl == NULL)
					{
						pOptions->ppImageStampUrl = (EXANNOSTAMPIMAGE **)malloc(sizeof(EXANNOSTAMPIMAGE *));
					}
					else
					{
						EXANNOSTAMPIMAGE **tmp = (EXANNOSTAMPIMAGE **)realloc(pOptions->ppImageStampUrl, (pOptions->dwCountImageStampUrl + 1) * sizeof(EXANNOSTAMPIMAGE *));
						if (tmp)
							pOptions->ppImageStampUrl = tmp;
					}
					if (pOptions->ppImageStampUrl)
					{
						pOptions->ppImageStampUrl[pOptions->dwCountImageStampUrl] = (EXANNOSTAMPIMAGE *)malloc(sizeof(EXANNOSTAMPIMAGE));
						if (pOptions->ppImageStampUrl[pOptions->dwCountImageStampUrl])
						{
							pIN = (PEXANNOSTAMPIMAGE)pOptions->ppImageStampUrl[pOptions->dwCountImageStampUrl];
							++pOptions->dwCountImageStampUrl;
						}
					}
				}
				if (pIN)
				{
					VTDWORD dwImageSize = (VTDWORD)strlen(pstrValue);
					int mult = 1;
					VTBYTE *pTmp;

					pIN->dwSpecType = 0;
					pIN->pStampImage = NULL;
					pIN->szStampName = NULL;
					if (pOptions->pszImageStampName)
					{
						VTDWORD dwNameSize = (VTDWORD)strlen((VTCHAR *)pOptions->pszImageStampName) + 1;
						pTmp = (VTBYTE *)malloc(dwNameSize);
						if (pTmp)
						{
							pIN->szStampName = pTmp;
							strncpy((VTCHAR *)pTmp, (VTCHAR *)pOptions->pszImageStampName, dwNameSize);
							pTmp[dwNameSize - 1] = '\0';
						}
						free(pOptions->pszImageStampName);
						pOptions->pszImageStampName = NULL;
					}
					if (pOptions->dwImageStampSpecType == IOTYPE_UNICODEPATH)
						mult = 2;
					pTmp = (VTBYTE *)malloc((dwImageSize + 1) * mult);
					if (pTmp)
					{
						memcpy(pTmp, pstrValue, dwImageSize * mult);
						pTmp[dwImageSize * mult] = 0;
						if (mult == 2)
							pTmp[dwImageSize * mult + 1] = 0;
						pIN->pStampImage = pTmp;
					}
					pIN->dwSpecType = pOptions->dwImageStampSpecType;
					pOptions->dwImageStampSpecType = 0;
				}
				if (pOptions->bPrintOpts)
					printf("Image Stamp File:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_REDACTIONCOLOR:
			sscanf(pstrValue, "%d", &pOptions->dwRedactionColor);
			if (pOptions->bPrintOpts)
				printf("Printed Redaction Color:\t\t%s\n", pstrValue);
			break;

		case SAMPLE_OPTION_SHOWREDACTIONLABELS:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"show redaction labels", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bShowRedactionLabels = (VTBOOL)dwValue;
			break;

		case SAMPLE_OPTION_REDACTIONS_ENABLED:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"enable redactions", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bEnableRedactions = (VTBOOL)dwValue;
			break;

		case SAMPLE_OPTION_REDACTION_LABEL_FONT_NAME:
			{
				VTDWORD dwSize = (VTDWORD)strlen(pstrValue) + 1;
				VTDWORD myCount;

				if (pOptions->pwzRedactionLabelFontName)
					free(pOptions->pwzRedactionLabelFontName);
				pOptions->pwzRedactionLabelFontName = (VTWORD *)malloc(dwSize * sizeof(VTWORD));
				for (myCount = 0; myCount < dwSize; ++myCount)
					pOptions->pwzRedactionLabelFontName[myCount] = (VTWORD)pstrValue[myCount];
				pOptions->pwzRedactionLabelFontName[dwSize] = '\0';
				if (pOptions->bPrintOpts)
					printf("Redaction Label Font:\t\t%s\n", pstrValue);
			}
			break;

		case SAMPLE_OPTION_VECTOROBJECTLIMIT:
			if (sscanf(pstrValue, "%d", &pOptions->dwVectorObjectLimit) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid vector object limit (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Vector Object Limit:\t\t%s\n", pstrValue);
			break;

        case SAMPLE_OPTION_PDF_FILTER_WORD_DELIM_FRACTION:
            sscanf(pstrValue, "%f", &pOptions->fPdfWordDelimFraction);
			if (pOptions->bPrintOpts)
				printf("PDF Word Delimiter Fraction:\t\t%s\n", pstrValue);
            break;

        case SAMPLE_OPTION_PDF_FILTER_MAX_EMBEDDED_OBJECTS:
            sscanf(pstrValue, "%u", &pOptions->dwPdfMaxEmbeddedObjects);
			if (pOptions->bPrintOpts)
				printf("PDF Max Embeddings:\t\t%s\n", pstrValue);
            break;

        case SAMPLE_OPTION_PDF_FILTER_MAX_VECTOR_PATHS:
            sscanf(pstrValue, "%u", &pOptions->dwPdfMaxVectorPaths);
			if (pOptions->bPrintOpts)
				printf("PDF Max Vector Paths:\t\t%s\n", pstrValue);
            break;

		case SAMPLE_OPTION_EMAIL_FIXEDWIDTH:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"fixed width email", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bEmailFixedWidth = (VTBOOL)dwValue;
			break;

		case SAMPLE_OPTION_HTML_FIXEDWIDTH:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"fixed width html", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bHTMLFixedWidth = (VTBOOL)dwValue;
			break;

		case SAMPLE_OPTION_PLAINTEXT_PAGINATION:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"plain text pagination", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;

			pOptions->bPlainTextPagination = (VTBOOL)dwValue;
			break;

		case SAMPLE_OPTION_PAGES_PER_FILE:
			if (sscanf(pstrValue, "%d", &pOptions->dwPagesPerFile) != 1)
			{
#ifndef DISABLE_REPORTING
				fprintf(stderr, "Error, line %d: Invalid number of pages per file (%s)\n", dwLine, pstrValue);
#endif
			}
			if (pOptions->bPrintOpts)
				printf("Pages per file:\t\t%s\n", pstrValue);
			break;

		/*tvarshne Pdf Space Improvement Plan-1 11/28/2020*/
		case SAMPLE_OPTION_PDF_FILTER_NEW_SPACING_ALGORITHM:
			seResult = StringToDWEx(YesNo, pstrValue, &dwValue, (char *)"pdf new spacing", dwLine);
			if (seResult != SCCERR_OK)
				return seResult;
			pOptions->bNewPDFspacing = (VTBOOL)dwValue;
			break;

	}
	pOptions->abSetOptions[dwOptCode] = TRUE;
	return seResult;
}




/*
|
|   function:   StringToDWEx
|   parameters: StringDW    *array
|               char        *Str
|               VTDWORD     *Num
|               char        *ErrMsg
|               VTDWORD     dwLine
|   returns:    SCCERR
|
|   purpose:    converts strings into VTDWORD values we can use to set otions
|
*/

SCCERR StringToDWEx(StringDW *array, VTLPSTR Str, VTDWORD *Num, VTLPSTR ErrMsg, VTDWORD dwLine)
{
	SCCERR seResult;

	seResult = StringToDW(array, Str, Num);

#ifndef DISABLE_REPORTING
	if (seResult != SCCERR_OK && ErrMsg)
	{
		fprintf(stderr, "Warning: line %d: Invalid %s -> %s\n", dwLine, ErrMsg, Str);
	}
#endif

	return(seResult);
}

SCCERR StringToDW(StringDW *array, VTLPSTR keyword, VTLPDWORD dwOptCode)
{
	int i;
	VTLPSTR tempString = NULL;
	int j, k;


	SCCERR seResult = SCCERR_OK;

	/* Convert Str to lower case. */
	for (i = 0; keyword[i] != '\0'; i++)
	{
		if (isupper(keyword[i]))
			keyword[i] = (VTCHAR)tolower(keyword[i]);
	}

	/*
	|   Scan through the input array of string/number pairs for the
	|   input string.  Return the number part of the string/number
	|   pair in *Num.  Function returns 0 if everything was OK.
	*/

	for (i = 0;; i++)
	{
		if ((array[i].Str == NULL) && (array[i].Num == STRINGDW_LISTEND))
		{
			seResult = SCCERR_BADPARAM;
			break;
		}

		if (array[i].Str != NULL)
		{

			if (strcmp(array[i].Str, keyword) == 0)
			{
				*dwOptCode = array[i].Num;
				break;
			}
			else
			{
				if (tempString == NULL)
				{
					tempString = (VTLPSTR)malloc(strlen(keyword) + 2);
					memset(tempString, 0, strlen(keyword) + 2);
					k = 0;

					/* get rid of the space */
					for (j = 0; keyword[j] != '\0'; j++)
					{
						while (keyword[j] == ' ')
							j++;
						tempString[k] = keyword[j];
						k++;
					}
				}
				if (strcmp(array[i].Str, tempString) == 0)
				{
					*dwOptCode = array[i].Num;
					break;
				}
			}
		}
	}
	if (tempString)
		free(tempString);

	return seResult;
}



SCCERR StringToWideStr(StringWS *array, VTLPSTR keyword, VTLPCWSTR *ppOption)
{
	int i;
	SCCERR seResult = SCCERR_BADPARAM;

	for (i = 0; *array[i].Str != '\0'; i++)
	{
#ifdef UNIX
		if (strcasecmp(array[i].Str, keyword) == 0)
#else
		if (strcmpi(array[i].Str, keyword) == 0)
#endif
		{
			*ppOption = array[i].pwStr;
			seResult = SCCERR_OK;
			break;
		}
	}
	return seResult;
}




VTBOOL DWToString(const StringDW* array, const VTDWORD dwId, VTLPSTR string)
{
	int i,j;

	for (i = 0;; i++)
	{
		if ((array[i].Str == NULL) && (array[i].Num == STRINGDW_LISTEND))
		{
			break;
		}

		if (array[i].Num == dwId)
		{
			if (array[i].Str == NULL)
				return FALSE;

			/*copies the buffer into destination string*/
			/*The destination string size MUST NOT be less than the source buffer size*/
			/*size check for destination string is not done because of function signature limitation*/
			for (j = 0;; j++)
			{

				string[j] = (array[i].Str)[j];

				if ((array[i].Str)[j] == '\0')
					break;
			}

			return TRUE;
		}
	}

	return FALSE;
}




/*
|
|   function:   PrintOpts
|   parameters: Option  *pOptions
|   returns:    VTVOID
|
|   purpose:    This one prints out the contents of the Options struct it takes
|               as its parameter if PRINT_OPTIONS is defined.
|
*/

VTVOID PrintOpts(Option *pOptions)
{
#ifdef PRINT_OPTIONS
	int  i;

	/* Print the values of the configuration options. */
	if (*pOptions->szTemplate)
	{
		printf("Template name:\t\t\t\"%s\"\n", pOptions->szTemplate);
	}
	else
	{
		printf("Template name:\t\t\t\"Using the built in system default template.\"\n");
	}
	for (i = 0; *Flavors[i].Str != '\0'; i++)
	{
		if (pOptions->dwFlavor == Flavors[i].Num)
		{
			printf("HTML Flavor:\t\t\t%s\n", Flavors[i].Str);
			break;
		}
		if (Flavors[i + 1].Str == NULL)
			break;
	}

	for (i = 0; *GraphicTypes[i].Str != '\0'; i++)
	{
		if (pOptions->dwGraphicType == GraphicTypes[i].Num)
		{
			printf("Graphics type:\t\t\t%s\n", GraphicTypes[i].Str);
			break;
		}
		if (GraphicTypes[i + 1].Str == NULL)
			break;
	}

	printf("Output JPEG quality:\t\t%d (100 = Highest quality, lowest compression)\n", pOptions->dwJPEGQuality);

	for (i = 0; *Charsets[i].Str != '\0'; i++)
	{
		if (pOptions->dwCharset == Charsets[i].Num)
		{
			printf("Output charset:\t\t\t%s\n", Charsets[i].Str);
			break;
		}
		if (Charsets[i + 1].Str == NULL)
			break;
	}

	if (pOptions->bInterlacedGIF)
	{
		printf("Output GIFs will be interlaced\n");
	}
	else
	{
		printf("Output GIFs will be non-interlaced\n");
	}
	printf("\n");

	if ((pOptions->dwFlags & SCCEX_CFLAG_STRICTDTD) != 0)
	{
		printf("HTML will be written to strict compliance with the flavor's DTD\n");
	}
	else
	{
		printf("HTML will be written that looks best in browsers\n");
	}
	printf("\n");
#else
	UNUSED(pOptions);
#endif    /* PRINT_OPTIONS */

}




VTDWORD ConvertASCIIToLittleUnicode(VTLPVOID pOut, VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize)
{
	VTLPBYTE pInput = (VTLPBYTE)pIn;
	VTLPBYTE pOutput = (VTLPBYTE)pOut;
	VTLPBYTE pEnd = pInput + dwInSize;
	VTDWORD  dwOutputBytes = 0;

	dwOutSize -= sizeof(VTWORD); /* subtract null character */
	while (pInput < pEnd)
	{
		dwOutputBytes += 2;
		if (dwOutputBytes > dwOutSize)
			break;

		*pOutput++ = *pInput++;
		*pOutput++ = 0;
	}
	*(VTLPWSTR)pOutput = 0;

	if (pInput < pEnd)
		dwOutputBytes = (VTDWORD)-1;

	return dwOutputBytes;
}




VTDWORD ConvertASCIIToBigUnicode(VTLPVOID pOut, VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize)
{
	VTLPBYTE pInput = (VTLPBYTE)pIn;
	VTLPBYTE pOutput = (VTLPBYTE)pOut;
	VTLPBYTE pEnd = pInput + dwInSize;
	VTDWORD  dwOutputBytes = 0;

	dwOutSize -= sizeof(VTWORD); /* subtract null character */
	while (pInput < pEnd)
	{
		dwOutputBytes += 2;
		if (dwOutputBytes > dwOutSize)
			break;

		*pOutput++ = 0;
		*pOutput++ = *pInput++;
	}
	*(VTLPWSTR)pOutput = 0;

	if (pInput < pEnd)
		dwOutputBytes = (VTDWORD)-1;
	return dwOutputBytes;
}




VTDWORD ConvertLittleUnicodeToASCII(VTLPVOID pOut, VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize)
{
	VTLPBYTE pInput = (VTLPBYTE)pIn;
	VTLPBYTE pOutput = (VTLPBYTE)pOut;
	VTLPBYTE pEnd = pInput + dwInSize;
	VTDWORD  dwOutputBytes = 0;

	dwOutSize -= sizeof(VTBYTE); /* subtract null character */
	while (pInput < pEnd)
	{
		dwOutputBytes++;
		if (dwOutputBytes > dwOutSize)
			break;

		*pOutput++ = *pInput;
		pInput += 2;
	}
	*pOutput = 0;

	if (pInput < pEnd)
		dwOutputBytes = (VTDWORD)-1;

	return dwOutputBytes;
}




VTDWORD ConvertBigUnicodeToASCII(VTLPVOID pOut, VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize)
{
	VTLPBYTE pInput = (VTLPBYTE)pIn;
	VTLPBYTE pOutput = (VTLPBYTE)pOut;
	VTLPBYTE pEnd = pInput + dwInSize;
	VTDWORD  dwOutputBytes = 0;

	dwOutSize -= sizeof(VTBYTE); /* subtract null character */
	while (pInput < pEnd)
	{
		dwOutputBytes++;
		if (dwOutputBytes > dwOutSize)
			break;

		*pOutput++ = *(pInput + 1);
		pInput += 2;
	}
	*pOutput = 0;

	if (pInput < pEnd)
		dwOutputBytes = (VTDWORD)-1;

	return dwOutputBytes;
}

VTDWORD ConvertUnicodeToUTF8(VTLPVOID pOut, VTDWORD dwOutSize, VTLPVOID pIn, VTDWORD dwInSize)
{
	VTLPWORD pInStr = (VTLPWORD)pIn;
	VTLPBYTE pOutStr = (VTLPBYTE)pOut;
	VTDWORD codepoint = 0;

	UNUSED(dwInSize);

	for (pInStr; (*pInStr != 0x00) && ((VTDWORD)(pOutStr - (VTLPBYTE)pOut) < dwOutSize - 1); ++pInStr)
	{
		if (*pInStr >= 0xD800 && *pInStr <= 0xDBFF)
			codepoint = ((*pInStr - 0xDBFF) << 10) + 0x10000;
		else
		{
			if (*pInStr >= 0xDC00 && *pInStr <= 0xDFFF)
				codepoint |= *pInStr - 0xDC00;
			else
				codepoint = *pInStr;

			if (codepoint <= 0x7f)
			{
				*pOutStr = (VTBYTE)codepoint; pOutStr++;
			}
			else if (codepoint <= 0x7FF)
			{
				*pOutStr = (VTBYTE)(0xC0 | ((codepoint >> 6) & 0x1F)); pOutStr++;
				*pOutStr = (VTBYTE)(0x80 | (codepoint & 0x3F));        pOutStr++;
			}
			else if (codepoint <= 0xffff)
			{
				*pOutStr = (VTBYTE)(0xE0 | ((codepoint >> 12) & 0x0F)); pOutStr++;
				*pOutStr = (VTBYTE)(0x80 | ((codepoint >> 6) & 0x3F)); pOutStr++;
				*pOutStr = (VTBYTE)(0x80 | (codepoint & 0x3F));        pOutStr++;
			}
			else
			{
				*pOutStr = (VTBYTE)(0xF0 | ((codepoint >> 18) & 0x07)); pOutStr++;
				*pOutStr = (VTBYTE)(0x80 | ((codepoint >> 12) & 0x3F)); pOutStr++;
				*pOutStr = (VTBYTE)(0x80 | ((codepoint >> 6) & 0x3F)); pOutStr++;
				*pOutStr = (VTBYTE)(0x80 | (codepoint & 0x3F));         pOutStr++;
			}
		}
	}
	*pOutStr = 0x00;

	return ((VTDWORD)(pOutStr - (VTLPBYTE)pOut));
}




VTBYTE GetDigit(char ch)
{
	if (isalpha(ch))
	{
		int digit = tolower(ch);
		return (VTBYTE)(digit - 'a' + 10);
	}
	return (VTBYTE)(ch - '0');
}




/* Convert 4 character hex string to a word, assuming valid input (pString) */
VTVOID StrToWord(VTLPSTR pString, VTLPWORD pwVal)
{
	VTBYTE byNibble;
	VTWORD wMult = 4096;
	int i;

	*pwVal = 0;

	for (i = 0; i < 4; i++)
	{
		byNibble = GetDigit(*pString++);
		*pwVal += byNibble * wMult;
		wMult >>= 4;
	}
}




VTDWORD Swcslen(VTLPCWSTR pwStr)
{
	VTDWORD dwLen;
	for (dwLen = 0; (*pwStr++ != 0); dwLen++);

	return dwLen;
}




/* Base64TranslationTable */
const VTBYTE   abyDecode[128] =
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 62, 0xFF, 0xFF, 0xFF, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/*---------------------------------------------------------------------
 base64decode:  Decodes a base 64 string

 pInput - base64 text (NOT null-terminated)
 pOutput - output buffer
 inSize - size of base64 data in bytes
 pOutSize - pointer to size of output buffer in bytes, will be set to
 the number of bytes of the decoded data.
 If the output buffer was not big enough, this will indicate how big
 it needs to be.

 returns
 BASE64_OK if everything worked
 BASE64_BUFFER_TOO_SMALL if decoded data can't fit in the output buffer
 BASE64_BAD_ENCODING if the encoded data was malformed
 ---------------------------------------------------------------------*/

VTDWORD base64decode(VTLPBYTE pInput, VTLPBYTE pOutput, VTDWORD inSize, VTLPDWORD pOutSize)
{
	VTDWORD dwInput = 0;
	VTDWORD outcount = 0;
	VTBYTE outbuf[3] = {0};
	VTDWORD dwRet = BASE64_OK;
	VTDWORD f;
	VTBYTE ch;

	while (dwInput < inSize)
	{
		ch = abyDecode[pInput[dwInput]];
		if (ch != 0xFF)
		{
			if (dwInput % 4 == 0)
			{
				outbuf[0] = ch << 2;
			}
			else if (dwInput % 4 == 1)
			{
				outbuf[0] = outbuf[0] | (ch >> 4);
				outbuf[1] = (ch & 0x0F) << 4;
			}
			else if (dwInput % 4 == 2)
			{
				outbuf[1] = outbuf[1] | (ch >> 2);
				outbuf[2] = (ch & 0x03) << 6;
			}
			else
				outbuf[2] = outbuf[2] | ch;
		}

		dwInput++;
		if (dwInput % 4 == 0)
		{
			for (f = 0; f < 3; f++)
			{
				if (outcount < *pOutSize)
					pOutput[outcount++] = outbuf[f];
				else
					outcount++;

				outbuf[f] = 0;
			}
		}

	}

	if (dwInput % 4 != 0)
	{
		for (f = 0; f < (dwInput % 4) - 1; f++)
		{
			if (outcount < *pOutSize)
				pOutput[outcount++] = outbuf[f];
			else
				outcount++;

			outbuf[f] = 0;
		}
	}

	if (outcount > *pOutSize)
		dwRet = BASE64_BUFFERTOOSMALL;

	*pOutSize = outcount;

	return dwRet;
}




VTDWORD decodeUnicode(VTLPCWSTR pInput, VTLPWSTR pOutput, VTDWORD inSize, VTLPDWORD pOutSize)
{
	static VTLPCSTR p = "ba";
	VTDWORD ret = base64decode((VTLPBYTE)pInput, (VTLPBYTE)pOutput, inSize, pOutSize);

	if (ret == BASE64_OK)
	{
		/* Switch from big-endian to little-endian, if necessary */
		if (*(VTLPWORD)p != 0x6261)
#ifdef __ANDROID__
		{
			VTDWORD i;
			VTWORD wVal;

			for (i = 0; i < (*pOutSize) / 2; i++)
			{
				wVal = ((pOutput[i] >> 8) & 0x00ff) | ((pOutput[i] << 8) & 0xff00);
				pOutput[i] = wVal;
			}
		}
#else
			swab((char*)pOutput, (char*)pOutput, *pOutSize);
#endif
	}
	return ret;
}




VTVOID SetCustomFileExtension(VTWORD wFI, VTLPSTR value)
{
	VTDWORD i;
	VTLPSTR dest;

	/* Look for an item in the CustomFileExtensions list matching this FI ID */
	for (i = 0; CustomFileExtensions[i].Num != STRINGDW_LISTEND; i++)
	{
		if (CustomFileExtensions[i].Num == wFI)
		{
			/* Allocate a new extension. Add 2 bytes for '.' and '\0' */
			dest = (VTLPSTR)malloc(strlen(value) + 2);
			CustomFileExtensions[i].Str = dest;

			/* Copy the extension value */
			if (value[0] != '.')
				*dest++ = '.';
			strcpy(dest, value);

			break;
		}
	}
}
/* Convert a hex string to a unicode string */
SCCERR  HexStrToUnicodeStr(VTLPSTR pString, VTLPWORD pwVal)
{
	SCCERR locRet = DAERR_BADPARAM;
	VTBOOL  bStartNewChar = FALSE;

	*pwVal = 0xffff;
	while (*pString != 0x00)
	{

		if ((*pString == '\\') && ((*(pString + 1) == 'u') || (*(pString + 1) == 'U')))
			bStartNewChar = TRUE;

		if (bStartNewChar)
		{
			/* skip the \u */
			pString++;
			pString++;
			if (*pwVal == 0xffff)
				*pwVal = 0;
			else
			{
				pwVal++;
				*pwVal = 0;
			}
			bStartNewChar = FALSE;
		}
		if (*pString >= '0' && *pString <= '9')
		{
			locRet = DAERR_OK;
			*pwVal = *pwVal * 16 + *pString - '0';
		}
		if (*pString >= 'a' && *pString <= 'f')
		{
			locRet = DAERR_OK;
			*pwVal = *pwVal * 16 + *pString - 'a' + 10;
		}
		if (*pString >= 'A' && *pString <= 'F')
		{
			locRet = DAERR_OK;
			*pwVal = *pwVal * 16 + *pString - 'A' + 10;
		}
		pString++;

	}
	return(locRet);
}

static VTVOID Cleanup(Option *pOptions)
{
	FREEAR(pOptions->szInFile);
	FREEAR(pOptions->szOutFile);
	FREEAR(pOptions->wzInFile);
	FREEAR(pOptions->wzOutFile);
	FREEAR(pOptions->szTemplate);
	FREEAR(pOptions->wzTemplate);
	FREEAR(pOptions->szFontDirectory);
	FREEAR(g_szInitFilePath);
	FREEAR(g_szInitFileName);
	FREEAR(pOptions->pszResourceURL);
	FREEAR(pOptions->pszOutputURL);
	FREEAR(pOptions->pszBaseURL);
	FREEAR(pOptions->pwzRedactionLabelFontName);
	FREEAR(pOptions->pWatermarkSpec);

	if (pOptions->ppExternalCss)
	{
		VTDWORD i;
		for (i = 0; i < pOptions->dwCountExternalCss; ++i)
			free(pOptions->ppExternalCss[i]);
		free(pOptions->ppExternalCss);
	}

	if (pOptions->ppImageStampFile)
	{
		VTDWORD i;
		for (i = 0; i < pOptions->dwCountImageStampFile; ++i)
		{
			if (pOptions->ppImageStampFile[i]->szStampName)
				free(pOptions->ppImageStampFile[i]->szStampName);
			if (pOptions->ppImageStampFile[i]->pStampImage)
				free(pOptions->ppImageStampFile[i]->pStampImage);
			free(pOptions->ppImageStampFile[i]);
		}
		free(pOptions->ppImageStampFile);
	}

	if (pOptions->ppImageStampUrl)
	{
		VTDWORD i;
		for (i = 0; i < pOptions->dwCountImageStampUrl; ++i)
		{
			if (pOptions->ppImageStampUrl[i]->szStampName)
				free(pOptions->ppImageStampUrl[i]->szStampName);
			if (pOptions->ppImageStampUrl[i]->pStampImage)
				free(pOptions->ppImageStampUrl[i]->pStampImage);
			free(pOptions->ppImageStampUrl[i]);
		}
		free(pOptions->ppImageStampUrl);
	}

	if (pOptions->ppPostLibScript)
	{
		VTDWORD i;
		for (i = 0; i < pOptions->dwCountPostLibScript; ++i)
			free(pOptions->ppPostLibScript[i]);
		free(pOptions->ppPostLibScript);
	}

	if (pOptions->ppPreLibScript)
	{
		VTDWORD i;
		for (i = 0; i < pOptions->dwCountPreLibScript; ++i)
			free(pOptions->ppPreLibScript[i]);
		free(pOptions->ppPreLibScript);
	}

	if (pOptions->FontFilterList.pFontList)
	{
		PFONTNAMELIST pFontList = pOptions->FontFilterList.pFontList;
		PFONTNAMELIST pDeleteFontList;

		while (pFontList)
		{
			pDeleteFontList = pFontList;
			pFontList = pFontList->pNextFont;
			free(pDeleteFontList);
		}
	}

	if (pOptions->FontAlias.pNextFontAlias)
	{
		PFONTALIAS pFontAlias = &pOptions->FontAlias;

		while (pFontAlias->pNextFontAlias)
		{
			while (pFontAlias->pNextFontAlias->pNextFontAlias)
			{
				pFontAlias = pFontAlias->pNextFontAlias;
			}
			free(pFontAlias->pNextFontAlias);
			pFontAlias->pNextFontAlias = NULL;
			pFontAlias = &pOptions->FontAlias;
		}
	}

	if (pOptions->sMailHeader.pHeadVis)
	{
		PEMAILHEADERINFO pInfo = pOptions->sMailHeader.pHeadVis;

		while (pInfo)
		{
			PEMAILHEADERINFO pInfoLast = pInfo;
			pInfo = pInfo->next;
			free(pInfoLast);
		}
	}

	if (pOptions->sMailHeader.pHeadHid)
	{
		PEMAILHEADERINFO pInfo = pOptions->sMailHeader.pHeadHid;

		while (pInfo)
		{
			PEMAILHEADERINFO pInfoLast = pInfo;
			pInfo = pInfo->next;
			free(pInfoLast);
		}
	}
}

VTVOID CleanupOpts(Option *pOptions)
{
	Cleanup(pOptions);
}

