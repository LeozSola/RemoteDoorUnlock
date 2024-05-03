#!/bin/bash

# Print HTTP headers
echo "Content-type: text/html"
echo ""

# Output HTML page
echo "<html><head><title>CGI Example</title></head><body>"
echo "<h1>CGI Example</h1>"
echo "<p>Query String: $QUERY_STRING</p>"
echo "</body></html>"
