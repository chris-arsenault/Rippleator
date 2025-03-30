#pragma once

#include <JuceHeader.h>
#include <fstream>
#include <string>
#include <mutex>
#include <ctime>
#include <iomanip>

/**
 * A simple debug logger for the Rippleator VST plugin.
 * This class provides file-based logging functionality to help track
 * initialization, audio processing, and ray tracing operations.
 */
class DebugLogger
{
public:
    /**
     * Initialize the logger.
     * This should be called once at the start of the application.
     */
    static void initialize()
    {
// #ifdef NDEBUG
//         return;
// #endif
        std::lock_guard<std::mutex> lock(getMutex());
        
        // Create log file with timestamp in filename
        auto time = std::time(nullptr);
        auto localTime = std::localtime(&time);
        
        std::ostringstream filename;
        filename << "Rippleator_Debug_";
        filename << std::put_time(localTime, "%Y%m%d_%H%M%S");
        filename << ".log";
        
        getLogFilePath() = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                          .getChildFile(filename.str());
        
        // Open the file and write header
        std::ofstream logFile(getLogFilePath().getFullPathName().toStdString(), std::ios::out);
        if (logFile.is_open())
        {
            logFile << "=== Rippleator Debug Log ===" << std::endl;
            logFile << "Started at: " << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << std::endl;
            logFile << "================================" << std::endl << std::endl;
            logFile.close();
        }
    }
    
    /**
     * Log a message to the debug log file.
     * @param message The message to log
     */
    static void log(const std::string& message)
    {
// #ifdef NDEBUG
//         return;
// #endif
        std::lock_guard<std::mutex> lock(getMutex());
        
        // Get current time
        auto time = std::time(nullptr);
        auto localTime = std::localtime(&time);
        
        // Format timestamp
        std::ostringstream timestamp;
        timestamp << std::put_time(localTime, "%H:%M:%S.") 
                 << std::setfill('0') << std::setw(3) 
                 << (juce::Time::getMillisecondCounter() % 1000);
        
        // Open file in append mode
        std::ofstream logFile(getLogFilePath().getFullPathName().toStdString(), std::ios::app);
        if (logFile.is_open())
        {
            logFile << "[" << timestamp.str() << "] " << message << std::endl;
            logFile.close();
        }
    }
    
    /**
     * Log a message with a category prefix.
     * @param category The category of the log message (e.g., "INIT", "AUDIO", "RAY")
     * @param message The message to log
     */
    static void logWithCategory(const std::string& category, const std::string& message)
    {
        log("[" + category + "] " + message);
    }
    
    /**
     * Get the path to the log file.
     * @return The path to the log file
     */
    static juce::File& getLogFilePath()
    {
        static juce::File logFilePath;
        return logFilePath;
    }
    
private:
    /**
     * Get the mutex used for thread safety.
     * @return Reference to the mutex
     */
    static std::mutex& getMutex()
    {
        static std::mutex mutex;
        return mutex;
    }
};