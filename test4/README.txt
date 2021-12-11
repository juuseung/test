Group 3 - The Best Group
Teammates: Juseung Lee, Kunal Patil, Jiaxing Wu

#This file records the implementataion of B+-tree index for assignment4#
-----------------------------------------------------------------------------------------------

There are four type of functions in the index manager:
  * Index Manager functions
  * B+-tree functions
  * Key functions
  * Debug functions

* Index Manager functions
  : These functions are used to initialize the index manager and shut it down, 
  freeing up all acquired resources.
* B+-tree functions
  : These functions are used to create or delete a b-tree index.
* Key functions
  : These functions are used to find, insert, and delete keys in/from a given B+-tree.
* Debug functions
  : These functions are used in the test cases and can be helpful for debugging.
 

-----------------------------------------------------------------------------------------------
Testing and Running: All of the tests are administered through the test_assign4_1.c file.

To compile and run the tests, type:
make
valgrind ./test_assign4

To clean up after tests, type:
make clean

