/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.routing.options;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.routing.http.placement.AbonentsPlacement;
import ru.sibinco.scag.backend.routing.http.placement.Option;
import ru.sibinco.scag.backend.routing.http.placement.SitePlacement;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.ArrayList;
import java.util.List;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 05.07.2006
 * Time: 12:00:00
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends EditBean {

    public static final String ADDRESS_SELECT = "addressSelect_";
    public static final String SITE_SELECT = "siteSelect_";
    public static final String ABON_USR_SELECT = "abonUsrSelect_";
    public static final String SITE_USR_SELECT = "siteUsrSelect_";
    public static final String URL_TYPE = "url";
    public static final String PARAM_TYPE = "param";
    public static final String HEADER_TYPE = "header";
    public static final String COOKIE_TYPE = "cookie";
    private AbonentsPlacement[] abonentAddress = new AbonentsPlacement[0];
    private String[] abonAddrName = new String[0];
    private String[] abonAddrType = new String[0];

    private AbonentsPlacement[] abonentUsr = new AbonentsPlacement[0];
    private String[] abonentUsrName = new String[0];
    private String[] abonentUsrType = new String[0];


    private SitePlacement[] siteAddress = new SitePlacement[0];
    private String[] siteAddrName = new String[0];
    private String[] siteAddrType = new String[0];

    private SitePlacement[] siteUsr = new SitePlacement[0];
    private String[] siteUsrName = new String[0];
    private String[] siteUsrType = new String[0];

    private String[] optionTypes = {URL_TYPE, PARAM_TYPE, HEADER_TYPE, COOKIE_TYPE};

    public String getId() {
        return null;
    }

    protected void load(final String loadId) throws SCAGJspException {
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        abonentAddress = getAbonentsPlacements(appContext.getHttpRoutingManager().getOptions().getAbonAddressPlace());
        siteAddress = getSitePlacement(appContext.getHttpRoutingManager().getOptions().getSiteAddressPlace());
        abonentUsr = getAbonentsPlacements(appContext.getHttpRoutingManager().getOptions().getAbonUsrPlace());
        siteUsr = getSitePlacement(appContext.getHttpRoutingManager().getOptions().getSiteUsrPlace());
        if (getMbSave() != null) {

            //**********************************************************************************//
            //                            GLOBAL ADDRESS PLACE                                  //
            //**********************************************************************************//
            List abonAddrList = new ArrayList();
            for (int i = 0; i < abonAddrName.length; i++) {
                String s = new StringBuffer().append(ADDRESS_SELECT).append(abonAddrName[i]).append(abonAddrType[i].
                        substring(abonAddrType[i].lastIndexOf('_'))).toString();
                String[] results = request.getParameterValues(s);

                int priority = i + 1;
                for (int j = 0; j < results.length; j++) {
                    try {
                        abonAddrList.add(new AbonentsPlacement(results[j], abonAddrName[i], priority));
                    } catch (SibincoException e) {
                        e.printStackTrace();
                    }
                }
            }
            abonentAddress = (AbonentsPlacement[]) abonAddrList.toArray(new AbonentsPlacement[abonAddrList.size()]);
            abonAddrList.clear();

            //**********************************************************************************//
            //                            DEFAULT SITE ADDRESS PLACE                            //
            //**********************************************************************************//
            List siteAddrList = new ArrayList();
            for (int i = 0; i < siteAddrName.length; i++) {
                String s = new StringBuffer().append(SITE_SELECT).append(siteAddrName[i]).append(siteAddrType[i].
                        substring(siteAddrType[i].lastIndexOf('_'))).toString();
                String[] results = request.getParameterValues(s);
                for (int j = 0; j < results.length; j++) {
                    try {
                        siteAddrList.add(new SitePlacement(results[j], siteAddrName[i]));
                    } catch (SibincoException e) {
                        e.printStackTrace();
                    }
                }
            }
            siteAddress = (SitePlacement[]) siteAddrList.toArray(new SitePlacement[siteAddrList.size()]);
            siteAddrList.clear();

            //**********************************************************************************//
            //                           ABONENT DEFAULT USR PLACE                              //
            //**********************************************************************************//

            List abonentUsrList = new ArrayList();
            for (int i = 0; i < abonentUsrName.length; i++) {
                String s = new StringBuffer().append(ABON_USR_SELECT).append(abonentUsrName[i]).
                        append(abonentUsrType[i].substring(abonentUsrType[i].lastIndexOf('_'))).toString();
                String[] results = request.getParameterValues(s);

                int priority = i + 1;
                for (int j = 0; j < results.length; j++) {
                    try {
                        abonentUsrList.add(new AbonentsPlacement(results[j], abonentUsrName[i], priority));
                    } catch (SibincoException e) {
                        e.printStackTrace();
                    }

                }
            }
            abonentUsr = (AbonentsPlacement[]) abonentUsrList.toArray(new AbonentsPlacement[abonentUsrList.size()]);
            abonentUsrList.clear();

            //**********************************************************************************//
            //                            DEFAULT SITE USR PLACE                                //
            //**********************************************************************************//
            List siteUsrrList = new ArrayList();
            for (int i = 0; i < siteUsrName.length; i++) {
                String s = new StringBuffer().append(SITE_USR_SELECT).append(siteUsrName[i]).append(siteUsrType[i].
                        substring(siteUsrType[i].lastIndexOf('_'))).toString();
                String[] results = request.getParameterValues(s);
                for (int j = 0; j < results.length; j++) {
                    try {
                        siteUsrrList.add(new SitePlacement(results[j], siteUsrName[i]));
                    } catch (SibincoException e) {
                        e.printStackTrace();
                    }
                }
            }
            siteUsr = (SitePlacement[]) siteUsrrList.toArray(new SitePlacement[siteUsrrList.size()]);
            siteAddrList.clear();
            saveOptions();
            super.process(request, response);
        }
    }

    protected void save() throws SCAGJspException {


        String messageText = "";

    }

    protected void saveOptions() throws SCAGJspException {
        final Option option = appContext.getHttpRoutingManager().getOptions();
        String messageText = "Options changed ";

        option.getAbonAddressPlace().clear();
        option.getAbonUsrPlace().clear();
        option.getSiteAddressPlace().clear();
        option.getSiteUsrPlace().clear();

        for (int i = 0; i < abonentAddress.length; i++) {
            option.getAbonAddressPlace().add(abonentAddress[i]);
        }
        for (int i = 0; i < abonentUsr.length; i++) {
            option.getAbonUsrPlace().add(abonentUsr[i]);
        }
        for (int i = 0; i < siteAddress.length; i++) {
            option.getSiteAddressPlace().add(siteAddress[i]);
        }
        for (int i = 0; i < siteUsr.length; i++) {
            option.getSiteUsrPlace().add(siteUsr[i]);
        }

        appContext.getHttpRoutingManager().setRoutesChanged(true);
        appContext.getHttpRoutingManager().setRoutesSaved(true);

        StatMessage message = new StatMessage(super.getLoginedPrincipal().getName(), "HTT Routes", messageText);
        appContext.getHttpRoutingManager().addStatMessages(message);
        StatusManager.getInstance().addStatMessages(message);

    }

    private AbonentsPlacement[] getAbonentsPlacements(List addressPlace) {
        return (AbonentsPlacement[]) addressPlace.toArray(new AbonentsPlacement[addressPlace.size()]);
    }

    private SitePlacement[] getSitePlacement(List addressPlace) {
        return (SitePlacement[]) addressPlace.toArray(new SitePlacement[addressPlace.size()]);
    }


    public String[] getOptionTypes() {
        return optionTypes;
    }

    public AbonentsPlacement[] getAbonentAddress() {
        return abonentAddress;
    }

    public void setAbonentAddress(AbonentsPlacement[] abonentAddress) {
        this.abonentAddress = abonentAddress;
    }

    public AbonentsPlacement[] getAbonentUsr() {
        return abonentUsr;
    }

    public void setAbonentUsr(AbonentsPlacement[] abonentUsr) {
        this.abonentUsr = abonentUsr;
    }

    public SitePlacement[] getSiteAddress() {
        return siteAddress;
    }

    public void setSiteAddress(SitePlacement[] siteAddress) {
        this.siteAddress = siteAddress;
    }

    public SitePlacement[] getSiteUsr() {
        return siteUsr;
    }

    public void setSiteUsr(SitePlacement[] siteUsr) {
        this.siteUsr = siteUsr;
    }

    public String[] getAbonAddrName() {
        return abonAddrName;
    }

    public void setAbonAddrName(String[] abonAddrName) {
        this.abonAddrName = abonAddrName;
    }

    public String[] getAbonAddrType() {
        return abonAddrType;
    }

    public void setAbonAddrType(String[] abonAddrType) {
        this.abonAddrType = abonAddrType;
    }

    public String[] getSiteAddrName() {
        return siteAddrName;
    }

    public void setSiteAddrName(String[] siteAddrName) {
        this.siteAddrName = siteAddrName;
    }

    public String[] getSiteAddrType() {
        return siteAddrType;
    }

    public void setSiteAddrType(String[] siteAddrType) {
        this.siteAddrType = siteAddrType;
    }

    public String[] getAbonentUsrName() {
        return abonentUsrName;
    }

    public void setAbonentUsrName(String[] abonentUsrName) {
        this.abonentUsrName = abonentUsrName;
    }

    public String[] getAbonentUsrType() {
        return abonentUsrType;
    }

    public void setAbonentUsrType(String[] abonentUsrType) {
        this.abonentUsrType = abonentUsrType;
    }

    public String[] getSiteUsrName() {
        return siteUsrName;
    }

    public void setSiteUsrName(String[] siteUsrName) {
        this.siteUsrName = siteUsrName;
    }

    public String[] getSiteUsrType() {
        return siteUsrType;
    }

    public void setSiteUsrType(String[] siteUsrType) {
        this.siteUsrType = siteUsrType;
    }
}
