//
// Created by Nadrino on 24/08/2020.
//

#ifndef SIMPLE_CPP_LOGGER_LOGGER_IMPL_H
#define SIMPLE_CPP_LOGGER_LOGGER_IMPL_H

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <string>
#include <cstring>
#include <utility>
#include <vector>
#include <thread>
#include <map>
#include <mutex>
#include <type_traits>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>


namespace {

  // Setters
  void Logger::setMaxLogLevel(int maxLogLevel_) {
    setMaxLogLevel(getLogLevel(maxLogLevel_));
  }

  void Logger::setMaxLogLevel(LogLevel maxLogLevel_) {
    _maxLogLevel_ = maxLogLevel_;
  }

  void Logger::setEnableColors(bool enableColors_) {
    _enableColors_ = enableColors_;
  }

  void Logger::setPropagateColorsOnUserHeader(bool propagateColorsOnUserHeader_) {
    _propagateColorsOnUserHeader_ = propagateColorsOnUserHeader_;
  }

  void Logger::setPrefixLevel(PrefixLevel prefixLevel_) {
    _prefixLevel_ = prefixLevel_;
  }

  void Logger::setUserHeaderStr(std::string userHeaderStr_) {
    _userHeaderStr_ = std::move(userHeaderStr_);
  }


  // Getters
  Logger::LogLevel Logger::getLogLevel(int logLevelInt_) {
    switch (logLevelInt_) {
      case 0:
        return (LogLevel::FATAL);
      case 1:
        return (LogLevel::ERROR);
      case 2:
        return (LogLevel::ALERT);
      case 3:
        return (LogLevel::WARNING);
      case 4:
        return (LogLevel::INFO);
      case 5:
        return (LogLevel::DEBUG);
      default:
        return (LogLevel::TRACE);
    }
  }

  Logger::PrefixLevel Logger::getPrefixLevel(int prefixLevelInt_) {
    switch (prefixLevelInt_) {
      case 0:
        return (PrefixLevel::NONE);
      case 1:
        return (PrefixLevel::MINIMAL);
      case 2:
        return (PrefixLevel::PRODUCTION);
      case 3:
        return (PrefixLevel::DEBUG);
      default:
        return (PrefixLevel::FULL);
    }
  }

  int Logger::getMaxLogLevelInt() {
    switch (_maxLogLevel_) {
      case LogLevel::FATAL:
        return 0;
      case LogLevel::ERROR:
        return 1;
      case LogLevel::ALERT:
        return 2;
      case LogLevel::WARNING:
        return 3;
      case LogLevel::INFO:
        return 4;
      case LogLevel::DEBUG:
        return 5;
      default:
        return 6;

    }
  }

  Logger::LogLevel Logger::getMaxLogLevel() {
    return _maxLogLevel_;
  }

  std::string Logger::getPrefixString() {
    buildCurrentPrefix();
    return _currentPrefix_;
  }

  std::string Logger::getPrefixString(Logger loggerConstructor){
    // Calling the constructor will automatically update the fields
    return Logger::getPrefixString();
  }


  // User Methods
  void Logger::quietLineJump() {
    _outputStream_ << std::endl;
  }


  // Macro-Related Methods
  Logger::Logger(LogLevel level, char const *file, int line) {
    if (level != _currentLogLevel_) _isNewLine_ = true; // force reprinting the prefix if the verbosity has changed

    _currentLogLevel_ = level;
    _currentFileName_ = Logger::splitString(file, "/").back();
    _currentLineNumber_ = line;
  }

  template<typename... TT>
  void Logger::operator()(const char *fmt_str, TT &&... args) {

    if (_currentLogLevel_ > _maxLogLevel_) return;
    std::lock_guard<std::mutex> lockGuard(_loggerMutex_);

    printFormat(fmt_str, std::forward<TT>(args)...);

    if (not _disablePrintfLineJump_ and fmt_str[strlen(fmt_str) - 1] != '\n') {
      _outputStream_ << std::endl;
      _isNewLine_ = true;
    }
  }

  template<typename T>
  Logger &Logger::operator<<(const T &data) {

    if (_currentLogLevel_ > _maxLogLevel_) return *this;

    std::stringstream dataStream;
    dataStream << data;

    std::lock_guard<std::mutex> lockGuard(_loggerMutex_);
    printFormat(dataStream.str().c_str());

    return *this;
  }

  Logger &Logger::operator<<(std::ostream &(*f)(std::ostream &)) {

    // Handling std::endl
    if (_currentLogLevel_ > _maxLogLevel_) return *this;

    _outputStream_ << f;
    _isNewLine_ = true;

    return *this;
  }


  // Protected Methods
  void Logger::buildCurrentPrefix() {
    _currentPrefix_ = ""; // reset

    // Time
    if (Logger::_prefixLevel_ >= Logger::PrefixLevel::PRODUCTION) {
      time_t rawTime = std::time(nullptr);
      struct tm timeInfo = *localtime(&rawTime);
      std::stringstream ss;
      ss << std::put_time(&timeInfo, "%H:%M:%S");
      _currentPrefix_ += ss.str();
    }

    // User header
    if(not _userHeaderStr_.empty()){
      if(not _currentPrefix_.empty()) _currentPrefix_ += " ";
      if(_enableColors_ and _propagateColorsOnUserHeader_) _currentPrefix_ += getTagColorStr(_currentLogLevel_);
      _currentPrefix_ += _userHeaderStr_;
      if(_enableColors_ and _propagateColorsOnUserHeader_) _currentPrefix_ += "\033[0m";
    }

    // Severity Tag
    if (Logger::_prefixLevel_ >= Logger::PrefixLevel::MINIMAL){
      if (not _currentPrefix_.empty()) _currentPrefix_ += " ";
      if (_enableColors_) _currentPrefix_ += getTagColorStr(_currentLogLevel_);
      char buffer[6];
      snprintf(buffer, 6, "%5.5s", getTagStr(_currentLogLevel_).c_str());
      _currentPrefix_ += buffer;
      if (_enableColors_) _currentPrefix_ += "\033[0m";
    }

    // Filename and Line#
    if (Logger::_prefixLevel_ >= Logger::PrefixLevel::DEBUG) {
      _currentPrefix_ += " ";
      if (_enableColors_) _currentPrefix_ += "\x1b[90m";
      _currentPrefix_ += _currentFileName_;
      _currentPrefix_ += ":";
      _currentPrefix_ += std::to_string(_currentLineNumber_);
      if (_enableColors_) _currentPrefix_ += "\033[0m";
    }

    if (Logger::_prefixLevel_ >= Logger::PrefixLevel::FULL){
      _currentPrefix_ += " ";
      if (_enableColors_) _currentPrefix_ += "\x1b[90m";
      _currentPrefix_ += "(thread: ";
      std::stringstream ss;
      ss << std::this_thread::get_id();
      _currentPrefix_ += ss.str();
      _currentPrefix_ += ")";
      if (_enableColors_) _currentPrefix_ += "\033[0m";
    }

    if (not _currentPrefix_.empty()){
      _currentPrefix_ += ": ";
    }
  }

  std::string Logger::getTagColorStr(LogLevel selectedLogLevel_) {

    switch (selectedLogLevel_) {

      case Logger::LogLevel::FATAL:
        return "\033[41m";
      case Logger::LogLevel::ERROR:
        return "\033[31m";
      case Logger::LogLevel::ALERT:
        return "\033[35m";
      case Logger::LogLevel::WARNING:
        return "\033[33m";
      case Logger::LogLevel::INFO:
        return "\x1b[32m";
      case Logger::LogLevel::DEBUG:
        return "\x1b[94m";
      case Logger::LogLevel::TRACE:
        return "\x1b[36m";
      default:
        return "";

    }

  }

  std::string Logger::getTagStr(LogLevel selectedLogLevel_) {

    switch (selectedLogLevel_) {

      case Logger::LogLevel::FATAL:
        return "FATAL";
      case Logger::LogLevel::ERROR:
        return "ERROR";
      case Logger::LogLevel::ALERT:
        return "ALERT";
      case Logger::LogLevel::WARNING:
        return "WARN";
      case Logger::LogLevel::INFO:
        return "INFO";
      case Logger::LogLevel::DEBUG:
        return "DEBUG";
      case Logger::LogLevel::TRACE:
        return "TRACE";
      default:
        return "";

    }
  }

  template<typename ... Args> void Logger::printFormat(const char *fmt_str, Args ... args ){

    std::string formattedString;
    if(sizeof...(Args) == 0) formattedString = fmt_str; // If there no extra args, string formatting is not needed
    else formattedString = formatString(fmt_str, std::forward<Args>(args)...);

    // Check if there is multiple lines
    if( doesStringContainsSubstring(formattedString, "\n") ) {

      // Print each line individually
      auto slicedString = splitString(formattedString, "\n");
      for (int i_line = 0; i_line < int(slicedString.size()); i_line++) {

        // If the last line is empty, don't print since a \n will be added.
        // Let the parent function do it.
        if (i_line == (slicedString.size()-1) and slicedString[i_line].empty()) {
          if(formattedString.back() == '\n') _isNewLine_ = true;
          break;
        }

        // The next printed line should contain the prefix
        _isNewLine_ = true;

        // Recurse
        printFormat(slicedString[i_line].c_str());

        // let the last line jump be handle by the user (or the parent function)
        if (i_line != (slicedString.size() - 1)) {
          _outputStream_ << std::endl;
        }

      } // for each line
    } // If multiline
    else{

      if(_isNewLine_){
        Logger::buildCurrentPrefix();
        _outputStream_ << _currentPrefix_;
        _isNewLine_ = false;
      }

      if (_enableColors_ and _currentLogLevel_ == LogLevel::FATAL)
        _outputStream_ << formatString("%s", getTagColorStr(LogLevel::FATAL).c_str());

      _outputStream_ << formattedString;

      if (_enableColors_ and _currentLogLevel_ == LogLevel::FATAL)
        _outputStream_ << formatString("\033[0m");
    } // else multiline
  }


  // Generic Functions
  std::vector<std::string> Logger::splitString(const std::string& input_string_, const std::string& delimiter_) {

    std::vector<std::string> output_splited_string;

    const char *src = input_string_.c_str();
    const char *next = src;

    std::string out_string_piece;

    while ((next = std::strstr(src, delimiter_.c_str())) != nullptr) {
      out_string_piece = "";
      while (src != next) {
        out_string_piece += *src++;
      }
      output_splited_string.emplace_back(out_string_piece);
      /* Skip the delimiter_ */
      src += delimiter_.size();
    }

    /* Handle the last token */
    out_string_piece = "";
    while (*src != '\0')
      out_string_piece += *src++;

    output_splited_string.emplace_back(out_string_piece);

    return output_splited_string;

  }

  template<typename ... Args>
  std::string Logger::formatString( const char *fmt_str, Args ... args )  {
    size_t size = snprintf( nullptr, 0, fmt_str, args ... ) + 1; // Extra space for '\0'
    if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, fmt_str, args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
  }

  bool Logger::doesStringContainsSubstring(std::string string_, std::string substring_, bool ignoreCase_){
    if(substring_.size() > string_.size()) return false;
    if(ignoreCase_){
      string_ = toLowerCase(string_);
      substring_ = toLowerCase(substring_);
    }
    if(string_.find(substring_) != std::string::npos) return true;
    else return false;
  }

  std::string Logger::toLowerCase(std::string& inputStr_){
    std::string output_str(inputStr_);
    std::transform(output_str.begin(), output_str.end(), output_str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return output_str;
  }


  // Private Members
  bool Logger::_enableColors_ = LOGGER_ENABLE_COLORS;
  bool Logger::_propagateColorsOnUserHeader_ = LOGGER_ENABLE_COLORS_ON_USER_HEADER;
  bool Logger::_disablePrintfLineJump_ = false;
  Logger::LogLevel Logger::_maxLogLevel_ = Logger::getLogLevel(LOGGER_MAX_LOG_LEVEL_PRINTED);
  Logger::PrefixLevel Logger::_prefixLevel_ = Logger::getPrefixLevel(LOGGER_PREFIX_LEVEL);
  std::string Logger::_userHeaderStr_;

  std::string Logger::_currentPrefix_;
  Logger::LogLevel Logger::_currentLogLevel_ = Logger::LogLevel::TRACE;
  std::string Logger::_currentFileName_;
  int Logger::_currentLineNumber_ = -1;
  bool Logger::_isNewLine_ = true;
  std::ostream& Logger::_outputStream_ = std::cout;
  std::mutex Logger::_loggerMutex_;

}

#endif //SIMPLE_CPP_LOGGER_LOGGER_IMPL_H