<link rel="STYLESHEET" type="text/css" href="/styles/time.css">
<script src="/scripts/time.js" type="text/javascript"></script>
<iframe id=timeIFrame class=timeHiddenLayer2 src="/images/blank.html"></iframe>

<div id=timePanel class=timeHiddenLayer oncontextmenu="return false;" onselectstart="return false;" ondragstart="return false;" onmousedown="return timeMD();" onmouseup="return timeMU();" onmousemove="return timeMM();" onlosecapture="return timeClose();" onfocusout="return timeClose();">
  <table border=1 bgcolor="#dododo" cellpadding=0 cellspacing=2>
    <tr>
      <td style="border:0;">
        <table cellpadding=1 cellspacing=0>
          <tr>
            <td id=timeTimePanel style="border-top:solid 1px #ffffff;display:none;padding-top:2px;">
              <table cellpadding=0 cellspacing=0>
                <tr>
                  <td rowspan=2 width=2px></td>
                  <td rowspan=2><div class=timeTimeH id=timeTimeHour>00</div></td>
                  <td valign=bottom align=center><div name=timeTimeHourUp style="background-color:#f0f0f0;border-top:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">5</div></td>
                  <td rowspan=2><div class=timeTimeM id=timeTimeMinute>00</div></td>
                  <td valign=bottom align=center><div name=timeTimeMinuteUp style="background-color:#f0f0f0;border-top:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">5</div></td>
                  <td rowspan=2><div class=timeTimeS id=timeTimeSecond>00</div></td>
                  <td valign=bottom align=center><div name=timeTimeSecondUp style="background-color:#f0f0f0;border-top:solid 1px #000000;border-right:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">5</div></td>
                  <td rowspan=2 width=30px align=right><div id=timeAMPMsv name=timeAMPMsv align=center style="padding:0px; padding-left:2px; background-color:#f0f0f0;border:solid 1px #000000;width=24px;height=16px;font-family:Tahoma,Arial,Helvetica,sans-serif;font-size:11px;font-weight:bold;visibility: hidden;">PM</div></td>
                  <td rowspan=2 width=40px align=right><div name=timeTimeOk align=center style="padding:0px; background-color:#f0f0f0;border:solid 1px #000000;width=30px;height=16px;font-family:Tahoma,Arial,Helvetica,sans-serif;font-size:11px;font-weight:bold;">Ok</div></td>
                </tr>
                <tr>
                  <td align=center><div name=timeTimeHourDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">6</div></td>
                  <td align=center><div name=timeTimeMinuteDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">6</div></td>
                  <td align=center><div name=timeTimeSecondDown style="background-color:#f0f0f0;border-bottom:solid 1px #000000;border-right:solid 1px #000000;width=9px;height=6px;font-family:Webdings;font-size:7px">6</div></td>
                </tr>
              </table>
            </td>
          </tr>
        </table>
      </td>
    </tr>
  </table>
</div>