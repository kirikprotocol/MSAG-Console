package ru.sibinco.smpp.ub_sme.util;

import org.apache.log4j.Category;

import java.io.*;
import java.net.*;
import java.util.ArrayList;

/**
 * User: pasha
 * Date: 07.12.2006
 * Time: 12:04:38
 */
public class SMCXProfileCommandHandler {

  private static org.apache.log4j.Category Logger =
      Category.getInstance(SMCXProfileCommandHandler.class);

  private Socket socket;

  private final static long AWATING_CHAR_TIME = 50;

  private boolean connected = false;

  private int connectTimeout = 10000;
  private int readTimeout = 10000;

  public void connect(String host, int port) throws IOException {
    SocketAddress sa = new InetSocketAddress(host, port);
    socket = new Socket();
    socket.setSoTimeout(readTimeout);
    socket.connect(sa, connectTimeout);
    connected = true;
  }

  public boolean authentication(String login, String password) throws IOException {
    if (!connected) {
      throw new IOException("not connected");
    }
    Response resp = getResponse();
    if (resp == null || !resp.isSuccess()) {
      return false;
    }
    if (Logger.isDebugEnabled()) Logger.debug("sending login");
    resp = sendCommand(login);
    if (resp == null || !resp.isSuccess()) {
      return false;
    }
    if (Logger.isDebugEnabled()) Logger.debug("sending password");
    resp = sendCommand(password);
    return !(resp == null || !resp.isSuccess());

  }

  public void close() throws IOException {
    if (!connected) {
      throw new IOException("not connected");
    }
    socket.close();
  }

  public Response sendCommand(String command) throws IOException {
    if (!connected) {
      throw new IOException("not connected");
    }
    PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
    pw.println(command);
    pw.flush();
    if (Logger.isDebugEnabled()) Logger.debug("sent command");
    return getResponse();
  }

  public Response getResponse() throws IOException {
    if (!connected) {
      throw new IOException("not connected");
    }
    BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    ArrayList list = new ArrayList();
    String s = null;

    while (true) {
      if (in.ready()) {
        s = in.readLine();
        list.add(s);
        if (!s.startsWith(" ")) {
          break;
        }
      }
      try {
        Thread.sleep(AWATING_CHAR_TIME);
      } catch (InterruptedException e) {
        return null;
      }
    }
    if (Logger.isDebugEnabled()) Logger.debug("got response last line " + s);
    String[] tok = s.split(" ");
    if (tok.length < 2) {
      return null;
    }
    boolean success = false;
    if (tok[0].startsWith("+")) {
      success = true;
    }
    String status = tok[1];
    String[] lines = (String[]) list.toArray(new String[0]);
    return new Response(success, status, lines);

  }

  public int getConnectTimeout() {
    return connectTimeout;
  }

  public void setConnectTimeout(int connectTimeout) {
    this.connectTimeout = connectTimeout;
  }

  public int getReadTimeout() {
    return readTimeout;
  }

  public void setReadTimeout(int readTimeout) {
    this.readTimeout = readTimeout;
  }
  //Response

  public static class Response {
    private boolean success;
    private String status;
    private String[] lines;

    public Response(boolean success, String status, String[] responseLines) {
      this.success = success;
      this.status = status;
      this.lines = responseLines;
    }

    public String[] getResponseLines() {
      return lines;
    }

    public boolean isSuccess() {
      return success;
    }

    public String getStatus() {
      return status;
    }

  }

}

