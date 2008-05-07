package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.jsp.AutostartService;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.admin.AdminException;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Properties;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.net.ServerSocket;
import java.net.Socket;

import sun.misc.Regexp;

/**
 * Created by Serge Lugovoy
 * Date: May 4, 2008
 * Time: 3:54:48 PM
 */
public class MMLConsole extends Thread implements AutostartService, SMEAppContext {
  public final static int ERRC_OK = 0;
  public final static int ERRC_AUTH_REQUIRED = 1; // session is not authenticated yet, expect LOGIN command
  public final static int ERRC_AUTH_FAILED = 2;   // invalid user/pass

  // commands format
  // LOGIN: USER='username', PASS='password'
  Pattern loginPattern = Pattern.compile("(?ix)login: \\s* user=\\'(.*)\\', \\s* pass=\\'(.*)\\'\\;");

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

    protected void sendAnswer( int errorCode ) throws IOException {
      String answ = "EXECUTED: ErrorCode="+errorCode;
      wr.write(answ);
      wr.write('\n');
      wr.flush();
      logger.debug(client+" <- "+answ);
    }

    protected boolean authenticate( String command ) {
      Matcher matcher = loginPattern.matcher(command);
      if( matcher.matches() ) {
        logger.debug("Command matched "+matcher.groupCount());
        String u = command.substring(matcher.start(1), matcher.end(1));
        String p = command.substring(matcher.start(2), matcher.end(2));
        if( u.equals(user) && p.equals(pass) ) return true;
      } else {
        logger.debug("Command not matched");
      }
      return false;
    }

    public void run() {
      logger.info("MML client connected: "+client);
      String line = null;
      try {
        while( (line = rd.readLine()) != null ) {
          logger.debug(client+" -> "+line);
          if( line.equalsIgnoreCase("quit") ) {
            logger.info(client+" end session");
            break;
          }
          if( !authenticated ) {
            if( line.startsWith("LOGIN") || line.startsWith("login") ) {
              authenticated = authenticate(line);
              if( authenticated ) sendAnswer(ERRC_OK);
              else sendAnswer(ERRC_AUTH_FAILED);
            } else {
              sendAnswer(ERRC_AUTH_REQUIRED);
            }
          } else {
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
