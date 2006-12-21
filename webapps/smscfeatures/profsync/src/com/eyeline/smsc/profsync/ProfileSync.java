package com.eyeline.smsc.profsync;

import org.apache.log4j.Category;

import java.util.Properties;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * Created by Serge Lugovoy
 * Date: Dec 5, 2006
 * Time: 3:30:18 PM
 */
public class ProfileSync {
  static Category logger = Category.getInstance(ProfileSync.class);
  static Object   semaphore = new Object();
  static boolean  isRunning = true;

  static Properties config = new Properties();
  static ServerSocket ssock = null;
  static Socket sock = null;
  static BufferedReader rd = null;
  static PrintWriter pw = null;
  static Pattern pattern = null;
  static ConsoleInteraction console = null;
  static CommandsFile   cmdFile = null;

  public static void main( String args[] ) {
    if( args.length < 1 ) {
      System.out.println("Usage: ProfileSync config.properties");
      return;
    }
    logger.info("Starting profile sync");
    try {
      config.load(new FileInputStream(args[0]));
    } catch (IOException e) {
      logger.error("Could not load config", e);
      return;
    }
    int serverPort = 0;
    try {
      serverPort = Integer.valueOf(config.getProperty("listen.port")).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid port numbner in listen.port", e);
    }
    try {
      cmdFile = new CommandsFile(config);
    } catch (IOException e) {
      logger.error("Could not initialize backup file", e);
      return;
    }
    try {
      ssock = new ServerSocket(serverPort);
    } catch (IOException e) {
      logger.error("Could not create server socket", e);
      return;
    }
    String s = "(?ix)(\\.\\d+\\.\\d+\\.\\d+):DC=(.*?);(USSDIN7BIT;)?REPORT=(.*?);TRANSLIT=(.*?)";
    pattern = Pattern.compile(s);
    try {
      console = new ConsoleInteraction(config, cmdFile);
    } catch (IOException e) {
      logger.error("Could not create console connection", e);
      try {
        ssock.close();
      } catch (IOException e1) {
      }
      return;
    }
    Runtime.getRuntime().addShutdownHook(new ShutdownHook());
    logger.info("Listening on port "+serverPort);
    while(isRunning) {
      try {
        sock = ssock.accept();
        sock.setSoLinger(true,5);
      } catch (IOException e) {
        logger.error("Exception in accept", e);
        continue;
      }
      logger.info("Socket accepted "+sock.getInetAddress().getHostAddress());
      try {
        rd = new BufferedReader(new InputStreamReader(sock.getInputStream()));
      } catch (IOException e) {
        logger.error("Could not get input stream", e);
        try {
          sock.close();
        } catch (IOException e1) {
        }
        continue;
      }
      try {
        pw = new PrintWriter(new OutputStreamWriter(sock.getOutputStream()));
      } catch (IOException e) {
        logger.error("Could not get input stream", e);
        try {
          rd.close();
        } catch (IOException e1) {
        }
        try {
          sock.close();
        } catch (IOException e1) {
        }
        continue;
      }
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
        try {
          rd.close();
        } catch (IOException e1) {
        }
        try {
          sock.close();
        } catch (IOException e1) {
        }
      }
    }
    try {
      ssock.close();
    } catch (IOException e) {
      logger.error("Could not close server socket", e);
    }
    logger.info("main terminated");
    synchronized(semaphore){
      semaphore.notifyAll();
    }
  }

  static void processLine( String line ) {
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

  static void shutdown() {
    synchronized(semaphore) {
      logger.info("shutdown requested");
      isRunning = false;
      try {
        if( sock != null ) sock.close();
      } catch (IOException e) {
      }
      try {
        if(rd != null ) rd.close();
      } catch (IOException e) {
      }
      try {
        if( ssock != null ) ssock.close();
      } catch (IOException e) {
      }
      console.interrupt();
      logger.info("shutdown notifies sent");
      try {
        semaphore.wait();
      } catch (InterruptedException e) {
      }
      logger.info("Shutdown complete");
    }
  }
}
