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

public class HumanSession extends Session
{
    private final static String CONSOLE_GREATING = "Welcome to SMSC Remote Console.";
    private final static String CONSOLE_FAREWELL = "Exited from SMSC Remote Console.";
    private final static String CONSOLE_PROMPT   = "> ";

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

    public HumanSession(Console owner, Socket socket) {
        super(owner, socket);
    }

    protected void display(PrintWriter writer, CommandContext ctx) {
        writer.print(ctx.getMessage()+'\r');
    }

}
