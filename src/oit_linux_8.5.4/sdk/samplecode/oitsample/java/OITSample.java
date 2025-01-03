/*
|   Outside In Export Technologies
|   Sample Exporter
|
|   Copyright (c) 2001, 2016, Oracle and/or its affiliates. All rights reserved.
|
|   You have a royalty-free right to use, modify, reproduce and
|   distribute the Sample Applications (and/or any modified version)
|   in any way you find useful, provided that you agree that Oracle
|   has no warranty obligations or liability for any Sample Application
|   files which are modified.
|
|   This sample application is intended to demonstrate how a Java program 
|   can use the Outside In Export API to export documents.
|
*/


import java.io.File;
import java.io.IOException;

import java.util.LinkedList;
import java.util.HashMap;

import com.oracle.outsidein.Callback;
import com.oracle.outsidein.OutsideIn;
import com.oracle.outsidein.OutsideInException;
import com.oracle.outsidein.FileFormat;
import com.oracle.outsidein.Exporter;

public class OITSample {

	private static String m_TimeZoneText = "UTC";

	public static class SampleCallback extends Callback
	{
		private int m_imageFileCount = 0;
		private File m_ParentFile;
		private static final HashMap<FileFormat, String> m_extensions = new HashMap<FileFormat, String>();

		static
		{
			m_extensions.put(FileFormat.FI_PDF, "pdf");
			m_extensions.put(FileFormat.FI_PNG, "png");
			m_extensions.put(FileFormat.FI_BMP, "bmp");
			m_extensions.put(FileFormat.FI_GIF, "gif");
			m_extensions.put(FileFormat.FI_TIFF, "tiff");
			m_extensions.put(FileFormat.FI_HTML, "html");
			m_extensions.put(FileFormat.FI_JPEGFIF, "jpg");

			m_extensions.put(FileFormat.FI_XML, "xml");

			m_extensions.put(FileFormat.FI_HTMLWCA, "html");
			m_extensions.put(FileFormat.FI_XHTMLB, "html");
			m_extensions.put(FileFormat.FI_HTMLAG, "html");
			m_extensions.put(FileFormat.FI_WIRELESSHTML, "html");

			m_extensions.put(FileFormat.FI_HTML5, "html");
			m_extensions.put(FileFormat.FI_JAVASCRIPT, "js");
			m_extensions.put(FileFormat.FI_HTML_CSS, "css");
			m_extensions.put(FileFormat.FI_TRUETYPEFONT, "ttf");
			
			m_extensions.put(FileFormat.FI_RAWTEXT, "dat");
		    m_extensions.put(FileFormat.FI_JSON, "json");
		    m_extensions.put(FileFormat.FI_WOFFFONT, "woff");
		    m_extensions.put(FileFormat.FI_WOFF2FONT, "woff");
		    m_extensions.put(FileFormat.FI_INTF, "intf");
		}

		public SampleCallback(File parentFile)
		{
			m_ParentFile = parentFile;
		}


		private File getNextImageFile(String extension)
		{
			String parentName = m_ParentFile.getName();
			int extensionIndex = parentName.lastIndexOf('.');
			String strFilename = parentName.substring(0,extensionIndex);

			if(null == extension)
			{
				if(parentName.length() > extensionIndex+1)
					extension = parentName.substring(extensionIndex+1);
				else
					extension = "";
			}

			m_imageFileCount++;
			strFilename += String.format("%04x", m_imageFileCount) + "." + extension;
			File sinkFile = new File(m_ParentFile.getParent(), strFilename);
			return sinkFile;
		}

		@Override
		public CreateNewFileResponse createNewFile(
				FileFormat parentOutputId,
				FileFormat outputId,
				AssociationValue association,
				String path) throws IOException
		{
			File sinkFile = getNextImageFile(m_extensions.get(outputId));

			return new CreateNewFileResponse(sinkFile, sinkFile.getName());
		}

		@Override
		public void newFileInfo(FileFormat parentOutputId,
						FileFormat outputId, AssociationValue association,
						String path, String url) throws IOException
		{
			System.out.println("Creating file: " + path);
		}
	}


	private static void put_usage()
	{
		System.out.println("Usage: OITSample <file> <outputfile> <outputtype> [fontpath]");
		System.out.println("  outputtype = [tiff|jpeg|bmp|png|gif|j2k|HTML|SearchML|SearchHTML|HTML5|SearchText|PageML|FlexionDoc|MHTML|PDF|PDFA|PDFA_2|INTF] - case insensitive");
		System.out.println("  fontpath = Location of fonts to use when creating output. Required argument for pdf outputtypes on unix.");
	}


	/**
	 * @param args
	 */
	public static void main(String[] args)
	{
		if (args.length < 3)
		{
		  put_usage();
		  return;
		}

		String strInputFilename = args[0];
		String strOutputFilename = args[1];

		FileFormat outputType = null;
		String strOutputType = args[2].toUpperCase();
		if(strOutputType.equals("HTML"))
			outputType =  FileFormat.FI_HTML;
		else if(strOutputType.equals("SEARCHML"))
			outputType =  FileFormat.FI_SEARCHML_LATEST;
		else if(strOutputType.equals("SEARCHHTML"))
			outputType =  FileFormat.FI_SEARCHHTML;
		else if(strOutputType.equals("HTML5"))
			outputType =  FileFormat.FI_HTML5;
		else if(strOutputType.equals("SEARCHTEXT"))
			outputType =  FileFormat.FI_SEARCHTEXT;
		else if(strOutputType.equals("PAGEML"))
			outputType =  FileFormat.FI_PAGEML;
		else if(strOutputType.equals("FLEXIONDOC"))
			outputType =  FileFormat.FI_XML_FLEXIONDOC_LATEST;
		else if(strOutputType.equals("GIF"))
			outputType =  FileFormat.FI_GIF;
		else if(strOutputType.equals("JPEG"))
			outputType =  FileFormat.FI_JPEGFIF;
		else if(strOutputType.equals("BMP"))
			outputType =  FileFormat.FI_BMP;
		else if(strOutputType.equals("PNG"))
			outputType =  FileFormat.FI_PNG;
		else if(strOutputType.equals("TIFF"))
			outputType =  FileFormat.FI_TIFF;
		else if(strOutputType.equals("J2K"))
			outputType =  FileFormat.FI_JPEG2000;
		else if(strOutputType.equals("MHTML"))
			outputType =  FileFormat.FI_MHTML;
		else if(strOutputType.equals("PDF"))
			outputType =  FileFormat.FI_PDF;
		else if(strOutputType.equals("PDFA"))
			outputType =  FileFormat.FI_PDFA;
		else if(strOutputType.equals("PDFA_2"))
			outputType =  FileFormat.FI_PDFA_2;
		else if(strOutputType.equals("INTF"))
			outputType =  FileFormat.FI_INTF;
		else
		{
			put_usage();
			return;
		}

		/* In Java 7, try-with-resources could replace the finally block
		try(Exporter exporter = OutsideIn.newLocalExporter())
		{ */
		Exporter exporter = null;
		try
		{
			exporter = OutsideIn.newLocalExporter();

			File outputFile = new File(strOutputFilename);
			SampleCallback callback = new SampleCallback(outputFile);

			exporter.setSourceFile(new File(strInputFilename))
//					.setSourceFormat(SccFi)
					.setDestinationFile(outputFile)
					.setDestinationFormat(outputType)
					.setCallbackHandler(callback);

			/* Outside In requires the fontpath to be explicitly specified
			 * on Unix for pdf export and web view export, otherwise it's
			 * optional */
			if(args.length > 3)
			{
				LinkedList<File> fontdir = new LinkedList<File>();
				fontdir.add(new File(args[3]));
				exporter.setFontDirectories(fontdir);
			}
			exporter.setTimeZoneText(m_TimeZoneText);
            exporter.setHiddenTextFlag(true);
			
			FileFormat normalFormat = exporter.getFileId(Exporter.FileIdInfoFlagValue.NORMAL);
			System.out.println("File Identifier : " + normalFormat.getDescription() + "(" + normalFormat.getId() + ")");

			FileFormat rawFormat = exporter.getFileId(Exporter.FileIdInfoFlagValue.RAWFI);
			System.out.println("File Identifier (Raw): " + rawFormat.getDescription() + "(" + rawFormat.getId() + ")");

			exporter.export();
			System.out.println("Export Successful");
		}
		catch(OutsideInException ex)
		{
			System.out.println(ex.getMessage());
		}
		/* If using try-with-resources in Java 7 we can get rid of the finally block */
		finally
		{
			if(null != exporter)
			{
				try
				{
					exporter.close();
				}
				catch(IOException ex)
				{
					System.out.println(ex.getMessage());
				}
			}
		}
	}

}
