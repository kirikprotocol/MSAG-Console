/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:45:30
 */
package ru.novosoft.smsc.admin.route;


public class Destination
{
  private Subject subj = null;
  private Mask mask = null;
  private SME sme = null;

  public Destination(Subject subj)
  {
    this(subj, subj.getDefaultSme());
  }

  public Destination(Subject subj, SME sme)
  {
    if (subj == null)
      throw new NullPointerException("Subject is null");
    if (sme == null)
      throw new NullPointerException("SME is null");

    this.subj = subj;
    this.sme = sme;
  }

  public Destination(Mask mask, SME sme)
  {
    if (mask == null)
      throw new NullPointerException("Mask is null");
    if (sme == null)
      throw new NullPointerException("SME is null");

    this.mask = mask;
    this.sme = sme;
  }

  public boolean isSubject()
  {
    return subj != null;
  }

  public String getName()
  {
    if (isSubject())
      return subj.getName();
    else
      return mask.getMask();
  }

  public MaskList getMasks()
  {
    if (isSubject())
      return subj.getMasks();
    else
      return new MaskList(mask);
  }

  public SME getSme()
  {
    return sme;
  }

  public void setSme(SME sme)
  {
    this.sme = sme;
  }
}
