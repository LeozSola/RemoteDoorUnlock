import os
import sys
import stat
import matplotlib.pyplot as plt

def generate_histogram(directory):
    # Dictionary to store file type counts
    file_type_counts = {
        'regular': 0,
        'directory': 0,
        'link': 0,
        'fifo': 0,
        'socket': 0,
        'block': 0,
        'character': 0
    }

    # Traverse the directory and count file types
    for root, dirs, files in os.walk(directory):

        for file in files:
            file_path = os.path.join(root, file)
            # Get file type using os.stat
            file_stat = os.lstat(file_path)
            if os.path.isfile(file_path):
                file_type = 'regular'
            elif os.path.islink(file_path):
                file_type = 'link'
            elif stat.S_ISFIFO(file_stat.st_mode):
                file_type = 'fifo'
            elif stat.S_ISSOCK(file_stat.st_mode):
                file_type = 'socket'
            elif stat.S_ISBLK(file_stat.st_mode):
                file_type = 'block'
            elif stat.S_ISCHR(file_stat.st_mode):
                file_type = 'character'
            file_type_counts[file_type] += 1
        for dir in dirs:
            file_path = os.path.join(root, dir)
            if os.path.isdir(file_path):
                file_type_counts['directory'] += 1
    # Generate the histogram
    file_types = list(file_type_counts.keys())
    counts = list(file_type_counts.values())
    plt.bar(file_types, counts)
    plt.xlabel('File Type')
    plt.ylabel('Frequency')
    plt.title('File Type Histogram')
    plt.savefig('plot.jpeg')  # Save the histogram as a JPEG file

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python my_histogram.py <directory>")
        sys.exit(1)

    directory = sys.argv[1]
    generate_histogram(directory)
