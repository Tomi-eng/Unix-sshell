# Project 1 Report: shell.c

This project was done by Folatomi Akeju and Jerrie Kraus-Liang for ECS150,
Winter 2021.	

## High-level design choices

* Refactoring Parser Functionality

The most prominent design choice that stands out was the separation of the
parser functionality into two main, separate functions parse_cmd and parse_arg. 
This was mainly done at the point where we started to implement functionality
for pipelining, as in cases where we received a pipeline of commands, we not
only needed to separate the command name from the arguments in each command, but
also to separate each piped command from one another. As a result, we made a
high-level design choice to first have our user input go through parse_cmd if a
pipe was encountered, and then go through parse_arg to separate arguments from
one another, which was then fed back into our command objects as easy reference
for later processing. Because the functions had a clear step and purpose in the
parsing, it made it easy for modifications to be made during error checking when
a change needed to be done, thus making it a good design choice.

When implementing output redirection, we found that we needed to get rid of any
spaces if encountered and clean up the file name; therefore, we decided to
create a helper function set_file for parse_arg that did the cleaning by getting
rid of any whitespaces.

* Parsing Commands as Structs

Commands were taken as structs in order to implement the parser functions as
C++like methods that receive our command object as a parameter, which helped in
instances such as moving out parser functionality into its own dedicated
function to improve readability and clarity in our code. It also helped keep
track of relevant data pertaining to the user commands and allowed us to access
member variables easily through the struct, which especially helped in more
complex cases such as pipelining where it was necessary to keep track of
multiple commands and their corresponding arguments (which we did so with std_in
structs), thus making it a good design choice overall. 

* Error Handling

We took advantage of the fact that, if execvp() is successful, then the process
swaps and doesn't run the next lines in the program, and because we did all of
our other error checking cases beforehand, we could finally handle the case
where the command isn't found simply by outputting it after execvp(). This
simplified handling this error as it may be more complex to do error checking
first and ensuring the command exists first before trying to execute the
command.

* Pipelining

We took advantage of the fact that, according to the project assignment, we
would only be required up to pass cases for up to three pipes in regards to
pipelining. Due to time, we were unable to make a scalable solution to work for
more than three pipes.

## References 

[Ref 1](https://www.gnu.org/software/libc/manual/html_node/Access-Modes.html),
[Ref 2](http://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html): These
links were referred to when searching for macros which allowed our file
descriptor to write to an output file.
