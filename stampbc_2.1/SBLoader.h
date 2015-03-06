#ifndef __SBLOADER_H
#define __SBLOADER_H

//******************************************************************************
// SBLoader.h
//
// ASC20021021 created
// ACS20021112 stamp version string introduced
//******************************************************************************

#include <string>

struct SBStampID {
  char* send;
  char* recv;
  char* id;
  int   memSlots;
};

class SBLoader
{
public:
  static const int packetSize;

  static SBLoader* instance();
  static void free();

  void setPort(string sPort);
  void setDebug(bool dbg);

  int typeToIndex(const string& sType);
  string versionToVersionID(const string& sVer);

  bool identify(string& sType, string& sVersion, int& index);
  bool download(const string objFile, string& sType, int slot);

private:
  SBLoader();
  virtual ~SBLoader();

  long fileSize(const string fileName);
  void resetDev(bool brkcond = false);
  void closeDev();
  void waitRecv(bool bQuiet);
  void receive(const char* pBuffer, int n, int& nReti, bool bQuiet = false);
  void send(const char* pBuffer, int n);
  void sendBreakCondition();
  void sendIdChars(const char* toSend, const char* toRecv, string& sVer,
    bool isBS2);
  void downloadPacket(const char* pBuffer, int packnum);

  int  m_fp;
  bool m_done;
  int  m_sel;
  bool m_dbg;

  string m_port;

  static SBLoader* m_pInstance;
  static const SBStampID m_stampID[];
  static const char* m_versionID[][2];
};

#endif
