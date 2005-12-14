/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 3:28:33 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.script;

import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.console.Gate;
import ru.novosoft.smsc.admin.console.Session;

import java.io.IOException;
import java.net.Socket;

public class ScriptGate extends Gate
{
    public ScriptGate(Console owner, int port) throws IOException {
        super(owner, port);
    }

    protected Session newSession(Console owner, Socket socket) {
        return new ScriptSession(owner, socket);
    }
}

