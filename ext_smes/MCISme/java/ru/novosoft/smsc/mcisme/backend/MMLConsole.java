package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.jsp.AutostartService;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.admin.AdminException;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Properties;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * Created by Serge Lugovoy
 * Date: May 4, 2008
 * Time: 3:54:48 PM
 */
public class MMLConsole extends Thread implements AutostartService, SMEAppContext {
  static Logger logger = Logger.getLogger(MMLConsole.class);
  int port = 0;
  String  user;
  String  pass;
  ServerSocket ssock;

  public void start(SMSCAppContext ctx) {
    try {
      File folder = ctx.getHostsManager().getServiceInfo("MCISme").getServiceFolder();
      Properties config = new Properties();
      config.load(new FileInputStream(new File(new File(folder, "conf"), "mmlconsole.properties")));
      port = Integer.valueOf(config.getProperty("port")).intValue();
      user = config.getProperty("user");
      pass = config.getProperty("pass");
    } catch (AdminException e) {
      logger.error("Could not initialize MCISme MML console", e);
    } catch (IOException e) {
      logger.error("Could not load config file", e);
    } catch (NumberFormatException e) {
      logger.error("Listen port not specified");
    }
    if( port == 0 ) return;
    try {
      ssock = new ServerSocket(port);
    } catch (IOException e) {
      logger.error("Could not create server socket", e);
      return;
    }
    ctx.registerSMEContext(this);
    start();
  }

  public void run() {
    while( ssock != null ) {
      Socket sock = null;
      try {
        sock = ssock.accept();
      } catch (IOException e) {
        if( ssock != null ) logger.error("Socket accept failed", e);
        break;
      }

      try {
        MMLProcessor proc = new MMLProcessor(sock);
        proc.start();
      } catch (IOException e) {
        logger.error("", e);
      }
    }
    close();
    synchronized (this ) {
      notifyAll();
    }
  }

  protected void close() {
    if( ssock != null ) {
      try {
        ssock.close();
        ssock = null;
      } catch (IOException e) {
      }
    }
  }

  public void shutdown() {
    logger.info("MCISme MML Console shutting down");
    synchronized (this) {
      close();
      interrupt();
      try {
        wait();
      } catch (InterruptedException e) {
      }
    }
    logger.info("MCISme MML Console down");
  }

  class MMLProcessor extends Thread {
    Socket sock;
    String client;
    BufferedReader rd;
    BufferedWriter wr;
    boolean authenticated = false;

    public MMLProcessor( Socket sock ) throws IOException {
      this.sock = sock;
      rd = new BufferedReader(new InputStreamReader(sock.getInputStream()));
      wr = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
      client = sock.getInetAddress().getHostAddress()+":"+sock.getPort();
    }

    protected void sendAnswer( String answ ) throws IOException {
      wr.write(answ);
      wr.write('\n');
      wr.flush();
      logger.debug(client+" <- "+answ);
    }

    protected boolean authenticate( String command ) {
      // todo - make auth logic
      return true;
    }

    public void run() {
      logger.info("MML client connected: "+client);
      String line = null;
      try {
        while( (line = rd.readLine()) != null ) {
          logger.debug(client+" -> "+line);
          if( !authenticated ) {
            if( line.startsWith("auth") || line.startsWith("AUTH") ) {
              authenticated = authenticate(line);
            } else {
              sendAnswer("EXECUTED: ErrorCode=1");
            }
          } else {
            if( line.equalsIgnoreCase("quit") ) {
              logger.info(client+" end session");
              break;
            }
          }
        }
      } catch (IOException e) {
        logger.error("I/O error while communicating with client", e);
      }
      if( rd != null ) {
        try {
          rd.close();
        } catch (IOException e) {
        }
      }
      if( wr != null ) {
        try {
          wr.close();
        } catch (IOException e) {
        }
      }
      if( sock != null ) {
        try {
          sock.close();
        } catch (IOException e) {
        }
      }
    }
  }
}
