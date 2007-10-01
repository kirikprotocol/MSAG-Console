package ru.sibinco.smsx.network.smscconsole;


import java.io.*;
import java.net.Socket;
import java.util.ArrayList;

import org.apache.log4j.Category;
import com.eyeline.sme.utils.worker.IterativeWorker;

/**
 * User: artem
 * Date: 17.07.2007
 */

class SmscConsoleClientImpl implements SmscConsoleClient {

  private static final Category log = Category.getInstance("CONSOLE CLIENT");

  private Socket socket;

  private final String host;
  private final int port;
  private final int timeout;
  private final String login;
  private final String password;

  private long lastWorkTime = System.currentTimeMillis();
  private boolean connected;
  private final ConnectionCloser closer;

  SmscConsoleClientImpl(String host, int port, int timeout, String login, String password) {
    this.host = host;
    this.port = port;
    this.timeout = timeout;
    this.login = login;
    this.password = password;

    closer = new ConnectionCloser();
    closer.start();
  }

  public void connect() throws SmscConsoleClientException{
    if (connected)
      return;
    connected = true;

    try {
      enterToConsole();
    } catch (IOException e) {
      throw new SmscConsoleClientException(e);
    }
  }

  public void close() {
    if (!connected)
      return;
    connected = false;
    log.info("Close called");

    leaveConsole();
  }


  private void enterToConsole() throws IOException {
    socket = new Socket(host, port);
    if (!authentication())
      throw new IOException("Authentication failed");
  }

  private void leaveConsole() {
    log.info("Leave console called");
    if (socket != null) {
      try {
        sendCommandInt("quit");
      } catch (IOException e) {
      }
      try {
        socket.close();
      } catch (IOException e) {
      }
    }
    socket=null;
  }

  private boolean authentication() throws IOException {
    SmscConsoleResponse resp = getResponse();
    if(resp==null||!resp.isSuccess()) {
      log.error("Can't auth");
      return false;
    }

    resp = sendCommandInt(login);
    if(resp==null||!resp.isSuccess()) {
      log.error("Can't auth: login is incorrect");
      return false;
    }


    resp = sendCommandInt(password);
    if(resp==null||!resp.isSuccess()) {
      log.error("Can't auth: password is incorrect");
      return false;
    }

    log.info("Auth ok");
    return true;
  }



  private SmscConsoleResponse sendCommandInt(String command) throws IOException {
    log.info("Cmd: " + command);
    // Send command
    PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
    pw.println(command);
    pw.flush();

    // Get response
    return getResponse();
  }

  public SmscConsoleResponse sendCommand(String command) throws SmscConsoleClientException {
    if (!connected)
      throw new SmscConsoleClientException("Client not connected");

    if (socket == null) {
      try {
        enterToConsole();
      } catch (IOException e) {
        log.error(e,e);
        throw new SmscConsoleClientException(e);
      }
    }

    lastWorkTime = System.currentTimeMillis();
    closer.nextIteration();

    // First try
    try {
      return sendCommandInt(command);
    } catch (IOException e) {
      log.error(e,e);
    }

    // Reconnect
    try {
      log.info("Trying to reconnect...");
      leaveConsole();
      enterToConsole();
    } catch (IOException ee) {
      log.error(ee,ee);
      throw new SmscConsoleClientException(ee);
    }

    // Second try
    try {
      return sendCommandInt(command);
    } catch (IOException e) {
      throw new SmscConsoleClientException("Can't send command to smsc console", e);
    }
  }

  /**
   *
   * @return null if handler is not connected or recieved unknown answer.
   * @throws IOException
   */
  private SmscConsoleResponse getResponse() throws IOException {
    BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    ArrayList list = new ArrayList();
    String s=null;

    while(true) {
      s = in.readLine();
      list.add(s);
      if(s.length() > 0 && s.charAt(0) != ' ')
        break;
    }

    final StringBuffer res = new StringBuffer();
    for (int i=0; i < list.size(); i++)
      res.append('\n').append(list.get(i));

    log.info("Resp:" + res.toString());

    String[] tok = s.split(" ");
    if (tok.length < 2) {
      return null;
    }
    boolean success = false;
    if (tok[0].length() > 0 && tok[0].charAt(0)=='+') {
      success = true;
    }
    String status = tok[1];
    String[] lines = (String[]) list.toArray(new String[0]);
    return new SmscConsoleResponse(success, status, lines);

  }


  private class ConnectionCloser extends IterativeWorker {

    int i=0;

    private ConnectionCloser() {
      super(log);
      setDelayBetweenIterations(timeout);
    }

    public void iterativeWork() {
      if (!isStarted())
        return;

      if (System.currentTimeMillis() - lastWorkTime > timeout && i == 1 && socket != null) {
        log.info("Connection to smsc console was closed by timeout");
        leaveConsole();
      }

      i = 1;
    }

    protected void stopCurrentWork() {
    }
  }

}
