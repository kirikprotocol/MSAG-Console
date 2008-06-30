<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<sm:page title="statistics.title" onLoad="populateArray();">

<jsp:attribute name="menu">
</jsp:attribute>
<jsp:attribute name="menu2">
</jsp:attribute>

<jsp:body>
<table>
    <tr>
        <td valign="top">
            <sm-ep:properties title="statistics.properties" noEdit="true">
                <sm-ep:list title="statistics.list.transport" name="transportId" values="${fn:join(bean.transportIds, ',')}"
                            valueTitles="${fn:join(bean.transportTitles, ',')}"/>
                    <c:choose>
                        <c:when test="${bean.administrator}">
                            <sm-ep:list title="statistics.list.service_providers" name="providerId"
                                        values="${fn:join(bean.providerIds, ',')}"
                                        valueTitles="${fn:join(bean.providerNames, '~')}" onChange="providerChanged();"/>
                        </c:when>
                        <c:otherwise>
                            <sm-ep:const title="statistics.const.service_provider" name="providerName" value="${bean.providerName}"/>
                        </c:otherwise>
                    </c:choose>
                <sm-ep:calendar title="statistics.calendar.fromdate" name="fromDate"/>
            </sm-ep:properties>
        </td>
        <td valign="bottom">
           <sm-ep:properties title="" noEdit="true">
                <sm-ep:property title="">&nbsp;</sm-ep:property>
                            <sm-ep:list title="statistics.list.services" name="serviceId"
                                  values="${fn:join(bean.serviceIds,',')}"
                                       valueTitles="${fn:join(bean.serviceNames,'~')}" onChange="serviceChanged();"/>
                <sm-ep:calendar title="statistics.calendar.tilldate" name="tillDate"/>
            </sm-ep:properties>
        </td>
    </tr>
</table>

<sm:break>
    <sm-pm:menu>
        <sm-pm:item name="mbQuery" value="statistics.item.value" title="statistics.item.title"/>
        <sm-pm:space/>
    </sm-pm:menu>
</sm:break>

<script language="JavaScript">
    var servicesHash;
    var providersHash;

    function toggleVisible(p, c) {
        var p = getElementByIdUni(p);
        var c = getElementByIdUni(c);
        var o = p.className == "collapsing_list_opened";
//        p.className = (p.className == "collapsing_list_opened") ? "collapsing_list_closed"  : "collapsing_list_opened";
        p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
        c.style.display = o ? "none" : "";
    }

//    function toggleVisible_(p, c) {
//        var p = getElementByIdUni(p);
//        var c = getElementByIdUni(c);
//        var o = p.className == "collapsing_list_opened";
//        p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
////        c.runtimeStyle.display = o ? "none" : "block";
//        c.style.display = o ? "none" : "";
//    }

    function populateArray() {
        servicesHash = new Array();
        providersHash = new Array();
        for (var i=1;i<getElementByIdUni("serviceId").length;i++) {
          curserv = getElementByIdUni("serviceId").options[i].value;
          serviceproviderid = curserv.split('//');
          var providerId = serviceproviderid[1];
          var serviceId = serviceproviderid[0];
          providersHash[serviceId] = providerId;
          if (servicesHash[providerId]) {
            array = servicesHash[providerId];
            array[array.length] = {value:serviceId, text:getElementByIdUni("serviceId").options[i].text};
            servicesHash[providerId] = array;
          } else {
            servicesHash[providerId] = [{value:serviceId, text:getElementByIdUni("serviceId").options[i].text}];
          }
        }
        selectedText = getElementByIdUni("serviceId").options[getElementByIdUni("serviceId").selectedIndex].text;
        providerChanged();
        for(var i=1;i<serviceId.length;i++){
          if (selectedText == getElementByIdUni("serviceId").options[i].text) getElementByIdUni("serviceId").options[i].selected = true;
        }
    }

    function providerChanged() {
      var serviceId = getElementByIdUni("serviceId");
      serviceId.options.length=0;
      serviceId.options[0] = new Option("ALL_SERVICES","-1",true,true);
      var providerId = getElementByIdUni("providerId").options[getElementByIdUni("providerId").selectedIndex].value
      var j = 1;
      if (providerId == "-1") {
         for (var i=1;i<servicesHash.length;i++) {
           var array = servicesHash[i];
           if (array && array.length && array.length>0)
           for (var y=0;y<array.length;y++) {
             serviceId.options[j] = new Option(array[y].text, array[y].value);
             j=j+1;
           }
         }
         return;
      }
      var services = servicesHash[providerId];
      if (services && services.length && services.length>0)
      for (var i=0;i<services.length;i++) {
           serviceId.options[j] = new Option(services[i].text, services[i].value);
           j=j+1;
      }
    }

    function serviceChanged() {
      var serviceId = getElementByIdUni("serviceId").options[getElementByIdUni("serviceId").selectedIndex].value;
      if (serviceId == "-1") return;
      var prId = providersHash[serviceId];
      if (getElementByIdUni("providerId").options[getElementByIdUni("providerId").selectedIndex].value == prId) return;
      for(i=1;i<getElementByIdUni("providerId").length;i++) {
         if (getElementByIdUni("providerId").options[i].value == prId) {
            getElementByIdUni("providerId").options[i].selected = true;
            return;
         }
      }
    }
</script>

<c:set var="statistics" value="${bean.statistics}"/>
<c:set var="httpStatistics" value="${bean.httpStatistics}"/>

<c:if test="${!(empty httpStatistics)}">
    <table class=properties_list cellspacing=0 cellspadding=0 border=0>
        <c:set var="disNo" value="${1}"/>
        <c:set var="total" value="${httpStatistics.total}"/>
        <c:set var="byDates" value="${httpStatistics.dateStat}"/>
        <tr><td colspan="9">&nbsp;</td></tr>
        <tr><td style="cursor:pointer;text-align:left"
                onClick="toggleVisible('pgs_http','cgs_http');">
            <div align="left" id="pgs_http" class="collapsing_list_opened"><div class="page_subtitle">
               <fmt:message>statistics.label.general</fmt:message></div>
            </div></td></tr>
            <tr id="cgs_http"><td colspan="9" width="100%">
                <table>
                    <tr>
                        <th width="27%" style="text-align:left">&nbsp;</th>
                        <th width="10%" style="text-align:left">Requested</th>
                        <th width="13%" style="text-align:left">Request Rejected</th>
                        <th width="10%" style="text-align:left">Responsed</th>
                        <th width="13%" style="text-align:left">Response Rejected</th>
                        <th width="10%" style="text-align:left">Delivered</th>
                        <th width="7%" style="text-align:left">Failed</th>
                        <th style="text-align:left">&nbsp;</th>
                    </tr>
                    <tr class=row0>
                        <th width="27%" style="text-align:left">Total HTTP processed:</th>
                        <th width="10%" style="text-align:left">${total.request}</th>
                        <th width="13%" style="text-align:left">${total.requestRejected}</th>
                        <th width="10%" style="text-align:left">${total.response}</th>
                        <th width="13%" style="text-align:left">${total.responseRejected}</th>
                        <th width="10%" style="text-align:left">${total.delivered}</th>
                        <th width="7%" style="text-align:left">${total.failed}</th>
                        <th style="text-align:left">&nbsp;</th>
                    </tr>
                    <c:forEach var="dayStat" items="${byDates}" varStatus="i">
                        <tr class=row0>
                            <td style="cursor:pointer;text-align:left"
                                onClick="toggleVisible('p${disNo}', 'c${disNo}');">
                                <div align=left id="p${disNo}" class="collapsing_list_closed"><fmt:formatDate
                                        value="${dayStat.date}" pattern="dd MMMM, yyyy"/></div>
                            </td>
                            <td style="text-align:left">${dayStat.request}</td>
                            <td style="text-align:left">${dayStat.requestRejected}</td>
                            <td style="text-align:left">${dayStat.response}</td>
                            <td style="text-align:left">${dayStat.responseRejected}</td>
                            <td style="text-align:left">${dayStat.delivered}</td>
                            <td style="text-align:left">${dayStat.failed}</td>
                            <td style="text-align:left">&nbsp;</td>
                        </tr>
                        <tr id="c${disNo}" style="display:none">
                            <td colspan="9" width="100%">
                                <table class=list cellspacing=0 cellpadding=0 border=0 width="100%">
                                    <c:forEach var="hourStat" items="${dayStat.hourStat}">
                                        <tr class=row1>
                                            <td width="27%" style="text-align:left">
                                                <div align=left style="padding-left:87px"><fmt:formatNumber
                                                    value="${hourStat.hour}" minIntegerDigits="2"/></div></td>
                                            <td width="10%" style="text-align:left">${hourStat.request}</td>
                                            <td width="13%" style="text-align:left">${hourStat.requestRejected}</td>
                                            <td width="10%" style="text-align:left">${hourStat.response}</td>
                                            <td width="13%" style="text-align:left">${hourStat.responseRejected}</td>
                                            <td width="10%" style="text-align:left">${hourStat.delivered}</td>
                                            <td width="7%" style="text-align:left">${hourStat.failed}</td>
                                            <td style="text-align:left">&nbsp;</td>
                                        </tr>
                                    </c:forEach>
                                </table>
                            </td>
                        </tr>
                        <c:set var="disNo" value="${disNo+1}"/>
                    </c:forEach>
                </table>
            </td></tr>
<%--        </tr>--%>
    </table>
    <table class=properties_list cellspacing=0 cellspadding=0 border=0>
        <c:set var="byRoute" value="${httpStatistics.routeIdStat}"/>
        <c:if test="${!(empty byRoute)}">
            <tr><td colspan="9">&nbsp;</td></tr>
            <tr><td style="cursor:pointer;text-align:left"
                    onClick="toggleVisible('prout_http','crout_http');">
                <div align="left" id="prout_http" class="collapsing_list_opened">
                   <div class="page_subtitle"><fmt:message>statistics.label.trafic_by_routes</fmt:message></div>
                </div></td>
                <tr id="crout_http"><td colspan="9">
                    <table>
                        <tr>
                            <th width="27%" style="text-align:left">Route Id</th>
                            <th width="10%" style="text-align:left">Requested</th>
                            <th width="13%" style="text-align:left">Request Rejected</th>
                            <th width="10%" style="text-align:left">Responsed</th>
                            <th width="13%" style="text-align:left">Response Rejected</th>
                            <th width="10%" style="text-align:left">Delivered</th>
                            <th width="7%" style="text-align:left">Failed</th>
                            <th style="text-align:left">&nbsp;</th>
                        </tr>
                        <c:forEach var="routeStat" items="${byRoute}" varStatus="i">
                            <tr class=row0>
                                <td style="cursor:pointer;text-align:left"
                                    onClick="toggleVisible('p${disNo}', 'c${disNo}');">
                                    <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(smf:getHttpRouteNameById(pageContext.request,routeStat.routeid))}</div>
                                </td>
                                <td style="text-align:left">${routeStat.request}</td>
                                <td style="text-align:left">${routeStat.requestRejected}</td>
                                <td style="text-align:left">${routeStat.response}</td>
                                <td style="text-align:left">${routeStat.responseRejected}</td>
                                <td style="text-align:left">${routeStat.delivered}</td>
                                <td style="text-align:left">${routeStat.failed}</td>
                                <th style="text-align:left">&nbsp;</th>
                            </tr>
                            <tr id="c${disNo}" style="display:none">
                                <td colspan=9 width="100%">
                                    <table class=properties_list cellspacing=0 cellpadding=0 border=0>
                                        <tr class=row0>
                                            <th width="28px">&nbsp;</th>
                                            <th width="27%" style="text-align:left">Error Code</th>
                                            <th width="7%" style="text-align:left">Count</th>
                                            <th>&nbsp;</th>
                                        </tr>
                                        <c:forEach var="routeErr" items="${routeStat.errors}">
                                            <tr class=row1>
                                                <th width="28px">&nbsp;</th>
                                                <td style="text-align:left"
                                                    nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.session,routeErr.errcode, true , "http"))}</td>
                                                <td style="text-align:left">${routeErr.counter}</td>
                                                <td>&nbsp;</td>
                                            </tr>
                                        </c:forEach>
                                    </table>
                                </td>
                            </tr>
                            <c:set var="disNo" value="${disNo+1}"/>
                        </c:forEach>
                     </table>
                </td></tr>
            </tr>
      </c:if>
    </table>
    <table class=properties_list cellspacing=0 cellspadding=0 border=0>
    <c:set var="bySme" value="${httpStatistics.smeIdStat}"/>
    <c:if test="${!(empty bySme)}">
        <tr><td colspan="9">&nbsp;</td></tr>
        <tr><td style="cursor:pointer;text-align:left"
                onClick="toggleVisible('psme_http','csme_http');">
            <div align="left" id="psme_http" class="collapsing_list_opened">
              <div class="page_subtitle"><fmt:message>statistics.label.url_activity</fmt:message></div>
            </div>
        </td>
            <tr id="csme_http"><td colspan="9">
                <table>
                    <tr>
                        <th width="27%" style="text-align:left">URL</th>
                        <th width="10%" style="text-align:left">Requested</th>
                        <th width="13%" style="text-align:left">Request Rejected</th>
                        <th width="10%" style="text-align:left">Responsed</th>
                        <th width="13%" style="text-align:left">Response Rejected</th>
                        <th width="10%" style="text-align:left">Delivered</th>
                        <th width="7%" style="text-align:left">Failed</th>
                        <th style="text-align:left">&nbsp;</th>
                    </tr>
                    <c:forEach var="smeStat" items="${bySme}" varStatus="i">
                        <tr class=row0>
                            <td style="cursor:pointer;text-align:left"
                                onClick="toggleVisible('p${disNo}', 'c${disNo}');">
                                <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(smeStat.smeid)}</div>
                            </td>
                            <td style="text-align:left">${smeStat.request}</td>
                            <td style="text-align:left">${smeStat.requestRejected}</td>
                            <td style="text-align:left">${smeStat.response}</td>
                            <td style="text-align:left">${smeStat.responseRejected}</td>
                            <td style="text-align:left">${smeStat.delivered}</td>
                            <td style="text-align:left">${smeStat.failed}</td>
                            <td style="text-align:left">&nbsp;</td>
                        </tr>
                        <tr id="c${disNo}" style="display:none">
                            <td colspan="9" width="100%">
                                <table class=properties_list cellspacing=0 cellpadding=0 border=0 width="100%">
                                    <tr class=row0>
                                        <th width="28px">&nbsp;</th>
                                        <th width="27%" style="text-align:left">Error Code</th>
                                        <th width="7%" style="text-align:left">Count</th>
                                        <th>&nbsp;</th>
                                    </tr>
                                    <c:forEach var="smeErr" items="${smeStat.errors}">
                                        <tr class=row1>
                                            <th width="28px">&nbsp;</th>
                                            <td style="text-align:left"
                                                nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.session,smeErr.errcode, true, "http"))}</td>
                                            <td style="text-align:left">${smeErr.counter}</td>
                                            <td>&nbsp;</td>
                                        </tr>
                                    </c:forEach>
                                </table>
                            </td>
                        </tr>
                        <c:set var="disNo" value="${disNo+1}"/>
                    </c:forEach>
                </table>
            </td></tr>
        </tr>
  </c:if>
</table>
</c:if>

<c:if test="${!(empty statistics)}">
<table class=properties_list cellspacing=0 cellspadding=0 border=0 style="">
    <c:set var="disNo" value="${1}"/>
    <c:set var="total" value="${statistics.total}"/>
    <c:set var="byDates" value="${statistics.dateStat}"/>
    <tr><td colspan="9">&nbsp;</td></tr>
    <tr><td style="cursor:pointer;text-align:left"
            onClick="toggleVisible('pgs','cgs');">
        <div align="left" id="pgs" class="collapsing_list_opened"><div class="page_subtitle">
           <fmt:message>statistics.label.general</fmt:message></div>
        </div></td></tr>
        <tr id="cgs">
          <td colspan="9" width="100%">
            <table border=0>
                <tr>
                    <th width="27%" style="text-align:left">&nbsp;</th>
                    <th width="8%" style="text-align:left">Accepted</th>
                    <th width="8%" style="text-align:left">Rejected</th>
                    <th width="8%" style="text-align:left">Delivered</th>
                    <th width="10%" style="text-align:left">GW Rejected</th>
                    <th width="8%" style="text-align:left">Failed</th>
<%--                    <th width="7%" style="text-align:left">&nbsp;</th>--%>
                    <th width="15%" style="text-align:left">Reciept OK/Failed</th>
                    <th style="text-align:left">&nbsp;</th>
                </tr>
                <tr class=row0>
                    <th width="27%" style="text-align:left">Total SMS processed:</th>
                    <th width="8%" style="text-align:left">${total.accepted}</th>
                    <th width="8%" style="text-align:left">${total.rejected}</th>
                    <th width="8%" style="text-align:left">${total.delivered}</th>
                    <th width="10%" style="text-align:left">${total.gw_rejected}</th>
                    <th width="8%" style="text-align:left">${total.failed}</th>
<%--                    <th width="7%" style="text-align:left">&nbsp;</th>--%>
                    <th width="15%" style="text-align:left">${total.recieptOk}/${total.recieptFailed}</th>
                    <th style="text-align:left">&nbsp;</th>
                </tr>
                <c:forEach var="dayStat" items="${byDates}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:pointer;text-align:right"
                            onClick="toggleVisible('p${disNo}', 'c${disNo}');">
                            <div align=left id="p${disNo}" class="collapsing_list_closed">
                                <fmt:formatDate value="${dayStat.date}" pattern="dd MMMM, yyyy"/>
                            </div>
                        </td>
                        <td style="text-align:left">${dayStat.accepted}</td>
                        <td style="text-align:left">${dayStat.rejected}</td>
                        <td style="text-align:left">${dayStat.delivered}</td>
                        <td style="text-align:left">${dayStat.gw_rejected}</td>
                        <td style="text-align:left">${dayStat.failed}</td>
<%--                        <th style="text-align:left">&nbsp;</th>--%>
                        <td style="text-align:left">${dayStat.recieptOk}/${dayStat.recieptFailed}</td>
                        <td style="text-align:left">&nbsp;</td>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan="9" width="100%" id="c${disNo}_td">
                            <table class=list cellspacing=0 cellpadding=0 border=0 width="100%" id="c${disNo}_table">
                                <c:forEach var="hourStat" items="${dayStat.hourStat}">
                                    <tr class=row1>
                                        <td width="27%" style="text-align:right">
                                             <div align=left style="padding-left:87px"><fmt:formatNumber
                                                value="${hourStat.hour}" minIntegerDigits="2"/> </div></td>
                                        <td width="8%" style="text-align:left">${hourStat.accepted}</td>
                                        <td width="8%" style="text-align:left">${hourStat.rejected}</td>
                                        <td width="8%" style="text-align:left">${hourStat.delivered}</td>
                                        <td width="10%" style="text-align:left">${hourStat.gw_rejected}</td>
                                        <td width="8%" style="text-align:left">${hourStat.failed}</td>
<%--                                        <th width="7%" style="text-align:left">&nbsp;</th>--%>
                                        <th width="15%"  style="text-align:left">${hourStat.recieptOk}/${hourStat.recieptFailed}</th>
                                        <th style="text-align:left">&nbsp;</th>
                                    </tr>
                                </c:forEach>
                            </table>
                        </td>
                    </tr>
                    <c:set var="disNo" value="${disNo+1}"/>
                </c:forEach>
            </table>
        </td></tr>
</table>
<table class=properties_list cellspacing=0 cellspadding=0 border=0>
    <c:set var="byRoute" value="${statistics.routeIdStat}"/>
    <c:if test="${!(empty byRoute)}">
    <tr><td colspan="9">&nbsp;</td></tr>
    <tr><td style="cursor:pointer;text-align:left"
            onClick="toggleVisible(prout,crout);">
        <div align="left" id="prout" class="collapsing_list_opened">
          <div class="page_subtitle"><fmt:message>statistics.label.trafic_by_routes</fmt:message></div>
        </div></td>
        <tr id="crout"><td colspan="9">
            <table>
                <tr>
                    <th width="27%" style="text-align:left">Route Id</th>
                    <th width="8%" style="text-align:left">Accepted</th>
                    <th width="8%" style="text-align:left">Rejected</th>
                    <th width="8%" style="text-align:left">Delivered</th>
                    <th width="10%" style="text-align:left">GW Rejected</th>
                    <th width="8%" style="text-align:left">Failed</th>
<%--                    <th width="7%" style="text-align:left">&nbsp;</th>--%>
                    <th width="15%" style="text-align:left">Reciept OK/Failed</th>
                </tr>
                <c:forEach var="routeStat" items="${byRoute}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:pointer;text-align:left"
                            onClick="toggleVisible('p${disNo}', 'c${disNo}');">
                            <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(routeStat.routeid)}</div>
                        </td>
                        <td style="text-align:left">${routeStat.accepted}</td>
                        <td style="text-align:left">${routeStat.rejected}</td>
                        <td style="text-align:left">${routeStat.delivered}</td>
                        <td style="text-align:left">${routeStat.gw_rejected}</td>
                        <td style="text-align:left">${routeStat.failed}</td>
                        <th style="text-align:left">${routeStat.recieptOk}/${routeStat.recieptFailed}</th>
                        <th style="text-align:left">&nbsp;</th>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan=9 width="100%">
                            <table class=properties_list cellspacing=0 cellpadding=0 border=0>
                                <tr class=row0>
                                    <th width="28px">&nbsp;</th>
                                    <th width="27%" style="text-align:left">Error Code</th>
                                    <th width="7%" style="text-align:left">Count</th>
                                    <th>&nbsp;</th>
                                </tr>
                                <c:forEach var="routeErr" items="${routeStat.errors}">
                                    <tr class=row1>
                                        <th width="28px">&nbsp;</th>
                                        <td style="text-align:left"
                                            nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.session,routeErr.errcode, true , "smpp"))}</td>
                                        <td style="text-align:left">${routeErr.counter}</td>
                                        <td>&nbsp;</td>
                                    </tr>
                                </c:forEach>
                            </table>
                        </td>
                    </tr>
                    <c:set var="disNo" value="${disNo+1}"/>
                </c:forEach>
            </table>
        </td></tr>
    </tr>
    </c:if>
</table>
<table class=properties_list cellspacing=0 cellspadding=0 border=0>
    <c:set var="bySme" value="${statistics.smeIdStat}"/>
    <c:if test="${!(empty bySme)}">
    <tr><td colspan="9">&nbsp;</td></tr>
    <tr><td style="cursor:pointer;text-align:left"
            onClick="toggleVisible(psme,csme);">
        <div align="left" id="psme" class="collapsing_list_opened">
          <div class="page_subtitle"><fmt:message>statistics.label.sme_activity</fmt:message></div>
        </div>
    </td>
        <tr id="csme"><td colspan="9">
            <table>
                <tr>
                    <th width="27%" style="text-align:left">SME Id</th>
                    <th width="8%" style="text-align:left">Accepted</th>
                    <th width="8%" style="text-align:left">Rejected</th>
                    <th width="8%" style="text-align:left">Delivered</th>
                    <th width="10%" style="text-align:left">GW Rejected</th>
                    <th width="8%" style="text-align:left">Failed</th>
                    <th style="text-align:left">&nbsp;</th>
                </tr>
                <c:forEach var="smeStat" items="${bySme}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:pointer;text-align:left"
                            onClick="toggleVisible('p${disNo}', 'c${disNo}');">
                            <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(smeStat.smeid)}</div>
                        </td>
                        <td style="text-align:left">${smeStat.accepted}</td>
                        <td style="text-align:left">${smeStat.rejected}</td>
                        <td style="text-align:left">${smeStat.delivered}</td>
                        <td style="text-align:left">${smeStat.gw_rejected}</td>
                        <td style="text-align:left">${smeStat.failed}</td>
                        <td style="text-align:left">&nbsp;</td>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan="9" width="100%">
                            <table class=properties_list cellspacing=0 cellpadding=0 border=0 width="100%">
                                <tr class=row0>
                                    <th width="28px">&nbsp;</th>
                                    <th width="27%" style="text-align:left">Error Code</th>
                                    <th width="7%" style="text-align:left">Count</th>
                                    <th>&nbsp;</th>
                                </tr>
                                <c:forEach var="smeErr" items="${smeStat.errors}">
                                    <tr class=row1>
                                        <th width="28px">&nbsp;</th>
                                        <td style="text-align:left"
                                            nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.session,smeErr.errcode, true, "smpp"))}</td>
                                        <td style="text-align:left">${smeErr.counter}</td>
                                        <td>&nbsp;</td>
                                    </tr>
                                </c:forEach>
                            </table>
                        </td>
                    </tr>
                    <c:set var="disNo" value="${disNo+1}"/>
                </c:forEach>
            </table>
        </td></tr>
    </tr>
  </c:if>
</table>
<table class=properties_list cellspacing=0 cellspadding=0 border=0>
    <c:set var="bySmsc" value="${statistics.smscIdStat}"/>
    <c:if test="${!(empty bySmsc)}">
    <tr><td colspan=9>&nbsp;</td></tr>
    <tr><td style="cursor:pointer;text-align:left"
            onClick="toggleVisible(psmsc,csmsc);">
        <div align="left" id="psmsc" class="collapsing_list_opened">
          <div class="page_subtitle"><fmt:message>statistics.label.smsc_activity</fmt:message></div>
        </div>
    </td>
        <tr id="csmsc" ><td colspan="9" width="100%">
            <table>
                <tr>
                    <th width="27%" style="text-align:left">SMSC Id</th>
                    <th width="8%" style="text-align:left">Accepted</th>
                    <th width="8%" style="text-align:left">Rejected</th>
                    <th width="8%" style="text-align:left">Delivered</th>
                    <th width="10%" style="text-align:left">GW Rejected</th>
                    <th width="8%" style="text-align:left">Failed</th>
                    <th style="text-align:left">&nbsp;</th>
                </tr>
                <c:forEach var="smscStat" items="${bySmsc}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:pointer;text-align:left"
<%--                            onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">--%>
                            onClick="toggleVisible('p${disNo}', 'c${disNo}');">
                            <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(smscStat.smscid)}</div>
                        </td>
                        <td style="text-align:left">${smscStat.accepted}</td>
                        <td style="text-align:left">${smscStat.rejected}</td>
                        <td style="text-align:left">${smscStat.delivered}</td>
                        <td style="text-align:left">${smscStat.gw_rejected}</td>
                        <td style="text-align:left">${smscStat.failed}</td>
                        <td style="text-align:left">&nbsp;</td>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan="9" width="100%">
                            <table class=properties_list cellspacing=0 cellpadding=0 border=0 width="100%">
                                <tr class=row0>
                                    <th width="28px">&nbsp;</th>
                                    <th width="27%" style="text-align:left">Error Code</th>
                                    <th width="7%" style="text-align:left">Count</th>
                                    <th>&nbsp;</th>
                                </tr>
                                <c:forEach var="smscErr" items="${smscStat.errors}">
                                    <tr class=row1>
                                        <th width="28px">&nbsp;</th>
                                        <td style="text-align:left"
                                            nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.session,smscErr.errcode, true , "smpp"))}</td>
                                        <td style="text-align:left">${smscErr.counter}</td>
                                        <td>&nbsp;</td>
                                    </tr>
                                </c:forEach>
                            </table>
                        </td>
                    </tr>
                    <c:set var="disNo" value="${disNo+1}"/>
                </c:forEach>
            </table>
        </td></tr>
    </tr>
  </c:if>
</table>
</c:if>
</jsp:body>
</sm:page>