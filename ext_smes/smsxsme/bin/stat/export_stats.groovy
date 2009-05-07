/**
 * User: artem
 * Date: 05.05.2009
 */
import ru.sibinco.smsc.utils.subjects.SmscSubjectsList
import ru.sibinco.smsc.utils.subjects.SmscSubject
import java.text.SimpleDateFormat

def ROUTE_IDS = ['"abonents > abonents"', '"websms > abonents"']
def SERVICE_NAMES = ["���","��������","���������","������","������","��������+���������","��������+������", "���������+������"]
def WEBSMS_SME_ID = "websms"

class Counter {
  def services = [:]; // ���������� ��������� ��� �� ��������
  def extraUsers = [:] // ���������� ���������� ������������� ������ ��� websms
  def websmsUsers = new HashSet() // ���������� ���������� ����������� websms
}

def counters = [:];
def websmsDailyCounters = [:]

Closure findIndex = {String line, int from, int number ->
  int j = from;
  number.times {j = line.indexOf(',', j+1)}
  return j;
}

Closure parseFile = {f, subjects ->
  println "Parse file: ${f.getName()}"

  def lineNo = 0;
  f.eachLine {String line->
    if (lineNo > 0) {
      // Source sourceAddress
      int j = findIndex(line, 0, 7)
      int k = line.indexOf(',', j+1)
      def sourceAddress = line.substring(j+2,k-1)
      // Source SmeId
      j = findIndex(line, k+1, 2)
      k = line.indexOf(',', j+1)
      def srcSmeId = line.substring(j+2, k-1)
      // Destination address
      j = findIndex(line, k+1, 0)
      k = line.indexOf(',', j+1)
      def destinationAddress = line.substring(j+1, k-1)
      // Route id
      j = findIndex(line, k+1, 4)
      k = line.indexOf(',', j+1)
      def routeId = line.substring(j+1,k);

      if (ROUTE_IDS.contains(routeId)) {
        // Fetch service id
        j = k
        k = line.indexOf(',', j+1)
        def serviceId = line.substring(j+1,k).asType(Integer)

        if (srcSmeId != WEBSMS_SME_ID) {
          SmscSubject s = subjects.getSubjectByAddress(sourceAddress);
          def subjectId = s == null ? "����������� ������" : s.getId();

          Counter c = counters[subjectId];
          if (!c) {
            c = new Counter();
            counters[subjectId] = c
          }

          if (!c.services[serviceId])
            c.services[serviceId] = 0
          c.services[serviceId]++

          if (!c.extraUsers[serviceId])
            c.extraUsers[serviceId] = new HashSet();
          c.extraUsers[serviceId] << sourceAddress

        } else {
          SmscSubject s = subjects.getSubjectByAddress(destinationAddress);
          def subjectId = s == null ? "����������� ������" : s.getId();

          Counter c = counters[subjectId];
          if (!c) {
            c = new Counter();
            counters[subjectId] = c
          }

          c.websmsUsers << destinationAddress
          if (!websmsDailyCounters[subjectId])
            websmsDailyCounters[subjectId] = 0
          websmsDailyCounters[subjectId] ++;
        }
      }
    }
    lineNo++;
  }
}

def subjects = new SmscSubjectsList();
try {
  subjects.load("conf/routes.xml");
} catch (Throwable e) {
  e.printStackTrace();
  return;
}

def dir = new File(args[0]);

def websmsDailyReport = new File("websms-daily-${dir.getName()}.csv")
websmsDailyReport.write("����; ����� ������; id �������; ������; ���������� ������������ ���\r\n")

def sdf = new SimpleDateFormat('yyyy-MMM-dd', Locale.ENGLISH)
def df = new SimpleDateFormat('dd.MM.yyyy')

dir.eachDir {dateDir ->
  def date = sdf.parse("${dir.getName()}-${dateDir.getName()}")
  dateDir.eachDir { hourDir->
    hourDir.eachFile {it-> parseFile(it, subjects) }    
  }
  websmsDailyCounters.entrySet().each {it-> websmsDailyReport.append("${df.format(date)}; ${it.key}; 999; SMS � �����; ${it.value}\r\n")}
}

def year = new SimpleDateFormat("yyyy").format(new Date())
def month = new SimpleDateFormat("MM").format(new Date())

// Generate services report
def servicesFile = new File("services-${dir.getName()}.csv")
servicesFile.write("���; �����; ����� ������; id �������; ������; ���������� ��������� SMS\r\n")
counters.entrySet().each {e->
  e.value.services.entrySet().each {
    servicesFile.append("$year; $month; ${e.key}; ${it.key}; ${SERVICE_NAMES[it.key]}; ${it.value}\r\n")
  }
}

// Generate extraUsers report
def usersFile = new File("users-${dir.getName()}.csv")
usersFile.write("���; �����; ����� ������; id �������; ������; ���������� ���������� �������������\r\n")
counters.entrySet().each {e->
  e.value.extraUsers.entrySet().each {
    usersFile.append("$year; $month; ${e.key}; ${it.key}; ${SERVICE_NAMES[it.key]}; ${it.value.size()}\r\n")
  }
  def total = 0, totalWithoutSms = 0
  for (int i=0; i<7; i++) {
    def set = e.value.extraUsers[i]
    if(set!=null) {
      total += set.size()
      if (i > 0)
        totalWithoutSms += set.size()
    }
  }
  usersFile.append("$year; $month; ${e.key}; 8; SMS Extra; $total\r\n")
  usersFile.append("$year; $month; ${e.key}; 9; SMS Extra (��� ���); $totalWithoutSms\r\n")
}

// Generate Websms monthly report
def websmsMonthlyFile = new File("websms-monthly-${dir.getName()}.csv")
websmsMonthlyFile.write("���; �����; ����� ������; id �������; ������; ���������� ���������� �����������\r\n")
counters.entrySet().each {e->
  websmsMonthlyFile.append("$year; $month; ${e.key}; 999; SMS � �����; ${e.value.websmsUsers.size()}\r\n")  
}
