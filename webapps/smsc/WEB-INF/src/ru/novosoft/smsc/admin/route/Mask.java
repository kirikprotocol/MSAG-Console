/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:37:28
 */
package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;


public class Mask
{
  private String mask = null;

  public Mask(String mask)
  {
    if (mask == null)
      throw new NullPointerException("Mask string is null");

    this.mask = mask.trim();
  }

  public String getMask()
  {
    return mask;
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof Mask)
      return mask.equals((Mask) obj);
    else
      return super.equals(obj);
  }

  public PrintWriter store(PrintWriter out)
  {
    out.println("    <mask value=\"" + StringEncoderDecoder.encode(mask) + "\"/>");
    return out;
  }
}
