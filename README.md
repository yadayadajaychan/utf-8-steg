# utf-8-steg
encodes/decodes data into zero-width utf-8 characters

This is currently an early prototype and needs a lot of polishing
## Usage
```
Examples:
message | steg -ed data > text
data | steg -em message > text (not implemented yet)
steg -ed data -m message > text
steg -xt text > data
text | steg -x > data
```
Message refers to the utf-8 text which will be visible to viewers. Data is the data to be encoded into utf-8 characters and inserted into the message. Text is the final product with the data encoded in the message.

`-e` or `-c` can be used to specify "encode" and `-x` is used to specify "decode".
`-m` is for the message file, `-d` is for the data file, and `-t` is for the text file. This program can not currently write its output to a file as it opens them read-only.

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
- [ ] Manual page
- [ ] Read data from stdin
- [ ] Add checksum to verify data integrity
- [ ] Better error handling

