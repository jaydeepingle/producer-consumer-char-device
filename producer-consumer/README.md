Name        : Jaydeep Digambar Ingle
B-Number    : B00671052
Email       : jingle1@binghamton.edu

------------------------------------------------------------------------------
Description:
Kernel Program : In kernel program there are mainly 3 methods
1. Open - Gets called when user calls the open method
2. Read - Reads the lines from the buffer
3. Write - Writes the file sent by producers
4. Close - Gets called when user calls the close method

TASK A:
Learned about named pipe

TASK B:
When we try running one producer and one consumer concurrently and
1. Stops a producer
- The consumer will read but each time it will read EOF and that is why pipe returns
  0 each time to the consumer.
It gets the following message:
error reading ret=0 errno=0 perror: Success

2. Stops a consumer
- Broken pipe error because named pipe is not being read.
It gets the following message:
error writing ret=-1 errno=32 perror: Broken pipe

3. one consumer and multiple producers
It will work fine

4. multiple consumers and one producer
Using script: Race condition with incorrect reads
Consumers will consume the data correctly

5. multiple producers and multiple consumers
Using script: Race condition with incorrect reads
Producers will produce and consumers will consume the data simultaneously
without error.

Also while running multiple producers and consumers there is an issue with 
the named pipe if multiple processes simultaneously going to read or write 
from the named pipe then it may return incorrect result (race condition) 
since its not synchronized. 

TASK C:
We have written a program to deal with simlutaneous read and writes from the
named pipe using mutex and semaphores.

------------------------------------------------------------------------------
Steps to run:
$make - Creates a device

$make insert - Registers device
The value of number of lines which is n is set to 3 for now. We can change it
by editing the make file or we can explicitly run
$sudo insmod linepipe.ko n=<value>

$make compile - Compiles the user programs including producer and consumer

$make produce <pipe-name> - Runs the producer program

$make consume <pipe-name> - Runs the consumer program

$make remove - Deregisters the device

$make clean - Removes unnecessary files
------------------------------------------------------------------------------
