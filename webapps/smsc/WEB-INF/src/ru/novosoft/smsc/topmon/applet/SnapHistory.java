package ru.novosoft.smsc.topmon.applet;

import ru.novosoft.smsc.topmon.SmeSnap;
import ru.novosoft.smsc.topmon.TopSnap;

/**
 * Created by: Serge Lugovoy
 * Date: 20.05.2004
 * Time: 18:46:53
 */
public class SnapHistory
{
  public static final int MAX_HISTORY_LENGTH = 2000;
  TopSnap snaps[] = new TopSnap[MAX_HISTORY_LENGTH];
  int head = 0;
  public int count = 0;
  SmeSnap smeSnaps[] = new SmeSnap[MAX_HISTORY_LENGTH];
  String currentSme = null;

  public void addSnap(TopSnap topSnap)
  {
    if (count < MAX_HISTORY_LENGTH) {
      snaps[count] = new TopSnap(topSnap);
      if (currentSme != null) {
        smeSnaps[count] = new SmeSnap(findSnap(topSnap));
      }
      count++;
    }
    else {
      snaps[head] = new TopSnap(topSnap);
      if (currentSme != null) {
        smeSnaps[head] = new SmeSnap(findSnap(topSnap));
      }
      head++;
      if (head == MAX_HISTORY_LENGTH) head = 0;
    }
  }

  SmeSnap findSnap(TopSnap snap)
  {
    for (int i = 0; i < snap.smeCount; i++) {
      if (snap.smeSnap[i].smeId.equals(currentSme)) {
        return snap.smeSnap[i];
      }
    }
    return null;
  }

  public String getCurrentSme()
  {
    return currentSme;
  }

  public void setCurrentSme(String sme)
  {
    if (currentSme == null || !currentSme.equals(sme)) {
      currentSme = new String(sme);
      for (int i = 0; i < count; i++) {
        smeSnaps[i] = new SmeSnap(findSnap(snaps[i]));
      }
    }
  }

  int itIdx;

  public SmeSnap getLast()
  {
    if (count == 0) return null;
    if (count == MAX_HISTORY_LENGTH) {
      if (head == 0)
        itIdx = count - 1;
      else
        itIdx = head - 1;
    }
    else {
      itIdx = count - 1;
    }
    return smeSnaps[itIdx];
  }

  public SmeSnap getPrev()
  {
    if (count == 0) return null;
    itIdx--;
    if (itIdx == -1) itIdx = count - 1;
    return smeSnaps[itIdx];
  }
}
