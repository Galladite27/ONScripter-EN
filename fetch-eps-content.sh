#!/bin/sh
if git clone https://github.com/Galladite27/ONScripter-EN-EPS eps ; then
    echo "EPS content successfully downloaded. Please read eps/README.md and eps/BuildInstructions for usage information."
else
    echo "EPS content download FAILED."
fi
