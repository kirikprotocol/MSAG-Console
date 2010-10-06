package ru.novosoft.smsc.admin.profile;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.cluster_controller.CCProfile;
import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.IOUtils;

import java.io.*;
import java.util.HashMap;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class TestProfileManager extends ProfileManagerImpl {

  private Profile defaultProfile;

  public TestProfileManager(boolean smsx, File profilesFile, FileSystem fs, ClusterController cc) {
    super(smsx, profilesFile, fs, cc);

    try {
      defaultProfile = new Profile(new Address(5,0,"DEFAULT"));

      defaultProfile.setUdhConcat(true);
      defaultProfile.setDivert("qwerty");
      defaultProfile.setLatin1(true);
      defaultProfile.setOutputAccessMask(1);
    } catch (AdminException e) {
      e.printStackTrace();
    }
  }

  public Profile getDefaultProfile() throws AdminException {
    return new Profile(defaultProfile);
  }

  public void updateDefaultProfile(Profile profile) throws AdminException {
    defaultProfile = new Profile(profile);
  }


  public static InputStream emptyProfilesFileAsStream(boolean smsx, int version) throws IOException {
    ByteArrayOutputStream os = new ByteArrayOutputStream();
    IOUtils.writeString(os, "SMSCPROF", 8);
    IOUtils.writeUInt32(os, version);
    return new ByteArrayInputStream(os.toByteArray());
  }

  public static Map<Address, CCProfile> loadProfiles(InputStream is, boolean smsx) throws AdminException, IOException {
    ProfilesSetImpl set = null;
    try {
      set = new ProfilesSetImpl(is, smsx);

      Map<Address, CCProfile> result = new HashMap<Address, CCProfile>();
      while (set.next()) {
        Profile p = set.get();
        result.put(p.getAddress(), profile2CCProfile(p));
      }
      return result;
    } finally {
      if (set != null)
        set.close();
    }
  }

  public static void saveProfiles(Map<Address, CCProfile> profiles, OutputStream file, boolean smsx, int version) throws IOException {
    OutputStream os = null;
    try {
      os = new BufferedOutputStream(file);

      IOUtils.writeString(os, "SMSCPROF", 8);
      IOUtils.writeUInt32(os, version);

      for (Map.Entry<Address, CCProfile> e : profiles.entrySet()) {
        Address a = e.getKey();
        CCProfile p = e.getValue();

        IOUtils.writeString(os, "SmScPrOf", 8);

        IOUtils.writeUInt8(os, a.getTone());
        IOUtils.writeUInt8(os, a.getNpi());
        IOUtils.writeString(os, a.getAddress(), 21);

        int codepage = 0;
        if (p.isLatin1())
          codepage = codepage & 1;
        if (p.isUcs2())
          codepage = codepage & 8;
        if (p.isUssdIn7Bit())
          codepage = codepage & 128;

        IOUtils.writeUInt32(os, codepage);
        switch (p.getReportOptions()) {
          case ReportNone:
            IOUtils.writeUInt32(os, 0);
            break;
          case ReportFull:
            IOUtils.writeUInt32(os, 1);
            break;
          default:
            IOUtils.writeUInt32(os, 3);
        }

        switch (p.getHide()) {
          case HideEnabled:
            IOUtils.writeUInt32(os, 1);
            break;
          case HideDisabled:
            IOUtils.writeUInt32(os, 0);
            break;
          default:
            IOUtils.writeUInt32(os, 2);
        }

        IOUtils.writeString(os, p.getLocale(), 32);

        IOUtils.writeUInt8(os, p.isHideModifiable() ? 1 : 0);
        IOUtils.writeString(os, p.getDivert(), 32);

        IOUtils.writeUInt8(os, p.isDivertActive() ? 1 : 0);
        IOUtils.writeUInt8(os, p.isDivertActiveAbsent() ? 1 : 0);
        IOUtils.writeUInt8(os, p.isDivertActiveBlocked() ? 1 : 0);
        IOUtils.writeUInt8(os, p.isDivertActiveBarred() ? 1 : 0);
        IOUtils.writeUInt8(os, p.isDivertActiveCapacity() ? 1 : 0);
        IOUtils.writeUInt8(os, p.isDivertModifiable() ? 1 : 0);

        IOUtils.writeUInt8(os, p.isUdhConcat() ? 1 : 0);
        IOUtils.writeUInt8(os, p.isTranslit() ? 1 : 0);

        if (version > 0x00010000) {
          IOUtils.writeUInt32(os, p.getClosedGroupId() == null ? 0 : p.getClosedGroupId());
          IOUtils.writeUInt32(os, p.getAccessMaskIn());
          IOUtils.writeUInt32(os, p.getAccessMaskOut());
          if (smsx) {
            IOUtils.writeUInt32(os, p.getSubscription());
            IOUtils.writeUInt8(os, p.getSponsored());
          }
        }
        if (version >= 0x00010100 && smsx) {
          IOUtils.writeString(os, p.getNick(), 10);
        }
      }
    } finally {
      if (os != null)
        os.close();
    }
  }
}
