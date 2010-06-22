package com.eyeline.smsc.profsync;

import org.apache.log4j.Category;

import java.util.Properties;
import java.util.HashMap;
import java.util.HashSet;
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
  static Pattern pattern = null;
  static ConsoleInteraction console = null;
  static CommandsFile   cmdFile = null;
  static HashSet registry = new HashSet(10);

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
        Socket sock = ssock.accept();
        sock.setSoLinger(true,5);
        logger.info("Socket accepted "+sock.getInetAddress().getHostAddress());
        try {
          Processor proc = new Processor(sock, registry, pattern, cmdFile);
          registry.add( proc );
          proc.start();
        } catch (IOException e) {
          logger.error("Could not create processor", e);
        }
      } catch (Throwable e) {
        logger.error("Exception in accept", e);
        continue;
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

  static void shutdown() {
    synchronized(semaphore) {
      logger.info("shutdown requested");
      isRunning = false;
      try {
        if( ssock != null ) ssock.close();
      } catch (IOException e) {
      }
      while( !registry.isEmpty() ) {
        Processor proc = null;
        synchronized (registry) {
          proc = (Processor) registry.iterator().next();
        }
        proc.shutdown();
      }
      console.shutdown();
      logger.info("shutdown notifies sent");
      try {
        semaphore.wait();
      } catch (InterruptedException e) {
      }
      logger.info("Shutdown complete");
    }
  }

}
