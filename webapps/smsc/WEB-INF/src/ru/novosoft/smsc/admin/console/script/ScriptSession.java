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

public class ScriptSession extends Session
{
    public ScriptSession(Console owner, Socket socket) {
        super(owner, socket);
    }

    protected void display(PrintWriter writer, CommandContext ctx) {
        // todo implement it
        writer.print(ctx.getMessage()+'\r');
    }
}
