#!/bin/sh

## FUNCTIONS ##

fnGCC46 ()
# Function: Xcode chainload
{
[ ! -f /usr/bin/xcodebuild ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && exit
echo "CHAINLOAD: GCC46"
export TARGET_TOOLS=GCC46
}

fnGCC47 ()
# Function: Xcode chainload
{
[ ! -f /usr/bin/xcodebuild ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && exit
echo "CHAINLOAD: GCC47"
export TARGET_TOOLS=GCC47
}

fnArchIA32 ()
# Function: IA32 Arch function
{
echo "ARCH: IA32"
export PROCESSOR=IA32
export Processor=Ia32
}

fnArchX64 ()
# Function: X64 Arch function
{
echo "ARCH: X64"
export PROCESSOR=X64
export Processor=X64
}

fnDebug ()
# Function: Debug version of compiled source
{
echo "TARGET: DEBUG"
export TARGET=DEBUG
export VTARGET=DEBUG
}

fnRelease ()
# Function: Release version of compiled source
{
echo "TARGET: RELEASE"
export TARGET=RELEASE
export VTARGET=RELEASE
}


fnHelpArgument ()
# Function: Help with arguments
{
echo "ERROR!"
echo "Example: ./cbuild.sh -xcode -ia32 -release"
echo "Example: ./cbuild.sh -gcc47 -x64 -release"
}

## MAIN ARGUMENT PART##

    case "$1" in
        '')
        fnGCC47
        ;;
        '-gcc46')
         fnGCC46
        ;;
        '-gcc47')
         fnGCC47
        ;;
        *)
         echo $"ERROR!"
         echo $"TARGET_TOOLS: {-gcc46|-gcc47}"
        exit 1
    esac
    case "$2" in
        '')
        fnArchX64
        ;;
        '-ia32')
         fnArchIA32
        ;;
        '-x64')
         fnArchX64
        ;;
        *)
         echo $"ERROR!"
         echo $"ARCH: {-ia32|-x64}"
        exit 1
    esac
    case "$3" in
        '')
         fnRelease
        ;;
        '-debug')
         fnDebug
        ;;
        '-release')
         fnRelease
        ;;
        *)
         echo $"ERROR!"
         echo $"TYPE: {-debug|-release}"
        exit 1
    esac
    case "$4" in
        '-clean')
        export ARG=clean
        ;;
        '-cleanall')
        export ARG=cleanall
        ;;
    esac

fnMainBuildScript ()
# Function MAIN DUET BUILD SCRIPT
{
set -e
shopt -s nocasematch
if [ -z "$WORKSPACE" ]
then
echo Initializing workspace
if [ ! -e `pwd`/edksetup.sh ]
then
cd ..
fi
export EDK_TOOLS_PATH=`pwd`/BaseTools
echo $EDK_TOOLS_PATH
source edksetup.sh BaseTools
else
echo Building from: $WORKSPACE
fi


BUILD_ROOT_ARCH=$WORKSPACE/Build/bareBoot$PROCESSOR/"$VTARGET"_"$TARGET_TOOLS"/$PROCESSOR

if  [[ ! -f `which build` || ! -f `which GenFv` ]];
then
echo Building tools as they are not in the path
make -C $WORKSPACE/BaseTools
elif [[ ( -f `which build` ||  -f `which GenFv` )  && ! -d  $EDK_TOOLS_PATH/Source/C/bin ]];
then
echo Building tools no $EDK_TOOLS_PATH/Source/C/bin directory
make -C $WORKSPACE/BaseTools
else
echo using prebuilt tools
fi

if [[ $ARG == cleanall ]]; then
make -C $WORKSPACE/BaseTools clean
build -p $WORKSPACE/bareBoot/bareBoot$Processor.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 clean
exit $?
fi

if [[ $ARG == clean ]]; then
build -p $WORKSPACE/bareBoot/bareBoot$Processor.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 clean
exit $?
fi

# Build the edk2 bareBoot
echo Running edk2 build for bareBoot$Processor
VERFILE=$WORKSPACE/bareBoot/Version.h
echo "#define FIRMWARE_VERSION L\"2.31\"" > $VERFILE
echo "#define FIRMWARE_BUILDDATE L\"`LC_ALL=C date \"+%Y-%m-%d %H:%M:%S\"`\"" >> $VERFILE
echo "#define FIRMWARE_REVISION L\"`cd $WORKSPACE/bareBoot; git tag | tail -n 1`\"" >> $VERFILE

build -p $WORKSPACE/bareBoot/bareBoot$Processor.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 $*

}


fnMainPostBuildScript ()
{
if [ -z "$EDK_TOOLS_PATH" ]
then
export BASETOOLS_DIR=$WORKSPACE/BaseTools/Source/C/bin
else
export BASETOOLS_DIR=$EDK_TOOLS_PATH/Source/C/bin
fi
export BOOTSECTOR_BIN_DIR=$WORKSPACE/bareBoot/BootSector/bin
export BUILD_DIR=$WORKSPACE/Build/bareBoot$PROCESSOR/"$VTARGET"_"$TARGET_TOOLS"

echo Compressing DUETEFIMainFv.FV ...
$BASETOOLS_DIR/LzmaCompress -e -o $BUILD_DIR/FV/DUETEFIMAINFV.z $BUILD_DIR/FV/DUETEFIMAINFV.Fv

echo Compressing DxeMain.efi ...
$BASETOOLS_DIR/LzmaCompress -e -o $BUILD_DIR/FV/DxeMain.z $BUILD_DIR/$PROCESSOR/DxeCore.efi

echo Compressing DxeIpl.efi ...
$BASETOOLS_DIR/LzmaCompress -e -o $BUILD_DIR/FV/DxeIpl.z $BUILD_DIR/$PROCESSOR/DxeIpl.efi	

echo Generate Loader Image ...

if [ $PROCESSOR = IA32 ]
then
$BASETOOLS_DIR/GenFw --rebase 0x10000 -o $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/$PROCESSOR/EfiLoader.efi
$BASETOOLS_DIR/EfiLdrImage -o $BUILD_DIR/FV/Efildr32 $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/FV/DxeIpl.z $BUILD_DIR/FV/DxeMain.z $BUILD_DIR/FV/DUETEFIMAINFV.z

cat $BOOTSECTOR_BIN_DIR/Start32.com $BOOTSECTOR_BIN_DIR/efi32.com2 $BUILD_DIR/FV/Efildr32 > $BUILD_DIR/FV/Efildr20	
#cat $BOOTSECTOR_BIN_DIR/Start32.com $BOOTSECTOR_BIN_DIR/efi32.com2 $BUILD_DIR/FV/Efildr32 > $BUILD_DIR/FV/boot
dd if=$BUILD_DIR/FV/Efildr20 of=$BUILD_DIR/FV/boot bs=512 skip=1
echo Done!
fi

if [ $PROCESSOR = X64 ]
then
$BASETOOLS_DIR/GenFw --rebase 0x10000 -o $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/$PROCESSOR/EfiLoader.efi
$BASETOOLS_DIR/EfiLdrImage -o $BUILD_DIR/FV/Efildr64 $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/FV/DxeIpl.z $BUILD_DIR/FV/DxeMain.z $BUILD_DIR/FV/DUETEFIMAINFV.z
cat $BOOTSECTOR_BIN_DIR/St32_64.com $BOOTSECTOR_BIN_DIR/efi64.com2 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/Efildr20Pure
#cat $BOOTSECTOR_BIN_DIR/St_20_iT.com $BOOTSECTOR_BIN_DIR/efi64.com2 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/Efildr20Pure
$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/Efildr20Pure -o $BUILD_DIR/FV/Efildr20
cat $BOOTSECTOR_BIN_DIR/St_gpt_iT.com $BOOTSECTOR_BIN_DIR/efi64.com2 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/Efildr20Pure
$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/Efildr20Pure -o $BUILD_DIR/FV/Efildgpt
dd if=$BUILD_DIR/FV/Efildr20 of=$BUILD_DIR/FV/boot bs=512 skip=1

echo Done!
fi

}

fnMainBuildScript
fnMainPostBuildScript

