#!/bin/sh
if git clone --depth 1 --branch master https://github.com/Galladite27/ONScripter-EN-EPS eps >/dev/null 2>&1 ; then
    echo "EPS content successfully downloaded. Please read eps/README.md and eps/BuildInstructions.md for usage information."
else
    echo "EPS content download FAILED."
fi
