#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <csignal>

bool running = true;

void signalHandler(int signum) {
    std::cout << "\nStopping monitoring via signal...\n";
    running = false;
}

void logPageFaultsAndMemoryStats() {
    std::ofstream log("page_fault_log.txt");
    if (!log) {
        std::cerr << "Failed to open log file.\n";
        return;
    }

    long prevMinor = -1, prevMajor = -1;
    int sameCount = 0;

    while (running) {
        std::ifstream statusFile("/proc/self/status");
        std::string line;
        long minorFaults = 0;
        long majorFaults = 0;
        std::string vmSize, vmRSS, vmSwap;

        while (getline(statusFile, line)) {
            if (line.find("Minflt:") != std::string::npos)
                minorFaults = std::stol(line.substr(line.find(":") + 1));
            if (line.find("Majflt:") != std::string::npos)
                majorFaults = std::stol(line.substr(line.find(":") + 1));
            if (line.find("VmSize:") != std::string::npos)
                vmSize = line;
            if (line.find("VmRSS:") != std::string::npos)
                vmRSS = line;
            if (line.find("VmSwap:") != std::string::npos)
                vmSwap = line;
        }

        // Get timestamp
        std::time_t now = std::time(nullptr);
        char timeBuf[100];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        // Check for unchanged faults
        if (minorFaults == prevMinor && majorFaults == prevMajor)
            sameCount++;
        else
            sameCount = 0;

        prevMinor = minorFaults;
        prevMajor = majorFaults;

        // Log to file
        log << "[" << timeBuf << "] Minor Faults: " << minorFaults
            << " | Major Faults: " << majorFaults
            << " | Stable Count: " << sameCount << "\n";
        log << "    " << vmSize << "\n";
        log << "    " << vmRSS << "\n";
        log << "    " << vmSwap << "\n\n";
        log.flush();

        std::cout << "Logged at " << timeBuf << " | Minor: " << minorFaults << ", Major: " << majorFaults << "\n";

        if (sameCount >= 5) {
            std::cout << "No change in page faults for 5 cycles. Stopping...\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    log << "Monitoring stopped due to stable page faults or signal.\n";
    log.close();
}

int main() {
    signal(SIGINT, signalHandler);
    std::cout << "Monitoring page faults and memory usage... Press Ctrl+C to stop.\n";
    logPageFaultsAndMemoryStats();
    return 0;
}