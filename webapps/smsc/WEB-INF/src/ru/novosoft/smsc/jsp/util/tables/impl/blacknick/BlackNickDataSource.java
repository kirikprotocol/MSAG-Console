package ru.novosoft.smsc.jsp.util.tables.impl.blacknick;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.admin.profiler.BlackNick;

/**
 * User: artem
 * Date: 19.07.2007
 */

public class BlackNickDataSource extends AbstractDataSourceImpl {

  public BlackNickDataSource() {
    super(new String[] {"nick"});
  }

  public void add(BlackNick nick) {
    super.add(new BlackNickDataItem(nick));
  }

  public void add(String nick) {
    super.add(new BlackNickDataItem(nick));
  }

  public void remove(BlackNick nick) {
    super.remove(new BlackNickDataItem(nick));
  }

}
