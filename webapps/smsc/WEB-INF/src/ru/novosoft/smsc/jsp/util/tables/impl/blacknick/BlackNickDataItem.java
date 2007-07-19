package ru.novosoft.smsc.jsp.util.tables.impl.blacknick;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;
import ru.novosoft.smsc.admin.profiler.BlackNick;

/**
 * User: artem
 * Date: 18.07.2007
 */

public class BlackNickDataItem extends AbstractDataItem {
  public BlackNickDataItem(BlackNick nick) {
    values.put("nick", nick.getNick());
  }

  public BlackNickDataItem(String nick) {
    values.put("nick", nick);
  }
}
