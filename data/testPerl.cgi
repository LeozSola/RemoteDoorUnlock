#!/usr/bin/perl

use strict;
use warnings;

# Get the raw data from the query string
my $raw_data = $ENV{'QUERY_STRING'};

# Print out the value of the QUERY_STRING for debugging
print "Content-type: text/plain\n\n";
print "QUERY_STRING: $raw_data\n";  # Print out the value of the QUERY_STRING for debugging

# Process the raw data (you can modify this as needed)
my $processed_data = "Processed data: $raw_data";

# Generate the HTML page
print "Content-type: text/html\n\n";
print "<html><head><title>Data Processor</title></head><body>\n";
print "<h1>Data Processor</h1>\n";
print "<p>Raw Data: $raw_data</p>\n";
print "<p>$processed_data</p>\n";
print "</body></html>\n";
