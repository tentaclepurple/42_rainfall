#!/bin/bash

for i in {0..9}; do
  mkdir -p "level$i/Ressources"
  touch "level$i/flag"
  touch "level$i/source"
  touch "level$i/walkthrough"
done

for i in {0..3}; do
  mkdir -p "bonus$i/Ressources"
  touch "bonus$i/flag"
  touch "bonus$i/source"
  touch "bonus$i/walkthrough"
done