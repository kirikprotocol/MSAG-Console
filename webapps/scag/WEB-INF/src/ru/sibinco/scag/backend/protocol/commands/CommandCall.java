package ru.sibinco.scag.backend.protocol.commands;

import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.daemon.Command;

import java.util.Map;
import java.util.Iterator;
import java.util.List;
import java.util.Collection;

import org.w3c.dom.Element;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 24.03.2005
 * Time: 16:46:35
 * To change this template use File | Settings | File Templates.
 */
public class CommandCall extends Command {


    public CommandCall(String commandName, Type returnType, Map args)
            throws SibincoException {

        super(commandName, "command_gw.dtd");
        Element callElem = document.getDocumentElement();
        callElem.setAttribute("returnType", returnType.getName());

        for (Iterator i = args.keySet().iterator(); i.hasNext();) {
            String paramName = (String) i.next();
            Object param = args.get(paramName);
            Element paramElem = document.createElement("param");
            callElem.appendChild(paramElem);
            paramElem.setAttribute("name", StringEncoderDecoder.encode(paramName));
            if (param instanceof String) {
                paramElem.setAttribute("type", Type.Types[Type.STRING_TYPE].getName());
                paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode((String) param)));
            } else if (param instanceof Integer) {
                paramElem.setAttribute("type", Type.Types[Type.INT_TYPE].getName());
                paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Integer) param).longValue()))));
            } else if (param instanceof Long) {
                paramElem.setAttribute("type", Type.Types[Type.INT_TYPE].getName());
                paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Long) param).longValue()))));
            } else if (param instanceof Boolean) {
                paramElem.setAttribute("type", Type.Types[Type.BOOLEAN_TYPE].getName());
                paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(String.valueOf(((Boolean) param).booleanValue()))));
            } else if (param instanceof List) {
                paramElem.setAttribute("type", Type.Types[Type.STRING_TYPE].getName());
                paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(encodeStringList((List) param))));
            } else if (param instanceof Collection) {
                paramElem.setAttribute("type", Type.Types[Type.STRING_TYPE].getName());
                paramElem.appendChild(document.createTextNode(StringEncoderDecoder.encode(encodeStringList(new SortedList((Collection) param)))));
            }
        }
    }

    private String encodeStringList(List list) {
        String result = "";
        for (Iterator i = list.iterator(); i.hasNext();) {
            String s = (String) i.next();
            result += encodeComma(s) + (i.hasNext() ? "," : "");
        }
        return result;
    }

    private String encodeComma(String value) {
        String result = "";
        for (int i = 0; i < value.length(); i++) {
            char c = value.charAt(i);
            if (c == ',' || c == '\\')
                result += '\\';
            result += c;
        }
        return result;
    }
}
