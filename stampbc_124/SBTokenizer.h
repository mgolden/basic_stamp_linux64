#ifndef __SBTOKENIZER_H
#define __SBTOKENIZER_H

//******************************************************************************
// SBTokenizer.h
//
// ASC20021014 created
// ASC20021021 adapted for tokenizer.so v1.16
//******************************************************************************

#include "defs.h"

#define TOKENIZER_SO "tokenizer_1.16.so"

class SBTokenizer {
public:
  struct TModuleRec {
    bool	Succeded;
    char*	Error;
    bool	DebugFlag;
    byte	TargetModule;
    int		TargetStart;
    char*	ProjectFiles[7];
    int		ProjectFilesStart[7];
    char*	Port;
    int		PortStart;
    int		SourceSize;
    int		ErrorStart;
    int		ErrorLength;
    byte	EEPROM[2048];
    byte	EEPROMFlags[2048];
    byte	VarCounts[4];
    byte	PacketCount;
    byte	PacketBuffer[2304];
  };

  static SBTokenizer* instance();
  static void free();

  bool TestRecAlignment(TModuleRec *Rec);
  bool Compile(TModuleRec *Rec,
    char* Source,
    bool DirectivesOnly,
    bool ParseStampDirective
  );
  byte Version();

private:
  SBTokenizer();
  virtual ~SBTokenizer();

  bool init();
  void notloaded();

  static SBTokenizer* m_pInstance;

  bool m_inited;

  void *m_libt;
  bool (*m_TestRecAlignment)(TModuleRec *Rec);
  bool (*m_Compile)(
    TModuleRec *Rec,
    char* Source,
    bool DirectivesOnly,
    bool ParseStampDirective
  );
  // is declared as int in the specs for tokenizer v1.16 but is
  // actually a byte.
  byte (*m_Version)();
};

#endif
