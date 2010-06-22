package com.eyeline.smsc.profsync;

import org.apache.log4j.Category;

import java.io.*;
import java.net.Socket;
import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: Dec 6, 2006
 * Time: 11:59:19 AM
 */
public class ConsoleInteraction extends Thread {
  static Category logger = Category.getInstance(ConsoleInteraction.class);

  String host;
  int port;
  String user;
  String password;
  int keepAliveInterval;
  Socket sock;
  PrintWriter pw;
  BufferedReader rd;
  CommandsFile cmdFile;

  public ConsoleInteraction(Properties config, CommandsFile cmdFile) throws IOException {
    this.cmdFile = cmdFile;
    host = config.getProperty("console.host");
    if (host == null || host.length() == 0) throw new IOException("console.host not specified");
    try {
      port = Integer.valueOf(config.getProperty("console.port")).intValue();
    } catch (NumberFormatException e) {
      throw new IOException("console.port invalid or not specified");
    }
    user = config.getProperty("console.user");
    if (user == null || user.length() == 0) throw new IOException("console.user not specified");
    password = config.getProperty("console.password");
    if (password == null || password.length() == 0) throw new IOException("console.password not specified");
    try {
      keepAliveInterval = Integer.valueOf(config.getProperty("console.keepalive")).intValue();
    } catch (NumberFormatException e) {
      keepAliveInterval = 5000;
      logger.warn("console.keepalive invalid or not specified, using default 5000");
    }
    start();
  }

  public synchronized boolean sendCommand(String command) throws IOException {
    return sendCommandInt(command);
  }

  public boolean sendCommandInt(String command) throws IOException {
    if (sock == null) {
      connect();
    }
    try {
      logger.debug("send command: " + command);
      pw.println(command);
      pw.flush();
      String line = rd.readLine();
      logger.debug("got resp: " + line);
      if (isOk(line)) return true;
    } catch (IOException e) {
      logger.warn("Could not send command, disconnecting...", e);
      disconnect();
    }
    return false;
  }

  protected void connect() throws IOException {
    try {
      sock = new Socket(host, port);
    } catch (IOException e) {
      disconnect();
      throw e;
    }
    try {
      rd = new BufferedReader(new InputStreamReader(sock.getInputStream()));
    } catch (IOException e) {
      logger.error("Could not get input stream");
      disconnect();
      throw e;
    }
    try {
      pw = new PrintWriter(new OutputStreamWriter(sock.getOutputStream()));
    } catch (IOException e) {
      logger.error("Could not get output stream");
      disconnect();
      throw e;
    }
    logger.info("Connected to webconsole");
    String line = rd.readLine();
    logger.debug("got resp: " + line);
    if (isOk(line)) {
      logger.debug("loging in as " + user);
      pw.println(user);
      pw.flush();
    } else {
      disconnect();
      throw new IOException("login failed: " + line);
    }
    line = rd.readLine();
    logger.debug("got resp: " + line);
    if (isOk(line)) {
      logger.debug("Sending password");
      pw.println(password);
      pw.flush();
    } else {
      disconnect();
      throw new IOException("login failed: " + line);
    }
    line = rd.readLine();
    logger.debug("got resp: " + line);
    if (!isOk(line)) {
      disconnect();
      throw new IOException("login failed: " + line);
    }
  }

  protected void disconnect() {
    if (pw != null) {
      pw.close();
      pw = null;
    }
    if (rd != null) {
      try {
        rd.close();
      } catch (IOException e1) {
      }
      rd = null;
    }
    if (sock != null) {
      try {
        sock.close();
      } catch (IOException e1) {
      }
      sock = null;
    }
  }

  protected boolean isOk(String line) {
    if (line.length() > 0 && line.charAt(0) == '+') return true;
    return false;
  }

  public void run() {
    while (!isInterrupted()) {
      synchronized (this) {
        try {
          if (cmdFile.isHaveRecords()) {
            logger.info("Sending commands");
            resendCommands();
          } else {
            if (sock != null) {
              logger.debug("Ping");
              pw.println("ping");
              pw.flush();
              String line = rd.readLine();
            }
          }
        } catch (IOException e) {
          logger.error("Connection to console lost", e);
          disconnect();
        } catch (Throwable ee) {
          logger.error("Unexpected exception in runner", ee);
        }
        try {
          wait(keepAliveInterval);
        } catch (InterruptedException e) {
          logger.info("Console interaction shutdown");
          disconnect();
          break;
        }
      }
    }
    synchronized (this) {
      notifyAll();
    }
  }

  public void shutdown() {
    synchronized (this) {
      interrupt();
      try {
        wait();
      } catch (InterruptedException e) {
      }
    }
  }

  void resendCommands() {
    cmdFile.roll();
    String command = null;
    while ((command = cmdFile.getCommand()) != null) {
      try {
        if (!sendCommandInt(command)) {
          try {
            cmdFile.addCommand(command);
          } catch (IOException e1) {
            logger.error("Failed to add command to file: " + command, e1);
          }
        }
      } catch (IOException e) {
        logger.error("Could not send command storing it...", e);
        try {
          cmdFile.addCommand(command);
        } catch (IOException e1) {
          logger.error("Failed to add command to file: " + command, e1);
        }
      }
    }
  }

}
