/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 3:33:01 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.script;

import ru.novosoft.smsc.admin.console.Session;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.console.CommandContext;

import java.net.Socket;
import java.io.PrintWriter;
import java.io.BufferedReader;
import java.util.Iterator;

public class ScriptSession extends Session
{
    private final static int CONSOLE_AUTH_FAIL_SLEEP = 10000;

    private final static String CONSOLE_AUTH_FAIL   = "Authentication failed. Access denied.";
    private final static String CONSOLE_AUTH_OK     = "Logged in. Access granted.";

    public ScriptSession(Console owner, Socket socket) {
        super(owner, socket);
    }

    protected boolean authorize(BufferedReader reader, PrintWriter writer)
            throws Exception
    {
        while (!isStopping)
        {
            String login = reader.readLine();
            if (login == null || login.length() == 0) continue;
            String password = reader.readLine();
            if (password == null) continue;

            if (authorizeUser(login, password)) {
                writer.print("+ "+CommandContext.CMD_OK);
                writer.println(' '+CONSOLE_AUTH_OK+'\r');
                writer.flush();
                return true;
            } else {
                writer.print("- "+CommandContext.CMD_AUTH_ERROR);
                writer.println(' '+CONSOLE_AUTH_FAIL+'\r');
                writer.flush();
                sleep(CONSOLE_AUTH_FAIL_SLEEP);
                break;
            }
        }
        return false;
    }

    protected void display(PrintWriter writer, CommandContext ctx)
    {
        int status = ctx.getStatus();
        String message = ctx.getMessage();

        if (status == CommandContext.CMD_LIST) {
            Iterator i = ctx.getResults().iterator();
            while (i.hasNext()) {
                Object obj = i.next();
                if (obj != null) {
                    String str = (obj instanceof String) ? (String)obj:obj.toString();
                    writer.println(' '+str+'\r');
                }
            }
        }
        writer.print(((status == CommandContext.CMD_LIST ||
                       status == CommandContext.CMD_OK) ? "+ ":"- ")+status);
        if (message != null) writer.print(' '+ctx.getMessage());
        writer.println('\r');
        writer.flush();
    }

}
