package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import org.apache.log4j.Category;

import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 15.12.2004
 * Time: 15:29:25
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManagerAltPreprocessor extends ProfileManagerState implements ScenarioStateProcessor
{
  private static Category logger = Category.getInstance(ProfileManagerPreprocessor.class);

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
  }

  public void process(ScenarioState state) throws ProcessingException
  {
    if (state.getAttribute(Constants.ATTR_MAIN) != null) return;

    String msg = state.getMessageString();
    if (msg == null)
      throw new ProcessingException("Profile option is undefined", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
    msg = msg.trim();

    if (!msg.equals(Constants.OPTION_EXIT))
    {
      int switchReason = 0;
      try { switchReason = Integer.parseInt(msg); } catch(NumberFormatException e) {
        throw new ProcessingException("Profile option "+msg+" is invalid", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
      }
      String activeReasons = (String)state.getAttribute(Constants.ATTR_REASONS);
      if (activeReasons == null)
        throw new ProcessingException("Failed to locate '"+Constants.ATTR_REASONS+"' attribute",
                                      ErrorCode.PAGE_EXECUTOR_EXCEPTION);
      activeReasons = activeReasons.toUpperCase();
      try
      {
        ProfileInfo info = getProfileInfo(state);
        int counter = 0; int oldEventMask = info.getEventMask();
        if      ((activeReasons.indexOf('B') != -1) && switchReason == ++counter)
          info.eventMask = switchEventMask(oldEventMask, ProfileInfo.MASK_BUSY);
        else if ((activeReasons.indexOf('N') != -1) && switchReason == ++counter)
          info.eventMask = switchEventMask(oldEventMask, ProfileInfo.MASK_NOREPLY);
        else if ((activeReasons.indexOf('A') != -1) && switchReason == ++counter)
          info.eventMask = switchEventMask(oldEventMask, ProfileInfo.MASK_ABSENT);
        else if ((activeReasons.indexOf('U') != -1) && switchReason == ++counter)
          info.eventMask = switchEventMask(oldEventMask, ProfileInfo.MASK_UNCOND);

        if (info.eventMask != oldEventMask) setProfileInfo(state, info);
        else logger.warn("Event mask is kept unchanged (mask="+oldEventMask+", msg='"+msg+"')");
        state.removeAttribute(Constants.ATTR_ERROR);
      }
      catch (ProfileManagerException exc) {
        state.setAttribute(Constants.ATTR_ERROR, exc);
        logger.warn("Profile exception was stored in session");
      }
    }
    state.removeAttribute(Constants.ATTR_MAIN);
  }

}
