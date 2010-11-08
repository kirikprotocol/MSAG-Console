package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.util.config.ManagedConfigFile;
import mobi.eyeline.informer.util.StringEncoderDecoder;

import java.io.*;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 03.11.2010
 * Time: 17:29:05
 */
class RestrictionsConfig implements ManagedConfigFile<RestrictionSettings> {

  private static final String dateFmt = "yyyy.MM.dd HH:mm:ss";

  public void save(InputStream oldFile, OutputStream newFile, RestrictionSettings settings) throws Exception {
    PrintWriter pw = new PrintWriter(new OutputStreamWriter(newFile,"UTF-8"));
    DateFormat fmt = new SimpleDateFormat(dateFmt);
    for(Restriction r : settings.getRestrictions(null)) {
      Object[] args = new Object[ 5 + (r.getUserIds()==null ? 0 : r.getUserIds().size()) ];
      args[0] = r.getId();
      args[1] = r.getName();
      args[2] = fmt.format(r.getStartDate());
      args[3] = fmt.format(r.getEndDate());
      args[4] = r.isAllUsers();
      if(r.getUserIds()!=null) {
        int i=5;
        for(String userId : r.getUserIds()) {
          args[i++] = userId;
        }
      }
      pw.println(StringEncoderDecoder.toCSVString(args));
    }
    pw.flush();
  }

  public RestrictionSettings load(InputStream is) throws Exception {
    List<Restriction> restrictions= new ArrayList<Restriction>();
    SimpleDateFormat fmt = new SimpleDateFormat(dateFmt);
    BufferedReader reader = new BufferedReader(new InputStreamReader(is,"UTF-8"));
    String line;
    while((line=reader.readLine())!=null) {
      List<String> vals = StringEncoderDecoder.csvDecode(line);
      Restriction r = new Restriction();
      r.setId(Integer.parseInt(vals.get(0)));
      r.setName(vals.get(1));
      r.setStartDate(fmt.parse(vals.get(2)));
      r.setEndDate(fmt.parse(vals.get(3)));
      r.setAllUsers(Boolean.valueOf(vals.get(4)));
      List<String> users = null;
      if(vals.size()>5) {
        users=new ArrayList<String>(vals.size()-5);
        for(int i=5 ; i< vals.size() ; i++) {
            users.add(vals.get(i));
        }
        r.setUserIds(users);
      }
      restrictions.add(r);
    }
    return new RestrictionSettings(restrictions);
  }
}
