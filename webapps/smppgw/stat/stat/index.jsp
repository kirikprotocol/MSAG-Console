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
      <c:set var="total" value="${statistics.total}"/>
      <c:set var="byDates" value="${statistics.dateStat}"/>
      <table class=properties_list cellspacing=0 cellspadding=0>
        <tr class=row0>
            <td colspan=6><div class=page_subtitle>General statistics</div></td>
        </tr>
        <tr class=row1>
            <th width="25%"><div align=right>&nbsp;</div></th>
            <th width="15%"><div align=right>Accepted</div></th>
            <th width="15%"><div align=right>Rejected</div></th>
            <th width="15%"><div align=right>Delivered</div></th>
            <th width="15%"><div align=right>Temporal</div></th>
            <th width="15%"><div align=right>Permanent</div></th>
        </tr>
        <tr class=row0>
            <th width="25%"><div align=right>Total SMS processed:</div></th>
            <th width="15%"><div align=right>${total.accepted}</div></th>
            <th width="15%"><div align=right>${total.rejected}</div></th>
            <th width="15%"><div align=right>${total.delivered}</div></th>
            <th width="15%"><div align=right>${total.tempError}</div></th>
            <th width="15%"><div align=right>${total.permError}</div></th>
        </tr>
        <c:set var="disNo" value="${1}"/>
        <c:forEach var="dayStat" items="${byDates}" varStatus="i">
          <tr class=row0>
            <td width="25%" align=right style="cursor:hand"
                onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
              <div id="p${disNo}" class='collapsing_list_${((i.last) ? "opened":"closed")}'>
                <fmt:formatDate value="${dayStat.date}" pattern="dd MMMM, yyyy"/>
              <div>
            </td>
            <td width="15%"><div align=right>${dayStat.accepted}</div></td>
            <td width="15%"><div align=right>${dayStat.rejected}</div></td>
            <td width="15%"><div align=right>${dayStat.delivered}</div></td>
            <td width="15%"><div align=right>${dayStat.tempError}</div></td>
            <td width="15%"><div align=right>${dayStat.permError}</div></td>
          </tr>
          <tr id="c${disNo}" style='display:${((i.last) ? "block":"none")}'>
            <td colspan=6><table class=list cellspacing=0 cellpadding=0 border=0>
              <c:forEach var="hourStat" items="${dayStat.hourStat}">
                <tr class=row1>
                  <td width="25%" align=right>&nbsp;&nbsp;&nbsp;${((hourStat.hour < 10) ? ("0"+hourStat.hour):(""+hourStat.hour))}</td>
                  <td width="15%"><div align=right>${hourStat.accepted}</div></td>
                  <td width="15%"><div align=right>${hourStat.rejected}</div></td>
                  <td width="15%"><div align=right>${hourStat.delivered}</div></td>
                  <td width="15%"><div align=right>${hourStat.tempError}</div></td>
                  <td width="15%"><div align=right>${hourStat.permError}</div></td>
                </tr>
              </c:forEach>
            </table></td>
          </tr>
          <c:set var="disNo" value="${disNo+1}"/>
        </c:forEach>

        <c:set var="bySme" value="${statistics.smeIdStat}"/>
        <tr>
            <td colspan=6><div class=page_subtitle>SME activity</div></td>
        </tr>
        <tr class=row1>
            <th width="25%"><div align=right>SME Id</div></th>
            <th width="15%"><div align=right>Accepted</div></th>
            <th width="15%"><div align=right>Rejected</div></th>
            <th width="15%"><div align=right>Delivered</div></th>
            <th width="15%"><div align=right>Temporal</div></th>
            <th width="15%"><div align=right>Permanent</div></th>
        </tr>
        <c:forEach var="smeStat" items="${bySme}" varStatus="i">
          <tr class=row0>
            <td width="25%" align=right style="cursor:hand"
                onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
              <div id="p${disNo}" class="collapsing_list_closed">
                ${fn:escapeXml(smeStat.smeid)}
              <div>
            </td>
            <td width="15%"><div align=right>${smeStat.accepted}</div></td>
            <td width="15%"><div align=right>${smeStat.rejected}</div></td>
            <td width="15%"><div align=right>${smeStat.delivered}</div></td>
            <td width="15%"><div align=right>${smeStat.tempError}</div></td>
            <td width="15%"><div align=right>${smeStat.permError}</div></td>
          </tr>
          <tr id="c${disNo}" style="display:none">
            <td colspan=6><table class=list cellspacing=0 cellpadding=0 border=0>
              <tr class=row0>
                <th width="25%"><div align=right>Errors</div></th>
                <th width="15%">&nbsp;</th><th width="15%">&nbsp;</th>
                <th width="15%">&nbsp;</th><th width="15%">&nbsp;</th><th width="15%">&nbsp;</th>
              </tr>
              <c:forEach var="smeErr" items="${smeStat.errors}">
                <tr class=row1>
                  <!-- TODO: Obtain errcode string -->
                  <td><div align=right>${smeErr.errcode}</div></td>
                  <td><div align=right>${smeErr.counter}</div></td>
                  <td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>
                </tr>
              </c:forEach>
              <tr class=row0>
                <th width="25%"><div align=right>Transactions</div></th>
                <th width="15%">&nbsp;</th><th width="15%">&nbsp;</th>
                <th width="15%">&nbsp;</th><th width="15%">&nbsp;</th><th width="15%">&nbsp;</th>
              </tr>
              <tr class=row1>
                <td><div align=right>TrOk</div></td>
                <td><div align=right>45895</div></td>
                <td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>
              </tr>
              <tr class=row1>
                <td><div align=right>TrFailed</div></td>
                <td><div align=right>495</div></td>
                <td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>
              </tr>
              <!-- TODO: Add transaction counters -->
            </table></td>
          </tr>
          <c:set var="disNo" value="${disNo+1}"/>
        </c:forEach>

        <c:set var="byRoute" value="${statistics.routeIdStat}"/>
        <tr>
            <td colspan=8> <div class=page_subtitle>Traffic by routes</div></td>
        </tr>
        <tr class=row0>
            <th width="25%"><div align=right>Route Id</div></th>
            <th width="15%"><div align=right>Accepted</div></th>
            <th width="15%"><div align=right>Rejected</div></th>
            <th width="15%"><div align=right>Delivered</div></th>
            <th width="15%"><div align=right>Temporal</div></th>
            <th width="15%"><div align=right>Permanent</div></th>
        </tr>
        <c:forEach var="routeStat" items="${byRoute}" varStatus="i">
          <tr class=row0>
            <td width="25%" align=right style="cursor:hand"
                onClick="toggleVisible(opForm.all.p${disNo}, opForm.all.c${disNo});">
              <div id="p${disNo}" class="collapsing_list_closed">
                ${fn:escapeXml(routeStat.routeid)}
              <div>
            </td>
            <td width="15%"><div align=right>${routeStat.accepted}</div></td>
            <td width="15%"><div align=right>${routeStat.rejected}</div></td>
            <td width="15%"><div align=right>${routeStat.delivered}</div></td>
            <td width="15%"><div align=right>${routeStat.tempError}</div></td>
            <td width="15%"><div align=right>${routeStat.permError}</div></td>
          </tr>
          <tr id="c${disNo}" style="display:none">
            <td colspan=6><table class=list cellspacing=0 cellpadding=0 border=0>
              <tr class=row0>
                <th width="25%"><div align=right>Errors</div></th>
                <th width="15%">&nbsp;</th><th width="15%">&nbsp;</th>
                <th width="15%">&nbsp;</th><th width="15%">&nbsp;</th><th width="15%">&nbsp;</th>
              </tr>
              <c:forEach var="routeErr" items="${routeStat.errors}">
                <tr class=row1>
                  <!-- TODO: Obtain errcode string -->
                  <td><div align=right>${routeErr.errcode}</div></td>
                  <td><div align=right>${routeErr.counter}</div></td>
                  <td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>
                </tr>
              </c:forEach>
            </table></td>
          </tr>
          <c:set var="disNo" value="${disNo+1}"/>
        </c:forEach>

      </table>
    </c:if>

  </jsp:body>

</sm:page>