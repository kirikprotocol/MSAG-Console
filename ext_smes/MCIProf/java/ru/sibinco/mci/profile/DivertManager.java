package ru.sibinco.mci.profile;

/**
 * Created by: Serge Lugovoy
 * Date: 06.09.2004
 * Time: 14:34:36
 */
public class DivertManager
{
  private static DivertManager instance = null;
  private static Object syncObj = new Object();

  public static DivertManager getInstance()
  {
    synchronized(syncObj) {
      return (instance == null) ? (instance = new DivertManager()):instance;
    }
  }

  protected DivertManager() {
    // TODO: init connection settings & estabilish connection to MSC
  }
  public DivertInfo getDivertInfo(String abonent)
  {
    // TODO: implement query to MSC
    return new DivertInfo(DivertScenarioConstants.BUSY, DivertScenarioConstants.ABSENT,
                          DivertScenarioConstants.NOTAVAIL, DivertScenarioConstants.UNCOND);
  }
}
