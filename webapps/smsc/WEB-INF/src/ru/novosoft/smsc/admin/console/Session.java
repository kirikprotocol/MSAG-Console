/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 15, 2002
 * Time: 4:21:23 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import java.io.*;
import java.net.Socket;
import java.net.SocketException;

public class Session extends Thread
{
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

    private final static String CONSOLE_GREATING = "Welcome to SMSC Remote Console.";
    private final static String CONSOLE_FAREWELL = "Exited from SMSC Remote Console.";
    private final static String CONSOLE_SIGN = "> ";

    private final static String COMMAND_QUIT = "quit";

    private Socket socket;
    private Console owner;

    private InputStream  is = null;
    private OutputStream os = null;

    public Session(Console owner, Socket socket) {
        this.owner = owner;
        this.socket = socket;
    }

    private void processInput()
        throws Exception
    {
        LineNumberReader reader = new LineNumberReader(new InputStreamReader(is));
        PrintWriter writer = new PrintWriter(os);

        writer.println(CONSOLE_GREATING+'\r');
        while (!isStopping) {
            writer.print(CONSOLE_SIGN); writer.flush();
            String input = reader.readLine();
            if (input != null) {
                if (input.equalsIgnoreCase(COMMAND_QUIT)) {
                    writer.println(CONSOLE_FAREWELL+'\r');
                    writer.flush(); sleep(1000);
                    break;
                }
                String output = owner.processCommand(input.trim());
                writer.println(output+'\r');
                writer.flush();
            }
        }
    }

    Object closeSemaphore = new Object();
    boolean isStopping = false;

    public void run()
    {
        try {
            is = socket.getInputStream();
            os = socket.getOutputStream();
            processInput();
        }
        catch (SocketException e) {
            logger.debug("Client "+socket.getInetAddress().getHostAddress()+" disconnected");
        } catch (IOException e) {
            logger.warn("I/O error occured for "+socket.getInetAddress().getHostAddress(), e);
        } catch (Exception e) {
            logger.error("Unexpected error occured for "+socket.getInetAddress().getHostAddress(), e);
        } finally {
            if( is != null ) try { is.close();} catch (Exception ee){};
            if( os != null ) try { os.close();} catch (Exception ee){};
            if( socket != null ) try { socket.close();} catch (Exception ee){};
        }
        owner.removeSession(this);
        synchronized(closeSemaphore) {
            closeSemaphore.notifyAll();
        }
    }

    public void close() {
        synchronized(closeSemaphore) {
            isStopping = true;
            try {
               closeSemaphore.wait();
            } catch (InterruptedException e) {}
        }
    }
}
