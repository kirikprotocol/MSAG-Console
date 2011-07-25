package mobi.eyeline.dcpgw.admin;

import mobi.eyeline.dcpgw.Gateway;
import mobi.eyeline.dcpgw.admin.protogen.protocol.UpdateConfig;
import mobi.eyeline.dcpgw.admin.protogen.protocol.UpdateConfigResp;
import mobi.eyeline.informer.util.config.XmlConfigException;
import org.apache.log4j.Logger;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
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
   private ObjectInputStream ois = null;
   private ObjectOutputStream oos = null;

   public Connect(Socket clientSocket) {
     client = clientSocket;
     try {
      ois = new ObjectInputStream(client.getInputStream());
      oos = new ObjectOutputStream(client.getOutputStream());
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
          UpdateConfig uc = (UpdateConfig) ois.readObject();
          log.debug("Read update config object from object input stream, sequence number '"+uc.getSeqNum()+"'.");
          ois.close();

          Gateway.updateConfiguration();
          UpdateConfigResp ucr = new UpdateConfigResp();
          oos.writeObject(ucr);
          oos.flush();
          oos.close();
          client.close();
      } catch (ClassNotFoundException e) {
          log.error(e);
          // todo ?
      } catch (IOException e) {
          log.error(e);
          // todo ?
      } catch (XmlConfigException e) {
          log.error(e);
          // todo ?
      }

   }

}
