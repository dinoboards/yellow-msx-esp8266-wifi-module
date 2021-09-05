

#include "parse-string.h"

size_t parseString(const char *buf, char *output) {
  size_t readCount = 0;
  bool isQuoted = buf[0] == '"'; // whether the string is quoted or just a plain word
  if (isQuoted) {
    readCount++; // move past the opening quote
  }

  size_t i = 0;
  for (; i < MAX_COMMAND_ARG_SIZE && buf[readCount] != '\0'; i++) { // loop through each character of the string literal
    if (isQuoted ? buf[readCount] == '"' : buf[readCount] == ',') {
      break;
    }
    if (buf[readCount] == '\\') { // start of the escape sequence
      readCount++;                // move past the backslash
      switch (buf[readCount]) {   // check what kind of escape sequence it is, turn it into the correct character
      case 'n':
        output[i] = '\n';
        readCount++;
        break;
      case 'r':
        output[i] = '\r';
        readCount++;
        break;
      case 't':
        output[i] = '\t';
        readCount++;
        break;
      case '"':
        output[i] = '"';
        readCount++;
        break;
      case '\\':
        output[i] = '\\';
        readCount++;
        break;
      case 'x': {    // hex escape, of the form \xNN where NN is a byte in hex
        readCount++; // move past the "x" character
        output[i] = 0;
        for (size_t j = 0; j < 2; j++, readCount++) {
          if ('0' <= buf[readCount] && buf[readCount] <= '9') {
            output[i] = output[i] * 16 + (buf[readCount] - '0');
          } else if ('a' <= buf[readCount] && buf[readCount] <= 'f') {
            output[i] = output[i] * 16 + (buf[readCount] - 'a') + 10;
          } else if ('A' <= buf[readCount] && buf[readCount] <= 'F') {
            output[i] = output[i] * 16 + (buf[readCount] - 'A') + 10;
          } else {
            return 0;
          }
        }
        break;
      }
      default: // unknown escape sequence
        return 0;
      }
    } else { // non-escaped character
      output[i] = buf[readCount];
      readCount++;
    }
  }
  if (isQuoted) {
    if (buf[readCount] != '"') {
      return 0;
    }
    readCount++;
  }

  if (buf[readCount] == ',')
    readCount++;

  output[i] = '\0';
  return readCount;
}
