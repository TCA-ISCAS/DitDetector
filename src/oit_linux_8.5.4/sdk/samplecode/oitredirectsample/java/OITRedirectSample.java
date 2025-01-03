/*
|   Outside In Export Technologies
|   Redirected IO Sample
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
|   can use redirected I/O in the Outside In Export API to export documents.
|
*/


import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

import java.util.HashMap;
import java.util.LinkedList;

import com.oracle.outsidein.Callback;
import com.oracle.outsidein.OutsideIn;
import com.oracle.outsidein.FileFormat;
import com.oracle.outsidein.Exporter;
import com.oracle.outsidein.SeekableByteChannel6;

public class OITRedirectSample
{

	private static void put_usage()
	{
		System.out.println("Usage: OITRedirectedSample <file> <outputfile> <outputtype> [fontpath]");
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

		MyCallbackHandler callback = null;
		MyRedirectedIO input = null;
		MyRedirectedIO output = null;

		/* In Java 7, try-with-resources could replace the finally block
		try(Exporter exporter = OutsideIn.newLocalExporter())
		{ */
		Exporter exporter = null;
		try
		{
			exporter = OutsideIn.newLocalExporter();
			callback = new MyCallbackHandler(new File(strInputFilename), new File(strOutputFilename));

			input = new MyRedirectedIO(new File(strInputFilename), "r");
			output = new MyRedirectedIO(new File(strOutputFilename), "rw");

			exporter.setSourceFile(input)
					.setDestinationFile(output)
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

			FileFormat normalFormat = exporter.getFileId(Exporter.FileIdInfoFlagValue.NORMAL);
			System.out.println("File Identifier : " + normalFormat.getDescription() + "(" + normalFormat.getId() + ")");

			FileFormat rawFormat = exporter.getFileId(Exporter.FileIdInfoFlagValue.RAWFI);
			System.out.println("File Identifier (Raw): " + rawFormat.getDescription() + "(" + rawFormat.getId() + ")");

			exporter.export();
			System.out.println("Export Successful");
			
			int fileCount = callback.getFilesCreatedCount();
			if (fileCount > 0)
			  System.out.println("Number of files created = " + fileCount);
		}
		catch(Exception ex)
		{
			System.out.println(ex.getMessage());
		}
		finally
		{
			if(exporter != null)
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





	/* Implement SeekableByteChannel6 to utilize redirected I/O */
	public static class MyRedirectedIO implements SeekableByteChannel6
	{
		RandomAccessFile raf;
		FileChannel fc;

		public MyRedirectedIO(File file, String permissions) throws FileNotFoundException
		{
			this.raf = new RandomAccessFile(file, permissions);
			this.fc = raf.getChannel();
		}

		@Override
		public void close()
		{
			try {
				if (fc != null) {fc.close(); fc = null;}
				if (raf != null) {raf.close(); raf = null;}
			} catch (IOException ex) {
				// LOG
			} 
		}

		@Override
		public long position() throws IOException
		{
			return fc.position();
		}
		@Override
		public SeekableByteChannel6 position(long newPosition) throws IOException
		{
			fc.position(newPosition);
			return this;
		}
		@Override
		public int read(ByteBuffer dst) throws IOException
		{
			return fc.read(dst);
		}
		@Override
		public long size() throws IOException
		{
			return fc.size();
		}
		@Override
		public int write(ByteBuffer src) throws IOException
		{
			return fc.write(src);
		}
		@Override
		public SeekableByteChannel6 truncate(long size) throws IOException
		{
			fc.truncate(size);
			return this;
		}
		@Override
		public boolean isOpen()
		{
			return fc.isOpen();
		}
	}






	/* this callback could be updated to use the MyRedirectedIO class */
	public static class MyCallbackHandler extends Callback
	{
		// Number of external files created.
		private int m_filesCreatedCount = 0;
		// Number to use as postfix for image filenames
		private int m_imageFileCount = 0;
		// Filename of the root destination document - used in this example to name external files.
		private File m_parentFile;
		// Filename of the source document - used to open secondary files where relative path is provided.
		private File m_sourceFile;
		// List of file formats expected for external files and their extensions.
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

		}

		public MyCallbackHandler(File sourceFile, File parentFile)
		{
			m_parentFile = parentFile;
			m_sourceFile = sourceFile;
		}


		public int getFilesCreatedCount() { return m_filesCreatedCount; }

		private File getNextImageFile(String extension)
		{
			String parentName = m_parentFile.getName();
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
			File sinkFile = new File(m_parentFile.getParent(), strFilename);
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
			MyRedirectedIO rio = new MyRedirectedIO(sinkFile, "rw");

			return new CreateNewFileResponse(rio, sinkFile.getName());
		}

		@Override
		public void newFileInfo(FileFormat parentOutputId,
						FileFormat outputId, AssociationValue association,
						String path, String url) throws IOException
		{
			m_filesCreatedCount++;
		}

		@Override
		public OpenFileResponse openFile(Callback.FileTypeValue fileType, String fileName) throws IOException
		{
			OpenFileResponse resp = null;
			MyRedirectedIO redirect = null;

			try 
			{
				File file = new File(fileName);
				if (!file.isAbsolute())
					file = new File(m_sourceFile.getCanonicalFile().getParent(), fileName);
				redirect = new MyRedirectedIO(file, "r");
				resp = new OpenFileResponse(redirect);
			}
			catch (Exception ex)
			{
				//possibly file not found - return null;
			}

			return resp;
		}
	}


}
