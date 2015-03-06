//******************************************************************************
// SBProject.cc
//
// ASC20021016 created
// ASC20021105 implemented basic functionality
// ASC20021106 get-accessor methods
// ASC20021112 flags as bitset
// ASC20021118 makeObjectFile: implemented more standard like suffix handling
// ASC20021119 F_MEMORYMAP added
// ASC20021127 tidy up according to feedback
// ASC20021202 namespace std
// ASC20030220 F_MEMORYUSAGE added
//******************************************************************************

#include <cstdio>
#include <string>
#include <cstring>
#include <cerrno>
#include <iostream>

using namespace std;

#include "defs.h"
#include "SBProject.h"

//------------------------------------------------------------------------------
// SBProject
//------------------------------------------------------------------------------
SBProject::SBProject():
  m_sType(STAMPID_BS2),
  m_sPort(COMM_DEVICE_DEFAULT),
  m_nFlags(0),
  m_iSlot(-1)
{}

//------------------------------------------------------------------------------
// ~SBCompiler
//------------------------------------------------------------------------------
SBProject::~SBProject()
{
  SBLoader::free();
  SBCompiler::free();
}

//------------------------------------------------------------------------------
// set
//------------------------------------------------------------------------------
void SBProject::setFlags(int n) { m_nFlags = n; }
void SBProject::setSlot(int n) { m_iSlot = n; }
void SBProject::setPort(const string& s) { m_sPort = s; }
void SBProject::setType(const string& s) { m_sType = s; }
void SBProject::setSourceFile(int i, const string& s) { m_sSourceFile[i] = s; }
void SBProject::setObjectFile(int i, const string& s) { m_sObjectFile[i] = s; }

//------------------------------------------------------------------------------
// get
//------------------------------------------------------------------------------
bool   SBProject::isFlags(int n) { return (m_nFlags & n) != 0; }
bool   SBProject::isFlagsAnd(int n) { return (m_nFlags & n) == n; }
int    SBProject::getFlags() { return m_nFlags; }
int    SBProject::getSlot() { return m_iSlot; }
string SBProject::getPort() { return m_sPort; }
string SBProject::getType() { return m_sType; }
string SBProject::getSourceFile(int i) { return m_sSourceFile[i]; }
string SBProject::getObjectFile(int i) { return m_sObjectFile[i]; }

//------------------------------------------------------------------------------
// makeObjectFileName
//------------------------------------------------------------------------------
string SBProject::makeObjectFileName(const string& src)
{
  int ndot = src.rfind('.');
  if (ndot != -1) {
    return src.substr(0, ndot).append(".o");
  }
  else {
    return src + ".o";
  }
}

//------------------------------------------------------------------------------
// process
//------------------------------------------------------------------------------
bool SBProject::process()
{
  if (isFlags(F_VERSIONINFO)) { 
    versionInfo();
    return true;
  }

  if (isFlags(F_IDENTIFYONLY)) {
    return identify();
  }

  bool comp(true);

  if (!isFlags(F_DOWNLOADONLY)) {
    comp = compile();
    if (isFlags(F_COMPILEONLY | F_SYNTAXONLY)) return comp;
  }

  if (comp) return download();

  return false;
}

//------------------------------------------------------------------------------
// identify
//------------------------------------------------------------------------------
bool SBProject::identify()
{
  SBLoader* ldr   = SBLoader::instance();

  setupLoader(ldr);

  string sType;
  string sVersion;
  int index;

  return ldr->identify(sType, sVersion, index);
}

//------------------------------------------------------------------------------
// compile
//------------------------------------------------------------------------------
bool SBProject::compile()
{
  SBCompiler* cmp = SBCompiler::instance();

  setupCompiler(cmp);

  bool directivesOnly(false);
  bool parseStampDirective(true);
  string sType("");
  string sPort("");

  // override stamp directive - use specified type
  if (isFlags(F_OVRSTAMP)) {
    parseStampDirective = false;
    sType = m_sType;
  }

  // compile first source
  bool compiled = cmp->compile(m_sSourceFile[0], m_sObjectFile[0],
    sType, sPort, directivesOnly, parseStampDirective, true);

  if (compiled && isFlags(F_MEMORYMAP | F_MEMORYUSAGE))
    cmp->dump(isFlags(F_MEMORYMAP));

  // use stamp type specified by stamp directive from the source
  if (!isFlags(F_OVRSTAMP)) {
    m_sType = sType;
  }

  // use the port specified by stamp directive from the source
  if (!isFlags(F_OVRPORT) && sPort != "") {
    m_sPort = sPort;
  }

  // compile other sources of this project
  if (compiled && !isFlags(F_IGNPROJECT)) {
    string src;

    int i(1);

    while ((src = cmp->getNextSource()) != "") {
       string obj = makeObjectFileName(src);

       m_sSourceFile[i] = src;
       m_sObjectFile[i] = obj;

       compiled = cmp->compile(src, obj, sType, sPort,
         directivesOnly, parseStampDirective, false);

       if (compiled) {
         if (isFlags(F_MEMORYMAP | F_MEMORYUSAGE))
           cmp->dump(isFlags(F_MEMORYMAP));
       }
       else {
         break;
       }

       i++;
    }
  }

  return compiled;
}

//------------------------------------------------------------------------------
// download
//------------------------------------------------------------------------------
bool SBProject::download()
{
  SBLoader* ldr = SBLoader::instance();
  bool loaded(true);

  setupLoader(ldr);

  bool single = m_sObjectFile[1] == "";

  if (isFlags(F_DOWNLOADONLY) || single) {
    loaded = ldr->download(m_sObjectFile[0], m_sType, m_iSlot);
  }
  else {
    for (int i = 0; i < 8 && loaded; i++) {
      string obj = m_sObjectFile[i];
      if (obj == "") break;

      loaded = ldr->download(m_sObjectFile[i], m_sType, i);
    }
  }

  return loaded;
}

//------------------------------------------------------------------------------
// versionInfo
//------------------------------------------------------------------------------
void SBProject::versionInfo()
{
  SBCompiler* cmp = SBCompiler::instance();

  setupCompiler(cmp);

  CERR << STAMPBC_VERSION << ", ";
  cmp->printVersionInfo();
}

//------------------------------------------------------------------------------
// setupCompiler
//------------------------------------------------------------------------------
void SBProject::setupCompiler(SBCompiler* pCmp)
{
  if (isFlags(F_SYNTAXONLY)) {
    m_sObjectFile[0] = "";
  }

  else if (m_sObjectFile[0] == "") {
    m_sObjectFile[0] = makeObjectFileName(m_sSourceFile[0]);
  }

  pCmp->setDebug(isFlags(F_VERBOSE));
}

//------------------------------------------------------------------------------
// setupLoader
//------------------------------------------------------------------------------
void SBProject::setupLoader(SBLoader* pLdr)
{
  pLdr->setPort(m_sPort);
  pLdr->setDebug(isFlags(F_VERBOSE));
}
