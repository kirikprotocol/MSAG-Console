package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.admin.UpdateConfigServer;
import mobi.eyeline.dcpgw.dcp.DcpConnection;
import mobi.eyeline.dcpgw.dcp.DcpConnectionImpl;
import mobi.eyeline.dcpgw.dcp.DeliveryChangeListenerImpl;
import mobi.eyeline.dcpgw.exeptions.CouldNotLoadJournalException;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.journal.Journal;
import mobi.eyeline.dcpgw.smpp.PDUListenerImpl;
import mobi.eyeline.dcpgw.smpp.Server;
import mobi.eyeline.informer.admin.InitException;
import mobi.eyeline.informer.admin.delivery.changelog.DeliveryChangesDetectorImpl;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.IOException;
import java.util.*;

public class Gateway extends Thread implements PDUListener {

    private static Logger log = Logger.getLogger(Gateway.class);

    private ProcessingQueue procQueue;

    private DeliveryChangesDetectorImpl deliveryChangesDetector;

    private FileSystem fileSystem;

    private PDUListenerImpl pduListener;

    public static void main(String args[]) {
        String user_dir = System.getProperty("user.dir");
        log.debug("user dir:"+user_dir);
        String config_file = user_dir + File.separator + "conf" + File.separator + "config.properties";
        log.debug("config file: "+config_file);

        try {
            new Gateway(config_file);
        } catch (Exception e) {
            log.error(e);
        }

    }

    public Gateway(String config_file) throws InitializationException, SmppException, IOException, XmlConfigException {
        log.debug("Try to initialize gateway ...");
        Runtime.getRuntime().addShutdownHook(this);

        Config config = Config.getInstance();
        config.init(config_file);
        Properties properties = config.getProperties();

        Journal.getInstance().init(config.getJournalDir(), config.getMaxJournalSize(), config.getMaxSubmitDateJournalSize(),
                config.getCleanJournalTimeout());
        try {
            Journal.getInstance().load();
        } catch (CouldNotLoadJournalException e) {
            throw new InitializationException(e);
        }

        pduListener = new PDUListenerImpl();
        procQueue = new ProcessingQueue(properties, pduListener, null);

        Server.getInstance().init(properties, this);

        fileSystem = FileSystem.getFSForSingleInst();
        try {
            deliveryChangesDetector = new DeliveryChangesDetectorImpl(config.getFinalLogDir(), fileSystem);

            DeliveryChangeListenerImpl deliveryChangeListener = new DeliveryChangeListenerImpl();
            deliveryChangesDetector.addListener(deliveryChangeListener);

            deliveryChangesDetector.start();
        } catch (InitException e) {
            log.error(e);
            throw new InitializationException(e);
        }

        try {
            new UpdateConfigServer(config.getUpdateServerHost(), config.getUpdateServerPort());
        } catch (IOException e) {
            log.error("Couldn't initialize update server.", e);
            throw new InitializationException(e);
        }

        log.debug("Gateway initialized.");
    }

    public boolean handlePDU(PDU pdu) {
        try {
            procQueue.add(pdu, pdu.isMessage());
        } catch (QueueException e) {
            try {
                Server.getInstance().send(((Request) pdu).getResponse(e.getStatus()), false);
            } catch (SmppException e1) {
                log.error("Could not send resp", e1);
            }
        }
        return true;
    }

    @Override
    public void run() {

        // Stop receiving messages.
        pduListener.setReject(true);

        // Stop adding messages to informer.
        Hashtable<String, DcpConnectionImpl> informer_user_connection_table = Config.getInstance().getDCPConnections();

        for(String informer_user: informer_user_connection_table.keySet()){
            DcpConnectionImpl dcpConnection = informer_user_connection_table.get(informer_user);

        }



        Server.getInstance().shutdown();
        Journal.getInstance().shutdown();
    }

}
