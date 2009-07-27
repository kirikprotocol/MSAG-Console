package com.eyeline.smsc.profsync;

import org.apache.log4j.Logger;

import java.net.Socket;
import java.util.HashSet;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.io.*;

/**
 * Created by Serge Lugovoy
 * Date: Jul 10, 2009
 * Time: 1:05:32 PM
 */
public class Processor extends Thread {
  static Logger logger = Logger.getLogger(Processor.class);

  Socket sock;
  HashSet registry;
  BufferedReader rd = null;
  PrintWriter pw = null;
  Pattern pattern = null;
  CommandsFile   cmdFile = null;

  public Processor(Socket sock, HashSet registry, Pattern pattern, CommandsFile cmdFile) throws IOException {
    super(sock.getRemoteSocketAddress().toString());
    this.sock = sock;
    this.registry = registry;
    this.pattern = pattern;
    this.cmdFile = cmdFile;

    try {
      rd = new BufferedReader(new InputStreamReader(sock.getInputStream()));
      pw = new PrintWriter(new OutputStreamWriter(sock.getOutputStream()));
    } catch (IOException e) {
      if( rd != null ) {
        try {
          rd.close();
        } catch (IOException e1) {
        }
        rd = null;
      }
      throw e;
    }
  }

  public void run() {
    String line = null;
    try {
      while((line = rd.readLine()) != null) {
        pw.print("OK");
        pw.flush();
        processLine(line);
      }
      logger.info("Connection closed");
    } catch (IOException e) {
      logger.error("Read line failed", e);
    } finally {
      if( rd != null ) {
        try {
          rd.close();
        } catch (IOException e1) {
        }
        rd = null;
      }
      if( pw != null ) {
        pw.close();
        pw = null;
      }
      if( sock != null ) {
        try {
          sock.close();
        } catch (IOException e1) {
        }
        sock = null;
      }
      synchronized (registry) {
        registry.remove(this);
      }
      synchronized (this) {
        notifyAll();
      }
    }
  }

  public void shutdown() {
    synchronized (this) {
      try {
        sock.close();
      } catch (IOException e) {
      }
      sock = null;
      try {
        wait();
      } catch (InterruptedException e) {
        logger.warn("Wait proccessor shutdown interrupted", e);
      }
    }
  }

  void processLine( String line ) {
    // .1.1.79139167634:DC=UCS2;USSDIN7BIT;REPORT=FINAL;TRANSLIT=NO
    // .1.1.79139167634:DC=UCS2;REPORT=FINAL;TRANSLIT=NO
    logger.info("Got line: "+line);
    Matcher matcher = pattern.matcher(line);
    if( matcher.matches() ) {
      try {
//        logger.info("Matched "+matcher.groupCount()+" groups");
        String address = getMatched(line, matcher, 1);
        String dcs = getMatched(line, matcher, 2);
        String ussd7bit = getMatched(line, matcher, 3);
        String report = getMatched(line, matcher, 4);
        String translit = getMatched(line, matcher, 5);
        logger.debug("Address="+address+" dcs="+dcs+" ussd="+ussd7bit+" report="+report+" translit="+translit);
        String command = "alter profile "+address
                  +" report "+report.toLowerCase()
                  +" encoding "+dcs.toLowerCase()+(ussd7bit.length()>0?" ussd7bit":"")
                  +" translit "+(translit.equalsIgnoreCase("no")?"off":"on");
        try {
          cmdFile.addCommand(command);
        } catch (IOException e1) {
          logger.error("Failed to add command to file: "+command, e1);
        }
      } catch (Throwable e) {
        logger.error("Unexpected error occured", e);
      }
    }
  }

  static String getMatched( String src, Matcher matcher, int group ) {
//    logger.debug("group "+group+" start="+matcher.start(group)+" end="+matcher.end(group));
    if( matcher.start(group) < matcher.end(group) ) {
      return src.substring(matcher.start(group),matcher.end(group));
    }
    else {
      return "";
    }
  }

}
