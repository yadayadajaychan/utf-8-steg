# utf-8-steg
encodes/decodes data into zero-width utf-8 characters

This is currently an early prototype and needs a lot of polishing
## Demo
t​͏‌⁠‌​r‌‍‍​y‌‍‍‌ ‌⁠​⁠u​‍​​s‌‍‍‌i‌⁠​⁠n​‍​​g‌‍​‌ ​‍​​t‌⁠​⁠h‌‍‌‌i‌‍​⁠s‌⁠​‍ ‌‍‌‌p‌⁠‌​r​‍​​o‌‍⁠‌g‌‍‌‌r‌⁠​⁠a‌⁠​⁠m‌‍​‌ ‌‍‌⁠t‌‍‌‌o​​‍‍ decode the data hidden in this message

## Usage
```
UTF-8-STEG(1)                    User Commands                   UTF-8-STEG(1)



NAME
       utf-8-steg - encodes/decodes data into zero-width utf-8 characters

SYNOPSIS
       utf-8-steg {-e|-c} [-d data] [-m message]

       utf-8-steg -x [-t text]

DESCRIPTION
       utf-8-steg  is  a  program  for encoding arbritary data into zero-width
       utf-8 characters and decoding it back. These zero-width characters  are
       not rendered as visible chracters and can be used to hide data.

       message refers to the utf-8 text which will be visible to viewers. data
       is the data to be encoded into zero-width utf-8 characters and inserted
       into  the  message.  text is the final product with the data encoded in
       the message.

OPTIONS
   Operation mode
       These options tell utf-8-steg what operation to perform.

       -e, -c In encode mode, utf-8-steg takes data,  encodes  it  into  zero-
              width  characters,  and  inserts  it into message. The resulting
              text is sent to standard output.

       -x     In decode mode, utf-8-steg takes text,  decodes  the  data,  and
              sends it to standard output.

   File selection
       These  options  specify  which  files  to  read from. If not specified,
       utf-8-steg reads from standard input.

       -d     Specifies the data file to read from.

       -m     Specifies the message file to read from.

       -t     Specifies the text file to read from.

   Informative output
       -v     Verbosely explain what is happening. Specify twice for more ver-
              bose output.

EXIT STATUS
       0      Success

       1      Minor errors

       2      Major errors

NOTES
       This program currently can't parse the argument '-' as stdin/stdout. If
       data is read from stdin in encode mode, utf-8-steg will not spread  out
       the  encoded  data  in the message because it can't get the size of the
       input stream.

       Each zero-width character takes up 3 bytes but only encodes two bits of
       data, meaning 12 bytes is going to be needed to encode 1 byte of data.

       message  can not contain any of the utf-8 characters used to encode the
       data, namely U+200B, U+200C, U+200D, U+2060, and U+034F.

BUGS
       Report bugs to https://github.com/yadayadajaychan/utf-8-steg/issues

EXAMPLES
       $ utf-8-steg -ed data

       Encodes data from file named "data", reads message from stdin, and out-
       puts text to stdout

       $ utf-8-steg -em message

       Encodes  data  from stdin, reads message from file named "message", and
       outputs text to stdout

       $ utf-8-steg -ed data -m message

       Encodes data from file named "data",  reads  message  from  file  named
       "message", and outputs text to stdout

       $ utf-8-steg -xt text

       Reads  text from file named "text" and outputs the decoded data to std-
       out

       $ utf-8-steg -x

       Reads text from stdin and outputs the decoded data to stdout

AUTHOR
       This manual page was written by Ethan Cheng <ethanrc0528@gmail.com>

SEE ALSO
       Project page: https://github.com/yadayadajaychan/utf-8-steg



utf-8-steg 0.1.2                  2022-06-12                     UTF-8-STEG(1)
```
## Installing
This program is available in the AUR as [utf-8-steg-git](https://aur.archlinux.org/pkgbase/utf-8-steg-git).

To install using an AUR helper, run `yay -S utf-8-steg-git`

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
This program currently can't parse the argument '-' as stdin/stdout and can't read data from stdin.

Each zero-width character takes up 3 bytes but only encodes two bits of data, meaning 12 bytes is going to be needed to encode 1 byte of data. When trying to encode very large files, pasting the text into an application can cause it to hang.

This may be common sense, but the message can not contain any of the utf-8 characters used to encode the data. 
## TODO
- [x] Command line options
- [x] Manual page
- [x] Read data from stdin
- [ ] Add checksum to verify data integrity
- [ ] Better error handling

