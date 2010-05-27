import mobi.eyeline.mcaia.protocol.TestServerProtocol;
import mobi.eyeline.mcaia.protocol.TestServerProtocolTag;
import mobi.eyeline.protogen.framework.BufferReader;
import mobi.eyeline.protogen.framework.BufferWriter;
import mobi.eyeline.protogen.framework.PDU;
import mobi.eyeline.protogen.framework.ServerConnection;
import java.io.IOException;
import java.net.Socket;
import java.util.HashMap;
import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: May 20, 2010
 * Time: 1:37:53 PM
 */
public class TestServerConnection extends ServerConnection  {
  protected TestServerConnection() {
  }

  @Override
  public void init(String configPrefix, Properties config, Socket sock, HashMap<String, ServerConnection> conMap) throws IOException {
    super.init(configPrefix, config, sock, conMap);
    rename(configPrefix);
  }

  public void handle(PDU object) {
    switch(TestServerProtocolTag.valueOf(object.getTag())) {
      case BusyResponse:
        System.out.println("rcvd: "+object.toString());
        break;
      default:
        System.out.println("invl: "+object.toString());
    }
  }

  @Override
  protected void onSend(BufferWriter bufferWriter, PDU pdu) throws IOException {
    TestServerProtocol.encodeMessage(bufferWriter, pdu);
  }

  @Override
  protected PDU onReceive(BufferReader bufferReader) throws IOException {
    return TestServerProtocol.decodeMessage(bufferReader);
  }
}
