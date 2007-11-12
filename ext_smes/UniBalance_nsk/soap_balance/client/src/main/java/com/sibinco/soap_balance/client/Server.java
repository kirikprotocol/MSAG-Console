package com.eyelinecom.whoisd.phones.tcp;

import com.eyelinecom.whoisd.sptp.SPTPServer;
import com.eyelinecom.whoisd.sptp.SPTPServerThread;
import com.eyelinecom.whoisd.util.InitializationException;
import com.eyelinecom.whoisd.util.DBConnectionManager;
import com.eyelinecom.whoisd.phones.storage.StorageManager;

import java.net.Socket;
import java.util.Properties;
import java.io.IOException;

import org.apache.log4j.Category;
import ru.sibinco.util.threads.ThreadsPool;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class Server extends SPTPServer {
  private final static Category logger = Category.getInstance(Server.class);
  private StorageManager storageManager;

  public Server(String configFileName) throws InitializationException {
    Properties config;

    // Load config
    try {
      config = new Properties();
      config.load(this.getClass().getClassLoader().getResourceAsStream(configFileName));
    } catch (IOException e) {
      logger.error("Exception occured during loading global configuration.", e);
      throw new InitializationException("Exception occured during loading configuration.", e);
    }

    /*
    ShutdownManager shutdownManager = new ShutdownManager();
    Runtime.getRuntime().addShutdownHook(shutdownManager);
    */

    // Init DB Connection Manager
    try {
      DBConnectionManager.getInstance();
    } catch (InitializationException e) {
      logger.error("Could not get DB connection manager.", e);
      throw new InitializationException("Could not get DB connection manager.", e);
    }

    init(config);
  }

  public void init(Properties config) throws InitializationException {
    super.init(config, "");
    storageManager = new StorageManager();
    storageManager.init(config);
  }

  protected SPTPServerThread createServerThread(SPTPServer server, Socket socket) {
    try {
      return new ServerThread(server, socket, storageManager);
    } catch (IOException e) {
      logger.error("I/O Error: " + e, e);
      return null;
    }
  }

  public static void main(String[] args) {
    if (args.length < 1 || args[0].length() == 0) {
      System.out.println("Can not start without configuration file!");
      logger.fatal("Can not start without configuration file!");
    }

    String configFileName = args[0];
    logger.debug("Configuration file: " + configFileName);
    try {
      (new Server(configFileName)).startService();
    } catch (InitializationException e) {
      logger.fatal("Server initialization error: " + e, e);
    }
  }

}
