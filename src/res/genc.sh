#!/bin/bash
for i in *.bmp
do
  ../../gencimg "$i" >"$i".i
done
