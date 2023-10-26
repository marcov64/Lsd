#!/bin/bash
#**************************************************************
#
#	LSD 8.1 - July 2023
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
# CREATE-INSTALLER-LINUX.SH
# Create LSD installer for Linux.
#**************************************************************

LSD_VER_NUM="8.1"
LSD_VER_TAG="stable-2"

if [ "$1" = "-h" ]; then
	echo "Create LSD installer for Linux"
	echo "Usage: ./create-installer-linux.sh [LSD FOLDER]"
	exit 0
fi

LSD_DIR="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd -P )"

if [ -z "$1" ]; then
	if [ ! -f "$LSD_DIR"/LMM ]; then
		echo "No LSD FOLDER provided or found, aborting"
		exit 1
	fi
else
	if [ -f "$1"/LMM ]; then
		LSD_DIR="$1"
	else
		echo "Provided LSD FOLDER not found, aborting"
		exit 2
	fi
fi

README="Readme.txt"
LSD_VER="$LSD_VER_NUM-$LSD_VER_TAG"
LSD_FILE_TAG="${LSD_VER//./-}"
INST_DIR="$LSD_DIR/installer"
FILENAME="LSD-installer-linux"

# save a Work folder template
mkdir $LSD_DIR/Work.template
cp -f $LSD_DIR/Work/*.txt $LSD_DIR/Work.template/

# create installer script
rm -f $INST_DIR/$FILENAME-$LSD_FILE_TAG.sh
$INST_DIR/makeself/makeself.sh \
--tar-extra "--exclude-from=$INST_DIR/exclude-installer-linux.txt" \
$LSD_DIR $INST_DIR/$FILENAME-$LSD_FILE_TAG.sh \
"LSD Installer ($LSD_VER)" \
./src/installer-loader-linux.sh

# create compressed distribution file (to preserve permissions)
rm -f $INST_DIR/$FILENAME-$LSD_FILE_TAG.zip
cd $INST_DIR
cp -f $LSD_DIR/$README $INST_DIR/
zip -q -9 $FILENAME-$LSD_FILE_TAG.zip $FILENAME-$LSD_FILE_TAG.sh $README
cd - > /dev/null

# cleanup
rm -f -R $LSD_DIR/Work.template $INST_DIR/$FILENAME-$LSD_FILE_TAG.sh $INST_DIR/$README

if [ -f "$INST_DIR/$FILENAME-$LSD_FILE_TAG.zip" ]; then
	echo "LSD installer package created: $INST_DIR/$FILENAME-$LSD_FILE_TAG.zip"
else
	echo "Error creating LSD installer"
fi
