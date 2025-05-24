#!/bin/bash
if [ "$(basename "$PWD")" != "build" ]; then
  cd build || exit 1
fi
./frontend/NextGigFrontend
