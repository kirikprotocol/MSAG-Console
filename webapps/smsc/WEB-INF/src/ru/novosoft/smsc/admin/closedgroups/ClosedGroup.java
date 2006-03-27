package ru.novosoft.smsc.admin.closedgroups;

import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.util.StringEncoderDecoder;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.PrintWriter;

public class ClosedGroup {
    private int id;      //32-битный идентификатор
    private String name; //64-байтное имя //todo надо сделать обрезание 64 байт
    private String def;  //описание группы
    private MaskList masks; //список масок принадлежащих группе

    public ClosedGroup(final Element closedGroupElem) throws AdminException {
        id = Integer.parseInt(closedGroupElem.getAttribute("id"));
        name = closedGroupElem.getAttribute("name");
        if (name.length() > Constants.CLOSED_GROUP_NAME_MAXLENGTH) {
          throw new AdminException("Closed group name is too long: " + name.length() + " chars \"" + name + '"');
        }
        def = closedGroupElem.getAttribute("desc");
        NodeList masksList = closedGroupElem.getElementsByTagName("mask");
        NodeList addrList = closedGroupElem.getElementsByTagName("address");
        String[] masks = new String[masksList.getLength() + addrList.getLength()];
        for (int j = 0; j < masksList.getLength(); j++) {
          Element maskElem = (Element) masksList.item(j);
          masks[j] = maskElem.getAttribute("value").trim();
        }
        for (int j = 0; j < addrList.getLength(); j++) {
          Element maskElem = (Element) addrList.item(j);
          masks[masksList.getLength()+j] = maskElem.getAttribute("value").trim();
        }
        this.masks = new MaskList(masks);
    }

    public ClosedGroup(String name, String def, String[] masks) throws AdminException {
        this.id = 0;
        this.name = name;
        this.def = def;
        this.masks = new MaskList(masks);
    }

    public PrintWriter store(final PrintWriter out) {
        out.println("   <group id=\"" + getId()
                + "\" name=\"" + StringEncoderDecoder.encode(getName())
                + "\" desc=\"" + StringEncoderDecoder.encode(getDef())
                + "\">");
        getMasks().store(out);
        out.println("   </group>");
        return out;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getDef() {
        return def;
    }

    public void setDef(String def) {
        this.def = def;
    }

    public MaskList getMasks() {
        return masks;
    }

    public void setMasks(MaskList masks) {
        this.masks = masks;
    }
}
