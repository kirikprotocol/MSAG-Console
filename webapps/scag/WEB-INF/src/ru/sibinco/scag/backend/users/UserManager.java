package ru.sibinco.scag.backend.users;

import org.w3c.dom.*;
import org.xml.sax.SAXException;
import org.apache.log4j.Logger;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.installation.HSDaemon;
import ru.sibinco.scag.web.security.*;
import ru.sibinco.scag.web.WebContext;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;

public class UserManager {

    private Logger logger = Logger.getLogger(this.getClass());

    private Map<String, User> users = Collections.synchronizedMap(new HashMap<String, User>());
    private final File configFile;
    private final HSDaemon hsDaemon;
    public UserManager(String config, HSDaemon hsDaemon) throws IOException, ParserConfigurationException, SAXException {
        this.configFile = new File(config);
        this.hsDaemon = hsDaemon;
        try {
            load();
        } catch (IOException e) {
            e.printStackTrace();
            logger.warn(e.getMessage());
            throw new IOException(e.getMessage());
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
            logger.warn(e.getMessage());
            throw new ParserConfigurationException(e.getMessage());
        } catch (SAXException e) {
            e.printStackTrace();
            logger.warn(e.getMessage());
            throw new SAXException(e.getMessage());
        }

    }

    private void load() throws IOException, ParserConfigurationException, SAXException {
        Document document = Utils.parse(new FileReader(configFile));
        NodeList userNodes = document.getElementsByTagName("user");
        for (int i = 0; i < userNodes.getLength(); i++) {
            User user = new ScagUser((Element) userNodes.item(i));
            users.put(user.getLogin(), user);
        }
    }

    public synchronized void apply() throws IOException, ParserConfigurationException, SAXException, SibincoException {
        store();

        Document usersXmlDocument = XMLDocumentParser.parse(new FileReader(configFile));

        Authenticator authenticator =  new XmlAuthenticator(usersXmlDocument);
        WebContext.setAuthenticator(authenticator);

        RoleMapper oldRoleMapper = WebContext.getRoleMapper();
        Map<String, Set<String>> role2URIPatterns = oldRoleMapper.getURIsForRoles();
        RoleMapper newRoleMapper = new XMLRoleMapper(usersXmlDocument, role2URIPatterns);
        WebContext.setRoleMapper(newRoleMapper);

        hsDaemon.store(configFile);
    }

    protected void store() throws IOException {
        File configNew = Functions.createNewFilenameForSave(configFile);

        PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(configNew), Functions.getLocaleEncoding()));
        Functions.storeConfigHeader(out, "users", "users.dtd", Functions.getLocaleEncoding());
        SortedList sortedList = new SortedList(users.keySet());
        for (Object o : sortedList) {
          String userLogin = (String) o;
          User user = users.get(userLogin);
          out.print(user.getXmlText());
        }
        Functions.storeConfigFooter(out, "users");
        out.flush();
        out.close();

        Functions.renameNewSavedFileToOriginal(configNew, configFile);
    }

    public synchronized Map getUsers() {
        return users;
    }
}
