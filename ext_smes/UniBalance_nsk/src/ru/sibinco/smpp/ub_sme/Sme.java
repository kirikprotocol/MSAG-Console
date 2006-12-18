package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.smpp.ub_sme.util.DBConnectionManager;
import ru.sibinco.smpp.ub_sme.InitializationException;
import ru.sibinco.util.threads.ThreadsPool;

import java.io.IOException;
import java.net.URL;
import java.util.Properties;
import java.util.jar.JarInputStream;
import java.util.jar.Manifest;

public class Sme {

  private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(Sme.class);

  public static String version = " SibInfoConsulting UniBalance ";

  static {
    URL codeBase = Sme.class.getProtectionDomain().getCodeSource().getLocation();
    if (codeBase.getPath().endsWith("UniBalance.jar")) {
      JarInputStream jin = null;
      try {
        jin = new JarInputStream(codeBase.openStream());
        Manifest mf = jin.getManifest();
        version = mf.getMainAttributes().getValue("Customer") + version;
        version = version + mf.getMainAttributes().getValue("Version");
      } catch (Exception e) {
        e.printStackTrace();
      } finally {
        if (jin != null)
          try {
            jin.close();
          } catch (IOException e) {
            e.printStackTrace();
          }
      }
    }
  }

  public Sme(String globalConfig) throws InitializationException {

    // load global config
    Properties config;
    try {
      config = new Properties();
      config.load(this.getClass().getClassLoader().getResourceAsStream(globalConfig));
    } catch (IOException e) {
      Logger.error("Exception occured during loading global configuration.", e);
      throw new InitializationException("Exception occured during loading global configuration.", e);
    }

    // init DB connection manager
    try {
      DBConnectionManager.getInstance();
    } catch (InitializationException e) {
      Logger.error("Could not get DB connection manager.", e);
      throw new InitializationException("Could not get DB connection manager.", e);
    }

    // init multiplexor
    Multiplexor multiplexor = new Multiplexor();
    try {
      multiplexor.init(config, "", null);
      Logger.info("Network multiplexor initialized.");
    } catch (SMPPException e) {
      Logger.error("Exception occured during initializing of multiplexor.", e);
      throw new InitializationException("Exception occured during initializing of multiplexor.");
    }

    // Init threads pool
    ThreadsPool threadsPool = new ThreadsPool(config);

    // SME engine
    SmeEngine engine = new SmeEngine(multiplexor, threadsPool);
    engine.init(config);
    multiplexor.setMessageListener(engine);
    multiplexor.setResponseListener(engine);

    // connect multiplexor
    try {
      multiplexor.connect();
    } catch (SMPPException e) {
      Logger.fatal("Exception occured during establishing connection in multiplexor.", e);
      throw new InitializationException("Exception occured during establishing connection in multiplexor.");
    }
  }

  public static void main(String[] args) {
    if (args.length != 1) {
      System.err.println("Use: java ru.sibinco.smpp.ub_sme.Sme <global_config_file>");
    } else {
      try {
        new Sme(args[0]);
      } catch (InitializationException e) {
        System.err.println("Exception occured during runnig Sme: " + e.getMessage() + " See logs for details.");
        e.printStackTrace(System.err);
      }
    }
  }
}
