<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<sm:page onLoad="commonOnLoad()">
<jsp:attribute name="title">
    <c:choose>
        <c:when test="${param.add}">routes.add.title</c:when>
        <c:otherwise>routes.edit.title</c:otherwise>
    </c:choose>
</jsp:attribute>

<jsp:attribute name="menu">
    <sm-pm:menu>
        <sm-pm:item name="mbSave" value="routes.edit.item.mbsave.value" title="routes.edit.item.mbsave.title" onclick="return validateHttpRoute()"/>
        <sm-pm:item name="mbCancel" value="routes.edit.item.mbcancel.value" title="routes.edit.item.mbcancel.title" onclick="clickCancel()"/>
        <sm-pm:space/>
    </sm-pm:menu>
</jsp:attribute>

<jsp:body>
<script src="content/scripts/http_routes.jsp" type="text/javascript"></script>

<table cellpadding="5">
    <col width="50%">
    <col width="50%">
    <tr>
        <td valign="top">
            <sm-ep:properties title="routes.edit.properties.route_info" noColoredLines="false">
                <c:if test="${!param.add}">
                    <sm-ep:txt title="routes.edit.txt.id" name="id" readonly="true"/>
                </c:if>
                <sm-ep:txt title="routes.edit.txt.name" name="name" maxlength="60" validation="nonEmpty"/>
            </sm-ep:properties>
        </td>
        <td valign="top">
            <sm-ep:properties title="routes.edit.properties.route_options" noColoredLines="false" noHeaders="false">
                <sm-ep:check title="routes.edit.check.enabled" name="enabled"/>
                <sm-ep:check title="routes.edit.check.default" name="defaultRoute" disabled="true"/>
                <sm-ep:check title="routes.edit.check.transit" name="transit"/>
                <sm-pm:space/>
            </sm-ep:properties>
        </td>
    </tr>
    <tr><td colspan="2"><hr></td></tr>
</table>
<table cellpadding="0" cellspacing="0" width="100%">
    <tr>
        <td align="left"><div class="page_subtitle_medium"><fmt:message>options.item.title.abonents</fmt:message></div></td>
        <td align="left"><div class="page_subtitle_medium"><fmt:message>options.item.title.sites</fmt:message>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</div></td>
    </tr>
</table>
    <br>
    <table class="properties_list" cellspacing="0" cellspadding="0" border="0">
<%--        <tr><td style="cursor:hand;text-align:left"--%>
        <tr><td style="cursor:pointer;text-align:left"
                onClick="toggleVisible(pgs_http,cgs_http);">
        <div align="left" id="pgs_http" class="collapsing_list_closed"><div class="page_subtitle">
               <u><font size="2"><fmt:message>options.item.title.address.usr</fmt:message></font></u></div>
            </div></td>
            <tr id="cgs_http" style="display:none;"><td colspan="9" width="100%">
    <%--<sm-et:section title="Address/USR Placement Options" opened="false"  name="allplacement">--%>
    <table cellpadding="0" cellspacing="0" width="100%">
        <tr>
            <td align="left"><hr></td>
            <td align="left"><hr></td>
        </tr>
    </table>
    <br>
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td>
        <div class="page_subtitle">USR place:</div>
        <br>
            <table id="abon_usr_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.abonentUsr}" var="pls">
                    <tr class="row${rowN%3}" id="abon_usr_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="abonUsrSelect_${pls.name}_${rowN}"
                                               name="abonUsrSelect_${pls.name}_${rowN}" class="txt">
                            <c:forEach items="${bean.optionTypes}" var="i">
                                <c:choose>
                                    <c:when test="${pls.type==i}">
                                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                    </c:when>
                                    <c:otherwise>
                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                    </c:otherwise>
                                </c:choose>
                            </c:forEach>
                        </select>
                        </td>
                        <td>&nbsp;</td>
                            <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                        <td>
                            <input type="text" size="45" style="color:black;" value="${pls.name}"
                                   readonly="true"/>
                            <input type="hidden" name="abonentUsrName" value="${pls.name}">
                            <input type="hidden" name="abonentUsrType" value="${pls.name}_${rowN}">
                        </td>

                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.abon_usr_tbl, 'abon_usr_place_${pls.name}_${rowN}');"
                                             style="cursor:pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        <!--<br>-->
        <hr width="50%" align="left"/>
        <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <col width="40%" align="left">
            <col width="50%" align="left">
            <col width="0%" align="left">
            <tr valign="top">
                <td width="7%"><select id="typeAbonSelectUsr" name="typeAbonSelectUsr" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="new_abon_usr" class="txt" size="50" name="new_abon_usr">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Abonent USR"
                         onclick="addAbonDefaultUSRplace(opForm.all.new_abon_usr, opForm.all.typeAbonSelectUsr.options[typeAbonSelectUsr.selectedIndex].value)"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
    </td>
        <td width="473">
        <div class="page_subtitle">USR place:</div>
        <br>
            <table id="site_usr_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.siteUsr}" var="pls">
                    <tr class="row${rowN%3}" id="site_usr_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="siteUsrSelect_${pls.name}_${rowN}"
                                               name="siteUsrSelect_${pls.name}_${rowN}" class="txt">
                            <c:forEach items="${bean.optionTypes}" var="i">
                                <c:choose>
                                    <c:when test="${pls.type==i}">
                                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                    </c:when>
                                    <c:otherwise>
                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                    </c:otherwise>
                                </c:choose>
                            </c:forEach>
                        </select>
                        </td>
                        <td>&nbsp;</td>
                            <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                        <td>
                            <input type="text" size="45" style="color:black;" value="${pls.name}"
                                   readonly="true"/>
                            <input type="hidden" name="siteUsrName" value="${pls.name}">
                            <input type="hidden" name="siteUsrType" value="${pls.name}_${rowN}">
                        </td>
                            <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.site_usr_tbl, 'site_usr_place_${pls.name}_${rowN}');"
                                             style="cursor:pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        <!--<br>-->
        <hr width="50%" align="left"/>
        <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <col width="40%" align="left">
            <col width="50%" align="left">
            <col width="0%" align="left">
            <tr valign="top">
                <td width="7%"><select id="typeSelectUsrSite" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="new_site_usr" class="txt" size="50" name="new_site_usr">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Site"
<%--                         onclick="addSiteDefaultUSRplace(opForm.all.new_site_usr, opForm.all.typeSelectUsrSite.options[typeSelectUsrSite.selectedIndex].value)"--%>
                         onclick="addSiteDefaultUSRplace(opForm.all.new_site_usr, opForm.all.typeSelectUsrSite.options[typeSelectUsrSite.selectedIndex].value)"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>
    </tr>
    </table>
    <br>
    <!------------------------ address prefix| address place---------------->
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td>
        <div class="page_subtitle">&nbsp;</div>
        <br>
        <table  width="50%" id="abon_address_tbl" class="properties_list" cellpadding="0" cellspacing="0">
            <tr>
                <td>&nbsp;</td>
                <td valign="top" width="100%">
                    <sm-ep:properties title="routes.edit.prefix.address" noColoredLines="false" noHeaders="false">
                        <sm-ep:txt title="routes.edit.ton" name="ton" maxlength="1" styleWidth="20px" validation="port"/>
                        <sm-ep:txt title="routes.edit.npi" name="npi" maxlength="1" styleWidth="20px" validation="port"/>
                        <sm-pm:space/>
                    </sm-ep:properties>
                </td>
            </tr>
        </table>
            <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="40%" align="left">
                <col width="50%" align="left">
                <col width="0%" align="left">
                <tr valign="top">
                    <td width="25%">
                    </td>
                    <td width="68%">&nbsp;
                    </td>
                    <td>&nbsp;</td>
                </tr>
            </table>
        </td>
            <td width="475" valign="">
            <div class="page_subtitle">Address place:</div>
            <br>
            <table width="50%" id="site_address_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.siteAddress}" var="pls">
                    <tr class="row${rowN%3}" id="site_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="siteSelect_${pls.name}_${rowN}" name="siteSelect_${pls.name}_${rowN}" class="txt">
                            <c:forEach items="${bean.optionTypes}" var="i">
                                <c:choose>
                                    <c:when test="${pls.type==i}">
                                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                    </c:when>
                                    <c:otherwise>
                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                    </c:otherwise>
                                </c:choose>
                            </c:forEach>
                        </select>
                        </td>
                        <td>&nbsp;</td>
                            <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                        <td>
                            <input type="text" size="45" style="color:black;" value="${pls.name}" readonly="true"/>
                            <input type="hidden" name="siteAddrName" value="${pls.name}">
                            <input type="hidden" name="siteAddrType" value="${pls.name}_${rowN}">
                        </td>
                            <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.site_address_tbl, 'site_place_${pls.name}_${rowN}');"
                                             style="cursor:pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>

            </table>
            <!--<br>-->
            <hr width="50%" align="left"/>
            <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="40%" align="left">
                <col width="50%" align="left">
                <col width="0%" align="left">
                <tr valign="top">
                    <td width="7%"><select id="typeSelectSite" class="txt">
                        <c:forEach items="${bean.optionTypes}" var="i">
                            <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                        </c:forEach>
                    </select>
                    </td>
                    <td width="25%">
                        <input id="new_site_address" class="txt" size="50" name="new_site_address">
                    </td>
                    <td width="68%"><img src="content/images/but_add.gif" alt="Add new Default Address place"
                                         onclick="addDefaultAddressPlace(opForm.all.new_site_address, opForm.all.typeSelectSite.options[typeSelectSite.selectedIndex].value)"
                                         style="cursor:pointer;"></td>
                    <td>&nbsp;</td>
                </tr>
            </table>

        </td>
         </tr>
        </table>
<%--</sm-et:section>--%>
    <!-- empty | route id-->
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td>
        <div class="page_subtitle">&nbsp;</div>
        <br>
        <table  width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <tr>
                <td>&nbsp;</td>
                <td valign="top" width="100%">
                </td>
            </tr>
        </table>
            <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="40%" align="left">
                <col width="50%" align="left">
                <col width="0%" align="left">
                <tr valign="top">
                    <td width="25%">
                    </td>
                    <td width="68%">&nbsp;
                    </td>
                    <td>&nbsp;</td>
                </tr>
            </table>
        </td>
        <td width="473">
        <div class="page_subtitle">Route Id place:</div>
        <br>
            <table id="site_route_id_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.siteRouteId}" var="pls">
                    <tr class="row${rowN%3}" id="site_route_id_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="siteRouteIdSelect_${pls.name}_${rowN}"
                                               name="siteRouteIdSelect_${pls.name}_${rowN}" class="txt">
                            <c:forEach items="${bean.optionTypes}" var="i">
                                <c:choose>
                                    <c:when test="${pls.type==i}">
                                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                    </c:when>
                                    <c:otherwise>
                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                    </c:otherwise>
                                </c:choose>
                            </c:forEach>
                        </select>
                        </td>
                        <td>&nbsp;</td>
                            <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                        <td>
                            <input type="text" size="45" style="color:black;" value="${pls.name}"
                                   readonly="true"/>
                            <input type="hidden" name="siteRouteIdName" value="${pls.name}">
                            <input type="hidden" name="siteRouteIdType" value="${pls.name}_${rowN}">
                        </td>
                            <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.site_route_id_tbl, 'site_route_id_place_${pls.name}_${rowN}');"
                                             style="cursor:pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        <!--<br>-->
        <hr width="50%" align="left"/>
        <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <col width="40%" align="left">
            <col width="50%" align="left">
            <col width="0%" align="left">
            <tr valign="top">
                <td width="7%"><select id="typeSelectRouteIdSite" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="new_site_route_id" class="txt" size="50" name="new_site_route_id">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Site"
                         onclick="addSiteDefaultRouteIdplace(opForm.all.new_site_route_id, opForm.all.typeSelectRouteIdSite.options[typeSelectRouteIdSite.selectedIndex].value)"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>         </tr>
        </table>
    <!-- empty | route id-->
    <br>
    <!-- empty | service id-->
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td>
        <div class="page_subtitle">&nbsp;</div>
        <br>
        <table  width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <tr>
                <td>&nbsp;</td>
                <td valign="top" width="100%">
                </td>
            </tr>
        </table>
            <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="40%" align="left">
                <col width="50%" align="left">
                <col width="0%" align="left">
                <tr valign="top">
                    <td width="25%">
                    </td>
                    <td width="68%">&nbsp;
                    </td>
                    <td>&nbsp;</td>
                </tr>
            </table>
        </td>
        <td width="473">
        <div class="page_subtitle">Service Id place:</div>
        <br>
            <table id="site_service_id_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.siteServiceId}" var="pls">
                    <tr class="row${rowN%3}" id="site_service_id_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="siteServiceIdSelect_${pls.name}_${rowN}"
                                               name="siteServiceIdSelect_${pls.name}_${rowN}" class="txt">
                            <c:forEach items="${bean.optionTypes}" var="i">
                                <c:choose>
                                    <c:when test="${pls.type==i}">
                                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                                    </c:when>
                                    <c:otherwise>
                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                    </c:otherwise>
                                </c:choose>
                            </c:forEach>
                        </select>
                        </td>
                        <td>&nbsp;</td>
                            <%--<td width="43%">Priority:&nbsp;${pls.name}</td>--%>
                        <td>
                            <input type="text" size="45" style="color:black;" value="${pls.name}"
                                   readonly="true"/>
                            <input type="hidden" name="siteServiceIdName" value="${pls.name}">
                            <input type="hidden" name="siteServiceIdType" value="${pls.name}_${rowN}">
                        </td>
                            <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.site_service_id_tbl, 'site_service_id_place_${pls.name}_${rowN}');"
                                             style="cursor:pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        <!--<br>-->
        <hr width="50%" align="left"/>
        <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <col width="40%" align="left">
            <col width="50%" align="left">
            <col width="0%" align="left">
            <tr valign="top">
                <td width="7%"><select id="typeSelectServiceIdSite" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="new_site_service_id" class="txt" size="50" name="new_site_service_id">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Site"
                         onclick="addSiteDefaultServiceIdplace(opForm.all.new_site_service_id, opForm.all.typeSelectServiceIdSite.options[typeSelectServiceIdSite.selectedIndex].value)"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>         </tr>
        </table>
    <!-- empty | service id-->
    </td></tr>
</table>
<br>
<table><tr><td colspan="2"><hr></td></tr></table>
<table cellpadding="12">
    <col width="42%">
    <col width="16%">
    <col width="42%">
    <tr>
        <td valign="top" width="40%">
            <!--<div class="page_subtitle">Abonents</div>-->
            <table cellpadding="0" cellspacing="0">
                <col width="50%" align="left">
                <col width="50%" align="right">
                <col width="0%" align="left">
                <tr valign="middle">
                    <td><fmt:message>routes.edit.subject</fmt:message></td>
                    <td><select id="abonSubjSelect" class="txt">
                        <c:forEach items="${bean.allUncheckedAbonSubjects}" var="i">
                            <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                        </c:forEach>
                    </select>
                    </td>
                    <td><img src="content/images/but_add.gif" onclick="addAbonentSubj()" style="cursor:pointer;"></td>
                </tr>
                <tr>
                    <td><fmt:message>routes.edit.address</fmt:message></td>
                    <td><input id="newAbonMask" class="txt" name="newAbonMask" validation="routeMaskNonEmpty"
                               onkeyup="resetValidation(this)"></td>
<%--                    <td><img src="content/images/but_add.gif" onclick="addAbonentMask(opForm.all.newAbonMask)"--%>
                    <td><img src="content/images/but_add.gif" onclick="addAbonentMask('newAbonMask')"
                             alt="Add Http Subject" style="cursor: pointer;"></td>
                </tr>
            </table>
            <hr>
            <c:set var="rowN" value="0"/>
            <table id="abonents_table" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="1%">
                <col width="100%">
                <c:forEach items="${bean.abonSubj}" var="i">
                    <c:set var="esubj" value="${fn:escapeXml(i)}"/>
                    <tr class="row${rowN%3}" id="subjRow_${esubj}">
                        <td><img src="content/images/subject.gif" alt="">
                            <input id="subjAbon" type="hidden" name="abonSubj" value="${esubj}">
<%--                            <input id="subjAbon_${esubj}" type="hidden" name="abonSubj" value="${esubj}">--%>
                        </td>
                        <td id="td_subjAbon">${esubj}</td>
                        <td><img src="content/images/but_del.gif" alt="" onclick="removeAbonetSubj('subjRow_${esubj}', '${esubj}');"
                                 style="cursor: pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
                <c:forEach items="${bean.abonAddress}" var="i">
                    <c:set var="emask" value="${fn:escapeXml(i)}"/>
                    <tr class="row${rowN%2}" id="maskRow_${emask}">
                        <td><img src="content/images/mask.gif"><input type="hidden" name="abonAddress" value="${emask}"></td>
                        <td id="td_abonAddress">${emask}</td>
                        <td><img src="content/images/but_del.gif"
<%--                                 onClick="removeRow(opForm.all.abonents_table, 'maskRow_${emask}')"--%>
                                 onClick="removeRow('abonents_table', 'maskRow_${emask}')"
                                 style="cursor: pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        </td>
        <td valign="top" width="60%">
            <table cellpadding="0" cellspacing="0">
                 <col width="50%" align="left">
                 <col width="50%" align="right">
                 <col width="0%" align="left">
                <tr>
                    <td><fmt:message>routes.edit.site.subject</fmt:message></td>
                    <td>
                        <select id="siteSubjSelect" class="txt">
                            <c:forEach items="${bean.allUncheckedSiteSubject}" var="i">
                                <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                            </c:forEach>
                        </select>
                    </td>
                    <td><img src="content/images/but_add.gif" alt="HTTP Site List" onclick="addSiteSubj()" style="cursor:pointer;"></td>
                </tr>
                <tr>
                    <td><fmt:message>routes.edit.site.port</fmt:message></td>

                    <td nowrap="true"><input id="newSite" class="txt" name="newSite"><b>:</b><input id="newPort" class="mintxt" name="newPort" value="80" validation="port" onkeyup="resetValidation(this)"></td>
                    <td><img src="content/images/but_add.gif" alt="Add new Site"
<%--                        onclick="addSite(opForm.all.newSite,opForm.all.newPort,'<fmt:message>routes.edit.add.new.path</fmt:message>','<fmt:message>scripts.site.already.exist</fmt:message>','<fmt:message>scripts.site.name.error</fmt:message>')"--%>
                        onclick="addSite('newSite','newPort','<fmt:message>routes.edit.add.new.path</fmt:message>','<fmt:message>scripts.site.already.exist</fmt:message>','<fmt:message>scripts.site.name.error</fmt:message>')"
                        style="cursor: pointer;"></td>
                </tr>
            </table>
            <hr>
            <table id="sites_table" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="1%">
                <col width="99%">
                <c:set var="rowN" value="0"/>
                <c:set var="defaultSiteObjId" value="${fn:escapeXml(bean.defaultSiteObjId)}"/>
                <c:forEach items="${bean.siteSubj}" var="i">
                    <c:set var="ssubj" value="${fn:escapeXml(i)}"/>
                    <tr class="row${rowN%3}" id="siteSubRow_${ssubj}">
                        <td><input type="radio" name="defaultSiteObjId" <c:if test="${defaultSiteObjId==ssubj}">checked</c:if> value="${ssubj}"><img src="content/images/subject.gif" alt=""></td>
                        <td>${ssubj}<input id="subSite" type="hidden" name="siteSubj" value="${ssubj}"></td>
<%--                        <td><img src="content/images/but_del.gif" alt="" onclick="removeSiteSubj('siteSubRow_${ssubj}');"--%>
                        <td><img src="content/images/but_del.gif" alt="" onclick="removeSiteSubj('siteSubRow_${ssubj}', '${ssubj}');"
                                style="cursor:pointer;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
          </table>
          <sm:sites_list type="route" />
        </td>
    </tr>
</table>
</jsp:body>
</sm:page>