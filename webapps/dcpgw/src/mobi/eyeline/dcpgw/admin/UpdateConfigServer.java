package mobi.eyeline.dcpgw.admin;

import org.apache.log4j.Logger;

import java.io.IOException;
import java.net.*;

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
        while (true) {
            try {
                log.debug("Waiting for connections.");
                Socket client = serverSocket.accept();
                log.debug("Accepted a connection from: " + client.getInetAddress());
                Connect c = new Connect(client);
            } catch (IOException e) {
                log.error(e);
                // todo ?
            }
        }
    }

}

