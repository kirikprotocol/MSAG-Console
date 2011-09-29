package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.dcpgw.utils.Utils;
import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 19.09.11
 * Time: 16:43
 */
public class ConfigurationManager {

    private static Logger log = Logger.getLogger(ConfigurationManager.class);

    private static ConfigurationManager instance = new ConfigurationManager();

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

    public static ConfigurationManager getInstance(){
        return instance;
    }

    public ConfigurationManager(){

    }

    public void init(String config_file) throws IOException, XmlConfigException, InitializationException {
        config = new Properties();
        config.load(new FileInputStream(config_file));
        String user_dir = System.getProperty("user.dir");

        smpp_endpoints_file = Utils.getProperty(config, "users.file", user_dir + File.separator + "conf" + File.separator + "endpoints.xml");
        deliveries_file = Utils.getProperty(config, "deliveries.file", user_dir + File.separator + "conf" + File.separator + "deliveries.xml");

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

        s = config.getProperty("sending.timeout.mls");
        if (s != null && !s.isEmpty()){
            sending_to_informer_timeout = Integer.parseInt(s);
            log.debug("Configuration property: sending.timeout.mls="+ sending_to_informer_timeout);
        } else {
            log.error("Configuration property 'sending.timeout.mls' is invalid or not specified in config");
            throw new InitializationException("Configuration property 'sending.timeout.mls' is invalid or not specified in config");
        }

        update_config_server_host = config.getProperty("update.config.server.host");

        s = Utils.getProperty(config, "update.config.server.port");

        update_config_server_port = Integer.parseInt(s);

        informer_user_delivery_ids_table = new Hashtable<String, HashSet<Integer>>();

        providers = loadInformerProviders();
        informer_user_password_table = loadInformerUsers();
        config_with_smpp_endpoints = loadSmppEndpoints();
    }

    public void update() throws IOException, XmlConfigException {
        Set<Provider> providers_temp = loadInformerProviders();
        Hashtable<String, String> informer_user_password_temp_table = loadInformerUsers();
        Properties properties_temp = loadSmppEndpoints();

        providers = providers_temp;

        connection_provider_table = new Hashtable<String, Provider>();
        informer_user_delivery_ids_table = new Hashtable<String, HashSet<Integer>>();

        for(Provider provider: providers){
            String[] endpoints = provider.getEndpointIds();
            for(String connection_name: endpoints) connection_provider_table.put(connection_name, provider);

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

        informer_user_password_table = informer_user_password_temp_table;
        config_with_smpp_endpoints = properties_temp;
    }

    public Properties getConfig(){
        return config_with_smpp_endpoints;
    }

    public Provider getProvider(String connection_name){
        return connection_provider_table.get(connection_name);
    }

    public Set<String> getInformerUsers(){
        return informer_user_password_table.keySet();
    }

    public String getInformerUserPassword(String user){
        return informer_user_password_table.get(user);
    }

    private Properties loadSmppEndpoints() throws IOException, XmlConfigException {
        Properties result = (Properties) config.clone();

        XmlConfig xmlConfig = new XmlConfig();

        xmlConfig.load(new File(smpp_endpoints_file));
        XmlConfigSection endpoints_section = xmlConfig.getSection("endpoints");
        Collection<XmlConfigSection> c = endpoints_section.sections();
        for(XmlConfigSection s : c) {
            String endpoint_name = s.getName();

            XmlConfigParam p = s.getParam("enabled");
            if (p.getBool()) {
                p = s.getParam("systemId");
                String systemId = p.getString();

                p = s.getParam("password");
                String password = p.getString();

                result.setProperty(CONNECTION_PREFIX + systemId + ".password", password);
                log.debug("Load endpoint: name=" + endpoint_name + ", systemId=" + systemId + ", password=" + password + " .");
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
}
