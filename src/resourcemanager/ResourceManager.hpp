#ifndef __RESOURCE_MANAGER_HPP__
#define __RESOURCE_MANAGER_HPP__

#include <string>

namespace smsc{
namespace resourcemanager{

class ResourceManager{
public:
  // �����頥� ��ப� �� �⨭��� ��� ��।������� ������ � ����.
  string getSetting(const string& locale,const string& key);
  // �����頥� ��ப� �� �⨭��� ��� ��䮫⭮� ������ � ����.
  string getSetting(const string& key );
  // �����頥� ��ப� �� ����� ��� ��।������� ������ � ����.
  string getString(const string& locale, const string& key);
  // �����頥� ��ப� �� ����� ��� ��䮫⭮� ������ � ����.
  string getString(const string& key);

  static ResourceManager& getInstance();
};

};//resourcemanager
};//smsc

#endif
