/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.routing;

import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import org.w3c.dom.Element;

import java.util.Map;
import java.io.PrintWriter;

/**
 * The <code>Destination</code> class represents
 * <p><p/>
 * Date: 21.10.2005
 * Time: 13:36:46
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Destination extends Source {

    private Svc svc = null;
    private Center center = null;
    private MetaEndpoint metaSvc = null;
    private MetaEndpoint metaCenter = null;

    public Destination(Subject subject) throws SibincoException {
        this(subject, subject.getSvc());
    }

    public Destination(Subject subject, Center center) throws SibincoException {
        super(subject);
        if ((this.center = center) == null)
            throw new SibincoException("Center is null");
    }

    public Destination(Subject subject, Svc svc) throws SibincoException {
        super(subject);
        if ((this.svc = svc) == null)
            throw new SibincoException("Sme is null");
    }

    public Destination(Subject subject, MetaEndpoint meta) throws SibincoException {
        super(subject);
        if( meta.getType().equals( Constants.META_TYPE_SERVICE) ){
            if ((this.metaSvc = meta) == null)
                throw new SibincoException("MetaSvc is null (with subject)");
        }else{
            if ((this.metaCenter = meta) == null)
                throw new SibincoException("MetaCenter is null (with subject)");
        }
    }

    public Destination(Mask mask, Svc svc) throws SibincoException {
        super(mask);
        if ((this.svc = svc) == null)
            throw new SibincoException("Sme is null");
    }

    public Destination(Mask mask, Center center) throws SibincoException {
        super(mask);
        if ((this.center = center) == null)
            throw new SibincoException("Ceneter is null");
    }

    public Destination(Mask mask, MetaEndpoint meta) throws SibincoException {
        super(mask);
        if(meta.getType().equals(Constants.META_TYPE_SERVICE)){
            if ((this.metaSvc = meta) == null)
                throw new SibincoException("MetaSvc is null (with mask)");
        } else{
            if ((this.metaCenter = meta) == null)
                throw new SibincoException("MetaCenter is null (with mask)");
        }
    }

//    public Destination(Mask mask, MetaEndpoint metaCenter) throws SibincoException {
//        super(mask);
//        if ((this.center = center) == null)
//            throw new SibincoException("Ceneter is null");
//    }

    public Destination(Element dstElem, Map subjects, SmppManager smppManager)
            throws SibincoException {
        super(dstElem, subjects);
        svc = (Svc) smppManager.getSvcs().get(dstElem.getAttribute("sme"));
        if (svc == null){
            center = (Center) smppManager.getCenters().get(dstElem.getAttribute("sme"));
            if(center==null){
                metaSvc = (MetaEndpoint) smppManager.getMetaServices().get(dstElem.getAttribute("sme"));
                if(metaSvc==null){
                    metaCenter = (MetaEndpoint) smppManager.getMetaCenters().get(dstElem.getAttribute("sme"));
                }
            }
        }
        if(svc == null && center == null && metaSvc == null && metaCenter == null){
            throw new SibincoException("Unknown Sme \"" + dstElem.getAttribute("sme") + '"');
        }
    }

    public Svc getSvc() {
        return svc;
    }

    public void setSvc(final Svc svc) {
        this.svc = svc;
    }

    public Center getCenter() {
        return center;
    }

    public void setCenter(final Center center) {
        this.center = center;
    }

    public MetaEndpoint getMetaSvc() {
        return metaSvc;
    }

    public void setMetaSvc(final MetaEndpoint metaSvc) {
        this.metaSvc = metaSvc;
    }

    public MetaEndpoint getMetaCenter() {
        return metaCenter;
    }

    public void setMetaCenter(final MetaEndpoint metaCenter) {
        this.metaCenter = metaCenter;
    }

    public void setMeta(final MetaEndpoint meta) {
        if( meta.getType().equals(Constants.META_TYPE_SERVICE))
            this.metaSvc = meta;
        else
            this.metaCenter = meta;
    }

    public PrintWriter store(PrintWriter out) {
//                getSvc() == null ? getCenter().getId() : getSvc().getId()) +
        out.println("    <destination sme=\"" + StringEncoderDecoder.encode(
                        getSvc()!=null ? getSvc().getId() :
                        getCenter()!=null ? getCenter().getId() :
                        getMetaSvc()!=null? getMetaSvc().getId():
                        getMetaCenter().getId()
                        ) + "\">");
        if (isSubject())
            out.println("      <subject id=\"" + StringEncoderDecoder.encode(subject.getName()) + "\"/>");
        else
            out.println("      <mask value=\"" + StringEncoderDecoder.encode(mask.getNormalizedMask()) + "\"/>");
        out.println("    </destination>");
        return out;
    }

}

