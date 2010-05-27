package mobi.eyeline.mcaia;

import mobi.eyeline.mcaia.protocol.BusyRequest;
import mobi.eyeline.mcaia.protocol.Status;

/**
 * Created by Serge Lugovoy
 * Date: May 26, 2010
 * Time: 3:21:12 PM
 */
public interface StatusHandler {
  public void handleStatus( BusyRequest req, Status status);
}
