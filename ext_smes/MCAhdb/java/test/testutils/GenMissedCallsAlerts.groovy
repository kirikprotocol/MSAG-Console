import java.text.SimpleDateFormat

def journalNamedf = new SimpleDateFormat("yyyyMMddHHmmss");
def df = new SimpleDateFormat("yyyyMMdd HH:mm:ss.SSS");
def startTime = new Date(System.currentTimeMillis() + 3600*1000);

def COUNT = 10
def journal = new File('/home/artem/Work/projects/smsc/ext_smes/MCAhdb/store/journals', 'journal.' + journalNamedf.format(startTime) + '.csv');

COUNT.times {
  def syffix = COUNT + it;
  def time = df.format(new Date(startTime.getTime() + it*100));
  journal << "D,$time,+79139$syffix,+79138$syffix,1,1\n"
}