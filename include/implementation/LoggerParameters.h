//
// Created by Adrien BLANCHET on 25/04/2021.
//

#ifndef SIMPLE_CPP_LOGGER_LOGGERPARAMETERS_H
#define SIMPLE_CPP_LOGGER_LOGGERPARAMETERS_H

#ifndef LOGGER_MAX_LOG_LEVEL_PRINTED
#define LOGGER_MAX_LOG_LEVEL_PRINTED   6 // 6 = TRACE (max verbosity level)
#endif

#ifndef LOGGER_PREFIX_LEVEL
#define LOGGER_PREFIX_LEVEL   2 // Production
#endif

#ifndef LOGGER_ENABLE_COLORS
#define LOGGER_ENABLE_COLORS   1
#endif

#ifndef LOGGER_ENABLE_COLORS_ON_USER_HEADER
#define LOGGER_ENABLE_COLORS_ON_USER_HEADER   0
#endif

#ifndef LOGGER_PREFIX_FORMAT
#define LOGGER_PREFIX_FORMAT "{TIME} {USER_HEADER} {SEVERITY} {FILELINE} {THREAD}"
#endif

#ifndef LOGGER_CLEAR_LINE_BEFORE_PRINT
#define LOGGER_CLEAR_LINE_BEFORE_PRINT 0
#endif

#ifndef LOGGER_WRITE_OUTFILE
#define LOGGER_WRITE_OUTFILE 0
#endif

#ifndef LOGGER_OUTFILE_NAME_FORMAT
#define LOGGER_OUTFILE_NAME_FORMAT "{EXE}_{TIME}.log"
#endif

#ifndef LOGGER_OUTFILE_FOLDER
#define LOGGER_OUTFILE_FOLDER "."
#endif

#endif //SIMPLE_CPP_LOGGER_LOGGERPARAMETERS_H
