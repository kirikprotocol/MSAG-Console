package ru.novosoft.smsc.perfmon;

import java.net.Socket;
import java.net.SocketException;
import java.io.ObjectOutputStream;
import java.io.IOException;
import java.io.DataOutputStream;
import java.util.Random;

public class PerfServerRunner extends Thread {
    private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

    Socket sock;
    PerfServer server;

    public PerfServerRunner( Socket sock, PerfServer server ) {
        this.sock = sock;
        this.server = server;
    }

    public void run() {
        DataOutputStream os = null;
        try {
            os = new DataOutputStream( sock.getOutputStream() );
            snapGenerator(os);
        } catch (SocketException e) {
            logger.debug("Client "+sock.getInetAddress().getHostAddress()+" disconnected");
        } catch (IOException e) {
            logger.warn("I/O error occured for "+sock.getInetAddress().getHostAddress(), e);
        } catch (Exception e) {
            logger.error("Unexpected error occured for "+sock.getInetAddress().getHostAddress(), e);
        } finally {
            if( os != null ) try { os.close();} catch (Exception ee){};
            if( sock != null ) try { sock.close();} catch (Exception ee){};
        }
        server.removeRunner(this);
        synchronized( shutSemaphore ) {
            shutSemaphore.notifyAll();
        }
    }

    void fillDebugSnap( PerfSnap snap ) {
        snap.last[1] = 120; snap.last[2] = 5; snap.last[3] = 12;
        snap.total[1] = 7825649; snap.total[2] = 9202; snap.total[3] = 189723;
        snap.uptime = 3600*24*3+3600*7+60*23+46;
        snap.sctime = 1038483396;
        snap.avg[1] = snap.total[1]/snap.uptime; snap.avg[2] = snap.total[2]/snap.uptime; snap.avg[3] = snap.total[3]/snap.uptime;
    }

    Object shutSemaphore = new Object();
    boolean isStopping = false;

    protected void snapGenerator(DataOutputStream os)
    throws IOException
    {
        int scale = 160;
        PerfSnap snap = new PerfSnap();
        logger.debug("Prepare snap for first time");
        fillDebugSnap(snap );
        int difsuccsess = 50;
        int diferror = 10;
        int difresch = 10;
        Random rand = new Random( System.currentTimeMillis() );
        long lastsec = System.currentTimeMillis()/1000;
        while(!isStopping) {
          long t = System.currentTimeMillis()/1000;
          if( t <= lastsec ) {
            try {
              Thread.currentThread().sleep( 50 );
            } catch (InterruptedException ex) {
            }
            continue;
          }
          lastsec = t;
          snap.uptime++;
          snap.sctime++;
          int rnd = rand.nextInt(); if( rnd < 0 ) rnd *= -1;
          int ns = (rnd%(scale/4))+(scale/2);
          rnd = rand.nextInt(); if( rnd < 0 ) rnd *= -1;
          int ne = rnd%diferror;
          rnd = rand.nextInt(); if( rnd < 0 ) rnd *= -1;
          int nr = rnd%difresch;//+10;
          snap.last[1] = ns;
          snap.last[2] = ne;
          snap.last[3] = nr;
          snap.total[1] += ns;
          snap.total[2] += ne;
          snap.total[3] += nr;
          snap.avg[1] = snap.total[1]/snap.uptime; snap.avg[2] = snap.total[2]/snap.uptime; snap.avg[3] = snap.total[3]/snap.uptime;
          logger.debug( "Writing snap: "+snap.uptime+" "+snap.sctime);
          snap.write( os );
          os.flush();
        }
    }

    public void shutdown() {
        synchronized( shutSemaphore ) {
            isStopping = true;
            try {
               shutSemaphore.wait();
            } catch (InterruptedException e) {
            }
        }
    }
}
