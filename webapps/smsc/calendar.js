function trim(str){	return str.replace(/^\s+|\s+$/g,'');}
function makeYear(year, curYear){ // +2:-7; +20:-79; +200:-799; ... etc
	year = trim(year);
	if (year<0||curYear<0||year-1+1!=year) return false;
	curYear+="";year+="";
	var calcYear= year;
	var clen	= curYear.length;
	var ylen	= year.length;
	if (ylen>clen||ylen==0) return false; else {
		var yearPref = curYear.substring(0,clen-ylen);
		calcYear = yearPref+year;
		var pwr = Math.pow(10,ylen-1);
		if (calcYear-curYear>2*pwr)	calcYear=(yearPref-1)+year; else
		if (curYear-calcYear>=8*pwr)calcYear=(yearPref-1+2)+year;
	}
	return calcYear;
}
var calendarMonths = new Array("January","February","March","April","May","June","July","August","September","October","November","December");
var calendarWD = new Array("Su","Mo","Tu","We","Th","Fr","Sa","Su");
var calendarLastPos = -1;
var calendarLastDayPos = -1;
var calendarCurYear=-1;
var calendarCurMonth=-1;
var calendarCurHour1=-1;
var calendarCurMinute1=-1;
var calendarCurSecond1=-1;
var calendarCurHour=0;
var calendarCurMinute=0;
var calendarCurSecond=0;
var calendarPressedYPos=-1;
var calendarCurPM=true;
var calendarPressedTime=false;
var calendarPressedHour=false;
var calendarPressedMinute=false;
var calendarPressedSecond=false;
var calendarMouseDown=false;
var calendarUS=false;
var calendarTime=false;
var calendarActivePanel=null;
function calendarMakeDateStr(year,month,day,hour,minute,second,PM){
	return calendarUS?(((month+1)<10?"0":"")+(month+1)+"/"+(day<10?"0":"")+day+"/"+year+(calendarTime?("  "+hour+":"+(minute<10?"0":"")+minute+":"+(second<10?"0":"")+second+" "+(PM?"PM":"AM")):"")):(day+"."+((month+1)<10?"0":"")+(month+1)+"."+year+(calendarTime?("  "+hour+":"+(minute<10?"0":"")+minute+":"+(second<10?"0":"")+second):""));
}
function calendarClose(){
	calendarPanel.releaseCapture();
	calendarPanel.runtimeStyle.display="none";
	document.all.calendarIFrame.runtimeStyle.display="none";
	calendarMouseDown=false;
	calendarPressedTime=false;
	calendarPressedHour=false;
	calendarPressedMinute=false;
	calendarPressedSecond=false;
	return false;
}
function createCalendarPanel(year,month,day,hour,minute,second,PM) {
	calendarActivePanel.value=calendarMakeDateStr(year,month,day,hour,minute,second,PM);
	calendarCurYear=year;
	calendarCurMonth=month;
	var startDay=calendarUS?-1:0;
	var date;
	for (i=calendarDaysTable.rows.length-1;i>=0;i--)calendarDaysTable.deleteRow(calendarDaysTable.rows(i));
	while((new Date(year,month,startDay)).getDay()!=(calendarUS?0:1)) startDay--;
	var dayPos=startDay;
	var curDate = new Date();
	curDate = new Date(curDate.getYear(), curDate.getMonth(), curDate.getDate());
	for (j=0;j<6;j++){
		row = calendarDaysTable.insertRow();
		row.align="center";
		for (i=0;i<7;i++){
			cell = row.insertCell();
			date=new Date(year,month,dayPos);
			if (date.getMonth()==month)
				if(date.getDate()==day) {
					cell.className="calendarDown";
					calendarLastPos = ((j*7)+i);
					calendarLastDayPos = dayPos;
				} else cell.className="calendarUp";
			else cell.className="calendarGreyUp";
			if (curDate.getTime()==date.getTime()) cell.style.fontWeight="bolder";
			cell.name = "calendarCell" + ((""+dayPos).length<2?" ":"") + dayPos + "" +((j*7)+i);
			cell.innerHTML=date.getDate();
			dayPos++;
		}
	}
	if (calendarTime) {
		if(calendarUS)if(calendarCurPM) calendarAMPMsv.innerHTML="PM"; else calendarAMPMsv.innerHTML="AM";
		calendarTimeHour.innerHTML = (calendarCurHour<10?"0":"")+calendarCurHour;
		calendarTimeMinute.innerHTML = (calendarCurMinute<10?"0":"")+calendarCurMinute;
		calendarTimeSecond.innerHTML = (calendarCurSecond<10?"0":"")+calendarCurSecond;
	}
	calendarYearPanel.innerHTML=calendarMonths[month]+"&nbsp;"+year;
}
function showCalendar(calendarInputText, us, showTime){
	calendarActivePanel=calendarInputText;
	if (showTime) {
		calendarTimePanel.style.display="block";
		calendarDaysPanel.style.borderBottom="solid 1px #000000";
		calendarDaysPanel.style.paddingBottom="2px";
		if (us) calendarAMPMsv.style.visibility="inherit";
		calendarTime=true;
	} else {
		calendarTimePanel.style.display="none";
		calendarDaysPanel.style.borderBottom="solid 0px #000000";
		calendarDaysPanel.style.paddingBottom="0px";
		calendarTime=false;
	}
	calendarUS = us;
	var dtText=calendarInputText.value;
	var year=-1;
	var month=-1;
	var day=-1;
	var hour=(us?12:0);
	var minute=0;
	var second=0;
	var PM=true;
	var pos1 = dtText.indexOf((calendarUS?"/":"."));
	if (pos1!=-1){
	var pos2 = dtText.indexOf((calendarUS?"/":"."),pos1+1);
	if (pos2!=-1){
	if (calendarUS) {
		var dtMonth = dtText.substring(0,pos1)-1;
		var dtDate  = dtText.substring(pos1+1,pos2);
	} else {
		var dtMonth = dtText.substring(pos1+1,pos2)-1;
		var dtDate  = dtText.substring(0,pos1);
	}
	var pos3=-1;
	var dtYear=-1;
	if (!calendarTime) dtYear=dtText.substring(pos2+1);
		else {
			pos3=dtText.indexOf(" ",pos2+1);
			if (pos3!=-1) dtYear=dtText.substring(pos2+1,pos3);
				else dtYear=dtText.substring(pos2+1);
		}
	if (dtYear=makeYear(dtYear,(new Date()).getYear())){
	var dt = new Date(dtYear, dtMonth, dtDate);
	if( (dt.getDate()		== dtDate) &&
		(dt.getMonth()		== dtMonth) &&
		(dt.getFullYear()	== dtYear)){
		month = dtMonth/1;
		day = dtDate/1;
		year = dtYear/1;
	}}}}
	if (year==-1) {
		var tdate = new Date();
		if (calendarCurYear==-1) {
			year=tdate.getFullYear();
			month=tdate.getMonth();
		} else {
			year=calendarCurYear;
			month=calendarCurMonth;
		}
		day=tdate.getDate();
	}
	if (calendarTime&&pos3!=-1){
		var pos4=dtText.indexOf(":",pos3+1);
		if (pos4!=-1) {
			var pos5=dtText.indexOf(":",pos4+1);
			if(pos5!=-1) {
				var pos6=-1;
				var dtHour=dtText.substring(pos3+1,pos4);
				var dtMinute=dtText.substring(pos4+1,pos5);
				var dtSecond=-1;
				if (calendarUS) {
					pos6=dtText.indexOf(" ",pos5+1);
					if (pos6!=-1) {
						dtSecond=dtText.substring(pos5+1,pos6);
						if (trim(dtText.substring(pos6+1))=="AM") PM=false;
					} else dtSecond=dtText.substring(pos5+1);
				} else {
				   dtSecond=dtText.substring(pos5+1);
				}
				if (calendarUS) {
					if (PM&&hour!=12) hour+=12;
					if (!PM&&hour==12) hour=0;
				}
				var dt = new Date(year, month, day, dtHour, dtMinute, dtSecond);
//				alert(dtHour+"\n"+dtMinute+"\n"+dtSecond+"\n"+dt);
				if( (dt.getHours()	== dtHour) &&
					(dt.getMinutes()== dtMinute) &&
					(dt.getSeconds()== dtSecond)){
					hour = dtHour/1;
					minute = dtMinute/1;
					second = dtSecond/1;
				}
			}
		}
		if (calendarUS) {hour%=12;if (hour==0) hour=12;}
		calendarCurHour = hour;
		calendarCurMinute = minute;
		calendarCurSecond = second;
		calendarCurPM = PM;
	}
//-------------------------------------------------
	var PageX=0;var PageY=0;var E=calendarInputText;while(E!=null){PageX+=E.offsetLeft;PageY+=E.offsetTop;E=E.offsetParent;}
	var row=calendarWeekDays;
	if (row.cells(0)==null)
	for (i=0;i<7;i++) {
		var cell=row.insertCell();
		cell.innerHTML=calendarWD[(calendarUS?0:1)+i];
		cell.style.width="22px";
	}
	calendarPanel.style.posLeft=PageX;
	calendarPanel.style.posTop=PageY+calendarInputText.offsetHeight;
	createCalendarPanel(year,month,day,hour,minute,second,PM);
	calendarPanel.runtimeStyle.display="block";
	document.all.calendarIFrame.runtimeStyle.posLeft=PageX;
	document.all.calendarIFrame.runtimeStyle.posTop=PageY+calendarInputText.offsetHeight;
	document.all.calendarIFrame.runtimeStyle.width=calendarPanel.runtimeStyle.width;
	document.all.calendarIFrame.runtimeStyle.height=calendarPanel.runtimeStyle.height;
	document.all.calendarIFrame.runtimeStyle.display="block";
	calendarPanel.setCapture();
 	return false;
}
function isCalendarOwner(o){
	while (o!=null){
		if (o==calendarPanel) return true;
		o=o.parentElement;
	}
	return false;
}
function calendarMD(){
	calendarPanel.setCapture();
	var a = window.event.srcElement;
	if (!isCalendarOwner(a)&&!calendarPressedTime) return calendarClose();
	var b = window.event.button;
	if (a!=null&&a.name!=null&&b==1){
		if(a.name.indexOf("calendarCell")!=-1){
			var dayPos=a.name.substring(12,14);
			var pos=a.name.substring(14);
			if ((new Date(calendarCurYear, calendarCurMonth, calendarLastDayPos)).getMonth()==calendarCurMonth)
				calendarDaysTable.rows((calendarLastPos-calendarLastPos%7)/7).cells(calendarLastPos%7).className="calendarUp"; else
				calendarDaysTable.rows((calendarLastPos-calendarLastPos%7)/7).cells(calendarLastPos%7).className="calendarGreyUp";
			var date=new Date(calendarCurYear, calendarCurMonth, dayPos);
			var year=date.getFullYear();
			var day=date.getDate();
			var month=date.getMonth();
			calendarActivePanel.value=calendarMakeDateStr(year,month,day,calendarCurHour,calendarCurMinute,calendarCurSecond,calendarCurPM);
			if (date.getMonth()==calendarCurMonth)
				calendarDaysTable.rows((pos-pos%7)/7).cells(pos%7).className="calendarDown"; else
				calendarDaysTable.rows((pos-pos%7)/7).cells(pos%7).className="calendarGreyDown";
			calendarLastPos=pos;
			calendarLastDayPos=dayPos;
			calendarMouseDown=true;
		} else if (calendarTime) {
			if (calendarUS&&a.name.indexOf("calendarAMPMsv")!=-1) {
				if(a.innerHTML=="AM") {calendarCurPM=true;a.innerHTML="PM";} else {calendarCurPM=false;a.innerHTML="AM";}
				var date=new Date(calendarCurYear, calendarCurMonth, calendarLastDayPos);
				calendarActivePanel.value=calendarMakeDateStr(date.getFullYear(),date.getMonth(),date.getDate(),calendarCurHour,calendarCurMinute,calendarCurSecond,calendarCurPM);
			} else if(a.name.indexOf("calendarTime")!=-1){
				if (a.name.indexOf("Ok")!=-1) return calendarClose(); else
				if (a.name.indexOf("Hour")!=-1){
					if (a.name.indexOf("Up")!=-1) calendarCurHour = (calendarCurHour + 1)%(calendarUS?12:24); else calendarCurHour = (calendarCurHour + 23)%(calendarUS?12:24);
					if (calendarCurHour==0) calendarCurHour=12;
					calendarTimeHour.innerHTML = (calendarCurHour<10?"0":"")+calendarCurHour;
					var date=new Date(calendarCurYear, calendarCurMonth, calendarLastDayPos);
					calendarActivePanel.value=calendarMakeDateStr(date.getFullYear(),date.getMonth(),date.getDate(),calendarCurHour,calendarCurMinute,calendarCurSecond,calendarCurPM);
					calendarPressedTime=true;
					calendarPressedHour=true;
					calendarPressedYPos=event.y;
					calendarCurHour1=calendarCurHour;
				} else
				if (a.name.indexOf("Minute")!=-1){
					if (a.name.indexOf("Up")!=-1) calendarCurMinute = (calendarCurMinute + 1)%60; else calendarCurMinute = (calendarCurMinute + 59)%60;
					calendarTimeMinute.innerHTML = (calendarCurMinute<10?"0":"")+calendarCurMinute;
					var date=new Date(calendarCurYear, calendarCurMonth, calendarLastDayPos);
					calendarActivePanel.value=calendarMakeDateStr(date.getFullYear(),date.getMonth(),date.getDate(),calendarCurHour,calendarCurMinute,calendarCurSecond,calendarCurPM);
					calendarPressedTime=true;
					calendarPressedMinute=true;
					calendarPressedYPos=event.y;
					calendarCurMinute1=calendarCurMinute;
				} else
				if (a.name.indexOf("Second")!=-1){
					if (a.name.indexOf("Up")!=-1) calendarCurSecond = (calendarCurSecond + 1)%60; else calendarCurSecond = (calendarCurSecond + 59)%60;
					calendarTimeSecond.innerHTML = (calendarCurSecond<10?"0":"")+calendarCurSecond;
					var date=new Date(calendarCurYear, calendarCurMonth, calendarLastDayPos);
					calendarActivePanel.value=calendarMakeDateStr(date.getFullYear(),date.getMonth(),date.getDate(),calendarCurHour,calendarCurMinute,calendarCurSecond,calendarCurPM);
					calendarPressedTime=true;
					calendarPressedSecond=true;
					calendarPressedYPos=event.y;
					calendarCurSecond1=calendarCurSecond;
				}
			}
		}
	}
}
function calendarMU(){
	var a = window.event.srcElement;
	if (calendarTime&&calendarPressedTime) {
		var date=new Date(calendarCurYear, calendarCurMonth, calendarLastDayPos);
		calendarActivePanel.value=calendarMakeDateStr(date.getFullYear(),date.getMonth(),date.getDate(),calendarCurHour,calendarCurMinute,calendarCurSecond,calendarCurPM);
		calendarPressedTime=false;
		calendarPressedHour=false;
		calendarPressedMinute=false;
		calendarPressedSecond=false;
		return;
	}
	if (!isCalendarOwner(a)) return calendarClose();
	if (a!=null&&a.name!=null) if (a.name.indexOf("calendarCell")!=-1&&calendarMouseDown){
		if (!calendarTime) return calendarClose();
		} else if (a.name.indexOf("calendarMonth")!=-1) {
		if (a.name.indexOf("calendarMonthMinus")!=-1) calendarCurMonth--; else calendarCurMonth++;
		var date=new Date(calendarCurYear,calendarCurMonth,1);
		calendarCurMonth = date.getMonth();
		calendarCurYear = date.getFullYear();
		calendarLastDayPos/=1;
		if (calendarLastDayPos<=0) calendarLastDayPos=1; else
			while ((new Date(calendarCurYear,calendarCurMonth,calendarLastDayPos)).getMonth()!=calendarCurMonth) calendarLastDayPos--;
		createCalendarPanel(calendarCurYear,calendarCurMonth,calendarLastDayPos,calendarCurHour,calendarCurMinute,calendarCurSecond,calendarCurPM);
	}
	calendarMouseDown=false;
}
function calendarMM(){
	var a = window.event.srcElement;
	var b = window.event.button;
	if (calendarTime&&calendarPressedTime) {
		var inc = calendarPressedYPos - event.y;
		inc = (inc-inc%5)/5;
		if (calendarPressedHour) {
			calendarCurHour = (calendarCurHour1 + 1536 + inc)%(calendarUS?12:24);
			if (calendarCurHour==0) calendarCurHour=12;
			calendarTimeHour.innerHTML = (calendarCurHour<10?"0":"")+calendarCurHour;
		} else if (calendarPressedMinute) {
			calendarCurMinute = (calendarCurMinute1 + 1560 + inc)%60;
			calendarTimeMinute.innerHTML = (calendarCurMinute<10?"0":"")+calendarCurMinute;
		} else if (calendarPressedSecond) {
			calendarCurSecond = (calendarCurSecond1 + 1560 + inc)%60;
			calendarTimeSecond.innerHTML = (calendarCurSecond<10?"0":"")+calendarCurSecond;
		}
	} else if (a!=null&&a.name!=null&&a.name.indexOf("calendarCell")!=-1)
		if (b!=1) calendarMouseDown=false; else {
		var dayPos=a.name.substring(12,14);
		var pos=a.name.substring(14);
		if (pos!=calendarLastPos){
			if ((new Date(calendarCurYear, calendarCurMonth, calendarLastDayPos)).getMonth()==calendarCurMonth)
				calendarDaysTable.rows((calendarLastPos-calendarLastPos%7)/7).cells(calendarLastPos%7).className="calendarUp"; else
				calendarDaysTable.rows((calendarLastPos-calendarLastPos%7)/7).cells(calendarLastPos%7).className="calendarGreyUp";
			var date=new Date(calendarCurYear, calendarCurMonth, dayPos);
			var year=date.getFullYear();
			var day=date.getDate();
			var month=date.getMonth();
			calendarActivePanel.value=calendarMakeDateStr(year,month,day,calendarCurHour,calendarCurMinute,calendarCurSecond,calendarCurPM);
			if (date.getMonth()==calendarCurMonth)
				calendarDaysTable.rows((pos-pos%7)/7).cells(pos%7).className="calendarDown"; else
				calendarDaysTable.rows((pos-pos%7)/7).cells(pos%7).className="calendarGreyDown";
			calendarLastPos=pos;
			calendarLastDayPos=dayPos;
		}
	}
}