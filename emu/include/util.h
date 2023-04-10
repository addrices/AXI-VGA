#pragma once
#include <stdio.h>
#include <assert.h>
#include <sys/syscall.h>

#ifndef __LOG_H__
#define __LOG_H__
enum COLOR
{
  COLOR_INFO = 32,
  COLOR_WARN = 33,
  COLOR_ERROR = 31,
  COLOR_FATAL = 36
};

static int s_color[] = {0, COLOR_INFO, COLOR_WARN, COLOR_ERROR, COLOR_FATAL};

#define AAA(fmt) printf(fmt)

#define PRINT(title, level, file, func, line, fmt, ...)                                       \
  printf("\033[%d;1m", s_color[level]);                                                       \
  printf("[%s], file: %s, func: %s, line: %d, " fmt, title, file, func, line, ##__VA_ARGS__); \
  printf("\033[0m")

#define PRINT_INFO(title, level, fmt, ...) \
  PRINT(title, level, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

//打印颜色，等级分为1，2，3，4
#define tracef(fmt, ...) PRINT_INFO("info", 1, fmt, ##__VA_ARGS__)
#define warnf(fmt, ...) PRINT_INFO("warn", 2, fmt, ##__VA_ARGS__)
#define errorf(fmt, ...) PRINT_INFO("error", 3, fmt, ##__VA_ARGS__)
#define fatalf(fmt, ...) PRINT_INFO("fatal", 4, fmt, ##__VA_ARGS__)

#define LOG_ASSERT(cond, fmt, ...) \
  if (!(cond))                     \
  {                                \
    errorf(fmt, ##__VA_ARGS__);    \
    assert(cond);                  \
  }

#endif
