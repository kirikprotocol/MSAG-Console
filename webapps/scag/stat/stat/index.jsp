<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@
        taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<sm:page title="Statistics" onLoad="populateArray();">

<jsp:attribute name="menu">
</jsp:attribute>
<jsp:attribute name="menu2">
</jsp:attribute>

<jsp:body>
<table>
    <tr>
        <td valign="top">
            <sm-ep:properties title="Query parameters" noEdit="true">
                <sm-ep:list title="Transport" name="transportId" values="${fn:join(bean.transportIds, ',')}"
                            valueTitles="${fn:join(bean.transportTitles, ',')}"/>
                    <c:choose>
                        <c:when test="${bean.administrator}">
                            <sm-ep:list title="Service provider(s)" name="providerId"
                                        values="${fn:join(bean.providerIds, ',')}"
                                        valueTitles="${fn:join(bean.providerNames, ',')}" onChange="providerChanged();"/>
                        </c:when>
                        <c:otherwise>
                            <sm-ep:const title="Service provider" name="providerName" value="${bean.providerName}"/>
                        </c:otherwise>
                    </c:choose>
                <sm-ep:calendar title="From date" name="fromDate"/>
            </sm-ep:properties>
        </td>
        <td valign="bottom">
           <sm-ep:properties title="" noEdit="true">
                <sm-ep:property title="">&nbsp;</sm-ep:property>
                            <sm-ep:list title="Service(s)" name="serviceId"
                                  values="${fn:join(bean.serviceIds,',')}"
                                       valueTitles="${fn:join(bean.serviceNames,',')}" onChange="serviceChanged();"/>
                <sm-ep:calendar title="Till date" name="tillDate"/>
            </sm-ep:properties>
         </td>
       </tr>
</table>
<sm:break>
    <sm-pm:menu>
        <sm-pm:item name="mbQuery" value="Query" title="Query statistics"/>
        <sm-pm:space/>
    </sm-pm:menu>
</sm:break>

<script language="JavaScript">
    var services;
    function toggleVisible(p, c) {
        var o = p.className == "collapsing_list_opened";
        p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
        c.runtimeStyle.display = o ? "none" : "block";
    }

    function populateArray() {
        services = new Array(opForm.all.serviceId.length*3);
        for (i=1;i<opForm.all.serviceId.length;i++) {
          curserv = opForm.all.serviceId.options[i].value;
          serviceproviderid = curserv.split('//');
          services[i*3] = serviceproviderid[0];
          services[i*3+1] = serviceproviderid[1];
          services[i*3+2] = opForm.all.serviceId.options[i].text;
        }
        selectedText = opForm.all.serviceId.options[opForm.all.serviceId.selectedIndex].text;
        providerChanged();
        for(i=1;i<opForm.all.serviceId.length;i++)
          if (selectedText == opForm.all.serviceId.options[i].text) opForm.all.serviceId.options[i].selected = true;
        //message = "";
        //for (i=0;i<services.length;i++) {
        //  message = message + 'services[' + i + ']=' + services[i] + ' | ' ;
        //}
        //alert(message);
    }

    function providerChanged() {
      for(i=opForm.all.serviceId.length;i>0;i--) opForm.all.serviceId.options[i]=null;
      opForm.all.serviceId.options[0] = new Option("ALL_SERVICES","-1",true,true);
      providerId = opForm.all.providerId.options[opForm.all.providerId.selectedIndex].value;
      j = 1;
      if (providerId == "-1") {
        for (i=1;i<services.length/3;i++) {
           opForm.all.serviceId.options[j] = new Option(services[i*3+2],services[i*3]);
           j=j+1;
         }
         return;
      }
      //alert('providerId = ' + providerId);
      for (i=1;i<services.length;i++) {
         if (services[i*3+1] == providerId) {
           //alert('services[' + (i*3+1) + ']=' + providerId);
           opForm.all.serviceId.options[j] = new Option(services[i*3+2],services[i*3]);
           j=j+1;
         }
      }
    }

      function serviceChanged() {
        serviceId = opForm.all.serviceId.options[opForm.all.serviceId.selectedIndex].value;
        //alert('serviceId = ' + serviceId);
        if (serviceId == "-1") {
          opForm.all.providerId.options[0].selected = true;
          return;
        }
        var prId;
        for(i=1;i<services.length/3;i++) {
             if(services[i*3] == serviceId) {
               prId = services[i*3+1];
               break;
             }
        }
        for(i=1;i<opForm.all.providerId.length;i++) {
           if (opForm.all.providerId.options[i].value == prId) {
              opForm.all.providerId.options[i].selected = true;
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
        <tr><td style="cursor:hand;text-align:right"
                onClick="toggleVisible(pgs_http,cgs_http);">
            <div align="left" id="pgs_http" class="collapsing_list_closed"><div class="page_subtitle">General
                statistics(for existent routes)</div>
            </div></td>
            <tr id="cgs_http" style="display:block"><td colspan="9" width="100%">
                <table>
                    <tr>
                        <th width="20%" style="text-align:right">&nbsp;</th>
                        <th width="10%" style="text-align:right">Requested</th>
                        <th width="13%" style="text-align:right">Request Rejected</th>
                        <th width="10%" style="text-align:right">Responsed</th>
                        <th width="13%" style="text-align:right">Response Rejected</th>
                        <th width="10%" style="text-align:right">Delivered</th>
                        <th width="7%" style="text-align:right">Failed</th>
                        <th style="text-align:right">&nbsp;</th>
                    </tr>
                    <tr class=row0>
                        <th width="20%" style="text-align:right">Total HTTP processed:</th>
                        <th width="10%" style="text-align:right">${total.request}</th>
                        <th width="13%" style="text-align:right">${total.requestRejected}</th>
                        <th width="10%" style="text-align:right">${total.response}</th>
                        <th width="13%" style="text-align:right">${total.responseRejected}</th>
                        <th width="10%" style="text-align:right">${total.delivered}</th>
                        <th width="7%" style="text-align:right">${total.failed}</th>
                        <th style="text-align:right">&nbsp;</th>
                    </tr>
                    <c:forEach var="dayStat" items="${byDates}" varStatus="i">
                        <tr class=row0>
                            <td style="cursor:hand;text-align:right"
                                onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
                                <div align=right id="p${disNo}" class="collapsing_list_closed"><fmt:formatDate
                                        value="${dayStat.date}" pattern="dd MMMM, yyyy"/></div>
                            </td>
                            <td style="text-align:right">${dayStat.request}</td>
                            <td style="text-align:right">${dayStat.requestRejected}</td>
                            <td style="text-align:right">${dayStat.response}</td>
                            <td style="text-align:right">${dayStat.responseRejected}</td>
                            <td style="text-align:right">${dayStat.delivered}</td>
                            <td style="text-align:right">${dayStat.failed}</td>
                            <td style="text-align:right">&nbsp;</td>
                        </tr>
                        <tr id="c${disNo}" style="display:none">
                            <td colspan="9" width="100%">
                                <table class=list cellspacing=0 cellpadding=0 border=0 width="100%">
                                    <c:forEach var="hourStat" items="${dayStat.hourStat}">
                                        <tr class=row1>
                                            <td width="20%" style="text-align:right"><fmt:formatNumber
                                                    value="${hourStat.hour}" minIntegerDigits="2"/></td>
                                            <td width="10%" style="text-align:right">${hourStat.request}</td>
                                            <td width="13%" style="text-align:right">${hourStat.requestRejected}</td>
                                            <td width="10%" style="text-align:right">${hourStat.response}</td>
                                            <td width="13%" style="text-align:right">${hourStat.responseRejected}</td>
                                            <td width="10%" style="text-align:right">${hourStat.delivered}</td>
                                            <td width="7%" style="text-align:right">${hourStat.failed}</td>
                                            <td style="text-align:right">&nbsp;</td>
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
    </table>
    <table class=properties_list cellspacing=0 cellspadding=0 border=0>
        <c:set var="byRoute" value="${httpStatistics.routeIdStat}"/>
        <c:if test="${!(empty byRoute)}">
        <tr><td colspan="9">&nbsp;</td></tr>
        <tr><td style="cursor:hand;text-align:right"
                onClick="toggleVisible(prout_http,crout_http);">
            <div align="left" id="prout_http" class="collapsing_list_closed"><div class="page_subtitle">Traffic by existent routes</div>
            </div></td>
            <tr id="crout_http" style="display:block"><td colspan="9">
                <table>
                    <tr>
                        <th width="20%" style="text-align:right">Route Id</th>
                        <th width="10%" style="text-align:right">Requested</th>
                        <th width="13%" style="text-align:right">Request Rejected</th>
                        <th width="10%" style="text-align:right">Responsed</th>
                        <th width="13%" style="text-align:right">Response Rejected</th>
                        <th width="10%" style="text-align:right">Delivered</th>
                        <th width="7%" style="text-align:right">Failed</th>
                        <th style="text-align:right">&nbsp;</th>
                    </tr>
                    <c:forEach var="routeStat" items="${byRoute}" varStatus="i">
                        <tr class=row0>
                            <td style="cursor:hand;text-align:right"
                                onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
                                <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(routeStat.routeid)}</div>
                            </td>
                            <td style="text-align:right">${routeStat.request}</td>
                            <td style="text-align:right">${routeStat.requestRejected}</td>
                            <td style="text-align:right">${routeStat.response}</td>
                            <td style="text-align:right">${routeStat.responseRejected}</td>
                            <td style="text-align:right">${routeStat.delivered}</td>
                            <td style="text-align:right">${routeStat.failed}</td>
                            <th style="text-align:right">&nbsp;</th>
                        </tr>
                        <tr id="c${disNo}" style="display:none">
                            <td colspan=9 width="100%">
                                <table class=properties_list cellspacing=0 cellpadding=0 border=0>
                                    <tr class=row0>
                                        <th width="27%" style="text-align:right">Error Code</th>
                                        <th width="7%" style="text-align:right">Count</th>
                                        <th>&nbsp;</th>
                                    </tr>
                                    <c:forEach var="routeErr" items="${routeStat.errors}">
                                        <tr class=row1>
                                            <td style="text-align:right"
                                                nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.request.locale, routeErr.errcode, true))}</td>
                                            <td style="text-align:right">${routeErr.counter}</td>
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
    <tr><td style="cursor:hand;text-align:right"
            onClick="toggleVisible(psme_http,csme_http);">
        <div align="left" id="psme_http" class="collapsing_list_closed"><div class="page_subtitle">URL activity</div></div>
    </td>
        <tr id="csme_http" style="display:block"><td colspan="9">
            <table>
                <tr>
                    <th width="20%" style="text-align:right">URL</th>
                    <th width="10%" style="text-align:right">Requested</th>
                    <th width="13%" style="text-align:right">Request Rejected</th>
                    <th width="10%" style="text-align:right">Responsed</th>
                    <th width="13%" style="text-align:right">Response Rejected</th>
                    <th width="10%" style="text-align:right">Delivered</th>
                    <th width="7%" style="text-align:right">Failed</th>
                    <th style="text-align:right">&nbsp;</th>
                </tr>
                <c:forEach var="smeStat" items="${bySme}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:hand;text-align:right"
                            onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
                            <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(smeStat.smeid)}</div>
                        </td>
                        <td style="text-align:right">${smeStat.request}</td>
                        <td style="text-align:right">${smeStat.requestRejected}</td>
                        <td style="text-align:right">${smeStat.response}</td>
                        <td style="text-align:right">${smeStat.responseRejected}</td>
                        <td style="text-align:right">${smeStat.delivered}</td>
                        <td style="text-align:right">${smeStat.failed}</td>
                        <td style="text-align:right">&nbsp;</td>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan="9" width="100%">
                            <table class=properties_list cellspacing=0 cellpadding=0 border=0 width="100%">
                                <tr class=row0>
                                    <th width="27%" style="text-align:right">Error Code</th>
                                    <th width="7%" style="text-align:right">Count</th>
                                    <th>&nbsp;</th>
                                </tr>
                                <c:forEach var="smeErr" items="${smeStat.errors}">
                                    <tr class=row1>
                                        <td style="text-align:right"
                                            nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.request.locale, smeErr.errcode, true))}</td>
                                        <td style="text-align:right">${smeErr.counter}</td>
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
<table class=properties_list cellspacing=0 cellspadding=0 border=0>
    <c:set var="disNo" value="${1}"/>
    <c:set var="total" value="${statistics.total}"/>
    <c:set var="byDates" value="${statistics.dateStat}"/>
    <tr><td colspan="9">&nbsp;</td></tr>
    <tr><td style="cursor:hand;text-align:right"
            onClick="toggleVisible(pgs,cgs);">
        <div align="left" id="pgs" class="collapsing_list_closed"><div class="page_subtitle">General statistics(for existent routes)</div>
        </div></td>
        <tr id="cgs" style="display:block"><td colspan="9" width="100%">
            <table border=0>
                <tr>
                    <th width="20%" style="text-align:right">&nbsp;</th>
                    <th width="8%" style="text-align:right">Accepted</th>
                    <th width="8%" style="text-align:right">Rejected</th>
                    <th width="8%" style="text-align:right">Delivered</th>
                    <th width="10%" style="text-align:right">GW Rejected</th>
                    <th width="8%" style="text-align:right">Failed</th>
                    <th width="7%" style="text-align:right">&nbsp;</th>
                    <th width="15%" style="text-align:right">Reciept OK/Failed</th>
                </tr>
                <tr class=row0>
                    <th width="20%" style="text-align:right">Total SMS processed:</th>
                    <th width="8%" style="text-align:right">${total.accepted}</th>
                    <th width="8%" style="text-align:right">${total.rejected}</th>
                    <th width="8%" style="text-align:right">${total.delivered}</th>
                    <th width="10%" style="text-align:right">${total.gw_rejected}</th>
                    <th width="8%" style="text-align:right">${total.failed}</th>
                    <th width="7%" style="text-align:right">&nbsp;</th>
                    <th width="15%" style="text-align:right">${total.recieptOk}/${total.recieptFailed}</th>
                    <th style="text-align:right">&nbsp;</th>
                </tr>
                <c:forEach var="dayStat" items="${byDates}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:hand;text-align:right"
                            onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
                            <div align=right id="p${disNo}" class="collapsing_list_closed"><fmt:formatDate
                                    value="${dayStat.date}" pattern="dd MMMM, yyyy"/></div>
                        </td>
                        <td style="text-align:right">${dayStat.accepted}</td>
                        <td style="text-align:right">${dayStat.rejected}</td>
                        <td style="text-align:right">${dayStat.delivered}</td>
                        <td style="text-align:right">${dayStat.gw_rejected}</td>
                        <td style="text-align:right">${dayStat.failed}</td>
                        <th style="text-align:right">&nbsp;</th>
                        <td style="text-align:right">${dayStat.recieptOk}/${dayStat.recieptFailed}</td>
                        <td style="text-align:right">&nbsp;</td>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan="9" width="100%">
                            <table class=list cellspacing=0 cellpadding=0 border=0 width="100%">
                                <c:forEach var="hourStat" items="${dayStat.hourStat}">
                                    <tr class=row1>
                                        <td width="20%" style="text-align:right"><fmt:formatNumber
                                                value="${hourStat.hour}" minIntegerDigits="2"/></td>
                                        <td width="8%" style="text-align:right">${hourStat.accepted}</td>
                                        <td width="8%" style="text-align:right">${hourStat.rejected}</td>
                                        <td width="8%" style="text-align:right">${hourStat.delivered}</td>
                                        <td width="10%" style="text-align:right">${hourStat.gw_rejected}</td>
                                        <td width="8%" style="text-align:right">${hourStat.failed}</td>
                                        <th width="7%" style="text-align:right">&nbsp;</th>
                                        <th width="15%"  style="text-align:right">${hourStat.recieptOk}/${hourStat.recieptFailed}</th>
                                        <th style="text-align:right">&nbsp;</th>
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
</table>
<table class=properties_list cellspacing=0 cellspadding=0 border=0>
    <c:set var="byRoute" value="${statistics.routeIdStat}"/>
    <c:if test="${!(empty byRoute)}">
    <tr><td colspan="9">&nbsp;</td></tr>
    <tr><td style="cursor:hand;text-align:right"
            onClick="toggleVisible(prout,crout);">
        <div align="left" id="prout" class="collapsing_list_closed"><div class="page_subtitle">Traffic by existent routes</div>
        </div></td>
        <tr id="crout" style="display:block"><td colspan="9">
            <table>
                <tr>
                    <th width="20%" style="text-align:right">Route Id</th>
                    <th width="8%" style="text-align:right">Accepted</th>
                    <th width="8%" style="text-align:right">Rejected</th>
                    <th width="8%" style="text-align:right">Delivered</th>
                    <th width="10%" style="text-align:right">GW Rejected</th>
                    <th width="8%" style="text-align:right">Failed</th>
                    <th width="7%" style="text-align:right">&nbsp;</th>                    
                    <th width="15%" style="text-align:right">Reciept OK/Failed</th>
                </tr>
                <c:forEach var="routeStat" items="${byRoute}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:hand;text-align:right"
                            onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
                            <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(routeStat.routeid)}</div>
                        </td>
                        <td style="text-align:right">${routeStat.accepted}</td>
                        <td style="text-align:right">${routeStat.rejected}</td>
                        <td style="text-align:right">${routeStat.delivered}</td>
                        <td style="text-align:right">${routeStat.gw_rejected}</td>
                        <td style="text-align:right">${routeStat.failed}</td>
                        <th style="text-align:right">&nbsp;</th>
                        <th style="text-align:right">${routeStat.recieptOk}/${routeStat.recieptFailed}</th>
                        <th style="text-align:right">&nbsp;</th>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan=9 width="100%">
                            <table class=properties_list cellspacing=0 cellpadding=0 border=0>
                                <tr class=row0>
                                    <th width="27%" style="text-align:right">Error Code</th>
                                    <th width="7%" style="text-align:right">Count</th>
                                    <th>&nbsp;</th>
                                </tr>
                                <c:forEach var="routeErr" items="${routeStat.errors}">
                                    <tr class=row1>
                                        <td style="text-align:right"
                                            nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.request.locale, routeErr.errcode, true))}</td>
                                        <td style="text-align:right">${routeErr.counter}</td>
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
    <tr><td style="cursor:hand;text-align:right"
            onClick="toggleVisible(psme,csme);">
        <div align="left" id="psme" class="collapsing_list_closed"><div class="page_subtitle">Existent SME activity</div></div>
    </td>
        <tr id="csme" style="display:block"><td colspan="9">
            <table>
                <tr>
                    <th width="20%" style="text-align:right">SME Id</th>
                    <th width="8%" style="text-align:right">Accepted</th>
                    <th width="8%" style="text-align:right">Rejected</th>
                    <th width="8%" style="text-align:right">Delivered</th>
                    <th width="10%" style="text-align:right">GW Rejected</th>
                    <th width="8%" style="text-align:right">Failed</th>
                    <th style="text-align:right">&nbsp;</th>
                </tr>
                <c:forEach var="smeStat" items="${bySme}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:hand;text-align:right"
                            onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
                            <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(smeStat.smeid)}</div>
                        </td>
                        <td style="text-align:right">${smeStat.accepted}</td>
                        <td style="text-align:right">${smeStat.rejected}</td>
                        <td style="text-align:right">${smeStat.delivered}</td>
                        <td style="text-align:right">${smeStat.gw_rejected}</td>
                        <td style="text-align:right">${smeStat.failed}</td>
                        <td style="text-align:right">&nbsp;</td>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan="9" width="100%">
                            <table class=properties_list cellspacing=0 cellpadding=0 border=0 width="100%">
                                <tr class=row0>
                                    <th width="27%" style="text-align:right">Error Code</th>
                                    <th width="7%" style="text-align:right">Count</th>
                                    <th>&nbsp;</th>
                                </tr>
                                <c:forEach var="smeErr" items="${smeStat.errors}">
                                    <tr class=row1>
                                        <td style="text-align:right"
                                            nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.request.locale, smeErr.errcode, true))}</td>
                                        <td style="text-align:right">${smeErr.counter}</td>
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
    <tr><td style="cursor:hand;text-align:right"
            onClick="toggleVisible(psmsc,csmsc);">
        <div align="left" id="psmsc" class="collapsing_list_closed"><div class="page_subtitle">Existent SMSC activity</div></div>
    </td>
        <tr id="csmsc" style="display:block"><td colspan="9" width="100%">
            <table>
                <tr>
                    <th width="20%" style="text-align:right">SMSC Id</th>
                    <th width="8%" style="text-align:right">Accepted</th>
                    <th width="8%" style="text-align:right">Rejected</th>
                    <th width="8%" style="text-align:right">Delivered</th>
                    <th width="10%" style="text-align:right">GW Rejected</th>
                    <th width="8%" style="text-align:right">Failed</th>
                    <th style="text-align:right">&nbsp;</th>
                </tr>
                <c:forEach var="smscStat" items="${bySmsc}" varStatus="i">
                    <tr class=row0>
                        <td style="cursor:hand;text-align:right"
                            onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
                            <div id="p${disNo}" class="collapsing_list_closed">${fn:escapeXml(smscStat.smscid)}</div>
                        </td>
                        <td style="text-align:right">${smscStat.accepted}</td>
                        <td style="text-align:right">${smscStat.rejected}</td>
                        <td style="text-align:right">${smscStat.delivered}</td>
                        <td style="text-align:right">${smscStat.gw_rejected}</td>
                        <td style="text-align:right">${smscStat.failed}</td>
                        <td style="text-align:right">&nbsp;</td>
                    </tr>
                    <tr id="c${disNo}" style="display:none">
                        <td colspan="9" width="100%">
                            <table class=properties_list cellspacing=0 cellpadding=0 border=0 width="100%">
                                <tr class=row0>
                                    <th width="27%" style="text-align:right">Error Code</th>
                                    <th width="7%" style="text-align:right">Count</th>
                                    <th>&nbsp;</th>
                                </tr>
                                <c:forEach var="smscErr" items="${smscStat.errors}">
                                    <tr class=row1>
                                        <td style="text-align:right"
                                            nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.request.locale, smscErr.errcode, true))}</td>
                                        <td style="text-align:right">${smscErr.counter}</td>
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