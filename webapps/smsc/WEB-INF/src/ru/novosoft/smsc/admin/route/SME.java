/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:50:54
 */
package ru.novosoft.smsc.admin.route;

import javax.servlet.http.HttpServletRequest;


public class SME
{
  private String id = null;

  public SME(String systemId)
  {
    if (systemId == null)
      throw new NullPointerException("SME System ID is null");

    id = systemId.trim();
  }

  public String getId()
  {
    return id;
  }

  public boolean equals(Object obj)
  {
    if (obj instanceof SME)
      return ((SME) obj).id.equals(id);
    else
      return super.equals(obj);
  }
}
