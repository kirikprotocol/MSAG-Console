<%@include file="/WEB-INF/inc/header.jspf"%>

<sm:page title="routes_tracing.title">
    <jsp:attribute name="menu">
    </jsp:attribute>
    <jsp:body>
        <table class=properties_list cellspacing=0 cellpadding=0>
            <col width="15%">
            <col width="100%">
            <c:set var="prop_rowN" value="0" scope="request"/>
            <sm-ep:list title="subjects.index.list.transportid" onChange="changeTransportId();" name="transportId"
                        values="${fn:join(bean.transportIds, ',')}"
                        valueTitles="${fn:join(bean.transportTitles, ',')}"/>
        </table>
        <br>
        <c:choose>
            <c:when test="${bean.appContext.statuses.smppRoutesLoaded && bean.transportId == 1}">
                <sm-ep:properties title="smpp.routes_tracing.properties">
                    <sm-ep:txt title="routes_tracing.txt.srcaddress" name="srcAddress"/>
                    <sm-ep:txt title="routes_tracing.txt.dstaddress" name="dstAddress"/>
                    <sm-ep:txt title="routes_tracing.txt.srcsysId" name="srcSysId"/>
                </sm-ep:properties>
                <sm:break>
                    <sm-pm:menu>
                        <sm-pm:item name="mbCheck" value="routes_tracing.item.mbcheck.value"
                                    title="routes_tracing.item.mbcheck.title"/>
                        <sm-pm:item name="mbTrace" value="routes_tracing.item.mbtrace.value"
                                    title="routes_tracing.item.mbtrace.title"/>
                        <sm-pm:space/>
                    </sm-pm:menu>
                </sm:break>
            </c:when>
            <c:when test="${bean.appContext.statuses.httpRoutesLoaded && bean.transportId == 2}">
                <sm-ep:properties title="http.routes_tracing.properties">
                    <sm-ep:txt title="routes_tracing.txt.service_id" name="serviceId"/>
                    <sm-ep:txt title="routes_tracing.txt.route_id" name="routeId"/>
                    <sm-ep:txt title="routes_tracing.txt.abonent" name="abonent"/>
                    <sm-ep:txt title="routes_tracing.txt.site" name="site"/>
                    <sm-ep:txt title="routes_tracing.txt.path" name="path"/>
                    <sm-ep:txt title="routes_tracing.txt.port" name="port"/>
                </sm-ep:properties>
                <sm:break>
                    <sm-pm:menu>
                        <sm-pm:item name="mbCheck" value="routes_tracing.item.mbcheck.value"
                                    title="routes_tracing.item.mbcheck.title"/>
                        <sm-pm:item name="mbTrace" value="routes_tracing.item.mbtrace.value"
                                    title="routes_tracing.item.mbtrace.title"/>
                        <sm-pm:space/>
                    </sm-pm:menu>
                </sm:break>
            </c:when>
            <c:otherwise>
                <c:if test="${bean.transportId != 3}">
                    <fmt:message>routes_tracing.label.trace_route</fmt:message>
                    <div class=content>
                        <table class=properties_list cell>
                            <tr class=row0><td>
                                <span class="C800">
                                    <b><fmt:message>routes_tracing.label.warning1</fmt:message></b></span>
                                <br>
                                <br>
                                <span class="C000"><fmt:message>routes_tracing.label.warning2</fmt:message></span><br>
                                <span class="C000"><fmt:message>routes_tracing.label.warning3</fmt:message></span><br>
                                &nbsp;
                            </td></tr>
                        </table>
                    </div>
                    <sm:break>
                        <sm-pm:menu>
                            <sm-pm:item name="mbCheck" value="routes_tracing.item.mbcheck.value"
                                        title="routes_tracing.item.mbcheck.title"/>
                            <sm-pm:space/>
                        </sm-pm:menu>
                    </sm:break>
                </c:if>
            </c:otherwise>
        </c:choose>

    <c:set var="message" value="${bean.traceMessage}"/>
    <c:set var="traceResults" value="${bean.traceResults}"/>
    <c:set var="routeInfo" value="${bean.routeInfo}"/>
    <c:set var="routeInfoMap" value="${bean.routeInfoMap}"/>
    <c:if test="${message !=null || traceResults !=null || routeInfo !=null }">
        <div class=content>
        <c:if test="${message !=null }">
            <c:set var="mt" value="${bean.messageType}"/>
            <table class=properties_list cell>
                <tr><td><span class="<c:choose>
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
            <div class=page_subtitle><fmt:message>routes_tracing.label.route_info</fmt:message></div>
            <table class=properties_list cell>
                <c:forEach items="${routeInfoMap}" var="i" varStatus="status">
                    <c:set var="key" value="${i.key}"/>
                    <c:set var="val" value="${i.value}"/>
                    <tr class='row${(status.count+1)%2}'>
                        <th width="30%" nowrap><c:choose>
                            <c:when test="${key != null && fn:length(fn:trim(key))>0}"><c:out
                                    value="${fn:escapeXml(key)}"/></c:when>
                            <c:otherwise>&nbsp;</c:otherwise>
                        </c:choose></th>
                        <td width="70%" nowrap><c:choose>
                            <c:when test="${val != null && fn:length(fn:trim(val))>0}">
                               <c:choose>
                                  <c:when test="${smf:isBoolean(val)}">
                                    <c:choose>
                                       <c:when test="${val}">
                                         <img align="left" src="content/images/ic_checked.gif">
                                       </c:when>
                                       <c:otherwise>
                                         <img align="left" src="content/images/ic_not_checked.gif">
                                       </c:otherwise>
                                    </c:choose>
                                  </c:when>
                                  <c:otherwise>
                                    <c:out value="${val}" escapeXml="false"/>
                                  </c:otherwise>
                               </c:choose> 
                            </c:when>
                            <c:otherwise>&nbsp;</c:otherwise>
                        </c:choose></td>
                    </tr>
                </c:forEach>
            </table>
        </c:if>
        <c:if test="${ traceResults !=null && fn:length(traceResults)>0 }">
            <br>
            <div class=page_subtitle><fmt:message>routes_tracing.label.trace</fmt:message></div>
            <textarea  readonly="true" style="font-family:Courier New,sans-serif; height:300px"><c:forEach items="${traceResults}" var="result">
                <c:set var="traceObj" value="${result}"/><c:if test="${traceObj != null && smf:isString(traceObj)}">${fn:trim(traceObj)}</c:if></c:forEach></textarea>
            </div>
        </c:if>
    </c:if>
</jsp:body>
</sm:page>


