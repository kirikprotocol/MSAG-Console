package ru.sibinco.scag.backend.rules;

import ru.sibinco.scag.beans.rules.RuleState;

import java.net.Socket;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

import org.apache.log4j.Logger;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 27.09.2006
 * Time: 17:59:31
 * To change this template use File | Settings | File Templates.
 */
public class Pinger {
  private final static Logger logger = Logger.getLogger(Pinger.class);
  private static final int pingValue = 57;

  public static void checkConnect(RuleState ruleState) {
    if (!ruleState.isPingable()) return;
    //check connect to the applet
    boolean appletRunning = false;
    Socket appletSocket = null;
    OutputStream os = null;
    InputStream is = null;
    String host = ruleState.getPingHostName();
    int port = ruleState.getPingPort();
    logger.debug("Checking connection to jEdit host - " +  host + "; ping port = " + port);
    try {
      appletSocket = new Socket(host, port);
      os = appletSocket.getOutputStream();
      //write dummy byte to socket
      os.write(pingValue);
      is = appletSocket.getInputStream();
      if (is.read() == pingValue) appletRunning = true;
    } catch (java.net.ConnectException e) {
      logger.error("Couldn't connect to "+host+" on port "+ port);
    } catch (IOException e) {
      // couldn't connect to socket and check applet, so unlock rule
      logger.error(e);
    } finally {
      if (os!=null) {
        try {
          os.close();
        } catch (IOException e) {}
      }
      if (is!=null) {
        try {
          is.close();
        } catch(IOException io) {}
      }
      if (appletSocket!=null) {
        try {
          appletSocket.close();
        } catch (IOException io) {}
      }
    }
    if (!appletRunning) {
      ruleState.setLocked(false);
      ruleState.setPingable(false);
    }
  }

}
