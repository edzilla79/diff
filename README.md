# Diff two speech recognition transcripts

This tool is used to examine speech recognition output and find words/characters that are different in 2 output transcripts. Supports utf-8 string inputs (it's been tested on American English (en-US) and Traditional Chinese (zh-CN)) 

### Prerequisites

Has been tested on MacOS and on Linux

### Compiling

To compile type:
```
make
```


### Running some examples:

For english files, which is word based:
```
./diff data/english.txt 0
```

For chinese files, which are character based:
```
./diff data/chinese.txt 1
```
