This commit changed two things:
- debugged extract_headers. it worked some of the time before but handled the blank line at the end of the headers incorrectly
- added a rudimentary write function to main.c. it has a hardcoded response and writes regardless of ready the client fd is ready or not, this was just for testing

ultimately the actual "bug" just turned out to be that prints were not being flushed to the terminal