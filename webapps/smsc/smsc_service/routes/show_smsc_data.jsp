<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ include file="utils.jsp" %>
<h4>Routes: SMEs</h4>
<table class="list" cellspacing="0">
  <thead class="list">
    <tr class="list">
      <th class="list">ID</th>
    </tr>
  </thead>
  <tbody>
  <%
    {
      SMEList smes = smsc.getSmes();
      for (Iterator i = smes.iterator(); i.hasNext(); )
      {
        SME sme = (SME) i.next();
        %><tr class="list">
          <td class="list"><%=StringEncoderDecoder.encode(sme.getId())%></td>
        </tr><%
      }
    }
  %>
  </tbody>
</table>
<h4>Routes: Routes</h4>
<table class="list" cellspacing="0">
  <thead class="list">
    <tr class="list">
      <th class="list" rowspan="2">Route&nbsp;ID</th>
      <th class="list" rowspan="2">B</th>
      <th class="list" rowspan="2">A</th>
      <th class="list" colspan="3">Sources</th>
      <th class="list" colspan="4">Destinations</th>
    </tr>
    <tr class="list">
      <th class="list">ID</th>
      <th class="list">isSubject</th>
      <th class="list">Masks</th>
      <th class="list">ID</th>
      <th class="list">isSubject</th>
      <th class="list">SME</th>
      <th class="list">Masks</th>
    </tr>
  </thead>
  <tbody><%
  {
    RouteList routes = smsc.getRoutes();
    for (Iterator i = routes.iterator(); i.hasNext(); )
    {
      Route route = (Route) i.next();
      int rows = route.getSources().size() > route.getDestinations().size() ? route.getSources().size() : route.getDestinations().size();
      Iterator srcIter = route.getSources().iterator();
      Iterator dstIter = route.getDestinations().iterator();
      boolean isNamesShowed = false;
      while (!isNamesShowed || srcIter.hasNext() || dstIter.hasNext()) {
        %><tr class="list"><%
        if (!isNamesShowed) {%>
          <td class="list" rowspan="<%=rows%>" style="color: <%=route.isEnabling() ? "Green" : "Red"%>;"><%=StringEncoderDecoder.encode(route.getName())%></td>
          <td class="list" rowspan="<%=rows%>"><%=route.isBilling()%></td>
          <td class="list" rowspan="<%=rows%>"><%=route.isArchiving()%></td><%
          isNamesShowed = true;
        }
        if (srcIter.hasNext()) {
          Source source = (Source) srcIter.next();%>
          <td class="list"><%=StringEncoderDecoder.encode(source.getName())%></td>
          <td class="list"><%=source.isSubject()%></td>
          <td class="list">
            <table border="1" cellspacing="0"><%
              for (Iterator k = source.getMasks().iterator(); k.hasNext(); )
              {
                Mask mask = (Mask) k.next();
                %><tr><td><%=mask.getMask()%></td></tr><%
              }%>
            </table>
          </td><%
        } else {%>
          <td class="list">&nbsp;</td>
          <td class="list">&nbsp;</td>
          <td class="list">&nbsp;</td><%
        }
        if (dstIter.hasNext()) {
          Destination destination = (Destination) dstIter.next();%>
          <td class="list"><%=StringEncoderDecoder.encode(destination.getName())%></td>
          <td class="list"><%=destination.isSubject()%></td>
          <td class="list"><%=StringEncoderDecoder.encode(destination.getSme().getId())%></td>
          <td class="list">
            <table border="1" cellspacing="0"><%
              for (Iterator k = destination.getMasks().iterator(); k.hasNext(); )
              {
                Mask mask = (Mask) k.next();
                %><tr><td><%=mask.getMask()%></td></tr><%
              }%>
            </table>
          </td><%
        } else {%>
          <td class="list">&nbsp;</td>
          <td class="list">&nbsp;</td>
          <td class="list">&nbsp;</td>
          <td class="list">&nbsp;</td><%
        }%>
      </tr><%
      }
    }
  }%>
  </tbody>
</table>
<%@ include file="/common/footer.jsp"%>