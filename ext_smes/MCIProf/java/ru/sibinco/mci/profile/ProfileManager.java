package ru.sibinco.mci.profile;

import java.util.Collection;
import java.util.Vector;

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

  private static Vector informFormats = new Vector();
  private static Vector notifyFormats = new Vector();
  static {
    informFormats.add(new FormatType(-1, "DEFAULT"));
    informFormats.add(new FormatType( 0, "EASY"));
    informFormats.add(new FormatType( 1, "COMPLEX"));
    notifyFormats.add(new FormatType(-1, "DEFAULT"));
    notifyFormats.add(new FormatType( 0, "EASY"));
    notifyFormats.add(new FormatType( 1, "FULL"));
  }

  protected ProfileManager()
  {
    // TODO: init connection settings & estabilish connection to MCISme DB
  }

  public ProfileInfo getProfileInfo(String abonent)
  {
    // TODO: implement query to MCISme DB
    return new ProfileInfo(new FormatType( 0, "EASY"), new FormatType(-1, "DEFAULT"), true, false);
  }
  public void setProfileInfo(String abonent, ProfileInfo info)
  {
    // TODO: implement profile (update in/insert to) MCISme DB
  }
  public Collection getFormatAlts(boolean inform)
  {
    // TODO: implement inform or notify message formats
    return (inform) ? informFormats : notifyFormats;
  }
}
