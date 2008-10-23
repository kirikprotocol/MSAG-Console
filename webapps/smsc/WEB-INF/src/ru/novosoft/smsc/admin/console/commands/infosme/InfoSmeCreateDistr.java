package ru.novosoft.smsc.admin.console.commands.infosme;

import ru.novosoft.smsc.admin.console.CommandContext;

import java.util.Date;
import java.util.Calendar;
import java.util.Set;
import java.util.Map;

/**
 *
 * User: alkhal
 * Date: 22.10.2008
 *
 */
public interface InfoSmeCreateDistr {

  public void createDistribution(CommandContext ctx, String fileName, String dateBegin, String dateEnd,
                                 String timeBegin, String timeEnd, String days, Boolean txmode, String address);

}
