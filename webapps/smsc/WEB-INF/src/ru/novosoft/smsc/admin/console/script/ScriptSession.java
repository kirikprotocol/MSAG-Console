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
import java.io.IOException;
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

    private void showMessage(int code, String message) throws IOException
    {
        printString(((code == CommandContext.CMD_LIST ||
                      code == CommandContext.CMD_OK) ? "+ ":"- ")+code);
        if (message != null) printString(" "+message);
        printString("\r\n");
    }

    protected String readTelnetLine(boolean echo)
        throws IOException
    {
        return super.readTelnetLine(false);
    }

    protected boolean authorize() throws Exception
    {
        int tries = 0;
        while (!isStopping)
        {
            showMessage(CommandContext.CMD_OK, CONSOLE_CONNECT);
            String login = readTelnetLine(false);
            if (login == null || login.length() == 0) continue;
            showMessage(CommandContext.CMD_OK, CONSOLE_LOGINOK);
            String password = readTelnetLine(false);
            if (password == null) continue;

            if (authorizeUser(login, password)) {
                showMessage(CommandContext.CMD_OK, CONSOLE_AUTH_OK);
                return true;
            } else {
                showMessage(CommandContext.CMD_AUTH_ERROR, CONSOLE_AUTH_FAIL);
                if (++tries >= CONSOLE_AUTH_FAIL_TRIES) {
                    sleep(100);
                    break;
                } else sleep(tries*CONSOLE_AUTH_FAIL_SLEEP);
            }
        }
        return false;
    }

    protected void display(CommandContext ctx)
        throws IOException
    {
        int status = ctx.getStatus();
        String message = ctx.getMessage();
        if (status == CommandContext.CMD_LIST) {
            Iterator i = ctx.getResults().iterator();
            while (i.hasNext()) {
                Object obj = i.next();
                if (obj != null) {
                    String str = (obj instanceof String) ? (String)obj:obj.toString();
                    printlnString(" "+str);
                }
            }
        }
        showMessage(status, message);
    }

}
