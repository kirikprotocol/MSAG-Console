package ru.sibinco.smppgw.backend.protocol.tables.impl.alias;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.03.2005
 * Time: 19:29:04
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.smppgw.backend.protocol.alias.Alias;
import ru.sibinco.smppgw.backend.protocol.tables.AbstractDataItem;


public class AliasDataItem extends AbstractDataItem
{
  protected AliasDataItem(Alias alias)
  {
    values.put("Address", alias.getAddress().getMask());
    values.put("Alias", alias.getAlias().getMask());
    values.put("Hide", new Boolean(alias.isHide()));
  }
}