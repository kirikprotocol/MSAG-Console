/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 17:59:20
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.AdminException;

public class ProfileDataItem extends AbstractDataItem
{
  protected ProfileDataItem(Profile profile)
      throws AdminException
  {
    /*super();
    System.out.println("profile = " + profile);
    System.out.println("profile.getCodepageString() = " + profile.getCodepageString());
    System.out.println("profile.getReportOptionsString() = " + profile.getReportOptionsString());
    System.out.println("profile.getMask() = " + profile.getMask());
    System.out.println("profile.getMask().getMask() = " + profile.getMask().getMask());*/
    values.put("Mask", profile.getMask().getMask());
    values.put("Codepage", profile.getCodepageString());
    values.put("Report info", profile.getReportOptionsString());
  }
}
