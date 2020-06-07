#!/bin/sh
# GPL License - see http://opensource.org/licenses/gpl-license.php
# Copyright 2005 *nixCoders team - don't forget to credits us

ET_SDK=et-linux-2.60-sdk.x86.run
SDK_FOLDER=sdk

die() {
	echo "Error: $1"
	exit 1
}

mkdir $SDK_FOLDER || die "Can't create sdk folder. Already here ?"
test ! -f $ET_SDK && ( wget ftp://ftp.idsoftware.com/idstuff/et/sdk/$ET_SDK || die "Can't download sdk." )
/bin/sh $ET_SDK --tar xfC $SDK_FOLDER || die "Can't extract sdk."

# Cleanup sdk
find $SDK_FOLDER -not -name "q_math.c" -not -iname "*.h" -exec rm -f \{\} \; 2> /dev/null
find $SDK_FOLDER -type d -exec rmdir -p \{\} \; 2> /dev/null
# Change 'Window' var name for no conflict with xlib
sed -e "s/Window/eth_Window/" $SDK_FOLDER/src/ui/ui_shared.h -i
exit 0
