/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:37:28
 */
package ru.novosoft.smsc.admin.route;


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
}
