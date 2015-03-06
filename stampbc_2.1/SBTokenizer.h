#ifndef __SBTOKENIZER_H
#define __SBTOKENIZER_H

//******************************************************************************
// SBTokenizer.h
//
// ASC20021014 created
// ASC20021021 adapted for tokenizer.so v1.16
// ASC20040203 adapted for tokenizer.so v1.23 (PBasic 2.5)
//******************************************************************************

#include "defs.h"

#define TOKENIZER_SO "tokenizer.so"

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
    int		LanguageVersion;
    int		LanguageStart;
    int		SourceSize;
    int		ErrorStart;
    int		ErrorLength;
    byte	EEPROM[2048];
    byte	EEPROMFlags[2048];
    byte	VarCounts[4];
    byte	PacketCount;
    byte	PacketBuffer[PACKET_BUFFER_SZ];
  };

  struct TSrcTokReference {
    word	SrcStart;
    word	TokStart;
  };

  static SBTokenizer* instance();
  static void free();

  bool TestRecAlignment(TModuleRec *Rec);
  bool Compile(TModuleRec *Rec,
    char* Source,
    bool DirectivesOnly,
    bool ParseStampDirective,
    TSrcTokReference* Ref
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
    bool ParseStampDirective,
    TSrcTokReference* Ref
  );
  // is declared as int in the specs for tokenizer v1.16 but is
  // actually a byte.
  byte (*m_Version)();
};

#endif
