package ru.novosoft.smsc.jsp.util.tables.impl.subject;

/*
 * Author: igork
 * Date: 13.06.2002
 * Time: 15:38:44
 */

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

import java.util.Collections;
import java.util.Iterator;
import java.util.Vector;


public class SubjectDataItem extends AbstractDataItem
{
  protected SubjectDataItem(Subject subj)
  {
    values.put("Name", subj.getName());
    values.put("Default SME", subj.getDefaultSme().getId());
    Vector masks = new Vector();
    for (Iterator i = subj.getMasks().iterator(); i.hasNext();) {
      masks.add(((Mask) i.next()).getMask());
    }
    Collections.sort(masks);
    values.put("Masks", masks);
    values.put("notes", subj.getNotes());
  }
}
