#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // Include for inet_ntoa
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h> // For directory handling
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

std::string get_server_directory() {
    char temp[PATH_MAX];
    if (getcwd(temp, sizeof(temp)) == NULL) {
        std::cerr << "Error getting current working directory" << std::endl;
        return "";  // Return an empty string on failure
    }
    return std::string(temp) + "/";
}

// Function to split string by delimiter
std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool is_directory(const std::string& path) {
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) return false;
    return S_ISDIR(statbuf.st_mode);
}

void serve_directory_listing(int connfd, const std::string& path) {
    DIR *dir;
    struct dirent *ent;
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    std::string server_directory = get_server_directory();

    if (path == server_directory) {
        response += "<html><body><h1>Welcome to Webserv!</h1><ul>";
    } else {
        response += "<html><body><h1>Directory Listing of " + path.substr(server_directory.length()) + "</h1><ul>";
    }

    if ((dir = opendir(path.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string name = std::string(ent->d_name);
            if (name == "." || name == ".." || name[0] == '.') continue; // Skip . & .. & hidden

            std::string fullPath = path + (path.back() == '/' ? "" : "/") + name;
            std::string linkPath = fullPath.substr(server_directory.length());  // Strip server directory path for display

            response += "<li><a href='" + linkPath + "'>" + name + "</a></li>";
        }
        closedir(dir);
        response += "</ul></body></html>";
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nDirectory not found";
    }

    write(connfd, response.c_str(), response.length());
}

// Serve the file content
void serve_file(int connfd, const std::string& path) {
    if (is_directory(path)) {
        serve_directory_listing(connfd, path);
        return;
    }
    
    std::ifstream file(path, std::ifstream::binary);
    if (file) {
        // Get length of file:
        file.seekg(0, file.end);
        int length = file.tellg();
        file.seekg(0, file.beg);

        char* buffer = new char[length];

        // Read data as a block:
        file.read(buffer, length);

        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(length) + "\r\n\r\n";
        write(connfd, response.c_str(), response.length());
        write(connfd, buffer, length);

        delete[] buffer;
    } else {
        std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found";
        write(connfd, response.c_str(), response.length());
    }
    file.close();
}

// URL decode function
std::string url_decode(const std::string& encoded_str) {
    std::string decoded_str;
    for (size_t i = 0; i < encoded_str.size(); ++i) {
        if (encoded_str[i] == '%') {
            int char_code;
            sscanf(encoded_str.substr(i + 1, 2).c_str(), "%x", &char_code);
            decoded_str += static_cast<char>(char_code);
            i += 2;
        } else if (encoded_str[i] == '+') {
            decoded_str += ' ';
        } else {
            decoded_str += encoded_str[i];
        }
    }
    return decoded_str;
}


// Execute a CGI script and send the output
void execute_cgi_script(int connfd, const std::string& path, const std::string& parameters) {
    // Check if the script is generating a plot

    std::string decoded_parameters = url_decode(parameters);

    setenv("QUERY_STRING", decoded_parameters.c_str(), 1);

    if (path.find("generateHist.cgi") != std::string::npos) {
        // Open a pipe to my_histogram.py
        if (parameters.empty()) {
            std::string error_message = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nUsage: ?directory=<directory_path>\n";
            write(connfd, error_message.c_str(), error_message.length());
            return;
        }

        bool valid_params = false;
        std::string directory;
        std::vector<std::string> param_parts = split(parameters, '=');
        if (param_parts.size() != 2) {
            std::string error_message = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: Invalid parameter format.\nUsage: ?directory=<directory_path>\n";
            write(connfd, error_message.c_str(), error_message.length());
            return;
        }
        if (param_parts[0] == "directory") {
            directory = url_decode(param_parts[1]);
            valid_params = true;
        }

        if (!valid_params) {
            std::string error_message = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nError: No directory specified.\nUsage: ?directory=<directory_path>\n";
            write(connfd, error_message.c_str(), error_message.length());
            return;
        }

        std::cout << "Directory " << directory.c_str() << std::endl;

        if (!is_directory(directory)) {
            std::string error_message = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nError: Invalid directory specified.\nUsage: ?directory=<directory_path>\n";
            write(connfd, error_message.c_str(), error_message.length());
            return;
        }

        if (directory == "/") {
            directory = "/home/amv1225/CS410/PS3";
        } else if (directory == "/.") {
            directory = "/home/amv1225/CS410/PS3/data";
        } else if (directory == "/..") {
            directory = "/home/amv1225/CS410/PS3";
        }

        FILE* pipe = popen(("python3 my_histogram.py " + directory).c_str(), "r");
        if (!pipe) {
            std::cerr << "Error executing CGI script\n";
            return;
        }

        // Read the output from the pipe and write it to a temporary file
        std::ofstream temp_file("plot.jpeg", std::ios::binary);
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            temp_file.write(buffer, strlen(buffer));
        }
        temp_file.close();

        // Close the pipe
        pclose(pipe);

        // Generate HTML content with embedded image
        std::string html_response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        html_response += "<html><body style='background-color:white;text-align:center;'>\n";
        html_response += "<h1 style='color:red;font-size:16pt;'>CS410 Webserver</h1>\n";
        html_response += "<br/>\n";
        html_response += "<img src='/plot.jpeg' alt='Histogram'/>\n";
        html_response += "</body></html>\n";

        // Send HTML response
        write(connfd, html_response.c_str(), html_response.length());


    } else {
        // Assuming the script is executable

        FILE* pipe = popen(path.c_str(), "r");
        if (!pipe) {
            std::cerr << "Error executing CGI script\n";
            return;
        }

        // Print HTTP headers
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        write(connfd, response.c_str(), response.length());

        // Read output from the CGI script and send it back to the client
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            write(connfd, buffer, strlen(buffer));
        }

        // Close the pipe
        pclose(pipe);
    }
}

// Shell helper function
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Handles requests 
void handle_arduino(int connfd, const std::string& path, const std::vector<std::pair<std::string, std::string>>& parameters_split) {
    const char* device = "/dev/ttyACM0";
    std::string command = path.substr(1); // Remove leading /

    // Password Checking
    if (parameters_split.empty() || parameters_split[0].first != "pass") {
        std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nIncorrect Unlock Request\nCorrect Format: /unlock?pass=*passcode*";
        write(connfd, response.c_str(), response.length());
        return;
    }

    bool passwordCorrect = (parameters_split[0].first == "pass" && parameters_split[0].second == "1234");
    if (!passwordCorrect) {
        std::string response = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain\r\n\r\nFailed Unlock Request\nIncorrect Password :(\nPlease Try Again";
        write(connfd, response.c_str(), response.length());
        return;
    }

    // Setup serial
    std::string setupCommand = "stty -F " + std::string(device) + " cs8 9600 ignbrk -brkint -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts";
    exec(setupCommand.c_str());

    // Prepare command to send to Arduino
    std::string sendCommand = "echo '" + std::string(command) + "' > " + std::string(device);
    
    // Execute the command to send data
    std::string output = exec(sendCommand.c_str());

    // Check for errors (simplified check, consider improving error handling)
    if (output.empty()) {
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nCommand sent to Arduino.";
        write(connfd, response.c_str(), response.length());
    } else {
        std::string response = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nFailed to send command to Arduino. Error: " + output;
        write(connfd, response.c_str(), response.length());
    }
}

// Function to handle HTTP GET requests
void handle_get_request(int connfd, const std::string& request) {
    std::cout << "Handling GET request: " << request << std::endl;
    size_t question_mark = request.find('?');
    std::string path = request.substr(0, question_mark);
    std::string parameters = question_mark != std::string::npos ? request.substr(question_mark + 1) : "";
    // Parameters just splits at ?, still needs & splits

    std::cout << "Path: " << path << std::endl;
    std::string server_directory = get_server_directory();
    std::string full_path;
    std::cout << "Parameters: " << parameters << std::endl;

    // Split parameters into (name, value) pairs
    std::vector<std::pair<std::string, std::string>> parameters_split;
    std::vector<std::string> pairs = split(parameters, '&');

    // For each "name=value" in pairs -> (name, value)
    for (const auto& pair : pairs) {
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            std::string name = pair.substr(0, eq_pos);
            std::string value = pair.substr(eq_pos + 1);
            parameters_split.emplace_back(name, value);
        }
    }

    // parameters_split displayed parameters
    for (const auto& param : parameters_split) {
        std::cout << "Parameter name: " << param.first << ", value: " << param.second << std::endl;
    }

    if (path == "/unlock" || path == "/lock") {
        handle_arduino(connfd, path, parameters_split);
        return;
    }

    if (path.empty() || path == "/") {
        full_path = server_directory;
    } else {
        // Remove leading slash from path to avoid doubles
        if (path[0] == '/') {
            full_path = server_directory + path.substr(1);
        } else {
            full_path = server_directory + path;
        }
    }

    std::cout << "Full Path: " << full_path << std::endl;


    if (path.find(".cgi") != std::string::npos) {
        execute_cgi_script(connfd, full_path, parameters);
    } else {
        serve_file(connfd, full_path);
    }
}

// Function to handle client requests
void handle_request(int connfd) {
    char buffer[1024] = {0};
    read(connfd, buffer, 1024);
    std::string request(buffer);

    if (request.empty()) {
        std::string response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\n\nBad Request: No data received";
        write(connfd, response.c_str(), response.length());
        close(connfd);
        return;
    }

    // Print the raw request
    std::cout << "Received request: " << request << std::endl;

    // Parse the request line
    std::istringstream req_stream(request);
    std::string method;
    std::string url;
    std::string version;
    req_stream >> method >> url >> version;

    std::cout << "Received url: " << url << std::endl;
    std::cout << "Received version: " << version << std::endl;

    if (method.empty() || url.empty() || version.empty()) {
        std::string response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\n\nBad Request: Malformed request line";
        write(connfd, response.c_str(), response.length());
        close(connfd);
        return;
    }

    // Handle according to the method
    if (method == "GET") {
        handle_get_request(connfd, url);
    } else {
        std::string response = "HTTP/1.1 501 Not Implemented\nContent-Type: text/plain\n\nMethod not implemented";
        write(connfd, response.c_str(), response.length());
    }

    std::cout << "====== End of request ====== \n\n" << std::endl;
    close(connfd);

}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./webserv <ip_address>:<port>\n";
        return 1;
    }

    std::string ip_port_str(argv[1]);

    // Split the argument into IP address and port number
    size_t colon_pos = ip_port_str.find(':');
    std::string ip_address;
    std::string port_str;
    int port;

    if (colon_pos != std::string::npos) {
        // IP address and port number provided
        ip_address = ip_port_str.substr(0, colon_pos);
        port_str = ip_port_str.substr(colon_pos + 1);
        port = atoi(port_str.c_str());
    } else {
        // Only port number provided, default IP address to INADDR_ANY
        ip_address = "0.0.0.0"; // Default to localhost
        port_str = ip_port_str;
        port = atoi(port_str.c_str());
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_address.c_str());
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error binding socket\n";
        return 1;
    }

    std::cout << "Server is listening on http://" << ip_address << ":" << port << std::endl;

    if (listen(sockfd, 10) < 0) {
        std::cerr << "Error listening on socket\n";
        return 1;
    }

    while (true) {
        int connfd = accept(sockfd, (struct sockaddr*)NULL, NULL);
        if (connfd < 0) {
            std::cerr << "Error accepting connection\n";
            continue;
        }

        // Fork a new process to handle the request
        if (fork() == 0) {
            close(sockfd);
            handle_request(connfd);
            exit(0);
        }
        close(connfd);
    }

    return 0;
}
