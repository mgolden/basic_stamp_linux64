#ifndef __SBPROJECT_H
#define __SBPROJECT_H

//******************************************************************************
// SBProject.h
//
// ASC20021016 created
// ASC20021105 implemented basic functionality
// ASC20021106 get-accessor methods
// ASC20021112 flags as bitset
// ASC20021119 F_MEMORYMAP added
// ASC20030221 F_MEMORYUSAGE added
//******************************************************************************

#include "SBCompiler.h"
#include "SBLoader.h"

class SBProject 
{
public:
  // Compiler Flags
  enum { F_VERBOSE = 1, F_COMPILEONLY = 2, F_DOWNLOADONLY = 4,
    F_IDENTIFYONLY = 8, F_VERSIONINFO = 16, F_IGNPROJECT = 32,
    F_SYNTAXONLY = 64, F_OVRPORT = 128, F_OVRSTAMP = 256,
    F_MEMORYMAP = 512, F_MEMORYUSAGE = 1024 };

  SBProject();
  virtual ~SBProject();

  void setFlags(int n);
  void setSlot(int n);
  void setPort(const string& s);
  void setType(const string& s);
  void setSourceFile(int i, const string& s);
  void setObjectFile(int i, const string& s);

  bool   isFlags(int n);
  bool   isFlagsAnd(int n);
  int    getFlags();
  int    getSlot();
  string getPort();
  string getType();
  string getSourceFile(int i);
  string getObjectFile(int i);

  string makeObjectFileName(const string& src);

  bool process();

private:
  void versionInfo();
  bool identify();
  bool compile();
  bool download();

  void setupCompiler(SBCompiler* pCmp);
  void setupLoader(SBLoader* pLdr);

  string m_sSourceFile[8];
  string m_sObjectFile[8];

  string m_sType;
  string m_sPort;

  int m_nFlags;
  int m_iSlot;
};

#endif

