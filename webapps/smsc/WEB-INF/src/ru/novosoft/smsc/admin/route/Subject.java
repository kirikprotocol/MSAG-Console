/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:03:51
 */
package ru.novosoft.smsc.admin.route;


public class Subject extends Mask
{
  private String name = null;
  private SME defaultSme = null;

  public Subject(String name, String mask, SME defaultSME)
  {
    super(mask);
    if (name == null)
      throw new NullPointerException("Name is null");
    if (defaultSME == null)
      throw new NullPointerException("DefaultSME is null");

    this.name = name;
    this.defaultSme = defaultSME;
  }

  public String getName()
  {
    return name;
  }

  public SME getDefaultSme()
  {
    return defaultSme;
  }
}
