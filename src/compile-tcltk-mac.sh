#!/bin/bash
#**************************************************************
#
#	LSD 9.0 - January 2026
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#	
#	See Readme.txt for copyright information of
#	third parties' code used in LSD
#
#**************************************************************

#**************************************************************
# COMPILE-TCLTK-MAC.SH
# Script to compile Tcl/Tk for macOS.
#**************************************************************

VER="8.6.18"
LSD_APP="LSD.app"
LMM_APP="LMM.app"
INST_APP="installer/LSD Installer.app"

if [[ "$1" == "-h" ]]; then
	echo "Compile Tcl/Tk for macOS and optionally update LSD"
	echo "Usage: ./compile-tcltk-mac.sh [TCL/TK VERSION] [LSD DIRECTORY]"
	exit 0
fi

if [[ "$1" != "" ]]; then
	VER="$1"
fi

if [[ "$2" != "" && -d "$2/$LSD_APP" && -d "$2/$LMM_APP" && -d "$2/$INST_APP" ]]; then
	LSD_DIR="$2"
else
	LSD_DIR=""
fi

ARCH="$( uname -m )"
DEST_DIR=~/"tcltk${VER}_${ARCH}"
CFLAGS="-arch $ARCH -mmacosx-version-min=11.0"
MAIN_VER="$( echo "$VER" | sed -e 's|\.[0-9]*$||' )"

if [[ -d "$DEST_DIR" ]]; then
	echo "Destination directory '$DEST_DIR' already exists"
	[[ "$(read -e -p 'Delete and continue? [y/N]> '; echo $REPLY)" == [Yy]* ]] || exit 1
	rm -rf "$DEST_DIR"
fi

# ensure latest version of Xcode tools are installed
echo "Updating Xcode command line tools"
if [[ "$(read -e -p 'Skip update? [y/N]> '; echo $REPLY)" != [Yy]* ]]; then
	sudo rm -rf /Library/Developer/CommandLineTools
	xcode-select --install
	echo "Please confirm Xcode command line tools installation, and wait until it finishes"
	while ! xcode-select -p &>/dev/null; do
		sleep 5
	done
fi

# download Tcl and Tk sources
if ! mkdir "$DEST_DIR"; then
	echo "Cannot create destination directory '$DEST_DIR', aborting"
	abort 2
fi
TMP_DIR="$( mktemp -d -p $DEST_DIR )"
curl -LO --output-dir "$TMP_DIR" "http://prdownloads.sourceforge.net/tcl/tcl$VER-src.tar.gz"
if [ $? -ne 0 ]; then
	echo "Cannot download 'http://prdownloads.sourceforge.net/tcl/tcl$VER-src.tar.gz', aborting"
	exit 3
fi
curl -LO --output-dir "$TMP_DIR" "http://prdownloads.sourceforge.net/tcl/tk$VER-src.tar.gz"
if [ $? -ne 0 ]; then
	echo "Cannot download 'http://prdownloads.sourceforge.net/tcl/tk$VER-src.tar.gz', aborting"
	exit 4
fi

# extract tarballs
tar -xf "$TMP_DIR/tcl$VER-src.tar.gz" -C "$DEST_DIR"
tar -xf "$TMP_DIR/tk$VER-src.tar.gz" -C "$DEST_DIR"
rm -rf "$TMP_DIR"

# make Tcl and Tk
export CFLAGS
cd "$DEST_DIR"

make -C tcl$VER/macosx deploy
if [[ $? != 0 ]]; then
	echo "Compilation error, aborting"
	exit 5
fi

make -C tk$VER/macosx deploy
if [[ $? != 0 ]]; then
	echo "Compilation error, aborting"
	exit 6
fi

make -C tcl$VER/macosx install-embedded INSTALL_ROOT="$DEST_DIR"
make -C tk$VER/macosx install-embedded INSTALL_ROOT="$DEST_DIR"

if [[ "$LSD_DIR" == "" ]]; then
	echo "Tcl/Tk frameworks were build at '$DEST_DIR',\nbut were not installed to LSD .app bundles"
	exit 0
fi

# update Tcl and Tk frameworks in LSD .app bundles
APP_CONT="Contents"
APP_FRMW="Frameworks"
DEST_FRMW="$APP_CONT/$APP_FRMW"
WISH_CONT="Applications/Utilities/Wish.app/$APP_CONT"
TCL_FRMW="Tcl.Framework"
TK_FRMW="Tk.Framework"
VER_PATH="Versions/$MAIN_VER"
CUR_PATH="Versions/Current"
SRC_TCL="build/tcl/$TCL_FRMW"
SRC_TK="build/tk/$TK_FRMW"
rm -rf "$LSD_DIR/$LSD_APP/$DEST_FRMW"
rm -rf "$LSD_DIR/$LMM_APP/$DEST_FRMW"
rm -rf "$LSD_DIR/$INST_APP/$DEST_FRMW"
cp -Rf "$DEST_DIR/$WISH_CONT/$APP_FRMW" "$LSD_DIR/$LSD_APP/$APP_CONT/"
cp -Rf "$DEST_DIR/$WISH_CONT/$APP_FRMW" "$LSD_DIR/$LMM_APP/$APP_CONT/"
cp -Rf "$DEST_DIR/$WISH_CONT/$APP_FRMW" "$LSD_DIR/$INST_APP/$APP_CONT/"

cp -Rf "$DEST_DIR/$SRC_TCL/$VER_PATH/Headers" "$LSD_DIR/$LSD_APP/$DEST_FRMW/$TCL_FRMW/$VER_PATH/"
cp -Rf "$DEST_DIR/$SRC_TCL/$VER_PATH/PrivateHeaders" "$LSD_DIR/$LSD_APP/$DEST_FRMW/$TCL_FRMW/$VER_PATH/"
cp -Rf "$DEST_DIR/$SRC_TCL/$VER_PATH/libtclstub$MAIN_VER.a" "$LSD_DIR/$LSD_APP/$DEST_FRMW/$TCL_FRMW/$VER_PATH/"

cp -Rf "$DEST_DIR/$SRC_TK/$VER_PATH/Headers" "$LSD_DIR/$LSD_APP/$DEST_FRMW/$TK_FRMW/$VER_PATH/"
cp -Rf "$DEST_DIR/$SRC_TK/$VER_PATH/PrivateHeaders" "$LSD_DIR/$LSD_APP/$DEST_FRMW/$TK_FRMW/$VER_PATH/"
cp -Rf "$DEST_DIR/$SRC_TK/$VER_PATH/libtkstub$MAIN_VER.a" "$LSD_DIR/$LSD_APP/$DEST_FRMW/$TK_FRMW/$VER_PATH/"

pushd "$LSD_DIR/$LSD_APP/$DEST_FRMW/$TCL_FRMW"
ln -s "$CUR_PATH/Headers" "Headers"
ln -s "$CUR_PATH/PrivateHeaders" "PrivateHeaders"
ln -s "$VER_PATH/libtclstub$MAIN_VER.a" "libtclstub$MAIN_VER.a"
popd

pushd "$LSD_DIR/$LSD_APP/$DEST_FRMW/$TK_FRMW"
ln -s "$CUR_PATH/Headers" "Headers"
ln -s "$CUR_PATH/PrivateHeaders" "PrivateHeaders"
ln -s "$VER_PATH/libtkstub$MAIN_VER.a" "libtkstub$MAIN_VER.a"
popd

cp -Rf "$DEST_DIR/$WISH_CONT/MacOS/Wish" "$LSD_DIR/$INST_APP/$APP_CONT/MacOS/"
cp -Rf "$DEST_DIR/$WISH_CONT/Resources/Wish.sdef" "$LSD_DIR/$INST_APP/$APP_CONT/Resources/"
cp -Rf "$DEST_DIR/$WISH_CONT/Resources/Credits.html" "$LSD_DIR/$INST_APP/$APP_CONT/Resources/"

echo "Tcl/Tk frameworks were build at '$DEST_DIR',\nand installed to LSD .app bundles"

exit 0
