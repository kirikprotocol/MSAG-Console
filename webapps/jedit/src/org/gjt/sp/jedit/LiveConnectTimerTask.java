package org.gjt.sp.jedit;

import org.gjt.sp.util.Log;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TimerTask;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 16.09.2010
 * Time: 12:00:25
 */
public class LiveConnectTimerTask extends TimerTask {

    private String host;
    private int port;
    private HashSet ruleIds = new HashSet();

    public LiveConnectTimerTask(String host, int port) {
          this.host = host;
          this.port = port;
          ruleIds = new HashSet();
    }
    
    public void run() {
        Socket socket = null;
        try {
            //System.out.println("LiveConnectTimerTask Create socket ... host="+host+" port="+port);
            socket = new Socket(host,port);
            //System.out.println("LiveConnectTimerTask Socket was created.");
            PrintWriter writer = new PrintWriter(socket.getOutputStream());
            if (ruleIds.size() >0) {
                synchronized(ruleIds) {
                    long startTime = System.currentTimeMillis();
                    for (Iterator i = ruleIds.iterator();i.hasNext();) {
                        String s = (String)i.next();
                        writer.println(s);
                        //System.out.println("LiveConnectTimerTask Write rule id:" + s);
                    }
                    int currentTime=(int)(System.currentTimeMillis()-startTime);
                    //System.out.println("LiveConnectTimerTask write time:"+currentTime+" ms");
                }
            }
            writer.close();
            //System.out.println("LiveConnectTimerTask Successfully wrote rules ids to socket.");
        } catch (UnknownHostException uhe){
            uhe.printStackTrace();
            System.out.println("LiveConnectTimerTask Unknown host="+host);
        } catch (IOException ioe) {
            ioe.printStackTrace();
            System.out.println("LiveConnectTimerTask Could not create socket: host="+host+" port="+port);
        } finally {
            if (socket != null){
                try{
                    //System.out.println("LiveConnectTimerTask Close socket ...");
                    socket.close();
                    //System.out.println("LiveConnectTimerTask Socket was closed.");
                } catch (IOException ioe){
                    ioe.printStackTrace();
                    System.out.println("Could not close socket: host="+host+" port="+port);
                }
            }
        }
    }

    public void addRuleId(String complexRuleId) {
        System.out.println("LiveConnectTimerTask.addRulesId Add rule id:"+complexRuleId);
        synchronized(ruleIds) {
            ruleIds.add(complexRuleId);
        }
    }

    public void removeRuleId(String complexRuleId) {
        System.out.println("LiveConnectTimerTask.removeRuleId Remove rule id:"+complexRuleId);
        synchronized(ruleIds) {
            ruleIds.remove(complexRuleId.toUpperCase());
        }
    }
    
}
