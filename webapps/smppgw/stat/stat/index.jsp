<%@include file="/WEB-INF/inc/header.jspf"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<sm:page title="Statistics">

  <jsp:attribute name="menu" ></jsp:attribute>
  <jsp:attribute name="menu2"></jsp:attribute>

  <jsp:body>

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
        <tr><td colspan=9><div class=page_subtitle>General statistics</div></td></tr>
        <tr>
            <th width="20%">&nbsp;</th>
            <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
            <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
            <th width="15%" style="text-align:right">SMS Trans</th>
            <th width="15%" style="text-align:right">USSD from SC</th>
            <th width="15%" style="text-align:right">USSD from SME</th>
        </tr>
        <tr>
            <th width="20%" style="text-align:right">&nbsp;</th>
            <th width="7%"  style="text-align:right">Acpt</th>
            <th width="7%"  style="text-align:right">Rejt</th>
            <th width="7%"  style="text-align:right">Dlvr</th>
            <th width="7%"  style="text-align:right">Temp</th>
            <th width="7%"  style="text-align:right">Perm</th>
            <th width="15%" style="text-align:right">Ok/Fail/Bill</th>
            <th width="15%" style="text-align:right">Ok/Fail/Bill</th>
            <th width="15%" style="text-align:right">Ok/Fail/Bill</th>
        </tr>
        <tr class=row0>
            <th width="20%" style="text-align:right">Total SMS processed:</th>
            <th width="7%"  style="text-align:right">${total.accepted}</th>
            <th width="7%"  style="text-align:right">${total.rejected}</th>
            <th width="7%"  style="text-align:right">${total.delivered}</th>
            <th width="7%"  style="text-align:right">${total.tempError}</th>
            <th width="7%"  style="text-align:right">${total.permError}</th>
            <th width="15%" style="text-align:right">${total.smsTrOk}/${total.smsTrFailed}/${total.smsTrBilled}</th>
            <th width="15%" style="text-align:right">${total.ussdTrFromScOk}/${total.ussdTrFromScFailed}/${total.ussdTrFromScBilled}</th>
            <th width="15%" style="text-align:right">${total.ussdTrFromSmeOk}/${total.ussdTrFromSmeFailed}/${total.ussdTrFromSmeBilled}</th>
        </tr>
        <c:forEach var="dayStat" items="${byDates}" varStatus="i">
          <tr class=row0>
            <td style="cursor:hand;text-align:right"
                onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
              <div align=right id="p${disNo}" class='collapsing_list_${((i.last) ? "opened":"closed")}'><fmt:formatDate value="${dayStat.date}" pattern="dd MMMM, yyyy"/></div>
            </td>
            <td style="text-align:right">${dayStat.accepted}</td>
            <td style="text-align:right">${dayStat.rejected}</td>
            <td style="text-align:right">${dayStat.delivered}</td>
            <td style="text-align:right">${dayStat.tempError}</td>
            <td style="text-align:right">${dayStat.permError}</td>
            <td style="text-align:right">${dayStat.smsTrOk}/${dayStat.smsTrFailed}/${dayStat.smsTrBilled}</td>
            <td style="text-align:right">${dayStat.ussdTrFromScOk}/${dayStat.ussdTrFromScFailed}/${dayStat.ussdTrFromScBilled}</td>
            <td style="text-align:right">${dayStat.ussdTrFromSmeOk}/${dayStat.ussdTrFromSmeFailed}/${dayStat.ussdTrFromSmeBilled}</td>
          </tr>
          <tr id="c${disNo}" style='display:${((i.last) ? "block":"none")}'>
            <td colspan=9 width="100%">
            <table class=list cellspacing=0 cellpadding=0 border=0 width="100%">
              <c:forEach var="hourStat" items="${dayStat.hourStat}">
                <tr class=row1>
                  <td width="20%" style="text-align:right"><fmt:formatNumber value="${hourStat.hour}" minIntegerDigits="2"/></td>
                  <td width="7%"  style="text-align:right">${hourStat.accepted}</td>
                  <td width="7%"  style="text-align:right">${hourStat.rejected}</td>
                  <td width="7%"  style="text-align:right">${hourStat.delivered}</td>
                  <td width="7%"  style="text-align:right">${hourStat.tempError}</td>
                  <td width="7%"  style="text-align:right">${hourStat.permError}</td>
                  <td width="15%" style="text-align:right">${hourStat.smsTrOk}/${hourStat.smsTrFailed}/${hourStat.smsTrBilled}</td>
                  <td width="15%" style="text-align:right">${hourStat.ussdTrFromScOk}/${hourStat.ussdTrFromScFailed}/${hourStat.ussdTrFromScBilled}</td>
                  <td width="15%" style="text-align:right">${hourStat.ussdTrFromSmeOk}/${hourStat.ussdTrFromSmeFailed}/${hourStat.ussdTrFromSmeBilled}</td>
                </tr>
              </c:forEach>
            </table></td>
          </tr>
          <c:set var="disNo" value="${disNo+1}"/>
        </c:forEach>

        <c:set var="bySme" value="${statistics.smeIdStat}"/>
        <c:if test="${!(empty bySme)}">
          <tr><td colspan=9>&nbsp;</td></tr>
          <tr><td colspan=9><div class=page_subtitle>SME activity</div></td></tr>
          <tr>
              <th width="20%">&nbsp;</th>
              <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
              <th width="7%">&nbsp;</th><th width="7%">&nbsp;</th><th width="7%">&nbsp;</th>
              <th width="15%" style="text-align:right">SMS Trans</th>
              <th width="15%" style="text-align:right">USSD from SC</th>
              <th width="15%" style="text-align:right">USSD from SME</th>
          </tr>
          <tr>
              <th width="20%" style="text-align:right">SME Id</th>
              <th width="7%"  style="text-align:right">Acpt</th>
              <th width="7%"  style="text-align:right">Rejt</th>
              <th width="7%"  style="text-align:right">Dlvr</th>
              <th width="7%"  style="text-align:right">Temp</th>
              <th width="7%"  style="text-align:right">Perm</th>
              <th width="15%" style="text-align:right">Ok/Fail/Bill</th>
              <th width="15%" style="text-align:right">Ok/Fail/Bill</th>
              <th width="15%" style="text-align:right">Ok/Fail/Bill</th>
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
              <td style="text-align:right">${smeStat.tempError}</td>
              <td style="text-align:right">${smeStat.permError}</td>
              <td style="text-align:right">${smeStat.smsTrOk}/${smeStat.smsTrFailed}/${smeStat.smsTrBilled}</td>
              <td style="text-align:right">${smeStat.ussdTrFromScOk}/${smeStat.ussdTrFromScFailed}/${smeStat.ussdTrFromScBilled}</td>
              <td style="text-align:right">${smeStat.ussdTrFromSmeOk}/${smeStat.ussdTrFromSmeFailed}/${smeStat.ussdTrFromSmeBilled}</td>
            </tr>
            <tr id="c${disNo}" style="display:none">
              <td colspan=9 width="100%">
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

        <c:set var="byRoute" value="${statistics.routeIdStat}"/>
        <c:if test="${!(empty byRoute)}">
          <tr><td colspan=9>&nbsp;</td></tr>
          <tr><td colspan=9><div class=page_subtitle>Traffic by routes</div></td></tr>
          <tr>
              <th width="20%" style="text-align:right">Route Id</th>
              <th width="7%"  style="text-align:right">Acpt</th>
              <th width="7%"  style="text-align:right">Rejt</th>
              <th width="7%"  style="text-align:right">Dlvr</th>
              <th width="7%"  style="text-align:right">Temp</th>
              <th width="7%"  style="text-align:right">Perm</th>
              <th width="15%">&nbsp;</th><th width="15%">&nbsp;</th><th width="15%">&nbsp;</th>
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
              <td style="text-align:right">${routeStat.tempError}</td>
              <td style="text-align:right">${routeStat.permError}</td>
              <td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>
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

      </table>
    </c:if>

  </jsp:body>

</sm:page>