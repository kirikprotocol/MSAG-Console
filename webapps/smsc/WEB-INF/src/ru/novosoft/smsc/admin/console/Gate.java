/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 3:08:43 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.io.IOException;

public abstract class Gate extends Thread
{
    protected org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

    private Console owner;

    protected int port;
    protected ServerSocket serverSocket = null;

    protected Gate(Console owner, int port) throws IOException {
        this.owner = owner;
        serverSocket = new ServerSocket(port);
    }

    boolean isStopping = false;
    Object closeSemaphore = new Object();

    protected abstract Session newSession(Console owner, Socket socket);

    public void run()
    {
        logger.debug("Starting listening on port "+port);
        while(!isStopping)
        {
            Socket socket = null;
            try {
                socket = serverSocket.accept();
                socket.setSoLinger(true, 0);
                logger.debug("Client "+socket.getInetAddress().getHostAddress()+" connected");
                Session session = newSession(owner, socket);
                owner.addSession(session);
                session.start();
            } catch (SocketException e) {
                logger.debug("Server socket closed.");
            } catch (IOException ex) {
                logger.error("Error accepting connection", ex );
                break;
            }
        }
        synchronized(closeSemaphore) {
            closeSemaphore.notifyAll();
        }
        logger.debug("Console closed");
    }

    public void close()
    {
        logger.debug( "close called");
        synchronized(closeSemaphore) {
            isStopping = true;
            try {
                serverSocket.close();
            } catch (Exception e) {
                logger.error("Can't close server socket", e);
            }
            try {
               closeSemaphore.wait();
            } catch (InterruptedException e) { }
        }
        /*while(sessions.size() > 0) {
            ((Session)sessions.get(0)).close();
        }*/
    }

}
