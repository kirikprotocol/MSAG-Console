/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon;

import ru.sibinco.lib.backend.util.SnapBufferReader;
import ru.sibinco.scag.scmon.snap.SmppSnap;
import ru.sibinco.scag.scmon.snap.HttpSnap;
import ru.sibinco.scag.scmon.snap.MmsSnap;

import java.net.Socket;
import java.net.SocketException;
import java.io.IOException;
import java.io.DataOutputStream;
import java.io.InputStream;

public class ScServerRunner extends Thread {
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

    Socket sock;
    Socket smscSock;
    ScServer server;
    SnapBufferReader inbuf = new SnapBufferReader();

    public ScServerRunner(Socket sock, ScServer server) throws IOException {
        this.sock = sock;
        this.server = server;
        logger.debug("Connecting to scmon data socket " + server.getSmscHost() + ":" + server.getTopPort());
        smscSock = new Socket(server.getSmscHost(), server.getTopPort());
        smscSock.setSoLinger(true, 5);
        logger.debug("Connected to performance data socket " + server.getSmscHost() + ":" + server.getTopPort());
    }

    public void run() {
        DataOutputStream os = null;
        InputStream is = null;
        try {
            os = new DataOutputStream(sock.getOutputStream());
            is = smscSock.getInputStream();
            ScSnap snap = new ScSnap();
            while (!isStopping) {
                readSnap(is, snap);
                snap.write(os);
                os.flush();
            }
        } catch (SocketException e) {
            logger.debug("ScMon: Client " + sock.getInetAddress().getHostAddress() + " disconnected");
        } catch (IOException e) {
            logger.warn("I/O error occured for " + sock.getInetAddress().getHostAddress(), e);
        } catch (Exception e) {
            logger.error("Unexpected error occured for " + sock.getInetAddress().getHostAddress(), e);
        } finally {
            if (is != null)
                try {
                    is.close();
                } catch (Exception ee) {
                }
            ;
            if (os != null)
                try {
                    os.close();
                } catch (Exception ee) {
                }
            ;
            if (sock != null)
                try {
                    sock.close();
                } catch (Exception ee) {
                }
            ;
            if (smscSock != null)
                try {
                    smscSock.close();
                } catch (Exception ee) {
                }
            ;
        }
        server.removeRunner(this);
        synchronized (shutSemaphore) {
            shutSemaphore.notifyAll();
        }
    }

    protected void readSnap(InputStream inputStream, ScSnap snap) throws IOException {

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

        private void printDebug(ScSnap snap) {
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

