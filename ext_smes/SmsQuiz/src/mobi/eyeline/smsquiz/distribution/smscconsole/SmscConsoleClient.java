package mobi.eyeline.smsquiz.distribution.smscconsole;

import org.apache.log4j.Logger;

import java.net.Socket;
import java.io.*;
import java.util.LinkedList;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;

/**
 * author: alkhal
 */
public class SmscConsoleClient {
  private static Logger logger = Logger.getLogger(SmscConsoleClient.class);

  private String login;
  private String password;
  private String host;
  private int port;
  private long timeout;
  private Socket socket;

  private boolean connected;
  private long lastWorkTime;
  private ScheduledExecutorService scheduledConnCloser;

  public SmscConsoleClient(String login, String password, String host, int port, long timeout, long closerPeriod) {
    socket = null;
    connected = false;
    this.login = login;
    this.password = password;
    this.port = port;
    this.host = host;
    this.timeout = timeout;
    lastWorkTime = System.currentTimeMillis();
    scheduledConnCloser = Executors.newSingleThreadScheduledExecutor(new ThreadFactory() {
      public Thread newThread(Runnable r) {
        return new Thread(r,"ConnectionCloser");
      }
    });
    scheduledConnCloser.scheduleAtFixedRate(new ConnectionCloser(),10,closerPeriod, java.util.concurrent.TimeUnit.SECONDS);
  }

  public void connect() throws SmscConsoleException {
    if(connected) {
      return;
    }
    connected = true;
    try{
      enterToConsole();
    }
    catch (Exception e) {
      logger.error("Unable to connect to the console", e);
      throw new SmscConsoleException("Unable to connect to the console", e);
    }
    logger.info("Console connected");
  }

  public void close() {
    if(!connected) {
      return;
    }
    connected = false;
    leaveConsole();
    scheduledConnCloser.shutdown();
    logger.info("Console disconnected");
  }

  public SmscConsoleResponse sendCommand(String comm) throws SmscConsoleException {
    if(!connected) {
      throw new SmscConsoleException("You're not connected");
    }
    if(socket==null) {
      try {
        enterToConsole();
      } catch (IOException e) {
        logger.error("Unable to connect to the console", e);
        throw new SmscConsoleException("Unable to connect to the console", e);
      }
    }


    lastWorkTime = System.currentTimeMillis();

    try{
      return _sendCommand(comm);
    } catch (IOException e) {
        logger.error("Connection was closed or broked. Try reconnect...", e);
    }

    try{
      leaveConsole();
      enterToConsole();
    } catch (IOException e) {
        logger.error("Connection was closed or broken. Try reconnect...", e);
    }

    try{
      return _sendCommand(comm);
    } catch (IOException e) {
      logger.error("Connection was closed or broken. Connection failed.", e);
      throw new SmscConsoleException("Connection was closed or broken. Connection failed.", e);
    }
  }


  private void enterToConsole() throws IOException{
    socket = new Socket(host, port);
    if(!authentication()) {
      throw new IOException("Auth failed");
    }
  }

  private void leaveConsole() {
    if(socket!=null) {
      try {
        _sendCommand("quit");
      } catch (IOException e) {
        logger.error("Unable to say 'GoodBye'", e);
      }
      try {
        socket.close();
      } catch (IOException e) {
        logger.error("Unable to close connection", e);
      }
    }
  }

  private boolean authentication() throws IOException{
    SmscConsoleResponse resp = getResponse();
    if(resp==null||!resp.isSuccess()) {
      logger.error("Can't auth");
      return false;
    }

    resp = _sendCommand(login);
    if(resp==null||!resp.isSuccess()) {
      logger.error("Can't auth: login is incorrect");
      return false;
    }
    
    resp = _sendCommand(password);
    if(resp==null||!resp.isSuccess()) {
      logger.error("Can't auth: password is incorrect");
      return false;
    }

    logger.info("Auth ok");
    return true;
    
  }
  private SmscConsoleResponse _sendCommand(String str) throws IOException {
    PrintWriter writer = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
    writer.println(str);
    writer.flush();
    writer.close();
    return getResponse();
  }
  private SmscConsoleResponse getResponse() throws IOException {
    BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    LinkedList<String> list = new LinkedList<String>();
    String str;

    while(true) {
      str = in.readLine();
      list.add(str);
      if(str.length() > 0 && str.charAt(0) != ' ')
        break;
    }

    if(logger.isInfoEnabled()) {
      final StringBuffer res = new StringBuffer();
      for (String aList : list)
        res.append('\n').append(aList);
      logger.info("Response:" + res.toString());
    }

    String[] tok = str.split(" ");
    if (tok.length < 2) {
      return null;
    }
    boolean success = false;
    if (tok[0].length() > 0 && tok[0].charAt(0)=='+') {
      success = true;
    }
    String status = tok[1];
    String[] lines = list.toArray(new String[list.size()]);
    return new SmscConsoleResponse(success, status, lines);
  }


  private class ConnectionCloser implements Runnable{
    public void run() {
      if((System.currentTimeMillis() - lastWorkTime)>timeout) {
        leaveConsole();
      }
    }
  }
}
