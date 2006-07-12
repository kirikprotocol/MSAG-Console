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
        <sm-pm:item name="mbSave" value="routes.edit.item.mbsave.value" title="routes.edit.item.mbsave.title"/>
        <sm-pm:item name="mbCancel" value="routes.edit.item.mbcancel.value" title="routes.edit.item.mbcancel.title" onclick="clickCancel()"/>
        <sm-pm:space/>
    </sm-pm:menu>
</jsp:attribute>

<jsp:body>
<script src="content/scripts/http_routes.js" type="text/javascript"></script>
<table cellpadding="5">
    <col width="50%">
    <col width="50%">
    <tr>
        <td valign="top">
            <sm-ep:properties title="routes.edit.properties.route_info" noColoredLines="false">
                <sm-ep:txt title="routes.edit.txt.id" name="id" maxlength="60" readonly="${!bean.add}" validation="nonEmpty"/>
            </sm-ep:properties>
        </td>
        <td valign="top">
            <sm-ep:properties title="routes.edit.properties.route_options" noColoredLines="false" noHeaders="false">
                <sm-ep:check title="routes.edit.check.enabled" name="enabled"/>
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
        <tr><td style="cursor:hand;text-align:left"
                onClick="toggleVisible(pgs_http,cgs_http);">
        <div align="left" id="pgs_http" class="collapsing_list_closed"><div class="page_subtitle">
               <u><font size="2"><fmt:message>options.item.title.address.usr</fmt:message></font></u></div>
            </div></td>
            <tr id="cgs_http" style="display:none"><td colspan="9" width="100%">
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
                            <input type="text" size="50" style="color:black;" value="${pls.name}"
                                   readonly="true"/>
                            <input type="hidden" name="abonentUsrName" value="${pls.name}">
                            <input type="hidden" name="abonentUsrType" value="${pls.name}_${rowN}">
                        </td>

                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.abon_usr_tbl, 'abon_usr_place_${pls.name}_${rowN}');"
                                             style="cursor:hand;"></td>
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
                         style="cursor:hand;"></td>
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
                            <input type="text" size="50" style="color:black;" value="${pls.name}"
                                   readonly="true"/>
                            <input type="hidden" name="siteUsrName" value="${pls.name}">
                            <input type="hidden" name="siteUsrType" value="${pls.name}_${rowN}">
                        </td>
                            <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.site_usr_tbl, 'site_usr_place_${pls.name}_${rowN}');"
                                             style="cursor:hand;"></td>
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
                         onclick="addSiteDefaultUSRplace(opForm.all.new_site_usr, opForm.all.typeSelectUsrSite.options[typeSelectUsrSite.selectedIndex].value)"
                         style="cursor:hand;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>
    </tr>
    </table>
    <br>
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
            <td width="475">
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
                            <input type="text" size="50" style="color:black;" value="${pls.name}" readonly="true"/>
                            <input type="hidden" name="siteAddrName" value="${pls.name}">
                            <input type="hidden" name="siteAddrType" value="${pls.name}_${rowN}">
                        </td>
                            <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow(opForm.all.site_address_tbl, 'site_place_${pls.name}_${rowN}');"
                                             style="cursor:hand;"></td>
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
                                         style="cursor:hand;"></td>
                    <td>&nbsp;</td>
                </tr>
            </table>

        </td>
         </tr>
        </table>
<%--</sm-et:section>--%>
    </td></tr>
</table>
<br>
<table><tr><td colspan="2"><hr></td></tr></table>
<table cellpadding="12">
    <col width="42%">
    <col width="16%">
    <col width="42%">
    <tr>
        <td valign="top" width="50%">
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
                    <td><img src="content/images/but_add.gif" onclick="addAbonentSubj()" style="cursor:hend;"></td>
                </tr>
                <tr>
                    <td><fmt:message>routes.edit.address</fmt:message></td>
                    <td><input id="newAbonMask" class="txt" name="newAbonMask" validation="routeMask"
                               onkeyup="resetValidation(this)"></td>
                    <td><img src="content/images/but_add.gif" onclick="addAbonentMask(opForm.all.newAbonMask)"
                             alt="Add Http Subject" style="cursor: hand;"></td>
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
                    <td><img src="content/images/subject.gif" alt=""></td>
                    <td>${esubj}<input id="subjAbon" type="hidden" name="abonSubj" value="${esubj}"></td>
                    <td><img src="content/images/but_del.gif" alt="" onclick="removeAbonetSubj('subRow_${esubj}');"
                             style="cursor: hand;"></td>
                </tr>
                <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
                <c:forEach items="${bean.abonAddress}" var="i">
                    <c:set var="emask" value="${fn:escapeXml(i)}"/>
                    <tr class="row${rowN%2}" id="maskRow_${emask}">
                        <td><img src="content/images/mask.gif"></td>
                        <td>${emask}<input type="hidden" name="abonAddress" value="${emask}"></td>
                        <td><img src="content/images/but_del.gif"
                                 onClick="removeRow(opForm.all.abonents_table, 'maskRow_${emask}')"
                                 style="cursor: hand;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
            </table>
        </td>
        <td valign="top" width="50%">
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
                    <td><img src="content/images/but_add.gif" alt="HTTP Site List" onclick="addSiteSubj()" style="cursor:hand;"></td>
                </tr>
                <tr>
                    <td><fmt:message>routes.edit.site.port</fmt:message></td>

                    <td nowrap="true"><input id="newSite" class="midtxt" name="newSite"><b>:</b><input id="newPort" class="mintxt" name="newPort" value="80" validation="port" onkeyup="resetValidation(this)"></td>
                    <td><img src="content/images/but_add.gif" alt="Add new Site" onclick="addSite(opForm.all.newSite,opForm.all.newPort,'<fmt:message>routes.edit.add.new.path</fmt:message>')"
                        style="cursor: hand;"></td>
                </tr>
            </table>
            <hr>
            <table id="sites_table" class="properties_list" cellpadding="0" cellspacing="0">
                <col width="1%">
                <col width="99%">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.siteSubj}" var="i">
                    <c:set var="ssubj" value="${fn:escapeXml(i)}"/>
                    <tr class="row${rowN%3}" id="siteSubRow_${ssubj}">
                        <td><img src="content/images/subject.gif" alt=""><img src="content/images/subject.gif" alt=""></td>
                        <td>${ssubj}<input id="subSite" type="hidden" name="siteSubj" value="${ssubj}"></td>
                        <td><img src="content/images/but_del.gif" alt="" onclick="removeSiteSubj('siteSubRow_${ssubj}');"
                                style="cursor:hand;"></td>
                    </tr>
                    <c:set var="rowN" value="${rowN+1}"/>
                </c:forEach>
          </table>
            <sm-et:section title="Sites List" opened="true"  name="allsites">
                <table id="div_site_table" cellpadding="0" cellspacing="0" class="properties_list">
                    <tr><td>
                        <c:forEach items="${bean.sites}" var="i">
                            <c:set var="esite" value="${fn:escapeXml(i.host)}"/>
                            <c:set var="eport" value="${fn:escapeXml(i.port)}"/>
                            <c:set var="esite_sub" value="${fn:substringBefore(esite,'.')}"/>
                            <c:set var="sectHeader" value="sectionHeader_${esite_sub}"/>
                            <c:set var="esite_table" value="sitesp_table_${esite_sub}"/>

                            <div class="collapsing_tree_opened" id="${sectHeader}"
                                 onclick="collasping_tree_showhide_section('${esite_sub}')">

                                <table id="${esite_table}" cellpadding="0" cellspacing="0" class="properties_list">
                                    <col width="1%">
                                    <col width="99%">
                                    <tr>
                                        <td width="100%">${fn:escapeXml(i.host)}</td>
                                        <td align="left" nowrap="true"><input type="hidden" name="sitesHost"  id="${esite}"
                                                                              value="${esite}">
                                            <input type="hidden" name="sitesPort" value="${esite}_${eport}"></td>
                                        <td><img src="content/images/but_del.gif"
                                                 onClick="removeSection('${esite_sub}')"
                                                 style="cursor:hand;"></td>
                                    </tr>

                                    <tr>
                                        <td nowrap="true"><fmt:message>routes.edit.add.new.path</fmt:message>&nbsp;</td>
                                        <td align="right"><input id="${esite_sub}_newPath" class="txt"
                                                                 name="${esite_sub}_newPath"></td>
                                        <td><img src="content/images/but_add.gif" alt="Add new path"
                                                 onclick="addPath(opForm.all.${esite_sub}_newPath, '${esite}', opForm.all.sitesp_table_${esite_sub}, '${esite_sub}')"
                                                 style="cursor:hand;"></td>
                                    </tr>
                                    <c:set var="rowN" value="0"/>
                                    <c:forEach items="${i.pathLinks}" var="j">
                                        <c:set var="epath" value="${fn:escapeXml(j)}"/>
                                        <tr class="row${rowN%2}" id="pathRow_${esite_sub}_${epath}">
                                            <td></td>
                                            <td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;${epath}<input type="hidden"
                                                                                             name="pathLinks"
                                                                                             value="${esite}_${epath}">
                                            </td>
                                            <td><img src="content/images/but_del.gif"
                                                     onClick="removeRow(opForm.all.sitesp_table_${esite_sub}, 'pathRow_${esite_sub}_${epath}')"
                                                     style="cursor:hand;"></td>
                                        </tr>
                                        <c:set var="rowN" value="${rowN+1}"/>
                                    </c:forEach>
                                </table>
                            </div>
                        </c:forEach>
                    </td></tr>
                </table>
            </sm-et:section>
        </td>
    </tr>
</table>
</jsp:body>
</sm:page>