import mobi.eyeline.mcaia.protocol.BusyRequest;
import mobi.eyeline.protogen.framework.*;
import org.apache.log4j.Logger;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Properties;

/**
 * Created by Serge Lugovoy
 * Date: May 20, 2010
 * Time: 1:31:09 PM
 */
public class TestServer extends Server<TestServerConnection> {
  private static Logger logger = Logger.getLogger(TestServer.class);

  public TestServer(String configPrefix, Properties config) throws IOException {
    super(configPrefix, config);
  }

  @Override
  protected TestServerConnection onAccept() {
    return new TestServerConnection();
  }

  public static void main(String args[]) {
    TestServer server = null;
    try {
      Properties config = new Properties();
      try {
        config.load(new FileReader(args[0]));
      } catch (IOException e) {
        System.out.println("Could not load config "+e.getMessage());
        return;
      }
      server = new TestServer("test", config);
      BufferedReader rd = new BufferedReader( new InputStreamReader(System.in) );
      String command = null;
      while(!Thread.currentThread().isInterrupted()) {
        System.out.print("> ");
        command = rd.readLine();
        if( command == null ) break;
        if( command.equalsIgnoreCase("quit") ) break;
        int pos = command.indexOf(' ');
        if( pos == -1 ) {
          System.out.println("invalid command, should be \"+71919189 +7906342344\"");
          continue;
        }
        String caller = command.substring(0, pos);
        String called = command.substring(pos+1);
        BusyRequest req = new BusyRequest(0, caller, called, System.currentTimeMillis(), (byte)1, (byte)1);
        TestServerConnection con = server.getConnection("test");
        if( con == null ) {
          System.out.println("No clients connected");
          continue;
        }
        con.send(req);
        System.out.println("sent: "+req);
      }
    } catch (Throwable e) {
      logger.error("Unexpected exception", e);
    } finally {
      if( server != null ) {
        server.shutdown();
      }
    }
  }
}
