package com.sibinco.soap_balance.client;

import com.eyelinecom.whoisd.sptp.SPTPServer;
import com.eyelinecom.whoisd.sptp.SPTPServerThread;
import com.sibinco.soap_balance.service.BalanceService;
import org.apache.log4j.Category;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

import java.io.IOException;
import java.net.Socket;
import java.util.Properties;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public abstract class Server extends SPTPServer implements InitializingBean
{
  private final static Category logger = Category.getInstance(Server.class);
  private Properties sptp_properties;

  public Server()
  {
  }

    public void setSptpProperties(Properties sptp_properties)
    {
        this.sptp_properties = sptp_properties;
    }

    public void afterPropertiesSet()
  {
      init(sptp_properties);
  }

  protected SPTPServerThread createServerThread(SPTPServer server, Socket socket) {
    try {
      return new ServerThread(server, socket, getBalanceService());
    } catch (IOException e) {
      logger.error("I/O Error: " + e, e);
      return null;
    }
  }

  protected abstract BalanceService getBalanceService();

  public static void main(String[] args)
  {
      ConfigurableApplicationContext factory = new ClassPathXmlApplicationContext("applicationContext.xml");
      Server server = (Server)factory.getBean("server");
      server.startService();
  }
}
