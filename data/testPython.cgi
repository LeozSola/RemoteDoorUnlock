#!/usr/bin/env python3

import cgi

# Get the query string from the environment
form = cgi.FieldStorage()

# Get the value of the 'data' parameter from the query string
data = form.getvalue('data', '')

# Process the data (you can modify this as needed)
processed_data = f"Processed data: {data}"

# Generate the HTML page
print("Content-type: text/html\n")
print("<html><head><title>Data Processor</title></head><body>")
print("<h1>Data Processor</h1>")
print(f"<p>Raw Data: {data}</p>")
print(f"<p>{processed_data}</p>")
print("</body></html>")
