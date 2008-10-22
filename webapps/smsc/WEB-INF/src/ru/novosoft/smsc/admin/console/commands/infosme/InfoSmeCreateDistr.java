package ru.novosoft.smsc.admin.console.commands.infosme;

import java.util.Date;
import java.util.Calendar;
import java.util.Set;

/**
 * Created by IntelliJ IDEA.
 * User: alkhal
 * Date: 22.10.2008
 * Time: 18:00:37
 * To change this template use File | Settings | File Templates.
 */
public interface InfoSmeCreateDistr {

  public boolean createDistribution(String fileName, Date dateBegin, Date dateEnd, Calendar timeBegin, Calendar timeEnd, Set days, String txmode);

}
