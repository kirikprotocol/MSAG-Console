package ru.sibinco.mci.profile;

/**
 * Created by: Serge Lugovoy
 * Date: 06.09.2004
 * Time: 14:34:36
 */
public class DivertManager {
  static DivertManager instance = null;
  static Object syncObj = new Object();
  public static DivertManager getInstance() {
    synchronized(syncObj) {
      if( instance == null ) {
        instance = new DivertManager();
      }
      return instance;
    }
  }
}
