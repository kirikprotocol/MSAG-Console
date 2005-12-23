function trim(str) {
    return str.replace(/^\s+|\s+$/g, '');
}
var timeLastPos = -1;
var timeCurHour1 = -1;
var timeCurMinute1 = -1;
var timeCurSecond1 = -1;
var timeCurHour = 0;
var timeCurMinute = 0;
var timeCurSecond = 0;
var timePressedYPos = -1;
var timeCurPM = true;
var timePressedTime = false;
var timePressedHour = false;
var timePressedMinute = false;
var timePressedSecond = false;
var timeMouseDown = false;
var timeUS = false;
var timeActivePanel = null;

function timeMakeTimeStr(hour, minute, second, PM) {
    return "" + hour + ":" + (minute < 10?"0":"") + minute + ":" + (second < 10?"0":"") + second + (timeUS ? (" " + (PM?"PM":"AM")) : "");
}
function timeClose() {
    timePanel.releaseCapture();
    timePanel.style.display = "none";
    document.getElementById('timeIFrame').style.display = "none";
    timeMouseDown = false;
    timePressedTime = false;
    timePressedHour = false;
    timePressedMinute = false;
    timePressedSecond = false;
    return false;
}
function createTimePanel(hour, minute, second, PM) {
    timeActivePanel.value = timeMakeTimeStr(hour, minute, second, PM);
    if (timeUS)if (timeCurPM) timeAMPMsv.innerHTML = "PM";
    else timeAMPMsv.innerHTML = "AM";
    timeTimeHour.innerHTML = (timeCurHour < 10?"0":"") + timeCurHour;
    timeTimeMinute.innerHTML = (timeCurMinute < 10?"0":"") + timeCurMinute;
    timeTimeSecond.innerHTML = (timeCurSecond < 10?"0":"") + timeCurSecond;
}
function showTime(timeInputText, us) {
    timeActivePanel = timeInputText;
    timeTimePanel.style.display = "block";
    if (us) timeAMPMsv.style.visibility = "inherit";

    timeUS = us;
    var dtText = timeInputText.value;
    var hour = (us?12:0);
    var minute = 0;
    var second = 0;
    var PM = true;

    var dt = new Date(1992, 2, 2);
    var pos4 = dtText.indexOf(":");
    if (pos4 != -1) {
        var pos5 = dtText.indexOf(":", pos4 + 1);
        if (pos5 != -1) {
            var pos6 = -1;
            var dtHour = dtText.substring(0, pos4);
            var dtMinute = dtText.substring(pos4 + 1, pos5);
            var dtSecond = -1;
            if (timeUS) {
                pos6 = dtText.indexOf(" ", pos5 + 1);
                if (pos6 != -1) {
                    dtSecond = dtText.substring(pos5 + 1, pos6);
                    if (trim(dtText.substring(pos6 + 1)) == "AM") PM = false;
                }
                else dtSecond = dtText.substring(pos5 + 1);
            }
            else {
                dtSecond = dtText.substring(pos5 + 1);
            }
            if (timeUS) {
                if (PM && hour != 12) hour += 12;
                if (!PM && hour == 12) hour = 0;
            }
            var dt = new Date(1992, 2, 2, dtHour, dtMinute, dtSecond);
            //		alert(dtHour+"\n"+dtMinute+"\n"+dtSecond+"\n"+dt);
            if ((dt.getHours() == dtHour) &&
                (dt.getMinutes() == dtMinute) &&
                (dt.getSeconds() == dtSecond)) {
                hour = dtHour / 1;
                minute = dtMinute / 1;
                second = dtSecond / 1;
            }
        }
    }
    if (timeUS) {
        hour %= 12;
        if (hour == 0) hour = 12;
    }
    timeCurHour = hour;
    timeCurMinute = minute;
    timeCurSecond = second;
    timeCurPM = PM;
    //-------------------------------------------------
    var PageX = 0;
    var PageY = 0;
    var E = timeInputText;
    while (E != null) {
        if (E.offsetParent != null && E.offsetParent.offsetParent != null) //hack for current design
            PageX += E.offsetLeft;

        PageY += E.offsetTop;
        E = E.offsetParent;
    }
    timePanel.style.posLeft = PageX;
    timePanel.style.posTop = PageY + timeInputText.offsetHeight;
    createTimePanel(hour, minute, second, PM);
    timePanel.style.display = "block";
    document.getElementById('timeIFrame').style.posLeft = PageX;
    document.getElementById('timeIFrame').style.posTop = PageY + timeInputText.offsetHeight;
    document.getElementById('timeIFrame').style.width = timePanel.offsetWidth;
    document.getElementById('timeIFrame').style.height = timePanel.offsetHeight;
    document.getElementById('timeIFrame').style.display = "block";
    timePanel.setCapture();
    return false;
}
function isTimeOwner(o) {
    while (o != null) {
        if (o == timePanel) return true;
        o = o.parentElement;
    }
    return false;
}
function timeMD() {
    timePanel.setCapture();
    var a = window.event.srcElement;
    if (!isTimeOwner(a) && !timePressedTime) return timeClose();
    var b = window.event.button;
    if (a != null && a.name != null && b == 1) {
        if (timeUS && a.name.indexOf("timeAMPMsv") != -1) {
            if (a.innerHTML == "AM") {
                timeCurPM = true;
                a.innerHTML = "PM";
            }
            else {
                timeCurPM = false;
                a.innerHTML = "AM";
            }
            timeActivePanel.value = timeMakeTimeStr(timeCurHour, timeCurMinute, timeCurSecond, timeCurPM);
        }
        else if (a.name.indexOf("timeTime") != -1) {
            if (a.name.indexOf("Ok") != -1) return timeClose();
            else
                if (a.name.indexOf("Hour") != -1) {
                    if (a.name.indexOf("Up") != -1) timeCurHour = (timeCurHour + 1) % (timeUS?12:24);
                    else timeCurHour = (timeCurHour + 23) % (timeUS?12:24);
                    if (timeCurHour == 0) timeCurHour = 12;
                    timeTimeHour.innerHTML = (timeCurHour < 10?"0":"") + timeCurHour;
                    timeActivePanel.value = timeMakeTimeStr(timeCurHour, timeCurMinute, timeCurSecond, timeCurPM);
                    timePressedTime = true;
                    timePressedHour = true;
                    timePressedYPos = event.y;
                    timeCurHour1 = timeCurHour;
                }
                else
                    if (a.name.indexOf("Minute") != -1) {
                        if (a.name.indexOf("Up") != -1) timeCurMinute = (timeCurMinute + 1) % 60;
                        else timeCurMinute = (timeCurMinute + 59) % 60;
                        timeTimeMinute.innerHTML = (timeCurMinute < 10?"0":"") + timeCurMinute;
                        timeActivePanel.value = timeMakeTimeStr(timeCurHour, timeCurMinute, timeCurSecond, timeCurPM);
                        timePressedTime = true;
                        timePressedMinute = true;
                        timePressedYPos = event.y;
                        timeCurMinute1 = timeCurMinute;
                    }
                    else
                        if (a.name.indexOf("Second") != -1) {
                            if (a.name.indexOf("Up") != -1) timeCurSecond = (timeCurSecond + 1) % 60;
                            else timeCurSecond = (timeCurSecond + 59) % 60;
                            timeTimeSecond.innerHTML = (timeCurSecond < 10?"0":"") + timeCurSecond;
                            timeActivePanel.value = timeMakeTimeStr(timeCurHour, timeCurMinute, timeCurSecond, timeCurPM);
                            timePressedTime = true;
                            timePressedSecond = true;
                            timePressedYPos = event.y;
                            timeCurSecond1 = timeCurSecond;
                        }
        }
    }
}
function timeMU() {
    var a = window.event.srcElement;
    if (timePressedTime) {
        timeActivePanel.value = timeMakeTimeStr(timeCurHour, timeCurMinute, timeCurSecond, timeCurPM);
        timePressedTime = false;
        timePressedHour = false;
        timePressedMinute = false;
        timePressedSecond = false;
        return;
    }
    if (!isTimeOwner(a)) return timeClose();
    timeMouseDown = false;
}
function timeMM() {
    var a = window.event.srcElement;
    var b = window.event.button;
    if (timePressedTime) {
        var inc = timePressedYPos - event.y;
        inc = (inc - inc % 5) / 5;
        if (timePressedHour) {
            timeCurHour = (timeCurHour1 + 1536 + inc) % (timeUS?12:24);
            if (timeCurHour == 0) timeCurHour = 12;
            timeTimeHour.innerHTML = (timeCurHour < 10?"0":"") + timeCurHour;
        }
        else if (timePressedMinute) {
            timeCurMinute = (timeCurMinute1 + 1560 + inc) % 60;
            timeTimeMinute.innerHTML = (timeCurMinute < 10?"0":"") + timeCurMinute;
        }
        else if (timePressedSecond) {
            timeCurSecond = (timeCurSecond1 + 1560 + inc) % 60;
            timeTimeSecond.innerHTML = (timeCurSecond < 10?"0":"") + timeCurSecond;
        }
    }
}