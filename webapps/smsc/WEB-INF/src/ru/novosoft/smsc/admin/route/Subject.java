/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:03:51
 */
package ru.novosoft.smsc.admin.route;


public class Subject
{
  private String name = null;
  private SME defaultSme = null;
  private MaskList masks = null;

  public Subject(String name, String masksString, SME defaultSME)
  {
    if (name == null)
      throw new NullPointerException("Name is null");
    if (masksString == null)
      throw new NullPointerException("Masks is null");
    if (defaultSME == null)
      throw new NullPointerException("DefaultSME is null");

    this.name = name;
    masks = new MaskList(masksString);
    if (masks.size() == 0)
      throw new NullPointerException("Masks is empty");
    this.defaultSme = defaultSME;
  }

  public String getName()
  {
    return name;
  }

  public MaskList getMasks()
  {
    return masks;
  }

  public SME getDefaultSme()
  {
    return defaultSme;
  }
}
