<%@ page import="ru.sibinco.lib.backend.util.StringEncoderDecoder,
                 java.util.List"%>
<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Routes Tracing">
  <jsp:attribute name="menu">
  </jsp:attribute>

  <jsp:body>
   <%-- <sm:table columns="checkbox,name,notes" names="c,name,notes" widths="1,49,50" edit="name"/>
    <%-- <c:set var="smes" value="${bean.smeIds}"/>  --%>

  <c:choose>
    <c:when test="${bean.appContext.statuses.routesLoaded}">

   <sm-ep:properties title="Routes tracing">
      <sm-ep:txt title="Source Address" name="srcAddress"   />
      <sm-ep:txt title="Destination Address" name="dstAddress"  />
      <sm-ep:txt title="Source System ID" name="srcSysId"  />
    </sm-ep:properties>

    <sm:break>
    <sm-pm:menu>
      <sm-pm:item name="mbCheck" value="Loadup & Check" title="Load & Check routing configuration"/>
      <sm-pm:item name="mbTrace" value="Trace route" title="Trace route" />
      <sm-pm:space/>
    </sm-pm:menu>
    </sm:break>

   </c:when>
   <c:otherwise>Trace Route


    <div class=content>
    <table class=properties_list cell>
      <tr class=row0><td>
       <span class="C800"><b>Warning !</b></span><br><br>
       <span class="C000">Active routing configuration is not loaded or has been changed.</span><br>
       <span class="C000">Please loadup new routing information to route tracer.</span><br>&nbsp;
      </td></tr>
    </table>
    </div>

    <sm:break>
     <sm-pm:menu>
      <sm-pm:item name="mbCheck" value="Loadup & Check" title="Load & Check routing configuration"/>
      <sm-pm:space/>
     </sm-pm:menu>
    </sm:break>

  </c:otherwise>
 </c:choose>
<!--example-->
<%--
<table>
<tr>
            <td>Mask</td>
            <td><input id=newDstMask class=txt name=new_dstMask validation="routeMask" onkeyup="resetValidation(this)"></td>
            <td><select name=new_dst_mask_sme_ id=newDstMaskSme>
              <c:forEach items="${bean.allSmes}" var="i">
                <option value="${fn:escapeXml(i)}" <c:if test="${i == bean.new_dst_mask_sme_}">selected</c:if>>${fn:escapeXml(i)}</option>
              </c:forEach>
              </select>
            </td>
            <td><img src="/images/but_add.gif" onclick="addDestMask()" style="cursor:hand;"></td>
          </tr>
</table>
--%>

<c:set var="message" value="${bean.traceMessage}"/>
<c:set var="traceResults" value="${bean.traceResults}"/>
<c:set var="routeInfo" value="${bean.routeInfo}"/>
<c:set var="routeInfoMap" value="${bean.routeInfoMap}"/>
<c:if test="${message !=null || traceResults !=null || routeInfo !=null }">
<div class=content>
<c:if test="${message !=null }">
<c:set var="mt" value="${bean.messageType}"/>
<table class=properties_list cell>
<tr><td>
  <span class="<c:choose>
                  <c:when test="${mt == bean.traceRouteFound }">C080</c:when>
                  <c:when test="${mt == bean.traceRouteNotFound}">C800</c:when>
                  <c:otherwise>C008</c:otherwise>
              </c:choose>">
    <b><c:out value="${fn:escapeXml(message)}"/></b>
  </span>
  </td></tr>
  </table>
</c:if>
<c:if test="${ routeInfo !=null }">
 <br>
  <div class=page_subtitle>Route info</div>
  <table class=properties_list cell>
<c:forEach items="${routeInfoMap}" var="i" varStatus="status">
 <c:set var="key" value="${i.key}"/>
 <c:set var="val" value="${i.value}"/>
 <tr class='row${(status.count+1)%2}'>
       <th width="30%" nowrap><c:choose>
        <c:when test="${key != null && fn:length(fn:trim(key))>0}" ><c:out value="${fn:escapeXml(key)}" /></c:when>
        <c:otherwise>&nbsp;</c:otherwise>
        </c:choose></th>
        <td width="70%" nowrap><c:choose>
        <c:when test="${val != null && fn:length(fn:trim(val))>0}"><c:out value="${ fn:escapeXml(val)}"/></c:when>
        <c:otherwise>&nbsp;</c:otherwise>
        </c:choose></td>
      </tr>
</c:forEach>
</table>
</c:if>
<c:if test="${ traceResults !=null && fn:length(traceResults)>0 }">
<br>
  <div class=page_subtitle>Trace</div>
  <textarea style="font-family:Courier New;height:300px">
<c:forEach items="${traceResults}" var="result" >
<c:set var="traceObj" value="${result}"/>
<c:if test="${traceObj != null && smf:isString(traceObj)}">
<c:out value="${traceObj}"/>
</c:if>
</c:forEach>
</textarea>
</div>
</c:if>
</c:if>
<%--
<%
String message = bean.getTraceMessage();
  List   traceResults = bean.getTraceResults();
  List   routeInfo = bean.getRouteInfo();
  int rowN = 0;
  if (message != null || traceResults != null || routeInfo != null) {%>
  <div class=content>
<%if (message != null) {
  int mt = bean.getMessageType();%>
  <table class=properties_list cell>
  <tr><td>
  <span class="<%= (mt == bean.TRACE_ROUTE_FOUND) ? "C080":
                   ((mt == bean.TRACE_ROUTE_NOT_FOUND) ? "C800":"C008")%>">
    <b><%=StringEncoderDecoder.encode(message)%></b>
  </span>
  </td></tr>
  </table>
<%}%>
<%if (routeInfo != null) {%>
  <br>
  <div class=page_subtitle>Route info</div>
  <table class=properties_list cell><%
    for (int i=0; i<routeInfo.size(); i+=2)
    {
      String key = (String)routeInfo.get(i);
      String val = (String)routeInfo.get(i+1);%>
      <tr class=row<%=rowN++&1%>>
        <th width="30%" nowrap><%=(key != null && key.trim().length()>0) ? StringEncoderDecoder.encode(key):"&nbsp;"%></th>
        <td width="70%" nowrap><%=(val != null && val.trim().length()>0) ? StringEncoderDecoder.encode(val):"&nbsp;"%></td>
      </tr><%
    }%>
  </table>
<%}%>
<%if (traceResults != null && traceResults.size() > 0) {%>
  <br>
  <div class=page_subtitle>Trace</div>
  <textarea style="font-family:Courier New;height:300px"><%
    for (int i=0; i<traceResults.size(); i++) {
      Object traceObj = traceResults.get(i);
      if (traceObj != null && traceObj instanceof String) { %><%=
          StringEncoderDecoder.encode((String)traceObj)+'\n'%><%
      }
    }
  %></textarea>
<%}%>
  </div>
<%}%>   --%>
</jsp:body>
</sm:page>


