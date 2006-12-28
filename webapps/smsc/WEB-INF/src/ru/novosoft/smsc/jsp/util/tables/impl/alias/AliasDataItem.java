package ru.novosoft.smsc.jsp.util.tables.impl.alias;

/*
 * Author: igork
 * Date: 11.06.2002
 * Time: 17:00:09
 */

import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;


public class AliasDataItem extends AbstractDataItem
{
  public AliasDataItem(Alias alias) {
    values.put(AliasDataSource.ALIAS_FIELD, alias.getAlias().getMask());
    values.put(AliasDataSource.ADDRESS_FIELD, alias.getAddress().getMask());
    values.put(AliasDataSource.HIDE_FIELD, new Boolean(alias.isHide()));
  }

  public AliasDataItem(String alias, String address, boolean hide) {
    values.put(AliasDataSource.ALIAS_FIELD, alias);
    values.put(AliasDataSource.ADDRESS_FIELD, address);
    values.put(AliasDataSource.HIDE_FIELD, new Boolean(hide));
  }
}
