#!/bin/sh

GVF=$1
DEF_VER=v0.1-need_annotated_tag_or_version

LF='
'

# First see if there is a version file (included in release tarballs),
# then try git-describe, then default.
if test -f $2
then
	VN=$(cat $2) || VN="$DEF_VER"
elif test -d $3/.git -o -f $3/.git &&
	VN=$(git --git-dir=$3/.git describe --abbrev=4 HEAD 2>/dev/null) &&
	#VN=$(git describe --tags --abbrev=4 HEAD 2>/dev/null) &&
	case "$VN" in
	*$LF*) (exit 1) ;;
	v[0-9]*)
		git --git-dir=$3/.git update-index -q --refresh
		test -z "$(git --work-tree=$3 --git-dir=$3/.git diff-index --name-only HEAD --)" ||
		VN="$VN-dirty" ;;
	esac
then
	: #VN=$(echo "$VN" | sed -e 's/-/./g');
else
	VN="$DEF_VER"
fi

VN=$(expr "$VN" : v*'\(.*\)')

if test -r $GVF
then
	VC=$(sed -e 's/^#define VERSION "\(.*\)"/\1/' <$GVF)
else
	VC=unset
fi
test "$VN" = "$VC" || {
	echo "#define VERSION \"$VN\"" >$GVF
}

