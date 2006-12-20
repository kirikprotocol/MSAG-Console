package org.gjt.sp.jedit;

import java.net.Socket;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.PrintWriter;
import java.util.Iterator;
import java.util.HashSet;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 27.09.2006
 * Time: 16:07:10
 * To change this template use File | Settings | File Templates.
 */
public class LiveConnect implements ActionListener {
  private String host;
  private int port;
  private HashSet ruleIds = new HashSet();

  public LiveConnect(String host, int port) {
      this.host = host;
      this.port = port;
      ruleIds = new HashSet();
  }

  public void actionPerformed(ActionEvent e) {
      try {
      Socket socket = new Socket(host,port);
      PrintWriter writer = new PrintWriter(socket.getOutputStream());
      synchronized(ruleIds) {
        for (Iterator i = ruleIds.iterator();i.hasNext();) {
          writer.println((String)i.next());
        }
      }
        writer.close();
        socket.close();
      } catch (Exception ee) {
        ee.printStackTrace();
      }
  }

  public void addRuleId(String complexRuleId) {
     synchronized(ruleIds) {
       ruleIds.add(complexRuleId);
     }
  }

  public void removeRuleId(String complexRuleId) {
     synchronized(ruleIds) {
       ruleIds.remove(complexRuleId.toUpperCase());
     }
  }
}
