package ru.sibinco.smpp.ub_sme;

import com.lorissoft.advertising.syncclient.AdvertisingClientImpl;
import com.lorissoft.advertising.syncclient.IAdvertisingClient;
import com.lorissoft.advertising.syncclient.IAdvertisingResponseHandler;

import java.util.Properties;

/**
 * User: pasha
 * Date: 13.12.2007
 * Time: 13:23:29
 */
public class BannerManager {

    private String bannerEngineServiceName = "UniBalance";
    private int bannerEngineClientID = 1;
    private int bannerEngineTransportType = 1;
    private int bannerEngineCharSet = 1;

    private boolean bannerEngineClientEnabled = false;
    private IAdvertisingClient bannerEngineClient;
    private String bannerEngineClientHost = "localhost";
    private String bannerEngineClientPort = "8555";
    private String bannerEngineClientTimeout = "1000";
    private String bannerEngineClientReconnectTimeout = "1000";
    private int bannerEngineTransactionId;
    private final Object bannerEngineTransactionIdSyncMonitor = new Object();

    public BannerManager(Properties config) {
        bannerEngineClientEnabled = Boolean.valueOf(config.getProperty("banner.engine.client.enabled", Boolean.toString(bannerEngineClientEnabled))).booleanValue();
        if (bannerEngineClientEnabled) {
            bannerEngineServiceName = config.getProperty("banner.engine.service.name", bannerEngineServiceName);
            if (bannerEngineServiceName.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.service.name\" is missed");
            }
            try {
                bannerEngineClientID = Integer.parseInt(config.getProperty("banner.engine.client.id", Integer.toString(bannerEngineClientID)));
            } catch (NumberFormatException e) {
                throw new InitializationException("Invalid value for config parameter \"banner.engine.client.id\": " + config.getProperty("banner.engine.client.id"));
            }
            try {
                bannerEngineTransportType = Integer.parseInt(config.getProperty("banner.engine.transport.type", Integer.toString(bannerEngineTransportType)));
            } catch (NumberFormatException e) {
                throw new InitializationException("Invalid value for config parameter \"banner.engine.transport.type\": " + config.getProperty("banner.engine.transport.type"));
            }
            try {
                bannerEngineCharSet = Integer.parseInt(config.getProperty("banner.engine.charset", Integer.toString(bannerEngineCharSet)));
            } catch (NumberFormatException e) {
                throw new InitializationException("Invalid value for config parameter \"banner.engine.charset\": " + config.getProperty("banner.engine.charset"));
            }

            bannerEngineClientHost = config.getProperty("banner.engine.client.host", bannerEngineClientHost);
            if (bannerEngineClientHost.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.client.host\" is missed");
            }
            bannerEngineClientPort = config.getProperty("banner.engine.client.port", bannerEngineClientPort);
            if (bannerEngineClientPort.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.client.port\" is missed");
            }
            bannerEngineClientTimeout = config.getProperty("banner.engine.client.timeout", bannerEngineClientTimeout);
            if (bannerEngineClientTimeout.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.client.timeout\" is missed");
            }
            bannerEngineClientReconnectTimeout = config.getProperty("banner.engine.client.reconnect.timeout", bannerEngineClientReconnectTimeout);
            if (bannerEngineClientReconnectTimeout.length() == 0) {
                throw new InitializationException("Mandatory config parameter \"banner.engine.client.reconnect.timeout\" is missed");
            }
            Properties bannerEngineClientConfig = new Properties();
            bannerEngineClientConfig.put("ip", bannerEngineClientHost);
            bannerEngineClientConfig.put("port", bannerEngineClientPort);
            bannerEngineClientConfig.put("CLIENTTIMEOUT", bannerEngineClientTimeout);
            bannerEngineClientConfig.put("WATCHDOGSLEEP", bannerEngineClientReconnectTimeout);

            bannerEngineClient = new AdvertisingClientImpl();
            bannerEngineClient.init(bannerEngineClientConfig);
        }
    }

    public void RequestBanner(String abonent, IAdvertisingResponseHandler handler) {
        byte[] banner = null;

        if (abonent.startsWith("+")) {
            abonent = ".1.1." + abonent.substring(1);
        } else if (!abonent.startsWith(".")) {
            abonent = ".1.1." + abonent;
        }
        int transactionId;
        synchronized (bannerEngineTransactionIdSyncMonitor) {
            transactionId = bannerEngineTransactionId++;
        }
        bannerEngineClient.requestLikelyBanner(abonent.getBytes(), abonent.getBytes().length, bannerEngineServiceName.getBytes(), bannerEngineTransportType, 140, bannerEngineCharSet, bannerEngineClientID, transactionId, handler);
    }


}
