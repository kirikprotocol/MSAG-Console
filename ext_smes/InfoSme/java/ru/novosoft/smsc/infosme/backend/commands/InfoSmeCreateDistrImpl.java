package ru.novosoft.smsc.infosme.backend.commands;

import ru.novosoft.smsc.admin.console.commands.infosme.InfoSmeCreateDistr;

import java.util.Date;
import java.util.Calendar;
import java.util.Set;

/**
 * Created by IntelliJ IDEA.
 * User: alkhal
 * Date: 22.10.2008
 * Time: 17:01:51
 * To change this template use File | Settings | File Templates.
 */
public class InfoSmeCreateDistrImpl implements InfoSmeCreateDistr {

  public boolean createDistribution(String fileName, Date dateBegin, Date dateEnd, Calendar timeBegin, Calendar timeEnd, Set days, String txmode) {
    return true;  //To change body of implemented methods use File | Settings | File Templates.
  }
}
