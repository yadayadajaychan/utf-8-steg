# utf-8-steg
encodes data into zero-width utf-8 characters

This is currently an early prototype and needs a lot of polishing
## Usage
I need to add command line option parsing, but for now, this is the basic format
```
message | steg data > text
steg text > data
text | steg > data
```
Message refers to the utf-8 text which will be visible to viewers. Data is the data to be encoded into utf-8 characters and inserted into the message. Text is the final product with the data encoded in the message.

If no arguments are passed, the program will read text from standard input and output the decoded data to standard output.

If one argument is passed, the program will assume if it is data or text based on whether or not the file contains the magic numbers. If the file is data, the message will be read from standard input and the text will be output to standard output. If the file is text, the data will be decoded and sent to standard output. 

## Compiling
```gcc main.c```

Tested on Arch Linux.

## Encoding
```
Binary encoding: utf-8 hex: Unicode code point: Description
00: 0xe2 0x80 0x8b: U+200B: zero-width space
01: 0xe2 0x80 0x8c: U+200C: zero-width non-joiner
10: 0xe2 0x80 0x8d: U+200D: zero-width joiner
11: 0xe2 0x81 0xa0: U+2060: word joiner

Delimiter: 0xcd 0x8f: U+034F: Combining Grapheme Joiner
Magic: 0xe2 0x80 0x8b 0xcd 0x8f
```
These characters are used to encode two bits of data each and are inserted into the message.

## Limitations
Each zero-width character takes up 3 bytes but only encodes two bits of data, meaning 12 bytes is going to be needed to encode 1 byte of data. When trying to encode very large files, pasting the text into an application can cause it to hang.

This may be common sense, but the message can not contain any of the utf-8 characters used to encode the data. 
## TODO
- [ ] Command line options
- [ ] Manual page
- [ ] Add checksum to verify data integrity
- [ ] Better error handling

