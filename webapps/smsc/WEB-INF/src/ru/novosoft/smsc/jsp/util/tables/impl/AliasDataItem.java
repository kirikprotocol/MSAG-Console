/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 17:00:09
 */
package ru.novosoft.smsc.jsp.util.tables.impl;

import ru.novosoft.smsc.admin.alias.Alias;


public class AliasDataItem extends AbstractDataItem
{
  protected AliasDataItem(Alias alias)
  {
    values.put("Address", alias.getAddress().getMask());
    values.put("Alias", alias.getAlias().getMask());
  }
}
