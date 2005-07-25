<%@include file="/WEB-INF/inc/header.jspf"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page title="Statistics">

  <jsp:attribute name="menu" ></jsp:attribute>
  <jsp:attribute name="menu2"></jsp:attribute>

  <jsp:body>
   <table>
   <tr><td valign="top">
    <sm-ep:properties title="Query parameters" noEdit="true">
      <c:choose>
        <c:when test="${bean.administrator}">
          <sm-ep:list  title="Sme provider" name="providerId"
                       values="${fn:join(bean.providerIds, ',')}"
                       valueTitles="${fn:join(bean.providerNames, ',')}"/>
        </c:when>
        <c:otherwise>
          <sm-ep:const title="Sme provider" name="providerName" value="${bean.providerName}"/>
        </c:otherwise>
      </c:choose>

      <sm-ep:calendar title="From date" name="fromDate"/>
      <sm-ep:calendar title="Till date" name="tillDate"/>
      </sm-ep:properties>
    </td>
    <td align="left">
    <sm-ep:properties title="SCAG Transport" noEdit="true">
    <sm-ep:check title="SMPP" name="transportName" />
    <sm-ep:check title="WAP" name="transportName" />
    <sm-ep:check title="MMS" name="transportName" />
     </sm-ep:properties>
    </td>
    </tr>

    </table>
    <sm:break>
      <sm-pm:menu>
        <sm-pm:item name="mbQuery"  value="Query" title="Query statistics"/>
        <sm-pm:space/>
      </sm-pm:menu>
    </sm:break>

    <script language="JavaScript">
    function toggleVisible(p, c)
    {
      var o = p.className == "collapsing_list_opened";
      p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
      c.runtimeStyle.display = o ? "none" : "block";
    }
    </script>

    <c:set var="statistics" value="${bean.statistics}"/>
    <c:if test="${!(empty statistics)}">
      <table class=properties_list cellspacing=0 cellspadding=0 border=0>
        <c:set var="disNo" value="${1}"/>
        <c:set var="total" value="${statistics.total}"/>
        <c:set var="byDates" value="${statistics.dateStat}"/>
        <tr><td colspan="9"><div class=page_subtitle>General statistics</div></td></tr>
        <tr>
            <th width="20%">&nbsp;</th><th width="7%">&nbsp;</th>
            <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
            <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
            <th width="15%" style="text-align:right">&nbsp;</th>
            <th width="15%" style="text-align:right">&nbsp;</th>
            <th width="15%" style="text-align:right">&nbsp;</th>
        </tr>
        <tr>
            <th width="20%" style="text-align:right">&nbsp;</th>
            <th width="7%"  style="text-align:right">Accepted</th>
            <th width="7%"  style="text-align:right">Rejected</th>
            <th width="7%"  style="text-align:right">Delivered</th>
            <th width="7%"  style="text-align:right">GW Rejected</th>
            <th width="7%"  style="text-align:right">Failed</th>
            <th width="7%" style="text-align:right">&nbsp;</th>
            <th width="15%" style="text-align:left">Bill OK/Fail</th>
            <th width="15%" style="text-align:right">Reciept OK/Failed</th>
        </tr>
        <tr class=row0>
            <th width="20%" style="text-align:right">Total SMS processed:</th>
            <th width="7%"  style="text-align:right">${total.accepted}</th>
            <th width="7%"  style="text-align:right">${total.rejected}</th>
            <th width="7%"  style="text-align:right">${total.delivered}</th>
            <th width="7%"  style="text-align:right">${total.gw_rejected}</th>
            <th width="7%"  style="text-align:right">${total.failed}</td>
            <th width="7%"  style="text-align:right">&nbsp;</th>
            <td width="15%"  style="text-align:left">${total.billingOk}/${total.billingFailed}</td>
            <th width="15%"  style="text-align:right">${total.recieptOk}/${total.recieptFailed}</th>
        </tr>
        <c:forEach var="dayStat" items="${byDates}" varStatus="i">
          <tr class=row0>
            <td style="cursor:hand;text-align:right"
                onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
              <div align=right id="p${disNo}" class="collapsing_list_closed"><fmt:formatDate value="${dayStat.date}" pattern="dd MMMM, yyyy"/></div>
            </td>
            <td style="text-align:right">${dayStat.accepted}</td>
            <td style="text-align:right">${dayStat.rejected}</td>
            <td style="text-align:right">${dayStat.delivered}</td>
            <td style="text-align:right">${dayStat.gw_rejected}</td>
            <td style="text-align:right">${dayStat.failed}</td>
            <th style="text-align:right">&nbsp;</th>
            <td style="text-align:left">${dayStat.billingOk}/${dayStat.billingFailed}</td>
            <th style="text-align:right">${dayStat.recieptOk}/${dayStat.recieptFailed}</th>
            <th style="text-align:right">&nbsp;</th>
          </tr>
          <tr id="c${disNo}" style="display:none">
            <td colspan="9" width="100%">
            <table class=list cellspacing=0 cellpadding=0 border=0 width="100%">
              <c:forEach var="hourStat" items="${dayStat.hourStat}">
                <tr class=row1>
                  <td width="20%" style="text-align:right"><fmt:formatNumber value="${hourStat.hour}" minIntegerDigits="2"/></td>
                  <td width="7%"  style="text-align:right">${hourStat.accepted}</td>
                  <td width="7%"  style="text-align:right">${hourStat.rejected}</td>
                  <td width="7%"  style="text-align:right">${hourStat.delivered}</td>
                  <td width="7%"  style="text-align:right">${hourStat.gw_rejected}</td>
                  <td width="7%"  style="text-align:right">${hourStat.failed}</td>
                  <th width="7%"  style="text-align:right">&nbsp;</th>
                  <td width="15%"  style="text-align:left">${hourStat.billingOk}/${hourStat.billingFailed}</td>
                  <th width="15%"  style="text-align:right">${hourStat.recieptOk}/${hourStat.recieptFailed}</th>
                </tr>
              </c:forEach>
            </table></td>
          </tr>
          <c:set var="disNo" value="${disNo+1}"/>
        </c:forEach>

        <c:set var="byRoute" value="${statistics.routeIdStat}"/>
        <c:if test="${!(empty byRoute)}">
          <tr><td colspan="9">&nbsp;</td></tr>
          <tr><td colspan="9"><div class=page_subtitle>Traffic by routes</div></td></tr>
          <tr>
              <th width="20%" style="text-align:right">Route Id</th>
              <th width="7%"  style="text-align:right">Accepted</th>
              <th width="7%"  style="text-align:right">Rejected</th>
              <th width="7%"  style="text-align:right">Delivered</th>
              <th width="7%"  style="text-align:right">GW Rejected</th>
              <th width="7%"  style="text-align:right">Failed</th>
              <th width="7%" style="text-align:right">&nbsp;</th>
              <th width="15%" style="text-align:left"> Bill OK/Fail</th>
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
              <td style="text-align:left">${routeStat.billingOk}/${routeStat.billingFailed}</td>
              <th style="text-align:right">${routeStat.recieptOk}/${routeStat.recieptFailed}</th>
              <th style="text-align:right">&nbsp;</th>
            </tr>
            <tr id="c${disNo}" style="display:none">
              <td colspan=9 width="100%">
              <table class=properties_list cellspacing=0 cellpadding=0 border=0>
                <tr class=row0>
                  <th width="27%" style="text-align:right">Error Code</th>
                  <th width="7%"  style="text-align:right">Count</th>
                  <th>&nbsp;</th>
                </tr>
                <c:forEach var="routeErr" items="${routeStat.errors}">
                  <tr class=row1>
                    <td style="text-align:right" nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.request.locale, routeErr.errcode, true))}</td>
                    <td style="text-align:right">${routeErr.counter}</td>
                    <td>&nbsp;</td>
                  </tr>
                </c:forEach>
              </table></td>
            </tr>
            <c:set var="disNo" value="${disNo+1}"/>
          </c:forEach>
        </c:if>

        <c:set var="bySme" value="${statistics.smeIdStat}"/>
        <c:if test="${!(empty bySme)}">
          <tr><td colspan="9">&nbsp;</td></tr>
          <tr><td colspan="9"><div class=page_subtitle>SME activity</div></td></tr>
          <tr>
              <th width="20%">&nbsp;</th><th width="7%">&nbsp;</th>
              <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
              <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
          </tr>
          <tr>
            <th width="20%" style="text-align:right">SME Id</th>
            <th width="7%"  style="text-align:right">Accepted</th>
            <th width="7%"  style="text-align:right">Rejected</th>
            <th width="7%"  style="text-align:right">Delivered</th>
            <th width="7%"  style="text-align:right">GW Rejected</th>
            <th width="7%"  style="text-align:right">Failed</th>
            <th width="15%" style="text-align:right">&nbsp;</th>
            <th width="15%" style="text-align:right">&nbsp;</th>
            <th width="15%" style="text-align:right">&nbsp;</th>
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
              <td style="text-align:right">&nbsp;</td>
              <td style="text-align:right">&nbsp;</td>
              <td style="text-align:right">&nbsp;</td>
            </tr>
            <tr id="c${disNo}" style="display:none">
              <td colspan="9" width="100%">
              <table class=properties_list cellspacing=0 cellpadding=0 border=0 width="100%">
                <tr class=row0>
                  <th width="27%" style="text-align:right">Error Code</th>
                  <th width="7%"  style="text-align:right">Count</th>
                  <th>&nbsp;</th>
                </tr>
                <c:forEach var="smeErr" items="${smeStat.errors}">
                  <tr class=row1>
                    <td style="text-align:right" nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.request.locale, smeErr.errcode, true))}</td>
                    <td style="text-align:right">${smeErr.counter}</td>
                    <td>&nbsp;</td>
                  </tr>
                </c:forEach>
              </table></td>
            </tr>
            <c:set var="disNo" value="${disNo+1}"/>
          </c:forEach>
        </c:if>

        <c:set var="bySmsc" value="${statistics.smscIdStat}"/>
        <c:if test="${!(empty bySmsc)}">
          <tr><td colspan=9>&nbsp;</td></tr>
          <tr><td colspan=9><div class=page_subtitle>SMSC activity</div></td></tr>
          <tr>
              <th width="20%">&nbsp;</th>
              <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
              <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
              <th width="7%">&nbsp;</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
          </tr>
          <tr>
              <th width="20%" style="text-align:right">SMSC Id</th>
              <th width="7%"  style="text-align:right">Accepted</th>
              <th width="7%"  style="text-align:right">Rejected</th>
              <th width="7%"  style="text-align:right">Delivered</th>
              <th width="7%"  style="text-align:right">GW Rejected</th>
              <th width="7%"  style="text-align:right">Failed</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
              <th width="15%" style="text-align:right">&nbsp;</th>
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
              <th style="text-align:right">&nbsp;</th>
              <th style="text-align:right">&nbsp;</th>
              <th style="text-align:right">&nbsp;</th>
            </tr>
            <tr id="c${disNo}" style="display:none">
              <td colspan="9" width="100%">
              <table class=properties_list cellspacing=0 cellpadding=0 border=0 width="100%">
                <tr class=row0>
                  <th width="27%" style="text-align:right">Error Code</th>
                  <th width="7%"  style="text-align:right">Count</th>
                  <th>&nbsp;</th>
                </tr>
                <c:forEach var="smscErr" items="${smscStat.errors}">
                  <tr class=row1>
                    <td style="text-align:right" nowrap>${fn:escapeXml(smf:getErrorMessage(pageContext.request.locale, smscErr.errcode, true))}</td>
                    <td style="text-align:right">${smscErr.counter}</td>
                    <td>&nbsp;</td>
                  </tr>
                </c:forEach>
              </table></td>
            </tr>
            <c:set var="disNo" value="${disNo+1}"/>
          </c:forEach>
        </c:if>
      </table>
    </c:if>
  </jsp:body>
</sm:page>