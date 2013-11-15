#!/bin/bash
# Prepare a valid GTK+-2.0 runtime environment and pack the 
# runtime and the Freeacq binaries in a folder so it can be
# used with the NSIS compiler to create a valid Setup.exe 
# for Microsoft Windows platforms. This script is intended to
# be ran on a Windows platform, with msys mingw compiler
# installed.
#
# Víctor Enríquez Miguel < victor . quicksilver att GM dot com >

HOMEPAGE="http://www.gtk.org/download/win32.php"

GLIB_FILENAME="glib_2.28.8-1_win32.zip"
GLIB_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.28/glib_2.28.8-1_win32.zip"
ATK_FILENAME="atk_1.32.0-2_win32.zip"
ATK_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/atk/1.32/atk_1.32.0-2_win32.zip"
PANGO_FILENAME="pango_1.29.4-1_win32.zip"
PANGO_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/pango/1.29/pango_1.29.4-1_win32.zip"
GDK_PIXBUF_FILENAME="gdk-pixbuf_2.24.0-1_win32.zip"
GDK_PIXBUF_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/gdk-pixbuf/2.24/gdk-pixbuf_2.24.0-1_win32.zip"
GTK_FILENAME="gtk+_2.24.10-1_win32.zip"
GTK_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.24/gtk+_2.24.10-1_win32.zip"
ZLIB_FILENAME="zlib_1.2.5-2_win32.zip"
ZLIB_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/zlib_1.2.5-2_win32.zip"
CAIRO_FILENAME="cairo_1.10.2-2_win32.zip"
CAIRO_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/cairo_1.10.2-2_win32.zip"
LIBPNG_FILENAME="libpng_1.4.3-1_win32.zip"
LIBPNG_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/libpng_1.4.3-1_win32.zip"
FREETYPE_FILENAME="freetype_2.4.2-1_win32.zip"
FREETYPE_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/freetype_2.4.2-1_win32.zip"
FONTCONFIG_FILENAME="fontconfig_2.8.0-2_win32.zip"
FONTCONFIG_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/fontconfig_2.8.0-2_win32.zip"
EXPAT_FILENAME="expat_2.0.1-1_win32.zip"
EXPAT_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/expat_2.0.1-1_win32.zip"
GETTEXT_FILENAME="gettext-runtime_0.18.1.1-2_win32.zip"
GETTEXT_URL="http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/gettext-runtime_0.18.1.1-2_win32.zip"

PACKAGES="GLIB ATK PANGO GDK_PIXBUF GTK ZLIB CAIRO LIBPNG FREETYPE FONTCONFIG EXPAT GETTEXT"

BINARIES="facqcapture.exe facqviewer.exe facqoscilloscope.exe facqplethysmograph.exe"

LANGUAGES="en es"

abort() {
	echo $1;
	exit 1;
}

checkexec() {
	echo Checking $1 ...;
	command -v "$1" >/dev/null 2>&1;
}

mingwinstall() {
	PACKAGE="$1"
	echo "Installing package $1 ..."
	mingw-get install "$1" >/dev/null 2>&1 || abort "Can't install required $PACKAGE Aborting."
}

packageget() {
	wget "$1" -c -O ./packages/"$2" >/dev/null 2>&1 || abort "Error downloading $2"
}

packageunzip() {
	if [ -n "$1" ]
	then
		echo "Uncompressing $1 ..."
		unzip -o ./packages/"$1" -d ./runtime/ >/dev/null 2>&1 || abort "Error uncompressing $1."
	fi
}

# Test for unzip binary, if not found try to install it.
# we need it for unpacking the GTK+-2.0 runtime zip files.
# The package name is msys-unzip.
checkexec unzip || mingwinstall msys-unzip

# Test for wget binary, if not found try to install it.
# we need it for downloading the GTK+-2.0 runtime files.
# The package name is msys-wget.
checkexec wget || mingwinstall msys-wget

# Start with a clean runtime
rm -Rf ./runtime ./freeacq

# Create a packages folder, this will serve as a store place 
# for the runtime files.
mkdir -p packages

# Create a runtime folder, this will serve as the final
# destination for the files contained in the zip runtime
# files.
mkdir -p runtime

# Download the packages and store them in the packages folder.
# Before downloading each file we must check that the file 
# doesn't already exist.
echo Downloading packages from $HOMEPAGE , this may take a while, be patient.
for x in `echo "$PACKAGES"`
do
	URL="${x}"_URL
	URL=${!URL}
	FILENAME="${x}"_FILENAME
	FILENAME=${!FILENAME}
	[ -e ./packages/$FILENAME ] || packageget "$URL" "$FILENAME"
done
echo Downloads completed successful.

# Unpack the zip files in the packages folder to the runtime
# folder.
echo "Uncompressing packages ..."
for x in `echo "$PACKAGES"`
do
	FILENAME="${x}"_FILENAME
	FILENAME=${!FILENAME}
	packageunzip "$FILENAME"
done
echo "Runtime ready"

# Copy the binaries from Freeacq src folder to runtime/bin
for x in $BINARIES
do
	cp -apux ../src/$x ./runtime/bin/ || abort "Error copying binaries"
done

# Copy the LANG.mo files from Freeacq po folder to
# runtime/share/locale/LANG/LC_MESSAGES/freeacq.mo
for x in $LANGUAGES
do
	mkdir -p ./runtime/share/locale/$x/LC_MESSAGES
	cp -apux ../po/$x.gmo ./runtime/share/locale/$x/LC_MESSAGES/freeacq.mo
done

# Test for makensis binary, required to build the Setup.exe
checkexec makensis || abort "makensis required. Aborting"

# Rename runtime to freeacq
mv ./runtime ./freeacq || abort "Can't rename runtime"

# Create the installer
echo "Creating the installer, this will take a while ..."
makensis setup.nsi > makensis.log || abort "makensis failed. Aborting"
echo "Installer created"
exit 0
