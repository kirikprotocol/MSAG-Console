package mobi.eyeline.dcpgw;

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
import java.util.Collection;
import java.util.Hashtable;
import java.util.Properties;
import java.util.Set;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 19.09.11
 * Time: 16:43
 */
public class ConfigurationManager {

    private static Logger log = Logger.getLogger(ConfigurationManager.class);

    private static ConfigurationManager instance = new ConfigurationManager();

    private String config;
    private String deliveries_file;

    private Properties properties;
    private Hashtable<String, String> informer_user_password_table;
    private Hashtable<String, Provider> connection_provider_table;

    public static final String CONNECTION_PREFIX = "smpp.sme.";

    public static ConfigurationManager getInstance(){
        return instance;
    }

    public ConfigurationManager(){

    }

    public void init(String config) throws IOException, XmlConfigException {
        this.config = config;
        connection_provider_table = loadProviders();
        informer_user_password_table = loadInformerUsers();
        properties = loadSmppConfigurations();
    }

    public void update() throws IOException, XmlConfigException {
        Hashtable<String, Provider> connection_provider_temp_table = loadProviders();
        Hashtable<String, String> informer_user_password_temp_table = loadInformerUsers();
        Properties properties_temp = loadSmppConfigurations();

        connection_provider_table = connection_provider_temp_table;
        informer_user_password_table = informer_user_password_temp_table;
        properties = properties_temp;
    }

    public Properties getProperties(){
        return properties;
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

    private Properties loadSmppConfigurations() throws IOException, XmlConfigException {
        Properties properties = new Properties();
        properties.load(new FileInputStream(config));
        String user_dir = System.getProperty("user.dir");

        String smpp_endpoints_file = Utils.getProperty(properties, "users.file", user_dir + File.separator + "conf" + File.separator + "endpoints.xml");
        deliveries_file = Utils.getProperty(properties, "deliveries.file", user_dir + File.separator + "conf" + File.separator + "deliveries.xml");

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

                properties.setProperty(CONNECTION_PREFIX + systemId + ".password", password);
                log.debug("Load endpoint: name=" + endpoint_name + ", systemId=" + systemId + ", password=" + password + " .");
            } else {
                log.debug("Endpoint '" + endpoint_name + "' is disabled.");
            }
        }
        return properties;
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

    private Hashtable<String, Provider> loadProviders() throws XmlConfigException {
        log.debug("Try to load providers ...");
        Hashtable<String, Provider> result = new Hashtable<String, Provider>();

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

            for (String endpoint_id : endpoint_ids) result.put(endpoint_id, provider);

            log.debug("Load " + provider.toString());
        }
        log.debug("Providers loaded.");
        return result;
    }

}
