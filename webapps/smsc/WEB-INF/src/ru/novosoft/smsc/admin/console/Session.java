/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 15, 2002
 * Time: 4:21:23 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.admin.console.parser.CommandLexer;
import ru.novosoft.smsc.admin.console.parser.CommandParser;

import java.io.*;
import java.net.Socket;
import java.net.SocketException;

public abstract class Session extends Thread
{
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

    private final static String COMMAND_QUIT = "quit";

    private Socket socket;
    private Console owner;

    private InputStream  is = null;
    private OutputStream os = null;

    public Session(Console owner, Socket socket) {
        this.owner = owner;
        this.socket = socket;
    }

    protected void greeting(PrintWriter writer) {};
    protected void farewell(PrintWriter writer) {};
    protected void prompt(PrintWriter writer) {};

    protected abstract void display(PrintWriter writer, CommandContext ctx);

    private void process()
        throws Exception
    {
        BufferedReader reader = new BufferedReader(new InputStreamReader(is));
        PrintWriter writer = new PrintWriter(os);

        greeting(writer);
        while (!isStopping) {
            prompt(writer);
            String input = reader.readLine();
            if (input == null) continue;
            if (input.equalsIgnoreCase(COMMAND_QUIT)) {
                farewell(writer); sleep(100); break;
            }
            CommandContext ctx = new CommandContext(owner.getSmsc());
            try {
                CommandLexer lexer = new CommandLexer(new StringReader(input));
                CommandParser parser = new CommandParser(lexer);
                parser.parse(ctx);
            }
            catch (Exception e) {
                ctx.setMessage(e.getMessage());
                ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
            }
            display(writer, ctx);
        }
    }

    Object closeSemaphore = new Object();
    boolean isStopping = false;

    public void run()
    {
        try {
            is = socket.getInputStream();
            os = socket.getOutputStream();
            process();
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
