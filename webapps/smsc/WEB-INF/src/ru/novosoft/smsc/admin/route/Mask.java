/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:37:28
 */
package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;


public class Mask
{
	private final static String full_pattern = "^\\.(0|1|2|3|4|5|6)\\.(0|1|3|4|6|8|9|10|14|18)\\.(\\d{1,21})(\\?*|\\**)$";
	private final static String plus_pattern = "^\\+(\\d{1,21})(\\?*)$";
	private final static String short_pattern = "^(\\d{1,21})(\\?*)$";

	private String mask = null;

	public Mask(String mask)
			  throws AdminException
	{
		if (mask == null)
			throw new NullPointerException("Mask string is null");
		this.mask = mask.trim();
		if (!isMaskValid(this.mask))
			throw new AdminException("Mask \"" + this.mask + "\" is not valid");
	}

	public String getMask()
	{
		return mask;
	}

	public boolean equals(Object obj)
	{
		if (obj instanceof Mask)
			return mask.equals(((Mask) obj).mask);
		else
			return super.equals(obj);
	}

	public PrintWriter store(PrintWriter out)
	{
		out.println("    <mask value=\"" + StringEncoderDecoder.encode(mask) + "\"/>");
		return out;
	}

	public static boolean isMaskValid(String maskStr)
	{
		// valid masks:
		// 	.[0..6].[0|1|3|4|6|8|9|10|14|18].[x]
		// 	+[x]	==> .1.1.[x]
		// 	[x]	==> .0.1.[x]
		// где x - numeric string длиной от 1 до 21

		return maskStr.matches(full_pattern) || maskStr.matches(short_pattern) || maskStr.matches(plus_pattern);
	}
}
