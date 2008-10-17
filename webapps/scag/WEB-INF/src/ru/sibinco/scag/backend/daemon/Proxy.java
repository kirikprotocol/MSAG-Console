/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.daemon;

import org.apache.log4j.Logger;

import java.net.Socket;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.protocol.commands.CommandWriter;
import ru.sibinco.scag.backend.protocol.response.ResponseReader;
import ru.sibinco.scag.backend.protocol.response.Response;

/**
 * The <code>Proxy</code> class represents
 * <p><p/>
 * Date: 27.10.2005
 * Time: 13:11:21
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Proxy {

    public static final byte STATUS_CONNECTED = 1;
    public static final byte STATUS_DISCONNECTED = 0;

    protected String host = "localhost";
    protected int port = 1024;
    protected Logger logger = Logger.getLogger(this.getClass());
    private byte status = STATUS_DISCONNECTED;
    private Socket socket = null;
    private OutputStream out;
    private InputStream in;
    private CommandWriter writer;
    private ResponseReader reader;

    public Proxy(String host, int port) {
        this.host = host;
        this.port = port;
        status = STATUS_DISCONNECTED;
    }

    public synchronized Response runCommand(Command command) throws SibincoException {
        logger.debug("Proxy.runCommand() runCommand(@" + command.getClass().getName() + ")");
        logger.debug("Proxy.runCommand() status = " + getStatus() + " (" + (getStatus() != STATUS_CONNECTED ? "disconnected" : "connected") + ")");
        if (getStatus() != STATUS_CONNECTED){
            logger.debug( "Proxy.runCommand() connect()" );
            connect(host, port);
        }
        try {
            logger.debug("Proxy.runCommand() write command: " + command);
            writer.write(command);
            logger.debug("Proxy.runCommand() reading response...");
            return reader.read();
        } catch (IOException e) {
            logger.error( "Proxy.runCommand() IOException need reconnect");
            e.printStackTrace();
            try {
                reconnect();
                logger.debug("Proxy.runCommand() IOException write command: " + command);
                writer.write(command);
                logger.debug("Proxy.runCommand() IOException reading response...");
                return reader.read();
            } catch (IOException e1) {
                disconnect();
                logger.error( "Proxy.runCommand() Couldn't write command or read response");
                throw new SibincoException("Couldn't write command or read response", e1);
            }
        } catch (NullPointerException npe) {throw new SibincoException("CommandWriter or ResponseReader is null");}
    }


    protected void reconnect() throws SibincoException {
        logger.debug("Proxy.reconnect()");
        disconnect();
        connect(host, port);
    }

    private void disconnect() {
        logger.debug("disconnect()");
        status = STATUS_DISCONNECTED;
        if (socket != null) {
            try {
                socket.close();
                socket = null;
            } catch (IOException e) {
                logger.debug("Exception on closeSocket ", e);
            }
        }
    }

    protected void connect(String host, int port) throws SibincoException {
        logger.debug("connect to \"" + host + ':' + port + '"');
        if (status == STATUS_DISCONNECTED) {
            this.host = host;
            this.port = port;

            try {
                socket = new Socket(host, port);
                socket.setSoTimeout(180000);
                out = socket.getOutputStream();
                in = socket.getInputStream();
                writer = new CommandWriter(out);
                reader = new ResponseReader(in);
                status = STATUS_CONNECTED;
            } catch (IOException e) {
                logger.warn("Could't connected to \"" + host + ':' + port + "\", nested: " + e.getMessage());
//                throw new SibincoException("Could't connected to \"" + host + ':' + port + "\", nested: " + e.getMessage());
            }
        } else {
            throw new SibincoException("Already connected");
        }

    }

    public byte getStatus() {
        return status;
    }

    public int getPort() {
        return port;
    }

    public String getHost() {
        return host;
    }


}
