package ru.sibinco.scag.beans.routing.subjects;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.SortByPropertyComparator;
import ru.sibinco.scag.beans.*;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.backend.routing.http.HttpRoutingManager;
import ru.sibinco.scag.Constants;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.util.*;


/**
 * Created by igork Date: 19.04.2004 Time: 17:39:13
 */
public class Index extends TabledBeanImpl implements TabledBean {

    private List siteItems = new ArrayList();
    private List subjectItems = new ArrayList();
    private String mbHttpSubjEdit = null;
    private String mbHttpSiteEdit = null;
    private String mbAddHttpSubj = null;
    private String mbAddHttpSite = null;
    private int subjectType;
    private String mbDeleteHttpSubj;
    private String mbDeleteHttpSite;

    protected int transportId =  -1;

    protected Collection getDataSource() {
        if (transportId == Transport.SMPP_TRANSPORT_ID) {
            return appContext.getScagRoutingManager().getSubjects().values();
        } else if (transportId == Transport.HTTP_TRANSPORT_ID) {
            return appContext.getHttpRoutingManager().getSites().values();
        } else if (transportId == Transport.MMS_TRANSPORT_ID) {
            return appContext.getScagRoutingManager().getSubjects().values();
        } else {
            return new HashMap().values();
        }
    }

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {

        HttpSession session = request.getSession();

        if (session.getAttribute(Constants.HTTP_TRANSPORT) == null) {
            session.setAttribute(Constants.HTTP_TRANSPORT, new Integer(Transport.SMPP_TRANSPORT_ID));
        } else {
            Integer tTransportId = (Integer) session.getAttribute(Constants.HTTP_TRANSPORT);
            if (tTransportId.intValue() != transportId && transportId != -1) {
                session.setAttribute(Constants.HTTP_TRANSPORT, new Integer(transportId));
            }
        }
        transportId = Integer.parseInt(String.valueOf(session.getAttribute(Constants.HTTP_TRANSPORT)));

        if (mbDeleteHttpSite != null) {
            super.process(request, response);
            deleteHttpSite();
        }
        if (mbDeleteHttpSubj != null) {
            super.process(request, response);
            deleteHttpSubject();
        }
        if (mbAddHttpSubj != null)
            throw new AddSiteSubjectException(Integer.toString(Transport.HTTP_TRANSPORT_ID),
                    Integer.toString(HttpRoutingManager.HTTP_SUBJECT_TYPE));
        if (mbAddHttpSite != null)
            throw new AddSiteSubjectException(Integer.toString(Transport.HTTP_TRANSPORT_ID),
                    Integer.toString(HttpRoutingManager.HTTP_SITE_TYPE));

        super.process(request, response);

        if (sort == null)
            sort = getDefaultSort();
        final SortedList results = new SortedList(getDataSource(), new SortByPropertyComparator(sort));
        totalSize = results.size();
        if (totalSize > startPosition)
            tabledItems = results.subList(startPosition, Math.min(totalSize, startPosition + pageSize));
        else
            tabledItems = new LinkedList();
        if (transportId == Transport.HTTP_TRANSPORT_ID) {
            final SortedList results2 = new SortedList(appContext.getHttpRoutingManager().getSubjects().values(), new SortByPropertyComparator(sort));
            totalSize = results2.size();
            if (totalSize > startPosition)
                subjectItems = results2.subList(startPosition, Math.min(totalSize, startPosition + pageSize));
            else
                subjectItems = new LinkedList();
        }
        if (transportId == Transport.HTTP_TRANSPORT_ID) {
            final SortedList results3 = new SortedList(appContext.getHttpRoutingManager().getSites().values(), new SortByPropertyComparator(sort));
            totalSize = results3.size();
            if (totalSize > startPosition)
                siteItems = results3.subList(startPosition, Math.min(totalSize, startPosition + pageSize));
            else
                siteItems = new LinkedList();
        }
    }

    protected void delete() {
        appContext.getScagRoutingManager().deleteSubjects(getUserName(), checkedSet);
    }

    protected void deleteHttpSite() throws SCAGJspException{
        appContext.getHttpRoutingManager().deleteSubjectsSite(getUserName(), checkedSet);
    }

    protected void deleteHttpSubject() throws SCAGJspException{
        appContext.getHttpRoutingManager().deleteSubjects(getUserName(), checkedSet);
    }

    public int getTransportId() {
        return transportId;
    }

    public void setTransportId(final int transportId) {
        this.transportId = transportId;
    }

    public String[] getTransportIds() {
        return Transport.transportIds;
    }

    public String[] getTransportTitles() {
        return Transport.transportTitles;
    }

    public void setSiteItems(List siteItems) {
        this.siteItems = siteItems;
    }

    public List getSiteItems() {
        return siteItems;
    }

    public List getSubjectItems() {
        return subjectItems;
    }

    public void setSubjectItems(List subjectItems) {
        this.subjectItems = subjectItems;
    }

    public String getMbHttpSubjEdit() {
        return mbHttpSubjEdit;
    }

    public void setMbHttpSubjEdit(String mbHttpSubjEdit) {
        this.mbHttpSubjEdit = mbHttpSubjEdit;
    }

    public String getMbAddHttpSubj() {
        return mbAddHttpSubj;
    }

    public void setMbAddHttpSubj(String mbAddHttpSubj) {
        this.mbAddHttpSubj = mbAddHttpSubj;
    }

    public String getMbAddHttpSite() {
        return mbAddHttpSite;
    }

    public void setMbAddHttpSite(String mbAddHttpSite) {
        this.mbAddHttpSite = mbAddHttpSite;
    }

    public int getSubjectType() {
        return subjectType;
    }

    public void setSubjectType(int subjectType) {
        this.subjectType = subjectType;
    }

    public String getMbHttpSiteEdit() {
        return mbHttpSiteEdit;
    }

    public void setMbHttpSiteEdit(String mbHttpSiteEdit) {
        this.mbHttpSiteEdit = mbHttpSiteEdit;
    }

    public String getMbDeleteHttpSubj() {
        return mbDeleteHttpSubj;
    }

    public void setMbDeleteHttpSubj(String mbDeleteHttpSubj) {
        this.mbDeleteHttpSubj = mbDeleteHttpSubj;
    }

    public String getMbDeleteHttpSite() {
        return mbDeleteHttpSite;
    }

    public void setMbDeleteHttpSite(String mbDeleteHttpSite) {
        this.mbDeleteHttpSite = mbDeleteHttpSite;
    }


}
