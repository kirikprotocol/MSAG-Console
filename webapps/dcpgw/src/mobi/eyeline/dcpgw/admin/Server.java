package mobi.eyeline.dcpgw.admin;

import org.apache.log4j.Logger;

import java.io.IOException;
import java.net.*;

public class Server extends Thread {

    private static Logger log = Logger.getLogger(Server.class);

    private ServerSocket serverSocket;

    public Server(int port) throws IOException {
        this.serverSocket = new ServerSocket(port);
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

