/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.routing;

import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.endpoints.centers.Center;
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

    public Destination(Subject subject) throws SibincoException {
        this(subject, subject.getDefaultSme());
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

    public Destination(Element dstElem, Map subjects, SmppManager smppManager)
            throws SibincoException {
        super(dstElem, subjects);
        svc = (Svc) smppManager.getSvcs().get(dstElem.getAttribute("sme"));
        if (svc == null){
            center = (Center) smppManager.getCenters().get(dstElem.getAttribute("sme"));
        }
        if(svc == null && center == null){
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

    public PrintWriter store(PrintWriter out) {
        out.println("    <destination sme=\"" + StringEncoderDecoder.encode(getSvc() == null ? getCenter().getId() : getSvc().getId()) + "\">");
        if (isSubject())
            out.println("      <subject id=\"" + StringEncoderDecoder.encode(subject.getName()) + "\"/>");
        else
            out.println("      <mask value=\"" + StringEncoderDecoder.encode(mask.getNormalizedMask()) + "\"/>");
        out.println("    </destination>");
        return out;
    }

}

