
import com.outsideinsdk.*;
import java.io.*;
import java.security.*;
import java.util.*;

/**
 * The <code>ExportTest</code> class tests the {@link Export Export} technology
 * according to the properties provided in a given configuration file.  The
 * configuration file is assumed to be correctly formatted.
 *
 * @author Kevin Glannon
 * @version 1.00
 * @see Export Export
 */
public class ExportTest
{
	private static final String INPUTPATHKEY = "inputpath";
	private static final String OUTPUTPATHKEY = "outputpath";
	private static final String OUTPUTIDKEY = "outputid";

	Properties configProps = new Properties();

	/**
	 *  Since <code>ExportTest</code> objects are always associated with a
	 *  configuration file, the constructor requires a configuration file path.
	 *
	 *  @param cfp  The configuration file path.
	 */
	public ExportTest(String cfp)
		throws FileNotFoundException, IOException
	{
		parseConfig(cfp);
	}

	/**
	 *  Parse the configuration file specified by the given path.
	 *
	 *  @param cfp  The configuration file path.
	 */
	public void parseConfig(String cfp)
		throws FileNotFoundException, IOException
	{
		//  Assure the configuration file exists and is readable.
		File cff = new File(cfp);

		if (!cff.exists() || !cff.isFile() || !cff.canRead())
		{
			throw(new InvalidParameterException("Configuration file unreadable."));
		}

		BufferedReader cfr = new BufferedReader(new FileReader(cff));

		String line;
		//  Loop over all lines from the file.
		while ((line = cfr.readLine()) != null)
		{
			processLine(line);
		}
	}

	/**
	 *  Support the parsing of the configuration file by processing a given
	 *  line.
	 *
	 *  @param l  A line from a configuration file.
	 */
	private void processLine(String l)
	{
		//  Look for comments.
		int indPound = l.indexOf('#');

		//  Remove comments and whitespace.
		String line = (indPound == -1) ? l.trim() :
																		 l.substring(0, indPound).trim();

		if (line.length() != 0)
		{
			StringTokenizer stl = new StringTokenizer(line);
                        String key = stl.nextToken();
                        String value = stl.nextToken();
                        while(stl.hasMoreTokens())
                        {
                          value +=" " + stl.nextToken();
                        }
                        //  Fill in the appropriate property.
                        configProps.setProperty(key, value);
		}
	}

	/**
	 *  Run the conversion using the given input path, output path.
	 *
	 *  @param ifp     Input path.
	 *  @param ofp     Output path.
	 *  @param timeout Export process timeout in milliseconds.
	 */
	public void convert(String ifp, String ofp, long timeout)
	{
		String oid = configProps.getProperty(OUTPUTIDKEY);

		//  Display the parameters.
		System.out.println("Input Path: "+ifp+" Output Path: "+ofp+
											 " Output ID: "+oid);

		//  Remove extra control properties.
		configProps.remove(INPUTPATHKEY);
		configProps.remove(OUTPUTPATHKEY);

		//  Create list of input files.
		File iff = new File(ifp);
		File [] iffa;
		if (iff.isDirectory())
			iffa = iff.listFiles();
		else
		{
			iffa = new File[1];
			iffa[0] = iff;
		}

		//  Create output directory if needed.  Assuming that if the input path
		//  is a directory, the output path should also be a directory.
		File off = new File(ofp);
		if (iff.isDirectory() && !off.exists()) off.mkdir();

		//  Process the conversion.
		Export e = new Export(configProps);
		if (off.isDirectory())
		{
			//  The outputid is in the form fi_XXX where XXX is a reasonable
			//  extension so we take the extension for the oid.
			//  oid.substring(3) means to get the string following the fi_
			String ext = "." + oid.substring(3);
			for (int i=0; i<iffa.length; i++)
			{
				String ifn = iffa[i].toString();
				String ofn = ofp + File.separator + iffa[i].getName() + ext;
				System.out.println("Converting "+ifn+" to "+ofn);
				ExportStatusCode result = e.convert(ifn, ofn, oid, timeout);
            if (result.getCode() == ExportStatusCode.SCCERR_OK.getCode())
            {
				   System.out.println("Conversion Successful!");
            }
            else {
               System.out.println("Conversion Error: " + result);
            }
			}
		}
		else
		{
			for (int i=0; i<iffa.length; i++)
			{
				ExportStatusCode result = e.convert(iffa[i].toString(), ofp, oid, timeout);
            if (result.getCode() == ExportStatusCode.SCCERR_OK.getCode())
            {
				   System.out.println("Conversion Successful!");
            }
            else {
               System.out.println("Conversion Error: " + result);
            }
			}
		}
	}

	/**
	 * Run the test according to the given arguments.  These arguments must adhere to the following usage.<br><br>
	 * Usage:<br>
	 * ExportTest InputPath OutputPath ConfigurationFile [Timeout]<br><br>
	 *
	 * InputPath and OutputPath may be single files or directories.  If InputPath is a directory, then all files in
	 * that directory will be converted, but without recursion.  If OutputPath is a directory, then all converted
	 * files from InputPath are placed in the OutputPath directory by appending an extension which represents the
	 * output file type. Timeout is in milliseconds.
	 *
	 *  @param args  Command line arguments.
	 */
	public static void main(String[] args)
	{
		int count = args.length;

		//  Check for specification of configuration file.
		if (count != 3 && count != 4)
		{
			System.out.println("Input path, output path and configuration file are required.");
			System.out.println("Usage: ExportTest InputPath OutputPath "+
												 "ConfigurationFile [Timeout(in milliseconds)]");
			System.out.println();
		}
		else
		{
			ExportTest ct = null;

			try
			{
				ct = new ExportTest(args[2]);
			}
			catch (Exception ex)
			{
				ex.printStackTrace();
				return;
			}

            long   timeout = 0;
            if( count == 4 )
            {
                timeout = Integer.decode( args[3] ).longValue();
            }

			ct.convert(args[0], args[1], timeout);
		}
	}
}
