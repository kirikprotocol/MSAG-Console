package mobi.eyeline.dcpgw.admin;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.admin.protogen.protocol.UpdateConfigResp;
import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 30.09.11
 * Time: 21:45
 */
public class UpdateConfigServer extends Thread {

    private static Logger log = Logger.getLogger(UpdateConfigServer.class);

    private ServerSocket serverSocket;

    public UpdateConfigServer(String host, int port) throws IOException {
        if (host!= null && !host.isEmpty()){
            serverSocket = new ServerSocket();
            serverSocket.bind(new InetSocketAddress(host,port));
            log.debug("Set update server host to '"+host+"'.");
        } else{
            serverSocket = new ServerSocket(port);
        }
        this.start();
    }

    public void run() {
        while (!isInterrupted()) {
            try {
                log.debug("Waiting for connections ...");
                Socket client = serverSocket.accept();
                log.debug("Accepted a connection from: " + client.getInetAddress());
                new Connect(client);
            } catch (IOException e) {
                log.error(e);
            }
        }
    }

    /**
     * Created by IntelliJ IDEA.
     * User: Stepanov Dmitry Nikolaevich
     * Date: 30.09.11
     * Time: 21:47
     */
    public static class Connect extends Thread {

           private Socket client = null;
           private InputStream is = null;
           private OutputStream os = null;

           public Connect(Socket clientSocket) {
             client = clientSocket;
             try {
                 is = client.getInputStream();
                 os = client.getOutputStream();
             } catch(IOException e1) {
                 try {
                    client.close();
                 }catch(Exception e) {
                    log.error(e.getMessage());
                 }
                 return;
             }
             this.start();
           }


           public void run() {
               try {
                   BufferReader buffer = new BufferReader(1024);
                   buffer.fillFully(is, 4);
                   int len = buffer.removeInt();

                   if (log.isDebugEnabled()) log.debug("Received packet len=" + len);
                   if (len > 0) buffer.fillFully(is, len);

                   if (log.isDebugEnabled()) log.debug("PDU received: " + buffer.getHexDump());

                   buffer.removeInt();

                   int seqNum = buffer.removeInt();

                   UpdateConfigResp configUpdateResp;

                   try {
                       Config.getInstance().update();
                       configUpdateResp = new UpdateConfigResp(seqNum, 0, "ok");
                   } catch (Exception e) {
                       configUpdateResp = new UpdateConfigResp(seqNum, 1, "Couldn't update configuration:"+e.getMessage());
                   }

                   serialize(configUpdateResp, os);

                   is.close();
                   os.close();
                   client.close();
               } catch (IOException e) {
                   log.error(e);
               }
           }

           private void serialize(mobi.eyeline.protogen.framework.PDU request, OutputStream os) throws IOException {
               BufferWriter writer = new BufferWriter();
               int pos = writer.size();
               writer.appendInt(0); // write 4 bytes for future length
               writer.appendInt(request.getTag());
               writer.appendInt(request.getSeqNum());
               request.encode(writer);
               int len = writer.size()-pos-4;
               writer.replaceInt( len,  pos); // fill first 4 bytes with actual length

               if (log.isDebugEnabled())
                   log.debug("Sending PDU: " + writer.getHexDump());

               writer.writeBuffer(os);
               os.flush();
           }
        }
}
