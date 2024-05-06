#!/usr/bin/env python3

import os
from urllib.parse import parse_qs

# Print HTTP headers
print("Content-type: text/html\n")

# Parse query string parameters
query_string = os.getenv("QUERY_STRING", "")
parameters = parse_qs(query_string)

# Start HTML response
print("<html><head><title>CGI Script</title></head><body>")
print("<h1>CGI Script Output</h1>")

# Print the entire query string
print("<p>Query String: {}</p>".format(query_string))

# Print parameters
print("<p>Parameters:</p>")
print("<ul>")
for key, value in parameters.items():
    print("<li>{}: {}</li>".format(key, ", ".join(value)))
print("</ul>")

# Get value of 'name' parameter
name = parameters.get('name', [''])[0]

# Check if 'name' parameter was provided
if name:
    print("<p>Hello, {}!</p>".format(name))
else:
    print("<p>No name provided in query string.</p>")

# End HTML response
print("</body></html>")
