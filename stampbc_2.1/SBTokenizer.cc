//******************************************************************************
// SBTokenizer.cc
//
// ASC20021014 created
// ASC20021021 adapted for tokenizer.so v1.16
// ASC20021127 tidy up according to feedback
// ASC20021202 namespace std
// ASC20030219 fix for problem where ATN held H after reset
//******************************************************************************

#include <dlfcn.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <iostream>

using namespace std;

#include "defs.h"
#include "SBTokenizer.h"

SBTokenizer* SBTokenizer::m_pInstance = 0;

//------------------------------------------------------------------------------
// SBTokenizer
//------------------------------------------------------------------------------
SBTokenizer::SBTokenizer() :
  m_inited(false)
{
  m_inited = init();
}

//------------------------------------------------------------------------------
// ~SBTokenizer
//------------------------------------------------------------------------------
SBTokenizer::~SBTokenizer()
{}

//------------------------------------------------------------------------------
// instance
//------------------------------------------------------------------------------
SBTokenizer* SBTokenizer::instance()
{
  if (!m_pInstance) {
    m_pInstance = new SBTokenizer;
  }

  return m_pInstance;
}

//------------------------------------------------------------------------------
// free
//------------------------------------------------------------------------------
void SBTokenizer::free()
{
  delete m_pInstance;
  m_pInstance = 0;
}

//------------------------------------------------------------------------------
// init
//------------------------------------------------------------------------------
bool SBTokenizer::init()
{
  m_libt = dlopen(TOKENIZER_SO, RTLD_LAZY);

  if (m_libt) {
    void* p(0);

    p = dlsym(m_libt, "TestRecAlignment");
    m_TestRecAlignment = (bool (*)(TModuleRec*))p;

    p = dlsym(m_libt, "Compile");
    m_Compile = (bool (*)(TModuleRec*, char*, unsigned char, unsigned char, SBTokenizer::TSrcTokReference*))p;

    p = dlsym(m_libt, "Version");
    m_Version = (byte (*)())p;

    return true;
  }
  else {
    CERR << EYEERR << " " << strerror(errno) << " opening " <<
      TOKENIZER_SO << ENDL;
    return false;
  }
}

//------------------------------------------------------------------------------
// notloaded
//------------------------------------------------------------------------------
void SBTokenizer::notloaded()
{
  CERR << EYEERR << " shared library " << TOKENIZER_SO <<
    " not loaded" << ENDL;
}

//------------------------------------------------------------------------------
// TestRecAlignment
//------------------------------------------------------------------------------
bool SBTokenizer::TestRecAlignment(TModuleRec *Rec)
{
  bool bRet(false);

  if (m_inited) {
    bRet = (*m_TestRecAlignment)(Rec);
  }
  else {
    notloaded();
  }

  return bRet;
}

//------------------------------------------------------------------------------
// Compile
//------------------------------------------------------------------------------
bool SBTokenizer::Compile(TModuleRec *Rec,
  char* Source,
  bool DirectivesOnly,
  bool ParseStampDirective,
  TSrcTokReference* Ref)
{
  bool bRet(false);

  if (m_inited) {
    bRet = (*m_Compile)(Rec, Source, DirectivesOnly, ParseStampDirective, Ref);
  }
  else {
    notloaded();
  }

  return bRet;
}

//------------------------------------------------------------------------------
// Version
//------------------------------------------------------------------------------
byte SBTokenizer::Version()
{
  byte nRet(0);

  if (m_inited) {
    nRet = (*m_Version)();
  }
  else {
    notloaded();
  }

  return nRet;
}

