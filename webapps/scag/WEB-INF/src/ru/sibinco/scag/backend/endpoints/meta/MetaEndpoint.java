package ru.sibinco.scag.backend.endpoints.meta;

import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.scag.backend.sme.ProviderManager;
import ru.sibinco.scag.backend.sme.Provider;
import ru.sibinco.scag.Constants;

import java.io.PrintWriter;
import java.util.Iterator;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.apache.log4j.Category;

/**
 * Created by IntelliJ IDEA.
 * User: mixim
 * Date: 22.10.2007
 * Time: 16:43:52
 * To change this template use File | Settings | File Templates.
 */
public class MetaEndpoint {

    private String id;
    private String policy = "";
    private String type = "";
    private boolean enabled;

//    private String[] centerIds = { "c1", "c2", "c3" };
//    private String[] spIds = { "s1", "s2", "s3" };
//    private String[] smeIds = { "sme1", "sme2", "sme3" };
//    private SortedList smeIds = new SortedList();

    private Category logger = Category.getInstance(this.getClass());

    public MetaEndpoint(String id) {
        this.id = id;
    }

    public MetaEndpoint(String id, String policy, String type) {
        this.id = id;
        this.policy = policy;
        this.type = type;
    }

//    public MetaEndpoint(String id, SortedList selectedSmes) {
//        this.id = id;
//        this.smeIds = selectedSmes;
//    }

    public MetaEndpoint(final Element svcElement, final ProviderManager providerManager)
            throws NullPointerException {
        final NodeList list = svcElement.getElementsByTagName("param");
        for (int i = 0; i < list.getLength(); i++) {
            final Element paramElem = (Element) list.item(i);
            final String name = paramElem.getAttribute("name");
            final String value = paramElem.getAttribute("value");
//            try {
                if ("metaGroup".equals(name)) {
                    id = StringEncoderDecoder.encode(value);
                } else if (name.startsWith("type")) {
                    type = value;
                } else if (name.startsWith("policy")){
                    policy = value;
                } else if ("enabled".equals(name)) {
                    enabled = Boolean.valueOf(value).booleanValue();
                }
//            } catch (NumberFormatException e) {
//                logger.warn("Int parameter \"" + name + "\" misformatted: '" + value + "', skipped");
//            }
        }
        if( null == id )
            throw new NullPointerException("SME System ID is null");
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public PrintWriter store(final PrintWriter out) {
        return storeFooter(storeBody(storeHeader(out)));
    }

    public PrintWriter store(final PrintWriter out, int type) {
        switch (type){
            case Constants.STORE_TYPE_SVC:
                return storeFooterSvc(storeBody(storeHeaderSvc(out)) );
//                break;
            case Constants.STORE_TYPE_CENTER:
                return storeFooterCenter(storeBody(storeHeaderCenter(out)) );
//                break;
            default:
                return storeFooter(storeBody(storeHeader(out)));
//                break;
        }
    }

    protected PrintWriter storeHeader(final PrintWriter out) {
        out.println("  <metarecord>");
        return out;
    }

    protected PrintWriter storeHeaderSvc(final PrintWriter out) {
        out.println("  <" + Constants.META_SME_RECORD_TAG + ">");
        return out;
    }

    protected PrintWriter storeHeaderCenter(final PrintWriter out) {
        out.println("  <" + Constants.META_SMSC_RECORD_TAG + ">");
        return out;
    }

    protected PrintWriter storeBody(final PrintWriter out) {
        out.println("\t<param name=\"systemId\"\tvalue=\"" + StringEncoderDecoder.encode(id) + "\"/>");
//        out.println("\t<param name=\"type\"\tvalue=\"" + StringEncoderDecoder.encode(type) + "\"/>");
        out.println("\t<param name=\"policy\"\tvalue=\"" + StringEncoderDecoder.encode(policy) + "\"/>");
        out.println("\t<param name=\"persistence\"\tvalue=\"" + enabled + "\"/>");
//        for(Iterator iter = smeIds.iterator(); iter.hasNext(); ){
//            String sme = (String)iter.next();
//            out.println("\t<param name=\"policy" + policy + "\"\tvalue=\"" + StringEncoderDecoder.encode(sme) + "\"/>");
//        }
        return out;
    }
    protected PrintWriter storeFooter(final PrintWriter out) {
        out.println("  </metarecord>");
        return out;
    }

    protected PrintWriter storeFooterSvc(final PrintWriter out) {
        out.println("  </" + Constants.META_SME_RECORD_TAG + ">");
        return out;
    }

    protected PrintWriter storeFooterCenter(final PrintWriter out) {
        out.println("  </" + Constants.META_SMSC_RECORD_TAG + ">");
        return out;
    }

    public String getPolicy() {
        return policy;
    }

    public void setPolicy(String policy) {
        this.policy = policy;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public boolean getEnabled() {
        return enabled;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }
//    public SortedList getSmeIds() {
//        return smeIds;
//    }
//
//    public void setSmeIds(SortedList smeIds) {
//        this.smeIds = smeIds;
//    }
//
//    public String[] getCenterIds() {
//        return centerIds;
//    }
//
//    public void setCenterIds(String[] centerIds) {
//        this.centerIds = centerIds;
//    }
//
//    public String[] getSpIds() {
//        return spIds;
//    }
//
//    public void setSpIds(String[] spIds) {
//        this.spIds = spIds;
//    }

}
