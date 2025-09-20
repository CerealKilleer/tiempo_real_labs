#!/usr/bin/env bash
# Uso: ./massif-max.sh massif.out.<pid>

if [ -z "$1" ]; then
  echo "Uso: $0 archivo_massif.out"
  exit 1
fi

FILE="$1"

TABLE=$(ms_print "$FILE")

HEADER=$(echo "$TABLE" | grep -m1 "n       time")

MAX=$(echo "$TABLE" | sed 's/,//g' | awk '
  $1 ~ /^[0-9]+$/ {
    if ($3 > max) {
      max=$3
      line=$0
    }
  }
  END {print line}
')

echo "Máximo:"
echo "$HEADER"
echo "$MAX"
echo
echo "Uso máximo total de memoria: $(echo "$MAX" | awk "{print \$3}") bytes"
