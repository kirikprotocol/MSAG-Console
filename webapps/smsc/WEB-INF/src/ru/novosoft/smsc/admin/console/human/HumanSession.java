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
import java.io.IOException;
import java.util.Iterator;

public class HumanSession extends Session
{
    private final static String CONSOLE_LOGIN     = "Login: ";
    private final static String CONSOLE_PASSWORD  = "Password: ";
    private final static String CONSOLE_AUTH_FAIL = "Authentication failed. Access denied.";
    private final static String CONSOLE_GREATING  = "Welcome to SMSC Remote Console.";
    private final static String CONSOLE_FAREWELL  = "Exited from SMSC Remote Console.";
    private final static String CONSOLE_PROMPT    = "> ";

    private final static char[] WILL_ECHO = {0xff, 0xfb, 0x01};
    private final static char[] WONT_ECHO = {0xff, 0xfc, 0x01};
    private final static char[] DO_ECHO   = {0xff, 0xfd, 0x01};
    private final static char[] DONT_ECHO = {0xff, 0xfe, 0x01};

    private final static char[] DO_TERMINAL_TYPE = {0xff, 0xfd, 0x18};
    private final static char[] DO_SUPRESS_GA = {0xff, 0xfd, 0x03};
    private final static char[] DONT_LINEMODE = {0xff, 0xfe, 0x22};
    private final static char[] DO_LINEMODE = {0xff, 0xfd, 0x22};

    protected void greeting(PrintWriter writer) {
        writer.println("\r\n\r\n"+CONSOLE_GREATING+'\r');
        writer.flush();
    }
    protected void farewell(PrintWriter writer) {
        writer.println("\r\n"+CONSOLE_FAREWELL+'\r');
        writer.flush();
    }
    protected void prompt(PrintWriter writer) {
        writer.print("\r\n"+CONSOLE_PROMPT);
        writer.flush();
    }

    protected boolean authorize(BufferedReader reader, PrintWriter writer)
        throws Exception
    {
        int tries = 0;

        writer.print(DO_LINEMODE); writer.flush();
        writer.print(DONT_LINEMODE); writer.flush();
        writer.print(DO_TERMINAL_TYPE); writer.flush();
        writer.print(WILL_ECHO); writer.flush();
        writer.print(DO_SUPRESS_GA); writer.flush();
        writer.print(DO_ECHO); writer.flush();

        while (!isStopping && !writer.checkError())
        {
            writer.print(CONSOLE_LOGIN); writer.flush();
            String login = readTelnetLine(reader, writer, true);
            login = login.trim();
            if (login.length() == 0) continue;

            writer.print(CONSOLE_PASSWORD); writer.flush();
            String password = readTelnetLine(reader, writer, false);
            if (password == null) continue;
            password = password.trim();

            if (authorizeUser(login, password)) return true;

            if (++tries >= CONSOLE_AUTH_FAIL_TRIES) {
                writer.println("\r\n"+CONSOLE_AUTH_FAIL+"\r\n"); writer.flush();
                sleep(100);
                break;
            } else {
                sleep(tries*CONSOLE_AUTH_FAIL_SLEEP);
                writer.println("\r\n"+CONSOLE_AUTH_FAIL+"\r\n"); writer.flush();
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
