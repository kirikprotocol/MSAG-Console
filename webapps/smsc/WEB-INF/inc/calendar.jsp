<link rel="STYLESHEET" type="text/css" href="<%=CPATH%>/calendar.css">
<script src="<%=CPATH%>/calendar.js" type="text/javascript"></script>
<iframe id=calendarIFrame class=calendarHiddenLayer2></iframe>
<div id=calendarPanel class=calendarHiddenLayer oncontextmenu="return false;" onselectstart="return false;" ondragstart="return false;" onmousedown="return calendarMD();" onmouseup="return calendarMU();" onmousemove="return calendarMM();" onlosecapture="return calendarClose();" onfocusout="return calendarClose();">
<table border=1 bgcolor="#dododo" cellpadding=0 cellspacing=2><tr><td style="border:0;"><table cellpadding=1 cellspacing=0><tr>
<td style="border:0; border-bottom:solid 1px #000000; padding-bottom:2px;padding-top:0px"><table height=100% cellpadding=0 cellspacing=0>
<tr align=center class=calendarYear><td colspan=5 id=calendarYearPanel></td>
<td colspan=2 align=rigth><table cellpadding=0 cellspacing=0 class=calendarMonth><tr align=right><td width="100%" name=calendarMonthMinus style="font-family: Webdings;">3</td><td name=calendarMonthPlus style="font-family: Webdings;">4</td></tr></table></td></tr>
<tr align=center style="padding-top:2px" class=calendarWeekDay id=calendarWeekDays>
</tr></table></td></tr><tr><td id=calendarDaysPanel style="border:0; border-top:solid 1px #ffffff; padding-top:2px">
<table height=100% cellpadding=0 cellspacing=0 class=calendarDate id=calendarDaysTable>
</table></td>
<tr><td id=calendarTimePanel style="border-top:solid 1px #ffffff;display:none;padding-top:2px;"><table cellpadding=0 cellspacing=0><tr>
<td rowspan=2 width=2px></td>
<td rowspan=2><div class=calendarTimeH id=calendarTimeHour>00</div></td>
<td valign=bottom align=center><div name=calendarTimeHourUp style="background-color:#f0f0f0;border-top:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">5</div></td>
<td rowspan=2><div class=calendarTimeM id=calendarTimeMinute>00</div></td>
<td valign=bottom align=center><div name=calendarTimeMinuteUp style="background-color:#f0f0f0;border-top:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">5</div></td>
<td rowspan=2><div class=calendarTimeS id=calendarTimeSecond>00</div></td>
<td valign=bottom align=center><div name=calendarTimeSecondUp style="background-color:#f0f0f0;border-top:solid 1px #000000;border-right:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">5</div></td>
<td rowspan=2 width=30px align=right><div id=calendarAMPMsv name=calendarAMPMsv align=center style="padding:0px; padding-left:2px; background-color:#f0f0f0;border:solid 1px #000000;width=24px;height=16px;font-family:Tahoma,Arial,Helvetica,sans-serif;font-size:11px;font-weight:bold;visibility: hidden;">PM</div></td>
<td rowspan=2 width=40px align=right><div name=calendarTimeOk align=center style="padding:0px; background-color:#f0f0f0;border:solid 1px #000000;width=30px;height=16px;font-family:Tahoma,Arial,Helvetica,sans-serif;font-size:11px;font-weight:bold;">Ok</div></td>
</tr><tr>
<td align=center><div name=calendarTimeHourDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">6</div></td>
<td align=center><div name=calendarTimeMinuteDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">6</div></td>
<td align=center><div name=calendarTimeSecondDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;border-right:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">6</div></td>
</tr></table></td></tr></tr></table></td></tr></table></div>