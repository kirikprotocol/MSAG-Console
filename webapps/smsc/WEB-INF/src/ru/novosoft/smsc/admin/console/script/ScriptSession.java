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
    private final static String CONSOLE_CONNECT   = "Connected. Login:";
    private final static String CONSOLE_LOGINOK   = "Login accepted. Password:";
    private final static String CONSOLE_AUTH_FAIL = "Authentication failed. Access denied.";
    private final static String CONSOLE_AUTH_OK   = "Logged in. Access granted.";

    public ScriptSession(Console owner, Socket socket) {
        super(owner, socket);
    }

    private void showMessage(PrintWriter writer, int code, String message)
    {
        writer.print(((code == CommandContext.CMD_LIST ||
                       code == CommandContext.CMD_OK) ? "+ ":"- ")+code);
        if (message != null) writer.print(' '+message);
        writer.println('\r');
        writer.flush();
    }
    protected boolean authorize(BufferedReader reader, PrintWriter writer)
            throws Exception
    {
        int tries = 0;
        while (!isStopping)
        {
            showMessage(writer, CommandContext.CMD_OK, CONSOLE_CONNECT);
            String login = reader.readLine();
            if (login == null || login.length() == 0) continue;
            showMessage(writer, CommandContext.CMD_OK, CONSOLE_LOGINOK);
            String password = reader.readLine();
            if (password == null) continue;

            if (authorizeUser(login, password)) {
                showMessage(writer, CommandContext.CMD_OK, CONSOLE_AUTH_OK);
                return true;
            } else {
                showMessage(writer, CommandContext.CMD_AUTH_ERROR, CONSOLE_AUTH_FAIL);
                if (++tries >= CONSOLE_AUTH_FAIL_TRIES) {
                    sleep(100);
                    break;
                } else sleep(tries*CONSOLE_AUTH_FAIL_SLEEP);
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
        showMessage(writer, status, message);
    }

}
