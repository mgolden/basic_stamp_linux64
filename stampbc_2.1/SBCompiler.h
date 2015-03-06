#ifndef __SBCOMPILER_H
#define __SBCOMPILER_H

//******************************************************************************
// SBCompiler.h
//
// ASC20021014 created
// ASC20021021 adapted for tokenizer.so v1.16
// ASC20021119 dump methods added
//******************************************************************************

#include "defs.h"
#include "SBTokenizer.h"

#define PACKETSIZE 18

class SBCompiler 
{
public:
  static SBCompiler* instance();
  static void free();

  bool testRecAlignment();
  void printVersionInfo();
  bool compile(
    const string& srcFile,
    const string& objFile, 
    string& sType,
    string& sPort,
    bool directivesOnly = false, 
    bool parseStampDirective = true,
    bool isProject = true
  );
  void dump(bool bMap);
  string getNextSource();
  void setDebug(bool b);
  string moduleIDToStampID(int mid);
  int stampIDToModuleID(string sid);

private:
  // size of the EEPROM array
  enum {EEsz = 2048};

  // EEPROM bitsets & flags
  enum {used = 0x80, usage = 0x7f};
  enum {empty = 0, undefined = 1, defined = 2, program = 3};
  
  SBCompiler();
  virtual ~SBCompiler();

  int readSourceFile(const string& srcFile, char* srcBuffer);
  int writeObjectFile(const string& objFile, byte* pckBuffer, int pckCount);
  void printOffendingLine(SBTokenizer::TModuleRec *rec, char* src);
  void dumpEEPROMLayout();
  void showPcntUsage();
  void dumpEEPROM();
  void dumpVariables();
  void dumpEtc();

  static SBCompiler* m_pInstance;
  static const char* m_stampID[];

  SBTokenizer*             m_pTk;
  int                      m_index;
  string                   m_sSource[7];
  bool                     m_dbg;
  SBTokenizer::TModuleRec* m_pRec;
};

#endif

