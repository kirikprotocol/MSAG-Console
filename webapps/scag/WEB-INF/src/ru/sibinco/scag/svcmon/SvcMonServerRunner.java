/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon;

import org.apache.log4j.Category;

import java.net.Socket;
import java.net.SocketException;
import java.io.IOException;
import java.io.DataOutputStream;
import java.io.InputStream;

import ru.sibinco.lib.backend.util.SnapBufferReader;
import ru.sibinco.scag.svcmon.snap.SmppSnap;
import ru.sibinco.scag.svcmon.snap.HttpSnap;
import ru.sibinco.scag.svcmon.snap.MmsSnap;

/**
 * The <code>SvcMonServerRunner</code> class represents
 * <p><p/>
 * Date: 07.12.2005
 * Time: 15:09:46
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SvcMonServerRunner extends Thread {

    private Category logger = Category.getInstance(SvcMonServerRunner.class);

    Socket socket;
    Socket svcSocket;
    SvcMonServer server;
    SnapBufferReader inbuf = new SnapBufferReader();

    public SvcMonServerRunner(Socket socket, SvcMonServer server) throws IOException {
        this.socket = socket;
        this.server = server;
        logger.debug("Connecting to svcmon data socket " + server.getSvcHost() + ":" + server.getSvcPort());
        svcSocket = new Socket(server.getSvcHost(), server.getSvcPort());
        svcSocket.setSoLinger(true, 5);
        logger.debug("Connecting to perfomance data socket " + server.getSvcHost() + ":" + server.getSvcPort());
    }

    public void run() {
        DataOutputStream os = null;
        InputStream is = null;
        try {
            os = new DataOutputStream(socket.getOutputStream());
            is = svcSocket.getInputStream();
            SvcSnap snap = new SvcSnap();

            while (!isStopping) {
                readSnap(is, snap);
                snap.write(os);
                os.flush();
            }
        } catch (SocketException e) {
            e.printStackTrace();
            logger.debug("SvcMon: Client " + socket.getInetAddress().getHostAddress() + " disconnected");

        } catch (IOException e) {
            e.printStackTrace();
            logger.warn("I/O error occured for " + socket.getInetAddress().getHostAddress(), e);
        } catch (Exception e) {
            e.printStackTrace();
            logger.error("Unexpected error occured for " + socket.getInetAddress().getHostAddress(), e);
        } finally {
            if (is != null)
                try {
                    is.close();
                } catch (IOException e) {

                }
            if (os != null)
                try {
                    os.close();
                } catch (IOException e) {

                }
            if (socket != null)
                try {
                    socket.close();
                } catch (IOException e) {

                }
            if (svcSocket != null)
                try {
                    svcSocket.close();
                } catch (IOException e) {

                }
        }
        server.removeRunner(this);
        synchronized (shutSemaphore) {
            shutSemaphore.notifyAll();
        }
    }

    protected void readSnap(InputStream inputStream, SvcSnap snap) throws IOException {

        int len = inbuf.readNetworkInt(inputStream);
        inbuf.fill(inputStream, len);
        int queueSize = inbuf.readNetworkInt();
        snap.init(inbuf);
        //printDebug(snap);
    }


    Object shutSemaphore = new Object();
    boolean isStopping = false;

    public void shutdown() {
        synchronized (shutSemaphore) {
            isStopping = true;
            try {
                shutSemaphore.wait();
            } catch (InterruptedException e) {

            }
        }
    }

    private void printDebug(SvcSnap snap) {
        if (logger.isDebugEnabled()) {
            logger.debug("got smpp snap: " + snap.smppCount);
            for (int i = 0; i < snap.smppCount; i++) {
                StringBuffer sb = new StringBuffer(128);
                for (int j = 0; j < SmppSnap.COUNTERS; j++) {
                    sb.append(snap.smppSnaps[i].smppSpeed[j]).append('/').append(snap.smppSnaps[i].smppAvgSpeed[j]).append(' ');
                }
                logger.debug("   smpp: " + snap.smppSnaps[i].smppId + " " + sb.toString());
            }
            logger.debug("got http snap: " + snap.httpCount);
            for (int i = 0; i < snap.httpCount; i++) {
                StringBuffer sb = new StringBuffer(128);
                for (int j = 0; j < HttpSnap.COUNTERS; j++) {
                    sb.append(snap.httpSnaps[i].httpSpeed[j]).append('/').append(snap.httpSnaps[i].httpAvgSpeed[j]).append(' ');
                }
                logger.debug("   http: " + snap.httpSnaps[i].httpId + " " + sb.toString());
            }
            logger.debug("got mms snap: " + snap.mmsCount);
            for (int i = 0; i < snap.mmsCount; i++) {
                StringBuffer sb = new StringBuffer(128);
                for (int j = 0; j < MmsSnap.COUNTERS; j++) {
                    sb.append(snap.mmsSnaps[i].mmsSpeed[j]).append('/').append(snap.mmsSnaps[i].mmsAvgSpeed[j]).append(' ');
                }
                logger.debug("   mms: " + snap.mmsSnaps[i].mmsId + " " + sb.toString());
            }
        }
    }

}

