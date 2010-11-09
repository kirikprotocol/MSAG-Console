package mobi.eyeline.informer.admin.restriction;

import mobi.eyeline.informer.admin.AdminException;

import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 03.11.2010
 * Time: 15:17:30
 */
class RestrictionSettings {

  private Map<Integer,Restriction> restrictions = new TreeMap<Integer,Restriction>();
  private int lastId=0;


  public RestrictionSettings(List<Restriction> restList) {
    lastId=0;
    for(Restriction r : restList) {
      if(r.getId()>lastId) lastId = r.getId();
      restrictions.put(r.getId(),r);
    }
  }

  public synchronized int addRestriction (Restriction r) throws AdminException {
    checkRestriction(r);
    lastId++;
    r.setId(lastId);
    r = new Restriction(r);
    restrictions.put(lastId,r);
    return lastId;
  }

  public synchronized void updateRestriction (Restriction r) throws AdminException {
    checkRestriction(r);
    Restriction old = restrictions.remove(r.getId());
    if(old == null) {
      throw new RestrictionException("restrictions.not.found",r.getId()+"");
    }
    restrictions.put(r.getId(), new Restriction(r));
  }

  public synchronized void deleteRestriction (int id) {
    restrictions.remove(id);
  }

  public synchronized Restriction getRestriction(int id) {
    Restriction r = restrictions.get(id);
    return r==null ? null : new Restriction(r);
  }

  public synchronized  List<Restriction> getRestrictions(RestrictionsFilter filter) {
    List<Restriction> ret = new ArrayList<Restriction>();
    for(Restriction r : restrictions.values()) {
      if(filter!=null) {
        if(filter.getStartDate()!=null) {
          long startT = filter.getStartDate().getTime();
          if(r.getEndDate().getTime()<startT) continue;
        }
        if(filter.getEndDate()!=null) {
          long endT = filter.getEndDate().getTime();
          if(r.getStartDate().getTime()>=endT) continue;
        }
        if(filter.getNameFilter()!=null && r.getName().indexOf(filter.getNameFilter())==-1) continue;
        if(filter.getUserId()!=null && !r.isAllUsers() ) {
          if(!r.getUserIds().contains(filter.getUserId())) continue;
        }
      }
      ret.add(new Restriction(r));
    }
    return ret;
  }

  void checkRestriction(Restriction r) throws AdminException {
    if(r.getName()==null || r.getName().trim().length()==0) throw new RestrictionException ("error.empty.name");
    if(r.getStartDate()==null) throw new RestrictionException ("error.startDate.null");
    if(r.getEndDate()==null) throw new RestrictionException ("error.endDate.null");
    if(r.getEndDate().getTime()<=r.getStartDate().getTime()) throw new RestrictionException ("error.reverse.dates");
    if(!r.isAllUsers() && (r.getUserIds()==null || r.getUserIds().size()==0)) {
      throw new RestrictionException ("error.empty.users.list");
    }
  }
}
