package com.sibinco.soap_balance.client_loading_test_tool;

import org.apache.log4j.Category;

import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: ilyagp
 * Date: 15.11.2007
 * Time: 17:12:30
 */
public class LoadingTestTool
{
    private final static Category logger = Category.getInstance(LoadingTestTool.class);
    private final static Category stats_logger = Category.getInstance("com.sibinco.soap_balance.client_loading_test_tool.stats");
    private static long counter = 0;
    private static long start_time;
    private static final Object counter_monitor = new Object();

    public static void main(String[] args)
    {
        if(args.length != 3)
        {
            System.out.println("Usage: LoadingTestTool <host> <port> <client_quantity>");
            return;
        }
        String host = args[0];
        String port = args[1];
        String clients_quantity_string = args[2];
        int clients_quantity = Integer.parseInt(clients_quantity_string);
        final Client[] clients = new Client[clients_quantity];
        for(int client_index = 0; client_index < clients.length; client_index++)
            clients[client_index] = new Client(host, port);
        start_time = System.currentTimeMillis();
        for(int client_index = 0; client_index < clients.length; client_index++)
        {
            final int inner_client_index = client_index;
            new Thread()
            {
                private long self_counter = 0;
                public void run()
                {
                    while(true)
                    {
                        try
                        {
                            clients[inner_client_index].getBalance("+79139200352");
                            self_counter++;
                            if(self_counter % 1000 == 0)
                            {
                                stats_logger.info("I am " + inner_client_index + " client. I've done " + self_counter + " requests.");
                            }
                            synchronized(counter_monitor)
                            {
                                counter++;
                                if(counter % 1000 == 0)
                                {
                                    stats_logger.info("There is " + counter/((System.currentTimeMillis() - start_time)/1000) + " operations per second");
                                }
                            }
                        } catch(ClientException during_client_interaction)
                        {
                            logger.warn(during_client_interaction);
                        }
                        try
                        {
                            sleep(50);
                        } catch(InterruptedException during_sleep)
                        {
                            logger.warn(during_sleep);
                        }
                    }
                }
            }.start();
        }
    }
}
