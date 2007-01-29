package ru.sibinco.smsx.services.sponsored;

import org.apache.log4j.Category;

import java.io.*;
import java.net.Socket;
import java.util.ArrayList;

/**
 * User: artem
 * Date: 22.01.2007
 */

final class SMSXProfileCommandHandler {
  private static org.apache.log4j.Category cat = Category.getInstance(SMSXProfileCommandHandler.class);
  private Socket socket;

  public void connect(String host, int port, int timeout) throws IOException {
    socket = new Socket(host, port);
    socket.setSoTimeout(timeout);
  }

  public void connect(String host, int port) throws IOException {
    connect(host, port, 0);
  }

  public boolean authentication(String login,String password) throws IOException {
    if(socket==null){
      return false;
    }
    Response resp = getResponse();
    if(resp==null||!resp.isSuccess()){
      return false;
    }
    cat.debug("Sending login.");
    resp = sendCommand(login);
    if(resp==null||!resp.isSuccess()){
      return false;
    }
    cat.debug("Sending password.");
    resp = sendCommand(password);
    if(resp==null||!resp.isSuccess()){
      return false;
    }

    return true;
  }

  public void close() {
    if (socket != null) {
      try {
        socket.close();
      } catch (IOException e) {
      }
    }
    socket=null;
  }

  /**
   *
   * @return null if handler is not connected or recieved unknown answer.
   * @throws IOException
   */
  public Response sendCommand(String command) throws IOException {
    if (null == socket) {
      return null;
    }
    PrintWriter pw = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
    pw.println(command);
    pw.flush();
    cat.debug("Sent command.");
    return getResponse();
  }

  /**
   *
   * @return null if handler is not connected or recieved unknown answer.
   * @throws IOException
   */
  public Response getResponse() throws IOException{
    cat.debug("Get response.");
    if (socket == null) {
      return null;
    }
    BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    ArrayList list = new ArrayList();
    String s=null;

    while(true){
      if(in.ready()){
        s = in.readLine();
        list.add(s);
        if(!s.startsWith(" ")){
          break;
        }
      }
      try {
        Thread.sleep(1000);
      } catch (InterruptedException e) {
        return null;
      }
    }
    cat.debug("Last line "+s+".");
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

  //Response

  public class Response {
    private boolean success;
    private String status;
    private String[] lines;
    Response(boolean success, String status, String[] responseLines) {
      this.success=success;
      this.status=status;
      this.lines=responseLines;
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
