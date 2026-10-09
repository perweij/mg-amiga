#!/bin/sh
# Check an Aminet upload pair against https://wiki.aminet.net rules:
#   amiga/aminet-check.sh mg-amiga.lha mg-amiga.readme [version]
# Names of 30 characters or less, same base name; the readme has LF line
# ends, lines of 78 characters or less, the required fields, a Short: of
# 40 characters or less, an e-mail address as Uploader:, and the given
# Version:; the archive passes lha's own test.
set -eu
arc=$1 readme=$2 want=${3:-}
fail() { echo "aminet-check: $*" >&2; exit 1; }

for f in "$arc" "$readme"; do
	n=$(basename "$f")
	[ ${#n} -le 30 ] || fail "$n: longer than 30 characters"
	echo "$n" | grep -Eq '^[A-Za-z0-9._-]+$' || fail "$n: only letters, digits, . _ -"
done
[ "$(basename "$arc" | sed 's/\.[^.]*$//')" = "$(basename "$readme" .readme)" ] ||
	fail "archive and readme need the same base name"

! grep -q "$(printf '\r')" "$readme" || fail "$readme: CR line ends"
awk 'length > 78 { printf "line %d is %d characters\n", NR, length; bad = 1 }
     END { exit bad }' "$readme" || fail "$readme: lines over 78 characters"
field() { sed -n "s/^$1:[[:space:]]*//p" "$readme" | head -1; }
for f in Short Uploader Type Architecture Version; do
	[ -n "$(field $f)" ] || fail "$readme: no $f: field"
done
short=$(field Short)
[ ${#short} -le 40 ] || fail "Short: is ${#short} characters, at most 40"
field Uploader | grep -Eq '[^ @]+@[^ @]+\.[^ @]+' ||
	fail "Uploader: needs an e-mail address (secret AMINET_EMAIL)"
[ -z "$want" ] || [ "$(field Version)" = "$want" ] ||
	fail "Version: is $(field Version), expected $want"

lha t "$arc" >/dev/null || fail "$arc: lha test failed"
echo "aminet-check: $(basename "$arc") and $(basename "$readme") are fine"
