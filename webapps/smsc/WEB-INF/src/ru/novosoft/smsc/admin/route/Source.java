/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 14:03:03
 */
package ru.novosoft.smsc.admin.route;


public class Source
{
  Subject subj = null;
  Mask mask = null;

  public Source(Subject s)
  {
    if (s == null)
      throw new NullPointerException("Subject is null");

    subj = s;
  }

  public Source(Mask m)
  {
    if (m == null)
      throw new NullPointerException("Mask is null");

    mask = m;
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

  public String getMask()
  {
    if (isSubject())
      return subj.getMask();
    else
      return mask.getMask();
  }
}
