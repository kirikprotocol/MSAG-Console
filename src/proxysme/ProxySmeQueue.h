//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#if !defined PROXY_SME_QUUE_HEADER
#define PROXY_SME_QUUE_HEADER

#include "proxysme.h"
#include <deque>
#include <stdexcept>
#include <memory>
#include "../smpp/smpp.h"
#include "../core/synchronization/Mutex.hpp"

SMSC_SMEPROXY_BEGIN

using namespace smsc::smpp;
using namespace smsc::core::synchronization;


enum DIRECTION { LEFT_TO_RIGHT, RIGHT_TO_LEFT, INVALID_DIRECTION };

/// �������
struct QCommand {
  DIRECTION   direction_;     /// ����������� �������
  SmppHeader* pdu_;           /// ������������ �����
  QCommand() : direction_(INVALID_DIRECTION), pdu_(0) {}
  ~QCommand() { if ( pdu_ ) disposePdu(pdu_); }
private:
  QCommand(const QCommand&);
  QCommand& operator = (const QCommand&);
};

/// ����������, ��������� ��� ���������� ������ �� �������
struct QueueLimitExceeded : public std::runtime_error {
  QueueLimitExceeded(const char* text = "SPQ::QueueLimitExceeded") :
    runtime_error(text) {}
};

/// ������� ������ 
class Queue {

  typedef std::deque<QCommand*> QUEUE; /// ��������� ������ ��� �������

  int   qlimit_;  /// ������ ����� �������
  QUEUE queue_;   /// ������� ������
  Mutex mutex_;   /// ������ �������������

public:

  /// ������������� ����� �� ����� �������
  void SetLimit(int limit);
  /// �������� ������� � ����� ������� (�������� ��������� � �������)
  bool PutBack(QCommand* cmd);
  /// ���������� ��������� �� ������ ������� � ������� 
  /// (�������� �������� � �������)
  QCommand* Top();
  /// ���������� ��������� �� ������ ������� � ������� � ������� �� �� ������� 
  /// (�������� ��������� �������)
  QCommand* Next();
  /// ������� ������� �� ������� � ���������� ������ - �������
  bool Pop(); 

  Queue(const ProxyConfig&);
  ~Queue();

};

SMSC_SMEPROXY_END


#endif // PROXY_SME_QUUE_HEADER



