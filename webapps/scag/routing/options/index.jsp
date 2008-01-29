<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="options.index.title">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="options.item.mbsave.value" title="options.item.mbsave.title"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
    <script src="content/scripts/http_routes.js" type="text/javascript"></script>
    <table cellpadding="0" cellspacing="0" width="100%">
        <tr>
            <td align="left"><div class="page_subtitle_medium"><fmt:message>options.item.title.abonent.options</fmt:message></div></td>
            <td align="left"><div class="page_subtitle_medium"><fmt:message>options.item.title.site.options</fmt:message>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</div></td>
        </tr>
        <tr>
            <td align="left"><hr></td>
            <td align="left"><hr></td>
        </tr>
    </table>
    <br>
    <table cellpadding="0" cellspacing="0" width="100%">
        <tr>
            <td align="left"><div class="page_subtitle">Global Address place:</div></td>
            <td align="left"><div class="page_subtitle">Default Address place:</div></td>
        </tr>
    </table>
    <br>
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td valign="top">

        <table id="abon_address_tbl" class="properties_list" cellpadding="0" cellspacing="0">
            <c:set var="rowN" value="0"/>
            <c:forEach items="${bean.abonentAddress}" var="pls">
                <tr><td>
                <tr class="row${rowN%2}" id="address_place_${pls.name}_${rowN}">
                    <td width="7%">
                        <select id="addressSelect_${pls.name}_${rowN}" name="addressSelect_${pls.name}_${rowN}" class="txt">
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
                        <input type="hidden" name="abonAddrName" value="${pls.name}">
                        <input type="hidden" name="abonAddrType" value="${pls.name}_${rowN}">
                    </td>
                    <%--<td>Priority:&nbsp;${pls.priority}</td>--%>
                    <td width="65%"><img src="content/images/but_del.gif" alt="" onClick="removeRow('abon_address_tbl', 'address_place_${pls.name}_${rowN}');"
                                style="cursor:pointer;"></td>
                </tr>
                <c:set var="rowN" value="${rowN+1}"/>
            </c:forEach>
             </td></tr>
        </table>
        <!--<br>-->
        <hr width="50%" align="left"/>
        <table width="50%" class="properties_list" cellpadding="0" cellspacing="0">
            <col width="40%" align="left">
            <col width="50%" align="left">
            <col width="0%" align="left">
            <tr valign="top">
                <td width="7%"><select id="typeSelectAddress" name="typeSelectAddress" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="newAbonAddress" class="txt" size="50" name="newAbonAddress">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Global Address place"
                         onclick="addGlobAddressPlace('newAbonAddress', 'typeSelectAddress')"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td><td>&nbsp;</td>
        <td valign="top">
            <table id="site_address_tbl" class="properties_list" cellpadding="0" cellspacing="0">
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
                                             onClick="removeRow('site_address_tbl', 'site_place_${pls.name}_${rowN}');"
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
                                         onclick="addDefaultAddressPlace('new_site_address', 'typeSelectSite')"
                                         style="cursor:pointer;"></td>
                    <td>&nbsp;</td>
                </tr>
            </table>

        </td>
         </tr>
        </table>
    <!-- usr table -->
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td valign="top">

        <br>
        <br>
        <br>
        <div class="page_subtitle">Default USR place:</div>
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
                                             onClick="removeRow('abon_usr_tbl', 'abon_usr_place_${pls.name}_${rowN}');"
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
                         onclick="addAbonDefaultUSRplace('new_abon_usr', 'typeAbonSelectUsr')"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
    </td>
        <td valign="top">

        <br>
        <br>
        <br>
        <div class="page_subtitle">Default USR place:</div>
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
                                             onClick="removeRow('site_usr_tbl', 'site_usr_place_${pls.name}_${rowN}');"
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
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Site  USR"
                         onclick="addSiteDefaultUSRplace('new_site_usr', 'typeSelectUsrSite')"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>
    </tr>
    </table>
    <!-- route id table -->
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td valign="top">

        <br>
        <br>
        <br>
        <div class="page_subtitle">Global Route id place:</div>
        <br>
            <table id="abon_route_id_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.abonentRouteId}" var="pls">
                    <tr class="row${rowN%3}" id="abon_route_id_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="abonRouteIdSelect_${pls.name}_${rowN}"
                                               name="abonRouteIdSelect_${pls.name}_${rowN}" class="txt">
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
                            <input type="hidden" name="abonentRouteIdName" value="${pls.name}">
                            <input type="hidden" name="abonentRouteIdType" value="${pls.name}_${rowN}">
                        </td>

                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow('abon_route_id_tbl', 'abon_route_id_place_${pls.name}_${rowN}');"
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
                <td width="7%"><select id="typeAbonSelectRouteId" name="typeAbonSelectRouteId" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="new_abon_route_id" class="txt" size="50" name="new_abon_route_id">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Abonent Route Id"
                         onclick="addAbonDefaultRouteIdplace('new_abon_route_id', 'typeAbonSelectRouteId')"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
    </td>
        <td valign="top">

        <br>
        <br>
        <br>
        <div class="page_subtitle">Default Route id place:</div>
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
                                             onClick="removeRow('site_route_id_tbl', 'site_route_id_place_${pls.name}_${rowN}');"
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
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Site  Route id"
                         onclick="addSiteDefaultRouteIdplace('new_site_route_id', 'typeSelectRouteIdSite')"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>
    </tr>
    </table>
    <!-- service id table -->
    <table width="100%" cellpadding="0" cellspacing="0">
        <tr><td valign="top">

        <br>
        <br>
        <br>
        <div class="page_subtitle">Global Service id place:</div>
        <br>
            <table id="abon_service_id_tbl" class="properties_list" cellpadding="0" cellspacing="0">
                <c:set var="rowN" value="0"/>
                <c:forEach items="${bean.abonentServiceId}" var="pls">
                    <tr class="row${rowN%3}" id="abon_service_id_place_${pls.name}_${rowN}">
                        <td width="7%"><select id="abonServiceIdSelect_${pls.name}_${rowN}"
                                               name="abonServiceIdSelect_${pls.name}_${rowN}" class="txt">
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
                            <input type="hidden" name="abonentServiceIdName" value="${pls.name}">
                            <input type="hidden" name="abonentServiceIdType" value="${pls.name}_${rowN}">
                        </td>

                        <td width="65%"><img src="content/images/but_del.gif" alt=""
                                             onClick="removeRow('abon_service_id_tbl', 'abon_service_id_place_${pls.name}_${rowN}');"
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
                <td width="7%"><select id="typeAbonSelectServiceId" name="typeAbonSelectServiceId" class="txt">
                    <c:forEach items="${bean.optionTypes}" var="i">
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:forEach>
                </select>
                </td>
                <td width="25%">
                    <input id="new_abon_service_id" class="txt" size="50" name="new_abon_service_id">
                </td>
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Abonent Service Id"
                         onclick="addAbonDefaultServiceIdplace('new_abon_service_id', 'typeAbonSelectServiceId')"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
    </td>
        <td valign="top">

        <br>
        <br>
        <br>
        <div class="page_subtitle">Default Service id place:</div>
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
                                             onClick="removeRow('site_service_id_tbl', 'site_service_id_place_${pls.name}_${rowN}');"
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
                <td width="68%"><img src="content/images/but_add.gif" alt="Add new Site Service id"
                         onclick="addSiteDefaultServiceIdplace('new_site_service_id', 'typeSelectServiceIdSite')"
                         style="cursor:pointer;"></td>
                <td>&nbsp;</td>
            </tr>
        </table>
        </td>
    </tr>
    </table>

    </jsp:body>
</sm:page>