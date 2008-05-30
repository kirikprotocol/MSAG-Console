package ru.sibinco.smpp.ub_sme.inman.test;

import java.io.OutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * Created by IntelliJ IDEA.
 * User: danger
 * Date: 12.03.2007
 * Time: 12:23:11
 * To change this template use File | Settings | File Templates.
 */
public class InManTestServer {

  public static void main(String[] args) {
    if (args.length == 0) {
      System.out.println("Ussage: java ru.sibinco.smpp.ub_sme.inman.test [port]");
    }
    InManTestServer inst = new InManTestServer();
    int port = Integer.parseInt(args[0]);
    inst.main(port);
  }

  public void main(int port) {
    Sender sender = null;
    ServerSocket ss = null;
    Socket sock = null;
    try {
      ss = new ServerSocket(port);
      sock = ss.accept();
      sender = new Sender(sock.getOutputStream());
      sender.start();
      InputStream is = sock.getInputStream();
      int b;
      while ((b = is.read()) != -1) {

      }
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      if (sender != null) {
        sender.stopThread();
      }
      if (ss != null) {
        try {
          ss.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
      if (sock != null) {
        try {
          sock.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
    }
  }

  ;

  class Sender extends Thread {
    OutputStream os;
    byte[] pdu = new byte[]{
        0x0, 0x0, 0x0, 0x3a, // PDU Length
        0x0, 0x7, // tag
        0x0, 0x1, // headerFormat
        0x0, 0x0, 0x0, 0x0, // dialogID
        0xb, 0x4d, 0x54, 0x53, 0x5f, 0x4e, 0x73, 0x6b, 0x5f, 0x53, 0x52, 0x49, // nmPolicy
        0x2, // contractType
        0x10, 0x2e, 0x31, 0x2e, 0x31, 0x2e, 0x37, 0x39, 0x31, 0x33, 0x39, 0x38, 0x36, 0x30, 0x30, 0x30, 0x35, // gsmSCFAddress
        0x0, 0x0, 0x0, 0xb, // serviceKeyOrError
        0xf, 0x32, 0x35, 0x30, 0x30, 0x31, 0x33, 0x39, 0x30, 0x30, 0x36, 0x32, 0x34, 0x38, 0x36, 0x33 // abImsi
    };
    boolean stop = false;

    public Sender(OutputStream os) {
      this.os = os;
    }

    public void stopThread() {
      this.stop = true;
    }

    public void run() {
      while (!stop) {
        try {
          os.write(pdu);
        } catch (IOException e) {
          return;
        }
      }
    }

  }
}
