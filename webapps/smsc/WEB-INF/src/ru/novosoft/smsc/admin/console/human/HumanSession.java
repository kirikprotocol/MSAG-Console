/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 3:31:24 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.human;

import ru.novosoft.smsc.admin.console.Session;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.console.CommandContext;

import java.net.Socket;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.util.Iterator;

public class HumanSession extends Session
{
    private final static int CONSOLE_AUTH_FAIL_SLEEP = 10000;
    private final static int CONSOLE_AUTH_FAIL_TRIES = 3;

    private final static String CONSOLE_LOGIN     = "Login: ";
    private final static String CONSOLE_PASSWORD  = "Password: ";
    private final static String CONSOLE_AUTH_FAIL = "Authentication failed. Access denied.";
    private final static String CONSOLE_GREATING  = "Welcome to SMSC Remote Console.";
    private final static String CONSOLE_FAREWELL  = "Exited from SMSC Remote Console.";
    private final static String CONSOLE_PROMPT    = "> ";

    protected void greeting(PrintWriter writer) {
        writer.println(CONSOLE_GREATING+'\r');
        writer.flush();
    }
    protected void farewell(PrintWriter writer) {
        writer.println(CONSOLE_FAREWELL+'\r');
        writer.flush();
    }
    protected void prompt(PrintWriter writer) {
        writer.print("\r\n"+CONSOLE_PROMPT);
        writer.flush();
    }

    protected boolean authorize(BufferedReader reader, PrintWriter writer)
        throws Exception
    {
        int maxTriesCount = CONSOLE_AUTH_FAIL_TRIES;
        while (!isStopping)
        {
            writer.print(CONSOLE_LOGIN); writer.flush();
            String login = reader.readLine();
            if (login == null || login.length() == 0) continue;
            writer.print(CONSOLE_PASSWORD); writer.flush();
            String password = reader.readLine();
            if (password == null) continue;
            if (authorizeUser(login, password)) return true;

            writer.println("\r\n"+CONSOLE_AUTH_FAIL+"\r\n"); writer.flush();
            if (--maxTriesCount <= 0) {
                sleep(CONSOLE_AUTH_FAIL_SLEEP); break;
            }
        }
        return false;
    }

    public HumanSession(Console owner, Socket socket) {
        super(owner, socket);
    }

    protected void display(PrintWriter writer, CommandContext ctx)
    {
        int status = ctx.getStatus();
        String message = ctx.getMessage();

        if (status == CommandContext.CMD_LIST) {
            if (message != null) writer.println(ctx.getMessage()+":\r");
            Iterator i = ctx.getResults().iterator();
            while (i.hasNext()) {
                Object obj = i.next();
                if (obj != null) {
                    String str = (obj instanceof String) ? (String)obj:obj.toString();
                    writer.println(' '+str+'\r');
                }
            }
        } else {
            writer.print((status == CommandContext.CMD_OK) ? "Ok. ":"Failed: ");
            writer.println(message+'\r');
        }
    }

}
