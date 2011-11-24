package ru.novosoft.smsc.admin.mcisme;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.mcisme.MCISmeException.Status;
import ru.novosoft.smsc.util.Address;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Collection;
import java.util.Properties;
import java.util.Vector;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * author: Aleksandr Khalitov
 */
public class MMLConsole extends Thread{

  private final static int ERRC_OK = 0;
  private final static int ERRC_AUTH_REQUIRED = 1; // session is not authenticated yet, expect LOGIN command
  private final static int ERRC_AUTH_FAILED = 2;   // invalid user/pass
  private final static int ERRC_INVALID_COMMAND = 3;   // invalid command
  private final static int ERRC_BADFORMAT_COMMAND = 4;   // command bad formatted, can't be parsed
  private final static int ERRC_COMMUNICATION_ERROR = 5;   // error communicating with profile server
  private final static int ERRC_INVALID_SERVICE = 6;   // error communicating with profile server

  private static final int CLIENT_INACTIVITY_TIMEOUT = 60000; // after this timeout client will be disconnected

  public final static String  serviceNotify = "restr_notif";
  public final static String  serviceWantNotify = "in_network";


  private final static Logger logger = Logger.getLogger("MMLConsole");

  private final String user;
  private final String pass;
  private ServerSocket ssock;

  private final MCISme mciSme;

  private final Collection<MMLProcessor> processors = new Vector<MMLProcessor>();

  private final Lock processorsLock = new ReentrantLock();

  public MMLConsole(Properties config, MCISme sme) throws AdminException{
    mciSme = sme;
    int port;
    try {
      port = Integer.parseInt(config.getProperty("port"));
      user = config.getProperty("user");
      pass = config.getProperty("pass");
    } catch (Exception e) {
      throw new MCISmeException("Could not load config file", e);
    }
    try {
      ssock = new ServerSocket(port);
    } catch (IOException e) {
      throw new MCISmeException("Could not load config file", e);
    }
    start();
  }

  public void run() {
    try {
      while(ssock != null){
        Socket sock;
        try {
          sock = ssock.accept();
        } catch (Exception e) {
          if( ssock != null ) logger.error("Socket accept failed", e);
          break;
        }

        try {
          MMLProcessor proc = new MMLProcessor(sock);
          addProcessor(proc);
          proc.start();
        } catch (Exception e) {
          logger.error(e, e);
        }
      }
    } finally {
      shutdownProcessors();
      close();
    }
  }

  private void addProcessor(MMLProcessor pr) {
    try{
      processorsLock.lock();
      processors.add(pr);
    }finally {
      processorsLock.unlock();
    }
  }
  private void removeProcessor(MMLProcessor pr) {
    try{
      processorsLock.lock();
      processors.remove(pr);
    }finally {
      processorsLock.unlock();
    }
  }

  private void shutdownProcessors() {
    try{
      processorsLock.lock();
      for (MMLProcessor pr : processors) {
        pr.shutdown();
      }
    }finally {
      processorsLock.unlock();
    }
  }


  protected void close() {
    if(ssock != null ) {
      try {
        ssock.close();
        ssock = null;
      } catch (IOException ignored) {
      }
    }
  }

  public void shutdown() {

    if(logger.isDebugEnabled()) {
      logger.debug("MCISme MML Console shutting down");
    }
    synchronized (this) {
      close();
      interrupt();
      try {
        join();
      } catch (InterruptedException ignored) {
      }
    }
    if(logger.isDebugEnabled()) {
      logger.debug("MCISme MML Console down");
    }
  }

  class MMLProcessor extends Thread {
    // commands format
    // LOGIN: USER='username', PASS='password'
    private final Pattern loginPattern = Pattern.compile("(?ix)login: \\s* user='(.*)' \\s* , \\s* pass='(.*)' \\s* ;");
    // GET: MSISDN='+79...';
    private final Pattern getPattern = Pattern.compile("(?ix)get: \\s* msisdn='(.*)' \\s* ;");
    // ADD: MSISDN='+79...', SERV_ID='in_network';
    private final Pattern addPattern = Pattern.compile("(?ix)add: \\s* msisdn='(.*)' \\s* , \\s* serv_id='(.*)' \\s* ;");
    // REM: MSISDN='+79...', SERV_ID='in_network';
    private final Pattern remPattern = Pattern.compile("(?ix)rem: \\s* msisdn='(.*)' \\s* , \\s* serv_id='(.*)' \\s* ;");

    private final Socket sock;
    private final String client;
    private boolean authenticated;

    public MMLProcessor(Socket sock) throws IOException {
      this.sock = sock;
      this.sock.setSoTimeout(CLIENT_INACTIVITY_TIMEOUT);
      client = sock.getInetAddress().getHostAddress()+":"+sock.getPort();
    }

    protected void sendAnswer(BufferedWriter wr, String data, int errorCode ) throws IOException {
      String answ = "EXECUTED: ErrorCode="+errorCode+","+data+';'+'\n';
      wr.write(answ);
      wr.flush();
      if(logger.isDebugEnabled()) {
        logger.debug(client+" <- "+answ);
      }
    }

    protected void sendAnswer(BufferedWriter wr, int errorCode) throws IOException {
      String answ = "EXECUTED: ErrorCode="+errorCode+';'+'\n';
      wr.write(answ);
      wr.flush();
      if(logger.isDebugEnabled()) {
        logger.debug(client+" <- "+answ);
      }
    }

    protected boolean authenticate(BufferedWriter wr, String command ) throws IOException {
      Matcher matcher = loginPattern.matcher(command);
      if(matcher.matches() ) {
        String u = command.substring(matcher.start(1), matcher.end(1));
        String p = command.substring(matcher.start(2), matcher.end(2));
        if(u.equals(user) && p.equals(pass) ) {
          sendAnswer(wr, ERRC_OK);
          return true;
        }
      } else {
        sendAnswer(wr, ERRC_BADFORMAT_COMMAND);
      }
      sendAnswer(wr, ERRC_AUTH_FAILED);
      return false;
    }

    protected void getInfo(BufferedWriter wr, String command) throws IOException {
      Matcher matcher = getPattern.matcher(command);
      if(! matcher.matches() ) {
        sendAnswer(wr, ERRC_BADFORMAT_COMMAND);
        return;
      }

      String _msisdn = command.substring(matcher.start(1), matcher.end(1));

      Profile profile = tryGetProfile(wr, _msisdn);

      if(profile == null) {
        return;
      }

      StringBuilder sb = new StringBuilder(64);
      sb.append("MSISDN='").append(_msisdn).append('\'');
      if( !profile.isNotify()) {
        sb.append(", SERV_ID='"+serviceNotify+'\'');
      }
      if( profile.isWantNotifyMe()) {
        sb.append(", SERV_ID='"+serviceWantNotify+'\'');
      }
      sendAnswer(wr, sb.toString(), ERRC_OK);
    }


    private Profile tryGetProfile(BufferedWriter wr, String _msisdn) throws IOException {
      if(!Address.validate(_msisdn)) {
        sendAnswer(wr, ERRC_BADFORMAT_COMMAND);
        return null;
      }
      Profile profile = null;
      Address subscr = new Address(_msisdn);
      try{
        profile = mciSme.getProfile(subscr);
      }catch (MCISmeException e) {
        logger.error(e,e);
        if(e.getStatus() == Status.SUBSCRIBER_NOT_FOUND) {
          profile = new Profile();
          profile.setSubscriber(subscr);
        }else {
          sendAnswer(wr, ERRC_COMMUNICATION_ERROR);
        }
      }catch (AdminException e) {
        logger.error(e,e);
        sendAnswer(wr, ERRC_COMMUNICATION_ERROR);
      }
      return profile;
    }

    protected void addService(BufferedWriter wr, String command) throws IOException {
      Matcher matcher = addPattern.matcher(command);
      if(!matcher.matches()) {
        sendAnswer(wr, ERRC_BADFORMAT_COMMAND);
        return;
      }
      String msisdn = command.substring(matcher.start(1), matcher.end(1));
      String service = command.substring(matcher.start(2), matcher.end(2));
      Profile profile = tryGetProfile(wr, msisdn);
      if(profile == null) {
        return;
      }
      if(service.equalsIgnoreCase(serviceNotify) ) {
        profile.setNotify(false);
      } else if(service.equalsIgnoreCase(serviceWantNotify) ) {
        profile.setWantNotifyMe(true);
      } else {
        sendAnswer(wr, ERRC_INVALID_SERVICE);
        return;
      }
      saveProfile(profile, wr);
    }


    private void saveProfile(Profile profile, BufferedWriter wr) throws IOException{
      try{
        mciSme.saveProfile(profile);
        sendAnswer(wr, ERRC_OK);
      }catch (AdminException e) {
        logger.error(e,e);
        sendAnswer(wr, ERRC_COMMUNICATION_ERROR);
      }
    }

    protected void removeService(BufferedWriter wr, String command) throws IOException {
      Matcher matcher = remPattern.matcher(command);
      if(!matcher.matches() ) {
        sendAnswer(wr, ERRC_BADFORMAT_COMMAND);
        return;
      }
      String msisdn = command.substring(matcher.start(1), matcher.end(1));
      String service = command.substring(matcher.start(2), matcher.end(2));
      Profile profile = tryGetProfile(wr, msisdn);
      if(profile == null) {
        return;
      }
      if(service.equalsIgnoreCase(serviceNotify) ) {
        profile.setNotify(true);
      } else if( service.equalsIgnoreCase(serviceWantNotify) ) {
        profile.setWantNotifyMe(false);
      } else {
        sendAnswer(wr, ERRC_INVALID_SERVICE);
        return;
      }
      saveProfile(profile, wr);
    }

    public void run() {
      if(logger.isDebugEnabled()) {
        logger.debug("MML client connected: "+client);
      }
      String line;
      BufferedReader rd = null;
      BufferedWriter wr = null;
      try {
        rd = new BufferedReader(new InputStreamReader(sock.getInputStream()));
        wr = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
        while((line = rd.readLine()) != null ) {
          if(logger.isDebugEnabled()) {
            logger.debug(client+" -> "+line);
          }
          if("quit".equalsIgnoreCase(line)) {
            if(logger.isDebugEnabled()) {
              logger.debug(client+" end session");
            }
            break;
          }
          if(!authenticated ) {
            auth(line, wr);
          } else {
            handleCommand(line, wr);
          }
        }
      } catch (IOException e) {
        logger.error("I/O error while communicating with client", e);
      } finally {
        close(rd, wr, sock);
        removeProcessor(this);
      }
    }

    private void auth(String line, BufferedWriter wr) throws IOException{
      if( line.startsWith("LOGIN") || line.startsWith("login") ) {
        authenticated = authenticate(wr, line);
      } else {
        sendAnswer(wr, ERRC_AUTH_REQUIRED);
      }

    }

    private void handleCommand(String line, BufferedWriter wr) throws IOException{
      if( line.startsWith("ADD") || line.startsWith("add") ) {
        addService(wr, line);
      } else if( line.startsWith("REM") || line.startsWith("rem") ) {
        removeService(wr, line);
      } else if(line.startsWith("GET") || line.startsWith("get") ) {
        getInfo(wr, line);
      } else {
        sendAnswer(wr, ERRC_INVALID_COMMAND);
      }
    }

    private void close(BufferedReader rd, BufferedWriter wr, Socket sock) {
      if(rd != null) {
        try{
          rd.close();
        }catch (IOException ignored){}
      }
      if(wr != null) {
        try{
          wr.close();
        }catch (IOException ignored){}
      }
      if(sock != null) {
        try{
          sock.close();
        }catch (IOException ignored){}
      }
    }

    public void shutdown() {
      try {
        sock.close();
        interrupt();
        join();
      } catch (Exception ignored) {
      }
    }
  }

}
