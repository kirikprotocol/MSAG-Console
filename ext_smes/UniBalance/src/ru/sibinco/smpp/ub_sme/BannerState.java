package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;
import ru.sibinco.util.threads.ThreadsPool;

/**
 * Created by Serge Lugovoy
 * Date: Nov 22, 2007
 * Time: 3:32:27 PM
 */
public interface BannerState extends State {
  public static final byte BE_RESP_WAIT = 4;
  public static final byte BE_RESP_ERR = 5;
  public static final byte BE_RESP_OK = 6;

  public String getBanner();

  public void setBanner(String message);

  public int getBannerState();

  public void setBannerState(int bannerState);

}
