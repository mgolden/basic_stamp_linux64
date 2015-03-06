//******************************************************************************
// SBLoader.cc
//
// ASC20021021 created
// ASC20021026 fixed reset problem (ioctl)
//   reset worked only upon the first open after starting a process
// ASC20021112 receive: possible buffer overflow fixed
//   stamp version string introduced
// ASC20021127 tidy up according to feedback
// ASC20021202 namespace std
// ASC20030219 fixed reset pulse problem
// ASC20030417 reworked reset/break condition sequence
// ASC20050513 fixed object buffer size bug
// ASC20050514 preliminary BS2px support
//******************************************************************************

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <iostream>

using namespace std;

#include "defs.h"
#include "SBLoader.h"

SBLoader* SBLoader::m_pInstance = 0;
const int SBLoader::packetSize = 18;

const SBStampID SBLoader::m_stampID[] = {
  {"BS2", "",         STAMPID_BS2,   0},
  {"e",   "e",        STAMPID_BS2E,  8},
  {"X",   "XYZ",      STAMPID_BS2SX, 8},
  {"P",   "pqrsPQRS", STAMPID_BS2P , 8},
  {"I",   "ijkIJK",   STAMPID_BS2PE, 8},
  {"T",   "tT",       STAMPID_BS2PX, 8}, // PRELIMINARY BS2px support
  {0,     0,          0,             0}
// |      |           |              |
// |      |           |              how many memory slots the stamp has
// |      |           type string returned by sType
// |      valid response chars from stamp
// request ID-char to be sent to the stamp
};

const char* SBLoader::m_versionID[][2] = {
  {"e", "v1.0"},
 
  {"X", "v1.0"}, 
  {"Y", "v1.1"}, 
  {"Z", "v1.2"}, 

  {"p", "24 v1.0"}, 
  {"q", "24 v1.1"}, 
  {"r", "24 v1.2"}, 
  {"s", "24 v1.3"}, 
  {"P", "40 v1.0"}, 
  {"Q", "40 v1.1"}, 
  {"R", "40 v1.2"}, 
  {"S", "40 v1.3"}, 

  {"i", "24 v1.0"},
  {"j", "24 v1.1"},
  {"k", "24 v1.2"},
  {"I", "40 v1.0"},
  {"J", "40 v1.1"},
  {"K", "40 v1.2"},

  {"t", "24 v1.0"},
  {"T", "40 v1.0"},

  {0,   0}
// |    |
// |    decoded version infromation
// coded version returned by the stamp
};

#define FP_SAFE		(m_fp != -1)
#define NO_TIMEOUT	(m_sel) 
#define CO_DEBUG	if (m_dbg) CERR

//------------------------------------------------------------------------------
// SBLoader/~SBLoader
//------------------------------------------------------------------------------
SBLoader::SBLoader():
  m_fp(-1),
  m_done(false),
  m_sel(0),
  m_dbg(false),
  m_port("")
{}

SBLoader::~SBLoader()
{
  closeDev();
}

//------------------------------------------------------------------------------
// instance/free
//------------------------------------------------------------------------------
SBLoader* SBLoader::instance()
{
  if (!m_pInstance) {
    m_pInstance = new SBLoader;
  }

  return m_pInstance;
}

void SBLoader::free()
{
  delete m_pInstance;
  m_pInstance = 0;
}

//------------------------------------------------------------------------------
// setPort
//------------------------------------------------------------------------------
void SBLoader::setPort(string sPort)
{
  m_port = sPort;
}

//------------------------------------------------------------------------------
// setDebug
//------------------------------------------------------------------------------
void SBLoader::setDebug(bool dbg)
{
  m_dbg = dbg;
}

//------------------------------------------------------------------------------
// typeToIndex
//------------------------------------------------------------------------------
int SBLoader::typeToIndex(const string& sType)
{
  int i = 0;
  char* type1  = m_stampID[0].id;
  char* pcType = (char*)sType.c_str();

  while (type1 && (strcasecmp(type1, pcType) != 0)) {
    type1 = m_stampID[++i].id;
  }
  return type1 ? i : -1;
}

//------------------------------------------------------------------------------
// versionToVersionID
//------------------------------------------------------------------------------
string SBLoader::versionToVersionID(const string& sVer)
{
  int i(0);
  while (m_versionID[i][0] && strcmp(m_versionID[i][0], sVer.c_str()) != 0) {
    i++;
  }

  return m_versionID[i][0] ? string(m_versionID[i][1]) : sVer;
}

//------------------------------------------------------------------------------
// identify
//------------------------------------------------------------------------------
bool SBLoader::identify(string& sType, string& sVersion, int& index)
{
  static bool once(true);
  index = -1;

  if (sType == "") {
    if (once) {
      CERR << "trying to identify the stamp" << endl;
      once = false;
    }

    // loop through all types and try to find the
    // type of that stamp.
    for (int i = 0; m_stampID[i].send; i++) {
      string type(m_stampID[i].id);
      string version;

      if (identify(type, version, index)) {
        sType = type;
        sVersion = version;
        break;
      }
    }

    if (!m_done) {
      CERR << EYEERR << " no stamp or unable to identify" << ENDL;
    }

    return m_done;
  }
  else {
    if (once) CERR << "identifying the stamp" << endl;

    // try to identify the specified stamp type only.
    index = typeToIndex(sType);

    if (index < 0) {
      CO_DEBUG << EYEERR << " not a valid stamp type: " << sType << ENDL;
      m_done = false;
      return m_done;
    }
  }

  // reset the stamp
#ifdef _BREAK_OLD_FASHION
  resetDev();
  sendBreakCondition();
  m_done = NO_TIMEOUT;
#else
  resetDev(true);
  m_done = true;
#endif

  // identification procedure
  if (m_done) {
    if FP_SAFE CO_DEBUG << EYEINF << " try " << m_stampID[index].id << ENDL;

    sendIdChars(m_stampID[index].send, m_stampID[index].recv,
      sVersion, index == 0);

    if (m_done) {
      sType = string(m_stampID[index].id);
      sVersion = versionToVersionID(sVersion);

      CERR << EYEINF <<  " stamp is a " << sType << ", firmware " << sVersion 
        << ENDL;
    }
    else {
      CO_DEBUG << EYEERR << " could not identify stamp as a " << sType << ENDL;
    }
  }

  return m_done;
}

//------------------------------------------------------------------------------
// fileSize
//------------------------------------------------------------------------------
long SBLoader::fileSize(const string fileName)
{
  struct stat sbuf;
  if (stat(fileName.c_str(), &sbuf) != 0) {
    CERR << EYEERR << " " << strerror(errno) << ": " << fileName << ENDL;
    return -1;
  }

  return sbuf.st_size;
}

//------------------------------------------------------------------------------
// download
//------------------------------------------------------------------------------
bool SBLoader::download(const string objFile, string& sType, int slot)
{
  char objBuffer[PACKET_BUFFER_SZ];
  int objSize(-1);
  int nPack(0);

  if (slot < 0) {
    CERR << "downloading " << objFile << endl;
  }
  else {
    CERR << "downloading " << objFile << " to slot " << slot << endl;
  }

  // check if object file fits into buffer
  long fsz = fileSize(objFile);
  if (fsz > PACKET_BUFFER_SZ) {
    CERR << EYEERR << " object file to big: " << objFile << " (" <<
      fsz << " byte)" << ENDL;
    m_done = false;
    return m_done;
  }
  else if (fsz < 0) {
    m_done = false;
    return m_done;
  }

  // check if slots are requested and available
  int index = typeToIndex(sType);
  if (index < 0) {
    CERR << EYEERR << " not a valid stamp type: " << sType << ENDL;
    m_done = false;
    return m_done;
  }

  int memSlots = m_stampID[index].memSlots;
  if (memSlots <= slot) {
    CERR << EYEERR << 
      " this stamp type does not support the requested memory slot" 
      << ENDL;
    m_done = false;
    return m_done;
  }
  else if ((slot < 0) && (memSlots > 0)) {
    CERR << EYEERR <<
      " this stamp type needs a defined memory slot" 
      << ENDL;
    m_done = false;
    return m_done;
  }

  // load object data file
  FILE* fp = fopen(objFile.c_str(), "r");
  if (fp) {
    char* buf = objBuffer;

    char ch;
    while (!feof(fp)) {
      ch = fgetc(fp);
      *buf++ = ch;
      objSize++;
    }

    nPack = objSize / packetSize;

    fclose(fp);

    CO_DEBUG << EYEINF << " object file loaded " << "(" << objSize <<
      " byte)" << ENDL;
  }
  else {
    CERR << EYEERR << " " << strerror(errno) << ": " << objFile << ENDL;
    m_done = false;
    return m_done;
  }

  // may be there is insifficient data
  if (nPack < 1) {
    CERR << EYEERR << "** not enough data (less than a packet) to download" <<
      ENDL;
    m_done = false;
    return m_done;
  }

  // try to identify stamp
  string sVersion;
  if (!identify(sType, sVersion, index)) {
    if (index < 0) {
      CERR << EYEERR << " " << sType << " is not a known stamp type" << ENDL;
    }
    else {
      CERR << EYEERR << " no stamp or not a " << sType << ENDL;
    }
    return m_done;
  }

  // transmit program slot number
  if ((slot >= 0) && FP_SAFE && m_done) {
    CO_DEBUG << EYEINF << " sending slot number " << slot << ENDL;

    char cSlotEcho;
    char cSlot(slot);
    int n;
    send(&cSlot, 1);
    receive(&cSlotEcho, 1, n);

    m_done = NO_TIMEOUT;
  }

  // download packets
  if (FP_SAFE && m_done) {
    CERR << EYEINF << " downloading " << nPack << " packets" << ENDL;

    char* buf(objBuffer);
    int packnum = 1;
    do {
      downloadPacket(buf, packnum++);
      buf += packetSize;
      nPack--;
    } while (m_done && nPack);
  }
  else {
    m_done = false;
    CERR << EYEERR << " cannot download to unidentified stamp type" << ENDL;
  }
 
  if (m_done) CO_DEBUG << EYEINF << " done" << ENDL;
  return m_done;
}

//------------------------------------------------------------------------------
// resetDev
//------------------------------------------------------------------------------
void SBLoader::resetDev(bool brkcond)
{
  closeDev();
  m_fp = open(m_port.c_str(), O_RDWR);

  if FP_SAFE {
    CO_DEBUG << EYEINF << " opened " << m_port << " as " << m_fp << ENDL;

    int dtr_bit = TIOCM_DTR;

    if (brkcond) {
      CO_DEBUG << EYEINC << " break sequence " << ENDL;

      // form the break sequence according to the specs
      // in "BASIC Stamp Programming Protocol".
      //        __
      // DTR __/  \___________________
      //        __¦_________
      // TX  __/  ¦         \_________ TCIFLUSH
      //       ¦  ¦         ¦        ¦
      //        2     36        20
      // minimum times in ms

      ioctl(m_fp, TIOCMBIC, &dtr_bit); // force L
      usleep(RESET_DTRPULSE_US);
      ioctl(m_fp, TIOCMBIS, &dtr_bit);
      ioctl(m_fp, TIOCSBRK, 0);
      usleep(RESET_DTRPULSE_US);
      ioctl(m_fp, TIOCMBIC, &dtr_bit);
      usleep(BREAK_TXPULSE_US);
      ioctl(m_fp, TIOCCBRK, 0);
      usleep(BREAK_TXRELAX_US);
      tcflush(m_fp, TCIFLUSH);
    }
    else {
      CO_DEBUG << EYEINC << " reset stamp" << ENDL;

      // form the reset pulse on DTR
      // Clear: -12V (physically L)
      // Set:   +12V (physically H)
      //    __
      // __/  \______ H-pulse at ATN resets stamp

      ioctl(m_fp, TIOCMBIC, &dtr_bit); // force L
      usleep(RESET_DTRPULSE_US);
      ioctl(m_fp, TIOCMBIS, &dtr_bit);
      usleep(RESET_DTRPULSE_US);
      ioctl(m_fp, TIOCMBIC, &dtr_bit);
    }
  }
  else {
    CERR << EYEERR << " " << strerror(errno) << " opening " << m_port << ENDL;
  }
}

//------------------------------------------------------------------------------
// closeDev
//------------------------------------------------------------------------------
void SBLoader::closeDev()
{
  if FP_SAFE {
    // it is said that a close might unkillably hang if there is
    // data left behind and tcflush helps.
    tcflush(m_fp, TCIOFLUSH);

    close(m_fp);
    CO_DEBUG << EYEINF << " closed " << m_fp << ENDL;
    m_fp = -1;
  }
}

//------------------------------------------------------------------------------
// waitRecv
//------------------------------------------------------------------------------
void SBLoader::waitRecv(bool bQuiet)
{
  if FP_SAFE {
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(m_fp, &rfds);

    tv.tv_sec  = RECV_TIMEOUT_S;
    tv.tv_usec = RECV_TIMEOUT_US;

    m_sel = select(m_fp + 1, &rfds, 0, 0, &tv);

    if (!bQuiet && !m_sel) CERR << EYEERR << " timeout on " << m_port << ENDL;
  }
}

//------------------------------------------------------------------------------
// receive
//------------------------------------------------------------------------------
void SBLoader::receive(const char* pBuffer, int n, int& nRet, bool bQuiet)
{
  if FP_SAFE {
    nRet = 0;
    int n0(n);
    do {
      waitRecv(bQuiet);

      if (m_sel) {
        int rd = read(m_fp, (void*)pBuffer, n0);
        pBuffer += rd;
        nRet += rd;
        n0 -= rd;
      }
    } while (NO_TIMEOUT && (nRet < n));
  }
}

//------------------------------------------------------------------------------
// send
//------------------------------------------------------------------------------
void SBLoader::send(const char* pBuffer, int n)
{
  if FP_SAFE {
    write(m_fp, pBuffer, n);
  }
}

//------------------------------------------------------------------------------
// sendBreakCondition
//------------------------------------------------------------------------------
void SBLoader::sendBreakCondition()
{
  if FP_SAFE {
    char buf[64];
    int n;

    CO_DEBUG << EYEINF << " sending break condition" << ENDL;

    tcsendbreak(m_fp, 0);

    receive(buf, 1, n, true);

    m_done = true;
  }
}

//------------------------------------------------------------------------------
// sendIdChars
//------------------------------------------------------------------------------
void SBLoader::sendIdChars(const char* toSend, const char* toRecv,
  string& sVer, 
  bool isBS2)
{
  if FP_SAFE {
    const int trshd = 2;
    char cr[32];
    int n;

    CO_DEBUG << EYEINF << " sending identification sequence" << ENDL;

    // ID process for standard BS2 type stamps
    if (isBS2) {
      // try to identify a BS2 type stamp
      while (m_done && (*toSend != '\000')) {
        char cq = 256 - *toSend;

        send(toSend, 1);
        receive(cr, trshd, n, true);

        m_done = (NO_TIMEOUT && (n == trshd) && (cr[1] == cq));
        toSend++;
      }

      // it's a BS2, so get version info
      if (m_done) {
        send("\000", 1);
        receive(cr, trshd, n);

        m_done = (NO_TIMEOUT && (n == trshd));
        if (m_done) {
          char rcVer[4];
          sprintf(rcVer, "%u", cr[1]);
          sVer = string(rcVer);
        }
      }
    }

    // ID process for other BS2 stamps
    else {
      send(toSend, 1);
      receive(cr, trshd, n, true);

      m_done = (NO_TIMEOUT && (n == trshd) && index(toRecv, cr[1]));
      if (m_done) sVer = cr[1];
    }
  }
}

//------------------------------------------------------------------------------
// downloadPacket
//------------------------------------------------------------------------------
void SBLoader::downloadPacket(const char* pBuffer, int packnum)
{
  if (FP_SAFE) {
    CO_DEBUG << EYEINF << " downloading packet " << packnum << ENDL;

    // packet + 1byte-ack
    int trshd(packetSize + 1);
    char inbuf[trshd];
    int n;

    send(pBuffer, packetSize);
    receive(inbuf, trshd, n);

    char ack = inbuf[trshd - 1];
    m_done = (NO_TIMEOUT && (n == trshd) && (ack == 0));

    CO_DEBUG << EYEINF << " packet ack: " << char(ack + '0') << ENDL;

    if (ack != 0) CERR << EYEERR << " packet NAK" << ENDL;
    if (n != trshd) CERR << EYEERR << " data error" << ENDL;
  }
}
