package ru.sibinco.mci.profile;

import ru.sibinco.smpp.appgw.scenario.*;
import org.apache.log4j.Category;

import java.util.Properties;
import java.util.HashMap;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 13.09.2004
 * Time: 14:02:29
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManagerPreprocessor extends ProfileManagerState implements ScenarioStateProcessor
{
  private static Category logger = Category.getInstance(ProfileManagerPreprocessor.class);

  public void init(Properties properties) throws ScenarioInitializationException
  {
    super.init(properties);
  }

  public void process(ScenarioState state) throws ProcessingException
  {
    if (state.getAttribute(Constants.ATTR_MAIN) == null)
    {
      String msg = state.getMessageString();
      HashMap formats = (HashMap)state.getAttribute(Constants.ATTR_FORMATS);

      if (msg == null)
        throw new ProcessingException("Profile option is undefined", ErrorCode.PAGE_EXECUTOR_EXCEPTION);

      msg = msg.trim();
      if (!msg.equals(Constants.OPTION_EXIT))
      {
        try
        {
          ProfileInfo info = getProfileInfo(state);
          if (formats == null) // set inform or notify flag
          {
            if      (msg.equals("1")) info.inform = !info.inform;
            else if (msg.equals("3")) info.notify = !info.notify;
            else new ProcessingException("Profile option '"+msg+"' in unknown", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
          }
          else // set inform or notify message format type
          {
            String reason = (String)state.getAttribute(Constants.ATTR_REASON);
            if (reason == null)
              throw new ProcessingException("Profile option is undefined", ErrorCode.PAGE_EXECUTOR_EXCEPTION);

            boolean inform = true;
            if      (reason.equals(Constants.INFORM)) inform = true;
            else if (reason.equals(Constants.NOTIFY)) inform = false;
            else throw new ProcessingException("Profile option '"+reason+"' in unknown", ErrorCode.PAGE_EXECUTOR_EXCEPTION);
            try {
              FormatType format = (FormatType)formats.get(new Integer(msg));
              if (format == null) throw new Exception("Profile message format is undefined");
              if (inform) info.informFormat = format;
              else info.notifyFormat = format;
            } catch (Exception exc) {
              String err = "Profile option is invalid '"+msg+"'";
              logger.error(err, exc);
              throw new ProcessingException(err, exc, ErrorCode.PAGE_EXECUTOR_EXCEPTION);
            }
          }
          setProfileInfo(state, info);
          state.removeAttribute(Constants.ATTR_ERROR);
        }
        catch (ProfileManagerException exc)
        {
          state.setAttribute(Constants.ATTR_ERROR, exc);
          logger.warn("Profile exception was stored in session");
        }
      }
    }
    state.removeAttribute(Constants.ATTR_FORMATS);
    state.removeAttribute(Constants.ATTR_MAIN);
  }

}
