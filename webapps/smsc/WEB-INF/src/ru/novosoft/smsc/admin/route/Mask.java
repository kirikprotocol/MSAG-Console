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
	private static final String pattern_header = "^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?";
	private static final String pattern1 = pattern_header + "\\d{1,20}\\?{0,19}$";
	private static final String pattern2 = pattern_header + "(\\d|\\?){1,20}$";

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

		return maskStr != null && maskStr.trim().length() > 0
				&& maskStr.matches(pattern1) && maskStr.matches(pattern2);
	}
}
