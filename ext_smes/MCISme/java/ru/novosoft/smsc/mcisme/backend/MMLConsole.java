package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.jsp.AutostartService;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.jsp.SMEAppContext;
import ru.novosoft.smsc.admin.AdminException;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;
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
  public final static int ERRC_INVALID_COMMAND = 3;   // invalid command
  public final static int ERRC_BADFORMAT_COMMAND = 4;   // command bad formatted, can't be parsed
  public final static int ERRC_COMMUNICATION_ERROR = 5;   // error communicating with profile server
  public final static int ERRC_INVALID_SERVICE = 6;   // error communicating with profile server

  public final static String  serviceNotify = "restr_notif";
  public final static String  serviceWantNotify = "in_network";


  // commands format
  // LOGIN: USER='username', PASS='password'
  Pattern loginPattern = Pattern.compile("(?ix)login: \\s* user=\\'(.*)\\' \\s* , \\s* pass=\\'(.*)\\' \\s* \\;");
  // GET: MSISDN='+79...';
  Pattern getPattern = Pattern.compile("(?ix)get: \\s* msisdn=\\'(.*)\\' \\s* \\;");
  // ADD: MSISDN='+79...', SERV_ID='in_network';
  Pattern addPattern = Pattern.compile("(?ix)add: \\s* msisdn=\\'(.*)\\' \\s* , \\s* serv_id=\\'(.*)\\' \\s* \\;");
  // REM: MSISDN='+79...', SERV_ID='in_network';
  Pattern remPattern = Pattern.compile("(?ix)rem: \\s* msisdn=\\'(.*)\\' \\s* , \\s* serv_id=\\'(.*)\\' \\s* \\;");

  static Logger logger = Logger.getLogger(MMLConsole.class);
  int port = 0;
  String  profilerHost;
  int     profilerPort;
  String  user;
  String  pass;
  ServerSocket ssock;

  private final Vector processors = new Vector();

  public void start(SMSCAppContext ctx) {
    try {
      File folder = ctx.getHostsManager().getServiceInfo("MCISme").getServiceFolder();
      Properties config = new Properties();
      config.load(new FileInputStream(new File(new File(folder, "conf"), "mmlconsole.properties")));
      profilerHost = config.getProperty("profiler.host");
      if( profilerHost == null || profilerHost.length() == 0 ) {
        logger.error("profiler.host parameter not specified");
        return;
      }
      profilerPort = Integer.valueOf(config.getProperty("profiler.port")).intValue();
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
    try {
      while( ssock != null ) {
        Socket sock = null;
        try {
          sock = ssock.accept();
        } catch (Exception e) {
          if( ssock != null ) logger.error("Socket accept failed", e);
          break;
        }

        try {
          MMLProcessor proc = new MMLProcessor(sock);
          processors.add(proc);
          proc.start();
        } catch (Exception e) {
          logger.error("", e);
        }
      }
    } finally {
      // Shutdown all mml processors
      Object[] processorObjects = processors.toArray();
      for (int i=0; i<processorObjects.length; i++) {
        ((MMLProcessor)processorObjects[i]).shutdown();
      }
      processors.clear();
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

    protected void sendAnswer( String data, int errorCode ) throws IOException {
      String answ = "EXECUTED: ErrorCode="+errorCode+","+data+';'+'\n';
      wr.write(answ);
      wr.flush();
      logger.debug(client+" <- "+answ);
    }

    protected void sendAnswer( int errorCode ) throws IOException {
      String answ = "EXECUTED: ErrorCode="+errorCode+';'+'\n';
      wr.write(answ);
      wr.flush();
      logger.debug(client+" <- "+answ);
    }

    protected boolean authenticate( String command ) throws IOException {
      Matcher matcher = loginPattern.matcher(command);
      if( matcher.matches() ) {
        String u = command.substring(matcher.start(1), matcher.end(1));
        String p = command.substring(matcher.start(2), matcher.end(2));
        if( u.equals(user) && p.equals(pass) ) {
          sendAnswer(ERRC_OK);
          return true;
        }
      } else {
        sendAnswer(ERRC_BADFORMAT_COMMAND);
      }
      sendAnswer(ERRC_AUTH_FAILED);
      return false;
    }

    protected void getInfo(String command) throws IOException {
      Matcher matcher = getPattern.matcher(command);
      if( matcher.matches() ) {
        String msisdn = command.substring(matcher.start(1), matcher.end(1));
        ProfStorageAccessor profStore = new ProfStorageAccessor(profilerHost, profilerPort);
        ProfileInfo profile = new ProfileInfo();
        if( profStore.getProfile(msisdn, profile) == 0 ) {
          StringBuffer sb = new StringBuffer(64);
          sb.append("MSISDN='").append(msisdn).append('\'');
          if( !profile.notify ) {
            sb.append(", SERV_ID='"+serviceNotify+'\'');
          }
          if( profile.wantNotifyMe ) {
            sb.append(", SERV_ID='"+serviceWantNotify+'\'');
          }
          sendAnswer(sb.toString(), ERRC_OK);
        } else {
          sendAnswer(ERRC_COMMUNICATION_ERROR);
        }
      } else {
        sendAnswer(ERRC_BADFORMAT_COMMAND);
      }
    }

    protected void addService(String command) throws IOException {
      Matcher matcher = addPattern.matcher(command);
      if( matcher.matches() ) {
        String msisdn = command.substring(matcher.start(1), matcher.end(1));
        String service = command.substring(matcher.start(2), matcher.end(2));
        ProfStorageAccessor profStore = new ProfStorageAccessor(profilerHost, profilerPort);
        ProfileInfo profile = new ProfileInfo();
        if( profStore.getProfile(msisdn, profile) == 0 ) {
          if( service.equalsIgnoreCase(serviceNotify) ) {
            profile.notify = false;
          } else if( service.equalsIgnoreCase(serviceWantNotify) ) {
            profile.wantNotifyMe = true;
          } else {
            sendAnswer(ERRC_INVALID_SERVICE);
            return;
          }
          if( profStore.setProfile(msisdn, profile) == 0 ) {
            sendAnswer(ERRC_OK);
          } else {
            sendAnswer(ERRC_COMMUNICATION_ERROR);
          }
        } else {
          sendAnswer(ERRC_COMMUNICATION_ERROR);
        }
      } else {
        sendAnswer(ERRC_BADFORMAT_COMMAND);
      }
    }

    protected void removeService(String command) throws IOException {
      Matcher matcher = remPattern.matcher(command);
      if( matcher.matches() ) {
        String msisdn = command.substring(matcher.start(1), matcher.end(1));
        String service = command.substring(matcher.start(2), matcher.end(2));
        ProfStorageAccessor profStore = new ProfStorageAccessor(profilerHost, profilerPort);
        ProfileInfo profile = new ProfileInfo();
        if( profStore.getProfile(msisdn, profile) == 0 ) {
          if( service.equalsIgnoreCase(serviceNotify) ) {
            profile.notify = true;
          } else if( service.equalsIgnoreCase(serviceWantNotify) ) {
            profile.wantNotifyMe = false;
          } else {
            sendAnswer(ERRC_INVALID_SERVICE);
            return;
          }
          if( profStore.setProfile(msisdn, profile) == 0 ) {
            sendAnswer(ERRC_OK);
          } else {
            sendAnswer(ERRC_COMMUNICATION_ERROR);
          }
        } else {
          sendAnswer(ERRC_COMMUNICATION_ERROR);
        }
      } else {
        sendAnswer(ERRC_BADFORMAT_COMMAND);
      }
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
            } else {
              sendAnswer(ERRC_AUTH_REQUIRED);
            }
          } else {
            if( line.startsWith("ADD") || line.startsWith("add") ) {
              addService(line);
            } else if( line.startsWith("REM") || line.startsWith("rem") ) {
              removeService(line);
            } else if( line.startsWith("GET") || line.startsWith("get") ) {
              getInfo(line);
            } else {
              sendAnswer(ERRC_INVALID_COMMAND);
            }
          }
        }
      } catch (IOException e) {
        logger.error("I/O error while communicating with client", e);
      } finally {
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
        processors.remove(this);
      }
    }

    public void shutdown() {
      try {
        sock.close();
        interrupt();
      } catch (Exception e) {
      }
    }
  }
}
