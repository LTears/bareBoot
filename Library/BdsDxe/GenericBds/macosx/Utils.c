
/* $Id: Utils.c $ */

/** @file
 * Utils.c - VirtualBox Console control emulation
 */

#include <macosx.h>
#include <Library/plist.h>

#include "cpu.h"
#include "../InternalBdsLib.h"
#include "../../BootMaintLib.h"

CHAR8                           *OSVersion;
EFI_GUID                        gPlatformUuid;
EFI_GUID                        gSystemID;
SETTINGS_DATA                   gSettings;
VOID                            *gConfigPlist;


BOOLEAN
  EfiGrowBuffer (
  IN OUT EFI_STATUS *Status,
  IN OUT VOID **Buffer,
  IN UINTN BufferSize
);

CHAR8 *DefaultMemEntry = "N/A";
CHAR8 *DefaultSerial = "CT288GT9VT6";
CHAR8 *BiosVendor = "Apple Inc.";
CHAR8 *AppleManufacturer = "Apple Computer, Inc.";
EFI_UNICODE_COLLATION_PROTOCOL *gUnicodeCollation = NULL;

CHAR8 *AppleFirmwareVersion[] = {
  "MB11.88Z.0061.B03.0809221748",
  "MB21.88Z.00A5.B07.0706270922",
  "MB41.88Z.0073.B00.0809221748",
  "MB52.88Z.0088.B05.0809221748",
  "MBP51.88Z.007E.B06.0906151647",
  "MBP61.88Z.0057.B16.1702172057",  // MBP62
  "MBP81.88Z.0047.B2A.1506082203",
  "MBP83.88Z.0047.B24.1110261426",
  "MBP91.88Z.00D3.B08.1205101904",
  "MBA31.88Z.0061.B07.0712201139",
  "MBA51.88Z.00EF.B01.1205221442",
  "MM21.88Z.009A.B00.0706281359",
  "MM51.88Z.0077.B10.1102291410",
  "MM61.88Z.0106.B00.1208091121",
  "IM81.88Z.00C1.B00.0803051705",
  "IM101.88Z.00CC.B00.0909031926",
  "IM111.88Z.0034.B02.1003171314",
  "IM112.88Z.0057.B03.1509231647",
  "IM113.88Z.0057.B01.1005031455",
  "IM121.88Z.0047.B1F.1201241648",
  "IM122.88Z.0047.B1F.1223021110",
  "IM131.88Z.010A.B00.1209042338",
  "MP31.88Z.006C.B05.0802291410",
  "MP41.88Z.0081.B04.0903051113",
  "MP51.88Z.007F.B03.1010071432"
};

CHAR8 *AppleBoardID[] = {
  "Mac-F4208CC8", //MB11 - yonah
  "Mac-F4208CA9", //MB21 - merom 05/07
  "Mac-F22788A9", //MB41 - penryn
  "Mac-F22788AA", //MB52
  "Mac-F42D86C8", //MBP51
  "Mac-F22586C8",          // MBP62   Intel Core i7-620M @ 2.66 GHz 
  "Mac-94245B3640C91C81", //MBP81 - i5 SB IntelHD3000
  "Mac-942459F5819B171B", //MBP83 - i7 SB  ATI
  "Mac-6F01561E16C75D06", //MBP92 - i5-3210M IvyBridge HD4000
  "Mac-942452F5819B1C1B", //MBA31
  "Mac-2E6FAB96566FE58C", //MBA52 - i5-3427U IVY BRIDGE IntelHD4000 did=166
  "Mac-F4208EAA", //MM21 - merom GMA950 07/07
  "Mac-8ED6AF5B48C039E1", //MM51 - Sandy + Intel 30000
  "Mac-F65AE981FFA204ED", //MM62 - Ivy
  "Mac-F227BEC8", //IM81 - merom 01/09
  "Mac-F2268CC8", //IM101 - wolfdale? E7600 01/
  "Mac-F2268DAE", //IM111 - Nehalem
  "Mac-F2238AC8", //IM112 - Clarkdale
  "Mac-F2238BAE", //IM113 - lynnfield
  "Mac-942B5BF58194151B", //IM121 - i5-2500 - sandy
  "Mac-942B59F58194171B", //IM122 - i7-2600
  "Mac-00BE6ED71E35EB86", //IM131 - -i5-3470S -IVY
  "Mac-F2268DC8", //MP31 - xeon quad 02/09 conroe
  "Mac-F4238CC8", //MP41 - xeon wolfdale
  "Mac-F221BEC8"  //MP51 - Xeon Nehalem 4 cores
};

CHAR8 *AppleReleaseDate[] = {
  "09/22/08", //mb11
  "06/27/07",
  "09/22/08",
  "01/21/09",
  "06/15/09", //mbp51
  "02/17/17",    // MBP62
  "02/07/11",
  "10/26/11",
  "05/10/2012", //MBP92
  "12/20/07",
  "05/22/2012", //mba52
  "08/07/07", //mm21
  "02/29/11", //MM51
  "08/09/2012", //MM62
  "03/05/08",
  "09/03/09", //im101
  "03/17/10",
  "09/23/15", //11,2
  "05/03/10",
  "01/24/12", //121 120124
  "02/23/12", //122
  "09/04/2012", //131
  "02/29/08",
  "03/05/09",
  "10/07/10"
};

CHAR8 *AppleProductName[] = {
  "MacBook1,1",
  "MacBook2,1",
  "MacBook4,1",
  "MacBook5,2",
  "MacBookPro5,1",
  "MacBookPro6,2",
  "MacBookPro8,1",
  "MacBookPro8,3",
  "MacBookPro9,2",
  "MacBookAir3,1",
  "MacBookAir5,2",
  "Macmini2,1",
  "Macmini5,1",
  "Macmini6,2",
  "iMac8,1",
  "iMac10,1",
  "iMac11,1",
  "iMac11,2",
  "iMac11,3",
  "iMac12,1",
  "iMac12,2",
  "iMac13,1",
  "MacPro3,1",
  "MacPro4,1",
  "MacPro5,1"
};

CHAR8 *AppleFamilies[] = {
  "MacBook",
  "MacBook",
  "MacBook",
  "MacBook",
  "MacBookPro",
  "MacBookPro",
  "MacBookPro",
  "MacBookPro",
  "MacBook Pro",
  "MacBookAir",
  "MacBook Air",
  "Macmini",
  "Mac mini",
  "Macmini",
  "iMac",
  "iMac",
  "iMac",
  "iMac",
  "iMac",
  "iMac",
  "iMac",
  "iMac",
  "MacPro",
  "MacPro",
  "MacPro"
};

CHAR8 *AppleSystemVersion[] = {
  "1.1",
  "1.2",
  "1.3",
  "1.3",
  "1.0",
  "1.0",  // MBP62
  "1.0",
  "1.0",
  "1.0",
  "1.0",
  "1.0",
  "1.1",
  "1.0",  //MM51
  "1.0",
  "1.3",
  "1.0",
  "1.0",
  "1.2",
  "1.0",
  "1.9",
  "1.9",
  "1.0",
  "1.3",
  "1.4",
  "1.2"
};

CHAR8 *AppleSerialNumber[] = {
  "W80A041AU9B",  //MB11
  "W88A041AWGP",  //MB21 - merom 05/07
  "W88A041A0P0",  //MB41
  "W88AAAAA9GU",  //MB52
  "W88439FE1G0",  //MBP51
  "CK132A91AGW",  // MBP62
  "W89F9196DH2G", //MBP81 - i5 SB IntelHD3000
  "W88F9CDEDF93", //MBP83 -i7 SB  ATI
  "C02HA041DTY3", //MBP92 - i5 IvyBridge HD4000
  "W8649476DQX",  //MBA31
  "C02HA041DRVC", //MBA52 - IvyBridge
  "W88A56BYYL2",  //MM21 - merom GMA950 07/07
  "C07GA041DJD0", //MM51 - sandy
  "C07JD041DWYN", //MM62 - IVY
  "W89A00AAX88",  //IM81 - merom 01/09
  "W80AA98A5PE",  //IM101 - wolfdale? E7600 01/09
  "G8942B1V5PJ",  //IM111 - Nehalem
  "W8034342DB7",  //IM112 - Clarkdale
  "QP0312PBDNR",  //IM113 - lynnfield
  "W80CF65ADHJF", //IM121 - i5-2500 - sandy
  "W88GG136DHJQ", //IM122 -i7-2600
  "C02JA041DNCT", //IM131 -i5-3470S -IVY
  "W88A77AA5J4",  //MP31 - xeon quad 02/09
  "CT93051DK9Y",  //MP41
  "CG154TB9WU3" //MP51 C07J50F7F4MC
};

CHAR8 *AppleChassisAsset[] = {
  "MacBook-White",
  "MacBook-White",
  "MacBook-Black",
  "MacBook-Black",
  "MacBook-Aluminum",
  "MacBook-Aluminum",   // MBP62
  "MacBook-Aluminum",
  "MacBook-Aluminum",
  "MacBook-Aluminum",
  "Air-Enclosure",
  "Air-Enclosure",
  "Mini-Aluminum",
  "Mini-Aluminum",
  "Mini-Aluminum",
  "iMac-Aluminum",
  "iMac-Aluminum",
  "iMac-Aluminum",
  "iMac-Aluminum",
  "iMac-Aluminum",
  "iMac-Aluminum",
  "iMac-Aluminum",
  "iMac-Aluminum",
  "Pro-Enclosure",
  "Pro-Enclosure",
  "Pro-Enclosure"
};

CHAR8 *AppleBoardSN = "C02032101R5DC771H";
CHAR8 *AppleBoardLocation = "Part Component";

UINT16
GetAdvancedCpuType (
  VOID
)
{
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL) {
    switch (gCPUStructure.Family) {

    case 0x06:{
        switch (gCPUStructure.Model) {
        case CPU_MODEL_DOTHAN: // Dothan
        case CPU_MODEL_YONAH:  // Yonah
          return 0x201;

        case CPU_MODEL_NEHALEM_EX: //Xeon 5300
          return 0x402;

        case CPU_MODEL_NEHALEM:  // Intel Core i7 LGA1366 (45nm)
          return 0x701; // Core i7

        case CPU_MODEL_FIELDS: // Lynnfield, Clarksfield, Jasper
          if (AsciiStrStr (gCPUStructure.BrandString, "i5") != NULL) {
            return 0x601; // Core i5
          }

          return 0x701; // Core i7

        case CPU_MODEL_DALES:  // Intel Core i5, i7 LGA1156 (45nm) (Havendale, Auburndale)
          if (AsciiStrStr (gCPUStructure.BrandString, "Core(TM) i3") != NULL) {
            return 0x901; // Core i3 //why not 902? Ask Apple
          }

          if (AsciiStrStr (gCPUStructure.BrandString, "i5") != NULL ||
              (gCPUStructure.Cores <= 2)) {
            return 0x602; // Core i5
          }

          return 0x702; // Core i7

          //case CPU_MODEL_ARRANDALE:
        case CPU_MODEL_CLARKDALE:  // Intel Core i3, i5, i7 LGA1156 (32nm) (Clarkdale, Arrandale)
          if (AsciiStrStr (gCPUStructure.BrandString, "i3") != NULL) {
            return 0x901; // Core i3
          }

          if (AsciiStrStr (gCPUStructure.BrandString, "i5") != NULL ||
              (gCPUStructure.Cores <= 2)) {
            return 0x601; // Core i5 - (M540 -> 0x0602)
          }

          return 0x701; // Core i7

        case CPU_MODEL_WESTMERE: // Intel Core i7 LGA1366 (32nm) 6 Core (Gulftown, Westmere-EP, Westmere-WS)
        case CPU_MODEL_WESTMERE_EX:  // Intel Core i7 LGA1366 (45nm) 6 Core ???
          return 0x701; // Core i7

        case CPU_MODEL_SANDY_BRIDGE:
          if (AsciiStrStr (gCPUStructure.BrandString, "i3") != NULL) {
            return 0x903; // Core i3
          }

          if (AsciiStrStr (gCPUStructure.BrandString, "i5") != NULL ||
              (gCPUStructure.Cores <= 2)) {
            return 0x603; // Core i5
          }

          return 0x703;

        case CPU_MODEL_IVY_BRIDGE:
        case CPU_MODEL_IVY_BRIDGE_E5:
          if (AsciiStrStr (gCPUStructure.BrandString, "i3") != NULL) {
            return 0x903; // Core i3 - Apple doesn't use it
          }

          if (AsciiStrStr (gCPUStructure.BrandString, "i5") != NULL ||
              (gCPUStructure.Cores <= 2)) {
            return 0x604; // Core i5
          }

          return 0x704;

        case CPU_MODEL_MEROM:  // Merom
          if (gCPUStructure.Cores >= 2) {
            if (AsciiStrStr (gCPUStructure.BrandString, "Xeon") != NULL) {
              return 0x402; // Quad-Core Xeon
            }
            else {
              return 0x301; // Core 2 Duo
            }
          }
          else {
            return 0x201; // Core Solo
          };

        case CPU_MODEL_PENRYN: // Penryn
        case CPU_MODEL_ATOM: // Atom (45nm)
        default:
          if (gCPUStructure.Cores >= 4) {
            return 0x402; // Quad-Core Xeon
          }
          else if (gCPUStructure.Cores == 1) {
            return 0x201; // Core Solo
          };
          return 0x301; // Core 2 Duo
        }
      }
    }
  }
  return 0x0;
}

MACHINE_TYPES
GetDefaultModel (
  VOID
)
{
  MACHINE_TYPES DefaultType = MacPro31;

  // TODO: Add more CPU models and configure the correct machines per CPU/GFX model
  if (gSettings.Mobile) {
    switch (gCPUStructure.Model) {
    case CPU_MODEL_ATOM:
      DefaultType = MacBookAir31; //MacBookAir1,1 doesn't support _PSS for speedstep!
      break;

    case CPU_MODEL_DOTHAN:
      DefaultType = MacBook11;
      break;

    case CPU_MODEL_YONAH:
      DefaultType = MacBook11;
      break;

    case CPU_MODEL_MEROM:
      DefaultType = MacBook21;
      break;

    case CPU_MODEL_PENRYN:
      if (gGraphics.Vendor == Nvidia) {
        DefaultType = MacBookPro51;
      }
      else {
        DefaultType = MacBook41;
      }

      break;

    case CPU_MODEL_JAKETOWN:
    case CPU_MODEL_SANDY_BRIDGE:
      if (AsciiStrStr (gCPUStructure.BrandString, "i3") != NULL ||
          AsciiStrStr (gCPUStructure.BrandString, "i5") != NULL) {
        DefaultType = MacBookPro81;
        break;
      }

      DefaultType = MacBookPro83;
      break;

    case CPU_MODEL_IVY_BRIDGE:
    case CPU_MODEL_IVY_BRIDGE_E5:
      DefaultType = MacBookAir52;
      break;

    default:
      if (gGraphics.Vendor == Nvidia) {
        DefaultType = MacBookPro51;
      }
      else {
        DefaultType = MacBook52;
      }

      break;
    }
  }
  else {
    switch (gCPUStructure.Model) {
    case CPU_MODEL_CELERON:
      DefaultType = MacMini21;
      break;

    case CPU_MODEL_LINCROFT:
      DefaultType = MacMini21;
      break;

    case CPU_MODEL_ATOM:
      DefaultType = MacMini21;
      break;

    case CPU_MODEL_MEROM:
      DefaultType = iMac81;
      break;

    case CPU_MODEL_PENRYN:
      DefaultType = MacPro31; //speedstep without patching; Hapertown is also a Penryn, according to Wikipedia
      break;

    case CPU_MODEL_NEHALEM:
      DefaultType = MacPro41;
      break;

    case CPU_MODEL_NEHALEM_EX:
      DefaultType = MacPro41;
      break;

    case CPU_MODEL_FIELDS:
      DefaultType = iMac112;
      break;

    case CPU_MODEL_DALES:
      DefaultType = iMac112;
      break;

    case CPU_MODEL_CLARKDALE:
      DefaultType = iMac112;
      break;

    case CPU_MODEL_WESTMERE:
      DefaultType = MacPro51;
      break;

    case CPU_MODEL_WESTMERE_EX:
      DefaultType = MacPro51;
      break;

    case CPU_MODEL_SANDY_BRIDGE:
      if (gGraphics.Vendor == Intel) {
        DefaultType = MacMini51;
      }

      if (AsciiStrStr (gCPUStructure.BrandString, "i3") != NULL ||
          AsciiStrStr (gCPUStructure.BrandString, "i5") != NULL) {
        DefaultType = iMac112;
        break;
      }

      if (AsciiStrStr (gCPUStructure.BrandString, "i7") != NULL) {
        DefaultType = iMac121;
        break;
      }

      DefaultType = MacPro51;
      break;

    case CPU_MODEL_IVY_BRIDGE:
    case CPU_MODEL_IVY_BRIDGE_E5:
      DefaultType = iMac122;  //do not make 13,1 by default because of OS 10.8.2 doesn't know it
      break;

    case CPU_MODEL_JAKETOWN:
      DefaultType = MacPro41;
      break;

    default:
      DefaultType = MacPro31;
      break;
    }
  }

  return DefaultType;
}

//---------------------------------------------------------------------------------

VOID
Pause (
  IN CHAR16 *Message
)
{
  if (Message != NULL) {
    Print (L"%s", Message);
  }

  gBS->Stall (4000000);
}

BOOLEAN
FileExists (
  IN EFI_FILE * RootFileHandle,
  IN CHAR16 *RelativePath
)
{
  EFI_STATUS Status;
  EFI_FILE *TestFile;

  Status =
    RootFileHandle->Open (RootFileHandle, &TestFile, RelativePath,
                          EFI_FILE_MODE_READ, 0);

  if (Status == EFI_SUCCESS) {
    TestFile->Close (TestFile);
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
egLoadFile (
  IN EFI_FILE_HANDLE BaseDir,
  IN CHAR16 *FileName,
  OUT UINT8 **FileData,
  OUT UINTN *FileDataLength
)
{
  EFI_STATUS Status;
  EFI_FILE_HANDLE FileHandle;
  EFI_FILE_INFO *FileInfo;
  UINT64 ReadSize;
  UINTN BufferSize;
  UINT8 *Buffer;

  if (BaseDir == NULL) {
    return EFI_NOT_FOUND;
  }

  Status =
    BaseDir->Open (BaseDir, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  FileInfo = EfiLibFileInfo (FileHandle);

  if (FileInfo == NULL) {
    FileHandle->Close (FileHandle);
    return EFI_NOT_FOUND;
  }

  ReadSize = FileInfo->FileSize;

  FreePool (FileInfo);

  if (ReadSize == 0) {
    FileHandle->Close (FileHandle);
    *FileData = NULL;
    *FileDataLength = 0;
    return EFI_SUCCESS;
  }

  if (ReadSize > MAX_FILE_SIZE) {
    ReadSize = MAX_FILE_SIZE;
  }

  BufferSize = (UINTN) ReadSize;  // was limited to 1 GB above, so this is safe
  Buffer = (UINT8 *) AllocateAlignedPages (EFI_SIZE_TO_PAGES (BufferSize), 16);

  if (Buffer == NULL) {
    FileHandle->Close (FileHandle);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = FileHandle->Read (FileHandle, &BufferSize, Buffer);
  FileHandle->Close (FileHandle);

  if (EFI_ERROR (Status)) {
    FreeAlignedPages (Buffer, EFI_SIZE_TO_PAGES (BufferSize));
    return Status;
  }

  *FileData = Buffer;
  *FileDataLength = BufferSize;
  return EFI_SUCCESS;
}

EFI_STATUS
egSaveFile (
  IN EFI_FILE_HANDLE BaseDir,
  IN CHAR16 *FileName,
  IN UINT8 *FileData,
  IN UINTN FileDataLength
)
{
  EFI_STATUS Status;
  EFI_FILE_HANDLE FileHandle;
  UINTN BufferSize;

  if (BaseDir == NULL) {
    return EFI_NOT_FOUND;
  }

  Status =
    BaseDir->Open (BaseDir, &FileHandle, FileName,
                   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);

  if (!EFI_ERROR (Status)) {
    FileHandle->Delete (FileHandle);
  }

  Status =
    BaseDir->Open (BaseDir, &FileHandle, FileName,
                   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE |
                   EFI_FILE_MODE_CREATE, 0);
  if (EFI_ERROR (Status))
    return Status;

  BufferSize = FileDataLength;
  Status = FileHandle->Write (FileHandle, &BufferSize, FileData);
  FileHandle->Close (FileHandle);

  return Status;
}

EFI_STATUS
SaveBooterLog (
  IN EFI_FILE_HANDLE BaseDir,
  IN CHAR16 *FileName
)
{
  CHAR8 *MemLogBuffer;
  UINTN MemLogLen;

  MemLogBuffer = GetMemLogBuffer ();
  MemLogLen = GetMemLogLen ();

  if (MemLogBuffer == NULL || MemLogLen == 0) {
    return EFI_NOT_FOUND;
  }

  return egSaveFile (BaseDir, FileName, (UINT8 *) MemLogBuffer, MemLogLen);
}

EFI_STATUS
InitializeUnicodeCollationProtocol (
  VOID
)
{
  EFI_STATUS Status;

  if (gUnicodeCollation != NULL) {
    return EFI_SUCCESS;
  }

  //
  // BUGBUG: Proper impelmentation is to locate all Unicode Collation Protocol
  // instances first and then select one which support English language.
  // Current implementation just pick the first instance.
  //
  Status =
    gBS->LocateProtocol (&gEfiUnicodeCollation2ProtocolGuid, NULL,
                         (VOID **) &gUnicodeCollation);
  if (EFI_ERROR (Status)) {
    Status =
      gBS->LocateProtocol (&gEfiUnicodeCollationProtocolGuid, NULL,
                           (VOID **) &gUnicodeCollation);

  }
  return Status;
}

BOOLEAN
MetaiMatch (
  IN CHAR16 *String,
  IN CHAR16 *Pattern
)
{
  if (!gUnicodeCollation) {
    // quick fix for driver loading on UEFIs without UnicodeCollation
    //return FALSE;
    return TRUE;
  }
  return gUnicodeCollation->MetaiMatch (gUnicodeCollation, String, Pattern);
}

EFI_STATUS
DirNextEntry (
  IN EFI_FILE * Directory,
  IN OUT EFI_FILE_INFO ** DirEntry,
  IN UINTN FilterMode
)
{
  EFI_STATUS Status;
  VOID *Buffer;
  UINTN LastBufferSize, BufferSize;
  INTN IterCount;

  for (;;) {

    // free pointer from last call
    if (*DirEntry != NULL) {
      FreePool (*DirEntry);
      *DirEntry = NULL;
    }

    // read next directory entry
    LastBufferSize = BufferSize = 256;
    Buffer = AllocateZeroPool (BufferSize);
    for (IterCount = 0;; IterCount++) {
      Status = Directory->Read (Directory, &BufferSize, Buffer);
      if (Status != EFI_BUFFER_TOO_SMALL || IterCount >= 4)
        break;
      if (BufferSize <= LastBufferSize) {
        BufferSize = LastBufferSize * 2;
      }
      Buffer = EfiReallocatePool (Buffer, LastBufferSize, BufferSize);
      LastBufferSize = BufferSize;
    }
    if (EFI_ERROR (Status)) {
      FreePool (Buffer);
      break;
    }

    // check for end of listing
    if (BufferSize == 0) {  // end of directory listing
      FreePool (Buffer);
      break;
    }

    // entry is ready to be returned
    *DirEntry = (EFI_FILE_INFO *) Buffer;
    if (*DirEntry) {
      // filter results
      if (FilterMode == 1) {  // only return directories
        if (((*DirEntry)->Attribute & EFI_FILE_DIRECTORY))
          break;
      }
      else if (FilterMode == 2) { // only return files
        if (((*DirEntry)->Attribute & EFI_FILE_DIRECTORY) == 0)
          break;
      }
      else  // no filter or unknown filter -> return everything
        break;
    }
  }
  return Status;
}

VOID
DirIterOpen (
  IN EFI_FILE * BaseDir,
  IN CHAR16 *RelativePath OPTIONAL,
  OUT DIR_ITER * DirIter
)
{
  if (RelativePath == NULL) {
    DirIter->LastStatus = EFI_SUCCESS;
    DirIter->DirHandle = BaseDir;
    DirIter->CloseDirHandle = FALSE;
  }
  else {
    DirIter->LastStatus =
      BaseDir->Open (BaseDir, &(DirIter->DirHandle), RelativePath,
                     EFI_FILE_MODE_READ, 0);
    DirIter->CloseDirHandle = EFI_ERROR (DirIter->LastStatus) ? FALSE : TRUE;
  }
  DirIter->LastFileInfo = NULL;
}

BOOLEAN
DirIterNext (
  IN OUT DIR_ITER * DirIter,
  IN UINTN FilterMode,
  IN CHAR16 *FilePattern OPTIONAL,
  OUT EFI_FILE_INFO ** DirEntry
)
{
  if (DirIter->LastFileInfo != NULL) {
    FreePool (DirIter->LastFileInfo);
    DirIter->LastFileInfo = NULL;
  }

  if (EFI_ERROR (DirIter->LastStatus)) {
    return FALSE; // stop iteration
  }

  for (;;) {
    DirIter->LastStatus =
      DirNextEntry (DirIter->DirHandle, &(DirIter->LastFileInfo), FilterMode);
    if (EFI_ERROR (DirIter->LastStatus)) {
      return FALSE;
    }
    if (DirIter->LastFileInfo == NULL) {  // end of listing
      return FALSE;
    }
    if (FilePattern != NULL) {
      if ((DirIter->LastFileInfo->Attribute & EFI_FILE_DIRECTORY)) {
        break;
      }
      if (MetaiMatch (DirIter->LastFileInfo->FileName, FilePattern)) {
        break;
      }
      // else continue loop
    }
    else
      break;
  }

  *DirEntry = DirIter->LastFileInfo;
  return TRUE;
}

EFI_STATUS
DirIterClose (
  IN OUT DIR_ITER * DirIter
)
{
  if (DirIter->LastFileInfo != NULL) {
    FreePool (DirIter->LastFileInfo);
    DirIter->LastFileInfo = NULL;
  }
  if (DirIter->CloseDirHandle)
    DirIter->DirHandle->Close (DirIter->DirHandle);
  return DirIter->LastStatus;
}

//---------------------------------------------------------------------------------

VOID *
GetDictionary (
  IN VOID *topDict,
  IN CHAR8 *dictName
)
{
  return plDictFind (topDict, dictName, (unsigned int) AsciiStrLen (dictName), plKindDict);
}

BOOLEAN
IsHexDigit (
  CHAR8 c
)
{
  return (IS_DIGIT (c) || (c >= 'A' && c <= 'F') ||
          (c >= 'a' && c <= 'f')) ? TRUE : FALSE;
}

UINT32
hex2bin (
  IN CHAR8 *hex,
  OUT UINT8 *bin,
  INT32 len
)
{
  CHAR8 *p;
  UINT32 i;
  UINT32 outlen;
  CHAR8 buf[3];

  outlen = 0;

  if (hex == NULL || bin == NULL || len <= 0 ||
      (INT32) AsciiStrLen (hex) != len * 2) {
    return 0;
  }

  buf[2] = '\0';
  p = (CHAR8 *) hex;

  for (i = 0; i < (UINT32) len; i++) {
    while ((*p == ' ') || (*p == ',')) {
      p++;
    }
    if (*p == '\0') {
      break;
    }
    if (!IsHexDigit (p[0]) || !IsHexDigit (p[1])) {
      return 0;
    }

    buf[0] = *p++;
    buf[1] = *p++;
    bin[i] = (UINT8) AsciiStrHexToUintn (buf);
    outlen++;
  }
  return outlen;
}

VOID *
GetDataSetting (
  IN VOID *dict,
  IN CHAR8 *propName,
  OUT UINTN *dataLen
)
{
  VOID *prop;
  UINT8 *data;
  UINT32 len;

  data = NULL;
  if (dataLen != NULL) {
    *dataLen = 0;
  }

  prop =
    plDictFind (dict, propName, (unsigned int) AsciiStrLen (propName),
                plKindAny);

  if (prop == NULL) {
    return NULL;
  }
  switch (plNodeGetKind (prop)) {
  case plKindData:
    len = plNodeGetSize (prop);
    data = AllocateCopyPool (len, plNodeGetBytes (prop));
    break;
  case plKindString:
    // assume data in hex encoded string property
    len = plNodeGetSize (prop) >> 1;  // 2 chars per byte
    data = AllocatePool (len);
    len = hex2bin (plNodeGetBytes (prop), data, len);
    break;
  default:
    len = 0;
    break;
  }

  if (dataLen != NULL) {
    *dataLen = len;
  }

  return data;
}

UINTN
GetNumProperty (
  VOID *dict,
  CHAR8 *key,
  UINTN def
)
{
  VOID *dentry;

  dentry =
    plDictFind (dict, key, (unsigned int) AsciiStrLen (key), plKindInteger);
  if (dentry != NULL) {
    def = (UINTN) plIntegerGet (dentry);
  }
  return def;
}

BOOLEAN
GetBoolProperty (
  VOID *dict,
  CHAR8 *key,
  BOOLEAN def
)
{
  VOID *dentry;

  dentry = plDictFind (dict, key, (unsigned int) AsciiStrLen (key), plKindBool);
  if (dentry != NULL) {
    return plBoolGet (dentry) ? TRUE : FALSE;
  }
  return def;
}

VOID
GetAsciiProperty (
  VOID *dict,
  CHAR8 *key,
  CHAR8 *aptr
)
{
  VOID *dentry;
  UINTN slen;

  dentry =
    plDictFind (dict, key, (unsigned int) AsciiStrLen (key), plKindString);
  if (dentry != NULL) {
    slen = plNodeGetSize (dentry);
    CopyMem (aptr, plNodeGetBytes (dentry), slen);
    aptr[slen] = '\0';
  }
}

BOOLEAN
GetUnicodeProperty (
  VOID *dict,
  CHAR8 *key,
  CHAR16 *uptr
)
{
  VOID *dentry;
  CHAR8 *tbuf;
  UINTN tsiz;

  dentry =
    plDictFind (dict, key, (unsigned int) AsciiStrLen (key), plKindString);
  if (dentry != NULL) {
    tsiz = plNodeGetSize (dentry);
    tbuf = AllocateCopyPool (tsiz + 1, plNodeGetBytes (dentry));
    if (tbuf != NULL) {
      tbuf[tsiz] = '\0';
      AsciiStrToUnicodeStr (tbuf, uptr);
      FreePool (tbuf);
      return TRUE;
    }
  }
  return FALSE;
}

CHAR8 *
GetStringProperty (
  VOID *dict,
  CHAR8 *key
)
{
  VOID *dentry;
  CHAR8 *tbuf;
  UINTN tsiz;

  tbuf = NULL;
  dentry =
    plDictFind (dict, key, (unsigned int) AsciiStrLen (key), plKindString);
  if (dentry != NULL) {
    tsiz = plNodeGetSize (dentry);
    tbuf = AllocateCopyPool (tsiz + 1, plNodeGetBytes (dentry));
    if (tbuf != NULL) {
      tbuf[tsiz] = '\0';
    }
  }
  return tbuf;
}

VOID
plist2dbg (
VOID* plist
)
{
#define XMLBUFSIZE 4096
  plbuf_t xbuf;
  char* lst;
  char* lbyte;

  xbuf.dat = AllocatePool(XMLBUFSIZE + 1);
  if (xbuf.dat == NULL) {
    return;
  }
  xbuf.dat[XMLBUFSIZE] = '\0';
  xbuf.len = XMLBUFSIZE;
  xbuf.pos = 0;

  plNodeToXml (plist, &xbuf);

  lbyte = &xbuf.dat[xbuf.pos];
  for (lst = xbuf.dat; lst < lbyte;) {
    char* eol;

    eol = ScanMem8(lst, lbyte - lst, '\n');
    if (eol == NULL) {
      break;
    } else {
      *eol = '\0';
      DBG ("%a\n", lst);
      lst = eol  + 1;
    }
  }
  FreePool (xbuf.dat);
#undef XMLBUFSIZE
}

VOID *
LoadPListFile (
  IN EFI_FILE * RootFileHandle,
  IN CHAR16 *XmlPlistPath
)
{
  EFI_STATUS Status;
  plbuf_t pbuf;
  VOID *plist;

  Status = EFI_NOT_FOUND;
  pbuf.pos = 0;

  Status =
    egLoadFile (RootFileHandle, XmlPlistPath, (UINT8 **) &pbuf.dat,
                (UINTN *) &pbuf.len);

  if (EFI_ERROR (Status) || pbuf.dat == NULL) {
    return NULL;
  }

  plist = plXmlToNode (&pbuf);
  FreeAlignedPages (pbuf.dat, EFI_SIZE_TO_PAGES (pbuf.len));
  if (plist == NULL) {
    Print (L"Error loading plist from %s\n", XmlPlistPath);
    return NULL;
  }

  return plist;
}

// ----============================----
EFI_STATUS
GetBootDefault (
  IN EFI_FILE * RootFileHandle
)
{
  VOID *spdict;

  ZeroMem (gSettings.DefaultBoot, sizeof (gSettings.DefaultBoot));

  gConfigPlist = LoadPListFile (RootFileHandle,
                                gPNDirExists ? gPNConfigPlist : L"\\EFI\\bareboot\\config.plist");

  if (gConfigPlist == NULL) {
    Print (L"Error loading bootdefault plist!\r\n");
    return EFI_NOT_FOUND;
  }

  DBG ("%a: config.plist begin\n", __FUNCTION__);
  plist2dbg (gConfigPlist);
  DBG ("%a: config.plist end\n", __FUNCTION__);

  spdict = GetDictionary (gConfigPlist, "SystemParameters");

  gSettings.SaveVideoRom = GetBoolProperty (spdict, "SaveVideoRom", FALSE);
  gSettings.ScreenMode = (UINT32) GetNumProperty (spdict, "ScreenMode", 0xffff);
  gSettings.BootTimeout = (UINT16) GetNumProperty (spdict, "Timeout", 0);
  gSettings.YoBlack = GetBoolProperty (spdict, "YoBlack", FALSE);
  gSettings.DebugKernel = GetBoolProperty (spdict, "DebugKernel", FALSE);
  gSettings.DebugKernelToCom = GetBoolProperty (spdict, "DebugKernelToCom", FALSE);

  if (!GetUnicodeProperty (spdict, "DefaultBootVolume", gSettings.DefaultBoot)) {
    gSettings.BootTimeout = 0xFFFF;
  }
  DBG
    ("GetBootDefault: DefaultBootVolume = %s, Timeout = %d, ScreenMode = %d\n",
     gSettings.DefaultBoot, gSettings.BootTimeout, gSettings.ScreenMode);

  return EFI_SUCCESS;
}

EFI_STATUS
GetUserSettings (
  VOID
)
{
  EFI_STATUS Status;
  VOID *dictPointer;
  VOID *array;
  VOID *prop;
  VOID *tmpval;
  MACHINE_TYPES Model;
  UINTN len;
  UINT32 i;
  CHAR8 cUUID[64];

  Status = EFI_NOT_FOUND;
  array = NULL;
  i = 0;

  if (gConfigPlist == NULL) {
    Print (L"Error loading usersettings plist!\r\n");
    return EFI_NOT_FOUND;
  }

  ZeroMem (gSettings.Language, sizeof (gSettings.Language));
  ZeroMem (gSettings.BootArgs, sizeof (gSettings.BootArgs));
  ZeroMem (gSettings.SerialNr, sizeof (gSettings.SerialNr));

  gSettings.CustomEDID = NULL;
  gSettings.ProcessorInterconnectSpeed = 0;

  dictPointer = GetDictionary (gConfigPlist, "SystemParameters");

  if (dictPointer != NULL) {
    GetAsciiProperty (dictPointer, "prev-lang", gSettings.Language);
    gSettings.LoadExtraKexts = GetBoolProperty (dictPointer, "LoadExtraKexts", TRUE);
    gSettings.CheckFakeSMC = GetBoolProperty (dictPointer, "CheckFakeSMC", TRUE);
    gSettings.NvRam = GetBoolProperty (dictPointer, "NvRam", FALSE);
    gSettings.Hibernate = GetBoolProperty (dictPointer, "Hibernate", FALSE);
    gSettings.CsrActiveConfig = (UINT32) GetNumProperty (dictPointer, "CsrActiveConfig", 0);
    DBG ("GetUserSettings: CsrActiveConfig = %d\n", gSettings.CsrActiveConfig);

    GetAsciiProperty (dictPointer, "boot-args", gSettings.BootArgs);
    if (AsciiStrLen (AddBootArgs) != 0) {
      AsciiStrCat (gSettings.BootArgs, AddBootArgs);
    }

    /*
     * XXX: previous implementation was not RFC4112 conforming.
     * Uuid string was treated as linear byte dump, so do the same
     */
    cUUID[0] = '\0';
    GetAsciiProperty (dictPointer, "PlatformUUID", cUUID);
    (void) AsciiStrXuidToBinary (cUUID, &gPlatformUuid);

    cUUID[0] = '\0';
    GetAsciiProperty (dictPointer, "SystemID", cUUID);
    (void) AsciiStrXuidToBinary (cUUID, &gSystemID);

    /* MLB/ROM */
    gSettings.MLB[0] = '\0';
    GetAsciiProperty (dictPointer, "MLB", gSettings.MLB);
    gSettings.ROM = GetDataSetting (dictPointer, "ROM", &gSettings.ROMLen);
  }

  dictPointer = GetDictionary (gConfigPlist, "Graphics");

  if (dictPointer != NULL) {
    gSettings.GraphicsInjector =
      GetBoolProperty (dictPointer, "GraphicsInjector", FALSE);
    gSettings.VRAM = LShiftU64 (GetNumProperty (dictPointer, "VRAM", 0), 20);
    gSettings.LoadVBios = GetBoolProperty (dictPointer, "LoadVBios", FALSE);
    gSettings.VideoPorts = (UINT16) GetNumProperty (dictPointer, "VideoPorts", 0);
    gSettings.DualLink = (UINT16) GetNumProperty (dictPointer, "DualLink", 0);
    GetUnicodeProperty (dictPointer, "FBName", gSettings.FBName);

    tmpval = GetDataSetting (dictPointer, "NVCAP", &len);
    if (tmpval != NULL) {
      if (len == sizeof (gSettings.NVCAP)) {
        CopyMem(gSettings.NVCAP, tmpval, sizeof (gSettings.NVCAP));
      }
      FreePool (tmpval);
    }

    tmpval = GetDataSetting (dictPointer, "DisplayCfg", &len);
    if (tmpval != NULL) {
      if (len == sizeof (gSettings.Dcfg)) {
        CopyMem(gSettings.Dcfg, tmpval, sizeof (gSettings.Dcfg));
      }
      FreePool (tmpval);
    }

    /* Real custom edid size ignored, assume 128 bytes always */
    gSettings.CustomEDID = GetDataSetting (dictPointer, "CustomEDID", &len);
  }

  dictPointer = GetDictionary (gConfigPlist, "PCI");

  if (dictPointer != NULL) {
    gSettings.PCIRootUID = (UINT16) GetNumProperty (dictPointer, "PCIRootUID", 0xFFFF);
    gSettings.ETHInjection = GetBoolProperty (dictPointer, "ETHInjection", FALSE);
    gSettings.USBInjection = GetBoolProperty (dictPointer, "USBInjection", FALSE);
    gSettings.HDALayoutId =
      (UINT16) GetNumProperty (dictPointer, "HDAInjection", 0);

    prop = plDictFind (dictPointer, "DeviceProperties", 16, plKindString);

    if (prop != NULL) {
      len = plNodeGetSize (prop);
      cDevProp = AllocatePool (len + 1);
      CopyMem (cDevProp, plNodeGetBytes (prop), len);
      cDevProp[len] = '\0';
      DBG ("GetUserSettings: cDevProp = <%a>\n", cDevProp);
    }
  }

  dictPointer = GetDictionary (gConfigPlist, "ACPI");

  gSettings.DropSSDT = GetBoolProperty (dictPointer, "DropOemSSDT", FALSE);
  gSettings.DropDMAR = GetBoolProperty (dictPointer, "DropDMAR", FALSE);
  gSettings.FixRegions = GetBoolProperty (dictPointer, "FixRegions", FALSE);
  // known pair for ResetAddr/ResetVal is 0x0[C/2]F9/0x06, 0x64/0xFE
  gSettings.ResetAddr =
    (UINT64) GetNumProperty (dictPointer, "ResetAddress", 0);
  gSettings.ResetVal = (UINT8) GetNumProperty (dictPointer, "ResetValue", 0);
  gSettings.PMProfile = (UINT8) GetNumProperty (dictPointer, "PMProfile", 0);
  gSettings.SavePatchedDsdt =
    GetBoolProperty (dictPointer, "SavePatchedDsdt", FALSE);

  gSettings.PatchDsdtNum = 0;
  array = plDictFind (dictPointer, "Patches", 7, plKindArray);
  if (array != NULL) {
    gSettings.PatchDsdtNum = (UINT32) plNodeGetSize (array);
    gSettings.PatchDsdtFind =
      AllocateZeroPool (gSettings.PatchDsdtNum * sizeof (UINT8 *));
    gSettings.PatchDsdtReplace =
      AllocateZeroPool (gSettings.PatchDsdtNum * sizeof (UINT8 *));
    gSettings.LenToFind =
      AllocateZeroPool (gSettings.PatchDsdtNum * sizeof (UINT32));
    gSettings.LenToReplace =
      AllocateZeroPool (gSettings.PatchDsdtNum * sizeof (UINT32));
    DBG ("gSettings.PatchDsdtNum = %d\n", gSettings.PatchDsdtNum);
    for (i = 0; i < gSettings.PatchDsdtNum; i++) {
      prop = plNodeGetItem (array, i);
      gSettings.PatchDsdtFind[i] = GetDataSetting (prop, "Find", &len);
      gSettings.LenToFind[i] = (UINT32) len;
      gSettings.PatchDsdtReplace[i] = GetDataSetting (prop, "Replace", &len);
      gSettings.LenToReplace[i] = (UINT32) len;

      DBG ("  %d. FindLen = %d; ReplaceLen = %d\n", (i + 1),
           gSettings.LenToFind[i], gSettings.LenToReplace[i]
        );
    }
  }

  gSettings.ACPIDropTables = NULL;
  array = plDictFind (dictPointer, "DropTables", 10, plKindArray);
  if (array != NULL) {
    UINT16 NrTableIds;
    ACPI_DROP_TABLE *DropTable;

    NrTableIds = (UINT16) plNodeGetSize (array);
    DBG ("Dropping %d tables\n", NrTableIds);
    if (NrTableIds > 0) {
      for (i = 0; i < NrTableIds; ++i) {
        UINT32 Signature = 0;
        UINT64 TableId = 0;
        CHAR8 *SigStr;
        CHAR8 *TablStr;
        CHAR8 s1 = 0, s2 = 0, s3 = 0, s4 = 0;
        UINTN idi = 0;
        CHAR8 id[8];

        DBG (" Drop table %d", (i + 1));
        prop = plNodeGetItem (array, i);
        // Get the table signatures to drop
        SigStr = GetStringProperty (prop, "Signature");
        if (AsciiStrLen (SigStr) != 4) {
          DBG (", bad signature\n");
          continue;
        }
        DBG (" signature = '");
        if (*SigStr) {
          s1 = *SigStr++;
          DBG ("%c", s1);
        }
        if (*SigStr) {
          s2 = *SigStr++;
          DBG ("%c", s2);
        }
        if (*SigStr) {
          s3 = *SigStr++;
          DBG ("%c", s3);
        }
        if (*SigStr) {
          s4 = *SigStr++;
          DBG ("%c", s4);
        }
        Signature = SIGNATURE_32 (s1, s2, s3, s4);
        DBG ("' (%8.8X)", Signature);
        // Get the table ids to drop
        TablStr = GetStringProperty (prop, "TableId");
        ZeroMem (id, sizeof (id));
        if (TablStr) {
          DBG (" table-id = '");
          while (*TablStr && (idi < 8)) {
            DBG ("%c", *TablStr);
            id[idi++] = *TablStr++;
          }
        }
        CopyMem (&TableId, id, 8);
        DBG ("' (%16.16lX)\n", TableId);

        DropTable = AllocateZeroPool (sizeof (ACPI_DROP_TABLE));

        DropTable->Signature = Signature;
        DropTable->TableId = TableId;

        DropTable->Next = gSettings.ACPIDropTables;
        gSettings.ACPIDropTables = DropTable;
      }
      gSettings.DropSSDT = FALSE;
    }
  }

  AsciiStrCpy (gSettings.BoardManufactureName, BiosVendor);
  AsciiStrCpy (gSettings.ChassisManufacturer, BiosVendor);
  AsciiStrCpy (gSettings.ManufactureName, BiosVendor);
  AsciiStrCpy (gSettings.VendorName, BiosVendor);

  AsciiStrCpy (gSettings.BoardSerialNumber, AppleBoardSN);
  AsciiStrCpy (gSettings.LocationInChassis, AppleBoardLocation);

  dictPointer = GetDictionary (gConfigPlist, "SMBIOS");

  gSettings.Mobile = GetBoolProperty (dictPointer, "Mobile", FALSE);

  GetAsciiProperty (dictPointer, "ProductName", gSettings.ProductName);

  Model = Unknown;
  if (AsciiStrLen (gSettings.ProductName) > 0) {
    for (i = 0; i < ARRAY_SIZE (AppleProductName); i++) {
      if (AsciiStrStr (AppleProductName[i], gSettings.ProductName) != NULL) {
        Model = i;
        break;
      }
    }
  } else {
    Model = GetDefaultModel ();
  }

  if (Model != Unknown) {
    AsciiStrCpy (gSettings.BoardNumber, AppleBoardID[Model]);
    AsciiStrCpy (gSettings.BoardVersion, AppleSystemVersion[Model]);
    AsciiStrCpy (gSettings.ChassisAssetTag, AppleChassisAsset[Model]);
    AsciiStrCpy (gSettings.FamilyName, AppleFamilies[Model]);
    AsciiStrCpy (gSettings.ProductName, AppleProductName[Model]);
    AsciiStrCpy (gSettings.ReleaseDate, AppleReleaseDate[Model]);
    AsciiStrCpy (gSettings.RomVersion, AppleFirmwareVersion[Model]);
    AsciiStrCpy (gSettings.SerialNr, AppleSerialNumber[Model]);
    AsciiStrCpy (gSettings.VersionNr, AppleSystemVersion[Model]);
  }

  GetAsciiProperty (dictPointer, "BiosReleaseDate", gSettings.ReleaseDate);
  GetAsciiProperty (dictPointer, "BiosVendor", gSettings.VendorName);
  GetAsciiProperty (dictPointer, "BiosVersion", gSettings.RomVersion);
  GetAsciiProperty (dictPointer, "Board-ID", gSettings.BoardNumber);
  GetAsciiProperty (dictPointer, "BoardManufacturer", gSettings.BoardManufactureName);
  GetAsciiProperty (dictPointer, "BoardSerialNumber", gSettings.BoardSerialNumber);
  GetAsciiProperty (dictPointer, "BoardVersion", gSettings.BoardVersion);
  GetAsciiProperty (dictPointer, "ChassisAssetTag", gSettings.ChassisAssetTag);
  GetAsciiProperty (dictPointer, "ChassisManufacturer", gSettings.ChassisManufacturer);
  GetAsciiProperty (dictPointer, "Family", gSettings.FamilyName);
  GetAsciiProperty (dictPointer, "LocationInChassis", gSettings.LocationInChassis);
  GetAsciiProperty (dictPointer, "Manufacturer", gSettings.ManufactureName);
  GetAsciiProperty (dictPointer, "SerialNumber", gSettings.SerialNr);
  GetAsciiProperty (dictPointer, "Version", gSettings.VersionNr);

  DBG ("BEGIN Product smbios datum\n");
  DBG ("ProductName = %a\n", gSettings.ProductName);
  DBG ("Mobile = %a\n", gSettings.Mobile ? "true" : "false");

  DBG ("%a = %a\n", "BiosReleaseDate", gSettings.ReleaseDate);
  DBG ("%a = %a\n", "BiosVendor", gSettings.VendorName);
  DBG ("%a = %a\n", "BiosVersion", gSettings.RomVersion);
  DBG ("%a = %a\n", "Board-ID", gSettings.BoardNumber);
  DBG ("%a = %a\n", "BoardManufacturer", gSettings.BoardManufactureName);
  DBG ("%a = %a\n", "BoardSerialNumber", gSettings.BoardSerialNumber);
  DBG ("%a = %a\n", "BoardVersion", gSettings.BoardVersion);
  DBG ("%a = %a\n", "ChassisAssetTag", gSettings.ChassisAssetTag);
  DBG ("%a = %a\n", "ChassisManufacturer", gSettings.ChassisManufacturer);
  DBG ("%a = %a\n", "Family", gSettings.FamilyName);
  DBG ("%a = %a\n", "LocationInChassis", gSettings.LocationInChassis);
  DBG ("%a = %a\n", "Manufacturer", gSettings.ManufactureName);
  DBG ("%a = %a\n", "SerialNumber", gSettings.SerialNr);
  DBG ("%a = %a\n", "Version", gSettings.VersionNr);

  gSettings.SPDScan = GetBoolProperty (dictPointer, "SPDScan", FALSE);

  array = plDictFind (dictPointer, "MemoryDevices", 13, plKindArray);
  if (array != NULL) {
    UINT8 Slot;
    UINT8 NrSlot;

    NrSlot = (UINT8) plNodeGetSize (array);
    DBG ("Custom Memory Devices slots = %d\n", NrSlot);
    if (NrSlot <= MAX_RAM_SLOTS) {
      for (i = 0; i < NrSlot; i++) {
        prop = plNodeGetItem (array, i);
        Slot = (UINT8) GetNumProperty (prop, "Slot", 0xff);
        gSettings.cMemDevice[Slot].InUse = TRUE;
        gSettings.cMemDevice[Slot].MemoryType =
          (UINT8) GetNumProperty (prop, "MemoryType", 0x02);
        gSettings.cMemDevice[Slot].Speed = (UINT16) GetNumProperty (prop, "Speed", 0x00); //MHz
        gSettings.cMemDevice[Slot].Size = (UINT16) GetNumProperty (prop, "Size", 0xffff); //MB

        gSettings.cMemDevice[Slot].DeviceLocator =
          GetStringProperty (prop, "DeviceLocator");
        gSettings.cMemDevice[Slot].BankLocator =
          GetStringProperty (prop, "BankLocator");
        gSettings.cMemDevice[Slot].Manufacturer =
          GetStringProperty (prop, "Manufacturer");
        gSettings.cMemDevice[Slot].SerialNumber =
          GetStringProperty (prop, "SerialNumber");
        gSettings.cMemDevice[Slot].PartNumber =
          GetStringProperty (prop, "PartNumber");

        DBG (" gSettings.cMemDevice[%d].MemoryType = 0x%x\n", Slot,
             gSettings.cMemDevice[Slot].MemoryType);
        DBG (" gSettings.cMemDevice[%d].Speed = %d MHz\n", Slot,
             gSettings.cMemDevice[Slot].Speed);
        DBG (" gSettings.cMemDevice[%d].Size = %d MB\n", Slot,
             gSettings.cMemDevice[Slot].Size);
        DBG (" gSettings.cMemDevice[%d].DeviceLocator = %a\n", Slot,
             gSettings.cMemDevice[Slot].DeviceLocator);
        DBG (" gSettings.cMemDevice[%d].BankLocator = %a\n", Slot,
             gSettings.cMemDevice[Slot].BankLocator);
        DBG (" gSettings.cMemDevice[%d].Manufacturer = %a\n", Slot,
             gSettings.cMemDevice[Slot].Manufacturer);
        DBG (" gSettings.cMemDevice[%d].SerialNumber = %a\n", Slot,
             gSettings.cMemDevice[Slot].SerialNumber);
        DBG (" gSettings.cMemDevice[%d].PartNumber = %a\n", Slot,
             gSettings.cMemDevice[Slot].PartNumber);
      }
    }
  }
  DBG ("END Product smbios datum\n");

  DBG ("PlatformUUID is %g (rfc4112)\n", &gPlatformUuid);
  DBG ("SystemID is %g (rfc4112)\n", &gSystemID);

  dictPointer = GetDictionary (gConfigPlist, "CPU");

  gSettings.PatchLAPIC = GetBoolProperty (dictPointer, "PatchLAPIC", FALSE);
  gSettings.PatchPM = GetBoolProperty (dictPointer, "PatchPM", FALSE);
  gSettings.PatchCPU = GetBoolProperty (dictPointer, "PatchCPU", FALSE);
  gSettings.CpuIdSing = (UINT32) GetNumProperty (dictPointer, "CpuIdSing", 0);

  if (GetBoolProperty (dictPointer, "Turbo", FALSE)) {
    if (gCPUStructure.TurboMsr != 0) {
      AsmWriteMsr64 (MSR_IA32_PERF_CONTROL, gCPUStructure.TurboMsr);
      gBS->Stall (100);
      i = 100000;
      while (AsmReadMsr64 (MSR_IA32_PERF_STATUS) & (1 << 21)) {
        if (!i--) {
          break;
        }
      }
    }
    AsmReadMsr64 (MSR_IA32_PERF_STATUS);
  }

  gSettings.CPUFrequency =
    (UINT64) GetNumProperty (dictPointer, "CPUFrequency", 0);
  gSettings.FSBFrequency =
    (UINT64) GetNumProperty (dictPointer, "FSBFrequency", 0);
  gSettings.ProcessorInterconnectSpeed =
    (UINT32) GetNumProperty (dictPointer, "QPI", 0);
  gSettings.CpuType =
    (UINT16) GetNumProperty (dictPointer, "ProcessorType",
                             GetAdvancedCpuType ());

  if (gSettings.FSBFrequency != 0) {
    gCPUStructure.FSBFrequency = gSettings.FSBFrequency;
    if (gSettings.CPUFrequency == 0) {
      gCPUStructure.CPUFrequency =
        MultU64x32 (gCPUStructure.FSBFrequency, gCPUStructure.MaxRatio);
    }
    DBG ("GetUserSettings: gCPUStructure.FSBFrequency = %d\n",
         gCPUStructure.FSBFrequency);
  }

  if (gSettings.CPUFrequency != 0) {
    gCPUStructure.CPUFrequency = gSettings.CPUFrequency;
    if (gSettings.FSBFrequency == 0) {
      gCPUStructure.FSBFrequency =
        DivU64x32 (gCPUStructure.CPUFrequency, gCPUStructure.MaxRatio);
    }
    DBG ("GetUserSettings: gCPUStructure.CPUFrequency = %d\n",
         gCPUStructure.CPUFrequency);
  }

  // KernelAndKextPatches
  gSettings.KPKernelPatchesNeeded = FALSE;
  gSettings.KPKextPatchesNeeded = FALSE;

  array = plDictFind (gConfigPlist, "KernelPatches", 13, plKindArray);
  if (array != NULL) {
    gSettings.NrKernel = (UINT32) plNodeGetSize (array);
    DBG ("gSettings.NrKernel = %d\n", gSettings.NrKernel);
    if ((gSettings.NrKernel <= 100)) {
      for (i = 0; i < gSettings.NrKernel; i++) {
        gSettings.AnyKernelData[i] = 0;
        len = 0;

        dictPointer = plNodeGetItem (array, i);
        gSettings.AnyKernelData[i] =
          GetDataSetting (dictPointer, "Find", &gSettings.AnyKernelDataLen[i]);
        gSettings.AnyKernelPatch[i] =
          GetDataSetting (dictPointer, "Replace", &len);

        if (gSettings.AnyKernelDataLen[i] != len || len == 0) {
          gSettings.AnyKernelDataLen[i] = 0;
          continue;
        }
        gSettings.KPKernelPatchesNeeded = TRUE;
        DBG ("  %d. kernel patch, length = %d, %a\n", (i + 1),
             gSettings.AnyKernelDataLen[i]
          );
      }
    }
  }

  array = plDictFind (gConfigPlist, "KextPatches", 11, plKindArray);
  if (array != NULL) {
    gSettings.NrKexts = (UINT32) plNodeGetSize (array);
    DBG ("gSettings.NrKexts = %d\n", gSettings.NrKexts);
    if ((gSettings.NrKexts <= 100)) {
      for (i = 0; i < gSettings.NrKexts; i++) {
        gSettings.AnyKextDataLen[i] = 0;
        len = 0;
        dictPointer = plNodeGetItem (array, i);
        gSettings.AnyKext[i] = GetStringProperty (dictPointer, "Name");
        // check if this is Info.plist patch or kext binary patch
        gSettings.AnyKextInfoPlistPatch[i] =
          GetBoolProperty (dictPointer, "InfoPlistPatch", FALSE);
        if (gSettings.AnyKextInfoPlistPatch[i]) {
          // Info.plist
          // Find and Replace should be in <string>...</string>
          gSettings.AnyKextData[i] = GetStringProperty (dictPointer, "Find");
          if (gSettings.AnyKextData[i] != NULL) {
            gSettings.AnyKextDataLen[i] =
              AsciiStrLen (gSettings.AnyKextData[i]);
          }
          gSettings.AnyKextPatch[i] =
            GetStringProperty (dictPointer, "Replace");
          if (gSettings.AnyKextPatch[i] != NULL) {
            len = AsciiStrLen (gSettings.AnyKextPatch[i]);
          }
        }
        else {
          // kext binary patch
          // Find and Replace should be in <data>...</data> or <string>...</string>
          gSettings.AnyKextData[i] =
            GetDataSetting (dictPointer, "Find", &gSettings.AnyKextDataLen[i]);
          gSettings.AnyKextPatch[i] =
            GetDataSetting (dictPointer, "Replace", &len);
        }
        if (gSettings.AnyKextDataLen[i] != len || len == 0) {
          gSettings.AnyKextDataLen[i] = 0;
          continue;
        }
        gSettings.KPKextPatchesNeeded = TRUE;
        DBG ("  %d. name = %a, length = %d, %a\n", (i + 1),
             gSettings.AnyKext[i], gSettings.AnyKextDataLen[i],
             gSettings.AnyKextInfoPlistPatch[i] ? "KextInfoPlistPatch " : "");
      }
    }
  }

  plNodeDelete (gConfigPlist);

  return Status;
}

STATIC CHAR16 *OSVersionFiles[] = {
  L"System\\Library\\CoreServices\\SystemVersion.plist",
  L"System\\Library\\CoreServices\\ServerVersion.plist",
  L"\\com.apple.recovery.boot\\SystemVersion.plist"
};

EFI_STATUS
GetOSVersion (
  IN EFI_FILE * FileHandle
)
{
  UINTN i;
  VOID *plist;

  /* Mac OS X */

  for (i = 0; i < 3; i++) {
    plist = LoadPListFile (FileHandle, OSVersionFiles[i]);
    if (plist != NULL) {
      OSVersion = GetStringProperty (plist, "ProductVersion");
      plNodeDelete (plist);
      DBG ("GetOSVersion: OSVersion = %a\n", OSVersion);
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}
