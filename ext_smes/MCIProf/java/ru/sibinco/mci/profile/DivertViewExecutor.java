package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import ru.aurorisoft.smpp.Message;
import org.apache.log4j.Category;

import java.text.MessageFormat;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.09.2004
 * Time: 16:28:11
 */
public class DivertViewExecutor extends DivertAbstractExecutor
{
  private static Category logger = Category.getInstance(DivertViewExecutor.class);

  private MessageFormat messageFormat = null;

  public void init(Properties properties) throws ScenarioInitializationException
  {
    try {
      super.init(properties);
      messageFormat = new MessageFormat(divertBundle.getString(DivertScenarioConstants.TAG_INFO));
    } catch (Exception e) {
      logger.error("", e);
      throw new ScenarioInitializationException(e.getMessage());
    }
  }

  public ExecutorResponse execute(ScenarioState state) throws ExecutingException
  {
    DivertInfo info = divertManager.getDivertInfo(state.getAbonent());
    Object[] args  = new Object[] {reasonBusyStr, reasonAbsentStr, reasonNotAvailStr, reasonUncondStr,
                                   info.getBusy(), info.getAbsent(), info.getNotavail(), info.getUncond()};
    String msgresp = messageFormat.format(args);
    Message resp = new Message();
    resp.setMessageString(msgresp);
    return new ExecutorResponse(new Message[]{resp}, false);
  }

}
