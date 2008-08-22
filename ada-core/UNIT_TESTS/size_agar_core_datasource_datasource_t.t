#!/bin/sh
# auto generated, do not edit

size_ada=`./ada_size "agar.core.datasource.datasource_t"`
if [ $? -ne 0 ]; then exit 2; fi
size_c=`./c_size "struct ag_data_source"`
if [ $? -ne 0 ]; then exit 2; fi

printf "%8d %8d %s -> %s\n" "${size_ada}" "${size_c}" "agar.core.datasource.datasource_t" "struct ag_data_source"

if [ ${size_ada} -ne ${size_c} ]
then
  echo "error: size mismatch" 1>&2
  exit 1
fi