/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include <macosx.h>

#include "kernel_patcher.h"
#if 0
#include "LoaderUefi.h"
#include "device_tree.h"
#endif

//
// Searches Source for Search pattern of size SearchSize
// and returns pointer to found occurence or NULL.
//
UINT8 *
SearchMemory (
  UINT8   *Source,
  UINT32  SourceSize,
  CHAR8   *Search,
  UINTN   SearchSize
)
{
  UINT8     *End;

  ASSERT ((Source != NULL && SourceSize > 0));
  ASSERT ((Search != NULL && SearchSize > 0));

  End = Source + SourceSize - SearchSize;

  while (Source <= End) {   
    if (CompareMem (Source, Search, SearchSize) == 0) {
      return Source;
    } else {
      Source++;
    }
  }
  return NULL;
}

//
// Searches Source for Search pattern of size SearchSize
// and returns the number of occurences.
//
UINTN
SearchAndCount (
  UINT8   *Source,
  UINT32  SourceSize,
  CHAR8   *Search,
  UINTN   SearchSize
)
{
  UINTN     NumFounds;
  UINT8     *End;

  NumFounds = 0;
  End = Source + SourceSize - SearchSize;

  while (Source <= End) {   
    if (CompareMem (Source, Search, SearchSize) == 0) {
      NumFounds++;
      Source += SearchSize;
    } else {
      Source++;
    }
  }
  return NumFounds;
}

//
// Searches Source for Search pattern of size SearchSize
// and replaces it with Replace up to MaxReplaces times.
// If MaxReplaces <= 0, then there is no restriction on number of replaces.
// Replace should have the same size as Search. 
// Returns number of replaces done.
//
UINTN
SearchAndReplace (
  UINT8   *Source,
  UINT32  SourceSize,
  CHAR8   *Search,
  UINTN   SearchSize,
  CHAR8   *Replace,
  INTN    MaxReplaces
)
{
  UINTN     NumReplaces;
  BOOLEAN   NoReplacesRestriction;
  UINT8     *End;

  NumReplaces = 0;
  NoReplacesRestriction = MaxReplaces <= 0;
  End = Source + SourceSize - SearchSize;

  while (Source <= End && (NoReplacesRestriction || MaxReplaces > 0)) {   
    if (CompareMem (Source, Search, SearchSize) == 0) {
#ifdef KERNEL_PATCH_DEBUG
      Print (L"%a: found at 0x%x.\n", __FUNCTION__, Source);
#endif
      CopyMem (Source, Replace, SearchSize);
      NumReplaces++;
      MaxReplaces--;
      Source += SearchSize;
    } else {
      Source++;
    }
  }
  return NumReplaces;
}

/** Global for storing KextBundleIdentifier */

CHAR8 gKextBundleIdentifier[256];
UINT32 gKextBundleIdLength;

/** Extracts kext BundleIdentifier from given Plist into gKextBundleIdentifier */

VOID
ExtractKextBundleIdentifier (
  CHAR8   *Plist,
  UINT32  PlistSize
)
{
  UINT8     *Tag;
  UINT8     *BIStart;
  UINT8     *BIEnd;
  INTN      DictLevel;
  
  DictLevel = 0;
  gKextBundleIdentifier[0] = '\0';
  gKextBundleIdLength = 0;
  
  // start with first <dict>
  Tag = SearchMemory ((UINT8 *)Plist, PlistSize, "<dict>", 6);
  if (Tag == NULL) {
    return;
  }
  
  Tag += 6;
  DictLevel++;
  
  while (*Tag != '\0') {
    if (CompareMem (Tag, "<dict>", 6) == 0) {
      // opening dict
      DictLevel++;
      Tag += 6;
    } else if (CompareMem (Tag, "</dict>", 7) == 0) {
      // closing dict
      DictLevel--;
      Tag += 7;
    } else if ((DictLevel == 1) && (CompareMem (Tag, "<key>CFBundleIdentifier</key>", 29) == 0)) {
      // BundleIdentifier is next <string>...</string>
      BIStart = SearchMemory (Tag + 29, (UINT32) (PlistSize - 29 - (Tag - (UINT8 *)Plist)), "<string>", 8);
      if (BIStart != NULL) {
        BIStart += 8; // skip "<string>"
        BIEnd = SearchMemory (BIStart, (UINT32) (PlistSize - (BIStart - (UINT8 *)Plist)), "</string>", 9);
        gKextBundleIdLength = (UINT32) (BIEnd - BIStart);
        if (BIEnd != NULL && gKextBundleIdLength < sizeof (gKextBundleIdentifier)) {
          CopyMem (gKextBundleIdentifier, BIStart, gKextBundleIdLength);
          gKextBundleIdentifier[gKextBundleIdLength] = '\0';
          return;
        }
      }
      Tag++;
    } else {
      Tag++;
    }
    
    // advance to next tag
    while ((*Tag != '<') && (*Tag != '\0')) {
      Tag++;
    }
  }
}

#if 0
////////////////////////////////////
//
// ATIConnectors patch
//
// bcc9's patch: http://www.insanelymac.com/forum/index.php?showtopic=249642
//

// inited or not?
BOOLEAN ATIConnectorsPatchInited = FALSE;

// ATIConnectorsController's boundle IDs for
// 0: ATI version - Lion, SnowLeo 10.6.7 2011 MBP
// 1: AMD version - ML
CHAR8 ATIKextBundleId[2][64];

//
// Inits patcher: prepares ATIKextBundleIds.
//
VOID
ATIConnectorsPatchInit (
  VOID
)
{
  //
  // prepar boundle ids
  //
  
  // Lion, SnowLeo 10.6.7 2011 MBP
  AsciiSPrint (
    ATIKextBundleId[0],
    sizeof(ATIKextBundleId[0]),
    "com.apple.kext.ATI%sController",
    gSettings.KPATIConnectorsController
  );
  // ML
  AsciiSPrint (
    ATIKextBundleId[1],
    sizeof (ATIKextBundleId[1]),
    "com.apple.kext.AMD%sController",
    gSettings.KPATIConnectorsController
  );

  ATIConnectorsPatchInited = TRUE;
}

#if 0
//
// Registers kexts that need force-load during WithKexts boot. 
//
VOID
ATIConnectorsPatchRegisterKexts (
  FSINJECTION_PROTOCOL  *FSInject,
  FSI_STRING_LIST       *ForceLoadKexts
)
{
  // for future?
  FSInject->AddStringToList (
              ForceLoadKexts,
              PoolPrint(L"\\AMD%sController.kext\\Contents\\Info.plist", gSettings.KPATIConnectorsController)
            );
  // Lion, ML, SnowLeo 10.6.7 2011 MBP
  FSInject->AddStringToList (
              ForceLoadKexts,
              PoolPrint(L"\\ATI%sController.kext\\Contents\\Info.plist", gSettings.KPATIConnectorsController)
            );
  // SnowLeo
  FSInject->AddStringToList (ForceLoadKexts, L"\\ATIFramebuffer.kext\\Contents\\Info.plist");
  
  // dependencies
  FSInject->AddStringToList (ForceLoadKexts, L"\\IOGraphicsFamily.kext\\Info.plist");
  FSInject->AddStringToList (ForceLoadKexts, L"\\ATISupport.kext\\Contents\\Info.plist");
}
#endif

//
// Patch function.
//
VOID
ATIConnectorsPatch (
  UINT8   *Driver,
  UINT32  DriverSize,
  CHAR8   *InfoPlist,
  UINT32  InfoPlistSize
)
{
  UINTN   Num;

  Num = 0;
  
  ExtractKextBundleIdentifier (InfoPlist, InfoPlistSize);
  // number of Data occurences must be 1
  Num = SearchAndCount (Driver, DriverSize, gSettings.KPATIConnectorsData, gSettings.KPATIConnectorsDataLen);
  if (Num != 1) {
    return;
  }
  
  // patch
  Num = SearchAndReplace (
          Driver,
          DriverSize,
          gSettings.KPATIConnectorsData,
          gSettings.KPATIConnectorsDataLen,
          gSettings.KPATIConnectorsPatch,
          1
        );
}

////////////////////////////////////
//
// AsusAICPUPM patch
//
// fLaked's SpeedStepper patch for Asus (and some other) boards:
// http://www.insanelymac.com/forum/index.php?showtopic=258611
//
// Credits: Samantha/RevoGirl/DHP
// http://www.insanelymac.com/forum/topic/253642-dsdt-for-asus-p8p67-m-pro/page__st__200#entry1681099
//
// 

UINT8   MovlE2ToEcx[] = { 0xB9, 0xE2, 0x00, 0x00, 0x00 };
UINT8   Wrmsr[]       = { 0x0F, 0x30 };

VOID
AsusAICPUPMPatch (
  UINT8   *Driver,
  UINT32  DriverSize,
  CHAR8   *InfoPlist,
  UINT32  InfoPlistSize
)
{
  UINTN   Index1;
  UINTN   Index2;
  UINTN   Count;

  Count = 0;

  //TODO: we should scan only __text __TEXT
  for (Index1 = 0; Index1 < DriverSize; Index1++) {
    // search for MovlE2ToEcx
    if (CompareMem ((Driver + Index1), MovlE2ToEcx, sizeof (MovlE2ToEcx)) == 0) {
      // search for wrmsr in next few bytes
      for (Index2 = (Index1 + sizeof (MovlE2ToEcx)); Index2 < (Index1 + sizeof (MovlE2ToEcx) + 16); Index2++) {
        if ((Driver[Index2] == Wrmsr[0]) && (Driver[Index2 + 1] == Wrmsr[1])) {
          // found it - patch it with nops
          Count++;
          Driver[Index2] = 0x90;
          Driver[Index2 + 1] = 0x90;
        }
      }
    }
  }
}

////////////////////////////////////
//
// AppleRTC patch to prevent CMOS reset
//
// http://www.insanelymac.com/forum/index.php?showtopic=253992
// http://www.insanelymac.com/forum/index.php?showtopic=276066
// 

UINT8   LionSearch_X64[]  = { 0x75, 0x30, 0x44, 0x89, 0xf8 };
UINT8   LionReplace_X64[] = { 0xeb, 0x30, 0x44, 0x89, 0xf8 };

UINT8   LionSearch_i386[]  = { 0x75, 0x3d, 0x8b, 0x75, 0x08 };
UINT8   LionReplace_i386[] = { 0xeb, 0x3d, 0x8b, 0x75, 0x08 };

UINT8   MLSearch[]  = { 0x75, 0x30, 0x89, 0xd8 };
UINT8   MLReplace[] = { 0xeb, 0x30, 0x89, 0xd8 };

//
// We can not rely on OSVersion global variable for OS version detection,
// since in some cases it is not correct (install of ML from Lion, for example).
// So, we'll use "brute-force" method - just try to patch.
// Actually, we'll at least check that if we can find only one instance of code that
// we are planning to patch.
//
VOID
AppleRTCPatch (
  UINT8   *Driver,
  UINT32  DriverSize,
  CHAR8   *InfoPlist,
  UINT32  InfoPlistSize
)
{

  UINTN   Num;
  UINTN   NumLion_X64;
  UINTN   NumLion_i386;
  UINTN   NumML;
  
  Num = 0;
  NumLion_X64 = 0;
  NumLion_i386 = 0;
  NumML = 0;

  if (is64BitKernel) {
    NumLion_X64 = SearchAndCount (Driver, DriverSize, LionSearch_X64, sizeof (LionSearch_X64));
    NumML = SearchAndCount (Driver, DriverSize, MLSearch, sizeof (MLSearch));
  } else {
    NumLion_i386 = SearchAndCount (Driver, DriverSize, LionSearch_i386, sizeof (LionSearch_i386));
  }
  
  if (NumLion_X64 + NumLion_i386 + NumML > 1) {
    // more then one pattern found - we do not know what to do with it
    // and we'll skipp it
    return;
  }
  
  if (NumLion_X64 == 1) {
    Num = SearchAndReplace (Driver, DriverSize, LionSearch_X64, sizeof (LionSearch_X64), LionReplace_X64, 1);
  } else if (NumLion_i386 == 1) {
    Num = SearchAndReplace (Driver, DriverSize, LionSearch_i386, sizeof (LionSearch_i386), LionReplace_i386, 1);
  } else if (NumML == 1) {
    Num = SearchAndReplace (Driver, DriverSize, MLSearch, sizeof (MLSearch), MLReplace, 1);
  }
}

////////////////////////////////////
//
// Place other kext patches here
//
// ...
#endif

////////////////////////////////////
//
// Generic kext patch functions
//
//
VOID
AnyKextPatch (
  UINT8   *Driver,
  UINT32  DriverSize,
  CHAR8   *InfoPlist,
  UINT32  InfoPlistSize,
  INT32   N
)
{
  UINTN   Num;
  
  if (gSettings.AnyKextInfoPlistPatch[N]) {
    if (InfoPlist == NULL || InfoPlistSize == 0) {
      DEBUG ((DEBUG_INFO, "%a: kext plist is not good (0x%p, %d)\n", __FUNCTION__, InfoPlist, InfoPlistSize));
      return;
    }
    // Info plist patch
    Num = SearchAndReplace (
            (UINT8 *) InfoPlist,
            InfoPlistSize,
            gSettings.AnyKextData[N],
            gSettings.AnyKextDataLen[N],
            gSettings.AnyKextPatch[N],
            -1
          );
    DBG ("%a: patch #%d (%a) applied %d times\n", __FUNCTION__, N + 1, gSettings.AnyKext[N], Num);
#ifdef KEXT_PATCH_DEBUG
    Print (L"%a: patch #%d (%a) applied %d times\n", __FUNCTION__, N + 1, gSettings.AnyKext[N], Num);
#endif
    return;
  }

  if (Driver == NULL || DriverSize == 0) {
      DEBUG ((DEBUG_INFO, "%a: kext binary is not good (0x%p, %d)\n", __FUNCTION__, Driver, DriverSize));
    return;
  }
  // kext binary patch
  Num = SearchAndReplace (
          Driver,
          DriverSize,
          gSettings.AnyKextData[N],
          gSettings.AnyKextDataLen[N],
          gSettings.AnyKextPatch[N],
          -1
        );
  DBG ("%a: patch #%d (%a) applied %d times\n", __FUNCTION__, N + 1, gSettings.AnyKext[N], Num);
#ifdef KEXT_PATCH_DEBUG
  Print (L"%a: patch #%d (%a) applied %d times\n", __FUNCTION__, N + 1, gSettings.AnyKext[N], Num);
#endif
}

#if 0
//
// Called from SetFSInjection(), before boot.efi is started,
// to allow patchers to prepare FSInject to force load needed kexts.
//
VOID
KextPatcherRegisterKexts (
  FSINJECTION_PROTOCOL  *FSInject,
  FSI_STRING_LIST       *ForceLoadKexts
)
{
  INTN i;
  
  if (gSettings.KPATIConnectorsController != NULL) {
    ATIConnectorsPatchRegisterKexts (FSInject, ForceLoadKexts);
  }
  
  for (i = 0; i < gSettings.NrKexts; i++) {
    FSInject->AddStringToList(ForceLoadKexts,
                PoolPrint(L"\\%a.kext\\Contents\\Info.plist",
                gSettings.AnyKext[i])
              );
  }

}
#endif

//
// PatchKext is called for every kext from prelinked kernel (kernelcache) or from DevTree (booting with drivers).
// Add kext detection code here and call kext speciffic patch function.
//
VOID
PatchKext (
  UINT8   *Driver,
  UINT32  DriverSize,
  CHAR8   *InfoPlist,
  UINT32  InfoPlistSize
)
{
  UINT32 i;

#if 0
  if (gSettings.KPATIConnectorsController != NULL) {
    //
    // ATIConnectors
    //
    if (!ATIConnectorsPatchInited) {
      ATIConnectorsPatchInit ();
    }
    if (AsciiStrStr (InfoPlist, ATIKextBundleId[0]) != NULL ||             // ATI boundle id
        AsciiStrStr (InfoPlist, ATIKextBundleId[1]) != NULL ||             // AMD boundle id
        AsciiStrStr (InfoPlist, "com.apple.kext.ATIFramebuffer") != NULL) { // SnowLeo
      ATIConnectorsPatch (Driver, DriverSize, InfoPlist, InfoPlistSize);
      return;
    }
  }
    
  if (gSettings.KPAsusAICPUPM &&
      AsciiStrStr (InfoPlist, "<string>com.apple.driver.AppleIntelCPUPowerManagement</string>") != NULL) {
    //
    // AsusAICPUPM
    //
    AsusAICPUPMPatch (Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else if (gSettings.KPAppleRTC && AsciiStrStr (InfoPlist, "com.apple.driver.AppleRTC") != NULL) {
    //
    // AppleRTC
    //
    AppleRTCPatch (Driver, DriverSize, InfoPlist, InfoPlistSize);
  } else {
    //
    //others
    //
  }
#endif

  ExtractKextBundleIdentifier (InfoPlist, InfoPlistSize);
  DBG ("%a: kext (%a)\n", __FUNCTION__, gKextBundleIdentifier);

  for (i = 0; i < gSettings.NrKexts; i++) {
    UINT32 namLen;

    if (gSettings.AnyKextDataLen[i] < 1) {
      DBG ("%a: bizzare patch #%d\n", __FUNCTION__, i);
      continue;
    }
    namLen = (UINT32) AsciiStrLen (gSettings.AnyKext[i]);
    if (SearchMemory ((UINT8 *)gKextBundleIdentifier, gKextBundleIdLength, gSettings.AnyKext[i], namLen) == NULL) {
      continue;
    }

    AnyKextPatch (Driver, DriverSize, InfoPlist, InfoPlistSize, i);
  }    
}

//
// Returns parsed hex integer key.
// Plist - kext pist
// Key - key to find
// WholePlist - _PrelinkInfoDictionary, used to find referenced values
//
// Searches for Key in Plist and it's value:
// a) <integer ID="26" size="64">0x2b000</integer>
//    returns 0x2b000
// b) <integer IDREF="26"/>
//    searches for <integer ID="26"... from WholePlist
//    and returns value from that referenced field
//
// Whole function is here since we should avoid ParseXML() and it's
// memory allocations during ExitBootServices(). And it seems that 
// ParseXML() does not support IDREF.
// This func is hard to read and debug and probably not reliable,
// but it seems it works.
//
UINT64
GetPlistHexValue (
  CHAR8  *Plist,
  UINT32 PlistSize,
  CHAR8  *Key,
  CHAR8  *WholePlist,
  UINT32 WholeSize
)
{
  UINT8     *Value;
  UINT8     *IntTag;
  UINT64    NumValue;
  UINT8     *IDStart;
  UINT8     *IDEnd;
  UINTN     IDLen;
  CHAR8     Buffer[48];

  NumValue = 0;

  // search for Key
  Value = SearchMemory ((UINT8 *)Plist, PlistSize, Key, AsciiStrLen(Key));
  if (Value == NULL) {
    return 0;
  }
  // search for <integer
  IntTag = SearchMemory (Value, (UINT32) (Value - (UINT8 *)Plist), "<integer", 8);
  if (IntTag == NULL) {
    return 0;
  }
  // find <integer end
  Value = SearchMemory (IntTag, (UINT32) (IntTag - (UINT8 *)Plist),  ">", 1);
  if (Value == NULL) {
    return 0;
  }
  // normal case: value is here
  if (Value[-1] != '/') {
    NumValue = AsciiStrHexToUint64 ((CHAR8 *)(Value + 1));
    return NumValue;
  }
  // it might be a reference: IDREF="173"/>
  Value = SearchMemory (IntTag, (UINT32) (IntTag - (UINT8 *)Plist), "<integer IDREF=\"", 16);
  if (Value != IntTag) {
    return 0;
  }
  // compose <integer ID="xxx" in the Buffer
  IDStart = SearchMemory (IntTag, (UINT32) (IntTag - (UINT8 *)Plist),  "\"", 1) + 1;
  IDEnd = SearchMemory (IDStart, (UINT32) (IDStart - (UINT8 *)Plist),  "\"", 1);
  IDLen = IDEnd - IDStart;
  if (IDLen > 8) {
    return 0;
  }
  AsciiStrCpyS (Buffer, sizeof (Buffer), "<integer ID=\"");
  AsciiStrnCatS (Buffer, sizeof (Buffer), (CHAR8 *)IDStart, IDLen);
  AsciiStrCatS (Buffer, sizeof (Buffer), "\"");
  // and search whole plist for ID
  IntTag = SearchMemory ((UINT8 *)WholePlist, WholeSize, Buffer, AsciiStrLen (Buffer));
  if (IntTag == NULL) {
    return 0;
  }
  // got it. find closing >
  Value = SearchMemory (IntTag, (UINT32) (WholeSize - (IntTag - (UINT8 *)WholePlist)), ">", 1);
  if (Value == NULL) {
    return 0;
  }
  if (Value[-1] == '/') {
    return 0;
  }
  // ...Str..() functions like to call StrLen() with safety belts.
  // Our datum is huge sometimes so reduce the risk.
  CopyMem (Buffer, Value + 1, 20);
  Buffer[20] = '\0';
  // we should have value now
  NumValue = AsciiStrHexToUint64 (Buffer);
  
  return NumValue;
}

//
// Iterates over kexts in kernelcache
// and calls PatchKext() for each.
//
// PrelinkInfo section contains following plist, without spaces:
// <dict>
//   <key>_PrelinkInfoDictionary</key>
//   <array>
//     <!-- start of kext Info.plist -->
//     <dict>
//       <key>CFBundleName</key>
//       <string>MAC Framework Pseudoextension</string>
//       <key>_PrelinkExecutableLoadAddr</key>
//       <integer size="64">0xffffff7f8072f000</integer>
//       <!-- Kext size -->
//       <key>_PrelinkExecutableSize</key>
//       <integer size="64">0x3d0</integer>
//       <!-- Kext address -->
//       <key>_PrelinkExecutableSourceAddr</key>
//       <integer size="64">0xffffff80009a3000</integer>
//       ...
//     </dict>
//     <!-- start of next kext Info.plist -->
//     <dict>
//       ...
//     </dict>
//       ...
VOID
PatchPrelinkedKexts (
  VOID
)
{
  CHAR8     *WholePlist;
  CHAR8     *DictPtr;
  CHAR8     *InfoPlistStart;
  CHAR8     *InfoPlistEnd;
  UINT32    InfoPlistSize;
  INTN      DictLevel;
  CHAR8     SavedValue;
  UINT32    KextAddr;
  UINT32    KextSize;
  UINT32    range;
  
  InfoPlistStart = NULL;
  InfoPlistEnd = NULL;
  DictLevel = 0;

  WholePlist = (CHAR8 *) (UINTN) PrelinkInfoAddr;
  DictPtr = WholePlist;
  range = PrelinkInfoSize;
  
  while ((DictPtr = (CHAR8 *)SearchMemory ((UINT8 *)DictPtr, range, "dict>", 5)) != NULL) {
    
    if (DictPtr[-1] == '<') {
      // opening dict
      DictLevel++;
      if (DictLevel == 2) {
        // kext start
        InfoPlistStart = DictPtr - 1;
      }
      
    } else if (DictPtr[-2] == '<' && DictPtr[-1] == '/') {
      
      // closing dict
      if (DictLevel == 2 && InfoPlistStart != NULL) {
        // kext end
        InfoPlistEnd = DictPtr + 5 /* "dict>" */;
        
        // terminate Info.plist with 0
        SavedValue = *InfoPlistEnd;
        *InfoPlistEnd = '\0';
        InfoPlistSize = (UINT32) (InfoPlistEnd - InfoPlistStart);
        
        // get kext address from _PrelinkExecutableSourceAddr
        // truncate to 32 bit to get physical addr
        KextAddr = (UINT32) GetPlistHexValue (InfoPlistStart, InfoPlistSize, kPrelinkExecutableSourceKey, WholePlist, PrelinkInfoSize);
        // KextAddr is always relative to 0x200000
        // and if KernelSlide is != 0 then KextAddr must be adjusted
        KextAddr += KernelSlide;
        // and adjust for AptioFixDrv's KernelRelocBase
        KextAddr += (UINT32) KernelRelocBase;
        
        KextSize = (UINT32) GetPlistHexValue (InfoPlistStart, InfoPlistSize, kPrelinkExecutableSizeKey, WholePlist, PrelinkInfoSize);

        // patch it
        PatchKext (
          (UINT8 *) (UINTN) KextAddr,
          KextSize,
          InfoPlistStart,
          InfoPlistSize
        );

        // restore saved char
        *InfoPlistEnd = SavedValue;
      }
      DictLevel--;
    }
    DictPtr += 5;
    range = (UINT32) (PrelinkInfoSize - (DictPtr - WholePlist));
  }
}

#if 0
//
// Iterates over kexts loaded by booter
// and calls PatchKext() for each.
//
VOID
PatchLoadedKexts (
  VOID
)
{
  DTEntry             MMEntry;
  _BooterKextFileInfo *KextFileInfo;
  CHAR8               *PropName;
  _DeviceTreeBuffer   *PropEntry;
  CHAR8               SavedValue;
  CHAR8               *InfoPlist;

  struct OpaqueDTPropertyIterator OPropIter;
  DTPropertyIterator  PropIter = &OPropIter;

  if (!dtRoot) {
    return;
  }
  
  DTInit(dtRoot);
  
  if (DTLookupEntry(NULL,"/chosen/memory-map", &MMEntry) == kSuccess)
  {
    if (DTCreatePropertyIteratorNoAlloc(MMEntry, PropIter) == kSuccess)
    {   
      while (DTIterateProperties(PropIter, &PropName) == kSuccess)
      {  
        if (AsciiStrStr(PropName,"Driver-"))
        {
          PropEntry = (_DeviceTreeBuffer*)(((UINT8*)PropIter->currentProperty) + sizeof(DeviceTreeNodeProperty));
          KextFileInfo = (_BooterKextFileInfo *)(UINTN)PropEntry->paddr;
          // Info.plist should be terminated with 0, but will also do it just in case
          InfoPlist = (CHAR8*)(UINTN)KextFileInfo->infoDictPhysAddr;
          SavedValue = InfoPlist[KextFileInfo->infoDictLength];
          InfoPlist[KextFileInfo->infoDictLength] = '\0';
          
          PatchKext(
                    (UINT8*)(UINTN)KextFileInfo->executablePhysAddr,
                    KextFileInfo->executableLength,
                    InfoPlist,
                    KextFileInfo->infoDictLength
                    );
          
          InfoPlist[KextFileInfo->infoDictLength] = SavedValue;
        }
      }
    } 
  }
}
#endif

//
// Entry for all kext patches.
// Will iterate through kext in prelinked kernel (kernelcache)
// or DevTree (drivers boot) and do patches.
//
VOID
KextPatcherStart (
  VOID
)
{
  if (isKernelcache) {
    PatchPrelinkedKexts();
  }
#if 0
  else {
    PatchLoadedKexts();
  }
#endif
}

