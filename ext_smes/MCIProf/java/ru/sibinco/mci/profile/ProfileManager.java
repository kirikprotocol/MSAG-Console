package ru.sibinco.mci.profile;

import java.util.Collection;
import java.util.ArrayList;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 15:07:55
 * To change this template use File | Settings | File Templates.
 */
public class ProfileManager
{
  private static ProfileManager instance = null;
  private static Object syncObj = new Object();

  public static ProfileManager getInstance()
  {
    synchronized(syncObj) {
      return (instance == null) ? (instance = new ProfileManager()):instance;
    }
  }

  protected ProfileManager()
  {
    // TODO: init connection settings & estabilish connection to MCISme DB
  }

  public ProfileInfo getProfileInfo(String abonent)
  {
    // TODO: implement query to MCISme DB
    return new ProfileInfo("EASY", "DEFAULT", true, false);
  }
  public void setProfileInfo(String abonent, ProfileInfo info)
  {
    // TODO: implement profile (update in/insert to) MCISme DB
  }
  public Collection getFormatAlts(boolean inform)
  {
    // TODO: implement inform or notify message formats
    ArrayList list = new ArrayList();
    if (inform) {
      list.add(new ServiceMessageType(0, "EASY"));
      list.add(new ServiceMessageType(1, "COMPLEX"));
      list.add(new ServiceMessageType(2, "OWN"));
    } else {
      list.add(new ServiceMessageType(0, "FULL"));
      list.add(new ServiceMessageType(1, "OWN"));
    }
    return list;
  }
}
