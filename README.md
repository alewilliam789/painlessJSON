# painlessJSON
This a simple, lightweight JSON parser that provides expressive error messages.
<br>
<br>

## Motivation
While using some online JSON parsers I thought that the error messages were poor. This parser was built to provide expressive error messages that describe the type of error made and the location of that error.
<br>
<br>

## Installing
To build the CLI run the command in the current directory:
```
make painless
```
That will build you an executable that you can call to process and parse your JSON:
```
./painless your_json_file_here.json
```
<br>
<br>
<br>
There are tests written and if you would like to run the tests go into the directory and run:
```
make test
./a.out
```

### Side note
I broke this off from the c-what-you-can-do repository. If you would like to see the full commit history I kept that repository up to check it out.
