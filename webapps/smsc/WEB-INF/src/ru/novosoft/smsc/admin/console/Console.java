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
import ru.novosoft.smsc.admin.console.commands.exceptions.*;

import java.net.Socket;
import java.net.ServerSocket;
import java.net.SocketException;
import java.util.ArrayList;
import java.io.IOException;

public class Console extends Thread
{
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

    private final static String COMMAND_APPLY = "apply";
    private final static String COMMAND_ALIAS = "alias";
    private final static String COMMAND_ROUTE = "route";
    private final static String COMMAND_PROFILE = "profile";
    private final static String COMMAND_SUBJECT = "subject";

    private final static String ACTION_ADD = "add";
    private final static String ACTION_VIEW = "view";
    private final static String ACTION_EDIT = "edit";
    private final static String ACTION_DELETE = "delete";

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

        try {
            CommandGroup alias = new CommandGroup();
            alias.register(ACTION_ADD, new AliasAddCommand(smsc));
            alias.register(ACTION_DELETE, new AliasDeleteCommand(smsc));
            alias.register(ACTION_EDIT, new AliasEditCommand(smsc));
            alias.register(ACTION_VIEW, new AliasViewCommand(smsc));
            commands.register(COMMAND_ALIAS, alias);

            CommandGroup profile = new CommandGroup();
            profile.register(ACTION_ADD, new ProfileAddCommand(smsc));
            profile.register(ACTION_DELETE, new ProfileDeleteCommand(smsc));
            profile.register(ACTION_EDIT, new ProfileEditCommand(smsc));
            commands.register(COMMAND_PROFILE, profile);

            CommandGroup subject = new CommandGroup();
            subject.register(ACTION_ADD, new SubjectAddCommand(smsc));
            subject.register(ACTION_DELETE, new SubjectDeleteCommand(smsc));
            subject.register(ACTION_VIEW, new SubjectViewCommand(smsc));
            subject.register(ACTION_EDIT, new SubjectEditCommand(smsc));
            commands.register(COMMAND_SUBJECT, subject);

            CommandGroup route = new CommandGroup();
            route.register(ACTION_ADD, new RouteAddCommand(smsc));
            route.register(ACTION_DELETE, new RouteDeleteCommand(smsc));
            route.register(ACTION_EDIT, new RouteEditCommand(smsc));
            commands.register(COMMAND_ROUTE, route);

            ApplyCommand apply = new ApplyCommand(smsc);
            commands.register(COMMAND_APPLY, apply);
        }
        catch (CommandRegisterException e) {
            logger.error("Commands registration failed", e);
        }
    }

    public String processCommand(String command)
        throws CommandProcessException
    {
        return commands.process(command);
    }

    boolean isStopping = false;
    Object closeSemaphore = new Object();

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
        while(sessions.size() > 0) {
            ((Session)sessions.get(0)).close();
        }
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
