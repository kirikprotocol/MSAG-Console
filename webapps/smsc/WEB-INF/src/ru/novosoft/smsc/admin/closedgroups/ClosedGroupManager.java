package ru.novosoft.smsc.admin.closedgroups;

import org.apache.log4j.Category;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.FactoryConfigurationError;
import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.Iterator;
import java.util.List;

public class ClosedGroupManager {
    private SMSCAppContext appContext = null;
    private ClosedGroupList closedGroups = null;
    private Category logger = Category.getInstance(this.getClass());

    protected final static String SMSC_CLOSED_GROUPS_CONFIG = "ClosedGroups.xml";
    protected final static String PARAM_NAME_last_used_id = "lastId";
    protected final static String SECTION_NAME_group = "group";
    protected final static String ROOT_ELEMENT = "records";
    public static int LAST_GROUP_ID = 0;

    public ClosedGroupManager(SMSCAppContext appContext) throws AdminException {
        this.appContext = appContext;
        load();
    }

    public MaskList getAbonentList(long id) throws AdminException {
        checkSmscIsOnline();
        MaskList result = new MaskList();
        List lst = appContext.getSmsc().cgGetAbonentList(id);
        for (int i = 0; i < lst.size(); i++) {
            Object o = lst.get(i);
            if (o != null && o instanceof String)
                result.add(new Mask((String) o));
            else
                logger.error("ClosedGroupManager.getAbonentList: bad result returned from SMSC-center");
        }
        return result;
    }

    public ClosedGroup add(String name, String def, String[] masks) throws AdminException {
        return add(new ClosedGroup(name, def, masks));
    }

    public ClosedGroup add(ClosedGroup cg) throws AdminException {
        checkSmscIsOnline();
        int newId = 1;
        while (getClosedGroups().get(newId) != null) newId = ++LAST_GROUP_ID;
        cg.setId(newId);
        appContext.getSmsc().cgAddGroup(cg.getId(), cg.getName(), cg.getMasks());
        closedGroups.put(cg);
        save();
        return cg;
    }

    public void alter(ClosedGroup cg, String newName, String newDescr, MaskList add, MaskList del) throws AdminException {
        if (!newDescr.equals("")) cg.setDef(newDescr);
        if (!newName.equals("")) {
            removeGroup(cg.getName());
            cg.setName(newName);
            appContext.getSmsc().cgAddGroup(cg.getId(), newName, cg.getMasks());
            closedGroups.put(cg);
        }
        if (!add.isEmpty() || !del.isEmpty()) {
            MaskList oldMasks = cg.getMasks();
            MaskList masks = new MaskList();
            for (Iterator i = oldMasks.iterator(); i.hasNext();) {
                Mask m = (Mask) i.next();
                if (!del.contains(m)) masks.add(m);
            }
            for (Iterator i = add.iterator(); i.hasNext();)
                masks.add((Mask) i.next());
            cg.setMasks(masks);
            appContext.getSmsc().cgUpdateGroupMasks(cg.getId(), add.getNames(), del.getNames());
        }
        save();
    }

    public ClosedGroup removeGroup(String cgName) throws AdminException {
        checkSmscIsOnline();
        appContext.getSmsc().cgRemoveGroup(closedGroups.get(cgName).getId());
        ClosedGroup result = closedGroups.remove(cgName);
        save();
        return result;
    }

    synchronized public void load() throws AdminException {
        try {
            final File smscConfFolder = WebAppFolders.getSmscConfFolder();
            File config = new File(smscConfFolder, SMSC_CLOSED_GROUPS_CONFIG);

            Document closedGroupDoc = Utils.parse(config.getAbsolutePath());
            NodeList a = closedGroupDoc.getElementsByTagName(PARAM_NAME_last_used_id);
            if (a.getLength() > 0)
                LAST_GROUP_ID = Integer.parseInt(((Element) a.item(0)).getAttribute("value"));
            closedGroups = new ClosedGroupList(closedGroupDoc.getDocumentElement());
        } catch (FactoryConfigurationError error) {
            logger.error("Couldn't configure xml parser factory", error);
            throw new AdminException("Couldn't configure xml parser factory: " + error.getMessage());
        } catch (ParserConfigurationException e) {
            logger.error("Couldn't configure xml parser", e);
            throw new AdminException("Couldn't configure xml parser: " + e.getMessage());
        } catch (SAXException e) {
            logger.error("Couldn't parse", e);
            throw new AdminException("Couldn't parse: " + e.getMessage());
        } catch (IOException e) {
            logger.error("Couldn't perform IO operation", e);
            throw new AdminException("Couldn't perform IO operation: " + e.getMessage());
        } catch (NullPointerException e) {
            logger.error("Couldn't parse", e);
            throw new AdminException("Couldn't parse: " + e.getMessage());
        }
    }

    synchronized public void save() throws AdminException {
        try {
            final File file = new File(WebAppFolders.getSmscConfFolder(), SMSC_CLOSED_GROUPS_CONFIG);
            final File newFile = Functions.createNewFilenameForSave(file);
            PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(newFile), Functions.getLocaleEncoding()));
            Functions.storeConfigHeader(out, ROOT_ELEMENT, "ClosedGroups.dtd", Functions.getLocaleEncoding());
            out.println("   <"+PARAM_NAME_last_used_id+" value=\"" + LAST_GROUP_ID + "\"/>");
            closedGroups.store(out);
            Functions.storeConfigFooter(out, ROOT_ELEMENT);
            out.flush();
            out.close();
            Functions.renameNewSavedFileToOriginal(newFile, file);
        } catch (FileNotFoundException e) {
            throw new AdminException("Couldn't save new routes settings: Couldn't write to destination config filename: " + e.getMessage());
        } catch (IOException e) {
            logger.error("Couldn't save new routes settings", e);
            throw new AdminException("Couldn't save new routes settings: " + e.getMessage());
        }
    }

    public ClosedGroupList getClosedGroups() {
        return closedGroups;
    }

    private synchronized void checkSmscIsOnline() throws AdminException {
/*        if (!appContext.getSmsc().getInfo().isOnline()) {
            if (!commands.isEmpty()) rollback();
            throw new AdminException("Smsc if offline");
        }*/
    }
}
