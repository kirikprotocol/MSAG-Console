/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 15, 2002
 * Time: 2:19:31 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console;

import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.util.config.Config;

import ru.novosoft.smsc.admin.console.commands.*;

import java.net.Socket;
import java.net.ServerSocket;
import java.net.SocketException;
import java.util.ArrayList;
import java.io.IOException;

public class Console extends Thread
{
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

    private Smsc smsc = null;
    private int port = 12401;
    private ServerSocket serverSocket = null;

    private ArrayList sessions = new ArrayList();
    private CommandGroup commands = null;

    public Console(SMSCAppContext context)
        throws IOException, Config.ParamNotFoundException, Config.WrongParamTypeException
    {
        if (context != null) {
            this.smsc = context.getSmsc();
            //port = context.getConfig().getInt("console.port");
            serverSocket = new ServerSocket(port);
            initCommands();
        }
    }

    private void initCommands()
    {
        commands = new CommandGroup();

        CommandGroup alias = new CommandGroup();
        alias.register("add", new AliasAddCommand());
        alias.register("delete", new AliasDeleteCommand());
        alias.register("edit", new AliasEditCommand());
        alias.register("view", new AliasViewCommand());
        commands.register("alias", alias);

        CommandGroup profile = new CommandGroup();
        profile.register("add", new ProfileAddCommand());
        profile.register("delete", new ProfileDeleteCommand());
        profile.register("edit", new ProfileEditCommand());
        commands.register("profile", profile);

        CommandGroup subject = new CommandGroup();
        subject.register("add", new SubjectAddCommand());
        subject.register("delete", new SubjectDeleteCommand());
        subject.register("view", new SubjectViewCommand());
        subject.register("edit", new SubjectEditCommand());
        commands.register("subject", subject);

        CommandGroup route = new CommandGroup();
        route.register("add", new RouteAddCommand());
        route.register("delete", new RouteDeleteCommand());
        route.register("edit", new RouteEditCommand());
        commands.register("route", route);

        ApplyCommand apply = new ApplyCommand(smsc);
        commands.register("apply", apply);
    }

    public String processCommand(String command) {
        return commands.process(command);
    }

    boolean isStopping = false;
    Object closeSemaphore = new Object();

    public void run()
    {
        logger.debug("CONSOLE: Starting listening on port "+port);
        while(!isStopping)
        {
            Socket socket = null;
            try {
                socket = serverSocket.accept();
                socket.setSoLinger(true, 0);
                logger.debug("Client "+socket.getInetAddress().getHostAddress()+" connected");
                Session session = new Session(this, socket);
                addSession(session);
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
        logger.debug("CONSOLE: Closed");
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
        while(sessions.size() > 0) {
            ((Session)sessions.get(0)).close();
        }
        //sessions.clear();
    }

    public Smsc getSmsc() {
        return smsc;
    }

    public void addSession(Session session) {
        synchronized (sessions) {
            sessions.add(session);
        }
    }

    public void removeSession(Session session) {
        synchronized (sessions) {
            sessions.remove(session);
        }
    }
}
