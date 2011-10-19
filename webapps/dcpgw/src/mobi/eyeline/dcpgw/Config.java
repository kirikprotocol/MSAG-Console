package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.dcp.DcpConnectionImpl;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.dcpgw.smpp.Server;
import mobi.eyeline.dcpgw.utils.Utils;
import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.protogen.DcpClient;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;
import mobi.eyeline.smpp.api.SmppException;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 19.09.11
 * Time: 16:43
 */
public class Config {

    private static Config instance = new Config();

    public static Config getInstance(){
        return instance;
    }

    private static Logger log = Logger.getLogger(Config.class);

    private String deliveries_file;
    private String smpp_endpoints_file;

    private Properties config, config_with_smpp_endpoints;
    private Hashtable<String, String> informer_user_password_table;
    private Hashtable<String, Provider> connection_provider_table;
    private Set<Provider> providers;

    public static final String CONNECTION_PREFIX = "smpp.sme.";

    private String informer_host;

    private int informer_port;

    private int informer_messages_list_capacity;

    private long sending_to_informer_timeout;

    private String update_config_server_host;

    private int update_config_server_port;

    private Hashtable<String, HashSet<Integer>> informer_user_delivery_ids_table;

    private static Hashtable<String, DcpConnectionImpl> informer_user_connection_table;

    private int max_journal_size_mb, max_submit_date_journal_size;

    private File journal_dir;

    private File final_log_dir;

    private int resend_receipts_interval;

    private int send_receipts_speed_default;
    private int delivery_response_timeout;
    private int send_receipt_max_time_default;

    private int journal_clean_timeout;

    private File message_id_rang_file, receipts_rang_file;

    private long initial_message_id_rang;
    private int initial_receipts_sn_rang;
    private int rang = 10000;

    public void init(String config_file) throws IOException, XmlConfigException, InitializationException {
        config = new Properties();
        config.load(new FileInputStream(config_file));
        String user_dir = System.getProperty("user.dir");

        smpp_endpoints_file = Utils.getProperty(config, "users.file", user_dir + File.separator + "conf" + File.separator + "endpoints.xml");
        deliveries_file = Utils.getProperty(config, "deliveries.file", user_dir + File.separator + "conf" + File.separator + "deliveries.xml");

        // Load messages ids range
        message_id_rang_file = new File(user_dir + File.separator + "conf" + File.separator + "message_id_rang");
        if (message_id_rang_file.createNewFile()){
            log.debug("Create new message id file: "+ message_id_rang_file);
            initial_message_id_rang = 0;
        } else {
            BufferedReader br = new BufferedReader(new FileReader(message_id_rang_file));
            String line = br.readLine();
            initial_message_id_rang = Long.parseLong(line);
            br.close();
            log.debug("Load from file initial message id rang: " + initial_message_id_rang);
        }

        PrintWriter pw = new PrintWriter(new FileWriter(message_id_rang_file));
        pw.println(initial_message_id_rang +rang);
        pw.flush();
        pw.close();
        log.debug("Write to file initial message id rang: "+ initial_message_id_rang + rang);

        // Load receipts rang
        receipts_rang_file = new File(user_dir + File.separator + "conf" + File.separator + "receipts_rang");
        if (receipts_rang_file.createNewFile()){
            log.debug("Create new receipts sequence number file: "+ receipts_rang_file);
            initial_receipts_sn_rang = 0;
        } else {
            BufferedReader br = new BufferedReader(new FileReader(receipts_rang_file));
            String line = br.readLine();
            initial_receipts_sn_rang = Integer.parseInt(line);
            br.close();
            log.debug("Load from file initial receipts sequence number rang: " + initial_receipts_sn_rang);
        }

        pw = new PrintWriter(new FileWriter(receipts_rang_file));
        if (Integer.MAX_VALUE - initial_receipts_sn_rang > rang){
            pw.println(initial_receipts_sn_rang +rang);
        } else {
            pw.println(rang);
        }
        pw.flush();
        pw.close();
        log.debug("Write to file initial receipts sequence number rang: "+ initial_receipts_sn_rang + rang);

        String s = config.getProperty("informer.host");
        if (s != null && !s.isEmpty()){
            informer_host = s;
            log.debug("Set informer host: "+ informer_host);
        } else {
            log.error("informer.host property is invalid or not specified in config");
            throw new InitializationException("informer.host property is invalid or not specified in config");
        }

        s = config.getProperty("informer.port");
        if (s != null && !s.isEmpty()){
            informer_port = Integer.parseInt(s);
            log.debug("Set informer port: "+ informer_port);
        } else {
            log.error("informer.port property is invalid or not specified in config");
            throw new InitializationException("informer.port property is invalid or not specified in config");
        }

        s = config.getProperty("informer.messages.list.capacity");
        if (s != null && !s.isEmpty()){
            informer_messages_list_capacity = Integer.parseInt(s);
            log.debug("Configuration property: informer.messages.list.capacity="+ informer_messages_list_capacity);
        } else {
            log.error("Configuration property 'informer.messages.list.capacity' is invalid or not specified in config");
            throw new InitializationException("Configuration property 'informer.messages.list.capacity' is invalid or not specified in config");
        }

        s = config.getProperty("informer.sending.timeout.mls");
        if (s != null && !s.isEmpty()){
            sending_to_informer_timeout = Integer.parseInt(s);
            log.debug("Configuration property: informer.sending.timeout.mls="+ sending_to_informer_timeout);
        } else {
            log.error("Configuration property 'informer.sending.timeout.mls' is invalid or not specified in config");
            throw new InitializationException("Configuration property 'informer.sending.timeout.mls' is invalid or not specified in config");
        }

        update_config_server_host = config.getProperty("update.config.server.host");

        s = Utils.getProperty(config, "update.config.server.port");

        update_config_server_port = Integer.parseInt(s);

        providers = loadInformerProviders();

        connection_provider_table = new Hashtable<String, Provider>();
        informer_user_delivery_ids_table = new Hashtable<String, HashSet<Integer>>();

        for(Provider provider: providers){
            String[] endpoints = provider.getEndpointIds();
            for(String connection_name: endpoints) {
                connection_provider_table.put(connection_name, provider);
                log.debug("con --> provider: "+connection_name + " --> "+provider.getName());
            }

            Vector<Delivery> deliveries = provider.getDeliveries();
            for(Delivery delivery: deliveries){
                int delivery_id = delivery.getId();
                String informer_user = delivery.getUser();
                if (!informer_user_delivery_ids_table.containsKey(informer_user))
                    informer_user_delivery_ids_table.put(informer_user, new HashSet<Integer>());

                Set<Integer> delivery_ids = informer_user_delivery_ids_table.get(informer_user);
                delivery_ids.add(delivery_id);
                log.debug("informer_user --> delivery_id: "+informer_user+" --> "+delivery_id);
            }
        }

        informer_user_password_table = loadInformerUsers();


        send_receipts_speed_default = Utils.getProperty(config, "send.receipts.speed.default", 100);
        send_receipt_max_time_default = Utils.getProperty(config, "send.receipt.max.time.default.min", 720);
        config_with_smpp_endpoints = loadSmppEndpoints();

        informer_user_connection_table = new Hashtable<String, DcpConnectionImpl>();
        for(String informer_user : informer_user_password_table.keySet()){
            try {
                DcpConnectionImpl connection = new DcpConnectionImpl(informer_user);
                informer_user_connection_table.put(informer_user, connection);
            } catch (AdminException e) {
                log.warn(e);
            }
        }

        max_journal_size_mb = Utils.getProperty(config, "journal.size.mb", 10);

        max_submit_date_journal_size = Utils.getProperty(config, "submit.date.journal.size.mb", 10);

        journal_dir = new File(Utils.getProperty(config, "journal.dir", System.getProperty("user.dir")+File.separator+"journal"));

        final_log_dir = new File(Utils.getProperty(config, "final.log.dir", System.getProperty("user.dir") + File.separator + "final_log"));



        resend_receipts_interval = Utils.getProperty(config, "resend.receipts.interval.sec", 60);

        delivery_response_timeout = Utils.getProperty(config, "resend.receipts.timeout.sec", 60);



        journal_clean_timeout = Utils.getProperty(config, "journal.clean.timeout.msl", 60000);

    }

    public void update() throws IOException, XmlConfigException, AdminException, SmppException {
        log.debug("Try to update configuration ...");
        Set<Provider> providers_temp = loadInformerProviders();
        Hashtable<String, String> informer_user_password_temp_table = loadInformerUsers();
        Properties properties_temp = loadSmppEndpoints();

        Server.getInstance().update(properties_temp);

        providers = providers_temp;

        connection_provider_table = new Hashtable<String, Provider>();
        informer_user_delivery_ids_table = new Hashtable<String, HashSet<Integer>>();

        for(Provider provider: providers){
            String[] endpoints = provider.getEndpointIds();
            for(String connection_name: endpoints) {
                connection_provider_table.put(connection_name, provider);
                log.debug("con --> provider: "+connection_name + " --> "+provider.getName());
            }

            Vector<Delivery> deliveries = provider.getDeliveries();
            for(Delivery delivery: deliveries){
                int delivery_id = delivery.getId();
                String informer_user = delivery.getUser();
                if (!informer_user_delivery_ids_table.containsKey(informer_user))
                    informer_user_delivery_ids_table.put(informer_user, new HashSet<Integer>());


                Set<Integer> delivery_ids = informer_user_delivery_ids_table.get(informer_user);
                delivery_ids.add(delivery_id);
                log.debug("informer_user --> delivery_id: "+informer_user+" --> "+delivery_id);
            }
        }

        Set<String> old_informer_users = informer_user_password_table.keySet();
        informer_user_password_table = informer_user_password_temp_table;
        Set<String> new_informer_users = informer_user_password_table.keySet();

        config_with_smpp_endpoints = properties_temp;

        // Add new dcp connections
        for(String new_user: new_informer_users){
            if (!old_informer_users.contains(new_user)){
                DcpConnectionImpl connection = new DcpConnectionImpl(new_user);
                informer_user_connection_table.put(new_user, connection);
            }
        }

        // Remove deleted dcp connections
        for(String old_user: informer_user_connection_table.keySet()){
            if (!new_informer_users.contains(old_user)){
                DcpConnectionImpl connection = informer_user_connection_table.remove(old_user);
                connection.interrupt();
            }
        }

        // Update passwords
        for(String old_user: informer_user_connection_table.keySet()){

            if (new_informer_users.contains(old_user)){
                DcpConnectionImpl connection = informer_user_connection_table.get(old_user);
                String new_password  = informer_user_password_table.get(old_user);
                DcpClient dcpClient = connection.getClient();
                if ( !new_password.equals(dcpClient.getPassword()) ){
                    dcpClient.setPassword(new_password);
                    log.debug("Update password for "+old_user+"_dcp_client.");
                }

            }

        }

        log.debug("Configuration updated.");
    }

    public Properties getProperties(){
        return config_with_smpp_endpoints;
    }

    public Provider getProvider(String connection_name){
        return connection_provider_table.get(connection_name);
    }

    public String getInformerUserPassword(String user){
        return informer_user_password_table.get(user);
    }

    private Properties loadSmppEndpoints() throws IOException, XmlConfigException {
        log.debug("l1");
        Properties result = (Properties) config.clone();

        XmlConfig xmlConfig = new XmlConfig();

        xmlConfig.load(new File(smpp_endpoints_file));
        XmlConfigSection endpoints_section = xmlConfig.getSection("endpoints");
        Collection<XmlConfigSection> c = endpoints_section.sections();
        log.debug("l2");

        for(XmlConfigSection s : c) {
            String endpoint_name = s.getName();
            log.debug(endpoint_name+"_l3");
            XmlConfigParam p = s.getParam("enabled");
            log.debug(endpoint_name+"_l31");
            if (p.getBool()) {
                log.debug(endpoint_name+"_l32");
                p = s.getParam("systemId");
                log.debug(endpoint_name+"_l33");
                String systemId = p.getString();
                log.debug(endpoint_name+"_l34");
                p = s.getParam("password");
                log.debug(endpoint_name+"_l35");
                String password = p.getString();
                log.debug(endpoint_name+"_l36");
                result.setProperty(CONNECTION_PREFIX + systemId + ".password", password);
                log.debug(endpoint_name+"_l37");
                int speed, max_time;
                XmlConfigSection s2 = s.getSection("send.receipts");
                log.debug(endpoint_name+"_l38");
                if (s2 != null){
                    log.debug(endpoint_name+"_l4");
                    p = s2.getParam("speed");
                    if (p != null){
                        speed = p.getInt();
                        result.setProperty(CONNECTION_PREFIX + systemId + ".send.receipts.speed", Integer.toString(speed));
                    } else {
                        speed = send_receipts_speed_default;
                        result.setProperty(CONNECTION_PREFIX + systemId + ".send.receipts.speed", Integer.toString(speed));
                    }

                    p = s2.getParam("max.time.min");
                    if (p!= null){
                        max_time = p.getInt();
                        result.setProperty(CONNECTION_PREFIX + systemId + ".send.receipt.max.time.min", Integer.toString(max_time));
                    } else {
                        max_time = send_receipt_max_time_default;
                        result.setProperty(CONNECTION_PREFIX + systemId + ".send.receipt.max.time.min", Integer.toString(max_time));
                    }

                } else {
                    speed = send_receipts_speed_default;
                    result.setProperty(CONNECTION_PREFIX + systemId + ".send.receipts.speed", Integer.toString(speed));

                    max_time = send_receipt_max_time_default;
                    result.setProperty(CONNECTION_PREFIX + systemId + ".send.receipt.max.time.min", Integer.toString(max_time));
                }

                log.debug("Load endpoint: name=" + endpoint_name + ", systemId=" + systemId + ", password=" + password + ", speed="+speed+", max_time="+max_time);
            } else {
                log.debug("Endpoint '" + endpoint_name + "' is disabled.");
            }
        }
        return result;
    }

    private Hashtable<String, String> loadInformerUsers() throws XmlConfigException {
        log.debug("Try to load users ...");
        Hashtable<String, String> t = new Hashtable<String, String>();

        XmlConfig xmlConfig = new XmlConfig();
        xmlConfig.load(new File(deliveries_file));

        XmlConfigSection users_section = xmlConfig.getSection("users");
        Collection<XmlConfigSection> c = users_section.sections();
        for (XmlConfigSection s : c) {
            String login = s.getName();
            XmlConfigParam p = s.getParam("password");
            String password = p.getString();
            t.put(login, password);
            log.debug("login: " + login + ", password: " + password);
        }
        log.debug("Users loaded.");
        return t;
    }

    private Set<Provider> loadInformerProviders() throws XmlConfigException {
        log.debug("Try to load providers ...");
        Set<Provider> result = new HashSet<Provider>();

        XmlConfig xmlConfig = new XmlConfig();
        xmlConfig.load(new File(deliveries_file));

        XmlConfigSection providers_section = xmlConfig.getSection("providers");
        Collection<XmlConfigSection> providers_collection = providers_section.sections();

        for (XmlConfigSection s : providers_collection) {

            Provider provider = new Provider();

            String name = s.getName();
            provider.setName(name);

            XmlConfigParam p = s.getParam("endpoint_ids");
            String[] endpoint_ids = p.getStringArray(",");
            provider.setEndpointIds(endpoint_ids);

            p = s.getParam("description");
            String description = p.getString();
            provider.setDescription(description);

            XmlConfigSection deliveries_section = s.getSection("deliveries");
            Collection<XmlConfigSection> deliveries_collection = deliveries_section.sections();

            for (XmlConfigSection d_s : deliveries_collection) {

                Delivery delivery = new Delivery();

                int delivery_id = Integer.parseInt(d_s.getName());
                delivery.setId(delivery_id);

                p = d_s.getParam("user");
                String user = p.getString();
                delivery.setUser(user);

                p = d_s.getParam("services_numbers");
                String[] services_numbers = p.getStringArray(",");
                delivery.setServicesNumbers(services_numbers);

                provider.addDelivery(delivery);
                log.debug("Load " + delivery.toString());
            }

            result.add(provider);

            log.debug("Load " + provider.toString());
        }
        log.debug("Providers loaded.");
        return result;
    }

    public String getUpdateServerHost(){
        return update_config_server_host;
    }

    public int getUpdateServerPort(){
        return update_config_server_port;
    }

    public String getInformerHost(){
        return informer_host;
    }

    public int getInformerPort(){
        return informer_port;
    }

    public int getInformerMessagesListCapacity(){
        return informer_messages_list_capacity;
    }

    public long getSendingToInformerTimeout(){
        return sending_to_informer_timeout;
    }


    public Set<Integer> getDeliveries(String informer_user) {
        return informer_user_delivery_ids_table.get(informer_user);
    }

    public DcpConnectionImpl getDCPConnection(String informer_user){
        return informer_user_connection_table.get(informer_user);
    }

    public int getMaxJournalSize(){
        return  max_journal_size_mb;
    }

    public File getJournalDir(){
        return journal_dir;
    }

    public File getFinalLogDir(){
        return final_log_dir;
    }

    public int getResendReceiptsInterval(){
        return resend_receipts_interval;
    }

    public int getSendReceiptMaxTimeDefault(){
        return send_receipt_max_time_default;
    }

    public int getDeliveryResponseTimeout(){
        return delivery_response_timeout;
    }

    public int getCleanJournalTimeout(){
        return journal_clean_timeout;
    }

    public int getSendReceiptsSpeed(){
        return send_receipts_speed_default;
    }

    public long getInitialMessageIdRang(){
        return initial_message_id_rang;
    }

    public int getRang(){
        return rang;
    }

    public File getMessageIdRangFile(){
        return message_id_rang_file;
    }

    public File getReceiptsRangFile(){
        return receipts_rang_file;
    }

    public int getReceiptsSequenceNumberRang(){
        return initial_receipts_sn_rang;
    }

    public int getMaxSubmitDateJournalSize() {
        return max_submit_date_journal_size;
    }
}