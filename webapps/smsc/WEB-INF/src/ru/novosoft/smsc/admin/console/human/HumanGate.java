/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 19, 2002
 * Time: 3:26:23 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.human;

import ru.novosoft.smsc.admin.console.Gate;
import ru.novosoft.smsc.admin.console.Console;
import ru.novosoft.smsc.admin.console.Session;

import java.net.Socket;
import java.io.IOException;

public class HumanGate extends Gate
{
    public HumanGate(Console owner, int port) throws IOException {
        super(owner, port);
    }

    protected Session newSession(Console owner, Socket socket) {
        return new HumanSession(owner, socket);
    }
}
