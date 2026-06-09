#!/bin/bash
OBJTREE="${KBUILD_OUTPUT:-.}"
echo '#define UTS_RELEASE "4.19.404R"' > "${OBJTREE}/include/generated/utsrelease.h"