package com.outsideinsdk;

/**
 * <code>ExportFormat</code> provides all the possible values for the output format of a conversion executed by
 * {@link Export#convert(String, String, String) Export.convert}.
 * 
 * @author John R. Pazdernik
 * @version 1.00
 */
public interface ExportFormat {
	public static final String HTML = "fi_html";
	public static final String WML = "fi_wml";
	public static final String HDML = "fi_hdml";
	public static final String XHTMLBASIC = "fi_xhtmlb";
	public static final String CHTML = "fi_chtml";
	public static final String TEXT = "fi_text";
	public static final String WIRELESSHTML = "fi_wirelesshtml";
	public static final String SEARCHML = "fi_searchml";
	public static final String PAGEML = "fi_pageml";
	public static final String XML_FLEXIONDOC4 = "fi_xml_flexiondoc4";
	public static final String  TIFF = "fi_tiff";
	public static final String  PNG = "fi_png";
	public static final String  BMP = "fi_bmp";
	public static final String  GIF = "fi_gif";
	public static final String  JPEG = "fi_jpeg";
}
