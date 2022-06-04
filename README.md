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

If one argument is passed, the program will assume if it is data or text based on whether or not the file contains the magic numbers.
## Encoding
TODO

## Limitations
Each zero-width character takes up 3 bytes but only encodes two bits of data, meaning 12 bytes is going to be needed to encode 1 byte of data. When trying to encode very large files, pasting the text into an application can cause it to hang.

This may be common sense, but the message can not contain any of the utf-8 characters used to encode the data. 
## TODO
- Add checksum to verify data integrity
- Better error handling

