/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 15:23:10
 */
package ru.novosoft.smsc.jsp.util.tables;

import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.Subject;


public interface Filter
{
  boolean isEmpty();
  boolean isItemAllowed(DataItem item);
}
