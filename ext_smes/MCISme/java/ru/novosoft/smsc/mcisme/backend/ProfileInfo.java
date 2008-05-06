package ru.novosoft.smsc.mcisme.backend;

/**
  * User: miker
 * Date: 04.08.2006
 */
public class ProfileInfo
{
	public boolean	inform = true;
	public boolean	notify = false;
  public boolean  wantNotifyMe = false;

  public int		informSelectedTemplate = -1;
	public int		notifySelectedTemplate = -1;
	
	public boolean	busy = false;
	public boolean	noReplay = false;
	public boolean	absent = false;
	public boolean	detach = false;
	public boolean	uncond = false;
	public boolean	other = false;

	public boolean isAddressNotExists = false;

	public ProfileInfo() {}
//	public ProfileInfo(ProfileInfo info){}
}
