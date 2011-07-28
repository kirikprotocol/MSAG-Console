package mobi.eyeline.dcpgw.admin;

import mobi.eyeline.dcpgw.Gateway;
import mobi.eyeline.dcpgw.admin.protogen.protocol.UpdateConfig;
import mobi.eyeline.dcpgw.admin.protogen.protocol.UpdateConfigResp;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.PDU;
import org.apache.log4j.Logger;

import java.io.*;
import java.net.Socket;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.07.11
 * Time: 14:48
 */
class Connect extends Thread {

   private static Logger log = Logger.getLogger(Connect.class);

   private Socket client = null;
   private InputStream is = null;
   private OutputStream os = null;

   public Connect(Socket clientSocket) {
     client = clientSocket;
     try {
      is = client.getInputStream();
      os = client.getOutputStream();
     } catch(Exception e1) {
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

          int tag = buffer.removeInt();

          int seqNum = buffer.removeInt();

          UpdateConfigResp configUpdateResp;
          try {
              Gateway.updateConfiguration();
              configUpdateResp = new UpdateConfigResp(seqNum, 0, "ok");
          } catch (XmlConfigException e) {
              configUpdateResp = new UpdateConfigResp(seqNum, 1, "Couldn't update configuration:"+e.getMessage());
          }

          serialize(configUpdateResp, os);

          is.close();
          os.close();
          client.close();
      } catch (IOException e) {
          log.error(e);
          // todo ?
      }
   }

   private static void serialize(PDU request, OutputStream os) throws IOException {
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
